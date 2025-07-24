#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifndef NOT_USED
#define NOT_USED(a) (void)(a)
#endif

#include "values.h"
#include "stack.h"

/* -------------------------------------------------------------------------- */
/*                                                                            */
/* -------------------------------------------------------------------------- */

void stack_dump(struct stack *s)
{
#if 0
	int i;
        struct tvalue *tv;

        NOT_USED(s);

        printf("===> (%s,%d) index = %d\n", __func__, __LINE__, s->index);
	for (i=TOP; i >= 0; i--) {
                tv = &s->data[i];
		printf("s->data[%d] = (%d,%ld(%p))\n",
                       i, tv->type, tv->value, (void *)tv->value);
        }
#else
        int i, top, indices;
        enum value_type vt;

        top = stack_gettop(s);
        printf("===> (%s,%d) top = %d\n", __func__, __LINE__, top);

        for (i=0; i<top; i++) {
                indices = -1 - i;
                vt = stack_type(s, indices);
                printf("stack[%d,%s] = ", indices, stack_typename(s, vt));
                if (vt == VT_NUMBER) {
                        printf("%u", stack_tounsigned(s, indices));
                } else if (vt == VT_PTR) {
                        printf("%p", (void *)stack_tointeger(s, indices));
                }
                printf("\n");
        }
#endif
}

#define RFORMAT "%38s:       "

void _dump_v_cb(struct value *value, void *data)
{
        (void)data;

        printf(RFORMAT, value->name);
        printf("(%p)", value);
        printf("\n");
}

void *_push_cb(void *carry, struct value *value, void *data)
{
        struct stack *stack = (struct stack *)carry;

        (void)data;
        stack_pushinteger(stack, (ptrdiff_t)value);
        return carry;
}

int main(int argc, char **argv)
{
        unsigned int value;
        struct stack *ts;
        char *p1 = "ABC";
        ptrdiff_t p;

        NOT_USED(argc);
        NOT_USED(argv);

        ts = stack_new(SLOT_DEFAULT_SIZE);
        if (!ts)
                return -1;

        stack_pushunsigned(ts, 5); stack_dump(ts);
        stack_pushunsigned(ts, 4); stack_dump(ts);
        stack_pushunsigned(ts, 3); stack_dump(ts);
        stack_pushunsigned(ts, 2); stack_dump(ts);
        stack_pushunsigned(ts, 1); stack_dump(ts);

        printf("===> (%s,%d) pushvalue(-4)\n", __func__, __LINE__);
        stack_pushvalue(ts, -4); stack_dump(ts);
        printf("===> (%s,%d) remove(2)\n", __func__, __LINE__);
        stack_remove(ts, 2); stack_dump(ts);
        printf("===> (%s,%d) insert(-4)\n", __func__, __LINE__);
        stack_insert(ts, -4); stack_dump(ts);
        printf("===> (%s,%d) replace(-3)\n", __func__, __LINE__);
        stack_replace(ts, -3); stack_dump(ts);
        printf("===> (%s,%d) copy(-1,-2)\n", __func__, __LINE__);
        stack_copy(ts, -1, -2); stack_dump(ts);
        printf("===> (%s,%d) copy(1,4)\n", __func__, __LINE__);
        stack_copy(ts, 1, 4); stack_dump(ts);

        value = stack_tounsigned(ts, -1); stack_dump(ts);
        printf("===> (%s,%d) value = %u\n", __func__, __LINE__, value);
        value = stack_tounsigned(ts, -3); stack_dump(ts);
        printf("===> (%s,%d) value = %u\n", __func__, __LINE__, value);


        printf("===> (%s,%d) p1 = %p\n", __func__, __LINE__, p1);
        stack_pushinteger(ts, (ptrdiff_t)p1); stack_dump(ts);
        p = stack_tointeger(ts, -1); stack_dump(ts);
        printf("===> (%s,%d) p = %p(%s)\n",
               __func__, __LINE__, (char *)p, (char *)p);
        printf("===> (%s,%d) copy(-1,-3)\n", __func__, __LINE__);
        stack_copy(ts, -1, -3); stack_dump(ts);
        p = stack_tointeger(ts, -3); stack_dump(ts);
        printf("===> (%s,%d) p = %p(%s)\n",
               __func__, __LINE__, (char *)p, (char *)p);
        stack_free(ts);

        printf("========================>\n");
        LIST_HEAD(values);
        add_value(new_value("pinapple"), VF_LAST, &values);
        add_value(new_value("guava"), VF_LAST, &values);
        add_value(new_value("apple"), VF_LAST, &values);
        add_value(new_value("orange"), VF_LAST, &values);
        add_value(new_value("banana"), VF_LAST, &values);
        add_value(new_value("peach"), VF_LAST, &values);
        foreach_value_list(_dump_v_cb, NULL, &values);
        ts = stack_new(SLOT_DEFAULT_SIZE);
        reduce_value_list(_push_cb, NULL, &values, ts);
        stack_dump(ts);
        while(stack_gettop(ts)) {
                struct value *value = (struct value *)stack_tointeger(ts, 1);
                printf(RFORMAT, value->name);
                printf("\n");

                stack_remove(ts, 1);
        }
        stack_free(ts);
        free_value_list(NULL, NULL, &values);

        return 0;
}
