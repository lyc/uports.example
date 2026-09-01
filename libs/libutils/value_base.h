/*
 * value_base.h
 *
 * Common value framework prototype: metadata, object binding, and optional
 * intrusive-list binding.
 */

#ifndef VALUE_BASE_H
#define VALUE_BASE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

#include "list.h"

#define VALUE_OK             0
#define VALUE_ERR_ARG       -1
#define VALUE_ERR_MISSING   -2
#define VALUE_ERR_INVALID   -3
#define VALUE_ERR_NOMEM     -4
#define VALUE_ERR_EXISTS    -5
#define VALUE_ERR_NOT_FOUND -6
#define VALUE_ERR_STATE     -7
#define VALUE_ERR_BOUNDS    -8

enum value_compare_mode {
        VALUE_COMPARE_KEY = 0,
        VALUE_COMPARE_SHALLOW,
        VALUE_COMPARE_DEEP,
};

enum value_list_add_flags {
        VALUE_LIST_ADD_DEFAULT = 0,
        VALUE_LIST_ADD_UNIQUE  = 0x0001,
        VALUE_LIST_ADD_FIRST   = 0x0002,
        VALUE_LIST_ADD_SORTED  = 0x0004,
        VALUE_LIST_ADD_LAST    = 0x0008,
};

#if defined(__GNUC__)
#define VALUE_UNUSED __attribute__((unused))
#else
#define VALUE_UNUSED
#endif

typedef int (*list_cmp_cb)(struct list_head *a, struct list_head *b,
                           void *data);

size_t list_count(struct list_head *head);
void list_sort_insertion(struct list_head *head, list_cmp_cb func, void *data);

/* ------------------------------------------------------------------------- */
/*                                                                           */
/* ------------------------------------------------------------------------- */

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

void value_config_init(struct value_config *m, uint64_t required);
bool value_config_has(const struct value_config *m, uint64_t field);
bool value_config_has_any(const struct value_config *m, uint64_t fields);
bool value_config_has_all(const struct value_config *m, uint64_t fields);
bool value_config_can_use(const struct value_config *m, uint64_t field);
bool value_config_ready(const struct value_config *m);
void value_config_mark_present(struct value_config *m, uint64_t field);
void value_config_mark_dirty(struct value_config *m, uint64_t field);
void value_config_mark_present_dirty(struct value_config *m, uint64_t fields);
void value_config_mark_invalid(struct value_config *m,
			       uint64_t field, const char *error);
void value_config_mark_invalid_fields(struct value_config *m, uint64_t fields);
void value_config_mark_valid(struct value_config *m, uint64_t field);
void value_config_clear_invalid(struct value_config *m, uint64_t fields);
uint64_t value_config_missing_required(const struct value_config *m);
bool value_config_complete(const struct value_config *m);
uint64_t value_config_dirty_fields(const struct value_config *m);
bool value_config_dirty_any(const struct value_config *m, uint64_t fields);
bool value_config_invalid_any(const struct value_config *m, uint64_t fields);
bool value_config_is_dirty(const struct value_config *m);
void value_config_accept(struct value_config *m);
void value_config_clear_field(struct value_config *m, uint64_t field);

typedef const char *(*value_field_name_fn)(uint64_t field);

struct value_meta_ops {
        const char *type_name;
        size_t meta_offset;
        value_field_name_fn field_name;
};

#define VALUE_FIELD_SET_CHANGED(type, value, member, field, new_value)  \
	do {								\
		if (!type ## _has_field(value, field)) {		\
			(value)->member = (new_value);			\
			type ## _meta_mark_present_dirty(value, field);	\
		} else if ((value)->member != (new_value)) {		\
			(value)->member = (new_value);			\
			type ## _meta_mark_dirty(value, field);		\
		}							\
	} while (0)

#define VALUE_FIELD_SET_CHANGED2(type, value, member1, member2, field,	\
				 new_value1, new_value2)		\
	do {								\
		if (!type ## _has_field(value, field)) {		\
			(value)->member1 = (new_value1);		\
			(value)->member2 = (new_value2);		\
			type ## _meta_mark_present_dirty(value, field);	\
		} else if (((value)->member1 != (new_value1)) ||	\
			   ((value)->member2 != (new_value2))) {	\
			(value)->member1 = (new_value1);		\
			(value)->member2 = (new_value2);		\
			type ## _meta_mark_dirty(value, field);		\
		}							\
	} while (0)

#define VALUE_FIELD_COMPARE(type, lhs, rhs, member, field)		\
	do {								\
		if (type ## _has_field(lhs, field) &&			\
		    type ## _has_field(rhs, field)) {			\
			if ((lhs)->member > (rhs)->member)		\
				return 1;				\
			if ((lhs)->member < (rhs)->member)		\
				return -1;				\
		}							\
	} while (0)

#define VALUE_FIELD_COMPARE2(type, lhs, rhs, member1, member2, field)	\
	do {								\
		if (type ## _has_field(lhs, field) &&			\
		    type ## _has_field(rhs, field)) {			\
			if ((lhs)->member1 > (rhs)->member1)		\
				return 1;				\
			if ((lhs)->member1 < (rhs)->member1)		\
				return -1;				\
			if ((lhs)->member2 > (rhs)->member2)		\
				return 1;				\
			if ((lhs)->member2 < (rhs)->member2)		\
				return -1;				\
		}							\
	} while (0)

#define VALUE_META_HEADER(type)						\
	extern const struct value_meta_ops type ## _meta_ops;		\
	struct value_config *type ## _meta(struct type *value);		\
	const struct value_config *type ## _meta_const(const struct type *value); \
	bool type ## _has_field(const struct type *value, uint64_t field); \
	bool type ## _meta_has(const struct type *value, uint64_t field);	\
	bool type ## _can_use_field(const struct type *value, uint64_t field); \
	bool type ## _meta_ready(const struct type *value);		\
	bool type ## _meta_has_any(const struct type *value, uint64_t fields); \
	bool type ## _meta_has_all(const struct type *value, uint64_t fields); \
	bool type ## _meta_dirty_any(const struct type *value, uint64_t fields); \
	bool type ## _meta_invalid_any(const struct type *value, uint64_t fields); \
	uint64_t type ## _missing_required(const struct type *value);	\
	bool type ## _meta_is_complete(const struct type *value);	\
	bool type ## _meta_complete(const struct type *value);		\
	uint64_t type ## _dirty_fields(const struct type *value);	\
	bool type ## _is_dirty(const struct type *value);		\
	void type ## _mark_present(struct type *value, uint64_t field);	\
	void type ## _meta_mark_present(struct type *value, uint64_t field); \
	void type ## _mark_dirty(struct type *value, uint64_t field);	\
	void type ## _meta_mark_dirty(struct type *value, uint64_t field); \
	void type ## _meta_mark_present_dirty(struct type *value, uint64_t fields); \
	void type ## _meta_mark_invalid(struct type *value, uint64_t fields); \
	void type ## _meta_clear_invalid(struct type *value, uint64_t fields); \
	void type ## _meta_accept(struct type *value);			\
	void type ## _clear_field(struct type *value, uint64_t field);	\
	const char *type ## _field_name(uint64_t field)

#define VALUE_META_BIND(type, meta_member, field_name_cb)		\
	const struct value_meta_ops type ## _meta_ops = {		\
		.type_name = #type,					\
		.meta_offset = offsetof(struct type, meta_member),	\
		.field_name = (value_field_name_fn)(field_name_cb),	\
	};								\
									\
	struct value_config *type ## _meta(struct type *value)		\
	{								\
		if (!value)						\
			return NULL;					\
		return (struct value_config *)((char *)value + type ## _meta_ops.meta_offset); \
	}								\
									\
	const struct value_config *type ## _meta_const(const struct type *value) \
	{								\
		if (!value)						\
			return NULL;					\
		return (const struct value_config *)( (const char *)value + type ## _meta_ops.meta_offset); \
	}								\
									\
	bool type ## _has_field(const struct type *value, uint64_t field) \
	{								\
		return value && value_config_has(type ## _meta_const(value), field); \
	}								\
									\
	bool type ## _meta_has(const struct type *value, uint64_t field) \
	{								\
		return type ## _has_field(value, field);		\
	}								\
									\
	bool type ## _can_use_field(const struct type *value, uint64_t field) \
	{								\
		return value && value_config_can_use(type ## _meta_const(value), field);\
	}								\
									\
	bool type ## _meta_ready(const struct type *value)		\
	{								\
		return value && value_config_ready(type ## _meta_const(value));	\
	}								\
									\
	bool type ## _meta_has_any(const struct type *value, uint64_t fields) \
	{								\
		return value && value_config_has_any(type ## _meta_const(value), fields); \
	}								\
									\
	bool type ## _meta_has_all(const struct type *value, uint64_t fields) \
	{								\
		return value && value_config_has_all(type ## _meta_const(value), fields); \
	}								\
									\
	bool type ## _meta_dirty_any(const struct type *value, uint64_t fields)	\
	{								\
		return value &&						\
			value_config_dirty_any(type ## _meta_const(value), fields); \
	}								\
									\
	bool type ## _meta_invalid_any(const struct type *value, uint64_t fields) \
	{								\
		return value &&						\
			value_config_invalid_any(type ## _meta_const(value), fields); \
	}								\
									\
	uint64_t type ## _missing_required(const struct type *value)	\
	{								\
		if (!value)						\
			return UINT64_MAX;				\
		return value_config_missing_required(type ## _meta_const(value)); \
	}								\
									\
	bool type ## _meta_is_complete(const struct type *value)	\
	{								\
		return value && value_config_complete(type ## _meta_const(value)); \
	}								\
									\
	bool type ## _meta_complete(const struct type *value)		\
	{								\
		return type ## _meta_is_complete(value);		\
	}								\
									\
	uint64_t type ## _dirty_fields(const struct type *value)	\
	{								\
		if (!value)						\
			return 0;					\
		return value_config_dirty_fields(type ## _meta_const(value)); \
	}								\
									\
	bool type ## _is_dirty(const struct type *value)		\
	{								\
		return value && value_config_is_dirty(type ## _meta_const(value)); \
	}								\
									\
	void type ## _mark_present(struct type *value, uint64_t field)	\
	{								\
		if (value)						\
			value_config_mark_present(type ## _meta(value), field);	\
	}								\
									\
	void type ## _meta_mark_present(struct type *value, uint64_t field) \
	{								\
		type ## _mark_present(value, field);			\
	}								\
									\
	void type ## _mark_dirty(struct type *value, uint64_t field)	\
	{								\
		if (value)						\
			value_config_mark_dirty(type ## _meta(value), field); \
	}								\
									\
	void type ## _meta_mark_dirty(struct type *value, uint64_t field) \
	{								\
		type ## _mark_dirty(value, field);			\
	}								\
									\
	void type ## _meta_mark_present_dirty(struct type *value, uint64_t fields) \
	{								\
		if (value)						\
			value_config_mark_present_dirty(type ## _meta(value), fields); \
	}								\
									\
	void type ## _meta_mark_invalid(struct type *value, uint64_t fields) \
	{								\
		if (value)						\
			value_config_mark_invalid_fields(type ## _meta(value), fields);	\
	}								\
									\
	void type ## _meta_clear_invalid(struct type *value, uint64_t fields) \
	{								\
		if (value)						\
			value_config_clear_invalid(type ## _meta(value), fields); \
	}								\
									\
	void type ## _meta_accept(struct type *value)			\
	{								\
		if (value)						\
			value_config_accept(type ## _meta(value));	\
	}								\
									\
	void type ## _clear_field(struct type *value, uint64_t field)	\
	{								\
		if (value)						\
			value_config_clear_field(type ## _meta(value), field); \
	}								\
									\
	const char *type ## _field_name(uint64_t field)			\
	{								\
		if (!type ## _meta_ops.field_name)			\
			return "unknown";				\
		return type ## _meta_ops.field_name(field);		\
	}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/* ------------------------------------------------------------------------- */

typedef void *(*value_calloc_fn)(size_t nmemb, size_t size, void *ctx);
typedef void (*value_dealloc_fn)(void *ptr, void *ctx);

struct value_memory_ops {
        value_calloc_fn calloc_fn;
        value_dealloc_fn dealloc_fn;
        void *ctx;
};

int value_memory_set(const struct value_memory_ops *ops);
void value_memory_reset(void);

typedef const char *(*value_key_fn)(const void *obj);
typedef int (*value_copy_fn)(void *dst, const void *src);
typedef void (*value_free_fn)(void *obj);
typedef bool (*value_pred_fn)(const void *obj, void *ctx);
typedef void (*value_each_fn)(void *obj, void *ctx);
typedef void *(*value_reduce_fn)(void *acc, void *obj, void *ctx);
typedef struct list_head *(*value_hook_map_fn)(void *obj, void *ctx);
typedef int (*value_sort_fn)(void *a, void *b, void *ctx);
typedef void (*value_init_fn)(void *obj, const char *name);
typedef void (*value_fini_fn)(void *obj);
typedef int (*value_validate_fn)(void *obj);
typedef int (*value_cmp_fn)(const void *a, const void *b);
typedef bool (*value_equal_fn)(const void *a, const void *b);

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

#define VALUE_OBJECT_HEADER(type)					\
	extern const struct value_object_ops type ## _object_ops

#define VALUE_OBJECT_WRAPPERS_HEADER(type)				\
	void type ## _fini(struct type *value);				\
	int type ## _copy(struct type *dst, const struct type *src);	\
	void type ## _free(struct type *value)

#define VALUE_OBJECT_BIND(						\
        type, init_cb, fini_cb, copy_cb, validate_cb, cmp_cb, equal_cb)	\
	const struct value_object_ops type ## _object_ops = {		\
		.type_name = #type,					\
		.size = sizeof(struct type),				\
		.init = (value_init_fn)(init_cb),			\
		.fini = (value_fini_fn)(fini_cb),			\
		.copy = (value_copy_fn)(copy_cb),			\
		.validate = (value_validate_fn)(validate_cb),		\
		.cmp = (value_cmp_fn)(cmp_cb),				\
		.equal = (value_equal_fn)(equal_cb),			\
	};

#define VALUE_OBJECT_WRAPPERS_BIND(type, fini_cb, copy_cb, free_cb)	\
	void type ## _fini(struct type *value)				\
	{								\
		if (value)						\
			fini_cb(value);					\
	}								\
									\
	int type ## _copy(struct type *dst, const struct type *src)	\
	{								\
		if (!dst || !src)					\
			return VALUE_ERR_ARG;				\
		return copy_cb(dst, src);				\
	}								\
									\
	void type ## _free(struct type *value)				\
	{								\
		if (!value)						\
			return;						\
		fini_cb(value);						\
		free_cb(value);						\
	}

bool value_equal(
        const struct value_object_ops *ops, const void *a, const void *b);

/* ------------------------------------------------------------------------- */
/*                                                                           */
/* ------------------------------------------------------------------------- */

struct value_list_ops {
        size_t hook_offset;
        value_key_fn key;
        value_init_fn init;
        value_copy_fn copy;
        value_free_fn free_obj;
        size_t size;
};

#define VALUE_LIST_LOOKUP_HEADER(type)					\
	struct type *type ## _lookup_unlocked(				\
		struct list_head *head, const char *key);		\
	struct type *type ## _lookup_move_front_unlocked(		\
		struct list_head *head, const char *key);		\
	void type ## _sort_key_unlocked(struct list_head *head)

#define VALUE_LIST_HEADER(type)						\
	extern const struct value_list_ops type ## _list_ops;		\
	size_t type ## _count(struct list_head *head);			\
	struct type *type ## _nth_unlocked(				\
		struct list_head *head, size_t index);			\
	void type ## _add_tail_unlocked(				\
		struct list_head *head, struct type *value);		\
	void type ## _foreach_unlocked(					\
		struct list_head *head,					\
		void (*fn)(struct type *value, void *ctx), void *ctx);	\
	void *type ## _reduce_unlocked(					\
		struct list_head *head,					\
		void *(*fn)(void *acc, struct type *value, void *ctx),	\
		void *ctx, void *acc);					\
	struct list_head *type ## _map_unlocked(			\
		struct list_head *out,					\
		struct list_head *(*fn)(struct type *value, void *ctx),	\
		void *ctx, struct list_head *head);			\
	void type ## _sort_unlocked(					\
		struct list_head *head,					\
		int (*fn)(struct type *a, struct type *b, void *ctx),	\
		void *ctx);						\
	size_t type ## _remove_if_unlocked(				\
		struct list_head *head,					\
		bool (*pred)(const struct type *value, void *ctx), void *ctx); \
	int type ## _filter_view_unlocked(				\
		struct list_head *out, struct list_head *in,		\
		bool (*pred)(const struct type *value, void *ctx), void *ctx); \
	void type ## _free_list_unlocked(struct list_head *head)

struct list_head *value_hook(void *obj, const struct value_list_ops *ops);
const struct list_head *value_hook_const(
        const void *obj, const struct value_list_ops *ops);
void *value_entry(struct list_head *hook, const struct value_list_ops *ops);
void *value_list_nth_unlocked(
        struct list_head *head, const struct value_list_ops *ops,
        size_t index);
void *value_list_lookup_unlocked(
        struct list_head *head, const struct value_list_ops *ops,
        const char *key);
void *value_list_lookup_move_front_unlocked(
        struct list_head *head, const struct value_list_ops *ops,
        const char *key);
void value_list_foreach_unlocked(
        struct list_head *head, const struct value_list_ops *ops,
        value_each_fn fn, void *ctx);
void *value_list_reduce_unlocked(
        struct list_head *head, const struct value_list_ops *ops,
        value_reduce_fn fn, void *ctx, void *acc);
struct list_head *value_list_map_hook_unlocked(
        struct list_head *out, struct list_head *in,
        const struct value_list_ops *ops, value_hook_map_fn fn, void *ctx);
void value_list_sort_unlocked(
        struct list_head *head, const struct value_list_ops *ops,
        value_sort_fn fn, void *ctx);
void value_list_sort_key_unlocked(
        struct list_head *head, const struct value_list_ops *ops);
size_t value_list_remove_if_unlocked(
        struct list_head *head, const struct value_list_ops *ops,
        value_pred_fn pred, void *ctx);
void value_list_free_unlocked(
        struct list_head *head, const struct value_list_ops *ops);
size_t value_list_count(struct list_head *head);

#define VALUE_LIST_BIND(type, key_member, hook_member, copy_cb, free_cb) \
	static const char *type ## _key(const void *obj)		\
	{								\
		const struct type *value = obj;				\
									\
		return value->key_member ? value->key_member : "";	\
	}								\
									\
	const struct value_list_ops type ## _list_ops = {		\
		.hook_offset = offsetof(struct type, hook_member),	\
		.key = type ## _key,					\
		.init = type ## _object_ops.init,			\
		.copy = (value_copy_fn)(copy_cb),			\
		.free_obj = (value_free_fn)(free_cb),			\
		.size = sizeof(struct type),				\
	};								\
									\
	struct type *type ## _lookup_unlocked(				\
		struct list_head *head, const char *key)		\
	{								\
		return value_list_lookup_unlocked(head, &type ## _list_ops, key); \
	}								\
									\
	struct type *type ## _lookup_move_front_unlocked(		\
		struct list_head *head, const char *key)		\
	{								\
		return value_list_lookup_move_front_unlocked(		\
			head, &type ## _list_ops, key);			\
	}								\
									\
	void type ## _sort_key_unlocked(struct list_head *head)		\
	{								\
		value_list_sort_key_unlocked(head, &type ## _list_ops);	\
	}								\
									\
	size_t type ## _count(struct list_head *head)			\
	{								\
		return value_list_count(head);				\
	}								\
									\
	struct type *type ## _nth_unlocked(				\
		struct list_head *head, size_t index)			\
	{								\
		return value_list_nth_unlocked(				\
			head, &type ## _list_ops, index);		\
	}								\
									\
	void type ## _add_tail_unlocked(struct list_head *head, struct type *value) \
	{								\
		if (head && value)					\
			list_add_tail(&value->hook_member, head);	\
	}								\
									\
	void type ## _foreach_unlocked(					\
		struct list_head *head,					\
		void (*fn)(struct type *value, void *ctx), void *ctx)	\
	{								\
		value_list_foreach_unlocked(				\
			head, &type ## _list_ops, (value_each_fn)fn, ctx); \
	}								\
									\
	void *type ## _reduce_unlocked(					\
		struct list_head *head,					\
		void *(*fn)(void *acc, struct type *value, void *ctx),	\
		void *ctx, void *acc)					\
	{								\
		return value_list_reduce_unlocked(			\
			head, &type ## _list_ops, (value_reduce_fn)fn, ctx, acc); \
	}								\
									\
	struct list_head *type ## _map_unlocked(			\
		struct list_head *out,					\
		struct list_head *(*fn)(struct type *value, void *ctx), void *ctx, \
		struct list_head *head)					\
	{								\
		return value_list_map_hook_unlocked(			\
			out, head, &type ## _list_ops, (value_hook_map_fn)fn, ctx); \
	}								\
									\
	void type ## _sort_unlocked(					\
		struct list_head *head,					\
		int (*fn)(struct type *a, struct type *b, void *ctx), void *ctx) \
	{								\
		value_list_sort_unlocked(				\
			head, &type ## _list_ops, (value_sort_fn)fn, ctx); \
	}								\
									\
	size_t type ## _remove_if_unlocked(				\
		struct list_head *head,					\
		bool (*pred)(const struct type *value, void *ctx), void *ctx) \
	{								\
		return value_list_remove_if_unlocked(			\
			head, &type ## _list_ops, (value_pred_fn)pred, ctx); \
	}								\
									\
	int type ## _filter_view_unlocked(				\
		struct list_head *out, struct list_head *in,		\
		bool (*pred)(const struct type *value, void *ctx), void *ctx) \
	{								\
		return value_list_filter_view_unlocked(			\
			out, in, &type ## _list_ops, (value_pred_fn)pred, ctx);	\
	}								\
									\
	void type ## _free_list_unlocked(struct list_head *head)	\
	{								\
		value_list_free_unlocked(head, &type ## _list_ops);	\
	}

#define VALUE_LIST_BIND_NOKEY(type, hook_member, copy_cb, free_cb)	\
	const struct value_list_ops type ## _list_ops = {		\
		.hook_offset = offsetof(struct type, hook_member),	\
		.key = NULL,						\
		.init = type ## _object_ops.init,			\
		.copy = (value_copy_fn)(copy_cb),			\
		.free_obj = (value_free_fn)(free_cb),			\
		.size = sizeof(struct type),				\
	};								\
									\
	size_t type ## _count(struct list_head *head)			\
	{								\
		return value_list_count(head);				\
	}								\
									\
	struct type *type ## _nth_unlocked(				\
		struct list_head *head, size_t index)			\
	{								\
		return value_list_nth_unlocked(				\
			head, &type ## _list_ops, index);		\
	}								\
									\
	void type ## _add_tail_unlocked(struct list_head *head, struct type *value) \
	{								\
		if (head && value)					\
			list_add_tail(&value->hook_member, head);	\
	}								\
									\
	void type ## _foreach_unlocked(					\
		struct list_head *head,					\
		void (*fn)(struct type *value, void *ctx), void *ctx)	\
	{								\
		value_list_foreach_unlocked(				\
			head, &type ## _list_ops, (value_each_fn)fn, ctx); \
	}								\
									\
	void *type ## _reduce_unlocked(					\
		struct list_head *head,					\
		void *(*fn)(void *acc, struct type *value, void *ctx),	\
		void *ctx, void *acc)					\
	{								\
		return value_list_reduce_unlocked(			\
			head, &type ## _list_ops, (value_reduce_fn)fn, ctx, acc); \
	}								\
									\
	struct list_head *type ## _map_unlocked(			\
		struct list_head *out,					\
		struct list_head *(*fn)(struct type *value, void *ctx), void *ctx, \
		struct list_head *head)					\
	{								\
		return value_list_map_hook_unlocked(			\
			out, head, &type ## _list_ops, (value_hook_map_fn)fn, ctx); \
	}								\
									\
	void type ## _sort_unlocked(					\
		struct list_head *head,					\
		int (*fn)(struct type *a, struct type *b, void *ctx), void *ctx) \
	{								\
		value_list_sort_unlocked(				\
			head, &type ## _list_ops, (value_sort_fn)fn, ctx); \
	}								\
									\
	size_t type ## _remove_if_unlocked(				\
		struct list_head *head,					\
		bool (*pred)(const struct type *value, void *ctx), void *ctx) \
	{								\
		return value_list_remove_if_unlocked(			\
			head, &type ## _list_ops, (value_pred_fn)pred, ctx); \
	}								\
									\
	int type ## _filter_view_unlocked(				\
		struct list_head *out, struct list_head *in,		\
		bool (*pred)(const struct type *value, void *ctx), void *ctx) \
	{								\
		return value_list_filter_view_unlocked(			\
			out, in, &type ## _list_ops, (value_pred_fn)pred, ctx);	\
	}								\
									\
	void type ## _free_list_unlocked(struct list_head *head)	\
	{								\
		value_list_free_unlocked(head, &type ## _list_ops);	\
	}

#define VALUE_LIST_BIND_DOMAIN_KEY(type, hook_member, copy_cb, free_cb)	\
	VALUE_LIST_BIND_NOKEY(type, hook_member, copy_cb, free_cb)

/* ------------------------------------------------------------------------- */
/*                                                                           */
/* ------------------------------------------------------------------------- */

struct value_filter_ref {
        void *ptr;
        value_dealloc_fn dealloc_fn;
        void *dealloc_ctx;
        struct list_head list;
};

int value_list_filter_view_unlocked(
        struct list_head *out, struct list_head *in,
        const struct value_list_ops *ops, value_pred_fn pred, void *ctx);

void value_filter_view_free(struct list_head *filters);
void *value_filter_view_first(struct list_head *filters);
void value_filter_view_foreach(
        void (*fn)(void *ptr, void *ctx), void *ctx,
        struct list_head *filters);
void *value_filter_view_nth(struct list_head *filters, size_t index);
int value_filter_view_clone(
        struct list_head *destination, const struct list_head *source);
void *value_filter_view_reduce(
        struct list_head *filters, value_reduce_fn fn, void *ctx, void *acc);
void value_filter_view_sort(
        struct list_head *filters, value_sort_fn fn, void *ctx);
struct list_head *value_filter_view_map(
	struct list_head *out, struct list_head *filters,
	value_hook_map_fn fn, void *ctx);

#endif /* VALUE_BASE_H */
