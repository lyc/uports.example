#include <stdio.h>
#include <string.h>

#include "value_codec.h"

static int codec_diag(
        struct value_codec_diag *diag,
        int code,
        enum value_codec_stage stage,
        const char *type_name,
        const char *field_path,
        const char *message)
{
        if (diag) {
                value_codec_diag_init(diag);
                diag->code = code;
                diag->stage = stage;
                if (type_name)
                        snprintf(diag->type_name, sizeof(diag->type_name),
                                 "%s", type_name);
                if (field_path)
                        snprintf(diag->field_path, sizeof(diag->field_path),
                                 "%s", field_path);
                if (message)
                        snprintf(diag->message, sizeof(diag->message),
                                 "%s", message);
        }
        return code;
}

void value_codec_diag_init(struct value_codec_diag *diag)
{
        if (diag)
                memset(diag, 0, sizeof(*diag));
}

void value_encoded_init(struct value_encoded *encoded)
{
        if (encoded)
                memset(encoded, 0, sizeof(*encoded));
}

void value_encoded_fini(struct value_encoded *encoded)
{
        if (!encoded)
                return;
        if (encoded->data && encoded->release)
                encoded->release(encoded->data, encoded->release_context);
        value_encoded_init(encoded);
}

static const struct value_codec_ops *codec_lookup_raw(
        const struct value_codec_registry *registry,
        const char *type_name,
        uint32_t schema_version)
{
        size_t i;

        if (!registry || !type_name)
                return NULL;
        for (i = 0; i < registry->codec_count; i++) {
                const struct value_codec_ops *codec = registry->codecs[i].codec;

                if (codec && codec->schema_version == schema_version &&
                    strcmp(codec->type_name, type_name) == 0)
                        return codec;
        }
        return NULL;
}

static const struct value_serializer_entry *serializer_lookup_raw(
        const struct value_codec_registry *registry,
        const char *name)
{
        size_t i;

        if (!registry || !name)
                return NULL;
        for (i = 0; i < registry->serializer_count; i++) {
                const struct value_serializer_entry *entry =
                        &registry->serializers[i];

                if (entry->serializer &&
                    strcmp(entry->serializer->name, name) == 0)
                        return entry;
        }
        return NULL;
}

int value_codec_registry_init(
        struct value_codec_registry *registry,
        struct value_codec_entry *codec_storage,
        size_t codec_capacity,
        struct value_serializer_entry *serializer_storage,
        size_t serializer_capacity)
{
        if (!registry || !codec_storage || !codec_capacity ||
            !serializer_storage || !serializer_capacity)
                return VALUE_ERR_ARG;

        memset(codec_storage, 0, codec_capacity * sizeof(*codec_storage));
        memset(serializer_storage, 0,
               serializer_capacity * sizeof(*serializer_storage));
        memset(registry, 0, sizeof(*registry));
        registry->codecs = codec_storage;
        registry->codec_capacity = codec_capacity;
        registry->serializers = serializer_storage;
        registry->serializer_capacity = serializer_capacity;
        return VALUE_OK;
}

int value_codec_register(
        struct value_codec_registry *registry,
        const struct value_codec_ops *codec,
        struct value_codec_diag *diag)
{
        if (!registry || !codec || !codec->type_name ||
            !codec->type_name[0] || !codec->schema_version)
                return codec_diag(diag, VALUE_ERR_ARG,
                                  VALUE_CODEC_STAGE_REGISTRY,
                                  codec ? codec->type_name : NULL, NULL,
                                  "invalid codec registration");
        if (registry->sealed)
                return codec_diag(diag, VALUE_ERR_STATE,
                                  VALUE_CODEC_STAGE_REGISTRY,
                                  codec->type_name, NULL,
                                  "registry is sealed");
        if (codec_lookup_raw(registry, codec->type_name,
                             codec->schema_version))
                return codec_diag(diag, VALUE_ERR_EXISTS,
                                  VALUE_CODEC_STAGE_REGISTRY,
                                  codec->type_name, NULL,
                                  "codec is already registered");
        if (registry->codec_count >= registry->codec_capacity)
                return codec_diag(diag, VALUE_ERR_BOUNDS,
                                  VALUE_CODEC_STAGE_REGISTRY,
                                  codec->type_name, NULL,
                                  "codec registry capacity exhausted");

        registry->codecs[registry->codec_count++].codec = codec;
        value_codec_diag_init(diag);
        return VALUE_OK;
}

int value_serializer_register(
        struct value_codec_registry *registry,
        const struct value_serializer_ops *serializer,
        void *context,
        struct value_codec_diag *diag)
{
        struct value_serializer_entry *entry;

        if (!registry || !serializer || !serializer->name ||
            !serializer->name[0] || !serializer->encode || !serializer->decode)
                return codec_diag(diag, VALUE_ERR_ARG,
                                  VALUE_CODEC_STAGE_REGISTRY,
                                  NULL, NULL,
                                  "invalid serializer registration");
        if (registry->sealed)
                return codec_diag(diag, VALUE_ERR_STATE,
                                  VALUE_CODEC_STAGE_REGISTRY,
                                  NULL, NULL, "registry is sealed");
        if (serializer_lookup_raw(registry, serializer->name))
                return codec_diag(diag, VALUE_ERR_EXISTS,
                                  VALUE_CODEC_STAGE_REGISTRY,
                                  NULL, NULL,
                                  "serializer is already registered");
        if (registry->serializer_count >= registry->serializer_capacity)
                return codec_diag(diag, VALUE_ERR_BOUNDS,
                                  VALUE_CODEC_STAGE_REGISTRY,
                                  NULL, NULL,
                                  "serializer registry capacity exhausted");

        entry = &registry->serializers[registry->serializer_count++];
        entry->serializer = serializer;
        entry->context = context;
        value_codec_diag_init(diag);
        return VALUE_OK;
}

static bool codec_kind_valid(enum value_codec_kind kind)
{
        return kind >= VALUE_CODEC_BOOL && kind <= VALUE_CODEC_LIST;
}

static uint64_t codec_unsigned_max(enum value_codec_kind kind)
{
        switch (kind) {
        case VALUE_CODEC_U8:
                return UINT8_MAX;
        case VALUE_CODEC_U16:
                return UINT16_MAX;
        case VALUE_CODEC_U32:
                return UINT32_MAX;
        case VALUE_CODEC_U64:
                return UINT64_MAX;
        default:
                return 0;
        }
}

static int codec_field_validate(
        const struct value_codec_ops *codec,
        const struct value_codec_field *field,
        struct value_codec_diag *diag)
{
        size_t i;
        size_t j;

        if (!field->name || !field->name[0] || !codec_kind_valid(field->kind))
                return codec_diag(diag, VALUE_ERR_INVALID,
                                  VALUE_CODEC_STAGE_SCHEMA,
                                  codec->type_name,
                                  field->name, "invalid field name or kind");
        if (!field->access || !field->access->is_present ||
            !field->access->clear)
                return codec_diag(diag, VALUE_ERR_INVALID,
                                  VALUE_CODEC_STAGE_SCHEMA,
                                  codec->type_name, field->name,
                                  "field presence/clear access is required");

        if (field->kind == VALUE_CODEC_LIST) {
                if (!field->nested_codec || !field->max_count ||
                    !field->list_access || !field->list_access->count ||
                    !field->list_access->at ||
                    !field->list_access->add_owned ||
                    !field->list_access->set_present)
                        return codec_diag(diag, VALUE_ERR_INVALID,
                                          VALUE_CODEC_STAGE_SCHEMA,
                                          codec->type_name, field->name,
                                          "invalid list descriptor");
        } else {
                if (!field->access->get || !field->access->set)
                        return codec_diag(diag, VALUE_ERR_INVALID,
                                          VALUE_CODEC_STAGE_SCHEMA,
                                          codec->type_name, field->name,
                                          "field get/set access is required");
                if (field->list_access)
                        return codec_diag(diag, VALUE_ERR_INVALID,
                                          VALUE_CODEC_STAGE_SCHEMA,
                                          codec->type_name, field->name,
                                          "non-list field has list access");
        }

        if (field->kind == VALUE_CODEC_OBJECT && !field->nested_codec)
                return codec_diag(diag, VALUE_ERR_INVALID,
                                  VALUE_CODEC_STAGE_SCHEMA,
                                  codec->type_name, field->name,
                                  "object field has no nested codec");
        if (field->kind != VALUE_CODEC_OBJECT &&
            field->kind != VALUE_CODEC_LIST && field->nested_codec)
                return codec_diag(diag, VALUE_ERR_INVALID,
                                  VALUE_CODEC_STAGE_SCHEMA,
                                  codec->type_name, field->name,
                                  "scalar field has nested codec");

        if (field->kind >= VALUE_CODEC_U8 && field->kind <= VALUE_CODEC_U64) {
                if (field->min_value > field->max_value ||
                    field->max_value > codec_unsigned_max(field->kind))
                        return codec_diag(diag, VALUE_ERR_INVALID,
                                          VALUE_CODEC_STAGE_SCHEMA,
                                          codec->type_name, field->name,
                                          "invalid unsigned range");
        }

        if (field->kind == VALUE_CODEC_STRING && !field->max_length)
                return codec_diag(diag, VALUE_ERR_INVALID,
                                  VALUE_CODEC_STAGE_SCHEMA,
                                  codec->type_name, field->name,
                                  "bounded string requires max_length");
        if (field->kind == VALUE_CODEC_UUID &&
            field->max_length != 36)
                return codec_diag(diag, VALUE_ERR_INVALID,
                                  VALUE_CODEC_STAGE_SCHEMA,
                                  codec->type_name, field->name,
                                  "UUID max_length must be 36");

        if (field->kind == VALUE_CODEC_ENUM) {
                if (!field->enum_values || !field->enum_count)
                        return codec_diag(diag, VALUE_ERR_INVALID,
                                          VALUE_CODEC_STAGE_SCHEMA,
                                          codec->type_name, field->name,
                                          "enum table is required");
                for (i = 0; i < field->enum_count; i++) {
                        const struct value_codec_enum *item =
                                &field->enum_values[i];

                        if (!item->symbol || !item->symbol[0])
                                return codec_diag(diag, VALUE_ERR_INVALID,
                                                  VALUE_CODEC_STAGE_SCHEMA,
                                                  codec->type_name, field->name,
                                                  "empty enum symbol");
                        for (j = i + 1; j < field->enum_count; j++) {
                                const struct value_codec_enum *other =
                                        &field->enum_values[j];

                                if (item->value == other->value ||
                                    (other->symbol &&
                                     strcmp(item->symbol, other->symbol) == 0))
                                        return codec_diag(
                                                diag, VALUE_ERR_INVALID,
                                                VALUE_CODEC_STAGE_SCHEMA,
                                                codec->type_name, field->name,
                                                "duplicate enum value or symbol");
                        }
                }
        } else if (field->enum_values || field->enum_count) {
                return codec_diag(diag, VALUE_ERR_INVALID,
                                  VALUE_CODEC_STAGE_SCHEMA,
                                  codec->type_name, field->name,
                                  "non-enum field has enum table");
        }
        return VALUE_OK;
}

static int codec_schema_validate(
        const struct value_codec_ops *codec,
        struct value_codec_diag *diag)
{
        size_t i;
        size_t j;
        int rc;

        if (!codec || !codec->type_name || !codec->type_name[0] ||
            !codec->schema_version || !codec->new_obj || !codec->free_obj ||
            !codec->validate || (codec->field_count && !codec->fields))
                return codec_diag(diag, VALUE_ERR_INVALID,
                                  VALUE_CODEC_STAGE_SCHEMA,
                                  codec ? codec->type_name : NULL, NULL,
                                  "incomplete codec descriptor");

        for (i = 0; i < codec->field_count; i++) {
                rc = codec_field_validate(codec, &codec->fields[i], diag);
                if (rc != VALUE_OK)
                        return rc;
                for (j = i + 1; j < codec->field_count; j++) {
                        if (codec->fields[j].name &&
                            strcmp(codec->fields[i].name,
                                   codec->fields[j].name) == 0)
                                return codec_diag(diag, VALUE_ERR_INVALID,
                                                  VALUE_CODEC_STAGE_SCHEMA,
                                                  codec->type_name,
                                                  codec->fields[i].name,
                                                  "duplicate field name");
                }
        }
        return VALUE_OK;
}

static size_t codec_index(
        const struct value_codec_registry *registry,
        const struct value_codec_ops *codec)
{
        size_t i;

        for (i = 0; i < registry->codec_count; i++) {
                if (registry->codecs[i].codec == codec)
                        return i;
        }
        return registry->codec_count;
}

static int codec_graph_visit(
        const struct value_codec_registry *registry,
        size_t index,
        unsigned char *state,
        struct value_codec_diag *diag)
{
        const struct value_codec_ops *codec = registry->codecs[index].codec;
        size_t i;

        if (state[index] == 1)
                return codec_diag(diag, VALUE_ERR_INVALID,
                                  VALUE_CODEC_STAGE_SCHEMA,
                                  codec->type_name, NULL,
                                  "recursive codec graph");
        if (state[index] == 2)
                return VALUE_OK;

        state[index] = 1;
        for (i = 0; i < codec->field_count; i++) {
                const struct value_codec_ops *nested =
                        codec->fields[i].nested_codec;
                size_t nested_index;
                int rc;

                if (!nested)
                        continue;
                nested_index = codec_index(registry, nested);
                if (nested_index == registry->codec_count)
                        return codec_diag(diag, VALUE_ERR_NOT_FOUND,
                                          VALUE_CODEC_STAGE_SCHEMA,
                                          codec->type_name,
                                          codec->fields[i].name,
                                          "nested codec is not registered");
                rc = codec_graph_visit(registry, nested_index, state, diag);
                if (rc != VALUE_OK)
                        return rc;
        }
        state[index] = 2;
        return VALUE_OK;
}

int value_codec_registry_seal(
        struct value_codec_registry *registry,
        struct value_codec_diag *diag)
{
        size_t i;
        int rc;

        if (!registry)
                return codec_diag(diag, VALUE_ERR_ARG,
                                  VALUE_CODEC_STAGE_REGISTRY,
                                  NULL, NULL, "registry is required");
        if (registry->sealed)
                return codec_diag(diag, VALUE_ERR_STATE,
                                  VALUE_CODEC_STAGE_REGISTRY,
                                  NULL, NULL, "registry is already sealed");
        if (!registry->codec_count || !registry->serializer_count)
                return codec_diag(diag, VALUE_ERR_MISSING,
                                  VALUE_CODEC_STAGE_REGISTRY,
                                  NULL, NULL,
                                  "codec and serializer registrations are required");

        for (i = 0; i < registry->codec_count; i++) {
                rc = codec_schema_validate(registry->codecs[i].codec, diag);
                if (rc != VALUE_OK)
                        return rc;
        }

        {
                unsigned char state[registry->codec_count];

                memset(state, 0, sizeof(state));
                for (i = 0; i < registry->codec_count; i++) {
                        rc = codec_graph_visit(registry, i, state, diag);
                        if (rc != VALUE_OK)
                                return rc;
                }
        }

        registry->sealed = true;
        value_codec_diag_init(diag);
        return VALUE_OK;
}

void value_codec_registry_fini(struct value_codec_registry *registry)
{
        if (!registry)
                return;
        if (registry->codecs)
                memset(registry->codecs, 0,
                       registry->codec_capacity * sizeof(*registry->codecs));
        if (registry->serializers)
                memset(registry->serializers, 0,
                       registry->serializer_capacity *
                       sizeof(*registry->serializers));
        memset(registry, 0, sizeof(*registry));
}

const struct value_codec_ops *value_codec_lookup(
        const struct value_codec_registry *registry,
        const char *type_name,
        uint32_t schema_version)
{
        if (!registry || !registry->sealed)
                return NULL;
        return codec_lookup_raw(registry, type_name, schema_version);
}

const struct value_serializer_entry *value_serializer_lookup(
        const struct value_codec_registry *registry,
        const char *name)
{
        if (!registry || !registry->sealed)
                return NULL;
        return serializer_lookup_raw(registry, name);
}

int value_encode(
        const struct value_codec_registry *registry,
        const char *serializer_name,
        const char *type_name,
        uint32_t schema_version,
        const void *object,
        size_t max_output,
        struct value_encoded *output,
        struct value_codec_diag *diag)
{
        const struct value_serializer_entry *serializer;
        const struct value_codec_ops *codec;
        int rc;

        if (output)
                value_encoded_init(output);
        if (!registry || !serializer_name || !type_name || !object ||
            !max_output || !output)
                return codec_diag(diag, VALUE_ERR_ARG,
                                  VALUE_CODEC_STAGE_ENCODE,
                                  type_name, NULL, "invalid encode arguments");
        if (!registry->sealed)
                return codec_diag(diag, VALUE_ERR_STATE,
                                  VALUE_CODEC_STAGE_ENCODE,
                                  type_name, NULL, "registry is not sealed");

        codec = codec_lookup_raw(registry, type_name, schema_version);
        if (!codec)
                return codec_diag(diag, VALUE_ERR_NOT_FOUND,
                                  VALUE_CODEC_STAGE_ENCODE,
                                  type_name, NULL, "codec is not registered");
        serializer = serializer_lookup_raw(registry, serializer_name);
        if (!serializer)
                return codec_diag(diag, VALUE_ERR_NOT_FOUND,
                                  VALUE_CODEC_STAGE_ENCODE,
                                  type_name, NULL,
                                  "serializer is not registered");

        rc = codec->validate(object);
        if (rc != VALUE_OK)
                return codec_diag(diag, rc, VALUE_CODEC_STAGE_VALIDATE,
                                  type_name, NULL,
                                  "object validation failed before encode");

        value_codec_diag_init(diag);
        rc = serializer->serializer->encode(
                registry, codec, object, max_output, output, diag,
                serializer->context);
        if (rc != VALUE_OK) {
                value_encoded_fini(output);
                if (diag && diag->code == VALUE_OK)
                        codec_diag(diag, rc, VALUE_CODEC_STAGE_ENCODE,
                                   type_name, NULL, "serializer encode failed");
                return rc;
        }
        if (!output->data || !output->release || output->length > max_output) {
                value_encoded_fini(output);
                return codec_diag(diag, VALUE_ERR_CODEC,
                                  VALUE_CODEC_STAGE_ENCODE,
                                  type_name, NULL,
                                  "serializer returned invalid output");
        }
        return VALUE_OK;
}

int value_decode(
        const struct value_codec_registry *registry,
        const char *serializer_name,
        const char *expected_type,
        uint32_t expected_version,
        const void *input,
        size_t input_length,
        void **output_object,
        struct value_codec_diag *diag)
{
        const struct value_serializer_entry *serializer;
        const struct value_codec_ops *codec;
        void *object = NULL;
        int rc;

        if (output_object)
                *output_object = NULL;
        if (!registry || !serializer_name || !expected_type || !input ||
            !input_length || !output_object)
                return codec_diag(diag, VALUE_ERR_ARG,
                                  VALUE_CODEC_STAGE_DECODE,
                                  expected_type, NULL,
                                  "invalid decode arguments");
        if (!registry->sealed)
                return codec_diag(diag, VALUE_ERR_STATE,
                                  VALUE_CODEC_STAGE_DECODE,
                                  expected_type, NULL,
                                  "registry is not sealed");

        codec = codec_lookup_raw(registry, expected_type, expected_version);
        if (!codec)
                return codec_diag(diag, VALUE_ERR_NOT_FOUND,
                                  VALUE_CODEC_STAGE_DECODE,
                                  expected_type, NULL,
                                  "codec is not registered");
        serializer = serializer_lookup_raw(registry, serializer_name);
        if (!serializer)
                return codec_diag(diag, VALUE_ERR_NOT_FOUND,
                                  VALUE_CODEC_STAGE_DECODE,
                                  expected_type, NULL,
                                  "serializer is not registered");

        value_codec_diag_init(diag);
        rc = serializer->serializer->decode(
                registry, codec, input, input_length, &object, diag,
                serializer->context);
        if (rc != VALUE_OK || !object) {
                if (object)
                        codec->free_obj(object);
                if (rc == VALUE_OK)
                        rc = VALUE_ERR_CODEC;
                if (diag && diag->code == VALUE_OK)
                        codec_diag(diag, rc, VALUE_CODEC_STAGE_DECODE,
                                   expected_type, NULL,
                                   "serializer decode failed");
                return rc;
        }

        rc = codec->validate_decoded ? codec->validate_decoded(object) :
                                       codec->validate(object);
        if (rc != VALUE_OK) {
                codec->free_obj(object);
                return codec_diag(diag, rc, VALUE_CODEC_STAGE_VALIDATE,
                                  expected_type, NULL,
                                  "decoded object validation failed");
        }
        *output_object = object;
        return VALUE_OK;
}
