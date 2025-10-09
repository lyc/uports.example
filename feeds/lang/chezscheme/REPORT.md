# ChezScheme uports Build Report

This report records the current `lang/chezscheme` package behavior for
ChezScheme `10.2.0`.

## 1. Bootstrapping Status

ChezScheme does not require a separately installed Chez Scheme for this package
flow. The upstream tree contains boot files, and the uports build uses the
normal upstream build system from the checked-out source.

Bootstrap classification: required self bootstrap. The bootstrap inputs are
bundled in the source checkout, including upstream boot files and submodules.

The port does need the upstream git submodules. The current `post-patch` step
runs:

```sh
git submodule init
git submodule update
```

Current uports does not automatically execute `BUILD_DEPENDS`, but this port's
bootstrap is source-tree based, not a host-Scheme package dependency.

## 2. Build Flow

The port fetches `cisco/ChezScheme` from git and checks out `v10.2.0`:

```make
USE_SCM = git
SCM_DETACH = v10.2.0
```

The configure step is upstream's configure script, with uports flags carried by
`GNU_CONFIGURE=yes`:

```text
--enable-libffi
--disable-x11
--installprefix=$(DESTDIR)$(PREFIX)
--temproot=$(WRKDIR)/stage
```

On Linux, `LD_LIBRARY_PATH=$(DESTDIR)$(PREFIX)/lib` is added while building so
newly built port libraries can be found. On Darwin, the port disables iconv and
has a patch to pass `DYLD_LIBRARY_PATH` to Scheme subprocesses. The Darwin
install step also rewrites the Chez runtime executable rpath from the build
root library directory to `@loader_path/../..`, so installed binaries under
`$(PREFIX)/lib/csv10.2.0/<machine>` can find dependency libraries in
`$(PREFIX)/lib` without relying on `DYLD_LIBRARY_PATH`.

Linux dependencies are currently:

```text
devel/ncurses
devel/libffi
converters/libiconv
```

## 3. Built Image Layout And Relationships

The verified Linux x86_64 staged package installs:

```text
/usr/bin/scheme         -> ../lib/csv10.2.0/ta6le/scheme
/usr/bin/petite         -> ../lib/csv10.2.0/ta6le/petite
/usr/bin/scheme-script  -> ../lib/csv10.2.0/ta6le/scheme-script

/usr/lib/csv10.2.0/ta6le/scheme
/usr/lib/csv10.2.0/ta6le/petite
/usr/lib/csv10.2.0/ta6le/scheme-script
/usr/lib/csv10.2.0/ta6le/scheme.boot
/usr/lib/csv10.2.0/ta6le/petite.boot
/usr/lib/csv10.2.0/ta6le/scheme-script.boot
/usr/lib/csv10.2.0/ta6le/scheme.h
/usr/lib/csv10.2.0/ta6le/libkernel.a
/usr/lib/csv10.2.0/ta6le/libz.a
/usr/lib/csv10.2.0/ta6le/liblz4.a
/usr/lib/csv10.2.0/ta6le/main.o
```

The runtime relationship is:

```text
/usr/bin/scheme
  -> /usr/lib/csv10.2.0/ta6le/scheme
       + /usr/lib/csv10.2.0/ta6le/scheme.boot
       + shared libraries from the target prefix/system
```

`petite` is the smaller runtime and uses `petite.boot`. `scheme-script` is the
script entry point and uses `scheme-script.boot`.

The architecture directory `ta6le` is part of Chez's machine naming. For other
architectures, the package layout may use a different machine directory and
therefore a different plist.

## 4. How To Test The Built Image

For a staged Linux test before installing dependencies into the host root, point
the loader at the uports library directory:

```sh
LD_LIBRARY_PATH=/home/lyc/pj/builds/uports.example/local/compiler/usr/lib \
  feeds/lang/chezscheme/work.compiler/stage/usr/bin/scheme --version
```

Expected output:

```text
10.2.0
```

Basic runtime smoke test:

```sh
LD_LIBRARY_PATH=/home/lyc/pj/builds/uports.example/local/compiler/usr/lib \
  feeds/lang/chezscheme/work.compiler/stage/usr/bin/scheme --script /dev/stdin <<'EOF'
(display (+ 20 22))
(newline)
EOF
```

Expected output:

```text
42
```

After package installation into the target root, the local `LD_LIBRARY_PATH`
override should not be needed if `libiconv`, `libffi`, and `ncurses` are
available in the target runtime library path.

## 5. Installing Outside `/usr`

ChezScheme installs relative front-end symlinks from `bin` into
`lib/csv10.2.0/<machine>`, but the package should still be built with the final
runtime prefix.

For `/usr/local`, build with:

```sh
PREFIX=/usr/local
```

Expected layout:

```text
/usr/local/bin/scheme -> ../lib/csv10.2.0/ta6le/scheme
/usr/local/lib/csv10.2.0/ta6le/scheme
/usr/local/lib/csv10.2.0/ta6le/scheme.boot
```

Do not build for `/usr` and then move the tree to `/usr/local`; package
manifests and any generated metadata should reflect the final install prefix.

On Darwin, a bad installed rpath looks like:

```text
LC_RPATH path /path/to/uports/local/compiler/usr/local/lib
```

This can make Emacs `run-chez` fail during Geiser startup because Geiser first
runs `scheme --version` as an Emacs subprocess. The subprocess may not inherit
the shell environment that made direct terminal execution work, producing a
dyld error for `@rpath/libffi.8.dylib`. A correct Darwin install should have:

```text
LC_RPATH path @loader_path/../..
```

## 6. Other Notes

The current Linux staged `scheme` binary links to `libiconv.so.2`; on a host
where uports dependencies are only installed under `local/compiler/usr/lib`,
direct staged execution needs `LD_LIBRARY_PATH` as shown above.
