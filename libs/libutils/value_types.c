#include <stdlib.h>
#include <string.h>

#include "value_types.h"

/* ------------------------------------------------------------------------- */
/*                                                                           */
/* ------------------------------------------------------------------------- */

static void value_fini_obj(struct value *value)
{
        free(value->name);
        value->name = NULL;
        INIT_LIST_HEAD(&value->list);
}

static int value_copy_obj(struct value *dst, const struct value *src)
{
        char *name = src->name ? strdup(src->name) : NULL;

        if (src->name && !name)
                return VALUE_ERR_NOMEM;

        free(dst->name);
        dst->name = name;
        return VALUE_OK;
}

VALUE_OBJECT_WRAPPERS_BIND(value, value_fini_obj, value_copy_obj, free);
VALUE_OBJECT_BIND(value, NULL, value_fini, value_copy,
                  NULL, NULL, NULL);
VALUE_LIST_BIND(value, name, list, value_copy, value_free);

struct value *value_new(const char *name)
{
        struct value *value = calloc(1, sizeof(*value));

        if (!value)
                return NULL;
        if (name) {
                value->name = strdup(name);
                if (!value->name) {
                        free(value);
                        return NULL;
                }
        }
        INIT_LIST_HEAD(&value->list);
        return value;
}

struct value *value_add_new_tail(struct list_head *head, const char *name)
{
        struct value *value;

        if (!head)
                return NULL;
        value = value_new(name);
        if (!value)
                return NULL;
        value_add_tail_unlocked(head, value);
        return value;
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/* ------------------------------------------------------------------------- */

static void kvalue_fini_obj(struct kvalue *kvalue)
{
        free(kvalue->value);
        free(kvalue->key);
        kvalue->value = NULL;
        kvalue->key = NULL;
        INIT_LIST_HEAD(&kvalue->list);
}

static int kvalue_copy_obj(struct kvalue *dst, const struct kvalue *src)
{
        char *key = src->key ? strdup(src->key) : NULL;
        char *value = src->value ? strdup(src->value) : NULL;

        if ((src->key && !key) || (src->value && !value)) {
                free(value);
                free(key);
                return VALUE_ERR_NOMEM;
        }

        free(dst->value);
        free(dst->key);
        dst->key = key;
        dst->value = value;
        return VALUE_OK;
}

VALUE_OBJECT_WRAPPERS_BIND(kvalue, kvalue_fini_obj, kvalue_copy_obj, free);
VALUE_OBJECT_BIND(kvalue, NULL, kvalue_fini, kvalue_copy,
                  NULL, NULL, NULL);
VALUE_LIST_BIND(kvalue, key, list, kvalue_copy, kvalue_free);

struct kvalue *kvalue_new(const char *key, const char *value)
{
        struct kvalue *kvalue = calloc(1, sizeof(*kvalue));

        if (!kvalue)
                return NULL;
        if (key) {
                kvalue->key = strdup(key);
                if (!kvalue->key)
                        goto fail;
        }
        if (value) {
                kvalue->value = strdup(value);
                if (!kvalue->value)
                        goto fail;
        }
        INIT_LIST_HEAD(&kvalue->list);
        return kvalue;

fail:
        free(kvalue->value);
        free(kvalue->key);
        free(kvalue);
        return NULL;
}

struct kvalue *kvalue_add_new_tail(
        struct list_head *head, const char *key, const char *value)
{
        struct kvalue *kvalue;

        if (!head)
                return NULL;
        kvalue = kvalue_new(key, value);
        if (!kvalue)
                return NULL;
        kvalue_add_tail_unlocked(head, kvalue);
        return kvalue;
}
