#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "values.h"

#define DEBUG_V 1

/* ------------------------------------------------------------------------- */
/*                                                                           */
/* ------------------------------------------------------------------------- */

size_t list_count(struct list_head *values)
{
        size_t n = 0;
        struct list_head *p;

        list_for_each(p, values)
                n++;
        return n;
}

void list_sort_insertion(struct list_head *head, list_cmp_cb func, void *data)
{
        struct list_head *p, *n;
        LIST_HEAD(sorted);

        if (!func) return;
        if (list_empty(head)) return;

        list_for_each_safe(p, n, head) {
                list_del(p);

                struct list_head *s;
                list_for_each(s, &sorted) {
                        if ((func)(p, s, data) < 0)
                                break;
                }
                list_add_tail(p, s);
        }

        // move sorted list back to head
        list_splice_init(&sorted, head);
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/* ------------------------------------------------------------------------- */

struct filter *new_filter(void *ptr)
{
        struct filter *filter;

        filter = calloc(1, sizeof(struct filter));
        if (!filter)
                return NULL;

#if defined(DEBUG_V)
        printf("--------> new filter(%p)\n", ptr);
#endif
        filter->ptr = ptr;
        return filter;
}

void free_filter(struct filter *filter)
{
        if (!filter)
                return;

#if defined(DEBUG_V)
        printf("--------> free filter(%p)\n", filter->ptr);
#endif
        free(filter);
}

int clone_filter_list(struct list_head *clone,
                      int start, int end, struct list_head *src)
{
        struct list_head *p;
        struct filter *new, *filter;
        int si, ei, i = 0;

        si = (start < 0) ? 0 : start;
        ei = (end < 0) ? (int)list_count(src) : end;

        list_for_each(p, src) {
                filter = list_entry(p, struct filter, list);
                if (i >= si && i < ei) {
                        new = new_filter(filter->ptr);
                        if (new)
                                list_add_tail(&new->list, clone);
                }
                i++;
        }
        return list_count(clone);
}

void free_filter_list(struct list_head *filters)
{
        struct list_head *p, *n;
        struct filter *filter;

        list_for_each_safe(p, n, filters) {
                filter = list_entry(p, struct filter, list);
                list_del(p);
                free_filter(filter);
        }
}

void foreach_filter_list(filter_cb func, void *data, struct list_head *filters)
{
        struct list_head *p;
        struct filter *filter;

        list_for_each(p, filters) {
                filter = list_entry(p, struct filter, list);
                (func)(filter->ptr, data);
        }
}

void map_filter_list(filter_map_cb func, void *data, struct list_head *filters)
{
        struct list_head *p, *n, *new;
        struct filter *filter;

        list_for_each_safe(p, n, filters) {
                filter = list_entry(p, struct filter, list);
                new = (func)(filter->ptr, data);
                if (new)
                        list_replace(p, new);
                free_filter(filter);
        }
}

void *reduce_filter_list(filter_reduce_cb func, void *data,
                         struct list_head *filters, void *carry)
{
        struct list_head *p;
        struct filter *filter;

        list_for_each(p, filters) {
                filter = list_entry(p, struct filter, list);
                carry = (func)(carry, filter->ptr, data);
        }
        return carry;
}

struct filter *first_filter(struct list_head *filters)
{
        struct list_head *p, *n;
        struct filter *filter;

        list_for_each_safe(p, n, filters) {
                filter = list_entry(p, struct filter, list);
                list_del(p);
                return filter;
        }
        return NULL;
}

struct filter *nth_filter(struct list_head *filters, size_t n)
{
        struct list_head *p;
        struct filter *filter;
        size_t i = 0;

        list_for_each(p, filters) {
                filter = list_entry(p, struct filter, list);
                if (i++ == n)
                        return filter;
        }
        return NULL;
}

/* ------------------------------------------------------------------------- */
/* struct value                                                              */
/* ------------------------------------------------------------------------- */

inline void _new_value(struct value *value, const char *name)
{
        if (name)
                value->name = strdup(name);
}

inline void _free_value(struct value *value)
{
        (void)value;
}

inline void _copy_value(struct value *value, struct value *src)
{
        (void)value;
        (void)src;
}

inline int _compare_value(struct value *c1, struct value *c2)
{
        (void)c1;
        (void)c2;

        return 0;
}

VALUE_HELPER_COMMON_DEFINE(value, name, list, values);
VALUE_HELPER_EXTRA_DEFINE(value, name, list, values);

/* deprecated */

struct value *add_value_ex(const char *value, struct list_head *values)
{
        struct value *v;

        v = new_value(value);
        if (!v)
                return NULL;
        list_add_tail(&v->list, values);
        return v;
}

/* ------------------------------------------------------------------------- */
/* struct kvalue                                                             */
/* ------------------------------------------------------------------------- */

inline void _new_kvalue(struct kvalue *kvalue, const char *key)
{
        if (key)
                kvalue->key = strdup(key);
}

inline void _free_kvalue(struct kvalue *kvalue)
{
        if (kvalue->value)
                free(kvalue->value);
}

inline void _copy_kvalue(struct kvalue *kvalue, struct kvalue *src)
{
        if (src->value) {
                free(kvalue->value);
                kvalue->value = strdup(src->value);
        }
}

inline int _compare_kvalue(struct kvalue *c1, struct kvalue *c2)
{
        return strcmp(c1->value, c2->value);
}

VALUE_HELPER_COMMON_DEFINE(kvalue, key, list, kvalues);
VALUE_HELPER_EXTRA_DEFINE(kvalue, key, list, kvalues);

/* deprceated */

struct kvalue *add_kvalue_ex(const char *key, const char *value,
                             struct list_head *kvalues)
{
        struct kvalue *kv;

        kv = new_kvalue(key);
        if (!kv)
                return NULL;
        kv->value = strdup(value);
        list_add_tail(&kv->list, kvalues);
        return kv;
}
