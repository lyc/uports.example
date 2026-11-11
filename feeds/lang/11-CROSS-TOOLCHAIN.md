# Cross-Toolchain Architecture And Design Proposal

## Status And Document Ownership

This report records design rationale, interface contracts and implementation
results. Proposed behavior is identified explicitly; describing a contract does
not establish manual acceptance or completed toolchain support. Public names,
target syntax, origins and paths labeled proposed require implementation
review before becoming supported interfaces.

[11-CROSS-TOOLCHAIN-TASKS.md](11-CROSS-TOOLCHAIN-TASKS.md) owns implementation
order, task checkboxes, acceptance and commit/evidence mapping. [framework handoff](../../make/uports/HANDOFF.md)
owns the changing checkpoint, next action and blockers. This report owns
design rationale and supported-interface descriptions as implementation lands.
Keep work-item numbers, temporary restrictions, validation progress and commit
history in the tracker. Update results after acceptance without duplicating
the execution history here.

The proposal extends [group/instance design](../../make/uports/docs/10.1-GROUP.md) and
[dependency handling](../../make/uports/docs/10.4-DEPENDENCY.md), preserving the
[fetch architecture](../../make/uports/docs/01-FETCH.md) and local cache contract.

## Objectives And Profiles

Generate tools from maintained port recipes with pinned sources, inspectable
dependencies and isolated output identities. A generated compiler must be
validated by building and running its intended software, not just by printing
its version. Linux is the initial build host; macOS acceptance is separate.

| Profile | Generated-code target | Target libc | Runtime/debug environment |
| --- | --- | --- | --- |
| MIT 2018 JOS | i386-jos-elf, 32-bit x86 ELF | None | MIT-patched x86 QEMU and target-capable GDB |
| MIT 2026 xv6 | riscv64-unknown-elf, 64-bit RISC-V | None | Upstream QEMU >= 7.2 and target-capable GDB >= 8.3 |
| Trixie ARM64 capability | aarch64-linux-gnu | glibc | Real ARM64 or QEMU user emulation for validation |

QEMU is a required generated deliverable for each MIT profile. Keep the
historical MIT-patched emulator independent from upstream QEMU. Both must be
explicitly selectable; installing one must not silently change the other
profile's emulator. The lab source revision is also pinned for acceptance.

### JOS Versions And Reproduction Policy

The accepted objective is to reproduce the releases listed by MIT's 2018
instructions. Their permission to use newer releases is not the acceptance
baseline for `mit2018`. A separate `jos12` configuration preserves the newer
toolchain experiments without representing them as historical reproduction.

| Component | Historical `mit2018` | Preserved `jos12` |
| --- | --- | --- |
| Binutils | 2.21.1, Flavor mit2018 | 2.46.1, Flavor jos12 |
| GCC | 4.6.4, gcc-core archive, Flavor mit2018 | 12.5.0, Flavor jos12 |
| GMP | 5.0.2, Flavor mit2018 | 6.3.0, Flavor default |
| MPFR | 3.1.2, Flavor mit2018 | 4.2.2, Flavor default |
| MPC | 0.9, Flavor mit2018 | 1.4.1, Flavor default |
| GDB | 7.3.1 | Separate debugger selection still requires review |
| QEMU | MIT-patched fork with a reviewed historical revision | No extra emulator configuration implied |

This matrix defines the accepted profile contract, not an assertion that every
configuration is implemented or accepted. The tracker owns migration order and
per-component evidence. Preserve source trees, cached packages, manifests and
acceptance logs when reassigning newer configurations to jos12. GCC 14.4 remains
exploratory evidence, not an additional active Flavor.

Use one recipe per component with versioned distinfo and selected manifests.
Binutils retains its native default and RISC-V mit2026 configurations. Historical
math library Flavors are additions to the existing recipes; modern default
configurations remain available. No separate jos12 math Flavors are needed:
GCC jos12 explicitly selects modern @default math instances. They run natively
in the toolchain group under its ordinary /usr/local prefix.
Historical GCC selects all three @mit2018 instances, with MPFR selecting
historical GMP and MPC selecting historical GMP/MPFR.

Select components directly through their Flavors, for example
toolchain:gcc@mit2018.install or toolchain:gcc@jos12.install. There is no separate
profile variable or mismatch guard. Recipes own their versions, output targets,
dependencies and PREFIX defaults. Project group settings provide default
Flavors for unqualified and aggregate commands; explicit Flavors take precedence.
Current Binutils/GCC aggregate defaults remain mit2018. Historical and modern
math remain available independently, and both installed compiler roots can
coexist. Existing provider policy chooses compatible native group instances.

Native/default producer variants, alongside i386, RISC-V and AArch64 variants,
are part of the remaining recipe work. Cross-toolchain executables run on the
host; CONFIGURE_TARGET describes their output target. Do not use CROSS_COMPILE
to build these host executables. Target headers and glibc require their own
sysroot/bootstrap-stage context.

Historical math instances run natively in the toolchain group, under the
mit2018 profile root. They support the host-running compiler and are not target
JOS libraries: do not set CONFIGURE_TARGET=i386-jos-elf or CROSS_COMPILE on them.
Their native include/lib paths must come from the selected provider instances.
Keep them distinct from compiler-group modern libraries and from target libgcc.
Follow MIT's native library configuration; additional static-only library build
policies are separate decisions. The GCC recipes explicitly link the selected
native math archives to avoid development-library runtime paths. This host-link
packaging choice leaves the math recipes' shared and static outputs available;
it does not impose a static-only library build or change target libgcc flags.

Both GCC configurations are C-only freestanding compilers for i386-jos-elf.
Build all-gcc before all-target-libgcc, then stage install-gcc before
install-target-libgcc. No target libc, Linux headers or newlib installation is
required. Keep native dependency flags separate from target flags and let
subconfigures select the correct compiler's preprocessor. Review options against
each selected release instead of copying newer flags into GCC 4.6.4.

GCC 13 removed STABS emission. The GCC 14 trial linked the lab but lacked its
debug records; the staged GCC 12 trial emits actual STABS function/line records.
Historical acceptance still requires the GCC 4.6.4 lab build, real kernel STABS
records, boot and debugger checks. Newer trial results do not satisfy that gate.

The development roots are local/i386-jos-elf for mit2018 and
local/i386-jos-elf-gcc12 for jos12, derived from DESTDIR/PREFIX. Both use the
i386-jos-elf executable prefix. Separate roots prevent their identically named
tools and versioned runtime files from overwriting one another. GCC jos12 must
depend explicitly on devel/binutils@jos12.

The official JOS lab1 acceptance revision is
`a56269d4beefc7d0b3672180aa46c654cfb63af4`. The previously selected MIT QEMU
pin is `d531b1b1d6b7696dfd9695c1d560e3df53e615c5`, the MIT fork tip dated
2018-09-07, reporting upstream version2.3.0. Its preceding MIT commits add page
table inspection, triple-fault diagnostic behavior and e1000 debug controls.
The page names the patched repository, not an exact commit: this pin is our
explicit reproducible selection, not a commit prescribed by the course.
The source requires Python2.4+ and rejects Python3; host GLib/gthread, Pixman
and zlib development files are prerequisites. Firmware binaries are shipped
under pc-bios; rebuilding ROM submodules is not part of initial x86 acceptance.
Host prerequisite policy: preserve the named MIT component versions and pinned
emulator source; host prerequisites are documented compatibility choices, not
an exact reproduction of the 2018 host environment. Python2.7.18 (2020),
GLib2.56.4 (December2018) and Pixman0.34.0 are selected for this build and are
not prescribed by MIT. New parent recipes lang/python27, devel/glib20 and
x11/pixman reuse native libffi/zlib/pkg-config support. They install below
DESTDIR/usr/local; the emulator remains below its profile PREFIX. Its shared
native libraries must be supplied when exporting or relocating the emulator.
Record source pins,
checksums and every modern-host compatibility patch. Do not silently replace
historical components or change generated target behavior to get a host build.

### Public Toolchain And Development Interface

The agreed parent-project interface uses CROSS_COMPILE as the public selector
for both producing a toolchain and consuming it in development. Tuple-named
aggregate targets map directly to component Flavors and scoped environments,
without requiring a second profile selector.
The historical tuple aggregate and optional Linux distribution export are
implemented in the parent Makefile. Consumer environment resolution and JOS
module integration remain follow-up work; the consumer command below is still
the agreed interface rather than completed application integration.

```sh
make i386-jos-elf
make EXPORT=yes i386-jos-elf
make CROSS_COMPILE=i386-jos-elf
```

The aggregate i386-jos-elf target selects historical mit2018 and installs
Binutils, GCC, GDB and MIT QEMU through the existing dependency planner.
Prerequisites remain declared dependencies. Producer builds must not inherit
the consumer CROSS_COMPILE; CONFIGURE_TARGET still selects compiler output.
Preserve DESTDIR/PREFIX installation control rather than hardcoding a host root.

### Historical JOS Distribution

The normal aggregate installs components below DESTDIR/i386-jos-elf (local by
default). EXPORT=yes additionally assembles a separate distribution; it does
not alter installed binaries, group layouts or component configurations. Other
EXPORT values than yes/no are rejected. Consumer CROSS_COMPILE is cleared in
the producer invocation. Component lifecycles run with one job because the
current package/stage prerequisites are not safe under parallel installation.
Development source preparation and compatible package
reuse retain their existing framework behavior.

The operational entry point is make/toolchain/export.py. The exporter supports
Linux ELF hosts and requires Python3, readelf, ldd, make, git and Debian package
metadata for host libraries. EXPORT=yes adds host:patchelf.install to the producer
invocation. The project selects feeds/sysutils/patchelf in host; framework base
group membership is unchanged. Its release source includes generated configure
files, so only a native C/C++17 compiler and ordinary Make tools are needed.
PATCHELF defaults to $(host_DESTDIR)$(host_PREFIX)/bin/patchelf (normally
local/usr/local/bin/patchelf); it may still name an explicit executable.
Patchelf is an export-only build tool and is not copied into the distribution.
Darwin exports are rejected, not silently treated as ELF distributions.

Keep the export machinery separate from target-specific composition and
validation. The parent Makefile remains the build/install interface; the Python
exporter operates on installed packages and does not resolve or build recipes.

| Module under make/toolchain | Responsibility |
| --- | --- |
| export.py | Input preparation, assembly order, invocation of exported validators and publication gate |
| packages.py | Package inventory/copying, notices, shipped validators/source, metadata/checksum/archive publication |
| linux.py | Host ELF dependencies, private runtime libraries, relative paths and Linux runtime notices/data |
| common.py | Command execution, clean environment and missing validation-source preparation through the framework SCM helper |
| profiles/jos.py | Historical composition, host-tool paths, source pin/default location and shell-validator configuration |
| validate.sh | Template installed at the distribution root as the standalone acceptance entry point |
| validators/*.sh | Shared shell helpers, runtime/compiler checks and real JOS build/boot/GDB acceptance |

Only i386-jos-elf is implemented. RISC-V and ARM64 will add composition/source
settings in profile modules and real-world checks in separate shell scripts,
then register their tuple and expose the parent make target. Reuse the
package and Linux runtime modules; do not copy the JOS exporter or make shared
code depend on JOS kernel names, firmware settings or lab commits. ARM64 sysroot
contents remain selected package payloads; their bootstrap/recipe implementation
is separate work. Runtime relocation scans declared host-tool paths and private
runtime libraries, not the target sysroot. Host/target CPU equality must never
cause target libraries to be patched as host libraries. Additional host formats
need an explicit backend rather than
ELF assumptions in a target profile. No dynamic plugin/configuration mechanism
or placeholder support for unfinished profiles is introduced.

Without JOS_SOURCE, EXPORT=yes prepares the accepted MIT 2018 lab1 commit
a56269d4beefc7d0b3672180aa46c654cfb63af4 automatically under
DESTDIR/src/jos-validation/<commit>. The profile supplies the upstream repository
and pin. The existing framework SCM checkout helper provides bounded network
operations, locking, atomic publication and protection of existing trees. This
small validation-only project checkout uses SCM_CACHE=no, not a new mirror/cache
layout. Repeated exports reuse it. The first preparation requires network access.

JOS_SOURCE may select an existing clean checkout instead; explicit source paths
are checked, never created/reset by the exporter. Export copies the accepted
source and rebuilds that copy for validation; the input checkout is not modified.
No src/jos module or application integration is implied. Normal installation
does not prepare validation sources. Generated dist outputs are ignored by Git.
Host/source input checks run before installation and do not require patchelf
to exist yet. The normal producer invocation then installs the export tool
and components. The full export requires the resulting patchelf executable.

Distribution contents and acceptance:

- Copy only package-owned files from historical Binutils, GCC/libgcc, GDB and
  MIT QEMU, including firmware and debugger/compiler data. Native static math
  archives and headers are build prerequisites, not blanket profile exports.
- Resolve host ELF runtime dependencies recursively, preferring installed native
  provider libraries. Copy non-libc runtime libraries to private lib/runtime;
  reject conflicting libraries with the same loader name. Keep libc and the
  ELF loader as host prerequisites. Do not install libraries into system paths.
- Patch copied executables and libraries with relative $ORIGIN runtime paths.
  No global loader environment or ldconfig changes. Include terminal data under
  share/terminfo; terminal users can set TERMINFO to that directory.
- Include component recipes/distinfo, available patches, prepared source license
  files and host runtime copyright notices. DISTRIBUTION.json records component
  and library versions, original library checksums, validation and required
  GLIBC symbol versions; SHA256SUMS covers regular payload files except mutable
  validation-source Git administration files.
- Ship validate.sh plus the selected validators, profile.conf, checks.list,
  host-elf.list and pinned JOS source below share/toolchain/validation. Keep the
  JOS .git directory, course refs/history and MIT upstream origin; omit build
  outputs. Materialize an independent repository even when the construction
  input is a linked worktree or uses another object store. Formal checks use only
  the exported folder as input; no workspace, Python, Git, uports scripts or
  separate lab checkout is needed after distribution creation.
- Move the assembled profile to a different parent/name before testing. Clear
  workspace loader/compiler variables, check the runtime closure, compile/link
  freestanding ELF32 code with libgcc, build the real JOS lab, boot it using
  bundled firmware, and check a GDB breakpoint/registers/instruction step.
- Publish only after acceptance. EXPORT_DIR defaults to dist. Outputs use an
  immutable i386-jos-elf-linux-<arch>-<UTC timestamp> directory and matching
  .tar.xz archive, plus a separate .checks directory for validation evidence.
  Failed staging/log directories are retained; prior exports are not replaced.

### Standalone Shell Validation

After unpacking or moving the distribution:

```sh
sh /path/to/toolchain/validate.sh
sh /path/to/toolchain/validate.sh compiler
sh /path/to/toolchain/validate.sh runtime /tmp/new-runtime-checks
sh /path/to/toolchain/validate.sh jos /tmp/new-jos-checks
```

The default all mode runs the profile's ordered checks. Runtime verifies the
private shared-library closure; compiler checks version, target and internal
tool/libgcc paths. JOS compiles/links freestanding code with libgcc/STABS, builds
the shipped real lab source, boots QEMU and checks GDB breakpoint/registers/step.
Publication uses this same entry point against the relocated assembly, not a
separate Python implementation. Python retains distribution construction and
source-input preparation only.

JOS intentionally retains its MIT course context. The exported validation source
is a normal Git checkout at the accepted commit, with origin pointing to MIT.
It has its own objects/refs, not links to workspace Git metadata or alternates.
SOURCE_GIT in the profile selects this policy. Use host Git on a writable source
copy for course development; formal shell acceptance still does not require Git.
Mutable source/.git administration files are excluded from SHA256SUMS so index
refreshes and Git queries do not invalidate payload checks. Source content,
tools, libraries and validator scripts remain covered by the checksums.

Output defaults to a newly created /tmp directory, printed at startup. Explicit
output directories must be empty and outside the distribution. Validators clear
loader/compiler environment overrides, retain commands/statuses and stage logs,
and terminate their QEMU child on completion or failure. Published distributions
also verify SHA256SUMS before checks. The source is built in a writable output
copy; a read-only exported directory remains unchanged.

Validation also serves as a tutorial: exercised commands print before execution
with shell-quoted arguments and their working directory. This remains visible
when results are redirected to logs or captured for assertions. The C probe,
QEMU background command and generated GDB commands are displayed. Shared run,
start_background and show_commands helpers provide this behavior for later
profiles. Result logs and exit-status checks remain intact; tutorial text does
not enter captured compiler-path values.

The exporter streams the same tutorial output live into its console and
validation.log. Export-time command paths name the temporary assembly; rerunning
the published validate.sh displays the distribution's current path and fresh
output paths for direct study or reuse.

Full acceptance requires standard Linux host utilities: sh, coreutils including
timeout/readlink/sha256sum, awk, sed, grep, ldd, make, tar and Perl. Linux /proc
is used to identify QEMU's own localhost GDB socket without Python/netcat/socat.
The unused JOS hand-in UPSTREAM discovery is disabled during validation builds,
so the course Makefile does not require Git. No native host compiler is used by
the JOS checks. Compiler mode is a reusable preflight for other standalone
projects; those projects add their own build/run checks using the same exported
tools and shell helpers. Future toolchain profiles must ship all their validator
scripts and data with the distribution and keep architecture-specific checks
out of the exporter.

For an existing lab checkout:

```sh
make EXPORT=yes i386-jos-elf \
  JOS_SOURCE=/path/to/accepted/jos
```

An export tested on the build host is not proof of compatibility with older
libc releases, another architecture or macOS. Test on another machine within
the documented baseline before claiming cross-host distribution acceptance.
The first accepted Linux x86_64 export requires GLIBC symbols through 2.38;
it is not a distribution for arbitrary older Linux hosts.

Normalize the public selector's optional trailing dash and resolve it to the
installed executable prefix, including the final dash. Resolve matching debugger,
emulator, firmware and scoped native runtime-library paths from the same project
mapping. Missing tools should fail with an instruction to run make i386-jos-elf;
ordinary application builds must not silently start producing a toolchain.
The tuple maps explicitly to historical mit2018: jos12 shares the target tuple
and requires a distinct convenient selection if exposed later. Do not infer
compiler versions from the executable tuple alone.

Integrate the pinned MIT JOS checkout under src/jos with a module.mk following
the parent's module conventions. Preserve upstream build rules, translate the
resolved prefix to JOS GCCPREFIX and select matching QEMU/GDB explicitly.
Read src/AGENTS.md and src/HANDOFF.md before implementing that integration;
preserve unrelated application work and workstream boundaries. The source
checkout layout and explicit development goals will follow that inspection.

Implement and validate in three steps: aggregate target/project mapping;
development environment resolution; pinned JOS/module integration with parent
build, boot and remote-debug validation. This is parent integration work, not
a new generic Flavor or bootstrap-stage mechanism. No recipe commit, source
checkout, framework extension or implementation is implied by recording it.

Implementation and validation order: historical GMP, MPFR, MPC; historical
Binutils; GCC/libgcc; GDB; MIT QEMU; full lab boot/debug acceptance. Changes are
prepared one component at a time for user manual validation. Recipe notes live
beside the corresponding parent-owned ports.

The ARM64 objective is a C/C++ SDK comparable in purpose to Trixie's
crossbuild-essential-arm64 dependency set, not identical Debian packages.
The baseline includes Binutils, GCC/G++, libgcc, libstdc++, Linux UAPI headers,
glibc runtime/development files and a usable target sysroot. Exact versions,
patches, ABI/ISA settings and kernel baseline are selected before building.

Neither MIT profile needs target glibc or Linux UAPI headers. A host library
used by the compiler or emulator is distinct from a target runtime library.

## Proposed Port Boundaries

The accepted core inventory contains seven port definitions. Concrete recipe
versions and any additional prerequisite ports require later review:

| Port | Responsibility |
| --- | --- |
| devel/binutils | Target assembler, linker and binary inspection tools |
| lang/gcc | Compiler and selected target runtimes, including stage recipes |
| devel/gdb | Host-running debugger with selected target support |
| emulators/qemu | Upstream system/user emulation configurations |
| emulators/qemu-mit | Separately pinned MIT debugging fork |
| devel/linux-headers | Sanitized Linux UAPI headers for the target sysroot |
| devel/glibc | Target headers, startup objects, loader and C libraries |

Reuse these recipes across target configurations and bootstrap passes rather
than creating a separate GCC directory for every target/stage. GCC runtime
package splitting is a later explicit packaging decision, not an assumption
that a single GCC install can overwrite every stage safely.

Reuse suitable native GMP/MPFR/MPC providers. Audit QEMU and GDB dependencies,
including legacy Python requirements and pinned submodules, before fixing the
total port count. Missing prerequisites may require additional ports, subject
to scope review. JOS/xv6 validation checkouts are separate from package ports.

## Build, Host And Target Roles

| Role | Meaning | Initial example |
| --- | --- | --- |
| Build | Machine executing configure/build commands | Current Linux machine |
| Host | Machine executing the produced tool | Same Linux machine |
| Target | Machine/ABI executing compiler output | JOS x86, xv6 RISC-V or ARM64 Linux |

Existing CONFIGURE_TARGET handling is the starting point for producing cross
tools. Existing CROSS_COMPILE handling selects an already available toolchain
to compile target software. Do not set the latter merely because GCC emits
foreign code: that could try to build the compiler itself for the target.

GMP used by host-running GCC is a host dependency. glibc being built for ARM64
is a target dependency. Dependency exports must keep their include/library
paths and compiler commands separate. Host utilities used during a target
build, such as xv6's filesystem-image generator, must use the host compiler.

Project profiles own target selection, installation roots and provider policy.
Ports own supported configuration and dependency intent. Preserve existing
host system-or-host/bs restrictions; adding toolchains does not authorize a
global group-policy change. Canadian cross builds are deferred.

## Relationship Between Groups, Flavors And Bootstrap Stages

| Concept | Responsibility | Example |
| --- | --- | --- |
| Group | Shared build/install environment and provider policy | ARM64 toolchain environment |
| Port | Component recipe | GCC or glibc |
| Flavor/variant | Supported configuration of one port | GCC bootstrap or final configuration |
| Bootstrap stage | Configured build's role, prerequisites and required outputs in an ordered flow | Minimal compiler enabling libc construction |
| Build instance | One selected port configuration in a group | ARM64 group's GCC bootstrap instance |

Group provides context; Flavor selects configuration; stage dependencies
establish order. Group membership alone does not order builds. The existing
bs group collects bootstrap tools; it is not a numbered compiler bootstrap
pass. Native bootstrap dependencies may come from bs/host according to
project provider policy without making each pass a separate group.

A single stage instance belongs to one port configuration. A complete
bootstrap sequence spans several such instances, potentially repeating the
same port with different flavors. For example, conceptually:

```text
Group: ARM64 toolchain
    binutils:target
          |
    gcc:bootstrap
          |
    initial libc / intermediate compiler instances
          |
    glibc:full
          |
    gcc:final
```

Names and separators here are illustrative, not approved command syntax or
a finalized bootstrap recipe. The same GCC recipe supplies distinct bootstrap
and final configurations; the dependency graph identifies their stage roles.
Each instance retains independent state and declared outputs. A stage may
produce temporary tools, headers or startup objects without producing a public
distributable package.

Use one configuration identity, represented by Flavor/variant, with explicit
dependency edges and output-readiness requirements for bootstrap participants.
Do not introduce a redundant public STAGE selector alongside FLAVOR, such as
FLAVOR=bootstrap STAGE=bootstrap. If a future requirement needs an additional
identity dimension, review it explicitly rather than adding it implicitly.
Bootstrap stages are also distinct from the existing package stage lifecycle
and STAGEDIR, which assemble installation files for packaging.

## Flavor Framework Boundary

The reusable declaration, discovery, target/dependency syntax, default identity,
artifact isolation, package/manifest and status-display contracts are maintained
in [10.5-FLAVOR.md](../../make/uports/docs/10.5-FLAVOR.md).
This report owns their application to toolchain profiles and bootstrap roles.
Flavor implementation history remains in the work tracker; it does not define
a second architecture or progress ledger.

## Toolchain Profile Contract

Profile roots use the target tuple, with explicit additional roots for
incompatible versions such as jos12. Keep DESTDIR and PREFIX as the standard
installation controls; development/testing uses DESTDIR=$(topdir)/local.
These contracts do not activate groups or change the parent Makefile.

| Profile / target tuple | Proposed native tool group | PREFIX | Development installation root |
| --- | --- | --- | --- |
| i386-jos-elf (mit2018) | toolchain | /i386-jos-elf | $(topdir)/local/i386-jos-elf |
| i386-jos-elf (jos12) | toolchain | /i386-jos-elf-gcc12 | $(topdir)/local/i386-jos-elf-gcc12 |
| riscv64-unknown-elf | toolchain | /riscv64-unknown-elf | $(topdir)/local/riscv64-unknown-elf |
| aarch64-linux-gnu | toolchain | /aarch64-linux-gnu | $(topdir)/local/aarch64-linux-gnu |

Keep compiler for the existing normal compiler/development environment and
declare one toolchain group for cross-toolchain production. They coexist as
separate groups; do not rename compiler, move its current packages, or change
its installation defaults. Selected port Flavors distinguish target/stage
configurations inside toolchain. Flavor identifiers follow the declaration
syntax; they map explicitly to tuples rather than requiring tuple strings as
Flavor names. The same origin can have several selected instances through flavored dispatch
and dependency resolution.

Set producer CONFIGURE_TARGET and tuple PREFIX per selected instance, not
globally on toolchain. GCC/Binutils/GDB need their selected output target;
QEMU and native prerequisites need their own configuration. CROSS_COMPILE
stays unset for native tool production. Do not generate groups implicitly or
set CONFIGURE_TARGET and CROSS_COMPILE together on a compiler-producing port.

Cross compilers, Binutils, GDB and QEMU are host-running tools. Install them
beneath local/<tuple>, not local/compiler, during development. This does not
automatically assign them to the existing host group: group/provider policy
and installation layout are separate decisions. Only selected profiles need
directories; the layout is unchanged when a single profile is enabled.
All three profiles must coexist with isolated build/package/install identities
and explicit compiler/debugger/emulator selection. DESTDIR may instead be /opt
or another chosen root; no particular DESTDIR is a permanent runtime location.
The tuple identifies the target, not the SDK version. The accepted jos12 root
is an explicit additional root for the same JOS tuple; other incompatible SDK
versions require their own reviewed installation roots.

Tools are exposed below DESTDIR+PREFIX/bin. Build/host are the actual native
system; CONFIGURE_TARGET selects the generated-code target for Binutils/GCC/GDB.
QEMU runs natively with its selected emulation configuration, not with
CROSS_COMPILE targeting JOS/xv6.
The two MIT profiles have no target libc/sysroot construction requirement.

Target libraries use a reusable target/sysroot environment, with a separate
instance for each target/compiler/ABI context. ARM64 Linux is the first instance;
the framework must not hard-code ARM64-specific group names or paths. Concrete
project group names are implementation configuration, not a new public selector.
For ARM64, DESTDIR is the selected profile root plus /sysroot, PREFIX=/usr,
and CROSS_COMPILE=aarch64-linux-gnu selects the target compiler. The same
DESTDIR/PREFIX model applies to other supported target tuples. Supporting a new
CPU also requires reviewed recipes, ABI settings and bootstrap requirements;
changing the tuple alone does not establish support. The MIT bare-metal profiles
do not require this Linux/glibc environment. This is a justified context boundary,
not one group per bootstrap pass. The exact GCC
sysroot configure/search-path and relocatable SDK packaging rules must be
reviewed against selected versions before staged SDK execution.

For example, toolchain's GCC is built with the native compiler and executes on
the host, while the ARM64 sysroot's glibc uses that generated ARM64 GCC and
contains ARM64 objects. Build utilities used during glibc construction still
run natively. Keep native dependency include/library/runtime paths separate
from target sysroot and pkg-config paths; do not put target libraries on the
host LD_LIBRARY_PATH. Linux header installation is architecture-specific;
not every header-producing step requires an executable cross compiler.
The separate context is needed even though both outputs share one profile root.
Bootstrap/final passes remain flavored instances with ordered sysroot writes,
not additional groups for each pass. No ARM64 target libraries go into the
ordinary local/compiler tree through this profile.

The ARM64 SDK includes target C headers and libraries. Sanitized Linux UAPI
headers come from the kernel's headers-install procedure; standard C/POSIX and
target-specific libc headers come from glibc. Both install under
<profile-root>/sysroot/usr/include. Target runtime/development libraries and
the loader belong under that sysroot; exact lib/lib64/multiarch placement is
reviewed with the selected versions. GCC also supplies compiler-specific
headers and target runtimes in its own configured, versioned directories.
Do not copy host /usr/include or use host libraries to populate the sysroot.

Configure the installed GCC with a default sysroot beneath its execution
prefix, and supply the actual build-time location separately. Proposed final
compiler arguments for the development ARM64 layout are:

```text
--target=aarch64-linux-gnu
--prefix=/aarch64-linux-gnu
--with-sysroot=/aarch64-linux-gnu/sysroot
--with-build-sysroot=<project>/local/aarch64-linux-gnu/sysroot
```

Derive these paths from the selected DESTDIR/PREFIX, not hard-coded project
paths. GCC documents relocation of a sysroot beneath its execution prefix
relative to the compiler binaries. --with-build-sysroot supplies the location
used while building target libraries; it does not replace the installed default.
See [GCC configuration](https://gcc.gnu.org/install/configure.html).
Bootstrap configurations may require different options. Validate -print-sysroot,
header/library searches and real compile/link behavior both in development and
after moving the complete profile to another DESTDIR. Configure Binutils
consistently and verify host runtime-library lookup; GCC's documented behavior
alone does not establish relocation of the complete SDK.

Proposed toolchain policy permits selected native providers from toolchain,
host, bs and compiler, plus validated native system providers. For jos12, select compiler's modern @default GMP/MPFR/MPC only when their
effective build environment and installed artifacts are native-compatible.
Historical mit2018 explicitly selects its @mit2018 native providers in toolchain. Group name alone does not prove this. Require explicit
selection when compatible candidates are ambiguous. Preserve host's existing
host/bs-only uports policy; toolchain's policy does not widen it.
Native dependency installation prefixes belong to their provider instances,
not the consuming toolchain's tuple root. Consumer flags must use those actual
paths; do not propagate CONFIGURE_TARGET or append /tools in library recipes.
This permits compatible native prerequisites to be shared across profiles;
the historical mit2018 version requirements remain exact.
Target-library groups
must use target-compatible uports/explicit SDK providers; they cannot satisfy
target libc requirements with a host-system library. Preserve existing host
group restrictions rather than reassigning current host tools or dependencies.
Profile activation and acceptance are tracked separately from this contract.

Automatic consumer matching remains proposed: CROSS_COMPILE identifies a
required target, which must resolve to a compatible explicitly selected
toolchain instance or configured SDK. Match target/configuration/readiness,
reject missing or ambiguous providers, order installation before consumer
execution, and export the selected bin directory. A tuple alone does not prove
ABI or bootstrap readiness. Current preflight only searches prefixed commands
on PATH; it does not construct this dependency. Current target contexts are
group-based and reject mixed CROSS_COMPILE values, supporting a separate
target/sysroot environment. Its external-provider mode must explicitly support
cross dependencies; do not silently reuse native host discovery policy.

Instance identity remains group + origin + flavor. Target tuple, ABI/ISA,
source/patch configuration, runtime prefix and dependency identities belong
in artifact state; checkout/cache locations alone are not portable content
identity. Stages add readiness/dependency relationships to those instances,
not another public STAGE selector. Flavor artifact paths and provider matching
follow the reusable framework contract; stage readiness remains separate
planned work.

The existing tools.mk normalized records, variant keys and shared lifecycle
dispatch are the integration boundary. Extend dependency metadata and execution
through that boundary instead of adding a second scheduler. Cross-toolchain
preflight checks existing compilers; constructing them is the responsibility of
component recipes. CONFIGURE_TARGET and CROSS_COMPILE retain their distinct
producer/consumer roles.

The core origins are the seven in the accepted inventory. Prerequisite closure and
exact component versions remain pending per-target audits; no additional ports
or unreviewed bootstrap passes are implicitly approved by this baseline.

## Bootstrap Stages

Flavors describe supported configurations. Bootstrap stages additionally
describe prerequisite outputs and execution order. Extend the existing graph
and scheduler; do not introduce a separate orchestration system.

The MIT profiles need ordinary Binutils/compiler/runtime ordering but do not
require the full GCC/glibc bootstrap graph. Complete both profiles before
implementing general staged-libc support.

For ARM64, the conceptual sequence is:

```text
Binutils + Linux UAPI headers
              |
        bootstrap GCC
              |
   initial libc components / intermediate compiler support
              |
         complete glibc
              |
     final GCC C/C++ and runtimes
```

This is not a fixed executable recipe. Selected GCC/glibc versions determine
whether separate libc headers/start files, intermediate libgcc or other passes
are necessary. Review that concrete graph before enabling execution.

Each pass declares exact providers and readiness outputs, including which
compiler runs on the host and which objects belong in the target sysroot.
Intermediate headers/stubs must never satisfy final libc readiness. Stage
completion needs saved configuration/output evidence even when the stage
does not produce a public package.

Dependencies order any writers sharing a sysroot. Define file ownership,
temporary output replacement and promotion explicitly; do not let concurrent
stages overwrite files or replace an accepted final SDK during experiments.
Resume after failure must distinguish valid prerequisite outputs from partial
installation. An upstream stage/source/ABI change invalidates affected
downstream state without discarding unrelated work or developer edits.

## Installation, Caches And Source Handling

Use tuple-named installation roots beneath the selected DESTDIR, with
PREFIX=/<tuple>, except explicitly reviewed additional roots such as
/i386-jos-elf-gcc12. Development installs use $(topdir)/local/<profile-root>.
A profile exposes its host-running tools and, for ARM64, its target sysroot.
Honor staging via DESTDIR and the intended runtime prefix separately; never
embed a temporary staging path as the compiler's final installation prefix.

Preserve the existing development-install contract:
`FORCE_DEVELOPMENT_INSTALL=yes` prepares sources through fetch/extract/patch
even when a compatible package is reused, skipping configure/build/stage/
package creation. Explicit `no` skips source preparation for that compatible
package; without one, both settings follow the normal source build flow.
`FORCE_REBUILD=yes` remains a cache bypass, including existing group policy
that sets it. Do not silently change group policy to demonstrate reuse.
Flavor/stage identity must reject incompatible artifacts without treating
checkout-local paths or missing local build state alone as incompatibility.
See [08-INSTALL.md](../../make/uports/docs/08-INSTALL.md) for the implemented package contract.

Bind compiler, linker, debugger and QEMU selection explicitly in each profile
and lab invocation. Determine compiler search paths, runtime loader behavior
and host shared-library dependencies as part of packaging. Relocation is an
acceptance question to define, not an automatic property of staged installation.

Preserve PORTSDIR/distfiles, packages and scm as authoritative local paths;
existing directories/symlinks outrank global-site defaults. Source caches may
be shared where coordinates match. Mutable patched/configured source trees
must not be shared unsafely between concurrent flavors or stages. Use separate
out-of-source build directories for GCC/Binutils and preserve fetched checkouts.

Prefer reviewed release archives/checksums or pinned commits. Record local
compatibility patches. Preserve the historical MIT fork separately; do not
replace it with upstream QEMU just because upstream builds more easily.

## Delivery And Validation Boundaries

Establish the profile/identity contract and Flavor foundation before MIT tool
integration. Complete both MIT profiles before general staged-libc execution,
then use that foundation for the complete ARM64 SDK.

The [task tracker](11-CROSS-TOOLCHAIN-TASKS.md) owns the single ordered work list,
implementation order, temporary restrictions, acceptance evidence and commits.
Keep the architecture organized by responsibility as delivery units evolve.

Use a real two-configuration pilot to exercise the Flavor contract before
complex compiler recipes. Preserve shared planner dispatch and selected-instance
growth constraints in [10.2-TOOLS.md](../../make/uports/docs/10.2-TOOLS.md). Required plist helpers
belong in the framework contract rather than an implicit prerequisite.

The Flavor pilot precedes complex compiler bootstrapping. Stage framework
validation can use early ARM64 prerequisite work, retained for the complete
SDK phase; do not build the entire SDK merely to review its graph.

Stage support follows the MIT profiles because they exercise compiler recipes
and the Flavor foundation without libc bootstrapping. This is the chosen
risk-reduction sequence, not a technical restriction. Contract discussion can
occur earlier; execution waits for prerequisite acceptance. Retain early ARM64 stage
outputs and evidence for complete SDK acceptance. Do not repeat accepted builds
solely because a later delivery unit uses the same evidence.
The selected recipe determines actual compiler/libc passes; a conceptual graph
does not mandate unnecessary passes.

| Acceptance | Required evidence |
| --- | --- |
| Flavor framework | Real configurations, exact dependencies, isolated outputs, repeat reuse, invalid selection/conflict behavior |
| JOS profile | Generated tools compile the pinned lab, correct ELF32 output, boot in generated MIT QEMU, GDB breakpoint/step |
| xv6 profile | Generated tools compile kernel/users/image, boot in generated RISC-V QEMU, basic runtime tests and GDB breakpoint/step |
| Stage framework | Resolved order, output readiness, intermediate/final distinction, failure/resume and invalidation behavior |
| ARM64 SDK | C/C++ compile/link, ELF/loader inspection, threads/exceptions, runtime on ARM64 or QEMU and a real uports consumer |

ARM64 user-mode QEMU is distinct from MIT system emulation. If selected for
validation, explicitly build/select that capability and run with the generated
target loader/sysroot. It does not prove complete target-kernel compatibility.

User manually validates each component/change before dependent work advances.
No automatic regression runs, build execution or new test scripts are implied
by this proposal. Record source/configuration, commands, logs, platform and
commit mapping in the tracker. Separate framework, port and parent-policy
commits, with authorization before committing or updating the parent gitlink.

## Open Decisions And Limits

Resolve these in the corresponding tracker tasks before implementation:

- Exact source releases/commits and required modern-host compatibility patches.
- Final port origins, prerequisite closure, group placement and profile paths.
- Remaining profile recipe mappings and historical-version migration.
- Exact ARM64 bootstrap graph, intermediate output ownership and package split.
- ISA/ABI/kernel baseline, sysroot layout and relocation requirements.
- macOS support and native dependencies, separately from Linux acceptance.

Out of initial scope: complete Debian package reproduction, additional target
ABIs, Canadian cross, full automatic language Flavor generation, all-flavor
matrix execution and solving the course lab exercises.

## Amendment: Native Toolchain Follow-up

After completing the cross-toolchain profiles, add a native-toolchain
implementation and validation phase. This is agreed follow-up scope, not a
claim that native GCC/GDB support is already implemented. Define the shared
recipe contract now while preserving accepted cross configurations.

### Native And Cross Producer Variants

GCC, Binutils and GDB producer executables run on the build host. Their output
or debugging target is independent of that execution environment. Provide
native/default variants alongside the existing cross Flavors, with isolated
work, state and package identities. Recipes own versions, target settings,
dependencies and default prefixes; groups supply native environments and
aggregate membership.

The recipe's declared default should support native operation once that support
is implemented. Project group defaults may still explicitly select a cross
Flavor for toolchain aggregates. Preserve the current mit2018 aggregate defaults
until deliberately changing them; native support does not silently redirect
existing project commands.

### Bootstrap Requirements

Identify and validate the host compiler and build tools needed to start native
toolchain construction. GCC may use its own bootstrap build procedure; this
does not require Binutils and GDB to use the same procedure.

Distinguish GCC's internal bootstrap from the cross-component stages used to
construct a Linux compiler, headers and libc. The bs/host groups describe
environments and provider policy, not bootstrap-stage ordering. Select and
validate each native recipe's bootstrap requirements explicitly.

### Tuple Routing

| Build role | Tuple/configuration routing |
| --- | --- |
| Native compiler tools | Build and run on the host; normally leave CONFIGURE_TARGET unset. |
| Cross compiler tools | Generated-code target tuple maps to the producer's CONFIGURE_TARGET. |
| Target libraries, including glibc | Use the target compiler, configure host tuple and sysroot for that library build. |

Tuple-named project targets select the matching component Flavors and scoped
settings. Keep CONFIGURE_TARGET local to the producers that need it rather
than exporting it globally into native prerequisites or emulator builds.
CROSS_COMPILE remains unset while building host-running producer executables;
it selects target tools when compiling target libraries or application code.
No additional toolchain profile variable is required.

Native acceptance must cover compiler output, assembly/linking, debugger use,
dependency/runtime paths and package installation/removal. Linux and macOS
acceptance remain separate. Record the implementation tasks and evidence in
the companion tracker when scheduling this phase.

### QEMU Recipes And Emulation Modes

Keep the historical MIT-patched QEMU in emulators/qemu-mit for the MIT 2018
JOS target. Add a separate emulators/qemu recipe for upstream QEMU used by
the RISC-V course and ARM64 validation. Both produce executables that run on
the build host; their emulated architectures are selected by QEMU's target
list, not CONFIGURE_TARGET or CROSS_COMPILE.

| Use | Executable | QEMU build target |
| --- | --- | --- |
| MIT 2018 JOS, historical MIT fork | qemu-system-i386 | i386-softmmu |
| MIT 2026 xv6, upstream QEMU | qemu-system-riscv64 | riscv64-softmmu |
| ARM64 complete guest machine and OS | qemu-system-aarch64 | aarch64-softmmu |
| ARM64 Linux applications on a Linux host | qemu-aarch64 | aarch64-linux-user |

System emulation supplies a virtual CPU, memory and devices and boots a guest
kernel. Linux user emulation runs individual target programs and translates
their Linux system calls to the host kernel; it does not boot a guest OS.
The aarch64-linux-user build target produces the qemu-aarch64 executable.
For dynamically linked programs, select the generated target loader/libraries
with qemu-aarch64 -L <sysroot>. This mode requires a Linux host; macOS
validation needs system emulation or another ARM64 runtime environment.

Neither MIT target requires ARM64 emulation. For the aarch64-linux-gnu SDK,
user emulation can validate applications; system emulation is an optional
full-system validation capability. QEMU is not inherently required to build
the cross compiler itself.

One upstream QEMU build/package can enable all three selected targets:

```sh
../configure \
  --target-list=riscv64-softmmu,aarch64-softmmu,aarch64-linux-user
```

The resulting executables can coexist under the same DESTDIR/PREFIX. Separate
architecture Flavors are not required merely to provide these executables.
Select the final target list and validate each enabled mode when implementing
the upstream recipe; this amendment does not claim that recipe is complete.

## Reference Sources

- [MIT 2018 tool requirements](https://pdos.csail.mit.edu/6.828/2018/tools.html)
- [MIT 2018 lab setup](https://pdos.csail.mit.edu/6.828/2018/labs/lab1/)
- [MIT QEMU fork](https://github.com/mit-pdos/6.828-qemu)
- [MIT 2026 tool requirements](https://pdos.csail.mit.edu/6.828/2026/tools.html)
- [MIT 2026 first lab](https://pdos.csail.mit.edu/6.828/2026/labs/util.html)
- [Debian Trixie crossbuild-essential-arm64](https://packages.debian.org/trixie/crossbuild-essential-arm64)
- [Debian Trixie cross libc development files](https://packages.debian.org/trixie/libc6-dev-arm64-cross)
- [FreeBSD Flavors](https://docs.freebsd.org/en/books/porters-handbook/flavors/)
- [crosstool-NG toolchain construction](https://crosstool-ng.github.io/docs/toolchain-construction/)
