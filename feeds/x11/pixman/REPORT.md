# Native Pixman prerequisite

Pixman 0.34.0 supplies QEMU's pixel operations. It is a compatible host-library
selection, not a release required by MIT. The official release archive provides
generated configure files, avoiding autoreconf and QEMU's old submodule URL.
GTK test integration and optional PNG support are disabled, so neither library
is needed. The library itself remains enabled for x86 system emulation.

The project selects a native toolchain instance at DESTDIR/usr/local, independent
of compiler-target library instances. It requires no target compiler or sysroot.

```sh
make V=1 toolchain:pixman.stage 2>&1 | tee /tmp/uports-pixman034-stage.log
```

Linux stage,11-entry manifest, install and cached removal/reinstall passed.
The real image-fill consumer selects0.34.0 from staged and restored installed
libraries and produces the expected ARGB pixel. Logs
/tmp/uports-pixman034-*.log. macOS and copied deployment remain pending.
After stage, generate the actual
platform manifest, inspect .pc/loader paths and compile/run an image-operation
consumer. Installed/copy behavior and package removal/reuse need separate
validation. No source patches or fabricated manifests are included.

Source: https://www.cairographics.org/releases/
