# chibicc 20201207 port report

## Source modes

chibicc does not publish releases or tags.  Upstream also states that its
history may be force-pushed because each commit corresponds to a section of
the compiler book.  The recipe therefore identifies this snapshot by its
upstream commit date and pins both source modes to:

```text
90d1f7f199cc55b13c7fdb5839d1409806633fdb
2020-12-07T11:50:43+09:00
```

Git is the default:

```make
USE_SCM ?= git
```

It clones `https://github.com/rui314/chibicc.git` through the framework cache
and detaches at the pinned commit.  `SCM_RETRIEVE_TIMEOUT=300` gives network
retrieval bounded failure behavior when the host provides `timeout`.

`USE_GITHUB=yes`, `GH_ACCOUNT=rui314` and the pinned `GH_TAGNAME` describe
the archive source. Set `SOURCE_MODE=dist` to select its tarball, or
`SOURCE_MODE=git` to select the explicit `SCM_REPO_URL`:

```text
https://codeload.github.com/rui314/chibicc/tar.gz/90d1f7f199cc55b13c7fdb5839d1409806633fdb
```

The pre-migration archive metadata is retained below for reference. The new
framework names the archive
`chibicc/rui314-chibicc-90d1f7f199cc55b13c7fdb5839d1409806633fdb_GH0.tar.gz`.
Its checksum must be recorded with `makesum` and validated before acceptance;
the previous checksum has not been assumed valid for the new archive.

```text
SHA256 (chibicc/90d1f7f199cc55b13c7fdb5839d1409806633fdb.tar.gz) = cadb44e82354002000f6d2bbf41d3155f47a9b3e9abd50a309fdcd9b5ef791e0
SIZE (chibicc/90d1f7f199cc55b13c7fdb5839d1409806633fdb.tar.gz) = 83814
```

Select the tarball explicitly through the source-mode interface:

```sh
ROOT=$PWD
make -C feeds/lang/chibicc stage SOURCE_MODE=dist \
  PORTSDIR="$ROOT/make/uports" \
  DESTDIR="$ROOT/local/compiler" \
  PREFIX=/usr/local \
  USE_ALTERNATIVE=yes \
  ALTERNATIVE_WRKDIR="$ROOT/local/compiler/src" \
  TYPE_SUFFIX=.compiler
```

## Build and bootstrap classification

There is no runtime or compiler bootstrap requirement.  The host C compiler
builds chibicc directly from its nine C translation units.  Upstream's
`stage2/chibicc` target is a self-hosting verification target, not a prerequisite
for the installed compiler.

The port overrides upstream's debug-oriented `CFLAGS` at the build rule.
`-O2` avoids embedding local source paths through `-g`, while retaining the
upstream C11, common-symbol, and warning options.

One source patch uses C99 `PRIu64` and `PRId64` format macros.  Upstream uses
`%lu` and `%ld` for fixed-width types, but those types have different underlying
typedefs on Darwin and Linux.  The patch removes undefined variadic format
mismatches without changing the generated values.

## Installed layout

Upstream assumes its executable and private headers remain next to each other
in a source checkout.  The package preserves that relationship under
`libexec` and installs a relocatable frontend:

```text
bin/chibicc
  -> shell frontend that derives PREFIX from its own location
  -> libexec/chibicc/chibicc
     -> libexec/chibicc/include/*.h

share/doc/chibicc/LICENSE
share/doc/chibicc/README.md
```

The frontend is required instead of a simple symlink.  chibicc reinvokes its
original command path for the internal `-cc1` phase and derives its private
include directory relative to that path.  The frontend always executes the
real binary from `libexec`, so staged, packaged, and final-prefix installations
all find the seven bundled C headers without environment variables.

## Darwin verification

The validated host is arm64 Darwin.  Both Git and tarball modes completed
build, stage, manifest generation, and package.  The 18-entry Darwin plist
exactly matches fresh output from both modes.  The installed compiler has only
`/usr/lib/libSystem.B.dylib` as a Mach-O dependency, and no workspace,
`DESTDIR`, or work-tree paths were found in the staged package.

chibicc's backend emits x86-64 System V ELF assembly, and its driver hardcodes
the GNU x86-64 Linux assembler, linker, startup objects, dynamic loader, and
library paths.  Consequently, a Darwin build can validate the frontend and
code generator with `-S`, but it cannot assemble, link, or execute the result
natively.

The staged compiler was tested through its installed frontend, including PATH
lookup and private-header loading:

```sh
ROOT=$PWD/feeds/lang/chibicc/work.compiler/stage/usr/local

cat >/tmp/chibicc-port-test.c <<'EOF'
#include <stdbool.h>
#include <stddef.h>

struct pair {
  long left;
  long right;
};

static long sum(struct pair value) {
  return value.left + value.right;
}

int main(void) {
  struct pair value = {20, 22};
  bool correct = sum(value) == 42;
  return correct ? 0 : 1;
}
EOF

PATH="$ROOT/bin:$PATH" \
  chibicc -S -o /tmp/chibicc-port-test.s /tmp/chibicc-port-test.c
```

The output contains an x86-64 `main` function and the expected constant `42`.
Git and tarball mode produced byte-identical assembly for this test.

## Required Linux verification

Full runtime validation requires x86-64 Linux, the platform targeted upstream.
After a native staged build, generate `pkg-plist.linux` from that stage rather
than copying the Darwin plist.  Then test the complete driver:

```sh
ROOT=$PWD/feeds/lang/chibicc/work.compiler/stage/usr/local

PATH="$ROOT/bin:$PATH" \
  chibicc -o /tmp/chibicc-port-test /tmp/chibicc-port-test.c

/tmp/chibicc-port-test
echo $?
```

The exit status must be zero.  Also run upstream's ordinary and self-hosted
test targets from the patched work source:

```sh
make test
make test-stage2
```

Finally, inspect the installed compiler with `readelf` and `ldd`, verify there
is no workspace or staged-root RPATH/RUNPATH, and repeat the leak scan used on
Darwin.  The current environment had no running Linux container or x86-64
Linux host, so these Linux-only checks remain explicitly unverified.
