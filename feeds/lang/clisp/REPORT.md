# CLISP uports Build Report

This note records how the CLISP port is built in uports, how that relates to
the upstream CLISP build flow, and how to test both staged and installed
artifacts.

## 1. Bootstrapping Status

CLISP does not require a preinstalled CLISP to build this port. The normal Unix
build is self-hosting after the initial C runtime is built:

Bootstrap classification: required self bootstrap. The bootstrap happens inside
the upstream CLISP build and does not need an external CLISP.

- the C compiler builds `lisp.run`;
- `lisp.run` loads CLISP sources from the tree;
- the build writes `lispinit.mem`;
- the generated runtime/image builds the final base and module layout.

Current uports does not need `BUILD_DEPENDS` for a CLISP host Lisp. It only needs
the declared libraries and normal build tools.

## 2. Build Flow

### Upstream CLISP Flow

CLISP does not require an existing host `clisp` compiler for a normal Unix
build. The official upstream flow is documented in `unix/INSTALL`:

```sh
./configure build-dir
cd build-dir
make
make check      # optional
make install
```

Internally, upstream `make` expands into the bootstrap sequence below:

```text
make init
make allc
make lisp.run
make interpreted.mem
make halfcompiled.mem
make lispinit.mem
make manual
make modular
make boot
make base
make full
```

The bootstrap is self-contained:

1. The C compiler builds `lisp.run`.
2. `lisp.run` loads CLISP's Lisp sources from the source tree.
3. CLISP writes `lispinit.mem`.
4. The newly built runtime/image compiles and links the bundled modules.

So the host needs normal build tools and libraries, not a preinstalled CLISP.

### uports Flow

The uports port supports both source modes:

- default SCM mode, using the upstream GitLab repository;
- tarball mode, using the BLFS `clisp-2.49.95.tar.xz` source tarball.

Upstream GNU/SourceForge still publishes the old stable `2.49` release tarball,
and the GitLab project provides repository archives/tags. The port keeps
`DISTVERSION=2.49.95` in both modes so `PKGNAME` includes a real
upstream-derived version instead of the earlier `clisp-g` git package name.

The expected distfile and package base are:

```text
clisp-2.49.95.tar.xz
clisp-2.49.95
```

To use the tarball instead of the default SCM checkout:

```sh
make USE_SCM= V=1 clisp.extract
```

The uports port keeps the official bootstrap model and wraps it in ports-style
targets:

```sh
make V=1 clisp.extract
make V=1 clisp.configure
make V=1 clisp.build
make V=1 clisp.install
```

The port configures CLISP with a separate build directory:

```sh
./configure $(CONFIGURE_ARGS) build
```

The important uports-specific configure choices are:

```text
--prefix=/usr
--mandir=/usr/share/man
--docdir=/usr/share/doc/clisp
--with-libffcall-prefix=$(DESTDIR)$(PREFIX)
--with-libreadline-prefix=$(DESTDIR)$(PREFIX)
--with-readline
--with-dynamic-ffi
--ignore-absence-of-libsigsegv
```

The port has a gettext switch:

```make
CLISP_WITH_GETTEXT ?= no
```

The default is `no`, which passes:

```text
--without-gettext
```

To enable gettext:

```sh
make V=1 CLISP_WITH_GETTEXT=yes clisp.configure
make V=1 CLISP_WITH_GETTEXT=yes clisp.build
```

When gettext is enabled, the port passes `--with-gettext`.

On non-Darwin hosts, the port also passes:

```text
--with-libiconv-prefix=$(DESTDIR)$(PREFIX)
```

`--without-gettext` is used for the current minimal uports build because the
gettext locale step hardlinks message catalogs through CLISP's internal
`locale` target and can fail in this environment. Disabling gettext removes
that locale target from the bootstrap dependency path.

If gettext is enabled, the port patches CLISP's generated build `Makefile`.
Upstream's build-time locale rule invokes:

```make
$(MAKE) install datarootdir=.. localedir='$$(datarootdir)/locale'
```

uports exports `DESTDIR=$(DESTDIR)` during builds. If that value leaks into the
build-time gettext install, CLISP creates a bad literal path like:

```text
local/compiler../locale
```

The port-side fix adds an empty `DESTDIR=` override to that generated locale
submake:

```make
$(MAKE) install datarootdir=.. localedir='$$(datarootdir)/locale' DESTDIR=
```

That keeps build-time locale files under CLISP's own build `locale` directory
instead of creating `local/compiler..`.

`--ignore-absence-of-libsigsegv` is used because libsigsegv is recommended by
upstream, but not currently required by this minimal uports package.

### Current Dependencies

The port declares:

```text
devel/libffcall
devel/readline
converters/libiconv    # non-Darwin only
```

`libffcall` provides dynamic FFI support. `readline` provides command-line
editing. `libiconv` is needed for the current Linux/uports prefix setup.

### gdbm Status

CLISP has an optional `modules/gdbm` module, but the current uports CLISP build
does not enable it. The current configure arguments do not include:

```text
--with-module=gdbm
```

The built runtime also does not link against `libgdbm`; `ldd` shows readline,
ffcall, iconv, and ncurses, but not gdbm. Therefore `databases/gdbm` is not a
required dependency for the current minimal CLISP package.

If the gdbm module is enabled later, then the dependency should be restored and
the configure arguments should include:

```text
--with-module=gdbm
--with-libgdbm-prefix=$(DESTDIR)$(PREFIX)
```

## 3. Built Image Layout And Relationships

The current staged package installs the user-facing commands:

```text
/usr/bin/clisp
/usr/bin/clisp-link
```

`/usr/bin/clisp` is a native launcher, not the full Lisp runtime. It finds and
execs the real runtime from the CLISP library directory. In the verified `/usr`
build, the launcher contains:

```text
/usr/lib/clisp-2.49.95+
base
lisp.run
lispinit.mem
```

The CLISP runtime tree is:

```text
/usr/lib/clisp-2.49.95+/
  base/
    lisp.run
    lispinit.mem
    lisp.a
    libgnu.a
    libnoreadline.a
    modules.h
    modules.o
    makevars
    calls.o
    gettext.o
    readline.o
    regexi.o
  build-aux/
    config.rpath
    depcomp
  data/
    Symbol-Table.text
    UnicodeDataFull.txt
  dynmod/
  linkkit/
    clisp.h
    modprep.lisp
    modules.c
```

The important runtime relationship is:

```text
/usr/bin/clisp
  -> /usr/lib/clisp-2.49.95+/base/lisp.run
       + /usr/lib/clisp-2.49.95+/base/lispinit.mem
       + /usr/lib/clisp-2.49.95+/base/modules.*
       + shared libraries from the package prefix/system
```

`lisp.run` is the native executable runtime. `lispinit.mem` is the memory image
containing the initialized Lisp system. The launcher normally supplies the
library directory, base directory, and memory image path so users can run just:

```sh
clisp
```

`clisp-link` is the module/linking helper. It expects a CLISP linking set, which
is the collection under `base/` containing:

```text
lisp.a
lisp.run
lispinit.mem
modules.h
modules.o
makevars
```

That linking set is used when creating or installing CLISP module sets.

The verified `lisp.run` shared-library dependencies are:

```text
libreadline.so.8
libtinfo.so.6
libm.so.6
libffcall.so.0
libiconv.so.2
libc.so.6
```

The current minimal package does not install extra dynamic modules under
`dynmod/`; the directory exists as the target location for dynamically linked
modules.

### Darwin Runtime Signing

On Darwin, `base/lisp.run` is a Mach-O executable. The port rewrites
length-preserving path strings in the staged `lisp.run` after install so the
runtime uses the final package prefix instead of the uports build root. That
post-link binary edit invalidates the linker-generated ad-hoc code signature.

If the signature is invalid, macOS can terminate the runtime before CLISP prints
an error:

```text
Killed: 9
```

Check this with:

```sh
codesign --verify --strict --verbose=2 \
  feeds/lang/clisp/work.compiler/stage/usr/local/lib/clisp-2.49.95+/base/lisp.run
```

The port has a Darwin `post-stage` step that re-signs the staged runtime after
the path rewrite and after the framework Darwin rpath fixup:

```sh
codesign --force --sign - \
  $(STAGEDIR)$(PREFIX)/lib/clisp-2.49.95+/base/lisp.run
```

### Darwin `logbitp` And Compiler Signatures

The Darwin/aarch64 build exposed a CLISP runtime/compiler bootstrap issue where
the primitive `logbitp` returned the wrong answer for positive fixnums. One
visible symptom was that compiled-function signature decoding lost `&key`
metadata, so compiler macros for functions such as `set-difference` could abort
with:

```text
(AND (= SYSTEM::OPT-NUM 0) (SYSTEM::MEMQ ':TEST SYSTEM::KEYWORDS)
     (SYSTEM::MEMQ ':TEST-NOT SYSTEM::KEYWORDS)) must evaluate to a non-NIL value.
```

This is not a SLIME-specific failure. A standalone CLISP compile was enough to
trigger it:

```lisp
(compile nil (lambda (a b) (set-difference a b)))
```

The port carries a source patch that redefines `cl:logbitp` in the Lisp image
using the bootstrap-safe `sys::%putd` function binder and working primitives:

```lisp
(logtest (ash 1 index) integer)
```

The same patch also makes the compiler's internal signature decoder use
`logand`/`ash` directly for bytecode header flags. That keeps the decoder from
depending on `logbitp` while the image is being bootstrapped.

After this patch, CLISP was confirmed to work with Emacs SLIME. The original
SLIME failure was reproduced as a standalone CLISP compiler failure, then
verified fixed by compiling SLIME's `xref.lisp` and by running SLIME normally
against the rebuilt installed CLISP.

## 4. How To Test The Built Image

### Test Inside Source Tree After Build

After:

```sh
make V=1 clisp.build
```

run the built executable directly from CLISP's build directory:

```sh
feeds/lang/clisp/work.compiler/clisp/build/clisp \
  -q -norc \
  -x '(+ 1 2)'
```

Expected output includes:

```text
3
```

For a stronger upstream-style check, run CLISP's own test target from the build
directory:

```sh
make -C feeds/lang/clisp/work.compiler/clisp/build check
```

Optional extended upstream checks are:

```sh
make -C feeds/lang/clisp/work.compiler/clisp/build check-recompile
make -C feeds/lang/clisp/work.compiler/clisp/build check-tests
make -C feeds/lang/clisp/work.compiler/clisp/build mod-check
```

These are slower and are better suited for a full validation run than for every
incremental port edit.

### Test Staged Install Tree

After:

```sh
make V=1 clisp.install
```

the staged files are under `feeds/lang/clisp/work.compiler/stage$(PREFIX)`.
For a `/usr` build that is:

```text
feeds/lang/clisp/work.compiler/stage/usr
```

The staged wrapper normally expects its installed library directory at
`/usr/lib/clisp-2.49.95+`. When testing from the stage tree, pass `-B` so CLISP
uses the staged library directory:

```sh
feeds/lang/clisp/work.compiler/stage/usr/bin/clisp \
  -q -norc \
  -B feeds/lang/clisp/work.compiler/stage/usr/lib/clisp-2.49.95+ \
  -x '(+ 2 5)'
```

Expected output:

```text
7
```

Without `-B`, a staged binary may print a warning because `/usr/lib/clisp-*`
does not exist in the real host root. That is expected for an uninstalled stage
tree and does not indicate a failed build.

On Darwin, also verify that the staged `lisp.run` signature is valid before
runtime testing. An invalid signature can show up as `Killed: 9` with no CLISP
diagnostic.

For a `/usr/local` Darwin build installed under the local compiler root, the
same test shape is:

```sh
cd local/compiler/usr/local/bin
./clisp -B ../lib/clisp-2.49.95+ -q -norc -x '(+ 1 2)'
```

Also test the compiler path that SLIME uses:

```sh
./clisp -B ../lib/clisp-2.49.95+ -q -norc \
  -x '(progn
        (compile nil (lambda (a b) (set-difference a b)))
        (ext:exit))'
```

Expected result: no compiler assertion.

For the reported SLIME `xref.lisp` failure, this direct compile test should
also complete without warnings or errors:

```sh
./clisp -B ../lib/clisp-2.49.95+ -q -norc \
  -x '(progn
        (compile-file "/Users/lyc/.emacs-gnu.d/elpa/slime-20250918.2258/xref.lisp"
                      :output-file "/private/tmp/xref-clisp-test.fas")
        (ext:exit))'
```

After this direct compiler test passes, start SLIME from Emacs with this CLISP
binary. A successful session should load SWANK/SLIME without the previous
`set-difference` compiler assertion.

### Test Package Creation

The default package repository path is:

```text
make/uports/packages
```

In this checkout it is a symlink to:

```text
/opt/distfiles/ports/packages
```

If `/opt` is mounted read-only, `clisp.install` can stage and create the
workdir package, but fail when copying the package into the repository. To test
package creation on a writable path:

```sh
make V=1 PACKAGES=/tmp/uports-packages clisp.install
```

The workdir package is created at:

```text
feeds/lang/clisp/work.compiler/pkg/clisp-g.pkg
```

### Test After Package Installation

After installing the package into an actual target root where `/usr/bin/clisp`
and `/usr/lib/clisp-2.49.95+` are present, test without the staged `-B` override:

```sh
/usr/bin/clisp -q -norc -x '(+ 20 22)'
```

Expected output:

```text
42
```

Also check that the installed binary can load the base image and modules:

```sh
/usr/bin/clisp -q -norc -x '(progn (format t "~A~%" (lisp-implementation-version)) (ext:exit))'
```

This should print the CLISP version and exit successfully.

## 5. Installing Outside `/usr`

CLISP is prefix-sensitive. Do not build with `PREFIX=/usr` and then move the
installed tree to `/usr/local`. The installed launcher embeds the CLISP library
directory, and `lisp.run` can also contain a build-time RUNPATH for dependent
libraries.

For the current `/usr` build, the staged launcher contains:

```text
/usr/lib/clisp-2.49.95+
```

and the runtime layout is:

```text
/usr/bin/clisp
/usr/lib/clisp-2.49.95+/base/lisp.run
/usr/lib/clisp-2.49.95+/base/lispinit.mem
```

If the target install prefix is `/usr/local`, build with:

```sh
PREFIX=/usr/local
```

The expected installed layout becomes:

```text
/usr/local/bin/clisp
/usr/local/bin/clisp-link
/usr/local/lib/clisp-2.49.95+/base/lisp.run
/usr/local/lib/clisp-2.49.95+/base/lispinit.mem
/usr/local/lib/clisp-2.49.95+/dynmod/
/usr/local/lib/clisp-2.49.95+/linkkit/
/usr/local/share/doc/clisp/
/usr/local/share/man/man1/clisp.1
```

The launcher should then contain:

```text
/usr/local/lib/clisp-2.49.95+
```

Check it with:

```sh
strings /usr/local/bin/clisp | rg 'clisp-2.49.95|lisp.run|lispinit.mem'
```

Also check the runtime RUNPATH:

```sh
readelf -d /usr/local/lib/clisp-2.49.95+/base/lisp.run | rg 'RUNPATH|RPATH'
```

For a clean `/usr/local` package, the RUNPATH should point at the final runtime
library prefix, not at the uports build root:

```text
/usr/local/lib
```

The current `/usr` stage is expected to show:

```text
Library runpath: [/usr/lib]
```

The Makefile has a `post-install` cleanup for this. It rewrites staged
`base/makevars` and length-preserving strings in `base/lisp.run` so package
runtime paths refer to `$(PREFIX)/include` and `$(PREFIX)/lib`, not
`$(DESTDIR)$(PREFIX)/include` or `$(DESTDIR)$(PREFIX)/lib`.

When testing the staged tree on a host where the uports libraries are not
actually installed in `/usr/lib`, use `LD_LIBRARY_PATH` only for the local test:

```sh
LD_LIBRARY_PATH=/home/lyc/pj/builds/uports.example/local/compiler/usr/lib \
  feeds/lang/clisp/work.compiler/stage/usr/bin/clisp \
  -q -norc \
  -B feeds/lang/clisp/work.compiler/stage/usr/lib/clisp-2.49.95+ \
  -x '(+ 2 5)'
```

That local override is not needed after installing into a target root where
`/usr/lib` or `/usr/local/lib` contains the package dependencies.
