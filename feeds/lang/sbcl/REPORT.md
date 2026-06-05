# SBCL Build and Bootstrap Report

Date: 2026-05-21

## Summary

The `lang/sbcl` port now builds SBCL `2.5.7` from the upstream SourceForge git tag `sbcl-2.5.7`.

The successful package build used an existing Homebrew SBCL as the cross-compilation host. The resulting uports-installed SBCL reports version `2.5.7`.

An additional CLISP bootstrap experiment was run with the locally built CLISP. That test confirmed that the port must pass the host Lisp through `make.sh --xc-host=...`, but the locally built CLISP `2.49.95+` failed as an SBCL bootstrap host.

## Port Changes

The SBCL port uses upstream's bootstrap scripts directly:

```make
SBCL_XC_HOST ?= sbcl --no-userinit --no-sysinit
SBCL_BUILD_ARGS ?= --prefix=$(PREFIX)
SBCL_INSTALL_ARGS ?= --prefix=$(PREFIX)

do-build:
	@(cd $(WRKSRC) && \
	    $(SETENV) $(MAKE_ENV) $(SH) make.sh $(SBCL_BUILD_ARGS) \
	        --xc-host="$(SBCL_XC_HOST)")

do-install:
	@(cd $(WRKSRC) && \
	    $(SETENV) $(MAKE_ENV) BUILD_ROOT=$(STAGEDIR) \
	        $(SH) install.sh $(SBCL_INSTALL_ARGS))
```

Important details:

- `SCM_DETACH = $(PORTNAME)-$(DISTVERSION)` pins git extraction to `sbcl-2.5.7`.
- `SBCL_XC_HOST` is passed as `--xc-host=...`; setting only an environment variable is not sufficient because SBCL's `make-config.sh` resets the default host unless the command-line option is used.
- `PKG_ENV` is adjusted to use the generated plist, because the port does not provide a static `pkg-plist`.

## 1. Bootstrapping Status

SBCL requires an existing Common Lisp implementation as the cross-compilation
host. The recommended host for this port is another SBCL:

```make
SBCL_XC_HOST ?= sbcl --no-userinit --no-sysinit
```

Bootstrap classification: required external bootstrap, provided by an existing
host Common Lisp implementation. SBCL is the recommended host.

The Homebrew SBCL host test succeeded. A local CLISP bootstrap experiment failed,
so CLISP should be treated as experimental for this SBCL version and platform.

Current uports does not automatically execute `BUILD_DEPENDS`, so the bootstrap
host must already be available in `PATH`, or `SBCL_XC_HOST` must be set to an
explicit executable path.

The tree also provides `lang/sbcl-bootstrap`, a binary bootstrap package that
installs a private `sbcl-bootstrap` command. To use it, build and install that
package first, then build this port:

```sh
make USE_GLOBALBASE=yes compiler@sbcl-bootstrap.install
make USE_GLOBALBASE=yes compiler@sbcl.build
```

If `$(DESTDIR)$(PREFIX)/bin/sbcl-bootstrap` exists, the port automatically uses:

```make
SBCL_XC_HOST ?= $(DESTDIR)$(PREFIX)/bin/sbcl-bootstrap --no-userinit --no-sysinit
```

`WITH_SBCL_BOOTSTRAP=yes` can still be used in direct port builds to force that
selection and to express the intended `BUILD_DEPENDS`.

## 2. Build Flow With Bootstrap

SBCL is not built by a normal `make all` flow. Upstream expects:

1. A host Common Lisp implementation.
2. `make.sh` to build the cross-compiler and target runtime.
3. `install.sh` to install the runtime, core, contribs, docs, and manpage.

For this port, the default bootstrap host is:

```sh
sbcl --no-userinit --no-sysinit
```

During the successful test, this resolved to Homebrew SBCL:

```text
SBCL 2.6.4
```

The final installed uports SBCL is independent of the host version and reports:

```text
2.5.7
```

Current uports does not automatically execute `BUILD_DEPENDS` yet, so the host
SBCL used by `SBCL_XC_HOST` must already be available in the host `PATH`, or
`SBCL_XC_HOST` must be set to an explicit executable path. The port currently
defaults to:

```make
SBCL_XC_HOST ?= sbcl --no-userinit --no-sysinit
```

The Makefile passes that host explicitly to upstream:

```make
$(SH) make.sh $(SBCL_BUILD_ARGS) --xc-host="$(SBCL_XC_HOST)"
```

This is important. Setting only the environment variable is not enough because
SBCL's build scripts can reset the default host unless `--xc-host=...` is
present on the `make.sh` command line.

SCM retrieval is also relevant for this package. The SBCL port defaults to SCM
mode and pins the checkout to:

```make
SCM_DETACH = sbcl-2.5.7
```

The framework now wraps network git retrieve operations with:

```make
SCM_RETRIEVE_TIMEOUT ?= 10s
```

This prevents a stuck SourceForge git mirror operation from hanging the build
indefinitely. The timeout can be overridden:

```sh
make SCM_RETRIEVE_TIMEOUT=30s fetch
make SCM_RETRIEVE_TIMEOUT=0 fetch
```

## 3. Built Image Layout And Relationships

The current staged package installs:

```text
/usr/bin/sbcl
/usr/lib/sbcl/sbcl.core
/usr/lib/sbcl/sbcl.mk
/usr/lib/sbcl/contrib/*.asd
/usr/lib/sbcl/contrib/*.fasl
/usr/share/doc/sbcl/
/usr/share/man/man1/sbcl.1
```

`/usr/bin/sbcl` is the native runtime executable. It is not a shell wrapper.
The main saved Lisp image is:

```text
/usr/lib/sbcl/sbcl.core
```

The normal runtime relationship is:

```text
/usr/bin/sbcl
  -> finds SBCL_HOME
  -> loads /usr/lib/sbcl/sbcl.core
  -> loads contrib ASDF/FASL files from /usr/lib/sbcl/contrib/
```

The staged runtime contains an embedded relative fallback home:

```text
../lib/sbcl
```

That means an installed `/usr/bin/sbcl` finds `/usr/lib/sbcl`, and an installed
`/usr/local/bin/sbcl` should find `/usr/local/lib/sbcl`, as long as `bin` and
`lib/sbcl` keep the same relative layout.

`SBCL_HOME` overrides the default home search. It is the correct way to test a
staged tree without installing it into the host root.

The runtime shared-library dependencies are minimal:

```text
libm.so.6
libc.so.6
```

Contrib systems such as `sb-gmp` and `sb-mpfr` may load external libraries at
runtime. Those are why the port declares:

```make
LIB_DEPENDS = libgmp.so:math/gmp libmpfr.so:math/mpfr
```

### `sb-perf` Contrib

Linux package manifests include:

```text
/usr/lib/sbcl/contrib/sb-perf.asd
/usr/lib/sbcl/contrib/sb-perf.fasl
```

`sb-perf` is an upstream SBCL contrib for Linux `perf` integration. It provides
functions such as:

```lisp
(sb-perf:write-perfmap)
(sb-perf:write-jitdump)
```

These write Linux perf symbol files such as:

```text
/tmp/perf-<pid>.map
/tmp/jit-<pid>.dump
```

so tools like `perf record`, `perf report`, and flamegraph workflows can show
SBCL Lisp function names instead of only raw addresses or `[unknown]` frames.

The Darwin package manifest intentionally omits `sb-perf`. macOS does not have
Linux `perf`, and the contrib is written around Linux perf map/jitdump behavior.
Even if some parts compile on Darwin, it does not provide the corresponding
integration for Apple Instruments, `sample`, or dtrace. Therefore the current
platform split is expected:

```text
Linux:  include sb-perf
Darwin: omit sb-perf
```

### `sb-simd` Contrib

The Linux x86_64 package manifest includes:

```text
/usr/lib/sbcl/contrib/sb-simd.asd
/usr/lib/sbcl/contrib/sb-simd.fasl
```

`sb-simd` is an upstream SBCL contrib that provides a convenient SIMD interface.
It defines packages and operations for x86 SIMD instruction sets such as SSE,
SSE2, SSE3, SSSE3, SSE4.1, SSE4.2, AVX, AVX2, and FMA.

SBCL decides whether to build `sb-simd` during `make-config.sh`, before
contribs are built. Upstream first blocklists it on every non-x86-64 target:

```sh
case "$sbcl_arch" in
    x86-64) ;; *) SBCL_CONTRIB_BLOCKLIST="$SBCL_CONTRIB_BLOCKLIST sb-simd" ;;
esac
```

For x86-64 targets, SBCL enables the SIMD target features and then builds and
runs `tools-for-build/avx2`:

```c
int main () {
  __builtin_cpu_init();
  return __builtin_cpu_supports("avx2") != 0;
}
```

The probe exits successfully only when the build CPU supports AVX2. If the
probe cannot be built or exits nonzero, `make-config.sh` adds `sb-simd` to
`SBCL_CONTRIB_BLOCKLIST`.

Expected package behavior:

```text
x86_64 Linux with AVX2:       include sb-simd
x86_64 macOS with AVX2:       likely include sb-simd
x86_64 without AVX2:          omit sb-simd
ARM64 Linux:                  omit sb-simd
Apple Silicon macOS:          omit sb-simd
```

The current Darwin plist was produced from an Apple Silicon build, so it omits
`sb-simd`. If uports later packages Intel macOS from an AVX2-capable x86_64
host, the Darwin manifest may need an x86_64-specific variant that includes
`sb-simd.asd` and `sb-simd.fasl`.

The installed `sbcl.mk` is used when rebuilding/linking runtime-related pieces.
In the current staged build it still records build-root include/lib paths:

```text
CFLAGS=-I/home/lyc/pj/builds/uports.example/local/compiler/usr/include ...
LDFLAGS=-L/home/lyc/pj/builds/uports.example/local/compiler/usr/lib
```

That does not affect normal `sbcl` startup, but it is not prefix-clean metadata
for development/rebuild use. If `sbcl.mk` matters for downstream consumers, the
port should add a `post-install` cleanup similar to the CLISP/gforth path
normalization.

## Successful Build Procedure

Fetch and extract using the shared SCM cache:

```sh
make USE_GLOBALBASE=yes SCMDIR_SITE=/opt/distfiles/ports/scm V=1 sbcl.extract
```

The source checkout was verified as:

```sh
git -C feeds/lang/sbcl/work.compiler/sbcl describe --tags --always --dirty
```

Result:

```text
sbcl-2.5.7
```

Build:

```sh
make USE_GLOBALBASE=yes SCMDIR_SITE=/opt/distfiles/ports/scm V=1 sbcl.build
```

Install/package:

```sh
make USE_GLOBALBASE=yes SCMDIR_SITE=/opt/distfiles/ports/scm V=1 sbcl.install
```

Generated package:

```text
make/uports/packages/All-darwin-aarch64/sbcl-2.5.7,1.pkg
```

## 4. How To Test The Built Image

### Test Staged Install Tree

After staging, test by pointing `SBCL_HOME` at the staged library directory:

```sh
SBCL_HOME=/home/lyc/pj/builds/uports.example/feeds/lang/sbcl/work.compiler/stage/usr/lib/sbcl \
  /home/lyc/pj/builds/uports.example/feeds/lang/sbcl/work.compiler/stage/usr/bin/sbcl \
  --noinform --non-interactive \
  --eval '(format t "~A~%" (lisp-implementation-version))'
```

Expected output:

```text
2.5.7
```

ASDF/contrib smoke test from stage:

```sh
SBCL_HOME=/home/lyc/pj/builds/uports.example/feeds/lang/sbcl/work.compiler/stage/usr/lib/sbcl \
  /home/lyc/pj/builds/uports.example/feeds/lang/sbcl/work.compiler/stage/usr/bin/sbcl \
  --noinform --non-interactive \
  --eval '(require :asdf)' \
  --eval '(format t "asdf-ok~%")'
```

Expected output:

```text
asdf-ok
```

Without `SBCL_HOME`, the staged binary may search relative to its staged
`bin/sbcl` path or the host install layout. Explicit `SBCL_HOME` is the most
reliable staged test.

### Test Installed Package

Installed binary version test:

```sh
local/compiler/usr/bin/sbcl --noinform --non-interactive \
  --eval '(format t "~A~%" (lisp-implementation-version))'
```

Expected result:

```text
2.5.7
```

ASDF/contrib smoke test:

```sh
local/compiler/usr/bin/sbcl --noinform --non-interactive \
  --eval '(require :asdf)' \
  --eval '(format t "asdf-ok~%")'
```

Expected result:

```text
asdf-ok
```

Optional contrib tests after GMP/MPFR are installed:

```sh
local/compiler/usr/bin/sbcl --noinform --non-interactive \
  --eval '(require :sb-gmp)' \
  --eval '(format t "sb-gmp-ok~%")'

local/compiler/usr/bin/sbcl --noinform --non-interactive \
  --eval '(require :sb-mpfr)' \
  --eval '(format t "sb-mpfr-ok~%")'
```

These require the package runtime to be able to find `libgmp` and `libmpfr`.

## 5. Installing Outside `/usr`

SBCL is less prefix-sensitive than gforth or CLISP because the runtime contains
a relative fallback home:

```text
../lib/sbcl
```

If built and installed with:

```sh
PREFIX=/usr/local
```

the expected layout is:

```text
/usr/local/bin/sbcl
/usr/local/lib/sbcl/sbcl.core
/usr/local/lib/sbcl/sbcl.mk
/usr/local/lib/sbcl/contrib/
/usr/local/share/doc/sbcl/
/usr/local/share/man/man1/sbcl.1
```

In that layout, `/usr/local/bin/sbcl` should find:

```text
/usr/local/lib/sbcl/sbcl.core
```

because `../lib/sbcl` from `/usr/local/bin` resolves to `/usr/local/lib/sbcl`.

Recommended `/usr/local` verification:

```sh
/usr/local/bin/sbcl --noinform --non-interactive \
  --eval '(format t "~A~%" (lisp-implementation-version))'

/usr/local/bin/sbcl --noinform --non-interactive \
  --eval '(require :asdf)' \
  --eval '(format t "asdf-ok~%")'
```

For staged `/usr/local` testing, use:

```sh
SBCL_HOME=/path/to/stage/usr/local/lib/sbcl \
  /path/to/stage/usr/local/bin/sbcl \
  --noinform --non-interactive \
  --eval '(format t "~A~%" (lisp-implementation-version))'
```

Do not build with `PREFIX=/usr` and then manually move the installed tree to
`/usr/local`. The runtime may still find `../lib/sbcl` if the relative layout is
preserved, but package metadata and `sbcl.mk` will not describe the actual
installation prefix. Build with the final runtime `PREFIX` instead.

## 6. Other Notes: CLISP Bootstrap Experiment

The locally built CLISP was checked:

```sh
local/compiler/src/clisp/build/clisp --version
```

Result:

```text
GNU CLISP 2.49.95+ (2024-11-03)
```

The CLISP-host SBCL build was tested in a separate work directory to avoid disturbing the successful SBCL package:

```sh
make -C feeds/lang/sbcl --no-print-directory \
  USE_GLOBALBASE=yes \
  SCMDIR=/opt/distfiles/ports/scm \
  PORTSDIR=/Volumes/pj/builds/uports.example/make/uports \
  PREFIX=/usr \
  DESTDIR=/Volumes/pj/builds/uports.example/local/compiler \
  TYPE_SUFFIX=.clispboot \
  SBCL_XC_HOST='/Volumes/pj/builds/uports.example/local/compiler/src/clisp/build/clisp -q -norc' \
  build
```

The corrected port passed CLISP to upstream as:

```text
--xc-host='/Volumes/pj/builds/uports.example/local/compiler/src/clisp/build/clisp -q -norc'
```

CLISP was then used, but the SBCL bootstrap failed early while loading `src/cold/shared.lisp`:

```text
*** - (AND (= SYSTEM::OPT-NUM 0) (SYSTEM::MEMQ ':TEST SYSTEM::KEYWORDS) (SYSTEM::MEMQ ':TEST-NOT SYSTEM::KEYWORDS)) must evaluate to a non-NIL value.
GNUmakefile:41: genesis/Makefile.features: No such file or directory
```

The CLISP bootstrap was retried after the local CLISP compiler fixes and after
installing CLISP as `/usr/local/bin/clisp`. The SBCL build reached
`make-host-2.sh`, then the host CLISP process crashed during the cross-compiler
run:

```text
[ 34/301] src/code/type-class
make-host-2.sh: line 59: ... Segmentation fault: 11 | $SBCL_XC_HOST
```

The failure reproduces outside the uports wrapper from the SBCL source tree:

```sh
cd local/compiler/src/sbcl
echo '(load "loader.lisp") (load-sbcl-file "make-host-2.lisp")' | clisp -q -norc
```

One run crashed at `src/code/type-class`; after retrying from the partially
generated tree, the same command passed that file but later crashed at:

```text
[161/301] src/compiler/sxhash
Segmentation fault: 11 | clisp -q -norc
```

The macOS crash report for `lisp.run` shows the fault inside CLISP itself while
hashing, not in a shell wrapper:

```text
exception: EXC_BAD_ACCESS / SIGSEGV
top frames:
  hashcode_bvector
  hashcode3
  hash_lookup_builtin
  C_gethash
  interpret_bytecode_
```

This means `SBCL_XC_HOST=clisp` is not a reliable bootstrap host for this SBCL
version/platform. The failure is reproducible with a direct CLISP invocation and
is not fixed by `-q -norc`.

Conclusion:

- Homebrew SBCL bootstrap works.
- Local CLISP `2.49.95+` does not currently work as an SBCL `2.5.7` bootstrap host.
- SBCL upstream documentation says CLISP compatibility is version-sensitive: `2.44.1` is noted as OK, while `2.47` is noted as not OK.
- Trying CLISP `2.44.1` on macOS arm64 failed earlier during CLISP's own build because the old CLISP source does not know the current CPU/ABI.

## 7. Other Notes: Dependency Notes

The SBCL build produced contrib warnings for GMP/MPFR:

```text
WARNING: GMP not loaded.
WARNING: MPFR was not loaded. This is likely because the shared library was not found.
```

At the time of testing, `libgmp` and `libmpfr` were not installed under `local/compiler/usr/lib`, even though the port declares:

```make
LIB_DEPENDS = libgmp.so:math/gmp libmpfr.so:math/mpfr
```

The base SBCL runtime and ASDF smoke test still passed. To fully validate `sb-gmp` and `sb-mpfr`, install/build the GMP and MPFR dependencies first, then rebuild SBCL and test requiring those contribs.

## 8. Current Recommendation

Use SBCL as the bootstrap host for the normal `lang/sbcl` package build.

Keep `SBCL_XC_HOST` configurable so future tests can use other host Lisps, but treat CLISP bootstrap as experimental on macOS arm64 unless a known-good CLISP build host is available.
