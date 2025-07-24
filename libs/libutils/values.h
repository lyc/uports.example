#ifndef __VALUES_H__
#define __VALUES_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <list.h>

/* ------------------------------------------------------------------------- */
/*                                                                           */
/* ------------------------------------------------------------------------- */

typedef int (*list_cmp_cb)(struct list_head *c1, struct list_head *c2,
                           void *data);

size_t list_count(struct list_head *values);
void list_sort_insertion(struct list_head *head, list_cmp_cb func, void *data);

/* ------------------------------------------------------------------------- */
/*                                                                           */
/* ------------------------------------------------------------------------- */

enum {
        VF_PRIMITIVE = 0,
        VF_SHALLOW,
        VF_DEEP,

        VF_FIRST = 0,
        VF_SORT,
        VF_NOEXISTED,
        VF_LAST,
};

/* ------------------------------------------------------------------------- */
/* struct filter                                                             */
/* ------------------------------------------------------------------------- */

struct filter {
        /* no key */
        void *ptr;

        struct list_head list; /* hook */
};

struct filter *new_filter(void *ptr);
void free_filter(struct filter *filter);

void free_filter_list(struct list_head *filters);
int clone_filter_list(struct list_head *clone,
                      int start, int end, struct list_head *src);

typedef void (*filter_cb)(void *ptr, void *data);
void foreach_filter_list(filter_cb func, void *data, struct list_head *filters);

typedef struct list_head *(*filter_map_cb)(void *ptr, void *data);
void map_filter_list(filter_map_cb func, void *data, struct list_head *filters);

typedef void *(*filter_reduce_cb)(void *carry, void *ptr, void *data);
void *reduce_filter_list(filter_reduce_cb func, void *data,
                         struct list_head *filters, void *carry);

struct filter *first_filter(struct list_head *filters);
struct filter *nth_filter(struct list_head *filters, size_t n);

/* ------------------------------------------------------------------------- */
/* macro                                                                     */
/*                                                                           */
/*     provide below functions, XXX can be value, kvalue, etc...             */
/*                                                                           */
/* ------------------------------------------------------------------------- */

/*
 *     value base API
 *     --------------
 *     struct XXX *new_XXX(char *name);
 *     void free_XXX(struct XXX *XXX);
 *     struct XXX *clone_XXX(struct XXX *XXX);
 *     int compare_XXX(struct XXX *c1, void *data, int flag);
 *                                    struct list_head *XXXs);
 *     NOTE: Any value type,
 *           _new_XXX, _free_XXX, _copy_XXX, and _compare_XXX must be provided.
 *
 *
 *     value list API
 *     --------------
 *     typedef bool (*XXX_free_cb)(struct XXX *XXX, void *data);
 *     void free_XXX_list(XXX_free_cb func, void *data, struct list_head *XXXs);
 *
 *
 *     value list common API
 *     ---------------------
 *     typedef void (*XXX_cb)(struct XXX *XXX, void *data);
 *     void foreach_XXX_list(XXX_cb func, void *data, struct list_head *XXXs);
 *
 *     typedef struct list_head *(*XXX_map_cb)(struct XXX *XXX, void *data);
 *     struct list_head *map_XXX_list(struct list_head *maps,
 *                                    XXX_map_cb func, void *data,
 *                                    struct list_head *XXXs);
 *
 *     typedef bool (*XXX_filter_cb)(struct XXX *XXX, void *data);
 *     struct list_head *filter_XXX_list(struct list_head *filters,
 *                                       XXX_filter_cb func, void *data,
 *                                       struct list_head *XXXs);
 *
 *     typedef void *(*XXX_reduce_cb)(void *carry, struct XXX *XXX, void *data);
 *     void *reduce_XXX_list(XXX_reduce_cb func, void *data,
 *                           struct list_head *XXXs, void *carry);
 */

#define VALUE_HELPER_COMMON_HEADER_DEFINE(type, name, hook, head)             \
struct type *new_ ## type(const char *name);                                  \
void free_ ## type(struct type *type);                                        \
struct type *clone_ ## type(struct type *type);                               \
int compare_ ## type(struct type *c1, void *data, int flag);                  \
                                                                              \
typedef bool (*type ## _free_cb)(struct type *type, void *data);              \
void free_ ## type ## _list(type ## _free_cb func, void *data,                \
                            struct list_head *head);                          \
                                                                              \
typedef void (*type ## _cb)(struct type *type, void *data);                   \
void foreach_ ## type ## _list(type ## _cb func, void *data,                  \
                               struct list_head *head);                       \
typedef struct list_head *(*type ## _map_cb)(struct type *type, void *data);  \
struct list_head *map_ ## type ## _list(struct list_head *maps,               \
                                        type ## _map_cb func, void *data,     \
                                        struct list_head *head);              \
typedef bool (*type ## _filter_cb)(struct type *type, void *data);            \
struct list_head *filter_ ## type ## _list(struct list_head *filters,         \
                                           type ##_filter_cb func, void *data,\
                                           struct list_head *head);           \
typedef void *(*type ##_reduce_cb)(void *carry, struct type *type, void *data);\
void *reduce_ ## type ## _list(type ## _reduce_cb func, void *data,           \
                               struct list_head *head, void *carry)

#define VALUE_HELPER_COMMON_DEFINE(type, name, hook, head)                    \
struct type *new_ ## type(const char *name)                                   \
{                                                                             \
        struct type *type;                                                    \
                                                                              \
        type = calloc(1, sizeof(struct type));                                \
        if (!type)                                                            \
                return NULL;                                                  \
                                                                              \
        _new_ ## type(type, name);                                            \
        return type;                                                          \
}                                                                             \
                                                                              \
void free_ ## type(struct type *type)                                         \
{                                                                             \
        if (!type)                                                            \
                return;                                                       \
                                                                              \
        _free_ ## type(type);                                                 \
                                                                              \
        if (type->name)                                                       \
                free(type->name);                                             \
        free(type);                                                           \
}                                                                             \
                                                                              \
struct type *clone_ ## type(struct type *type)                                \
{                                                                             \
        struct type *clone;                                                   \
                                                                              \
        if (!type)                                                            \
                return NULL;                                                  \
                                                                              \
        clone = new_ ## type(type->name);                                     \
        if (clone)                                                            \
                _copy_ ## type(clone, type);                                  \
        return clone;                                                         \
}                                                                             \
                                                                              \
int compare_ ## type(struct type *c1, void *data, int flag)                   \
{                                                                             \
        int rc;                                                               \
        struct type *c2;                                                      \
                                                                              \
        /* always return -1 (less) if NULL comparator */                      \
        if (!data)                                                            \
                return -1;                                                    \
                                                                              \
        /* PRIMITIVE compare */                                               \
        if (flag == VF_PRIMITIVE)                                             \
                return strcmp(c1->name, (char *)data);                        \
                                                                              \
        c2 = (struct type *)data;                                             \
                                                                              \
        /* SHALLOW compare */                                                 \
        rc = strcmp(c1->name, c2->name);                                      \
        if (flag == VF_SHALLOW)                                               \
                return rc;                                                    \
                                                                              \
        /* DEEP compare */                                                    \
        if (flag == VF_DEEP && rc != 0)                                       \
                return rc;                                                    \
                                                                              \
        return _compare_ ## type(c1, c2);                                     \
}                                                                             \
                                                                              \
void free_ ## type ## _list(type ## _free_cb func, void *data,                \
                            struct list_head *head)                           \
{                                                                             \
        bool check = true;                                                    \
        struct list_head *p, *n;                                              \
        struct type *type;                                                    \
                                                                              \
        list_for_each_safe(p, n, head) {                                      \
                type = list_entry(p, struct type, hook);                      \
                if (func)                                                     \
                        check = (func)(type, data);                           \
                if (check) {                                                  \
                        list_del(p);                                          \
                        free_ ## type(type);                                  \
                }                                                             \
        }                                                                     \
}                                                                             \
                                                                              \
void foreach_ ## type ## _list(type ## _cb func, void *data,                  \
                               struct list_head *head)                        \
{                                                                             \
        struct list_head *p;                                                  \
        struct type *type;                                                    \
                                                                              \
        list_for_each(p, head) {                                              \
                type = list_entry(p, struct type, hook);                      \
                (func)(type, data);                                           \
        }                                                                     \
}                                                                             \
                                                                              \
struct list_head *map_ ## type ## _list(struct list_head *maps,               \
                                        type ## _map_cb func, void *data,     \
                                        struct list_head *head)               \
{                                                                             \
        struct list_head *p, *n;                                              \
        struct type *type;                                                    \
                                                                              \
        list_for_each(p, head) {                                              \
                type = list_entry(p, struct type, hook);                      \
                n = (func)(type, data);                                       \
                if (n)                                                        \
                        list_add_tail(n, maps);                               \
        }                                                                     \
        return maps;                                                          \
}                                                                             \
                                                                              \
struct list_head *filter_ ## type ## _list(struct list_head *filters,         \
                                           type ##_filter_cb func, void *data,\
                                           struct list_head *head)            \
{                                                                             \
        struct list_head *p;                                                  \
        struct type *type;                                                    \
                                                                              \
        list_for_each(p, head) {                                              \
                type = list_entry(p, struct type, hook);                      \
                if ((func)(type, data)) {                                     \
                        struct filter *filter = new_filter(type);             \
                        if (filter)                                           \
                                list_add_tail(&filter->list, filters);        \
                }                                                             \
        }                                                                     \
        return filters;                                                       \
}                                                                             \
                                                                              \
void *reduce_ ## type ## _list(type ## _reduce_cb func, void *data,           \
                               struct list_head *head, void *carry)           \
{                                                                             \
        struct list_head *p;                                                  \
        struct type *type;                                                    \
                                                                              \
        list_for_each(p, head) {                                              \
                type = list_entry(p, struct type, hook);                      \
                carry = (func)(carry, type, data);                            \
        }                                                                     \
        return carry;                                                         \
}

/*
 *     value extra API
 *     --------------
 *     void add_XXX(struct XXX &new, int flag, struct list_head *XXXs);
 *     struct XXX *lookup_XXX(const char *name, struct list_head *XXXs);
 */

#define VALUE_HELPER_EXTRA_HEADER_DEFINE(type, name, hook, head)              \
struct list_head *add_ ## type(struct type *new, int flag,                    \
                               struct list_head *head);                       \
struct type *lookup_ ## type(const char *name, struct list_head *head);       \
struct type *nth_ ## type(struct list_head *head, size_t n)

#define VALUE_HELPER_EXTRA_DEFINE(type, name, hook, head)                     \
struct list_head *add_ ## type(struct type *new, int flag,                    \
                            struct list_head *head)                           \
{                                                                             \
        struct list_head *p;                                                  \
        struct type *type;                                                    \
                                                                              \
        if (flag == VF_FIRST) {                                               \
                list_add(&new->list, head);                                   \
        } else if (flag == VF_LAST) {                                         \
                list_add_tail(&new->list, head);                              \
        } else if (flag == VF_SORT) {                                         \
                list_for_each(p, head) {                                      \
                        type = list_entry(p, struct type, hook);              \
                        if (compare_ ## type(new, type, VF_SHALLOW) < 0)      \
                                break;                                        \
                }                                                             \
                list_add_tail(&new->hook, p);                                 \
        } else if (flag == VF_NOEXISTED) {                                    \
        }                                                                     \
        return 0;                                                             \
}                                                                             \
                                                                              \
struct type *lookup_ ## type(const char *name, struct list_head *head)        \
{                                                                             \
        struct list_head *p;                                                  \
        struct type *type;                                                    \
                                                                              \
        list_for_each(p, head) {                                              \
                type = list_entry(p, struct type, list);                      \
                if (compare_ ## type(type, (void *)name, VF_PRIMITIVE) == 0)  \
                        return type;                                          \
        }                                                                     \
        return NULL;                                                          \
}                                                                             \
                                                                              \
struct type *nth_ ## type(struct list_head *head, size_t n)                   \
{                                                                             \
        struct list_head *p;                                                  \
        struct type *type;                                                    \
        size_t i = 0;                                                         \
                                                                              \
        list_for_each(p, head) {                                              \
                type = list_entry(p, struct type, list);                      \
                if (i++ == n)                                                 \
                        return type;                                          \
        }                                                                     \
        return NULL;                                                          \
}

/* ------------------------------------------------------------------------- */
/* struct value                                                              */
/* ------------------------------------------------------------------------- */

struct value {
        char *name;

        struct list_head list; /* hook */
};

void _new_value(struct value *value, const char *name);
void _free_value(struct value *value);
void _copy_value(struct value *value, struct value *src);
int _compare_value(struct value *c1, struct value *c2);

VALUE_HELPER_COMMON_HEADER_DEFINE(value, name, list, values);
VALUE_HELPER_EXTRA_HEADER_DEFINE(value, name, list, values);

struct value *add_value_ex(const char *value, struct list_head *values);

/* ------------------------------------------------------------------------- */
/* struct kvalue                                                             */
/* ------------------------------------------------------------------------- */

struct kvalue {
        char *key;
        char *value;

        struct list_head list; /* hook */
};

void _new_kvalue(struct kvalue *kvalue, const char *name);
void _free_kvalue(struct kvalue *kvalue);
void _copy_kvalue(struct kvalue *kvalue, struct kvalue *src);
int _compare_kvalue(struct kvalue *c1, struct kvalue *c2);

VALUE_HELPER_COMMON_HEADER_DEFINE(kvalue, key, list, kvalues);
VALUE_HELPER_EXTRA_HEADER_DEFINE(kvalue, key, list, kvalues);

struct kvalue *add_kvalue_ex(const char *key, const char *value,
                             struct list_head *kvalues);

#endif /* __VALUES_H__ */
