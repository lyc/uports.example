#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <value_codec.h>

#include "t_support.h"

struct codec_sample {
	uint32_t number;
	int valid;
};

struct serializer_probe {
	int encode_rc;
	int decode_rc;
	int return_null;
	int allocate_then_fail;
	int invalid_output;
	int release_count;
	int free_count;
};

static struct serializer_probe *active_probe;

static bool number_present(const void *object)
{
	return object != NULL;
}

static int number_get(const void *object, struct value_codec_value *value)
{
	const struct codec_sample *sample = object;

	if (!sample || !value)
		return VALUE_ERR_ARG;
	value->kind = VALUE_CODEC_U32;
	value->as.unsigned_integer = sample->number;
	return VALUE_OK;
}

static int number_set(void *object, const struct value_codec_value *value)
{
	struct codec_sample *sample = object;

	if (!sample || !value || value->kind != VALUE_CODEC_U32)
		return VALUE_ERR_ARG;
	sample->number = (uint32_t)value->as.unsigned_integer;
	return VALUE_OK;
}

static int number_clear(void *object)
{
	struct codec_sample *sample = object;

	if (!sample)
		return VALUE_ERR_ARG;
	sample->number = 0;
	return VALUE_OK;
}

static const struct value_codec_field_ops number_access = {
	.is_present = number_present,
	.get = number_get,
	.set = number_set,
	.clear = number_clear,
};

static const struct value_codec_field sample_fields[] = {
	{
		.name = "number",
		.kind = VALUE_CODEC_U32,
		.required = true,
		.min_value = 0,
		.max_value = UINT32_MAX,
		.access = &number_access,
	},
};

static int sample_new(void **object)
{
	struct codec_sample *sample;

	if (!object)
		return VALUE_ERR_ARG;
	sample = calloc(1, sizeof(*sample));
	if (!sample)
		return VALUE_ERR_NOMEM;
	sample->valid = 1;
	*object = sample;
	return VALUE_OK;
}

static void sample_free(void *object)
{
	if (active_probe)
		active_probe->free_count++;
	free(object);
}

static int sample_validate(const void *object)
{
	const struct codec_sample *sample = object;

	return sample && sample->valid ? VALUE_OK : VALUE_ERR_INVALID;
}

VALUE_CODEC_BIND(codec_sample, 1, sample_fields,
		 sample_new, sample_free, sample_validate);

static int split_validate_calls;
static int split_validate_decoded_calls;

static int split_validate(const void *object)
{
	split_validate_calls++;
	return sample_validate(object);
}

static int split_validate_decoded(void *object)
{
	struct codec_sample *sample = object;

	split_validate_decoded_calls++;
	if (!sample)
		return VALUE_ERR_ARG;
	sample->number++;
	return sample_validate(sample);
}

VALUE_CODEC_BIND_EX(codec_split_sample, 1, sample_fields,
		    sample_new, sample_free, split_validate,
		    split_validate_decoded);

static void encoded_release(void *data, void *context)
{
	struct serializer_probe *probe = context;

	probe->release_count++;
	free(data);
}

static int mock_encode(
	const struct value_codec_registry *registry,
	const struct value_codec_ops *codec, const void *object,
	size_t max_output, struct value_encoded *output,
	struct value_codec_diag *diag, void *context)
{
	const struct codec_sample *sample = object;
	struct serializer_probe *probe = context;
	char buffer[32];
	int length;

	(void)registry;
	(void)codec;
	(void)diag;
	if (probe->encode_rc && !probe->allocate_then_fail)
		return probe->encode_rc;
	length = snprintf(buffer, sizeof(buffer), "%u", sample->number);
	if (length < 0 || (size_t)length > max_output)
		return VALUE_ERR_BOUNDS;
	output->data = malloc((size_t)length + 1);
	if (!output->data)
		return VALUE_ERR_NOMEM;
	memcpy(output->data, buffer, (size_t)length + 1);
	output->length = (size_t)length;
	output->release = encoded_release;
	output->release_context = probe;
	if (probe->invalid_output) {
		free(output->data);
		output->data = NULL;
	}
	if (probe->encode_rc)
		return probe->encode_rc;
	return VALUE_OK;
}

static int mock_decode(
	const struct value_codec_registry *registry,
	const struct value_codec_ops *codec, const void *input,
	size_t input_length, void **object, struct value_codec_diag *diag,
	void *context)
{
	struct serializer_probe *probe = context;
	struct codec_sample *sample;
	char buffer[32];
	char *end;
	unsigned long number;

	(void)registry;
	(void)diag;
	if (probe->decode_rc && !probe->allocate_then_fail)
		return probe->decode_rc;
	if (input_length >= sizeof(buffer))
		return VALUE_ERR_BOUNDS;
	if (codec->new_obj(object) != VALUE_OK)
		return VALUE_ERR_NOMEM;
	if (probe->decode_rc)
		return probe->decode_rc;
	if (probe->return_null) {
		codec->free_obj(*object);
		*object = NULL;
		return VALUE_OK;
	}
	sample = *object;
	memcpy(buffer, input, input_length);
	buffer[input_length] = '\0';
	number = strtoul(buffer, &end, 10);
	if (*end || number > UINT32_MAX)
		return VALUE_ERR_INVALID;
	sample->number = (uint32_t)number;
	return VALUE_OK;
}

static const struct value_serializer_ops mock_serializer = {
	.name = "mock",
	.encode = mock_encode,
	.decode = mock_decode,
};

static int registry_setup(struct value_codec_registry *registry,
	struct value_codec_entry *codecs,
	struct value_serializer_entry *serializers,
	struct serializer_probe *probe)
{
	struct value_codec_diag diag;

	T_CHECK(value_codec_registry_init(registry, codecs, 2,
		serializers, 2) == VALUE_OK);
	T_CHECK(value_codec_register(registry, &codec_sample_codec_ops,
		&diag) == VALUE_OK);
	T_CHECK(value_serializer_register(registry, &mock_serializer,
		probe, &diag) == VALUE_OK);
	return 0;
}

static int test_registry(void)
{
	struct value_codec_registry registry;
	struct value_codec_entry codecs[2];
	struct value_serializer_entry serializers[2];
	struct serializer_probe probe = { 0 };
	struct value_codec_diag diag;

	T_CHECK(registry_setup(&registry, codecs, serializers, &probe) == 0);
	T_CHECK(value_codec_lookup(&registry, "codec_sample", 1) == NULL);
	T_CHECK(value_codec_register(&registry, &codec_sample_codec_ops,
		&diag) == VALUE_ERR_EXISTS);
	T_CHECK(value_serializer_register(&registry, &mock_serializer,
		&probe, &diag) == VALUE_ERR_EXISTS);
	T_CHECK(diag.stage == VALUE_CODEC_STAGE_REGISTRY);
	T_CHECK(value_codec_registry_seal(&registry, &diag) == VALUE_OK);
	T_CHECK(value_codec_lookup(&registry, "codec_sample", 1) ==
		&codec_sample_codec_ops);
	T_CHECK(value_serializer_lookup(&registry, "mock") != NULL);
	T_CHECK(value_serializer_register(&registry, &mock_serializer,
		&probe, &diag) == VALUE_ERR_STATE);
	T_CHECK(value_codec_registry_seal(&registry, &diag) == VALUE_ERR_STATE);
	T_CHECK(value_codec_lookup(&registry, "codec_sample", 2) == NULL);
	value_codec_registry_fini(&registry);
	return 0;
}

static int test_encode_decode(void)
{
	struct value_codec_registry registry;
	struct value_codec_entry codecs[2];
	struct value_serializer_entry serializers[2];
	struct serializer_probe probe = { 0 };
	struct value_codec_diag diag;
	struct value_encoded encoded;
	struct codec_sample source = { .number = 42, .valid = 1 };
	struct codec_sample *decoded;
	void *object = NULL;

	active_probe = &probe;
	T_CHECK(registry_setup(&registry, codecs, serializers, &probe) == 0);
	T_CHECK(value_codec_registry_seal(&registry, &diag) == VALUE_OK);
	value_encoded_init(&encoded);
	T_CHECK(value_encode(&registry, "mock", "codec_sample", 1,
		&source, 16, &encoded, &diag) == VALUE_OK);
	T_CHECK(encoded.length == 2 && strcmp(encoded.data, "42") == 0);
	value_encoded_fini(&encoded);
	T_CHECK(probe.release_count == 1);

	T_CHECK(value_decode(&registry, "mock", "codec_sample", 1,
		"73", 2, &object, &diag) == VALUE_OK);
	decoded = object;
	T_CHECK(decoded->number == 73);
	sample_free(decoded);

	source.valid = 0;
	T_CHECK(value_encode(&registry, "mock", "codec_sample", 1,
		&source, 16, &encoded, &diag) == VALUE_ERR_INVALID);
	T_CHECK(diag.stage == VALUE_CODEC_STAGE_VALIDATE);
	probe.return_null = 1;
	T_CHECK(value_decode(&registry, "mock", "codec_sample", 1,
		"1", 1, &object, &diag) == VALUE_ERR_CODEC);
	T_CHECK(object == NULL && diag.stage == VALUE_CODEC_STAGE_DECODE);
	probe.return_null = 0;
	probe.allocate_then_fail = 1;
	probe.decode_rc = VALUE_ERR_CODEC;
	{
		int frees = probe.free_count;

		T_CHECK(value_decode(&registry, "mock", "codec_sample", 1,
			"1", 1, &object, &diag) == VALUE_ERR_CODEC);
		T_CHECK(object == NULL && probe.free_count == frees + 1);
	}
	probe.decode_rc = 0;
	probe.encode_rc = VALUE_ERR_CODEC;
	source.valid = 1;
	{
		int releases = probe.release_count;

		T_CHECK(value_encode(&registry, "mock", "codec_sample", 1,
			&source, 16, &encoded, &diag) == VALUE_ERR_CODEC);
		T_CHECK(probe.release_count == releases + 1);
	}
	probe.encode_rc = 0;
	probe.allocate_then_fail = 0;
	probe.invalid_output = 1;
	T_CHECK(value_encode(&registry, "mock", "codec_sample", 1,
		&source, 16, &encoded, &diag) == VALUE_ERR_CODEC);
	T_CHECK(encoded.data == NULL);

	value_codec_registry_fini(&registry);
	active_probe = NULL;
	return 0;
}

static int test_split_validation(void)
{
	struct value_codec_registry registry;
	struct value_codec_entry codecs[1];
	struct value_serializer_entry serializers[1];
	struct serializer_probe probe = { 0 };
	struct value_codec_diag diag;
	struct value_encoded encoded;
	struct codec_sample source = { .number = 41, .valid = 1 };
	struct codec_sample *decoded = NULL;

	split_validate_calls = 0;
	split_validate_decoded_calls = 0;
	T_CHECK(value_codec_registry_init(&registry, codecs, 1,
		serializers, 1) == VALUE_OK);
	T_CHECK(value_codec_register(&registry, &codec_split_sample_codec_ops,
		&diag) == VALUE_OK);
	T_CHECK(value_serializer_register(&registry, &mock_serializer,
		&probe, &diag) == VALUE_OK);
	T_CHECK(value_codec_registry_seal(&registry, &diag) == VALUE_OK);

	value_encoded_init(&encoded);
	T_CHECK(value_encode(&registry, "mock", "codec_split_sample", 1,
		&source, 16, &encoded, &diag) == VALUE_OK);
	T_CHECK(source.number == 41);
	T_CHECK(split_validate_calls == 1 &&
		split_validate_decoded_calls == 0);
	T_CHECK(value_decode(&registry, "mock", "codec_split_sample", 1,
		encoded.data, encoded.length, (void **)&decoded,
		&diag) == VALUE_OK);
	T_CHECK(decoded && decoded->number == 42);
	T_CHECK(split_validate_calls == 1 &&
		split_validate_decoded_calls == 1);

	sample_free(decoded);
	value_encoded_fini(&encoded);
	value_codec_registry_fini(&registry);
	return 0;
}

static int test_invalid_schema(void)
{
	static const struct value_codec_field bad_fields[] = {
		{
			.name = "number",
			.kind = VALUE_CODEC_U32,
			.max_value = UINT32_MAX,
			.access = NULL,
		},
	};
	static const struct value_codec_ops bad_codec = {
		.type_name = "bad",
		.schema_version = 1,
		.fields = bad_fields,
		.field_count = 1,
		.new_obj = sample_new,
		.free_obj = sample_free,
		.validate = sample_validate,
	};
	struct value_codec_registry registry;
	struct value_codec_entry codecs[1];
	struct value_serializer_entry serializers[1];
	struct serializer_probe probe = { 0 };
	struct value_codec_diag diag;

	T_CHECK(value_codec_registry_init(&registry, codecs, 1,
		serializers, 1) == VALUE_OK);
	T_CHECK(value_codec_register(&registry, &bad_codec, &diag) == VALUE_OK);
	T_CHECK(value_serializer_register(&registry, &mock_serializer,
		&probe, &diag) == VALUE_OK);
	T_CHECK(value_codec_registry_seal(&registry, &diag) == VALUE_ERR_INVALID);
	T_CHECK(diag.stage == VALUE_CODEC_STAGE_SCHEMA);
	T_CHECK(strcmp(diag.field_path, "number") == 0);
	value_codec_registry_fini(&registry);
	return 0;
}

static int test_registry_capacity(void)
{
	struct value_codec_registry registry;
	struct value_codec_entry codecs[1];
	struct value_serializer_entry serializers[1];
	struct serializer_probe probe = { 0 };
	struct value_codec_diag diag;
	struct value_codec_ops second_codec = codec_sample_codec_ops;
	struct value_serializer_ops second_serializer = mock_serializer;

	second_codec.type_name = "codec_sample_v2";
	second_serializer.name = "mock2";
	T_CHECK(value_codec_registry_init(NULL, codecs, 1,
		serializers, 1) == VALUE_ERR_ARG);
	T_CHECK(value_codec_registry_init(&registry, NULL, 1,
		serializers, 1) == VALUE_ERR_ARG);
	T_CHECK(value_codec_registry_init(&registry, codecs, 1,
		serializers, 1) == VALUE_OK);
	T_CHECK(value_codec_register(&registry, &codec_sample_codec_ops,
		&diag) == VALUE_OK);
	T_CHECK(value_codec_register(&registry, &second_codec,
		&diag) == VALUE_ERR_BOUNDS);
	T_CHECK(value_serializer_register(&registry, &mock_serializer,
		&probe, &diag) == VALUE_OK);
	T_CHECK(value_serializer_register(&registry, &second_serializer,
		&probe, &diag) == VALUE_ERR_BOUNDS);
	value_codec_registry_fini(&registry);
	return 0;
}

static int test_schema_graph(void)
{
	struct value_codec_registry registry;
	struct value_codec_entry codecs[2];
	struct value_serializer_entry serializers[1];
	struct serializer_probe probe = { 0 };
	struct value_codec_diag diag;
	struct value_codec_field nested_field = {
		.name = "child",
		.kind = VALUE_CODEC_OBJECT,
		.required = true,
		.access = &number_access,
	};
	struct value_codec_ops parent = {
		.type_name = "parent",
		.schema_version = 1,
		.fields = &nested_field,
		.field_count = 1,
		.new_obj = sample_new,
		.free_obj = sample_free,
		.validate = sample_validate,
	};

	nested_field.nested_codec = &codec_sample_codec_ops;
	T_CHECK(value_codec_registry_init(&registry, codecs, 2,
		serializers, 1) == VALUE_OK);
	T_CHECK(value_codec_register(&registry, &parent, &diag) == VALUE_OK);
	T_CHECK(value_serializer_register(&registry, &mock_serializer,
		&probe, &diag) == VALUE_OK);
	T_CHECK(value_codec_registry_seal(&registry, &diag) ==
		VALUE_ERR_NOT_FOUND);
	T_CHECK(diag.stage == VALUE_CODEC_STAGE_SCHEMA);
	value_codec_registry_fini(&registry);

	nested_field.nested_codec = &parent;
	T_CHECK(value_codec_registry_init(&registry, codecs, 2,
		serializers, 1) == VALUE_OK);
	T_CHECK(value_codec_register(&registry, &parent, &diag) == VALUE_OK);
	T_CHECK(value_serializer_register(&registry, &mock_serializer,
		&probe, &diag) == VALUE_OK);
	T_CHECK(value_codec_registry_seal(&registry, &diag) ==
		VALUE_ERR_INVALID);
	T_CHECK(diag.stage == VALUE_CODEC_STAGE_SCHEMA);
	value_codec_registry_fini(&registry);
	return 0;
}

int main(void)
{
	T_RUN(test_registry);
	T_RUN(test_encode_decode);
	T_RUN(test_split_validation);
	T_RUN(test_invalid_schema);
	T_RUN(test_registry_capacity);
	T_RUN(test_schema_graph);
	puts("t_value_codec: PASS");
	return 0;
}
