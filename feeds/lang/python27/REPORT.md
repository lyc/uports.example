# Native Python 2 build interpreter

Python 2.7.18 is the final Python 2 release, selected for compatibility with
historical QEMU and GLib build scripts. It is a 2020 release, not a version
specified by MIT or a reproduction of the 2018 host environment. The agreed
policy preserves MIT's named tool releases and pinned emulator source while
documenting host prerequisites as compatibility choices.

Use the official release archive, native GNU89 C and a static libpython.
altinstall installs versioned python2.7 commands without replacing python or
python3. ensurepip is disabled. This interpreter is a native build tool;
Unversioned 2to3/idle/pydoc/smtpd.py helpers left by upstream altinstall are
omitted from stage; versioned interpreter/config commands remain available.
CROSS_COMPILE is rejected. Optional extension discovery follows upstream and
does not promise a complete Python distribution or current OpenSSL support.

The project toolchain group installs it under DESTDIR/usr/local, separate from
profile output roots and the compiler group's target development environment.
No bootstrap interpreter, target compiler or target sysroot is required.
The recipe uses shipped configure files; no autoreconf or source patches.

Manual first check from the project root:

```sh
set -o pipefail
make V=1 toolchain:python27.stage 2>&1 | tee /tmp/uports-python2718-stage.log
```

Linux stage and generator runtime checks passed. The json/struct round trips,
child interpreter, staged stdlib paths and absence of unversioned commands
were checked in /tmp/uports-python2718-runtime.log. Optional SSL/database/
readline/compression modules were omitted by upstream detection; this is not
a full Python runtime acceptance. Installation/package reuse checks are
complete on Linux for the current generator scope. Cached reinstall reused the
compatible package without compilation; restored json/struct/subprocess and
prefix/helper checks passed in /tmp/uports-python2718-restored.log.
The actual Linux manifest contains4653 entries. macOS remains pending. Inspect
loader and stdlib lookup separately for copied deployments; current acceptance
does not establish copied-profile relocation.

Source: https://www.python.org/downloads/release/python-2718/
