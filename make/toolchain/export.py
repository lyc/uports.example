#!/usr/bin/env python3
"""Assemble installed toolchains and run their profile acceptance checks."""

import argparse
import datetime
from pathlib import Path
import platform
import shutil
import subprocess
import sys
import tempfile

from common import check_source, clean_environment, run_live
import linux
import packages
from profiles import jos


# Add a profile only when its component recipes and acceptance are implemented.
PROFILES = {jos.TUPLE: jos}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--profile", choices=PROFILES, required=True)
    for name in ("project", "destdir", "output"):
        parser.add_argument("--" + name, type=Path, required=True)
    parser.add_argument("--validation-source", type=Path)
    parser.add_argument("--patchelf", default="patchelf")
    parser.add_argument("--check", action="store_true",
                        help="prepare/check host/source inputs before install")
    args = parser.parse_args()
    profile = PROFILES[args.profile]
    if (platform.system() != "Linux" or
            platform.machine() not in ("x86_64", "aarch64")):
        raise RuntimeError("Toolchain export supports Linux ELF hosts only")
    environment = clean_environment()
    patchelf = shutil.which(args.patchelf)
    for tool in ("readelf", "ldd", "make", "git", "dpkg-query", "timeout"):
        if not shutil.which(tool, path=environment["PATH"]):
            raise RuntimeError(f"Export needs host tool: {tool}")
    project = args.project.resolve()
    destdir = args.destdir.resolve()
    source = args.validation_source
    if source is None:
        source = profile.default_source(project, destdir, environment)
    check_source(source, profile.SOURCE_COMMIT, profile.SOURCE_FILES,
                 environment)
    source_metadata = profile.SOURCE_METADATA
    if args.check:
        return
    if not patchelf:
        raise RuntimeError("Export needs patchelf; install host:patchelf first")
    output = args.output.resolve()
    if output.is_relative_to(destdir):
        raise RuntimeError("EXPORT_DIR must be outside the development install")
    output.mkdir(parents=True, exist_ok=True)
    stamp = datetime.datetime.now(datetime.timezone.utc)
    stamp = stamp.strftime("%Y%m%dT%H%M%S%fZ")
    name = f"{profile.TUPLE}-linux-{platform.machine()}-{stamp}"
    work = Path(tempfile.mkdtemp(prefix=".toolchain-export-", dir=output))
    print(f"  EXPORT  staging and logs: {work}", flush=True)
    bundle = work / profile.TUPLE
    bundle.mkdir()
    try:
        inventory = packages.installed_packages(destdir, profile.DATABASE_ROOTS)
        selected = packages.copy_components(
            inventory, bundle, profile.COMPONENTS, profile.PREFIX
        )
        native_prefixes = [destdir / p for p in profile.NATIVE_PREFIXES]
        libraries = linux.bundle_libraries(
            bundle, native_prefixes, profile.HOST_PATHS, environment
        )
        notices = bundle / "share/distribution/licenses"
        notices.mkdir(parents=True)
        runtime, owners = linux.runtime_notices(
            libraries, inventory, notices, environment
        )
        for package in {p["name"]: p for p in selected + owners}.values():
            packages.source_notices(project, package, notices)
        description = profile.DESCRIPTION
        if profile.TERMINAL_DATA:
            linux.terminal_data(bundle, list(reversed(native_prefixes)),
                                notices)
            description += (
                "\nSet TERMINFO to this profile's share/terminfo for terminals."
            )
        binaries = linux.relocate(bundle, profile.HOST_PATHS, patchelf,
                                  environment)
        packages.install_validation(bundle, source.resolve(), profile,
                                    environment)
        validation_dir = bundle / "share/toolchain/validation"
        (validation_dir / "host-elf.list").write_text("".join(
            str(path.relative_to(bundle)) + "\n" for path in binaries
        ))
        relocated = work / "relocated/profile"
        relocated.parent.mkdir()
        bundle.rename(relocated)
        binaries = [relocated / p.relative_to(bundle) for p in binaries]
        checks = work / "checks"
        checks.mkdir()
        validation_log = work / "validation.log"
        run_live(["sh", relocated / "validate.sh", "all", checks],
                 environment, validation_log)
        validation_log.rename(checks / "validation.log")
        metadata = {
            "host_os": "Linux", "host_arch": platform.machine(),
            "tested_libc": platform.libc_ver(), **source_metadata,
            "components": [{k: p[k] for k in ("name", "version", "origin")}
                           for p in selected], "runtime_libraries": runtime,
            "validation": profile.VALIDATION,
            "required_glibc_symbol_versions": linux.glibc_versions(
                binaries, environment
            ),
        }
        packages.publish(relocated, work, output, name, metadata, description)
    except Exception:
        print(f"Export failed; development install untouched; evidence: {work}",
              file=sys.stderr)
        raise


if __name__ == "__main__":
    try:
        main()
    except (RuntimeError, OSError, subprocess.TimeoutExpired) as error:
        sys.exit(str(error))
