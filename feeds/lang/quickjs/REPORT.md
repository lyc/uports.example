# QuickJS uports Build Report

This report records the current `lang/quickjs` package behavior for QuickJS
`2025-09-13_2`.

## 1. Bootstrapping Status

QuickJS does not require an external JavaScript engine for the normal package
flow. It builds its own `qjs` interpreter and `qjsc` compiler from C sources.

Bootstrap classification: no external bootstrap needed. In SCM mode, there is a
source-tree bootstrap helper, but not a separate host runtime package.

In SCM mode, the port runs an upstream bootstrap helper before the normal build:

```make
make test2-bootstrap
```

That step is local to the QuickJS source tree. It is not a separate uports
bootstrap package dependency.

## 2. Build Flow

The port supports tarball mode from `bellard.org` and SCM mode from GitHub.
Tarball mode uses:

```text
quickjs-2025-09-13-2.tar.xz
quickjs-extras-2025-09-13.tar.xz
```

SCM mode uses:

```make
USE_SCM ?= git
SCM_PROTOCOL = https://
MASTER_SITES = github.com/
MASTER_SITE_SUBDIR = bellard
SCM_DEST = quickjs
```

The upstream build target is:

```make
ALL_TARGET = testall
```

If `PREFIX` is set, the port passes it to upstream make:

```make
MAKE_ENV += prefix=$(PREFIX)
```

The port generates `quickjs.pc` from `files/quickjs.pc.in` in `post-patch` and
installs it under `$(PREFIX)/lib/pkgconfig` in `post-install`.

## 3. Built Image Layout And Relationships

The package plist describes this runtime/development layout:

```text
/bin/qjs
/bin/qjsc
/include/quickjs/quickjs.h
/include/quickjs/quickjs-libc.h
/lib/quickjs/libquickjs.a
/lib/pkgconfig/quickjs.pc
```

Runtime relationship:

```text
qjs
  -> QuickJS command-line interpreter

qjsc
  -> bytecode/native executable compiler
  -> uses QuickJS runtime code from the same source build

libquickjs.a
  -> static library for embedding QuickJS
  -> headers under include/quickjs
```

`quickjs.pc` exposes the embedding flags:

```text
Libs: -L${libdir}/quickjs -lquickjs -lm -ldl -lpthread
Cflags: -I${includedir}
```

## 4. How To Test The Built Image

After staging or installing, test the interpreter:

```sh
feeds/lang/quickjs/work.compiler/stage/usr/bin/qjs -e 'console.log(1 + 2)'
```

Expected output:

```text
3
```

Test the compiler by producing and running bytecode:

```sh
feeds/lang/quickjs/work.compiler/stage/usr/bin/qjsc \
  -o /tmp/qjs-test.c -c /tmp/test.js
```

For a simpler package smoke test, compile a small program to an executable when
the host compiler is available:

```sh
printf 'console.log("quickjs-ok")\n' >/tmp/quickjs-test.js
feeds/lang/quickjs/work.compiler/stage/usr/bin/qjsc \
  -o /tmp/quickjs-test /tmp/quickjs-test.js
/tmp/quickjs-test
```

Expected output:

```text
quickjs-ok
```

Check the pkg-config file:

```sh
PKG_CONFIG_PATH=feeds/lang/quickjs/work.compiler/stage/usr/lib/pkgconfig \
  pkg-config --libs --cflags quickjs
```

## 5. Installing Outside `/usr`

QuickJS is less image-sensitive than gforth, CLISP, or SBCL because it does not
install a separate saved runtime image. It still uses `PREFIX` for install
locations and the generated `quickjs.pc`.

For `/usr/local`, build with:

```sh
PREFIX=/usr/local
```

Expected layout:

```text
/usr/local/bin/qjs
/usr/local/bin/qjsc
/usr/local/include/quickjs/
/usr/local/lib/quickjs/libquickjs.a
/usr/local/lib/pkgconfig/quickjs.pc
```

Verify `quickjs.pc` after install:

```sh
pkg-config --variable=prefix quickjs
```

It should print the prefix used at package build/install time.

## 6. Other Notes

The current plist is named `pkg-plist`, not `pkg-plist.linux`. If platform
differences appear later, split it into platform-specific plists.

SCM mode currently has no release branch/tag detach in the Makefile. Tarball
mode is more reproducible for the `2025-09-13_2` package version.
