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
WITH_GETTEXT ?= no
```

On Darwin, the port forces:

```make
override WITH_GETTEXT = no
```

The default no-gettext mode passes:

```text
--without-gettext
--disable-nls
```

On non-Darwin hosts, gettext can be enabled with:

```sh
make V=1 WITH_GETTEXT=yes clisp.configure
make V=1 WITH_GETTEXT=yes clisp.build
```

When gettext is enabled on a non-Darwin host, the port passes `--with-gettext`.

On non-Darwin hosts, the port also passes:

```text
--with-libiconv-prefix=$(DESTDIR)$(PREFIX)
```

`--without-gettext` is used for the current minimal uports build because the
gettext locale step hardlinks message catalogs through CLISP's internal
`locale` target and can fail in this environment. Disabling gettext removes
that locale target from the bootstrap dependency path.

`--disable-nls` is also required for no-gettext builds on macOS. CLISP's
`--without-gettext` adds `-DNO_GETTEXT` for CLISP's own message layer, but the
bundled gnulib regex code still follows `ENABLE_NLS` and `HAVE_LIBINTL_H`. On
Intel macOS with gettext headers in `/usr/local`, configure can set
`ENABLE_NLS=1`; then `libgnu.a` references `_libintl_dgettext`, while the final
`clisp-link add boot base ...` command does not add `-lintl` because gettext was
disabled. The resulting link failure looks like:

```text
Undefined symbols for architecture x86_64:
  "_libintl_dgettext", referenced from:
      _rpl_re_compile_pattern in libgnu.a
      _rpl_regerror in libgnu.a
```

Keeping `--disable-nls` with `--without-gettext` prevents gnulib from compiling
those gettext calls and keeps the default package independent of host gettext.

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

The port also has a documentation switch:

```make
WITH_DOC ?= yes
```

The default `WITH_DOC=yes` leaves upstream documentation targets enabled and
uses the normal `pkg-plist.*` manifests that include `share/doc/clisp`. On
Linux, complete documentation output needs `groff` for PostScript files and
`ps2pdf` from Ubuntu's `ghostscript` package for PDF files. If `ps2pdf` is
installed after CLISP was already configured, rerun `clisp.configure` so the
generated build `Makefile` records the PDF tool.

CLISP's generated `Makefile` installs the short manuals twice when the full
documentation tools are available: `install-man` writes `clisp.{html,pdf,ps}`
and `clisp-link.{html,pdf,ps}` to `share/doc/clisp`, while `install-doc` writes
the same generated files to `share/doc/clisp/doc`. The port removes the
top-level copies in `post-install` and keeps the `doc/` copies so the Linux
plist is deterministic across hosts with different documentation tool sets.

To build a smaller runtime-focused package, set:

```sh
make V=1 WITH_DOC=no clisp.build
make V=1 WITH_DOC=no clisp.install
```

With `WITH_DOC=no`, the package keeps the CLISP runtime, module link kit,
editor support files, and manpages, but omits `share/doc/clisp`. In this mode
the port patches CLISP's generated build `Makefile` after configure so:

```text
all
```

does not depend on the upstream `manual` target, and:

```text
install
```

runs `install-bin install-man` instead of `install-bin install-man install-doc`.
The `install-man` target is also narrowed to the two installed manpage files, so
it does not require HTML or PostScript documentation as build prerequisites.
As a fallback for reused work trees that were configured before this switch was
added, `post-install` also removes `share/doc/clisp` when `WITH_DOC=no`.

The no-doc package manifest is selected with:

```make
PLIST_NAME = pkg-plist-nodoc
```

### Current Dependencies

The port declares:

```text
devel/libffcall
devel/readline
devel/ncurses       # when readline is enabled
converters/libiconv    # non-Darwin only
```

`libffcall` provides dynamic FFI support. `readline` provides command-line
editing. `ncurses` is listed explicitly because the generated configure probes
link and run against `libreadline`; on Darwin, `libreadline.8.2.dylib` loads
`@rpath/libncurses.6.dylib`, so missing ncurses makes probes such as
`intparam.h` generation abort at runtime. `libiconv` is needed for the current
Linux/uports prefix setup.

### Optional Module Configuration

CLISP does not have an SBCL-style contrib blocklist that automatically enables
or disables installed modules from CPU feature probes. SBCL can decide whether
to install contribs such as `sb-perf` or `sb-simd` from operating-system and
processor checks. CLISP's comparable extension surface is instead the upstream
external module system and is mostly controlled by configure arguments.

The upstream build has this default base module set:

```text
i18n
syscalls
regexp
```

`readline` is special: it is added to the base linking set when readline support
is enabled and found. Extra modules are only added when configure receives
explicit module requests such as:

```text
--with-module=pcre
--with-module=gdbm
--with-module=clx/new-clx
```

In this CLISP 2.49.95 source tree there are 24 top-level directories under
`modules`:

```text
asdf
berkeley-db
bindings
clx
dbus
dirkey
editor
fastcgi
gdbm
gtk2
i18n
libsvm
matlab
netica
oracle
pari
pcre
postgresql
queens
rawsock
readline
regexp
syscalls
zlib
```

Counting beyond the base modules and `readline`, this leaves 20 extra module
directories:

```text
asdf
berkeley-db
bindings
clx
dbus
dirkey
editor
fastcgi
gdbm
gtk2
libsvm
matlab
netica
oracle
pari
pcre
postgresql
queens
rawsock
zlib
```

The configure-backed external module set is smaller. The source tree has 18
module `configure` scripts:

```text
berkeley-db
clx/new-clx
dbus
dirkey
fastcgi
gdbm
gtk2
i18n
libsvm
oracle
pari
pcre
postgresql
rawsock
readline
regexp
syscalls
zlib
```

After removing the base modules and `readline`, there are 14 configure-backed
optional modules:

```text
berkeley-db
clx/new-clx
dbus
dirkey
fastcgi
gdbm
gtk2
libsvm
oracle
pari
pcre
postgresql
rawsock
zlib
```

The current uports CLISP package enables none of those optional modules because
the port does not pass any `--with-module=...` arguments. The installed module
layout should therefore be stable across Linux and Darwin except for normal
platform differences such as shared library handling, paths, and documentation
tool availability. If optional modules are enabled later, expose them as
explicit port flags, for example `WITH_GDBM`, `WITH_PCRE`, or `WITH_ZLIB`, and
update dependencies and package manifests for the selected module set.

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

## 3. Port Patches

The CLISP port currently carries three source patches:

```text
files/0001-fix-logbitp-and-signature-decoding.patch
files/0002-use-c-compatible-lisp-function-typedef.patch
files/0003-disable-impnotes-network-check.patch
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

`0001-fix-logbitp-and-signature-decoding.patch` redefines `cl:logbitp` in the
Lisp image using the bootstrap-safe `sys::%putd` function binder and working
primitives:

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

### ARM64 C Function Typedef

The Linux/ARM64 build failed while compiling `spvw.o` with:

```text
../src/lispbibl.d:9327:37: error: ISO C requires a named argument before '...'
  typedef Values (*lisp_function_t)(...);
```

That declaration uses an ellipsis-only prototype. Plain C does not accept this
form because a variadic function prototype must have at least one named
argument before `...`.

`0002-use-c-compatible-lisp-function-typedef.patch` changes the typedef and the
generated header emitter from:

```c
Values (*lisp_function_t)(...)
```

to:

```c
Values (*lisp_function_t)()
```

This keeps the old CLISP meaning of a C function pointer with unspecified
arguments while avoiding the invalid ellipsis-only C syntax. After this patch,
`make V=1 clisp.build` completed on the ARM64 machine, and the built image
started successfully with:

```sh
feeds/lang/clisp/work.compiler/clisp-2.49.95/build/clisp \
  -K full -q -x '(+ 1 2)'
```

Expected output:

```text
3
```

### Impnotes Network Check

When CLISP is built from an SCM checkout, upstream `src/makemake.in` treats the
presence of `.git` as a developer build and tries to fetch:

```text
http://www.gnu.org/software/clisp/impnotes/id-href.map
```

That fetch is only a freshness check for the upstream implementation notes map;
it is not a normal package fetch input. In this uports environment it also goes
through the host `/usr/local/sbin/wget` buildinfo wrapper, which can fail before
CLISP's own fallback logic completes.

`0003-disable-impnotes-network-check.patch` disables that developer-only
network check so `WITH_DOC=yes` builds are reproducible from the checked-out
source tree and bundled documentation files.

## 4. Built Image Layout And Relationships

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

## 5. How To Test The Built Image

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

## 6. Installing Outside `/usr`

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

The `lisp.run` rewrite intentionally avoids Perl. The port uses a small Bash
helper that finds exact byte offsets with `grep -aboF`, then writes the shorter
runtime path plus NUL padding with `dd conv=notrunc`. This preserves the binary
size and only changes the embedded path strings.

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
