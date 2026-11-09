# Native gettext prerequisite

GLib2.56.4's GLIB_GNU_GETTEXT macro requires a working msgfmt even when libc
already provides gettext functions. The first native GLib configure failed
because msgfmt was absent; its error text misleadingly emphasizes library
support. This is a missing declared build dependency, not a libc failure.

GNU gettext0.22.5 is a host compatibility choice under the agreed prerequisite
policy, not an MIT-prescribed release. Build from the official release archive,
with bundled libunistring and static internal support libraries. Java, Emacs,
libasprintf and OpenMP integration are disabled; shipped manuals are used.
The shared Info index is omitted to coexist with other native packages.
No platform manifest or source patch is fabricated. macOS is unvalidated.

The first build exposed an inherited deployment DESTDIR affecting gettext's
internal libtextstyle header-install rules. Headers went below local/home/...
instead of the build tree. The recipe clears DESTDIR only on the native build
make invocation; normal install still uses STAGEDIR. Existing redirected output
is preserved, not treated as installed package content. Retry needs no clean
or reconfigure because the real build header target remains absent.

The project installs this native build tool under DESTDIR/usr/local. GLib
declares msgfmt:devel/gettext explicitly; provider PATH selects the installed
tool. It is not a target compiler dependency or part of the target sysroot.

```sh
set -o pipefail
make V=1 toolchain:gettext.stage 2>&1 | tee /tmp/uports-gettext0225-stage.log
```

After stage, compile a real PO catalog and inspect its translated message,
record actual staged files, then validate install/reuse. Retry GLib afterward
without cleaning its source or bypassing configure tests.

Linux stage,2226-entry manifest, install and cached removal/reinstall passed.
Staged and restored msgfmt compile/check a real PO catalog, msgunfmt roundtrips
it, and Python2 GNUTranslations returns the expected translated message.
msgfmt loads only system libc/libm with loader overrides unset. Logs
/tmp/uports-gettext0225-*.log. macOS and copied deployments remain unvalidated.

Source: https://ftp.gnu.org/gnu/gettext/gettext-0.22.5.tar.xz
