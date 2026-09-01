# Value Framework Tests

This directory verifies the generic value macro, list, metadata, object,
ownership, and codec contracts implemented by `libs/libutils/value_*.[ch]`.

## Naming

- `t_*.c` files are executable tests or test-local scenario implementations.
- `t_*.h` files contain test support or test-local model declarations.
- Each executable name matches its main source name.

## Test Programs

| Program | Primary coverage |
| --- | --- |
| `t_value_list` | Typed keyed lists, lookup, move-to-front, filter, map, and reduce |
| `t_value_domain_key` | Domain-key lists and handwritten lookup |
| `t_value_no_key` | Types without generated key lookup |
| `t_value_metadata` | Presence, dirty, validation, and copy metadata |
| `t_value_embedded` | Embedded values, ownership, and detached hooks |
| `t_value_list_mutation` | Generic/key sorting and conditional removal |
| `t_value_errors` | Invalid arguments, allocation failure, and cleanup |
| `t_value_types` | Generic `struct value` and `struct kvalue` helpers |
| `t_value_macros` | Changed-field and comparison macros |
| `t_value_filter_ref` | Borrowed filter views, allocator provenance, and view operations |
| `t_value_codec` | Registry, schema, diagnostics, encode/decode, and ownership |
| `t_ru_value_model` | Larger RU-domain integration fixture |

`t_ru_value_fixture.[ch]` defines the RU test model and
`t_ru_scenarios.[ch]` implements its integration scenarios. They are test
fixtures, not part of the generic framework contract.

## Running

Run the complete debug suite with:

```sh
make test
```

Additional isolated configurations are available:

```sh
make test-release
```

The release target uses the build system's native `obj/` output directory;
debug tests use `objd/`. Sanitizer targets are not exposed until the common
build system provides a dedicated sanitizer/profile object configuration.

The target builds every program, executes each one with the locally built
`libutils`, and stops with a nonzero result on the first failure.

## Ownership Rules

- Test copy callbacks must roll back partial allocations and report errors.
- Copying an intrusive value must preserve the destination hook linkage.
- Filter views borrow their values; freeing a view frees wrappers only.
- Source values must outlive all filter views that reference them.
- Map callbacks own mapped allocations; callers clean partial output after a
  callback-reported failure.
- Successful encoded output is released with `value_encoded_fini()`.
- Failed decode operations must not publish partially constructed objects.
