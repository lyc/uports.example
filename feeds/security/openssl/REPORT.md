# OpenSSL source migration and validation

## Sources and mode selection

OpenSSL 3.5.6 keeps its release site and Git declarations together:

- Archive: `https://www.openssl.org/source/openssl-3.5.6.tar.gz`.
- Repository: `https://github.com/openssl/openssl.git`.
- Requested Git tag: `openssl-3.5.6`, resolving during validation to
  `286ddeaac037533bbdce65b3c689e3f7ffebf0f6`.

The default remains `USE_SCM ?= git`. Explicit `SOURCE_MODE=dist` selects
the release archive; `SOURCE_MODE=git` selects the repository. The latter
uses a distinct `openssl-3.5.6-git` root. The existing Git cache URL/key and
legacy default source-root spelling are preserved. No cache conversion is
required. These changes concern the feed recipe, not the separate bundled
framework OpenSSL recipe.

While other ports remain unmigrated, scope wrapper selection to this instance:

```sh
make PORTS_compiler_openssl_EXTRA_ENVS=SOURCE_MODE=dist compiler@openssl.fetch
make PORTS_compiler_openssl_EXTRA_ENVS=SOURCE_MODE=git compiler@openssl.fetch
```

The archive is 53121812 bytes. Its SHA256 matches the digest published by
[OpenSSL](https://www.openssl.org/source/openssl-3.5.6.tar.gz.sha256):
`deae7c80cba99c4b4f940ecadb3c3338b13cb77418409238e57d7f31f2a3b736`.
The new maintained distinfo records that size and digest. The release URL and
filename are unchanged. Both source forms supply `config` and Perl `Configure`;
normal configuration generates `configdata.pm`. No Autotools bootstrap was
added. Existing shared-library, no-assembly, zlib and platform settings remain.

## Validation status and manual workflow

The user will validate this migration manually in the compiler phase, after
the bs, host and textproc groups. There is no OpenSSL-specific test script.
Manual acceptance remains pending; the earlier results below are historical
evidence, not a substitute for the user's acceptance.

Earlier checks used parent-wrapper fetches with SITE cache settings and
instance-scoped source modes, then isolated direct-port builds and packages.
They verified the release against maintained distinfo and OpenSSL's published
digest. Direct builds used dependency mode `none` and the native compiler.

The first archive build found that host zlib development headers were absent.
The repair built the real cached zlib 1.3.2 release through its upstream
configure/make flow into the temporary target root, then resumed the failed
OpenSSL build. This was dependency preparation, not validation of the
separately queued CMake-based uports zlib recipe. Nothing is installed on the
host. Runtime loader checks verify both OpenSSL libraries and this temporary
zlib are used.

Both source modes passed configuration, compilation, staging and package creation.
Checks cover staged headers, libssl/libcrypto, documentation, pkg-config files,
the existing Linux plist, source provenance, SHA256 on known input, and AES-256
CBC encryption/decryption with PBKDF2. Identical package repeats preserve state,
packages and staged-file hashes/mtimes; wrapper fetch repeats also pass. No
Linux plist edits were needed. Parent dependency diagnostics pass.

Detailed historical evidence locations and changing status belong in the
framework workstream handoff. Future validation commands will be provided for
manual execution against the user's selected dependency environment.

## Limits

This validates Linux native builds at a staged `/usr/local` prefix. Darwin,
cross builds, final-prefix host installation, arbitrary relocation and the
full upstream OpenSSL test suite are not established by these checks. Package
and framework changes remain uncommitted until the user accepts all validation.
