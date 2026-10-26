# Mini-XML Source And Runtime Verification

## Source Declaration

Mini-XML 4.0.5 uses `USE_GITHUB=yes`, `GH_ACCOUNT=michaelrsweet`, and
`DISTVERSIONPREFIX=v`. The helper supplies the project name, v4.0.5 tag,
codeload URL, `_GH0` cache name, and `mxml-4.0.5` extraction directory.
`DIST_SUBDIR=mxml` remains the local cache/distinfo namespace.

The downloaded `michaelrsweet-mxml-v4.0.5_GH0.tar.gz` is byte-identical to the
previous cached `v4.0.5.tar.gz`: SHA256
`e021c37f8818f21628a8fc40412138ea0c5e6320c93da3d3fc03d1f0dab86fdc`,
1571504 bytes. Distinfo records the new cache name. No version, patch,
configure, library layout, or plist change is required by this source migration.

## Linux Verification

A fresh isolated copy of the port fetched from codeload, generated distinfo,
and completed checksum/extract/patch/configure/build/stage with `PREFIX=/usr`
and an explicit temporary DESTDIR. No host installation was performed.
The archive becomes a local Git baseline through the normal patch phase;
this port currently has no maintained patch series.

The staged shared library is `usr/lib/libmxml4.so.2`, with the linker symlink
and static archive alongside it. The header is
`usr/include/libmxml4/mxml.h`; pkg-config metadata is
`usr/lib/pkgconfig/mxml4.pc`. Documentation and manual pages remain described
by the existing Linux plist.

A C smoke test linked against the staged library with an explicit staged
rpath and called `mxmlNewElement(NULL, "root")`,
`mxmlNewText(root, false, "hello")`, and `mxmlSaveAllocString(root, NULL)`.
It asserted serialization contains `<root>hello</root>`, then freed the output
and deleted the tree. This verifies actual library loading and XML behavior,
not merely a version string.

The slice 6 real-world script also passes archive -> Git tag -> explicit
commit -> archive -> Git tag builds and packages, source provenance, unchanged
repeats, and staged XML runtime checks. From the parent project, use
`make -C make/uports/Tools/tests slice6`. Source mode can be scoped to the
compiler instance with `PORTS_compiler_mxml_EXTRA_ENVS=SOURCE_MODE=dist` or
`SOURCE_MODE=git`. Both sources include the configure inputs needed by this
port; no additional Git bootstrap hook is required.

## Darwin And Relocation

The existing Darwin architecture flags, shared-library fixups, and plist are
unchanged. This migration has not been executed on Darwin. Staged-rpath Linux
testing does not establish arbitrary installed-prefix relocation; validate
Darwin loading and any nonstandard-prefix deployment separately.
