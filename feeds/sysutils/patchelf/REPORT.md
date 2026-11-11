# Native Patchelf Export Tool

Patchelf 0.18.0 is built from the upstream release asset, which includes generated
configure files. It requires a native C/C++17 compiler; no Autotools bootstrap
or target compiler is needed. The recipe rejects cross-compilation settings
and currently supports release sources only.

The parent project selects this recipe in bs. Default installation is below
DESTDIR/PREFIX, normally local/usr/local. EXPORT=yes i386-jos-elf installs it
through the existing group lifecycle and uses that path automatically. Ordinary
i386-jos-elf installation does not add this dependency. Patchelf modifies copied
host ELF files during export and is not a runtime dependency of the distribution.

Manual commands:

```sh
make V=1 host:patchelf.stage
make V=1 host:patchelf.install
make EXPORT=yes i386-jos-elf JOS_SOURCE=/path/to/accepted/jos
```

Source: https://github.com/NixOS/patchelf/releases/tag/0.18.0

Linux x86_64 staging passed with a matching 12-entry manifest. Packaging and
installation completed through EXPORT=yes without a PATCHELF override. The
managed executable relocated the real distribution; compiler/libgcc, JOS boot
and GDB breakpoint/step checks passed. Logs: /tmp/uports-patchelf-stage.log and
/tmp/uports-bs-patchelf-export.log. Sandbox DNS/cache-write restrictions required
outside-sandbox retries; no system package was installed. macOS remains
unvalidated, and the exporter currently supports Linux only.
