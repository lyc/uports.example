# CPython uports Build Report

This report records the current `lang/cpython` package behavior for CPython
`3.14.5`.

## 1. Bootstrapping Status

CPython does not require a preinstalled target Python for this native package
flow. The upstream build creates and uses its own build-time interpreter while
building the standard library and extension modules.

Bootstrap classification: required self bootstrap. The build creates the
interpreter it needs during the upstream build; no external target Python package
is required for this native build.

Current uports does not automatically execute `BUILD_DEPENDS`, but this port
does not currently express a separate host-Python bootstrap package dependency.
It does require the declared C libraries to be built/installed first.

## 2. Build Flow

The port can use SCM mode and pins the checkout to `v3.14.5`:

```make
USE_SCM ?= git
SCM_DETACH = v3.14.5
```

The configure step is the standard CPython configure script:

```make
GNU_CONFIGURE = yes
CONFIGURE_ARGS += --with-openssl=$(DESTDIR)$(PREFIX) \
                  --with-openssl-rpath=$(DESTDIR)$(PREFIX)/$(OPENSSL_LIBDIR)
```

`OPENSSL_LIBDIR` selects `lib64` when `$(DESTDIR)$(PREFIX)/lib64/libssl.so`
exists, otherwise `lib`.

The current port also applies `files/0001-disable-some-modules-now.patch`, which
disables:

```text
_tkinter
```

The port now declares provider libraries for `_sqlite3`, `_uuid`, `_zstd`,
`_bz2`, and `_lzma`. Build/install `databases/sqlite`, `devel/libuuid`,
`archivers/zstd`, `archivers/bzip2`, and `archivers/xz` before reconfiguring
CPython. `_tkinter` remains disabled intentionally because Tcl/Tk is not needed
for the current package.

Current declared library dependencies are:

```text
archivers/zlib
archivers/bzip2
archivers/xz
archivers/zstd
security/openssl
databases/sqlite
databases/gdbm
devel/libffi
devel/ncurses
devel/readline
devel/libuuid
```

## 3. Built Image Layout And Relationships

The staged package installs the interpreter commands:

```text
/usr/bin/python3.14
/usr/bin/python3      -> python3.14
/usr/bin/python3.14-config
/usr/bin/python3-config -> python3.14-config
/usr/bin/pip3.14
/usr/bin/pip3         -> pip3.14
/usr/bin/pydoc3.14
/usr/bin/pydoc3       -> pydoc3.14
/usr/bin/idle3.14
/usr/bin/idle3        -> idle3.14
```

The main runtime layout is:

```text
/usr/lib/python3.14/                 standard library
/usr/lib/python3.14/lib-dynload/     extension modules
/usr/lib/python3.14/site-packages/   installed Python packages, including pip
/usr/include/python3.14/             C API headers
/usr/lib/libpython3.14.a             static libpython
/usr/lib/pkgconfig/python-3.14.pc
/usr/lib/pkgconfig/python-3.14-embed.pc
```

Runtime relationship:

```text
/usr/bin/python3.14
  -> computes sys.prefix/sys.exec_prefix from its install location
  -> loads /usr/lib/python3.14
  -> imports extension modules from /usr/lib/python3.14/lib-dynload
```

Important extension modules currently present include:

```text
_ssl
_hashlib
_ctypes
_gdbm
_dbm
zlib
readline
pyexpat
```

Optional extension modules intentionally not present in the current package:

```text
_tkinter
```

## 4. How To Test The Built Image

Direct staged test:

```sh
feeds/lang/cpython/work.compiler/stage/usr/bin/python3.14 \
  -c 'import sys; print(sys.version.split()[0]); print(sys.prefix)'
```

Expected version:

```text
3.14.5
```

Import smoke test:

```sh
feeds/lang/cpython/work.compiler/stage/usr/bin/python3.14 - <<'EOF'
import ctypes
import hashlib
import ssl
import zlib
print("python-ok")
print(ssl.OPENSSL_VERSION)
EOF
```

Expected output includes:

```text
python-ok
OpenSSL
```

Pip smoke test:

```sh
feeds/lang/cpython/work.compiler/stage/usr/bin/python3.14 -m pip --version
```

After package installation into the target root, test the installed command:

```sh
/usr/bin/python3.14 -c 'import sys; print(sys.version.split()[0])'
```

## 5. Installing Outside `/usr`

CPython computes `sys.prefix` from the interpreter location and standard-library
landmarks, but generated scripts, `python3-config`, pkg-config files, and
extension RUNPATHs can still be prefix-sensitive. Build with the final runtime
prefix instead of moving an installed tree.

For `/usr/local`, build with:

```sh
PREFIX=/usr/local
```

Expected layout:

```text
/usr/local/bin/python3.14
/usr/local/lib/python3.14/
/usr/local/include/python3.14/
/usr/local/lib/pkgconfig/python-3.14.pc
```

Verify:

```sh
/usr/local/bin/python3.14 -c 'import sys; print(sys.prefix)'
/usr/local/bin/python3.14-config --prefix
```

Both should report `/usr/local`.

## 6. Other Notes

### Darwin curses provider consistency

The host may expose Homebrew SDK ncursesw.pc while /usr/local/include and
/usr/local/lib supply a narrow ncurses build. Metadata success alone then
defines HAVE_NCURSESW although the selected headers lack wide functions.
The port declares devel/ncurses and patches configure.ac plus its Autoconf
2.72-generated configure to compile/link wide insert and unget calls before
accepting Darwin ncursesw metadata. A mismatch rejects that candidate and
uses the existing ncurses/panel fallback; a matching wide installation remains
supported. No warning is suppressed or module disabled. Linux retains the
existing selection behavior.

Existing trees need the new patch applied and configure rerun. Darwin build
and runtime acceptance remain pending. After building, import curses and
curses.panel, inspect _curses loader dependencies, and use curses.wrapper in
a real terminal to confirm initialization and terminal I/O. A narrow provider
does not expose wide-character-only curses methods.

### Darwin SDK and deployment target

New Darwin SDKs declare dup3 and pipe2 as available starting in macOS 27.
CPython 3.14.5's generic function-link probes can enable them while compiling
for an older deployment target, then posixmodule.c fails availability checks.
This port conservatively sets ac_cv_func_dup3=no and ac_cv_func_pipe2=no on
Darwin, retaining CPython's existing dup2/pipe and descriptor-inheritance
fallbacks. Linux probes remain unchanged. On macOS 27 this also intentionally
disables os.pipe2 until the workaround is revisited for an upstream version
with deployment-aware checks or guarded calls.

Existing configured trees must rerun configure: rebuild alone retains the
old pyconfig.h. After reconfiguration, HAVE_DUP3 and HAVE_PIPE2 must be
undefined on Darwin. Validate os.pipe and os.dup2 with data transfer and
descriptor inheritance checks after a successful build. Darwin acceptance
of this workaround is pending; no availability warning is suppressed.

The current `_ssl` extension was observed with a RUNPATH pointing at the local
uports dependency root:

```text
/home/lyc/pj/builds/uports.example/local/compiler/usr/lib
```

That is acceptable for local staged testing but is not prefix-clean package
metadata. Before treating this package as relocatable or final, verify extension
RUNPATHs and adjust `--with-openssl-rpath` or post-install cleanup so runtime
paths refer to the final `PREFIX` rather than `$(DESTDIR)$(PREFIX)`.
