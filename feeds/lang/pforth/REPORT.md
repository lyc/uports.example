# pForth uports Build Report

This report records the `lang/pForth` package behavior for pForth `2.0.1`.

## 1. Bootstrapping Status

pForth does not require an external Forth system. The package builds the C
kernel and then uses the freshly built `pforth` to generate the dictionary used
by the normal runtime.

Bootstrap classification: required self bootstrap. The bootstrap is
self-contained in the source tree.

## 2. Source Modes

The port defaults to SCM mode:

```make
USE_SCM ?= git
SCM_PROTOCOL = https://
MASTER_SITES = github.com/
MASTER_SITE_SUBDIR = philburk
SCM_DEST = pforth-2.0.1
SCM_DETACH = v2.0.1
```

Tarball mode is also described by the Makefile and uses the GitHub tag archive
for `v2.0.1`.

## 3. Build Flow

Upstream documents the Unix build under `platforms/unix`:

```sh
cd platforms/unix
make all
make test
```

The uports port follows that layout by setting:

```make
BUILD_WRKSRC = $(WRKSRC)/platforms/unix
INSTALL_WRKSRC = $(WRKSRC)/platforms/unix
ALL_TARGET = all
```

There is no external bootstrap dependency.

## 4. Runtime Layout

The package installs:

```text
$(PREFIX)/bin/pforth
$(PREFIX)/bin/pforth_standalone
$(PREFIX)/libexec/pforth/pforth-bin
$(PREFIX)/share/pforth/pforth.dic
$(PREFIX)/share/pforth/fth/
$(PREFIX)/share/doc/pforth/
```

`pforth` is a wrapper that runs `libexec/pforth/pforth-bin` with the installed
dictionary path by default. `pforth_standalone` is the all-in-one runtime with
the dictionary built into the executable. The staged package keeps the generated
`pforth.dic` next to the installed Forth sources under `share/pforth`.

## 5. How To Test The Built Runtime

Run the standalone binary from the staged tree:

```sh
printf '3 4 + . cr bye\n' | \
  feeds/lang/pForth/work/stage/usr/bin/pforth_standalone
```

Run the dictionary-loading wrapper:

```sh
printf '3 4 + . cr bye\n' | \
  feeds/lang/pForth/work/stage/usr/bin/pforth
```

Expected output:

```text
7
```

The upstream test target can also be run from the build directory:

```sh
cd feeds/lang/pForth
make V=1 -C work/pforth-2.0.1/platforms/unix test
```

## 6. Installing Outside `/usr`

`pforth_standalone` does not need an external dictionary path. The installed
`pforth` wrapper derives its prefix from its own path, so it works from a staged
tree and after installation under a non-final root such as `local/compiler/usr`.
Use `libexec/pforth/pforth-bin` directly when testing custom dictionary
locations.
