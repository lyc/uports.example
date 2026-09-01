#ifndef VALUE_CODEC_H
#define VALUE_CODEC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "value_base.h"

#define VALUE_CODEC_DIAG_TYPE_MAX 64
#define VALUE_CODEC_DIAG_PATH_MAX 256
#define VALUE_CODEC_DIAG_MESSAGE_MAX 256

enum value_codec_stage {
        VALUE_CODEC_STAGE_NONE = 0,
        VALUE_CODEC_STAGE_REGISTRY,
        VALUE_CODEC_STAGE_SCHEMA,
        VALUE_CODEC_STAGE_ENCODE,
        VALUE_CODEC_STAGE_DECODE,
        VALUE_CODEC_STAGE_VALIDATE,
};

struct value_codec_diag {
        int code;
        enum value_codec_stage stage;
        char type_name[VALUE_CODEC_DIAG_TYPE_MAX];
        char field_path[VALUE_CODEC_DIAG_PATH_MAX];
        char message[VALUE_CODEC_DIAG_MESSAGE_MAX];
};

void value_codec_diag_init(struct value_codec_diag *diag);

enum value_codec_kind {
        VALUE_CODEC_BOOL = 0,
        VALUE_CODEC_U8,
        VALUE_CODEC_U16,
        VALUE_CODEC_U32,
        VALUE_CODEC_U64,
        VALUE_CODEC_ENUM,
        VALUE_CODEC_STRING,
        VALUE_CODEC_UUID,
        VALUE_CODEC_MAC,
        VALUE_CODEC_IPV4,
        VALUE_CODEC_OBJECT,
        VALUE_CODEC_LIST,
};

struct value_codec_text {
        const char *data;
        size_t length;
};

struct value_codec_bytes {
        const uint8_t *data;
        size_t length;
};

struct value_codec_value {
        enum value_codec_kind kind;
        union {
                bool boolean;
                uint64_t unsigned_integer;
                struct value_codec_text text;
                struct value_codec_bytes bytes;
                const void *object;
        } as;
};

typedef bool (*value_codec_present_fn)(const void *object);
typedef int (*value_codec_get_fn)(
        const void *object, struct value_codec_value *value);
typedef int (*value_codec_set_fn)(
        void *object, const struct value_codec_value *value);
typedef int (*value_codec_clear_fn)(void *object);

struct value_codec_field_ops {
        value_codec_present_fn is_present;
        value_codec_get_fn get;
        value_codec_set_fn set;
        value_codec_clear_fn clear;
};

typedef size_t (*value_codec_list_count_fn)(const void *object);
typedef const void *(*value_codec_list_at_fn)(
        const void *object, size_t index);
typedef int (*value_codec_list_add_owned_fn)(void *object, void *element);
typedef int (*value_codec_list_set_present_fn)(void *object);

struct value_codec_list_ops {
        value_codec_list_count_fn count;
        value_codec_list_at_fn at;
        value_codec_list_add_owned_fn add_owned;
        value_codec_list_set_present_fn set_present;
};

struct value_codec_enum {
        uint64_t value;
        const char *symbol;
};

struct value_codec_ops;

struct value_codec_field {
        const char *name;
        enum value_codec_kind kind;
        bool required;

        uint64_t min_value;
        uint64_t max_value;
        size_t max_length;
        size_t max_count;

        const struct value_codec_enum *enum_values;
        size_t enum_count;

        const struct value_codec_ops *nested_codec;
        const struct value_codec_field_ops *access;
        const struct value_codec_list_ops *list_access;
};

typedef int (*value_codec_new_fn)(void **object);
typedef void (*value_codec_free_fn)(void *object);
typedef int (*value_codec_validate_fn)(const void *object);
typedef int (*value_codec_validate_decoded_fn)(void *object);

struct value_codec_ops {
        const char *type_name;
        uint32_t schema_version;

        const struct value_codec_field *fields;
        size_t field_count;

        value_codec_new_fn new_obj;
        value_codec_free_fn free_obj;
        value_codec_validate_fn validate;
        value_codec_validate_decoded_fn validate_decoded;
};

#define VALUE_CODEC_HEADER(type) \
        extern const struct value_codec_ops type ## _codec_ops

#define VALUE_CODEC_BIND_EX(type, version, field_array, new_cb, free_cb, \
                            validate_cb, validate_decoded_cb)		\
        const struct value_codec_ops type ## _codec_ops = {		\
                .type_name = #type,					\
                .schema_version = (version),				\
                .fields = (field_array),				\
                .field_count = sizeof(field_array) / sizeof((field_array)[0]), \
                .new_obj = (new_cb),					\
                .free_obj = (free_cb),					\
                .validate = (validate_cb),				\
                .validate_decoded = (validate_decoded_cb),		\
        }

#define VALUE_CODEC_BIND(type, version, field_array, new_cb, free_cb, validate_cb) \
        VALUE_CODEC_BIND_EX(type, version, field_array, new_cb, free_cb, \
                            validate_cb, NULL)

struct value_encoded {
        void *data;
        size_t length;
        void (*release)(void *data, void *context);
        void *release_context;
};

void value_encoded_init(struct value_encoded *encoded);
void value_encoded_fini(struct value_encoded *encoded);

struct value_codec_registry;

struct value_serializer_ops {
        const char *name;

        int (*encode)(
                const struct value_codec_registry *registry,
                const struct value_codec_ops *codec,
                const void *object,
                size_t max_output,
                struct value_encoded *output,
                struct value_codec_diag *diag,
                void *context);

        int (*decode)(
                const struct value_codec_registry *registry,
                const struct value_codec_ops *codec,
                const void *input,
                size_t input_length,
                void **object,
                struct value_codec_diag *diag,
                void *context);
};

struct value_codec_entry {
        const struct value_codec_ops *codec;
};

struct value_serializer_entry {
        const struct value_serializer_ops *serializer;
        void *context;
};

struct value_codec_registry {
        struct value_codec_entry *codecs;
        size_t codec_capacity;
        size_t codec_count;

        struct value_serializer_entry *serializers;
        size_t serializer_capacity;
        size_t serializer_count;

        bool sealed;
};

int value_codec_registry_init(
        struct value_codec_registry *registry,
        struct value_codec_entry *codec_storage,
        size_t codec_capacity,
        struct value_serializer_entry *serializer_storage,
        size_t serializer_capacity);

int value_codec_register(
        struct value_codec_registry *registry,
        const struct value_codec_ops *codec,
        struct value_codec_diag *diag);

int value_serializer_register(
        struct value_codec_registry *registry,
        const struct value_serializer_ops *serializer,
        void *context,
        struct value_codec_diag *diag);

int value_codec_registry_seal(
        struct value_codec_registry *registry,
        struct value_codec_diag *diag);

void value_codec_registry_fini(struct value_codec_registry *registry);

const struct value_codec_ops *value_codec_lookup(
        const struct value_codec_registry *registry,
        const char *type_name,
        uint32_t schema_version);

const struct value_serializer_entry *value_serializer_lookup(
        const struct value_codec_registry *registry,
        const char *name);

int value_encode(
        const struct value_codec_registry *registry,
        const char *serializer_name,
        const char *type_name,
        uint32_t schema_version,
        const void *object,
        size_t max_output,
        struct value_encoded *output,
        struct value_codec_diag *diag);

int value_decode(
        const struct value_codec_registry *registry,
        const char *serializer_name,
        const char *expected_type,
        uint32_t expected_version,
        const void *input,
        size_t input_length,
        void **output_object,
        struct value_codec_diag *diag);

#endif /* VALUE_CODEC_H */
