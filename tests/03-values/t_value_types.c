#include "t_support.h"
#include <string.h>

#include <value_types.h>

int main(void)
{
        LIST_HEAD(values);
        LIST_HEAD(kvalues);
        struct value *value;
        struct value *value_clone;
        struct kvalue *kvalue;
        struct kvalue *kvalue_clone;

        value = value_add_new_tail(&values, "argument");
        T_CHECK(value);
        T_CHECK(value->name);
        T_CHECK(value_count(&values) == 1);
        T_CHECK(value_lookup_unlocked(&values, "argument") == value);

        value_clone = value_new(NULL);
        T_CHECK(value_clone);
        T_CHECK(value_copy(value_clone, value) == VALUE_OK);
        T_CHECK(value_clone->name);
        T_CHECK(strcmp(value_clone->name, value->name) == 0);
        T_CHECK(value_clone->name != value->name);
        T_CHECK(list_empty(&value_clone->list));

        kvalue = kvalue_add_new_tail(&kvalues, "command", "show status");
        T_CHECK(kvalue);
        T_CHECK(kvalue->key);
        T_CHECK(kvalue->value);
        T_CHECK(kvalue_count(&kvalues) == 1);
        T_CHECK(kvalue_lookup_unlocked(&kvalues, "command") == kvalue);

        kvalue_clone = kvalue_new(NULL, NULL);
        T_CHECK(kvalue_clone);
        T_CHECK(kvalue_copy(kvalue_clone, kvalue) == VALUE_OK);
        T_CHECK(kvalue_clone->key);
        T_CHECK(strcmp(kvalue_clone->key, kvalue->key) == 0);
        T_CHECK(kvalue_clone->value);
        T_CHECK(strcmp(kvalue_clone->value, kvalue->value) == 0);
        T_CHECK(kvalue_clone->key != kvalue->key);
        T_CHECK(kvalue_clone->value != kvalue->value);
        T_CHECK(list_empty(&kvalue_clone->list));

        value_free(value_clone);
        kvalue_free(kvalue_clone);
        value_free_list_unlocked(&values);
        kvalue_free_list_unlocked(&kvalues);
        return 0;
}
