# Value Macro Framework Design

This document defines the final architecture and usage rules for the value
macro framework in `libs/libutils`.

The framework provides reusable object lifecycle, intrusive-list operations,
field-state metadata, borrowed filter views, and small concrete `value` and
`kvalue` types. Domain models bind these mechanics while retaining ownership of
their own fields, validation rules, parsing, serialization, and business logic.

## 1. Architecture

### 1.1 Layering

The framework has three layers:

```text
Domain models
    file, node, carrier, interface, endpoint, ccmd, ...
        |
        | VALUE_OBJECT_*, VALUE_LIST_*, VALUE_META_*
        v
Generic framework
    value_base.h
    value_base.c
        |
        | intrusive list operations
        v
Linux-style list.h

Concrete generic types
    value_types.h
    value_types.c
        |
        | bind normal framework objects
        v
    struct value
    struct kvalue
```

The generic framework does not know domain types. Domain code supplies
lifecycle callbacks, key policy, validation, comparison, parsing, and output
behavior.

### 1.2 Source ownership

`value_base.h` owns:

- framework status codes
- comparison and list-add policy enums
- metadata declarations and binding macros
- object operation descriptors and binding macros
- list operation descriptors and binding macros
- generic runtime API declarations
- filter-view declarations
- memory-operation declarations

`value_base.c` owns:

- metadata state transitions
- object equality dispatch
- generic intrusive-list algorithms
- keyed lookup and move-to-front lookup
- foreach, map, reduce, sort, remove, count, and free operations
- borrowed filter-view allocation and cleanup
- generic list counting and insertion sorting
- configurable framework allocation

`value_types.h` owns:

- `struct value`
- `struct kvalue`
- their typed object/list declarations
- their public constructor, destructor, and append helpers

`value_types.c` owns:

- lifecycle behavior for `value` and `kvalue`
- their object and list bindings
- typed constructors and destructors
- typed append helpers

Domain headers own:

- domain structure definitions
- field bit definitions
- generated API declarations
- handwritten domain-specific APIs

Domain C files own:

- lifecycle callback implementations
- macro bindings
- constructors and destructors
- domain-key lookup
- parsing and serialization
- validation and business rules

### 1.3 Object model

Object behavior is described by:

```c
struct value_object_ops {
        const char *type_name;
        size_t size;
        value_init_fn init;
        value_fini_fn fini;
        value_copy_fn copy;
        value_validate_fn validate;
        value_cmp_fn cmp;
        value_equal_fn equal;
};
```

All callbacks are optional. A type binds only the behavior it supports.

Object binding does not allocate objects automatically. Constructors remain
typed functions owned by the concrete type.

### 1.4 Intrusive-list model

A list-aware object embeds its own `struct list_head`. List behavior is
described by:

```c
struct value_list_ops {
        size_t hook_offset;
        value_key_fn key;
        value_init_fn init;
        value_copy_fn copy;
        value_free_fn free_obj;
        size_t size;
};
```

The runtime uses `hook_offset` to convert between an object and its embedded
list hook. No container type or external wrapper is required.

An object can be:

- not list-aware
- string-keyed
- list-aware without a key
- list-aware with a domain-specific key

### 1.5 Key policy

String-keyed objects use `VALUE_LIST_BIND`.

Examples:

- `file.name`
- `node.ip`
- `carrier.name`
- `interface.name`
- `ipv4_address.ip`
- `ccmd.cmd`

Objects with no lookup identity use `VALUE_LIST_BIND_NOKEY`.

Objects with numeric, composite, normalized, or otherwise domain-specific keys
use `VALUE_LIST_BIND_DOMAIN_KEY`. Domain code provides handwritten lookup
functions.

The generic framework does not define:

- composite-key encoding
- partial matching
- key normalization
- duplicate resolution
- domain-specific indexing

### 1.6 Metadata model

Field state is represented by:

```c
struct value_config {
        uint64_t present;
        uint64_t required;
        uint64_t dirty;
        uint64_t validated;
        uint64_t invalid;

        time_t last_update;
        time_t last_validation;
        uint32_t validation_count;

        const char *source;
        char validation_error[256];
};
```

Each bit identifies a field defined by the domain model.

The metadata state supports:

- required-field completeness
- safe-use checks
- dirty tracking
- validation state
- invalid-field tracking
- accepting a new baseline
- source and error reporting

Metadata is optional. Types that do not require field-state tracking do not
embed `struct value_config` and do not use `VALUE_META_BIND`.

### 1.7 Filter-view model

Filter operations create borrowed views. They do not clone or own the source
objects.

The generic view uses `struct value_filter_ref`. This is the only supported
filter-wrapper representation. The former legacy `struct filter` API has been
retired.

For every view:

- the view owns only its wrapper allocation
- the source list owns the referenced object
- freeing a view must not free the referenced object
- the source object must outlive the view

Owned filtered results must be produced by explicitly cloning or mapping source
objects.

### 1.8 Directional value-pair model

Some M-Plane domain code needs paired list heads for directional values. This
is not part of the generic `value_base.h` object/list contract, but it is part
of the current production and `tests/25-files` domain value model.

The implementation uses one physical pair shape:

```c
struct value_pair {
        union {
                struct {
                        struct list_head tx;
                        struct list_head rx;
                };
                struct {
                        struct list_head ul;
                        struct list_head dl;
                };
                struct {
                        struct list_head first;
                        struct list_head second;
                };
        };
};
```

Semantic typedefs describe intent without duplicating implementation:

- `trx`: TX/RX value-list pair
- `udl`: UL/DL value-list pair
- `trx_filter2`: TX/RX pair of borrowed value-filter views
- `udl_filter2`: UL/DL pair of borrowed value-filter views

Generic helpers operate on `struct value_pair`:

- `VALUE_PAIR_HEAD()`
- `INIT_VALUE_PAIR_HEAD()`
- `value_pair_empty()`
- `value_filter_view_pair_free()`
- `value_filter_view_pair_foreach()`

The pair helpers are agent-domain conveniences, not generic libutils API.

Direction-specific names such as `TRX_HEAD()`, `UDL_HEAD()`,
`INIT_TRX_HEAD()`, and `INIT_UDL_HEAD()` are thin semantic wrappers over the
same generic pair initializer.

Value-pair containers own only list heads. They do not know the contained value
type. Type-specific ownership remains explicit; for example `net` UL/DL lists
use `free_net_udl()` and `foreach_net_udl()` because only the `net` layer knows
how to free or iterate `struct net` values correctly.

### 1.9 Thread-safety model

Generated list functions use the `_unlocked` suffix.

The framework performs no locking. The caller must provide synchronization when
a list can be accessed concurrently.

This keeps lock ownership at the application layer and avoids hidden lock
ordering inside generic code.

Generic `value_filter_view_*()` operations intentionally have no `_unlocked`
suffix (Policy B). A view is a caller-owned temporary container with no lock of
its own. The caller must synchronize access to the source typed list while
constructing the view and must ensure referenced objects remain valid while
the view is consumed. The absence of the suffix does not add implicit locking.

### 1.10 Ownership rules

Each allocated object has one owner.

For a list-bound object:

- allocation is owned by the concrete constructor
- list insertion transfers no ownership by itself
- list cleanup uses the bound `free_obj` callback
- an object must be removed from its list before separate destruction
- direct `type_fini()` or `type_free()` on a linked object is invalid; unlink it
  first or use the generated list cleanup operation
- embedded child values are finalized by their parent

For copied owned state:

- constructors duplicate caller-owned input where persistent ownership is
  required
- destructors free duplicated strings
- copy callbacks deeply copy owned allocations
- borrowed pointers remain borrowed and are copied as pointers
- a `NULL` owned field in the source clears the corresponding destination field
- nested owned lists and child objects are cloned before destination replacement

For borrowed filter views:

- wrappers are owned by the destination view list
- referenced values remain owned by the source list
- the source objects must outlive every wrapper that references them
- cloning a view clones only its wrappers, never its referenced values
- freeing a view releases only wrappers, using the allocator provenance stored
  in each wrapper

## 2. Implementation

### 2.1 Status codes

Framework operations use:

```c
VALUE_OK
VALUE_ERR_ARG
VALUE_ERR_MISSING
VALUE_ERR_INVALID
VALUE_ERR_NOMEM
```

Functions returning pointers use `NULL` for failure or absence where the API is
unambiguous.

### 2.2 Operation policies

Comparison modes are:

```c
VALUE_COMPARE_KEY
VALUE_COMPARE_SHALLOW
VALUE_COMPARE_DEEP
```

List-add policies are:

```c
VALUE_LIST_ADD_DEFAULT
VALUE_LIST_ADD_UNIQUE
VALUE_LIST_ADD_FIRST
VALUE_LIST_ADD_SORTED
VALUE_LIST_ADD_LAST
```

Comparison depth and insertion policy are separate concepts and must not share
domain-specific interpretation.

The macro-generated list API primarily exposes explicit operations such as
`type_add_tail_unlocked()` and `type_sort_unlocked()`. Models with historical
flag-taking APIs may translate these policies in handwritten wrappers.

### 2.3 Object declaration and binding

A header declares the object descriptor:

```c
VALUE_OBJECT_HEADER(type);
```

A C file defines it:

```c
VALUE_OBJECT_BIND(type,
                  init_cb,
                  fini_cb,
                  copy_cb,
                  validate_cb,
                  compare_cb,
                  equal_cb);
```

The generated symbol is:

```c
const struct value_object_ops type_object_ops;
```

Callbacks may be `NULL`.

### 2.4 Typed lifecycle wrappers

When a model needs standard typed finalization, copying, and freeing wrappers,
its header declares:

```c
VALUE_OBJECT_WRAPPERS_HEADER(type);
```

Its C file binds:

```c
VALUE_OBJECT_WRAPPERS_BIND(type, fini_cb, copy_cb, free_cb);
```

This generates:

```c
void type_fini(struct type *value);
int type_copy(struct type *dst, const struct type *src);
void type_free(struct type *value);
```

These wrappers are the standard lifecycle boundary for heap-allocated,
copyable typed objects. Domain code should not expose a second public copy or
finalization API with different semantics.

Callback contracts are:

- `fini_cb(value)` releases all resources owned by `value`, clears owned
  pointers, and leaves the object safe to finalize again. The object must be
  unlinked before direct finalization.
- `copy_cb(dst, src)` performs a complete deep copy of owned data and returns
  `VALUE_OK` or a framework error.
- `free_cb(value)` releases the object storage after finalization. It is
  normally `free`.

Every `copy_cb` must satisfy all of these rules:

- validate through the generated wrapper and use framework status codes
- treat `dst == src` as successful without changing the object
- allocate and clone all replacement state before modifying `dst`
- leave `dst` unchanged when any allocation or nested copy fails
- replace destination fields even when the corresponding source field is
  `NULL`
- preserve every intrusive-list hook already belonging to `dst`, including
  hooks in embedded child values
- never copy or reinitialize destination list hooks
- propagate nested copy failures instead of returning unconditional success

For nested objects or owned lists, build temporary state first. After all
allocations and child copies succeed, release the destination's old owned
state and splice or assign the completed replacement. Do not assign a
temporary list-head structure directly because its nodes still reference the
temporary head.

The generated `type_copy()` validates `dst` and `src`. The generated
`type_free()` accepts `NULL`.

The object and list bindings should consume the generated functions:

```c
VALUE_OBJECT_WRAPPERS_BIND(
        type, type_fini_obj, type_copy_obj, free);
VALUE_OBJECT_BIND(
        type, NULL, type_fini, type_copy, NULL, NULL, NULL);
VALUE_LIST_BIND(
        type, key, list, type_copy, type_free);
```

Constructors remain typed domain functions. Production descriptors currently
use `.init = NULL`; construction may require parameters that cannot be
represented by the generic init callback. This is a deliberate framework
boundary, not permission to define a second copy/finalization contract.

Whether descriptor fields should represent complete construction capabilities
or only partial runtime capabilities is deferred in `TBD.md`, section 5.

Use handwritten lifecycle functions only when the type is intentionally
non-copyable or an established domain operation has incompatible semantics.
Document such exceptions next to the binding.

### 2.5 String-keyed list declaration and binding

A keyed list type declares:

```c
VALUE_LIST_HEADER(type);
VALUE_LIST_LOOKUP_HEADER(type);
```

It binds:

```c
VALUE_LIST_BIND(type, key_member, hook_member, copy_cb, free_cb);
```

This generates:

```c
extern const struct value_list_ops type_list_ops;

size_t type_count(struct list_head *head);
struct type *type_nth_unlocked(struct list_head *head, size_t index);
void type_add_tail_unlocked(struct list_head *head, struct type *value);
struct type *type_lookup_unlocked(struct list_head *head, const char *key);
struct type *type_lookup_move_front_unlocked(
        struct list_head *head, const char *key);
void type_foreach_unlocked(...);
void *type_reduce_unlocked(...);
struct list_head *type_map_unlocked(...);
void type_sort_unlocked(...);
size_t type_remove_if_unlocked(...);
int type_filter_view_unlocked(...);
void type_free_list_unlocked(struct list_head *head);
```

The generated key callback treats a `NULL` string key as an empty string.

### 2.6 No-key and domain-key list binding

No-key binding:

```c
VALUE_LIST_BIND_NOKEY(type, hook_member, copy_cb, free_cb);
```

Domain-key binding:

```c
VALUE_LIST_BIND_DOMAIN_KEY(type, hook_member, copy_cb, free_cb);
```

Both generate functional list operations but no string lookup functions.

Domain-key binding currently has the same runtime mechanics as no-key binding.
Its distinct name documents that handwritten lookup exists because the object
has a meaningful non-string key.

### 2.7 Metadata declaration and binding

A metadata-aware type declares:

```c
VALUE_META_HEADER(type);
```

It binds:

```c
VALUE_META_BIND(type, meta_member, field_name_cb);
```

The generated API includes:

```c
type_meta()
type_meta_const()
type_has_field()
type_can_use_field()
type_meta_ready()
type_meta_has_any()
type_meta_has_all()
type_meta_dirty_any()
type_meta_invalid_any()
type_missing_required()
type_meta_complete()
type_dirty_fields()
type_is_dirty()
type_mark_present()
type_mark_dirty()
type_meta_mark_present_dirty()
type_meta_mark_invalid()
type_meta_clear_invalid()
type_meta_accept()
type_clear_field()
type_field_name()
```

`field_name_cb` may be `NULL`. In that case, generated field-name lookup returns
`"unknown"`.

### 2.8 Metadata field helper macros

For common field-aware domain setters and comparators, the framework provides:

```c
VALUE_FIELD_SET_CHANGED(type, value, member, field, new_value)
VALUE_FIELD_SET_CHANGED2(type, value, member1, member2, field,
                         new_value1, new_value2)
VALUE_FIELD_COMPARE(type, lhs, rhs, member, field)
VALUE_FIELD_COMPARE2(type, lhs, rhs, member1, member2, field)
```

These helpers depend only on the generated `VALUE_META_*` API for `type`.
`VALUE_FIELD_COMPARE*` assumes it is used inside an `int` comparator and may
return `1` or `-1` directly.

### 2.9 Generic list runtime

The typed macros delegate to these generic operations:

```c
value_hook()
value_hook_const()
value_entry()
value_list_nth_unlocked()
value_list_lookup_unlocked()
value_list_lookup_move_front_unlocked()
value_list_foreach_unlocked()
value_list_reduce_unlocked()
value_list_map_hook_unlocked()
value_list_sort_unlocked()
value_list_remove_if_unlocked()
value_list_free_unlocked()
value_list_count()
value_list_filter_view_unlocked()
```

The runtime receives `struct value_list_ops` and therefore does not need to know
the concrete type.

### 2.10 Sorting

`value_list_sort_unlocked()` sorts an intrusive list using a caller-provided
typed comparator.

The implementation delegates to the generic insertion sorter:

```c
list_sort_insertion()
```

The sorter is stable for elements that compare equal because only negative
comparisons insert before an existing item.

### 2.11 Remove and free

`type_remove_if_unlocked()` removes every matching object and invokes the bound
free callback when one is configured.

`type_free_list_unlocked()` removes every object and invokes the bound free
callback when one is configured.

If a list binding has no free callback, these operations detach objects without
destroying them. The caller remains responsible for their lifetime.

### 2.12 Map and reduce

Map operates on list hooks:

```c
struct list_head *type_map_unlocked(
        struct list_head *out,
        struct list_head *(*fn)(struct type *value, void *ctx),
        void *ctx,
        struct list_head *head);
```

The callback decides which hook is appended to the output list. This can be:

- a hook from a newly allocated clone
- a hook from another owned object
- a hook from a dedicated wrapper

The callback must not place one intrusive hook in two lists simultaneously.

Reduce threads an accumulator through every object:

```c
void *type_reduce_unlocked(
        struct list_head *head,
        void *(*fn)(void *acc, struct type *value, void *ctx),
        void *ctx,
        void *acc);
```

The accumulator and its ownership belong to the caller.

### 2.13 Generic filter-view operations

`type_filter_view_unlocked()` initializes a fresh output view, reads a typed
source list, and creates one `struct value_filter_ref` wrapper for every value
accepted by the predicate. The output must not contain live wrappers when the
operation begins. On allocation failure the operation frees all wrappers it
created and returns `VALUE_ERR_NOMEM`.

The resulting caller-owned view is consumed with the untyped Policy B API:

```c
value_filter_view_first()
value_filter_view_nth()
value_filter_view_foreach()
value_filter_view_reduce()
value_filter_view_sort()
value_filter_view_map()
value_filter_view_clone()
value_filter_view_free()
```

`first`, `nth`, `foreach`, `reduce`, `sort`, and `map` operate on referenced
domain objects. `sort` changes wrapper order only. `map` appends callback-returned
hooks to caller-owned output and does not consume the input view.

`clone` creates a second set of wrappers pointing to the same domain objects.
It does not clone those objects. `free` destroys wrappers only. Every view,
including a cloned view, must be freed exactly once before its source objects
are destroyed.

### 2.14 Memory operations

Framework-owned helper allocations use:

```c
struct value_memory_ops {
        value_calloc_fn calloc_fn;
        value_dealloc_fn dealloc_fn;
        void *ctx;
};
```

The allocator can be replaced with:

```c
value_memory_set()
```

and restored with:

```c
value_memory_reset()
```

Allocator replacement applies to framework helper allocations, especially
filter-view wrappers. Concrete domain constructors retain ownership of their
own allocation policy unless they explicitly use the framework allocator.

### 2.15 Concrete `value` type

`struct value` is:

```c
struct value {
        char *name;
        struct list_head list;
};
```

It is bound as a normal string-keyed framework object.

Public convenience functions are:

```c
struct value *value_new(const char *name);
void value_free(struct value *value);
struct value *value_add_new_tail(
        struct list_head *head, const char *name);
```

The constructor duplicates `name`.

### 2.16 Concrete `kvalue` type

`struct kvalue` is:

```c
struct kvalue {
        char *key;
        char *value;
        struct list_head list;
};
```

It is bound as a normal string-keyed framework object.

Public convenience functions are:

```c
struct kvalue *kvalue_new(const char *key, const char *value);
void kvalue_free(struct kvalue *kvalue);
struct kvalue *kvalue_add_new_tail(
        struct list_head *head,
        const char *key,
        const char *value);
```

The constructor duplicates both strings when non-`NULL`.

### 2.17 Build integration

The framework is part of `libutils`.

The Make build includes:

```make
value_types.c
value_base.c
```

The CMake build includes the same sources.

There is no legacy `values.[ch]` implementation and no compatibility ABI.

## 3. Usage and Examples

### 3.1 Minimal keyed object

Header:

```c
#include <value_base.h>

struct device {
        char *name;
        struct list_head list;
};

VALUE_OBJECT_HEADER(device);
VALUE_LIST_HEADER(device);
VALUE_LIST_LOOKUP_HEADER(device);

struct device *device_new(const char *name);
void device_free(struct device *device);
```

Implementation:

```c
static void device_init(void *obj, const char *name)
{
        struct device *device = obj;

        device->name = name ? strdup(name) : NULL;
        INIT_LIST_HEAD(&device->list);
}

static void device_fini(void *obj)
{
        struct device *device = obj;

        free(device->name);
}

VALUE_OBJECT_BIND(device, device_init, device_fini,
                  NULL, NULL, NULL, NULL);
VALUE_LIST_BIND(device, name, list, NULL, device_free);

struct device *device_new(const char *name)
{
        struct device *device = calloc(1, sizeof(*device));

        if (!device)
                return NULL;
        device_init(device, name);
        return device;
}

void device_free(struct device *device)
{
        if (!device)
                return;
        device_fini(device);
        free(device);
}
```

Usage:

```c
LIST_HEAD(devices);
struct device *device;

device = device_new("radio-0");
if (!device)
        return VALUE_ERR_NOMEM;

device_add_tail_unlocked(&devices, device);

device = device_lookup_unlocked(&devices, "radio-0");
device_free_list_unlocked(&devices);
```

### 3.2 Object with generated lifecycle wrappers

Header:

```c
struct sensor {
        char *name;
        int reading;
        struct list_head list;
};

VALUE_OBJECT_WRAPPERS_HEADER(sensor);
VALUE_OBJECT_HEADER(sensor);
VALUE_LIST_HEADER(sensor);
VALUE_LIST_LOOKUP_HEADER(sensor);
```

Implementation:

```c
static void sensor_fini_obj(struct sensor *sensor)
{
        free(sensor->name);
        sensor->name = NULL;
        INIT_LIST_HEAD(&sensor->list);
}

static int sensor_copy_obj(
        struct sensor *dst, const struct sensor *src)
{
        char *name;

        if (dst == src)
                return VALUE_OK;

        name = src->name ? strdup(src->name) : NULL;
        if (src->name && !name)
                return VALUE_ERR_NOMEM;

        free(dst->name);
        dst->name = name;
        dst->reading = src->reading;
        return VALUE_OK;
}

VALUE_OBJECT_WRAPPERS_BIND(
        sensor, sensor_fini_obj, sensor_copy_obj, free);
VALUE_OBJECT_BIND(
        sensor, NULL, sensor_fini, sensor_copy, NULL, NULL, NULL);
VALUE_LIST_BIND(sensor, name, list, sensor_copy, sensor_free);
```

### 3.3 Metadata-aware object

Define field bits:

```c
enum endpoint_field {
        ENDPOINT_F_NAME    = 1ULL << 0,
        ENDPOINT_F_ADDRESS = 1ULL << 1,
};
```

Define the object:

```c
struct endpoint {
        char *name;
        char *address;
        struct value_config meta;
        struct list_head list;
};

VALUE_OBJECT_HEADER(endpoint);
VALUE_LIST_HEADER(endpoint);
VALUE_LIST_LOOKUP_HEADER(endpoint);
VALUE_META_HEADER(endpoint);
```

Bind it:

```c
static const char *endpoint_field_name(uint64_t field)
{
        switch (field) {
        case ENDPOINT_F_NAME:
                return "name";
        case ENDPOINT_F_ADDRESS:
                return "address";
        default:
                return "unknown";
        }
}

VALUE_OBJECT_BIND(endpoint, endpoint_init, endpoint_fini,
                  endpoint_copy, endpoint_validate, NULL, NULL);
VALUE_LIST_BIND(endpoint, name, list, endpoint_copy, endpoint_free);
VALUE_META_BIND(endpoint, meta, endpoint_field_name);
```

Initialize and update metadata:

```c
value_config_init(&endpoint->meta,
                  ENDPOINT_F_NAME | ENDPOINT_F_ADDRESS);

endpoint_mark_present(endpoint, ENDPOINT_F_NAME);
endpoint_mark_dirty(endpoint, ENDPOINT_F_ADDRESS);

if (!endpoint_meta_ready(endpoint))
        return VALUE_ERR_MISSING;

if (endpoint_meta_invalid_any(endpoint, ENDPOINT_F_ADDRESS))
        return VALUE_ERR_INVALID;

endpoint_meta_accept(endpoint);
```

### 3.4 Domain-key object

An object keyed by a numeric identifier should not create a string key solely
for the framework:

```c
struct eaxcid {
        uint16_t eaxc_id;
        struct list_head list;
};

VALUE_OBJECT_HEADER(eaxcid);
VALUE_LIST_HEADER(eaxcid);
```

Bind list mechanics without generated string lookup:

```c
VALUE_OBJECT_BIND(eaxcid, eaxcid_init, eaxcid_fini,
                  eaxcid_copy, eaxcid_validate,
                  eaxcid_compare, eaxcid_equal);
VALUE_LIST_BIND_DOMAIN_KEY(
        eaxcid, list, eaxcid_copy, eaxcid_free);
```

Provide domain lookup:

```c
struct eaxcid *eaxcid_lookup_id(
        struct list_head *head, uint16_t eaxc_id)
{
        struct list_head *pos;

        list_for_each(pos, head) {
                struct eaxcid *value =
                        value_entry(pos, &eaxcid_list_ops);

                if (value->eaxc_id == eaxc_id)
                        return value;
        }
        return NULL;
}
```

### 3.5 No-key event list

```c
struct event {
        uint64_t timestamp;
        int code;
        struct list_head list;
};

VALUE_OBJECT_HEADER(event);
VALUE_LIST_HEADER(event);

VALUE_OBJECT_BIND(event, event_init, event_fini,
                  event_copy, NULL, NULL, NULL);
VALUE_LIST_BIND_NOKEY(event, list, event_copy, event_free);
```

The type receives count, foreach, map, reduce, sort, remove, filter-view, and
free-list operations, but no lookup API.

### 3.6 Foreach

```c
static void dump_device(struct device *device, void *ctx)
{
        FILE *out = ctx;

        fprintf(out, "%s\n", device->name);
}

device_foreach_unlocked(&devices, dump_device, stdout);
```

### 3.7 Sort

```c
static int device_name_compare(
        struct device *a, struct device *b, void *ctx)
{
        (void)ctx;
        return strcmp(a->name, b->name);
}

device_sort_unlocked(&devices, device_name_compare, NULL);
```

### 3.8 Remove

```c
static bool remove_named_device(
        const struct device *device, void *ctx)
{
        const char *name = ctx;

        return strcmp(device->name, name) == 0;
}

device_remove_if_unlocked(
        &devices, remove_named_device, "radio-0");
```

The matching object is removed and destroyed with the bound free callback.

### 3.9 Reduce

```c
static void *count_active(
        void *acc, struct device *device, void *ctx)
{
        size_t *count = acc;
        bool (*is_active)(const struct device *) = ctx;

        if (is_active(device))
                (*count)++;
        return count;
}

size_t active = 0;

device_reduce_unlocked(
        &devices, count_active, device_is_active, &active);
```

### 3.10 Borrowed filter view

```c
static bool device_is_selected(
        const struct device *device, void *ctx)
{
        const char *prefix = ctx;

        return strncmp(device->name, prefix, strlen(prefix)) == 0;
}

LIST_HEAD(selected);

if (device_filter_view_unlocked(
            &selected, &devices,
            device_is_selected, "radio-") != VALUE_OK)
        return VALUE_ERR_NOMEM;

struct device *first = value_filter_view_first(&selected);

value_filter_view_free(&selected);
```

`first` remains owned by `devices`. It must not be freed through the view.

### 3.11 Generic `value` list

```c
LIST_HEAD(arguments);

if (!value_add_new_tail(&arguments, "--verbose"))
        return VALUE_ERR_NOMEM;

if (!value_add_new_tail(&arguments, "--dry-run")) {
        value_free_list_unlocked(&arguments);
        return VALUE_ERR_NOMEM;
}

struct value *arg =
        value_lookup_unlocked(&arguments, "--verbose");

value_free_list_unlocked(&arguments);
```

### 3.12 Generic `kvalue` list

```c
LIST_HEAD(commands);

if (!kvalue_add_new_tail(
            &commands, "status", "show environment"))
        return VALUE_ERR_NOMEM;

struct kvalue *command =
        kvalue_lookup_unlocked(&commands, "status");

kvalue_free_list_unlocked(&commands);
```

### 3.13 Production binding pattern

Copyable production domain types follow this structure:

Header:

```c
struct radio *radio_new(const char *name);
VALUE_OBJECT_WRAPPERS_HEADER(radio);
VALUE_OBJECT_HEADER(radio);
VALUE_LIST_HEADER(radio);
VALUE_LIST_LOOKUP_HEADER(radio);
```

C file:

```c
VALUE_OBJECT_WRAPPERS_BIND(
        radio, radio_fini_obj, radio_copy_obj, free);
VALUE_OBJECT_BIND(
        radio, NULL, radio_fini, radio_copy, NULL, NULL, NULL);
VALUE_LIST_BIND(
        radio, name, list, radio_copy, radio_free);
```

Application code then uses:

```c
radio_add_tail_unlocked()
radio_count()
radio_lookup_unlocked()
radio_foreach_unlocked()
radio_sort_unlocked()
radio_remove_if_unlocked()
radio_free_list_unlocked()
```

The generic `value` and `kvalue` types follow the standard generated-wrapper
pattern. Their copy functions duplicate all owned strings and leave list hooks
untouched.

The production callbacks normalized to this contract currently include:

- `eaxcid` copies scalar state without changing its list hook
- `file`, `ccmd`, `model`, and `prbs_config` transactionally replace owned
  strings and scalar state
- `endpoint` preserves its outer list hook and the hook in its embedded
  `eaxcid`
- `carrier` preserves its outer list hook, deep-copies its owned name, and
  copies `struct value_config` metadata as part of the value snapshot
- `link` preserves its outer list hook, deep-copies owned identity strings, and
  shallow-copies borrowed relationship pointers to carrier, radio, endpoint,
  and element values
- `group` transactionally clones owned strings and borrowed filter wrappers
- `ru_config` builds temporary carrier and endpoint lists before replacing its
  nested lists
- `fhm_config` builds a temporary deep-copied `group` before replacing its
  owned child

Focused tests for these callbacks copy into already-linked destinations and
verify predecessor, successor, count, deep ownership, and nested state.

Macro presence alone does not prove lifecycle compliance. Remaining
wrapper-bound production types, including `node`, `component`, `ipv4_address`,
`interface`, `element`, `radio`, and `inventory`, must be audited and converted
to the same transactional contract. Until then, they use the typed API surface
but are not documented as verified implementations of every rule above.
The detailed audit scope and completion criteria are tracked in `TBD.md`,
section 3.

Current production bindings include:

- `node`
- `file`
- `ccmd`
- `model`
- `ru_config`
- `fhm_config`
- `prbs_config`
- `component`
- `ipv4_address`
- `interface`
- `element`
- `radio`
- `carrier`
- `eaxcid`
- `endpoint`
- `link`
- `inventory`
- `group`

All listed production bindings use the typed framework API. Some models still
declare underscore-prefixed domain helpers such as `_new_*`, `_copy_*`, and
`_compare_*`. These are not the removed framework compatibility ABI, but their
long-term public/private API status is deferred in `TBD.md`, section 2.

The separate `tests/25-files` prototype value model and test facade are not
part of the production framework contract. Their consolidation and unique
coverage audit are deferred in `TBD.md`, section 1.

## 4. Design Q/A

### 4.1 Should `VALUE_OBJECT_BIND` and `VALUE_OBJECT_WRAPPERS_BIND` be unified?

Decision: keep the low-level macros separate, but add a convenience macro only
for the common copyable-object case if boilerplate becomes a problem.

`VALUE_OBJECT_BIND` creates the framework descriptor:

```c
const struct value_object_ops type_object_ops;
```

It describes the type to generic code: type name, size, init, fini, copy,
validate, compare, and equal callbacks.

`VALUE_OBJECT_WRAPPERS_BIND` creates the typed lifecycle functions:

```c
type_fini()
type_copy()
type_free()
```

Most normal values use both:

```c
VALUE_OBJECT_WRAPPERS_BIND(type, type_fini_obj, type_copy_obj, free);
VALUE_OBJECT_BIND(type, NULL, type_fini, type_copy, NULL, NULL, NULL);
```

Unifying them completely would reduce boilerplate, but it would also hide
important lifecycle choices and make exceptions harder to express. Some values
are intentionally non-copyable or need descriptor callbacks that do not map
directly to public typed wrappers.

Recommended future improvement:

```c
VALUE_OBJECT_STANDARD_BIND(type, type_fini_obj, type_copy_obj, free);
```

This should be only a convenience macro that expands to the current wrapper
plus descriptor binding. `VALUE_OBJECT_BIND` must remain available for
non-standard values.

### 4.2 Why do values still handwrite `type_new()`?

Decision: keep constructors handwritten.

The framework describes object lifecycle, but construction is domain-specific.
A constructor decides:

- required arguments such as name, direction, IP address, or XML source
- default field values
- owned-string duplication
- list-head initialization
- embedded object or nested-list initialization
- metadata initial state
- rollback behavior when partial construction fails
- allocation source or policy

The framework may later provide a basic allocator such as `type_alloc()` or
`value_new(&type_object_ops)`, but that would only allocate and optionally call
`init`. It cannot replace domain constructors such as `endpoint_new()`,
`net_new()`, or `carrier_new()`.

### 4.3 Should every value use `VALUE_OBJECT_WRAPPERS_BIND`?

Decision: prefer wrappers for normal copyable values, but do not apply them
blindly.

`VALUE_OBJECT_WRAPPERS_BIND` publishes a typed lifecycle API:

```c
type_fini()
type_copy()
type_free()
```

Adding it says that all three operations are safe and have the framework
lifecycle semantics. In particular, `type_copy(dst, src)` must be valid for an
already-initialized destination, must preserve destination list membership and
embedded list hooks, must deep-copy owned data, and must have an explicit rule
for borrowed references and metadata.

Use wrappers when the type is a normal heap/list value and its lifecycle rules
are defined and tested. Do not add wrappers only for API symmetry. A value
should stay descriptor-only, or use explicit handwritten lifecycle APIs, when:

- it is intentionally non-copyable
- it is embedded-only or not safely heap-freeable through `type_free()`
- its copy operation would require unresolved relationship re-binding
- it has borrowed pointers without a documented lifetime/copy rule
- its current copy function is a domain merge/update operation rather than a
  full value snapshot copy
- its finalization must not be exposed as public typed lifecycle API

Callback implementations should use the `_obj` suffix to make the generated
public API boundary visible:

```c
static void type_fini_obj(struct type *value);
static int type_copy_obj(struct type *dst, const struct type *src);

VALUE_OBJECT_WRAPPERS_BIND(type, type_fini_obj, type_copy_obj, free);
```

`type_fini_obj()` criteria:

- releases every resource owned by `value`
- does not free the `value` object storage
- leaves pointer fields in a harmless state when practical
- handles outer and embedded list hooks by an explicit documented rule
- does not destroy borrowed objects

`type_copy_obj()` criteria:

- accepts an already-initialized destination
- handles `dst == src`
- preserves destination outer and embedded list hooks
- deep-copies owned data
- copies, clears, or re-resolves borrowed references by a documented rule
- copies or resets metadata by a documented rule
- keeps `dst` valid, and preferably unchanged, if allocation fails
- avoids raw whole-struct assignment unless hooks and owned pointers are
  explicitly restored

`type_free()` is generated by `VALUE_OBJECT_WRAPPERS_BIND`. The callback passed
as `free_cb` must free object storage exactly once and must be valid for the
allocation source used by that value's constructors. Use `free` for ordinary
heap objects.

Before adding wrappers to an existing type, add or update tests that copy into
an already-linked destination and verify list linkage, owned data, borrowed
reference behavior, metadata, and cleanup on failure.

## 5. Maintenance Rules

New framework changes must update this document when they alter:

- layer ownership
- macro contracts
- generated APIs
- ownership rules
- metadata semantics
- thread-safety rules
- public typed APIs

Migration history, rejected alternatives, and temporary prototype status do not
belong in this final-state design document.

Deferred decisions, alternatives, tradeoffs, and completion criteria are
maintained in `TBD.md`.
