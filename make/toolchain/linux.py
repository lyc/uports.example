"""Linux ELF runtime closure, relocation and native library notices."""

import hashlib
import os
from pathlib import Path
import platform
import re
import shutil
import struct

from common import run
from packages import package_paths


# These belong to the documented host libc baseline, never to the bundle.
HOST_LIBRARIES = re.compile(
    r"^(lib(c|m|pthread|dl|rt|resolv|util|anl)\.so\.[0-9]+|"
    r"ld-linux[^/]*\.so\.[0-9]+)$"
)


def host_elf(path):
    if not path.is_file() or path.is_symlink():
        return False
    with path.open("rb") as stream:
        header = stream.read(20)
    if len(header) != 20 or header[:4] != b"\x7fELF":
        return False
    endian = "<" if header[5] == 1 else ">"
    kind, machine = struct.unpack(endian + "HH", header[16:20])
    expected = {"x86_64": 62, "aarch64": 183}[platform.machine()]
    return kind in (2, 3) and machine == expected


def dependencies(path, environment):
    output = run(["ldd", path], environment)
    if "not found" in output:
        raise RuntimeError(f"Unresolved runtime dependency: {path}\n{output}")
    result = {}
    for line in output.splitlines():
        match = re.match(r"\s*(\S+) => (/.*?) \(0x", line)
        if match:
            result[match[1]] = Path(match[2])
    return result


def host_binaries(bundle, host_paths):
    # CPU identity alone cannot distinguish native tools from target sysroots.
    binaries = []
    for relative in (*host_paths, "lib/runtime"):
        root = bundle / relative
        paths = root.rglob("*") if root.is_dir() else (root,)
        for path in paths:
            if host_elf(path) and path not in binaries:
                binaries.append(path)
    return binaries


def bundle_libraries(bundle, native_prefixes, host_paths, environment):
    runtime = bundle / "lib/runtime"
    runtime.mkdir(parents=True)
    resolver = environment.copy()
    # Prefer native deployment contexts over host libraries.
    roots = native_prefixes
    resolver["LD_LIBRARY_PATH"] = ":".join(
        str(root / directory)
        for root in roots for directory in ("lib", "lib64")
    )
    pending = host_binaries(bundle, host_paths)
    copied = {}
    for binary in pending:
        for name, source in dependencies(binary, resolver).items():
            if HOST_LIBRARIES.fullmatch(name):
                continue
            digest = hashlib.sha256(source.read_bytes()).hexdigest()
            if name in copied:
                if copied[name][1] != digest:
                    raise RuntimeError(f"Conflicting runtime library: {name}")
                continue
            target = runtime / name
            shutil.copy2(source, target)
            copied[name] = (source.resolve(), digest)
            pending.append(target)
    return copied


def relocate(bundle, host_paths, patchelf, environment):
    runtime = bundle / "lib/runtime"
    binaries = host_binaries(bundle, host_paths)
    for path in binaries:
        section = run(["readelf", "-d", path], environment)
        if "(NEEDED)" not in section:
            continue
        for name in re.findall(r"\(NEEDED\).*\[([^]]+)\]", section):
            if "/" in name:
                raise RuntimeError(
                    f"Absolute runtime dependency in {path}: {name}"
                )
        relative = os.path.relpath(runtime, path.parent)
        search = "$ORIGIN" if relative == "." else "$ORIGIN/" + relative
        run([patchelf, "--set-rpath", search, path], environment)
    return binaries


def runtime_notices(libraries, packages, notices, environment):
    records = []
    owners = {}
    for package in packages:
        for path, relative in package_paths(package):
            if path.is_file():
                owners[path.resolve()] = package
    used = {}
    for name, (source, digest) in libraries.items():
        owner = owners.get(source)
        if owner:
            used[owner["name"]] = owner
            record = {key: owner[key] for key in ("name", "origin", "version")}
        else:
            result = run(["dpkg-query", "-S", source], environment)
            result = result.splitlines()[0]
            package = result.rsplit(": ", 1)[0]
            version = run(["dpkg-query", "-W", "-f=${Version}", package],
                          environment).strip()
            copyright_file = Path("/usr/share/doc") / package.split(":")[0]
            copyright_file /= "copyright"
            destination = notices / package.replace(":", "-")
            destination.mkdir(exist_ok=True)
            shutil.copy2(copyright_file, destination / "copyright")
            record = {"name": package, "version": version, "origin": "Debian"}
        records.append({"library": name, "original_sha256": digest, **record})
    return records, list(used.values())


def terminal_data(bundle, native_prefixes, notices):
    candidates = [root / "share/terminfo" for root in native_prefixes]
    candidates += [Path("/usr/share/terminfo"), Path("/lib/terminfo")]
    target = bundle / "share/terminfo"
    target.mkdir(parents=True)
    for directory in candidates:
        if directory.is_dir():
            for source in directory.rglob("*"):
                if source.is_file():
                    destination = target / source.relative_to(directory)
                    destination.parent.mkdir(parents=True, exist_ok=True)
                    if not destination.exists():
                        shutil.copy2(source, destination)
    if not any(target.rglob("xterm*")):
        raise RuntimeError("Missing xterm terminal database for export")
    term_license = notices / "terminal-data"
    term_license.mkdir()
    shutil.copy2("/usr/share/doc/ncurses-base/copyright",
                 term_license / "copyright")


def glibc_versions(binaries, environment):
    versions = set()
    for path in binaries:
        output = run(["readelf", "--version-info", path], environment)
        versions.update(re.findall(r"\bGLIBC_[0-9.]+", output))
    return sorted(versions)
