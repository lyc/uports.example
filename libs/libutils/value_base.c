/*
 * value_base.c
 *
 * Common value framework prototype implementation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "value_base.h"

size_t list_count(struct list_head *head)
{
        size_t count = 0;
        struct list_head *pos;

        list_for_each(pos, head)
                count++;
        return count;
}

void list_sort_insertion(struct list_head *head, list_cmp_cb func, void *data)
{
        struct list_head *pos, *next;
        LIST_HEAD(sorted);

        if (!func || list_empty(head))
                return;

        list_for_each_safe(pos, next, head) {
                struct list_head *item;

                list_del(pos);
                list_for_each(item, &sorted) {
                        if (func(pos, item, data) < 0)
                                break;
                }
                list_add_tail(pos, item);
        }
        list_splice_init(&sorted, head);
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/* ------------------------------------------------------------------------- */

void value_config_init(struct value_config *m, uint64_t required)
{
        if (!m)
                return;
        memset(m, 0, sizeof(*m));
        m->required = required;
}

bool value_config_has(const struct value_config *m, uint64_t field)
{
        return m && ((m->present & field) == field);
}

bool value_config_has_any(const struct value_config *m, uint64_t fields)
{
        return m && ((m->present & fields) != 0);
}

bool value_config_has_all(const struct value_config *m, uint64_t fields)
{
        return m && ((m->present & fields) == fields);
}

bool value_config_can_use(const struct value_config *m, uint64_t field)
{
        return value_config_has(m, field) && ((m->invalid & field) == 0);
}

bool value_config_ready(const struct value_config *m)
{
        return value_config_complete(m) && ((m->invalid & m->required) == 0);
}

void value_config_mark_present(struct value_config *m, uint64_t field)
{
        if (!m)
                return;
        m->present |= field;
        m->last_update = time(NULL);
}

void value_config_mark_dirty(struct value_config *m, uint64_t field)
{
        if (!m)
                return;
        m->present |= field;
        m->dirty |= field;
        m->last_update = time(NULL);
}

void value_config_mark_present_dirty(struct value_config *m, uint64_t fields)
{
        if (!m)
                return;
        m->present |= fields;
        m->dirty |= fields;
        m->last_update = time(NULL);
}

void value_config_mark_invalid(struct value_config *m, uint64_t field, const char *error)
{
        if (!m)
                return;
        m->validated |= field;
        m->invalid |= field;
        m->validation_count++;
        m->last_validation = time(NULL);
        if (error)
                snprintf(m->validation_error,
                         sizeof(m->validation_error), "%s", error);
}

void value_config_mark_invalid_fields(struct value_config *m, uint64_t fields)
{
        if (!m)
                return;
        m->validated |= fields;
        m->invalid |= fields;
        m->validation_count++;
        m->last_validation = time(NULL);
}

void value_config_mark_valid(struct value_config *m, uint64_t field)
{
        if (!m)
                return;
        m->validated |= field;
        m->invalid &= ~field;
        m->validation_count++;
        m->last_validation = time(NULL);
}

void value_config_clear_invalid(struct value_config *m, uint64_t fields)
{
        if (!m)
                return;
        m->invalid &= ~fields;
        m->last_validation = time(NULL);
}

uint64_t value_config_missing_required(const struct value_config *m)
{
        if (!m)
                return UINT64_MAX;
        return m->required & ~m->present;
}

bool value_config_complete(const struct value_config *m)
{
        return m && (value_config_missing_required(m) == 0);
}

uint64_t value_config_dirty_fields(const struct value_config *m)
{
        return m ? m->dirty : 0;
}

bool value_config_dirty_any(const struct value_config *m, uint64_t fields)
{
        return m && ((m->dirty & fields) != 0);
}

bool value_config_invalid_any(const struct value_config *m, uint64_t fields)
{
        return m && ((m->invalid & fields) != 0);
}

bool value_config_is_dirty(const struct value_config *m)
{
        return m && m->dirty != 0;
}

void value_config_accept(struct value_config *m)
{
        if (!m)
                return;
        m->dirty = 0;
}

void value_config_clear_field(struct value_config *m, uint64_t field)
{
        if (!m)
                return;
        m->present &= ~field;
        m->dirty &= ~field;
        m->validated &= ~field;
        m->invalid &= ~field;
        m->last_update = time(NULL);
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/* ------------------------------------------------------------------------- */

static void *value_default_calloc(size_t nmemb, size_t size, void *ctx)
{
        (void)ctx;
        return calloc(nmemb, size);
}

static void value_default_dealloc(void *ptr, void *ctx)
{
        (void)ctx;
        free(ptr);
}

static struct value_memory_ops value_memory = {
        .calloc_fn = value_default_calloc,
        .dealloc_fn = value_default_dealloc,
        .ctx = NULL,
};

int value_memory_set(const struct value_memory_ops *ops)
{
        if (!ops || !ops->calloc_fn || !ops->dealloc_fn)
                return VALUE_ERR_ARG;
        value_memory = *ops;
        return VALUE_OK;
}

void value_memory_reset(void)
{
        value_memory.calloc_fn = value_default_calloc;
        value_memory.dealloc_fn = value_default_dealloc;
        value_memory.ctx = NULL;
}

bool value_equal(
        const struct value_object_ops *ops, const void *a, const void *b)
{
        if (!ops || !a || !b)
                return false;
        if (ops->equal)
                return ops->equal(a, b);
        if (ops->cmp)
                return ops->cmp(a, b) == 0;
        return a == b;
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/* ------------------------------------------------------------------------- */

struct list_head *value_hook(void *obj, const struct value_list_ops *ops)
{
        if (!obj || !ops)
                return NULL;
        return (struct list_head *)((char *)obj + ops->hook_offset);
}

const struct list_head *value_hook_const(
        const void *obj, const struct value_list_ops *ops)
{
        if (!obj || !ops)
                return NULL;
        return (const struct list_head *)((const char *)obj + ops->hook_offset);
}

void *value_entry(struct list_head *hook, const struct value_list_ops *ops)
{
        if (!hook || !ops)
                return NULL;
        return (void *)((char *)hook - ops->hook_offset);
}

void *value_list_nth_unlocked(
        struct list_head *head, const struct value_list_ops *ops,
        size_t index)
{
        struct list_head *p;

        if (!head || !ops)
                return NULL;

        list_for_each(p, head) {
                if (!index)
                        return value_entry(p, ops);
                index--;
        }
        return NULL;
}

void *value_list_lookup_unlocked(
        struct list_head *head, const struct value_list_ops *ops,
        const char *key)
{
        struct list_head *p;

        if (!head || !ops || !ops->key || !key)
                return NULL;

        list_for_each(p, head) {
                void *obj = value_entry(p, ops);
                const char *obj_key = ops->key(obj);

                if (obj_key && !strcmp(obj_key, key))
                        return obj;
        }
        return NULL;
}

void *value_list_lookup_move_front_unlocked(
        struct list_head *head, const struct value_list_ops *ops,
        const char *key)
{
        void *obj;
        struct list_head *hook;

        obj = value_list_lookup_unlocked(head, ops, key);
        if (!obj)
                return NULL;

        hook = value_hook(obj, ops);
        if (head->next != hook) {
                list_del(hook);
                list_add(hook, head);
        }
        return obj;
}

void value_list_foreach_unlocked(
        struct list_head *head, const struct value_list_ops *ops,
        value_each_fn fn, void *ctx)
{
        struct list_head *p;

        if (!head || !ops || !fn)
                return;

        list_for_each(p, head)
                fn(value_entry(p, ops), ctx);
}

void *value_list_reduce_unlocked(
        struct list_head *head, const struct value_list_ops *ops,
        value_reduce_fn fn, void *ctx, void *acc)
{
        struct list_head *p;

        if (!head || !ops || !fn)
                return acc;

        list_for_each(p, head)
                acc = fn(acc, value_entry(p, ops), ctx);
        return acc;
}

struct list_head *value_list_map_hook_unlocked(
        struct list_head *out, struct list_head *in,
        const struct value_list_ops *ops, value_hook_map_fn fn, void *ctx)
{
        struct list_head *p;

        if (!out || !in || !ops || !fn)
                return out;

        list_for_each(p, in) {
                struct list_head *hook = fn(value_entry(p, ops), ctx);

                if (hook)
                        list_add_tail(hook, out);
        }
        return out;
}

static int value_list_sort_cmp(struct list_head *a, struct list_head *b,
                               void *data)
{
        struct {
                const struct value_list_ops *ops;
                value_sort_fn fn;
                void *ctx;
        } *sort = data;

        return sort->fn(value_entry(a, sort->ops),
                        value_entry(b, sort->ops), sort->ctx);
}

void value_list_sort_unlocked(
        struct list_head *head, const struct value_list_ops *ops,
        value_sort_fn fn, void *ctx)
{
        struct {
                const struct value_list_ops *ops;
                value_sort_fn fn;
                void *ctx;
        } sort = {
                .ops = ops,
                .fn = fn,
                .ctx = ctx,
        };

        if (!head || !ops || !fn)
                return;

        list_sort_insertion(head, value_list_sort_cmp, &sort);
}

static int value_list_key_cmp(void *a, void *b, void *ctx)
{
        const struct value_list_ops *ops = ctx;
        const char *a_key = ops->key(a);
        const char *b_key = ops->key(b);

        if (!a_key)
                return b_key ? -1 : 0;
        if (!b_key)
                return 1;
        return strcmp(a_key, b_key);
}

void value_list_sort_key_unlocked(
        struct list_head *head, const struct value_list_ops *ops)
{
        if (!ops || !ops->key)
                return;

        value_list_sort_unlocked(head, ops, value_list_key_cmp, (void *)ops);
}

size_t value_list_remove_if_unlocked(
        struct list_head *head, const struct value_list_ops *ops,
        value_pred_fn pred, void *ctx)
{
        struct list_head *p, *n;
        size_t removed = 0;

        if (!head || !ops || !pred)
                return 0;

        list_for_each_safe(p, n, head) {
                void *obj = value_entry(p, ops);

                if (!pred(obj, ctx))
                        continue;
                list_del_init(p);
                if (ops->free_obj)
                        ops->free_obj(obj);
                removed++;
        }
        return removed;
}

void value_list_free_unlocked(
        struct list_head *head, const struct value_list_ops *ops)
{
        struct list_head *p, *n;

        if (!head || !ops)
                return;

        list_for_each_safe(p, n, head) {
                void *obj = value_entry(p, ops);

                list_del_init(p);
                if (ops->free_obj)
                        ops->free_obj(obj);
        }
}

size_t value_list_count(struct list_head *head)
{
        struct list_head *p;
        size_t count = 0;

        if (!head)
                return 0;

        list_for_each(p, head)
                count++;
        return count;
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/* ------------------------------------------------------------------------- */

int value_list_filter_view_unlocked(
        struct list_head *out, struct list_head *in,
        const struct value_list_ops *ops, value_pred_fn pred, void *ctx)
{
        struct list_head *p;

        if (!out || !in || !ops || !pred)
                return VALUE_ERR_ARG;

        INIT_LIST_HEAD(out);
        list_for_each(p, in) {
                void *obj = value_entry(p, ops);

                if (pred(obj, ctx)) {
                        struct value_filter_ref *filter;

                        filter = value_memory.calloc_fn(
                                1, sizeof(*filter), value_memory.ctx);
                        if (!filter) {
                                value_filter_view_free(out);
                                return VALUE_ERR_NOMEM;
                        }
                        filter->ptr = obj;
                        filter->dealloc_fn = value_memory.dealloc_fn;
                        filter->dealloc_ctx = value_memory.ctx;
                        list_add_tail(&filter->list, out);
                }
        }
        return VALUE_OK;
}


void value_filter_view_free(struct list_head *filters)
{
        struct list_head *p, *n;

        if (!filters)
                return;

        list_for_each_safe(p, n, filters) {
                struct value_filter_ref *filter;

                filter = list_entry(p, struct value_filter_ref, list);
                list_del(p);
                filter->dealloc_fn(filter, filter->dealloc_ctx);
        }
}

void *value_filter_view_first(struct list_head *filters)
{
        struct value_filter_ref *filter;

        if (!filters || list_empty(filters))
                return NULL;

        filter = list_entry(filters->next, struct value_filter_ref, list);
        return filter->ptr;
}

void value_filter_view_foreach(
        void (*fn)(void *ptr, void *ctx), void *ctx,
        struct list_head *filters)
{
        struct list_head *p;

        if (!fn || !filters)
                return;

        list_for_each(p, filters) {
                struct value_filter_ref *filter = list_entry(p, struct value_filter_ref, list);

                fn(filter->ptr, ctx);
        }
}

void *value_filter_view_nth(struct list_head *filters, size_t index)
{
	struct list_head *pos;
	size_t current = 0;

	if (!filters)
		return NULL;

	list_for_each(pos, filters) {
		struct value_filter_ref *filter;

		if (current++ != index)
			continue;
		filter = list_entry(pos, struct value_filter_ref, list);
		return filter->ptr;
	}
	return NULL;
}

int value_filter_view_clone(
	struct list_head *destination, const struct list_head *source)
{
	const struct list_head *pos;

	if (!destination || !source || destination == source)
		return VALUE_ERR_ARG;

	INIT_LIST_HEAD(destination);
	list_for_each(pos, source) {
		const struct value_filter_ref *source_ref;
		struct value_filter_ref *clone;

		source_ref = list_entry(
			pos, struct value_filter_ref, list);
		clone = value_memory.calloc_fn(
			1, sizeof(*clone), value_memory.ctx);
		if (!clone) {
			value_filter_view_free(destination);
			return VALUE_ERR_NOMEM;
		}
		clone->ptr = source_ref->ptr;
		clone->dealloc_fn = value_memory.dealloc_fn;
		clone->dealloc_ctx = value_memory.ctx;
		list_add_tail(&clone->list, destination);
	}
	return VALUE_OK;
}

void *value_filter_view_reduce(
	struct list_head *filters, value_reduce_fn fn, void *ctx, void *acc)
{
	struct list_head *pos;

	if (!filters || !fn)
		return acc;

	list_for_each(pos, filters) {
		struct value_filter_ref *filter;

		filter = list_entry(pos, struct value_filter_ref, list);
		acc = fn(acc, filter->ptr, ctx);
	}
	return acc;
}

void value_filter_view_sort(
	struct list_head *filters, value_sort_fn fn, void *ctx)
{
	struct list_head *pos, *next;
	LIST_HEAD(sorted);

	if (!filters || !fn)
		return;

	list_for_each_safe(pos, next, filters) {
		struct value_filter_ref *value;
		struct list_head *item;

		value = list_entry(pos, struct value_filter_ref, list);
		list_del(pos);
		list_for_each(item, &sorted) {
			struct value_filter_ref *sorted_value;

			sorted_value = list_entry(
				item, struct value_filter_ref, list);
			if (fn(value->ptr, sorted_value->ptr, ctx) < 0)
				break;
		}
		list_add_tail(pos, item);
	}
	list_splice_init(&sorted, filters);
}

struct list_head *value_filter_view_map(
	struct list_head *out, struct list_head *filters,
	value_hook_map_fn fn, void *ctx)
{
	struct list_head *pos;

	if (!out || !filters || !fn)
		return out;

	list_for_each(pos, filters) {
		struct value_filter_ref *filter;
		struct list_head *hook;

		filter = list_entry(
			pos, struct value_filter_ref, list);

		hook = fn(filter->ptr, ctx);
		if (hook)
			list_add_tail(hook, out);
	}

	return out;
}
