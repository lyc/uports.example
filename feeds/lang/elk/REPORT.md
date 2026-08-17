# Elk 3.99.8 port report

## Port shape and source

Elk is the Extension Language Kit, an R4RS-era Scheme implementation intended
primarily for embedding in C and C++ applications.  The package also installs
the standalone `elk` interpreter, a `scheme-elk` symlink, the embeddable
`libelk` library and headers, Scheme runtime files, loadable extensions,
examples, and manual pages.

The default source mode is `USE_SCM ?= git`.  It uses the Scheme Conservatory
repository and detaches the checkout at commit
`5ba5ffc44da4943139fa27879df2aa748abf1c71`, the upstream 3.99.8 release
revision.  `SCM_RETRIEVE_TIMEOUT=300` gives SCM retrieval bounded failure
behavior when the host provides the framework's timeout utility.

An official release tarball is also supported without changing the default:
set `USE_SCM=` explicitly to fetch `elk-3.99.8.tar.bz2` from
`https://sam.zoy.org/elk/`.  Its recorded distinfo is:

```text
SHA256 (elk/elk-3.99.8.tar.bz2) = a148320c8d2c2b1277ad3572a9d8a9eb4db0473643caa35f098e28c9da14dc66
SIZE (elk/elk-3.99.8.tar.bz2) = 701206
```

The project wrapper does not preserve an explicitly empty variable for its
dotted convenience targets.  Invoke the port directly when selecting the
tarball, while still supplying the framework and target roots:

```sh
ROOT=$PWD
make -C feeds/lang/elk stage USE_SCM= \
  PORTSDIR="$ROOT/make/uports" \
  DESTDIR="$ROOT/local/compiler" \
  PREFIX=/usr/local \
  USE_ALTERNATIVE=yes \
  ALTERNATIVE_WRKDIR="$ROOT/local/compiler/src" \
  TYPE_SUFFIX=.compiler
```

The SCM and release-tarball sources contain `configure.ac` and Automake inputs
but no generated `configure` script.  The old `bootstrap` script incorrectly
rejects modern Automake versions with more than one dot in their version
number.  Both source modes therefore use the framework's `autoreconf` support
and declare the resulting Autoconf, Automake, and libtool build requirements
through `USES`.

## Bootstrap classification and build fixes

Elk has no language-runtime bootstrap.  Its interpreter and library are built
directly from C and C++ sources with the host toolchain.  Autotools generation
is a build-system preparation step, not an Elk or Scheme bootstrap.

Elk's source uses K&R-style C definitions that C99 and newer modes reject.
The port selects `-std=gnu89`, preserving the language mode expected by the
source instead of broadly rewriting the historical implementation.

Three small source patches are required:

- refer to the same-directory `libelk.la` target by the spelling emitted by
  modern Automake;
- include the declaration of `strlen` and use the portable `%p` conversion for
  procedure pointers;
- expose Darwin's `wait3` and `wait4` declarations when configure enables
  those UNIX-extension entry points.

Elk's dynamic loader intentionally reads installed `.la` files to discover the
corresponding loadable object name.  The recipe therefore uses
`libtool:keepla`.  The uports libtool stage hook was made portable across GNU
and BSD `sed`: it now uses an explicit `.bak` suffix and deletes those backups.
This retains sanitized `.la` metadata without accidental `.la-e` files.

## Installed layout

The principal installed relationships are:

```text
bin/elk                              standalone interpreter
bin/scheme-elk -> elk                collision-resistant Scheme alias
lib/libelk.0.dylib                   Darwin shared runtime
lib/libelk.dylib -> libelk.0.dylib   unversioned development link
lib/libelk.a                         static runtime
lib/libelk.la                        libtool development metadata
include/elk                          embedding and extension headers
share/elk                            Scheme startup and library sources
lib/elk/*.la                         extension metadata read by Elk
lib/elk/*.so                         loadable Elk extensions on Darwin
share/doc/elk/examples               Scheme and C++ examples
```

The validated Darwin build installs the bitstring, debug, elk-eval, hack,
newhandler, record, regexp, struct, and UNIX extensions.  X11, Xt, Athena, and
Motif development files were not present, so their optional native extensions
were not built.  Their example source files are still installed as upstream
documentation.

## Runtime verification

Elk is configured with the final prefix `/usr/local`.  A staged or target-root
test must therefore supply the staged library to dyld and override Elk's
colon-separated load path with `-p`.  The load path must contain both the
Scheme sources and the directory containing extension `.la` files:

```sh
cd /path/to/uports.example
ELK_ROOT="$PWD/feeds/lang/elk/work.compiler/stage/usr/local"

printf '(write (+ 20 22)) (newline)\n' |
  env DYLD_LIBRARY_PATH="$ELK_ROOT/lib" \
  "$ELK_ROOT/bin/elk" \
    -p "$ELK_ROOT/share/elk:$ELK_ROOT/lib/elk" -l -
```

This prints `42`.  The `scheme-elk` symlink and real native extension loading
were tested with:

```sh
printf "(require 'regexp)\n\
(write (regexp? (make-regexp \"a+\")))\n\
(newline)\n" |
  env DYLD_LIBRARY_PATH="$ELK_ROOT/lib" \
  "$ELK_ROOT/bin/scheme-elk" \
    -p "$ELK_ROOT/share/elk:$ELK_ROOT/lib/elk" -l -

printf "(require 'unix)\n\
(write (system-record? (unix-system-info)))\n\
(newline)\n" |
  env DYLD_LIBRARY_PATH="$ELK_ROOT/lib" \
  "$ELK_ROOT/bin/elk" \
    -p "$ELK_ROOT/share/elk:$ELK_ROOT/lib/elk" -l -
```

Both commands print `#t`.  Running with `-v load` additionally confirmed that
the regexp test dynamically loaded the staged `lib/elk/regexp.so` rather than
silently using only Scheme code.

When Elk is actually installed at its configured final prefix, `/usr/local`,
neither `DYLD_LIBRARY_PATH` nor `-p` should be necessary:

```sh
/usr/local/bin/elk
```

The staged and packaged target-root installation was also reinstalled under
`local/compiler/usr/local`; arithmetic and regexp loading passed there with
the corresponding target-root loader and `-p` settings.

## Darwin validation and remaining platform work

The validated platform is arm64 Darwin.  A clean SCM checkout completed
autoreconf, configure, build, stage, package, and reinstall.  A separate clean
official-tarball fetch completed checksum verification, extraction,
autoreconf, configure, build, stage, and package.  Its arithmetic, regexp, and
UNIX runtime tests all passed.  The Darwin plist has 163 entries and exactly
matches fresh `generate-plist` results from both source modes.

Mach-O inspection showed `@rpath/libelk.0.dylib` dependencies and only the
final `/usr/local/lib` rpath on the interpreter, shared runtime, and loadable
extensions.  No workspace, `DESTDIR`, or work-tree paths were found in the
stage.  Darwin uses `.dylib` for `libelk` but retains upstream `.so` names for
loadable bundles.

The build emits many warnings from its historical C interfaces and obsolete
Autoconf macros.  It also emits non-fatal libtool probing diagnostics in the
sandbox, but the generated binaries, bundles, stage, and package complete and
run successfully.

Linux has not yet been validated.  It needs a native clean stage, a
platform-generated plist, ELF SONAME and RUNPATH inspection, and the same
arithmetic, regexp, and UNIX-extension runtime tests.  Linux target-root tests
should use `LD_LIBRARY_PATH` instead of `DYLD_LIBRARY_PATH`; an actual final
installation must resolve its final-prefix library through RUNPATH,
`ldconfig`, or the host loader configuration.
