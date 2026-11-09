# GCC JOS configurations

| Flavor | GCC release/source | Binutils | Native math libraries | Default PREFIX |
| --- | --- | --- | --- | --- |
| mit2018 | 4.6.4, gcc-core tar.bz2 | mit2018, 2.21.1 | GMP 5.0.2 / MPFR 3.1.2 / MPC 0.9 | /i386-jos-elf |
| jos12 | 12.5.0, full tar.xz | jos12, 2.46.1 | Modern compiler-group defaults | /i386-jos-elf-gcc12 |

Both configurations run on the host and generate i386-jos-elf C code plus
target libgcc. Native/latest, RISC-V and Linux/glibc bootstrap configurations
remain separate work. The historical version follows the
[project reproduction contract](../11-CROSS-TOOLCHAIN.md); the newer
configuration preserves the earlier GCC 12 experiment.

## Source and configure choices

mit2018 uses MIT's gcc-core-4.6.4.tar.bz2 archive. DISTNAME names that archive,
while WRKSRC explicitly names its gcc-4.6.4 extracted directory. Native GNU89
matches the historical host C dialect. Configure uses --enable-languages=c,
--disable-werror, --disable-libssp, --disable-libmudflap, --with-newlib and
--without-headers. Do not pass newer-only GCC options to this release.

jos12 retains the previously prepared GCC 12.5 configuration and its optional
feature restrictions. GCC 13 removed STABS output; the earlier GCC 14 trial
linked JOS with empty STABS placeholders. GCC 12 emits actual function/line
records, but remains a newer-profile experiment, not historical reproduction.

GCC 4.6's source confirms the build interfaces used here: --with-build-time-tools,
HOST_GMPLIBS/GMPLIBS and separate target flags. Both configurations use release
sources; no download_prerequisites or implicit native dependency vendoring.

## Native and target dependencies

Historical dependencies explicitly select @mit2018 for GMP, MPFR, MPC and
Binutils. jos12 selects Binutils@jos12 and @default math libraries. Project
policy selects one compiler profile per invocation:

```sh
make TOOLCHAIN_PROFILE=mit2018 toolchain:gcc@mit2018.stage
make TOOLCHAIN_PROFILE=jos12 toolchain:gcc@jos12.stage
```

The selector controls the native provider group as well as the GCC root
configuration. Historical math instances live in toolchain; modern defaults
come from compiler. Mismatched GCC requests or GCC-only project Flavor
overrides are rejected before execution. Both profiles can remain installed
in their separate roots. Building both GCC profiles in one invocation is
outside this project policy; generic framework Flavor dispatch is unchanged.

Provider exports supply actual include/library directories. Legacy injection
based on the consumer PREFIX is disabled. Pre-configure locates static native
libmpc.a, libmpfr.a and libgmp.a from those directories and fails if absent.
HOST_GMPLIBS/GMPLIBS link these archives in dependency order. This is an explicit
host-link packaging choice to avoid development-library runtime paths; the
native library recipes still build shared and static libraries with historical
versions and their reviewed defaults. Configure verifies headers and versions.

CONFIGURE_TARGET selects generated code, while CROSS_COMPILE stays unset for
native compiler production. --with-build-time-tools points at the selected
profile's target Binutils directory without baking --with-as/--with-ld
workspace paths into installed compiler configuration. PREFIX is the runtime
root; DESTDIR is deployment placement. Build objects stay outside source.

Build all-gcc before all-target-libgcc. Stage install-gcc before
install-target-libgcc. No target libc, Linux headers or newlib installation is
required. --with-newlib controls libgcc assumptions. Target flags are separate
from native include/library flags. MAKE_ENV clears the inherited host CPP so
target subconfigures choose the new compiler's preprocessor. The generated
shared Info index is omitted; compiler manuals remain packaged.

## Preserved work and acceptance

The earlier GCC 12 work, recipe/report and manifest are preserved in
work.toolchain.flavor-mit2018.gcc-12.5.0. The former manifest is the jos12 input
candidate; a fresh stage at the new prefix must validate it. GCC 14's source,
cache/build evidence and checksum remain archival, not an active Flavor.
Source caches and development trees are not deleted by this migration.

Adding named default Flavors to native prerequisite recipes changed their
source identity. A jos12 validation may need to prepare modern default-Flavored
artifacts; earlier unflavored cache evidence is not automatically equivalent.
Do not infer a rebuild is required merely from missing local markers: inspect
the package compatibility result and actual requested configuration.

Historical GCC 4.6.4 Linux stage, 276-entry manifest, installation and cached
removal/reinstall passed. Installed and copied profiles locate their own tools
and libgcc, link ELF32/i386 division helpers without undefined symbols and emit
STABS function/line records. cc1 loads only system libc. The original profile
remained installed during the copy check. Logs are /tmp/uports-gcc464-*.log.

The unmodified JOS checkout at a56269d4beefc7d0b3672180aa46c654cfb63af4
built kernel.img with real kernel STABS records; boot block382/510 bytes.
Evidence: /tmp/uports-jos-gcc464-{build,stabs}.log. The linker warned that .bss
changed to PROGBITS. Kernel boot and debugger behavior remain later acceptance.
jos12's new prefix and macOS remain unvalidated.

## Manual validation

From the project root:

```sh
set -o pipefail
make TOOLCHAIN_PROFILE=mit2018 V=1 toolchain:gcc@mit2018.stage \
  2>&1 | tee /tmp/uports-gcc464-stage.log
```

After successful stage, record its actual platform manifest before install.
Require -dumpmachine=i386-jos-elf and a libgcc path inside the selected profile.
Compile/link freestanding 32-bit C with volatile 64-bit division and
-nostdlib -lgcc; inspect ELF32/i386 and absence of unresolved symbols. Inspect
native cc1/plugin dependencies and repeat from a copied complete profile.
Package removal/reuse needs separate acceptance.

Build the pinned JOS lab from a fresh checkout using the selected GCCPREFIX.
Require real STABS function/line records beyond linker placeholders; report
any compatibility changes explicitly. Kernel boot and in-kernel debug lookup
remain later MIT QEMU/GDB acceptance. Do not run target code as a host program.

References: [MIT 2018 tools](https://pdos.csail.mit.edu/6.828/2018/tools.html),
[GCC 4.6 releases](https://gcc.gnu.org/gcc-4.6/),
[GCC 12 releases](https://gcc.gnu.org/gcc-12/),
[STABS removal](https://gcc.gnu.org/gcc-13/changes.html).
