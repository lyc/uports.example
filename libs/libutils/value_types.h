#ifndef VALUE_TYPES_H
#define VALUE_TYPES_H

#include <value_base.h>

/* ------------------------------------------------------------------------- */
/*                                                                           */
/* ------------------------------------------------------------------------- */

struct value {
        char *name;
        struct list_head list;
};

VALUE_OBJECT_WRAPPERS_HEADER(value);
VALUE_OBJECT_HEADER(value);
VALUE_LIST_HEADER(value);
VALUE_LIST_LOOKUP_HEADER(value);

struct value *value_new(const char *name);
void value_free(struct value *value);
struct value *value_add_new_tail(struct list_head *head, const char *name);

/* ------------------------------------------------------------------------- */
/*                                                                           */
/* ------------------------------------------------------------------------- */

struct kvalue {
        char *key;
        char *value;
        struct list_head list;
};

VALUE_OBJECT_WRAPPERS_HEADER(kvalue);
VALUE_OBJECT_HEADER(kvalue);
VALUE_LIST_HEADER(kvalue);
VALUE_LIST_LOOKUP_HEADER(kvalue);

struct kvalue *kvalue_new(const char *key, const char *value);
void kvalue_free(struct kvalue *kvalue);
struct kvalue *kvalue_add_new_tail(
        struct list_head *head, const char *key, const char *value);

#endif /* VALUE_TYPES_H */
