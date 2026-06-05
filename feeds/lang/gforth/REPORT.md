# Gforth Port Build Report

This note records the current `lang/gforth` package design and the behavior
verified with `gforth-0.7.9_20260610` built from tarball mode.

## 1. Bootstrapping Status

`lang/gforth` requires a host gforth for bootstrapping. The package should use
the local `lang/gforth-0.7.3` bootstrap package, not an arbitrary `gforth` from
the host `PATH`.

Bootstrap classification: required external bootstrap, provided by the separate
`lang/gforth-0.7.3` bootstrap package.

Current uports does not execute `BUILD_DEPENDS` automatically yet, so the
practical status is:

- build and install `lang/gforth-0.7.3` manually first;
- build `lang/gforth` second;
- keep `BUILD_DEPENDS` in the Makefile to document the intended framework
  relationship.

## 2. Build Flow With Bootstrap

`lang/gforth` is built with a separate bootstrap package:

- `lang/gforth-0.7.3` now produces package base `gforth-bootstrap`.
- The bootstrap package installs versioned commands only, such as
  `gforth-0.7.3`, `gforth-fast-0.7.3`, `gforth-itc-0.7.3`, `gforthmi-0.7.3`,
  and `vmgen-0.7.3`.
- It does not install public unversioned commands like `gforth`, `gforth-fast`,
  `gforthmi`, or `vmgen`.
- It is intended only as the host gforth used while building the current
  `lang/gforth` package.

The current `lang/gforth` Makefile records the intended bootstrap dependency:

```make
BUILD_DEPENDS += gforth-bootstrap>=0.7.3:lang/gforth-0.7.3
```

Current uports does not execute `BUILD_DEPENDS` automatically yet. Therefore,
the practical build order is manual:

```sh
cd feeds/lang/gforth-0.7.3
make V=1 package install-package \
  PORTSDIR=/home/lyc/pj/builds/uports.example/make/uports \
  DESTDIR=/home/lyc/pj/builds/uports.example/local/compiler \
  PREFIX=/usr

cd ../gforth
make V=1 package install-package \
  PORTSDIR=/home/lyc/pj/builds/uports.example/make/uports \
  USE_SCM= \
  DESTDIR=/home/lyc/pj/builds/uports.example/local/compiler \
  PREFIX=/usr
```

The dependency line should still stay in the Makefile because it documents the
correct relationship and gives the framework a hook once automatic dependency
handling is implemented.

During configure, the current package forces the bootstrap host explicitly:

```make
GFORTH_BOOT_PATH = $(DESTDIR)$(PREFIX)/share/gforth/0.7.3:$(DESTDIR)$(PREFIX)/lib/gforth/0.7.3
GFORTH_BOOT_BIN  = $(DESTDIR)$(PREFIX)/bin/gforth-0.7.3

CONFIGURE_ENV += GFORTHPATH=$(GFORTH_BOOT_PATH) \
                 GFORTH=$(GFORTH_BOOT_BIN)
```

The verified configure result was:

```text
checking for gforth... "/home/lyc/pj/builds/uports.example/local/compiler/usr/bin/gforth-0.7.3" -i "/home/lyc/pj/builds/uports.example/local/compiler/usr/lib/gforth/0.7.3/gforth.fi"
```

So after the bootstrap package has been installed manually, the current package
does not use a random `gforth` from `PATH`; it uses the versioned
`gforth-0.7.3` from the bootstrap package.

The source mode can be either SCM or tarball. The default SCM mode is pinned
with `SCM_DETACH` because Gforth embeds `PACKAGE_VERSION` in installed command
names and library/share paths. A moving Git checkout can therefore produce
different package manifests while `DISTVERSION` stays unchanged.

For `DISTVERSION=0.7.9_20260610`, the pinned Git commit is:

```make
SCM_DETACH = be0316636de19aed535564874b807be68ec47a11
```

That commit reports:

```text
AC_INIT([gforth],[0.7.9_20260610],...)
```

In tarball mode, the distfile is always named `gforth.tar.xz`, so the Makefile
uses:

```make
DISTFILES   = $(PORTNAME).tar.xz
DIST_SUBDIR = $(PORTNAME)-$(DISTVERSIONFULL)
```

This avoids collisions between different snapshot versions that reuse the same
upstream filename.

In SCM mode, the git destination is also the versioned dist name:

```make
SCM_DEST = $(DISTNAME)
```

For `0.7.9_20260610`, both source modes should produce the same work-source
directory name:

```text
tarball top directory: gforth-0.7.9_20260610/
git checkout path:     $(WRKDIR)/gforth-0.7.9_20260610
```

Do not identify the pinned git source with `git describe --dirty` after
patch/build steps. The checked-out commit itself is clean; any `-dirty` suffix
comes from local patching or generated build files in `WRKSRC`.

## 3. Built Image Layout And Relationships

For the verified amd64 build, the staged command layout is:

```text
/usr/bin/gforth                         -> gforth-0.7.9_20260610-amd64
/usr/bin/gforth-0.7.9_20260610          -> gforth-0.7.9_20260610-amd64
/usr/bin/gforth-amd64                   -> gforth-0.7.9_20260610-amd64
/usr/bin/gforth-0.7.9_20260610-amd64

/usr/bin/gforth-fast                    -> gforth-fast-0.7.9_20260610-amd64
/usr/bin/gforth-fast-0.7.9_20260610     -> gforth-fast-0.7.9_20260610-amd64
/usr/bin/gforth-fast-amd64              -> gforth-fast-0.7.9_20260610-amd64
/usr/bin/gforth-fast-0.7.9_20260610-amd64

/usr/bin/gforth-itc                     -> gforth-itc-0.7.9_20260610-amd64
/usr/bin/gforth-itc-0.7.9_20260610      -> gforth-itc-0.7.9_20260610-amd64
/usr/bin/gforth-itc-amd64               -> gforth-itc-0.7.9_20260610-amd64
/usr/bin/gforth-itc-0.7.9_20260610-amd64

/usr/bin/gforth-ditc                    -> gforth-ditc-0.7.9_20260610-amd64
/usr/bin/gforth-ditc-0.7.9_20260610     -> gforth-ditc-0.7.9_20260610-amd64
/usr/bin/gforth-ditc-amd64              -> gforth-ditc-0.7.9_20260610-amd64
/usr/bin/gforth-ditc-0.7.9_20260610-amd64

/usr/bin/gforthmi                       -> gforthmi-0.7.9_20260610-amd64
/usr/bin/gforthmi-0.7.9_20260610        -> gforthmi-0.7.9_20260610-amd64
/usr/bin/gforthmi-amd64                 -> gforthmi-0.7.9_20260610-amd64
/usr/bin/gforthmi-0.7.9_20260610-amd64

/usr/bin/vmgen                          -> vmgen-0.7.9_20260610
/usr/bin/vmgen-0.7.9_20260610
```

The installed image files are:

```text
/usr/lib/gforth/0.7.9_20260610/gforth.fi
/usr/lib/gforth/0.7.9_20260610/kernl64l.fi
```

Relationship:

- `kernl64l.fi` is the low-level kernel image for amd64 little-endian.
- `gforth.fi` is the full saved system image used by normal gforth startup.
- `gforth-*-amd64` binaries are engine executables.
- The user-facing `gforth` symlink resolves to the versioned amd64 engine.
- `gforthmi` creates or rebuilds saved images.
- `vmgen` is the VM generator tool installed with gforth.

During the build, gforth may also generate temporary/cross images such as
`kernl32l.fi`, `kernl32b.fi`, `kernl64l.fi`, `kernl64b.fi`, and intermediate
`gforth-light.fi` inside the work tree. For the amd64 package, only the runtime
images needed by the installed system are staged.

Runtime C-library binding files are installed under:

```text
/usr/lib/gforth/0.7.9_20260610/amd64/libcc-named/
```

Notable generated bindings include:

```text
libgflibc.*
libgfpthread.*
libgflibffi.*
libgfcstr.*
libgfsocket.*
libgfmmap.*
libgfserial.*
libgffilestat.*
libgftime.*
```

Dependency notes:

- `libgflibffi.so` links to `libffi.so`.
- `libffcall` is optional and controlled by `WITH_LIBFFCALL`.
  The default is `WITH_LIBFFCALL=no`, so configure is forced with
  `ac_cv_lib_avcall___builtin_avcall=no` and the installed package does not
  include `libgffflib.*`.
- With `WITH_LIBFFCALL=yes`, the port adds
  `libavcall.so:devel/libffcall`, lets configure enable `FFCALLFLAG=true`, and
  builds the optional `unix/fflib.fs` wrapper as `libgffflib.*`.
- `pkg-plist.linux` records the default package layout
  (`WITH_LIBFFCALL=no`). A `WITH_LIBFFCALL=yes` package currently needs either
  a regenerated plist that includes `libgffflib.*` or future uports conditional
  plist support.
- `libltdl` is required for gforth's libtool-style dynamic loading path.
- `swig` is optional. Without swig, configure warns and skips optional generated
  bindings, but the core package builds and runs.

### Documentation Formats

Documentation generation is controlled by `WITH_DOCUMENT`, which defaults to
`no`. With the default setting, the port disables documentation formats that
require TeX or HTML tooling. Upstream gforth does not provide dedicated
`--disable-html` or `--disable-tex` configure switches, so the port forces the
relevant configure tool probes to their "not found" fallback:

```make
WITH_DOCUMENT=no
TEXI2DVI="echo texi2dvi disabled"
TEX="echo tex disabled"
TEXI2HTML="echo texi2html disabled"
texinfo_link=false
```

This makes configure leave `INSTALLPDF` and `INSTALLHTML` empty even on hosts
where TeX or `texi2html` are installed. The port also patches upstream
`Makefile.in` so `install-info`, `install-seq`, and `install-images` do not pull
in the broad `doc` or `install-txt` targets. Otherwise `make install` can still
generate and install `doc/gforth.txt` on hosts with `makeinfo`, and an explicit
`make doc` can pull HTML, PS, and PDF back in through the `doc` aggregate target.
The `ps` target needs both TeX-generated `.dvi` files and `dvips`, so it is
controlled by `WITH_DOCUMENT` together with HTML, PDF, info, and text output.

With `WITH_DOCUMENT=no`, generated `gforth.txt`, info, HTML, PDF, and PS
documentation are not installed. A `post-install` cleanup also removes
`share/gforth/<version>/doc/gforth.txt` so reused staged trees stay
deterministic.

With `WITH_DOCUMENT=yes`, the port leaves upstream document handling enabled:
configure probes the host tools normally, and `install-info` keeps its upstream
dependency on the broad `doc` target. In that mode installed documentation
depends on which TeX, Texinfo, and HTML tools are available on the build host.

### X11 SWIG Bindings

X11 SWIG bindings are disabled by default:

```make
WITH_X11=no
```

With this default, configure is forced to skip the X11, Xrandr, GLX, and VA/X11
header probes. This prevents upstream from adding `x.fs`, `xrandr.fs`,
`glx.fs`, `va_glx.fs`, or `va_x11.fs` to `unix/Makefile` on hosts where stale
or partial X11 headers would otherwise make `SWIG-GEN x.fs` fail.

### Emacs Lisp Support

The port keeps upstream gforth's normal Emacs handling. If configure finds an
Emacs binary, upstream enables `gforth.elc`, byte-compiles `gforth.el`, and
installs both `gforth.el` and `gforth.elc`.

The discovered `emacs` must work in batch mode:

```sh
emacs --batch --no-site-file -f batch-byte-compile gforth.el
```

On this host the original `/usr/local/bin/emacs` wrapper always added GUI
options such as `--background-color` and `-geometry`, and those options broke
`--batch` execution before `gforth.elc` could be generated. The wrapper was fixed
outside the port to skip GUI-only options for `--batch`, so the port does not
need an Emacs-specific workaround.

### macOS Compiler

The current gforth port requires GNU GCC on macOS/Darwin. The system `gcc` and
`g++` commands provided by Xcode are Apple Clang frontends and are not treated as
GNU GCC for this port.

On Darwin, when `CC` and `CXX` are not explicitly supplied, the port searches
for a versioned GNU compiler installed by Homebrew first:

```text
/opt/homebrew/bin/gcc-N
/opt/homebrew/bin/g++-N
/usr/local/bin/gcc-N
/usr/local/bin/g++-N
```

then by MacPorts:

```text
/opt/local/bin/gcc-mp-N
/opt/local/bin/g++-mp-N
```

The search prefers newer versions in the configured range. If no matching
compiler pair is found, the port stops with a clear error. A caller can still
override the selection explicitly:

```sh
make CC=/path/to/gcc CXX=/path/to/g++ ...
```

### Staged Prefix Cleanup

The wrapper build may stage gforth below a prefix different from the configured
runtime prefix, for example `stage/usr/local` even when the runtime target
prefix is `/usr`. The generated libcc directory is also machine-specific
(`amd64`, `arm64`, and so on).

For that reason `post-install` discovers staged files with:

```text
$(STAGEDIR)/*/lib/gforth/$(DISTVERSION)/
$(STAGEDIR)/*/lib/gforth/$(DISTVERSION)/*/libcc-named/
```

instead of hard-coding `$(STAGEDIR)$(PREFIX)` or one architecture name. It then
removes local `$(DESTDIR)$(PREFIX)` references from `envos.fs` and staged
libtool archives.

The earlier repeated ffcall build message:

```text
libgffflib.la: file not found
unix/fflib.fs: error: open-lib failed
```

was not a missing source file. It came from trying to open the freshly linked
`libgffflib` wrapper while its dependent `libavcall.so.1` and
`libcallback.so.1` were not visible to the dynamic loader. The port build
environment now sets:

```make
LD_LIBRARY_PATH=$(DESTDIR)$(PREFIX)/lib:$$LD_LIBRARY_PATH
```

so the optional ffcall wrapper can be opened when `WITH_LIBFFCALL=yes`.
Also, `envos.fs` must keep `$(DESTDIR)$(PREFIX)` during build so generated
libcc wrappers link against the local dependency root; prefix cleanup is applied
to staged files in `post-install` instead.

## 4. How To Test The Built Image

After staging, verify that the image header does not contain the build root:

```sh
head -1 work.compiler/stage/usr/lib/gforth/0.7.9_20260610/gforth.fi
LC_ALL=C awk 'BEGIN{RS="Gforth6"} NR==1{print length($0), length($0)%8; exit}' \
  work.compiler/stage/usr/lib/gforth/0.7.9_20260610/gforth.fi
strings work.compiler/stage/usr/lib/gforth/0.7.9_20260610/gforth.fi | rg '/home/lyc/pj/builds/uports.example'
```

Expected header for a `/usr` build:

```text
#! /usr/bin/gforth-0.7.9_20260610 --image-file
```

The `awk` check should print an offset with modulo `0`; the gforth image loader
searches for `Gforth6` on an aligned boundary. The `strings` command should
print nothing.

Also verify that the staged `.la` files have the target prefix, not the local
build root:

```sh
rg -n '^libdir=' work.compiler/stage/usr/lib/gforth/0.7.9_20260610/amd64/libcc-named/*.la
```

Expected style:

```text
libdir='/usr/lib/gforth/0.7.9_20260610/amd64/libcc-named'
```

To test the staged tree without installing into the host root, use a bind mount
environment that maps staged gforth data to the runtime paths expected by the
image:

```sh
bwrap \
  --ro-bind / / \
  --tmpfs /tmp \
  --bind /home/lyc/pj/builds/uports.example/feeds/lang/gforth/work.compiler/stage/usr/share/gforth /usr/share/gforth \
  --bind /home/lyc/pj/builds/uports.example/feeds/lang/gforth/work.compiler/stage/usr/lib/gforth /usr/lib/gforth \
  env XDG_CACHE_HOME=/tmp \
  /home/lyc/pj/builds/uports.example/feeds/lang/gforth/work.compiler/stage/usr/bin/gforth -e '." ok" cr bye'
```

Verified output:

```text
ok
```

If testing directly from the staged directory without bind-mounting staged
`/usr`, set both the source path and `libccdir`:

```sh
cd work.compiler/stage/usr/bin
XDG_CACHE_HOME=/tmp/gforth-stage-test \
libccdir=../lib/gforth/0.7.9_20260610/amd64/libcc-named \
./gforth-0.7.9_20260610-amd64 \
  -p ../share/gforth/0.7.9_20260610:../lib/gforth/0.7.9_20260610 \
  -e '." ok" cr bye'
```

For a package-installed test, install the package into the target root and run:

```sh
/usr/bin/gforth -e '." ok" cr bye'
```

The installed package should not need `GFORTHPATH` for normal startup when the
files are installed under the prefix used at build time.

## 5. Installing Outside `/usr`

The gforth image and generated libtool files are prefix-sensitive. Do not build
with `PREFIX=/usr` and then move the installed files to `/usr/local`; the saved
image header, gforth search paths, engine RUNPATH, and `.la` `libdir` values can
then point at the wrong location.

If the target install prefix is `/usr/local`, build the port with:

```sh
PREFIX=/usr/local
```

Then the package should contain paths like:

```text
/usr/local/bin/gforth-0.7.9_20260610
/usr/local/lib/gforth/0.7.9_20260610/gforth.fi
/usr/local/lib/gforth/0.7.9_20260610/kernl64l.fi
/usr/local/share/gforth/0.7.9_20260610/
```

The image header should become:

```text
#! /usr/local/bin/gforth-0.7.9_20260610 --image-file
```

The `.la` files should contain:

```text
libdir='/usr/local/lib/gforth/0.7.9_20260610/amd64/libcc-named'
```

The engine RUNPATH should also follow the chosen prefix:

```sh
readelf -d /usr/local/bin/gforth-0.7.9_20260610-amd64 | rg 'RUNPATH|RPATH'
```

Expected style:

```text
Library runpath: [/usr/local/lib]
```

The current Makefile avoids post-processing the binary `gforth.fi`. Instead, it
patches the upstream install rule so `gforthmi` creates `gforth.fi` with the
final runtime preamble:

```make
GFORTH_PREAMBLE="#! @bindir@/gforth-@PACKAGE_VERSION@ --image-file"
```

This matters because `gforth.fi` is binary and alignment-sensitive. Rewriting
the image with `sed` can move the `Gforth6` magic off the aligned boundary that
the loader scans and cause:

```text
image gforth.fi doesn't seem to be a Gforth (>=0.8) image
```

The Makefile still handles staged `.la` path cleanup by replacing:

```text
$(DESTDIR)$(PREFIX)
```

with:

```text
$(PREFIX)
```

in staged `libcc-named/*.la` files. This is correct for both `/usr` and
`/usr/local`, as long as `PREFIX` is set to the final runtime prefix during the
build.

## 6. Other Notes: Proposal For uports `BUILD_DEPENDS`

Current uports parses variables such as `BUILD_DEPENDS`, `RUN_DEPENDS`, and
`LIB_DEPENDS` in ports and `USES` files, but it does not yet execute
`BUILD_DEPENDS` automatically. A practical first implementation should be
small and package-oriented:

1. Keep the FreeBSD-style dependency syntax:

   ```text
   probe:origin
   name>=version:origin
   /path/to/file:origin
   libname.so:origin
   ```

2. Resolve each dependency to its `origin`, for example:

   ```text
   gforth-bootstrap>=0.7.3:lang/gforth-0.7.3
   ```

   resolves to:

   ```text
   $(PORTSDIR)/../feeds/lang/gforth-0.7.3
   ```

   or another configured ports root containing that origin.

3. Before `configure`, check whether the dependency is already installed in
   `$(DESTDIR)/var/db/pkg` using the existing package database. For the first
   version, checking package base/origin is enough; version comparison can be
   added after that.

4. If missing, run the dependency port:

   ```sh
   make package install-package
   ```

   with the parent build variables forwarded, especially:

   ```text
   PORTSDIR DESTDIR PREFIX USE_GLOBALBASE DISTDIR PACKAGES_SITE SCMDIR_SITE
   USE_ALTERNATIVE ALTERNATIVE_WRKDIR
   ```

5. Add a dedicated framework target such as `build-depends`, and make
   `_CONFIGURE_DEP` depend on it:

   ```make
   _CONFIGURE_DEP = build-depends patch
   ```

   or, more conservatively:

   ```make
   _CONFIGURE_DEP = patch build-depends
   ```

   The second form lets local patches apply before dependency checks, but the
   first form is usually cleaner because dependency availability is an input to
   configure.

6. Add recursion protection. At minimum, pass a variable such as
   `DEPENDS_CHAIN="$(DEPENDS_CHAIN) $(ORIGIN)"` and fail if the next dependency
   origin is already in the chain.

7. Add user controls:

   ```text
   NO_DEPENDS=yes          skip all automatic dependency builds
   NO_BUILD_DEPENDS=yes    skip only BUILD_DEPENDS
   FORCE_DEPENDS=yes       rebuild/install dependency packages even if installed
   ```

For `lang/gforth`, once this is implemented, a normal build of current
`gforth` should automatically install `lang/gforth-0.7.3` first, and the
existing `GFORTH=$(DESTDIR)$(PREFIX)/bin/gforth-0.7.3` setting will still be
needed to force upstream configure to use the correct bootstrap executable.
