# MPC native library configurations

| Flavor | Release | Source | Default PREFIX |
| --- | --- | --- | --- |
| default | 1.4.1 | Existing Git default; SOURCE_MODE=dist for release | /usr/local |
| mit2018 | 0.9 | Historical release tar.gz | /i386-jos-elf |

The historical configuration follows the
[toolchain reproduction contract](../../lang/11-CROSS-TOOLCHAIN.md).
Dependencies explicitly select math/gmp@mit2018 (5.0.2) and math/mpfr@mit2018
(3.1.2). These are native host libraries supporting the compiler, not JOS
target libraries. CONFIGURE_TARGET is cleared; mit2018 rejects CROSS_COMPILE.
Project membership preserves modern compiler providers alongside the historical
toolchain instances. Historical configure flags use the selected providers'
actual include/library/runtime paths rather than guessing from consumer PREFIX.

MPC 0.9 uses its original release archive, generated configure and shipped
mpc.info. It does not inherit the modern recipe's autoreconf or linker options.
GNU89 selects the historical C dialect; MAKEINFO=true avoids regeneration with
host Texinfo. Build all/check includes upstream tests. Generated share/info/dir
is omitted from the package to avoid shared index ownership conflicts; MPC's
manual remains included. This release is LGPL 2.1 or later, as stated in its
header and COPYING.LIB, rather than the newer default release's license.

The first build exposed two host/staging compatibility issues. MPFR's .la
contained an absolute runtime-prefix dependency that failed under development
DESTDIR. Historical math recipes use the existing libtool:keepla cleanup to
retain .la files with empty dependency_libs; do not rewrite libdir to a workspace
path. Updated prerequisite packages must be restaged/repackaged/reinstalled.
MPC's release configure also skips EGREP discovery when CC is preset. Supply
EGREP explicitly so its GNU linker probe does not silently disable shared
libraries. This requires reconfiguring MPC in a fresh preserved work namespace.

Versioned checksums, a separate package suffix and a historical platform plist
isolate outputs. Both Flavors have distinct work/source/state namespaces.
Existing work.compiler, installed modern libraries and cached packages are
preserved. Adding Flavors does not migrate old work trees automatically.

## Manual validation

From the project root:

```sh
set -o pipefail
make V=1 toolchain:mpc@mit2018.stage \
  2>&1 | tee /tmp/uports-mpc09-stage.log
```

Confirm historical GMP/MPFR are reused before configure, then inspect upstream
test results and actual stage files. Record the historical platform plist
before package/install. Native complex arithmetic must report MPC 0.9 with
GMP 5.0.2 and MPFR 3.1.2, using the profile's shared libraries. Verify static
linking and cached-package reuse/removal separately. macOS requires its own
compatibility work and manifest.

Linux stage passed all 60 upstream tests after the libtool metadata and EGREP
corrections. Shared libraries are enabled; libmpc.so.2.0.0 loads as 0.9 with
historical MPFR/GMP resolved from the profile. The actual 11-entry manifest
passes package inventory checking. Installed payload matches stage; shared and
static 128-bit consumers confirm all three historical versions and pass exact
complex multiplication/division round trips. The shared loader selects profile
libraries. Manual removal/reinstall reuses the cached package without
compilation; restored shared/static complex arithmetic passes. macOS acceptance
remains pending.
