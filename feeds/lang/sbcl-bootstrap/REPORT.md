# SBCL Bootstrap Port Report

## Summary

`lang/sbcl-bootstrap` packages an upstream prebuilt SBCL binary distribution as
a private bootstrap host for building `lang/sbcl`.

Bootstrap classification: no external bootstrap needed for this port. It is a
binary bootstrap package. The resulting package is not intended to be the user
SBCL; it installs `sbcl-bootstrap` for build use.

## Source Selection

SBCL source tarballs do not bundle a prebuilt image. Upstream provides source
tarballs and separate platform binary tarballs. This port fetches one of the
official binary tarballs and installs only the runtime executable and core image.

The selected distfile depends on the build host:

```text
linux/x86_64    sbcl-2.6.3-x86-64-linux-binary.tar.bz2
linux/aarch64   sbcl-1.4.2-arm64-linux-binary.tar.bz2
linux/i386      sbcl-1.4.3-x86-linux-binary.tar.bz2
darwin/x86_64   sbcl-2.2.9-x86-64-darwin-binary.tar.bz2
darwin/aarch64  sbcl-2.4.0-arm64-darwin-binary.tar.bz2
```

The older platform binaries are intentional: SBCL's download table does not
publish the newest SBCL binary for every OS/CPU pair, and upstream documents
using older binaries as bootstrap hosts for newer source builds.

## Installed Layout

The package avoids colliding with the real `lang/sbcl` package by installing the
upstream binary under a private tree:

```text
$(PREFIX)/bin/sbcl-bootstrap
$(PREFIX)/libexec/sbcl-bootstrap/bin/sbcl
$(PREFIX)/libexec/sbcl-bootstrap/lib/sbcl/sbcl.core
```

`sbcl-bootstrap` is a small wrapper. It computes its private runtime root
relative to its own installed path, sets `SBCL_HOME` if the caller has not
already set it, and then execs the private `sbcl` binary.

## How To Use

Build and install the bootstrap package first:

```sh
make USE_GLOBALBASE=yes compiler@sbcl-bootstrap.install
```

Then build SBCL with:

```sh
make USE_GLOBALBASE=yes compiler@sbcl.build
```

The main `lang/sbcl` port automatically uses the bootstrap command when
`$(DESTDIR)$(PREFIX)/bin/sbcl-bootstrap` exists. Direct port builds can also use
`WITH_SBCL_BOOTSTRAP=yes`, which maps to:

```make
SBCL_XC_HOST=$(DESTDIR)$(PREFIX)/bin/sbcl-bootstrap --no-userinit --no-sysinit
```

Because current uports does not automatically execute `BUILD_DEPENDS`, the
bootstrap package must be built and installed manually before building
`lang/sbcl`.

## Smoke Test

After staging:

```sh
work.compiler/stage/usr/bin/sbcl-bootstrap \
  --noinform --non-interactive \
  --eval '(format t "~A~%" (lisp-implementation-version))'
```

Expected output is the bootstrap SBCL version selected for the platform.

## Notes

The bootstrap binary and core must stay together. SBCL core images are not
portable across arbitrary runtime executables, so this package installs a matched
runtime/core pair from the same upstream binary tarball.
