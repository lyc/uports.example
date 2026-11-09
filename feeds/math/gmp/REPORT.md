# GMP native library configurations

| Flavor | Release | Purpose | Default PREFIX |
| --- | --- | --- | --- |
| default | 6.3.0 | Existing native compiler/development libraries | /usr/local |
| mit2018 | 5.0.2 | Historical MIT JOS compiler prerequisite | /i386-jos-elf |

The historical release is selected by the
[toolchain reproduction contract](../../lang/11-CROSS-TOOLCHAIN.md).
It runs on the host, not the generated-code target. CONFIGURE_TARGET is cleared
as for the existing library recipe; mit2018 rejects CROSS_COMPILE. Project
settings supply DESTDIR=local and select toolchain:gmp@mit2018. Modern compiler
consumers retain their compiler-group default provider. Provider exports use
the selected instance's actual include/library directories.

mit2018 uses GNU's gmp-5.0.2.tar.bz2 with its own distinfo, package suffix and
platform manifest. It uses release configure defaults rather than inheriting
6.3.0's C++/PIC/linker options, configure patch or Darwin assembly policy. The
release includes gmp.info; MAKEINFO=true avoids regenerating it with host
Texinfo. The ordinary all/check build retains upstream arithmetic tests.
The first real configure failed its long long reliability test because host
GCC14 rejects implicit int and implicit function declarations in the old probe.
mit2018 selects -std=gnu89, matching the release's expected C dialect without
patching its source or suppressing those diagnostics for other configurations.
This flag applies to the historical native library build only. Retrying the
stage command reruns the failed configure; no source cleanup is needed.
No historical assembly workaround is assumed from the 6.3.0 recipe.

Linux manual stage passed all 145 upstream tests. libgmp.so.10.0.2 is native
x86-64, needs only libc and has no embedded runtime search path. The generated
share/info/dir is omitted from the package: it is a shared installation index,
already owned by Binutils in the profile. The three gmp.info files remain.
Installed shared/static arithmetic consumers report 5.0.2 and pass 200-bit
multiply/divide equality; the shared loader resolves the profile library.
Historical packages retain .la files through libtool:keepla, which clears
transitive dependency_libs while preserving the runtime libdir. This avoids
unusable absolute .la references when libraries are installed through DESTDIR.
Manual removal/reinstall passed using the cached package without compilation;
the restored shared arithmetic consumer passes. macOS acceptance remains pending.
Intel macOS needs modern compiler, assembly and loader validation. GMP 5.0.2
predates AArch64 support, so native Apple Silicon needs reviewed compatibility
work rather than copying the modern Flavor's Darwin settings.

Adding declared Flavors gives both configurations separate work/source/state
paths, including the default. Existing work.compiler and modern installed
libraries are preserved; they are not automatically migrated or rebuilt.

## Manual validation

From the project root:

```sh
set -o pipefail
make V=1 toolchain:gmp@mit2018.stage \
  2>&1 | tee /tmp/uports-gmp502-stage.log
make V=1 toolchain:gmp@mit2018.generate-plist
```

Inspect the actual stage and record pkg-plist.mit2018.<os>-<arch> before
install/package acceptance. Require native libraries/headers and upstream
checks, then an installed arithmetic consumer, package removal/reuse and
appropriate loader paths. Linux x86-64 checks are complete as recorded above;
macOS and dependent historical MPFR/MPC/GCC remain separate acceptance.
