# Historical JOS GDB

GDB 7.3.1 is the debugger release listed by MIT's 2018 tools instructions.
The mit2018 Flavor targets i386-jos-elf and installs host-running tools under
PREFIX=/i386-jos-elf, placed beneath the caller's DESTDIR. It does not require
the target compiler or native GMP/MPFR/MPC libraries to build.

Configure retains MIT's --target, --program-prefix and --disable-werror.
The recipe adds --without-python because this release's optional Python
integration predates current Python 3; Python scripting is outside this initial
historical debugger acceptance. Native C uses GNU89. Generated release files
and shipped documentation are used with MAKEINFO=true. Build objects live
outside source; the shared Info index is omitted from the package.

The bundled support components also install libiberty.a, BFD/configure/standards
manuals and BFD/opcodes translation catalogs. These collide with Binutils in a
shared profile. GDB's post-install hook omits these files from its stage while
retaining debugger manuals, data and translations. GDB links its bundled
support code during build; the omitted files are not debugger runtime inputs.
The first install correctly failed ownership checks; no overwrite is allowed.

Native ncurses and zlib are explicit library dependencies. Configure requests
curses and zlib support, uses bundled readline, disables Python and Expat, and
disables NLS to avoid an undeclared gettext dependency. Expat was absent in the
accepted historical build; XML target descriptions remain outside this recipe's
enabled features. Provider policy selects system or permitted uports instances;
these are host libraries, not i386 target libraries. Record actual configuration
and loader dependencies during validation. No modern-host
source patches or warning suppression have been added. macOS is unvalidated.
Only mit2018 is declared: selecting another project profile must not silently
substitute this historical debugger.

Use selected provider paths without the generic target-prefix library flags.
The profile contains Binutils' older libiberty.a; putting that directory ahead
of GDB's bundled libraries makes the BFD ELF probe fail on filename_ncmp and
silently builds a debugger without ELF symbol support. The export runtime check
exposed this; do not force the configure probe to succeed or ignore the failure.
PACKAGE_BUILD_ID=i386-jos-elf-native-providers rejects affected cached binaries.

Manual stage from the project root:

```sh
set -o pipefail
make V=1 toolchain:gdb@mit2018.stage \
  2>&1 | tee /tmp/uports-gdb731-stage.log
```

After stage, record the actual platform manifest, inspect native loader paths,
and use batch GDB to load the accepted JOS kernel, resolve i386 architecture,
symbols and source lines. Installed/copied tools and package removal/reuse need
separate checks. Remote breakpoints and stepping require the later MIT QEMU
integration; a successful symbol-file load alone does not prove remote support.

Reference: https://pdos.csail.mit.edu/6.828/2018/tools.html

## Linux validation

The explicit-dependency configuration disables NLS. Its completed Linux stage
contains 42 entries; the manifest omits the former locale directories and
matches that inventory. After correcting native library search paths, the BFD
ELF probe succeeds, packaging/install pass, and installed GDB resolves the JOS
kernel's i386_init source line. The relocated distribution also passes the
QEMU remote breakpoint/register/step check. Evidence:
/tmp/uports-gdb-export-fixed-install.log and
/tmp/uports-jos-aggregate-export.log. The results below describe the earlier
build, before explicit dependency declarations.

Stage, 77-entry manifest, installation and cached removal/reinstall passed.
Restored GDB resolves i386_init to kern/init.c line24 in the accepted historical
JOS kernel. Staged batch checks identify i386 and disassemble the function.
Native loader dependencies resolve to system ncurses/tinfo, libm and libc.
Expat was not discovered; remote target-description support must be reviewed
during MIT QEMU integration. Copied-profile batch checks also passed with
LD_LIBRARY_PATH/LD_PRELOAD unset: i386 architecture, source line24 and function
disassembly. Copy retained at /tmp/uports-gdb731-copy.3Q2AQ4/profile; original
installation remained present. This establishes copied executable symbol
reading, not independence of every installed data-file lookup. Remote debugging
and macOS remain pending. Logs: /tmp/uports-gdb731-*.log.
