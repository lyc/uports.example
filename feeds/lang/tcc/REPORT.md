# TinyCC 0.9.28rc.20260714 port report

## Sources

The recipe uses the TinyCC mob snapshot at:

```text
d9d02c56401e43be43760b63f7d82f771a7ed1f6
2026-07-14T16:38:29+03:00
```

This snapshot identifies itself as `0.9.28rc`.  The port appends the commit
date to make the moving development version packageable.  The last formal
release, 0.9.27, predates usable Darwin arm64 support: it identifies arm64 as
32-bit ARM and cannot produce a native arm64 Mach-O executable.  The pinned
mob snapshot has the current arm64 backend and Mach-O writer.

Both source modes use the public `TinyCC/tinycc` GitHub mirror of the
repo.or.cz upstream repository and resolve to that exact commit.  Git is the
default:

```make
USE_SCM ?= git
```

It clones `https://github.com/TinyCC/tinycc.git` through the framework cache
and detaches at the pinned commit.  Retrieval has a 300-second bound.  Set
`SOURCE_MODE=dist` to select GitHub's archive of the same commit, or
`SOURCE_MODE=git` to explicitly select Git. The archive is described by
`USE_GITHUB=yes`, `GH_ACCOUNT=TinyCC` and the pinned `GH_TAGNAME`:

```text
https://codeload.github.com/TinyCC/tinycc/tar.gz/d9d02c56401e43be43760b63f7d82f771a7ed1f6
```

The pre-migration archive metadata is retained below for reference. The new
framework names the archive
`tinycc/TinyCC-tinycc-d9d02c56401e43be43760b63f7d82f771a7ed1f6_GH0.tar.gz`.
Record its checksum with `makesum` and validate it before acceptance; the
previous checksum has not been assumed valid for the new archive.

```text
SHA256 (tinycc/d9d02c56401e43be43760b63f7d82f771a7ed1f6.tar.gz) = 5cf1779bd3d5994491fc821277169bda76e41d46f11d8b82d8e66add0f5a2e55
SIZE (tinycc/d9d02c56401e43be43760b63f7d82f771a7ed1f6.tar.gz) = 1034789
```

The framework creates a synthetic Git commit around extracted archives.
TinyCC normally derives its displayed identity from the current Git commit,
so the recipe supplies the pinned identity to `make`.  Both modes report:

```text
tcc version 0.9.28rc 2026-07-14 HEAD@d9d02c56 (AArch64 Darwin)
```

Select and build the archive mode directly with:

```sh
ROOT=$PWD
make -C feeds/lang/tcc stage SOURCE_MODE=dist \
  PORTSDIR="$ROOT/make/uports" \
  DESTDIR="$ROOT/local/compiler" \
  PREFIX=/usr/local
```

## Bootstrap and build flow

Bootstrap classification: none.  The host C compiler builds `tcc` and
`libtcc.a` directly.  That newly built `tcc` then builds `libtcc1.a` and the
runtime objects used for `-run`, backtraces, and bounds checking.  No existing
TinyCC installation or other language runtime is required.

`ZERO_AR_DATE=1` makes the host-generated `libtcc.a` deterministic on Darwin.
The Git and archive builds produced identical hashes for `tcc`, `libtcc.a`,
`libtcc1.a`, and `bcheck.o`.

Upstream compiles `bcheck.o` with `-bt`, which normally embeds its absolute
build directory as DWARF metadata.  The port patch lets this one rule record
`.` as its compilation directory.  This removes the workspace leak while
retaining bcheck's own symbols and source-level backtraces.  The complete
upstream test suite, including backtrace and bounds-check output comparisons,
passes with the patch.

## Installed layout

The important installed relationships are:

```text
bin/tcc                         compiler, linker, assembler, and -run frontend
include/libtcc.h                embedding API
lib/libtcc.a                    static embedding library
lib/tcc/libtcc1.a               target runtime support
lib/tcc/*.o                     -run, backtrace, and bounds-check support
lib/tcc/include/*.h             TinyCC private system headers
share/man, share/info, share/doc
```

TinyCC records the final `PREFIX` as its private library location.  When
testing a staged tree, pass `-B"$ROOT/lib/tcc"` so it uses staged headers and
runtime objects.  A final installation at its configured prefix does not need
`-B`.  Embedded users should call `tcc_set_lib_path()` with the staged
`lib/tcc` directory during staged testing.

## Runtime and compiler verification

The validated host is arm64 Darwin.  Both Git and archive modes completed a
clean configure, build, stage, exact plist check, and runtime tests.  Package
generation was verified from the final default-Git build.  The installed
compiler and runtime objects contain no workspace, work-tree, staged-root, or
`DESTDIR` paths.

A real staged `-run` test is:

```sh
ROOT=$PWD/feeds/lang/tcc/work/stage/usr/local

cat >/tmp/tinycc-port-test.c <<'EOF'
#include <stdio.h>
int main(void) {
  printf("%d\n", 6 * 7);
  return 0;
}
EOF

"$ROOT/bin/tcc" -B"$ROOT/lib/tcc" -run /tmp/tinycc-port-test.c
```

It prints `42`.  Native compilation is tested with:

```sh
"$ROOT/bin/tcc" -B"$ROOT/lib/tcc" \
  -o /tmp/tinycc-port-test /tmp/tinycc-port-test.c
/tmp/tinycc-port-test
file /tmp/tinycc-port-test
otool -L /tmp/tinycc-port-test
```

The program prints `42`, is an arm64 Mach-O executable, and depends only on
`/usr/lib/libSystem.B.dylib`.  The `libtcc` API was also compiled against
`include/libtcc.h` and `lib/libtcc.a`, used to compile an in-memory function,
relocated with `tcc_relocate`, and called successfully with result `42`.

Bounds checking was verified separately by compiling an out-of-bounds array
write with `-b -run`.  It exits 255 and reports both the bounds violation and
the source line.  Upstream `make test` ends with `ALL TESTS PASSED`.

## Platform status

The Darwin plist was generated from the actual arm64 Darwin stage.  TinyCC's
intermediate object format is ELF even when the final Darwin executable is
Mach-O; this is expected internal behavior of the Mach-O linker.

Linux has not yet been validated.  Generate `pkg-plist.linux` from a native
Linux stage rather than copying the Darwin plist.  Repeat `-run`, native
compilation, bounds checking, the `libtcc` embedding test, and upstream
`make test`; then inspect ELF dependencies and RPATH/RUNPATH and repeat the
path-leak scan.
