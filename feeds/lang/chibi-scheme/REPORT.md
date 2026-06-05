# Chibi-Scheme uports Build Report

This report records the current `lang/chibi-scheme` package behavior for
Chibi-Scheme `0.12`.

## 1. Bootstrapping Status

Chibi-Scheme does not require a preinstalled Scheme implementation. The package
build starts from C sources, builds the `chibi-scheme` executable and shared
runtime library, then uses the freshly built executable to generate FFI helper
sources and module metadata.

Bootstrap classification: required self bootstrap. The bootstrap is
self-contained in the source tree.

## 2. Source Modes

The port supports both source modes:

```text
default: USE_SCM=git, GitHub tag 0.12
tarball: USE_SCM=, GitHub tag archive 0.12.tar.gz
```

Both modes extract or clone to `work/chibi-scheme-0.12`, so the package name
stays versioned as `chibi-scheme-0.12`.

## 3. Build Flow

The upstream build is a plain make build. uports passes:

```text
PREFIX=$(PREFIX)
DESTDIR=$(STAGEDIR)
```

The install phase invokes upstream `make install`. Upstream attempts to generate
image files only when `DESTDIR` is empty; during staged package installs those
commands are skipped by design and their ignored nonzero status is expected.

On Darwin, upstream links the executable and loadable modules against
`@rpath/libchibi-scheme.0.12.0.dylib` but does not add install rpaths for all
installed paths. The port adds relative rpaths in `post-stage`:

```text
bin/chibi-scheme:              @loader_path/../lib
lib/chibi/chibi/*.dylib:       relative path back to $(PREFIX)/lib
lib/chibi/chibi/*/*.dylib:     relative path back to $(PREFIX)/lib
```

## 4. Runtime Layout

Installed commands:

```text
$(PREFIX)/bin/chibi-scheme
$(PREFIX)/bin/chibi-ffi
$(PREFIX)/bin/chibi-doc
$(PREFIX)/bin/snow-chibi
```

Core runtime library:

```text
Darwin:
$(PREFIX)/lib/libchibi-scheme.0.12.0.dylib
$(PREFIX)/lib/libchibi-scheme.0.dylib
$(PREFIX)/lib/libchibi-scheme.dylib

Linux:
$(PREFIX)/lib/libchibi-scheme.so.0.12.0
$(PREFIX)/lib/libchibi-scheme.so.0
$(PREFIX)/lib/libchibi-scheme.so
```

Scheme source modules, including `*.scm` and `*.sld`, are installed under
`$(PREFIX)/share/chibi`. Compiled loadable modules are installed under
`$(PREFIX)/lib/chibi`.

The compiled default module search path is generated in
`include/chibi/install.h` during the build:

```text
$(PREFIX)/share/chibi
$(PREFIX)/lib/chibi
/usr/local/share/snow
/usr/local/lib/snow
```

`CHIBI_MODULE_PATH` overrides or supplements that path for staged tests and for
installs under a non-final root.

## 5. How To Test The Built Runtime

### Staged Tree

Direct staged execution outside the final `/usr` prefix needs `CHIBI_MODULE_PATH`
overridden because the binary's compiled default module path points at the final
runtime prefix:

```sh
CHIBI_MODULE_PATH=feeds/lang/chibi-scheme/work/stage/usr/share/chibi:feeds/lang/chibi-scheme/work/stage/usr/lib/chibi \
  feeds/lang/chibi-scheme/work/stage/usr/bin/chibi-scheme \
  -q -mchibi.filesystem -e '(display "ok") (newline)'
```

Expected output:

```text
ok
```

### Installed Under A Non-Final Root

When the package is installed under a non-final root, such as building for
`/usr` but installing under `local/compiler/usr`, use the installed tree paths:

```sh
CHIBI_MODULE_PATH=local/compiler/usr/share/chibi:local/compiler/usr/lib/chibi \
  local/compiler/usr/bin/chibi-scheme \
  -q -mchibi.filesystem -e '(display "ok") (newline)'
```

Expected output:

```text
ok
```

### Installed At The Final Prefix

After installation into the final prefix, `CHIBI_MODULE_PATH` should not be
needed because Chibi's compiled default module path points at
`$(PREFIX)/share/chibi` and `$(PREFIX)/lib/chibi`:

```sh
/usr/bin/chibi-scheme \
  -q -mchibi.filesystem -e '(display "ok") (newline)'
```

Expected output:

```text
ok
```
