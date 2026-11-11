"""Installed package inventory, source notices and distribution publication."""

import hashlib
import json
import os
from pathlib import Path
import re
import shlex
import shutil
import tarfile

from common import copy_git_source


def installed_packages(destdir, database_roots):
    packages = []
    for relative in database_roots:
        root = destdir / relative
        database = root / "var/db/pkg"
        if not database.is_file():
            continue
        for line in database.read_text().splitlines():
            fields = shlex.split(line)
            if len(fields) != 6:
                raise RuntimeError(f"Malformed package database entry: {line}")
            name, version, origin, prefix, category, manifest = fields
            entries = (database.parent / manifest).read_text().splitlines()
            packages.append({
                "name": name.rstrip(":"), "version": version,
                "origin": origin, "prefix": prefix, "root": root,
                "entries": entries,
            })
    return packages


def package_paths(package):
    root = package["root"] / package["prefix"].lstrip("/")
    for entry in package["entries"]:
        if not entry or entry.startswith("@comment"):
            continue
        if entry.startswith("@dir "):
            entry = entry[5:]
        elif entry.startswith("@"):
            raise RuntimeError(f"Unsupported export manifest entry: {entry}")
        relative = Path(entry)
        if relative.is_absolute() or ".." in relative.parts:
            raise RuntimeError(f"Unsafe export manifest entry: {entry}")
        yield root / relative, relative


def copy_components(packages, bundle, components, prefix):
    selected = []
    for origin, name in components.items():
        matches = [p for p in packages if p["origin"] == origin and
                   p["name"] == name and p["prefix"] == prefix]
        if len(matches) != 1:
            raise RuntimeError(f"Missing installed component: {name}")
        package = matches[0]
        selected.append(package)
        for source, relative in package_paths(package):
            target = bundle / relative
            if source.is_symlink():
                link = os.readlink(source)
                resolved = (target.parent / link).resolve()
                if (Path(link).is_absolute() or
                        not resolved.is_relative_to(bundle)):
                    raise RuntimeError(f"Nonrelocatable package link: {source}")
                target.parent.mkdir(parents=True, exist_ok=True)
                target.symlink_to(link)
            elif source.is_dir():
                target.mkdir(parents=True, exist_ok=True)
            elif source.is_file():
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(source, target)
            else:
                raise RuntimeError(f"Missing installed package file: {source}")
    return selected


def source_notices(project, package, notices):
    destination = notices / package["name"]
    destination.mkdir(parents=True, exist_ok=True)
    origin = package["origin"]
    port = project / "feeds" / origin
    if not port.is_dir():
        port = project / "make/uports" / origin
    for name in ("Makefile", "distinfo", f"distinfo.{package['version']}"):
        source = port / name
        if source.is_file():
            shutil.copy2(source, destination / name)
    if (port / "files").is_dir():
        shutil.copytree(port / "files", destination / "patches")
    found = False
    for work in sorted(port.glob("work.*")):
        if not work.is_dir():
            continue
        for source in work.iterdir():
            if not source.is_dir() or source.name in ("stage", "build", "pkg"):
                continue
            for license_file in source.iterdir():
                if license_file.is_file() and re.match(
                    r"^(COPYING|COPYRIGHT|LICENSE|LICENCE)([.\-].*)?$",
                    license_file.name, re.I,
                ):
                    shutil.copy2(license_file, destination / license_file.name)
                    found = True
    if not found:
        raise RuntimeError(f"Missing prepared source licenses for {origin}")


def install_validation(bundle, source, profile, environment):
    directory = bundle / "share/toolchain/validation"
    directory.mkdir(parents=True)
    templates = Path(__file__).resolve().parent
    shutil.copy2(templates / "validate.sh", bundle / "validate.sh")
    (bundle / "validate.sh").chmod(0o755)
    for name in ("common", *profile.VALIDATORS):
        target = directory / (name + ".sh")
        shutil.copy2(templates / "validators" / target.name, target)
        target.chmod(0o755)
    (directory / "profile.conf").write_text("".join(
        name + "=" + shlex.quote(value) + "\n"
        for name, value in profile.VALIDATION_CONFIG.items()
    ))
    (directory / "checks.list").write_text(
        "\n".join(profile.VALIDATORS) + "\n"
    )
    if profile.SOURCE_GIT:
        copy_git_source(source, directory / "source",
                        profile.SOURCE_REPOSITORY, profile.SOURCE_COMMIT,
                        environment)
    else:
        shutil.copytree(source, directory / "source",
                        ignore=shutil.ignore_patterns(".git", "obj"))


def publish(bundle, work, output, name, metadata, description):
    git_metadata = bundle / "share/toolchain/validation/source/.git"
    git_notice = ""
    if git_metadata.is_dir():
        git_notice = (
            "Validation source includes independent Git history.\n"
            "Use host Git on a writable source copy for project workflows.\n"
            "SHA256SUMS excludes mutable source/.git administration files.\n"
        )
    (bundle / "DISTRIBUTION.json").write_text(
        json.dumps(metadata, indent=2) + "\n"
    )
    (bundle / "README.txt").write_text(
        description + "\n\n" + git_notice +
        "Add this directory's bin to PATH. Runtime libraries are private.\n"
        "Standalone checks: sh ./validate.sh [all|check-name] [output-dir]\n"
        "Available checks: share/toolchain/validation/checks.list\n"
        "Checks need only this distribution and standard Linux host tools.\n"
        "Host tools: sh, coreutils (including timeout), awk, sed, grep, ldd,\n"
        "tar, make and Perl; Linux /proc is needed for the JOS GDB socket.\n"
        "No Python, Git, uports workspace or separate lab checkout is needed.\n"
        "Host architecture and libc requirements: DISTRIBUTION.json.\n"
        "The host supplies libc and the ELF loader; neither is bundled.\n"
        "Other host machines and macOS have not been validated.\n"
        "Recipes, source references and licenses:\n"
        "  share/distribution/licenses\n"
        "Debian sources: apt-get source for the recorded package/version.\n"
    )
    checksums = []
    for path in sorted(bundle.rglob("*")):
        if path.is_relative_to(git_metadata):
            continue
        if path.is_file() and not path.is_symlink():
            digest = hashlib.sha256(path.read_bytes()).hexdigest()
            checksums.append(digest + "  " + str(path.relative_to(bundle)))
    (bundle / "SHA256SUMS").write_text("\n".join(checksums) + "\n")
    archive = work / (name + ".tar.xz")
    with tarfile.open(archive, "w:xz", dereference=False) as stream:
        stream.add(bundle, arcname=name)
    destination = output / name
    bundle.rename(destination)
    archive.rename(output / archive.name)
    (work / "checks").rename(output / (name + ".checks"))
    shutil.rmtree(work)
    print(f"  EXPORT  {destination}\n  ARCHIVE {destination}.tar.xz")
