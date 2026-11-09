# Native GLib prerequisite

GLib 2.56.4 supplies GLib/gthread for the pinned MIT QEMU. It is a December2018
release, selected for host compatibility, not prescribed by MIT. We reproduce
the named MIT tools and pinned emulator source, not an exact 2018 host image.
This release retains Autotools and bundled PCRE, avoiding a new Meson bootstrap
or separate PCRE1 port for this historical consumer.

Dependencies: versioned native Python2.7, gettext tools, pkg-config, libffi and
zlib. GLIB_GNU_GETTEXT requires msgfmt even with libc gettext functions; the
first configure failed because that build tool was missing. The project
selects toolchain instances under DESTDIR/usr/local and retains the existing
compiler-group libffi/zlib instances. Provider exports supply include/library
and pkg-config paths; no historical GMP/MPFR/MPC environment is needed.
Optional libmount/libelf/SELinux/FAM, generated manuals and installed/always-built
tests are disabled. This does not remove GLib's threading or object/I/O APIs.
Python remains a runtime dependency for installed code-generation scripts.

After prerequisite validation:

```sh
make V=1 toolchain:glib20.stage 2>&1 | tee /tmp/uports-glib2564-stage.log
```

Linux stage,1082-entry manifest, install and cached removal/reinstall passed.
Staged/restored thread and GObject generic-closure/libffi invocation pass with
GLib2.56.4 and native libffi8. Logs /tmp/uports-glib2564-*.log. macOS and copied
deployment remain pending. Record actual staged files only after
success, inspect .pc prefixes and native loader paths, compile/run a real
GLib/thread consumer, then validate install and cached reuse. Packaging and
runtime relocation of the complete emulator profile remain acceptance work.
No source patches or generated platform manifests have been added.

Source: https://download.gnome.org/sources/glib/2.56/
