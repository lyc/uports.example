# GNU Binutils target profiles

One recipe provides independently pinned releases and configurations:

| Flavor | Release | Output target | Default PREFIX |
| --- | --- | --- | --- |
| default | 2.47 | Native host | /usr/local |
| mit2018 | 2.21.1 | i386-jos-elf | /i386-jos-elf |
| jos12 | 2.46.1 | i386-jos-elf | /i386-jos-elf-gcc12 |
| mit2026 | 2.47 | riscv64-unknown-elf | /riscv64-unknown-elf |

All executables run natively. mit2018 reproduces MIT's listed historical
release, not its optional newer-version allowance. jos12 preserves the newer
Binutils configuration in a distinct root, with the same i386-jos-elf executable
prefix. Its corresponding GCC12 configuration is separate work. MIT 2026
requires RISC-V Binutils but does not prescribe 2.47: that is our selected pin.
End-to-end JOS/xv6 compatibility requires later compiler/lab acceptance.

Sources: [MIT tools](https://pdos.csail.mit.edu/6.828/2018/tools.html),
[MIT 2026 tools](https://pdos.csail.mit.edu/6.828/2026/tools.html),
[GNU releases](https://ftp.gnu.org/gnu/binutils/).

## Configuration and paths

Configure/build/install use the separate `WRKDIR/build` directory.
CONFIGURE_TARGET selects generated-code target; CROSS_COMPILE stays unset.
The recipe rejects a target inconsistent with its selected Flavor. Historical
2.21.1 uses the exact GNU tar.bz2 release, --disable-werror as documented by MIT,
and GNU89 for the host compiler. Other configure defaults remain those of the
historical release; newer-only options are not passed to it. The modern
configurations retain bundled zlib/static libraries with optional NLS, CTF,
gprofng, zstd and debuginfod disabled. MAKEINFO=true avoids rebuilding shipped
release documentation. No target libc or GMP/MPFR/MPC dependency is needed.
The historical package omits the generated shared Info index, preserving the
manual files without assigning ownership of a global documentation index.

The project registers toolchain alongside compiler. Legacy group introspection
requires a group PREFIX default. Existing per-instance environment exclusion
omits that default for Binutils, allowing the recipe to select each Flavor's
prefix. Explicit caller PREFIX remains supported through recursive make.
The project selects mit2018 for unqualified Binutils requests; explicit
@default selects the native build. PREFIX is runtime placement; DESTDIR is
deployment placement and is not embedded in the configure prefix.

```text
work:    feeds/devel/binutils/work.toolchain.flavor-mit2018/
source:  local/src/binutils/flavor-mit2018/binutils-2.21.1/
stage:   work.toolchain.flavor-mit2018/stage/i386-jos-elf/
install: local/i386-jos-elf/bin/i386-jos-elf-{as,ld,objdump,...}
```

## Manual acceptance

From the parent project:

```sh
make V=1 toolchain:binutils@mit2018.fetch
make V=1 toolchain:binutils@mit2018.checksum
make V=1 toolchain:binutils@mit2018.stage
make V=1 toolchain:binutils@mit2018.generate-plist
```

Each Flavor's static manifest comes from its reviewed stage inventory.
Profile Flavors select pkg-plist.<flavor>.<os>-<arch>; native uses ordinary
platform pkg-plist lookup. Version-specific distinfo.2.21.1, distinfo.2.46.1
and distinfo.2.47 prevent makesum for one release from replacing another's
entries. After recording the actual historical manifest, validate
package/install/deinstall/reinstall and repeated invocation without rebuilding.
Successful staging alone is not package acceptance.

Use the installed tools to assemble/link a real object:

```sh
tools="$PWD/local/i386-jos-elf/bin"
checkdir=$(mktemp -d /tmp/uports-jos-binutils.XXXXXX)
printf '.text\n.globl _start\n_start:\n  nop\n' > "$checkdir/start.s"
"$tools/i386-jos-elf-as" --32 -o "$checkdir/start.o" "$checkdir/start.s"
"$tools/i386-jos-elf-ld" -m elf_i386 -e _start -Ttext 0x100000 \
  -o "$checkdir/start.elf" "$checkdir/start.o"
"$tools/i386-jos-elf-readelf" -h "$checkdir/start.elf"
"$tools/i386-jos-elf-objdump" -d "$checkdir/start.elf"
```

Require ELF32, Intel 80386, entry 0x100000 and the nop instruction. Inspect host
runtime dependencies with ldd (Linux) or otool -L (macOS), and verify relocated
profile tool execution. Native macOS needs separate compiler/GNU Make checks.
Earlier 2.46.1 Linux checks under the former /i386-jos-elf root passed, with
116 payload files matching stage and installed/copied tools producing ELF32
i386 output. That work and its report/manifest remain preserved in
work.toolchain.flavor-mit2018.binutils-2.46.1. The former manifest is now the
jos12 input candidate; it must match a fresh stage at its distinct prefix before
claiming jos12 acceptance.

Historical 2.21.1 Linux stage succeeds. Staged tools assemble an ELF32 i386
object, create an archive and resolve its helper symbol in a linked executable
at entry 0x100000. Disassembly is correct with no unresolved symbols; the target
binary is not executed. Native as/ld need only system libc. The actual
202-entry manifest passes package inventory checking. All 147 installed regular
files and symlinks match stage. Installed/copied profiles pass the same archive
symbol link, with explicit -L for the sample archive; automatic relocated SDK
library lookup is not established by this check. Manual removal/reinstall reuses
the cached package without compilation; restored tools work and historical math
registrations remain intact. Historical Binutils Linux component checks pass;
full JOS compiler/boot/debug acceptance remains separate.

The previous 2.46.1 tests also showed that without a sysroot, LD's library search
retains its configured absolute prefix. A copied profile needed explicit -L for
libraries placed in its target-library directory; a real archive/symbol link
passes with that path. This does not require target libc or a new sysroot for
JOS. GCC's automatic libgcc lookup remains part of compiler acceptance; basic
tool relocation does not establish automatic library discovery for the SDK.
Earlier 2.46.1 package removal/reinstall passed: registration was renewed
while build/stage/package cookies stay unchanged, and all reinstalled payload
files matched stage. Historical 2.21.1, new-prefix jos12 and native macOS checks
remain pending; earlier newer-version results do not satisfy historical gates.
