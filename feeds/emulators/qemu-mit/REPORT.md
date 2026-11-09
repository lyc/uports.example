# MIT JOS QEMU

The mit2018 Flavor pins mit-pdos/6.828-qemu at
d531b1b1d6b7696dfd9695c1d560e3df53e615c5, dated2018-09-07, VERSION2.3.0.
MIT's tools page names this fork, not a commit. This explicit pin includes the
MIT page-table inspection, triple-fault diagnostics and e1000 debugging changes.
It is not replaced by upstream QEMU. DISTVERSION also records the fork date.
GitHub archives are the default; USE_SCM=git selects the same pinned source.

Host prerequisite policy, agreed with the user: reproduce MIT's named compiler,
debugger and math releases and the pinned emulator source, while documenting
host libraries/interpreters as compatible build choices. Python2.7.18,
GLib2.56.4 and Pixman0.34.0 are not versions prescribed by MIT and do not claim
an exact 2018 host environment. Existing native libffi/zlib/pkg-config recipes
are reused. GNU gettext0.22.5 supplies GLib's required msgfmt build tool and is
also a host compatibility choice. Python3 cannot replace Python2 without a separate
porting change. No such change or compatibility source patch is included.

Project prerequisites install under DESTDIR/usr/local; emulator output defaults
to DESTDIR/i386-jos-elf. BUILD_DEPENDS selects Python2.7 and pkg-config;
LIB_DEPENDS selects GLib, Pixman and zlib. The resolver includes libffi and build
tools transitively. Native include/lib/pkg-config/loader paths come from provider
exports. PKG_CONFIG_SYSROOT_DIR applies the common native deployment root to
the dependencies' runtime-prefix .pc files. Do not set CROSS_COMPILE or
CONFIGURE_TARGET: --target-list selects emulated machines, not a cross compiler.

This old custom configure uses QEMU_CFLAGS for probes and does not consume
exported CFLAGS/CPPFLAGS there. Pass native provider/compiler flags through
--extra-cflags explicitly. The first zlib probe otherwise failed to find the
installed header despite correct library search paths; no zlib reinstall or
source patch is needed. LDFLAGS is already consumed by configure.

Build i386-softmmu and x86_64-softmmu outside source. Disable KVM/werror per MIT,
and select headless operation without SDL/GTK/VNC/curses. Optional tools, guest
agent, device-tree support and generated documentation are omitted from this
initial x86 scope. Shipped pc-bios firmware remains installed; ROM submodule
rebuilds are unnecessary. System Pixman means no Pixman submodule fetch.

Validate native prerequisites one by one before the emulator:

```sh
set -o pipefail
make V=1 toolchain:qemu-mit@mit2018.stage \
  2>&1 | tee /tmp/uports-qemu-mit-stage.log
```

Linux/macOS builds are pending. No manifest exists before real stage. Acceptance
must inspect native loader/firmware lookup, install/package reuse, JOS boot,
GDB7.3.1 connection/registers/breakpoint/step and copied deployment behavior.
GDB currently lacks Expat: actual remote behavior decides whether it must be
enabled. Native GLib/Pixman are shared runtime dependencies, unlike GCC's static
math link; export packaging must carry or otherwise provide them. A copied
emulator alone is not a complete exported bundle.

After mit2018 QEMU acceptance, revisit the deferred project
CROSS_COMPILE-to-TOOLCHAIN_PROFILE mapping. No mapping is implemented here.

## Linux acceptance

Stage,91-entry manifest, installation and cached removal/reinstall passed.
Staged/restored i386 emulator boots the pinned historical JOS image to its
kernel monitor. Both binaries report2.3.0; x86_64 guest execution is not claimed.
Boot scripts scope native shared-library paths and explicit firmware paths,
use a snapshot disk and accept timeout only after the JOS welcome message.
The first stdio serial setup produced no output; file-backed serial passed.

Installed GDB7.3.1 connects to installed QEMU, hits a hardware breakpoint at
i386_init, reads registers/source-line metadata and advances one instruction.
Expat was not required for this i386 remote exchange. A missing source listing
in the first script run was its directory search configuration; future runs
explicitly add the retained JOS source directory. Logs
/tmp/uports-qemu-mit-{stage,install,reinstall,restored,gdb}.log; remote artifacts
/tmp/uports-qemu-mit-gdb.2chm22pp. Separate package-assembled runtime bundle
passed: both binaries resolve zlib/GLib/Pixman from its temporary root and i386
boots JOS with that root's firmware/library paths. Artifacts
/tmp/uports-qemu-mit-bundle.ITmAEc, log /tmp/uports-qemu-mit-bundle.log. Original
installation remained present; explicit runtime paths were supplied. This is
not an automatic exporter/launcher or cross-host portability claim. Native
shared dependencies remain required. macOS is unvalidated.

References:
- https://pdos.csail.mit.edu/6.828/2018/tools.html
- https://github.com/mit-pdos/6.828-qemu/commit/d531b1b1d6b7696dfd9695c1d560e3df53e615c5
