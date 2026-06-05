# sqlite uPorts Notes

## Source Modes

Default source mode uses SQLite's upstream Unix autoconf release bundle:

```make
MASTER_SITES = https://sqlite.org/2026/
DISTNAME = sqlite-autoconf-3530200
```

This tarball corresponds to SQLite `3.53.2`.

SCM mode is available through the GitHub mirror of SQLite's official Fossil
repository:

```sh
make USE_SCM=git V=1 extract
```

The SCM checkout is pinned to tag `version-3.53.2`.

The release tarball remains the default because upstream describes the
autoconf bundle as the source package that includes the generated `configure`
script. The raw repository source may require additional upstream generation
steps for some workflows.

## Installed Layout

Common installed files:

```text
bin/sqlite3
include/sqlite3.h
include/sqlite3ext.h
lib/libsqlite3.a
lib/pkgconfig/sqlite3.pc
share/man/man1/sqlite3.1
```

Darwin shared library files:

```text
lib/libsqlite3.dylib
lib/libsqlite3.0.dylib
lib/libsqlite3.3.53.2.dylib
```

Linux shared library files are expected from the upstream install rules:

```text
lib/libsqlite3.so
lib/libsqlite3.so.0
lib/libsqlite3.so.0.8.6
```

## Build Flow

The port uses the normal configure build:

```sh
make V=1 stage \
  PORTSDIR=/path/to/uports \
  DESTDIR=/path/to/local/compiler \
  PREFIX=/usr
```

Readline support is disabled for now to avoid accidentally linking the
`sqlite3` CLI against a host `/usr/local` readline library. That can be revisited
after uPorts has a declared readline dependency for this port.

To generate or verify distinfo:

```sh
make V=1 makesum PORTSDIR=/path/to/uports
make V=1 checksum PORTSDIR=/path/to/uports
```

## Runtime Test

After install or stage, run a real query instead of only checking the version:

```sh
sqlite3 :memory: 'select sqlite_version(), 1 + 1;'
```

Expected output includes `3.53.2|2`.
