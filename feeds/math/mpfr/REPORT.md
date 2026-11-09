# MPFR native library configurations

| Flavor | Release | Source | Default PREFIX |
| --- | --- | --- | --- |
| default | 4.2.2 | Existing Git default; SOURCE_MODE=dist for release | /usr/local |
| mit2018 | 3.1.2 | Historical release tar.bz2 | /i386-jos-elf |

The historical configuration follows the
[toolchain reproduction contract](../../lang/11-CROSS-TOOLCHAIN.md).
Its exact library dependency is math/gmp@mit2018, providing GMP 5.0.2.
Both libraries run natively on the build host. Historical MPFR rejects
CROSS_COMPILE and clears CONFIGURE_TARGET as the existing library recipe does.
Project settings keep historical instances in toolchain and modern defaults
in compiler. Provider exports supply native include/library/runtime paths;
the historical recipe does not guess GMP's prefix from MPFR's own PREFIX.

mit2018 retains the historical release and configure defaults, uses GNU89 for
its older C sources, and builds all/check. The shipped mpfr.info avoids host
Texinfo regeneration through MAKEINFO=true. The generated shared Info index
is excluded from the package, while MPFR documentation remains included.
No newer-version configure options or source patches are applied implicitly.

Versioned distinfo, an explicit package suffix and a separate platform plist
distinguish the historical release. Both declared Flavors use isolated work
and source paths. Existing work.compiler, installed modern libraries and their
cached packages are preserved. Adding Flavors does not migrate old work trees.

## Manual validation

From the project root:

```sh
set -o pipefail
make V=1 toolchain:mpfr@mit2018.stage \
  2>&1 | tee /tmp/uports-mpfr312-stage.log
```

The dependency plan must select GMP 5.0.2 before MPFR configure. Inspect actual
stage files and upstream test results, then record the historical platform
plist before package/install. Verify the native runtime uses the historical
GMP library, including a real precision/rounding consumer. Package removal and
cached reuse, loader paths and macOS require their own acceptance.

Linux stage passed all 160 upstream tests. The staged library loads as 3.1.2
with GMP resolved from the historical profile; the actual 27-entry manifest
passes the package inventory checker. The first fetch exposed dependency loader
paths overriding system wget libraries; the framework now restores caller
loader paths for fetch while retaining dependency paths during configure/build.
Installed files/symlinks match stage. Shared/static consumers confirm MPFR
3.1.2 with GMP 5.0.2, directed 2-bit rounding and 256-bit pi bounds. Shared
loader paths select both historical profile libraries. Manual removal/reinstall
reuses the cached package without compilation; restored rounding checks pass.
MPC's subsequent build exposed an absolute /i386-jos-elf/lib/libgmp.la entry
in MPFR's dependency_libs. Historical packages now use libtool:keepla to clear
transitive .la dependencies without baking development DESTDIR into libdir.
Restage/repackage/reinstall is required to update the earlier package metadata.
macOS acceptance remains pending.
