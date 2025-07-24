#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "values.h"

/* ------------------------------------------------------------------------- */
/* main ...                                                                  */
/* ------------------------------------------------------------------------- */

#define RFORMAT "%38s:       "

void _dump_v_cb(struct value *value, void *data)
{
        (void)data;

        printf(RFORMAT, value->name);
        printf("(%p)", value);
        printf("\n");
}

bool _name_v_cb(struct value *value, void *data)
{
        char c;

        (void)data;
        c = value->name[0];
        if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u')
                return true;
        return false;
}

void _dump_fv_cb(void *ptr, void *data)
{
        struct value *value = (struct value *)ptr;

        (void)data;
        printf(RFORMAT, value->name);
        printf("\n");
}

struct list_head *_v_2_kv_cb(struct value *value, void *data)
{
        char *p;
        int i, len, diff;
        struct kvalue *kvalue;

        (void)data;
        kvalue = new_kvalue(value->name);
        if (!kvalue)
                return NULL;
        kvalue->value = strdup(value->name);
        len = strlen(kvalue->value);
        diff  = 'a' - 'A';
        for (p=kvalue->value, i=0; i<len; i++, p++) {
                if (*p >= 'a')
                        *p -= diff;
        }
        return &kvalue->list;
}

void _dump_kv_cb(struct kvalue *kvalue, void *data)
{
        (void)data;

        printf(RFORMAT, kvalue->key);
        printf("(%p, %s)\n", kvalue, kvalue->value);
}

void *_cal_kv_cb(void *carry, struct kvalue *kvalue, void *data)
{
        int *cnt = (int *)carry;
        int len, i;
        char *p, c = ((char *)data)[0];

        len = strlen(kvalue->key);
        for (p=kvalue->key, i=0; i<len; i++, p++)
                if (*p == c)
                        *cnt += 1;
        return carry;
}

int main(int argc, char *argv[])
{
        (void)argc;
        (void)argv;

        LIST_HEAD(lt);
        add_value(new_value("pinapple"), VF_SORT, &lt);
        add_value(new_value("guava"), VF_SORT, &lt);
        add_value(new_value("apple"), VF_SORT, &lt);
        add_value(new_value("orange"), VF_SORT, &lt);
        add_value(new_value("banana"), VF_SORT, &lt);
        add_value(new_value("peach"), VF_FIRST, &lt);
        foreach_value_list(_dump_v_cb, NULL, &lt);

        LIST_HEAD(filters);
        filter_value_list(&filters, _name_v_cb, NULL, &lt);
        foreach_filter_list(_dump_fv_cb, NULL, &filters);
        free_filter_list(&filters);

        LIST_HEAD(maps);
        map_value_list(&maps, _v_2_kv_cb, NULL, &lt);
        foreach_kvalue_list(_dump_kv_cb, NULL, &maps);
        int cnt = 0;
        reduce_kvalue_list(_cal_kv_cb, "a", &maps, &cnt);
        printf("total of %s have: %d\n", "a", cnt);
        free_kvalue_list(NULL, NULL, &maps);

        free_value_list(NULL, NULL, &lt);
        return 0;
}
