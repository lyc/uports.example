# Gambit 4.9.7 port report

## Sources

The recipe supports both upstream source forms and defaults to Git:

```make
USE_SCM ?= git
```

The Git mode checks out `https://github.com/gambit/gambit` and detaches at
tag `v4.9.7`, commit
`24b444492277f1f11246e0ce80797ff14ec0d954`.  The official release tarball is
also supported by setting `USE_SCM=` explicitly.  It is fetched from:

```text
https://gambitscheme.org/4.9.7/gambit-v4_9_7.tgz
```

The recorded archive metadata is:

```text
SHA256 (gambit/gambit-v4_9_7.tgz) = 5243d3d5cc3a82e718601680b12f482594d2914b1a892b8a4c8012bb9d343c22
SIZE (gambit/gambit-v4_9_7.tgz) = 16716857
```

The framework creates a synthetic Git commit after extracting a tarball.
Gambit's version stamp is derived from `git describe` and that commit's date,
so an unadjusted tarball build would expose the extraction date as its runtime
version.  The tarball post-patch hook normalizes the synthetic commit to the
upstream release epoch and tags it `v4.9.7`.  Both source modes consequently
report:

```text
v4.9.7 20250713110245
```

The project wrapper may not preserve an explicitly empty variable in a
convenience target.  To select the tarball reliably, invoke the port directly:

```sh
ROOT=$PWD
make -C feeds/lang/gambit stage USE_SCM= \
  PORTSDIR="$ROOT/make/uports" \
  DESTDIR="$ROOT/local/compiler" \
  PREFIX=/usr/local \
  USE_ALTERNATIVE=yes \
  ALTERNATIVE_WRKDIR="$ROOT/local/compiler/src" \
  TYPE_SUFFIX=.compiler
```

## Bootstrap and build flow

Gambit is a self-bootstrap from generated C, with no external Scheme
implementation required.  Both upstream source forms include pregenerated C
for the runtime, interpreter, and compiler.  The `gsc-boot.unix` file is
intentionally a no-op bootstrap script: it prevents regeneration of those C
files before a working `gsc` exists.  The host C compiler first builds the
runtime, `gsi`, and `gsc`; that new `gsc` then builds the bundled C and
JavaScript modules.

The Git checkout can regenerate `configure` when Autotools input timestamps
make it appear stale.  The release tarball normally uses its shipped
release-generated `configure`.  This source-mode distinction is worth checking
when Autoconf changes, even though both modes currently produce the same
installed manifest and runtime version.

The recipe does not enable Gambit's `--enable-single-host` option.  With Clang,
that option embeds the complete runtime into each dynamic module and made a
single `_irregex` module compile take more than nine minutes while many modules
were still pending.  The normal multi-object build retains the complete module
set and builds each module in seconds.  Clang warns that `-fmodulo-sched` is
unsupported; the warning is non-fatal.

## Installed layout

The principal installed relationships are:

```text
bin/gsi-gambit                  interpreter
bin/gsc-gambit                  compiler
bin/gsi-script, bin/gsc-script script frontends
bin/scheme-*                    Scheme-standard frontends
bin/six, bin/six-script         SIX language frontends
include/gambit                  embedding/runtime headers
lib/gambit/libgambit*.a         static runtime/compiler archives
lib/gambit                      Scheme, C, JavaScript, and compiled modules
share/info, share/man, doc      documentation
```

The validated build installs static Gambit archives rather than a Gambit
shared library.  On Darwin, `gsi-gambit`, `gsc-gambit`, and a generated native
program depend only on `/usr/lib/libSystem.B.dylib`.

## Runtime verification

Gambit records the final prefix in its runtime configuration.  A basic staged
interpreter test does not need a module path:

```sh
ROOT=$PWD/feeds/lang/gambit/work.compiler/stage/usr/local
"$ROOT/bin/gsi-gambit" \
  -e '(begin (display (+ 20 22)) (newline) (exit))'
```

This prints `42`.  To verify a staged module, override Gambit's final library
directory with the `~~lib` runtime option:

```sh
"$ROOT/bin/gsi-gambit" -:~~lib="$ROOT/lib/gambit" \
  -e '(begin
        (import (srfi 1))
        (write (fold + 0 (list 10 20 12)))
        (newline)
        (exit))'
```

This also prints `42`.  A real compiler and generated-program test is:

```sh
printf '(display (* 6 7))\n(newline)\n' > /tmp/gambit-port-test.scm

"$ROOT/bin/gsc-gambit" \
  -:~~bin="$ROOT/bin",~~lib="$ROOT/lib/gambit",~~include="$ROOT/include/gambit" \
  -exe -o /tmp/gambit-port-test /tmp/gambit-port-test.scm

/tmp/gambit-port-test
```

The compiled program prints `42`.  When Gambit is installed at its configured
final prefix, `/usr/local`, the `~~bin`, `~~lib`, and `~~include` staging
overrides are unnecessary:

```sh
/usr/local/bin/gsi-gambit \
  -e '(begin (import (srfi 1)) (write (fold + 0 (list 10 20 12))) (newline))'
```

No `GUILE_*_PATH`, `DYLD_LIBRARY_PATH`, or equivalent Guile settings apply to
Gambit.

## Platform validation

The validated platform is arm64 Darwin.  Clean Git and official-tarball builds
completed configure, build, stage, and package.  Interpreter arithmetic, SRFI
module loading, native compilation, and the generated executable all passed
for both source modes.  The Darwin plist contains 1,431 entries and exactly
matches a fresh tarball-mode `generate-plist`.  The stage contains 1,022 files
or symlinks and no embedded workspace, `DESTDIR`, or work-tree paths were
found.

Linux has not yet been validated.  Generate its plist from a real Linux staged
install, then inspect ELF dependencies and any RPATH/RUNPATH entries and repeat
the interpreter, module-loading, and compiler tests.  Do not derive a Linux
plist from the Darwin output.
