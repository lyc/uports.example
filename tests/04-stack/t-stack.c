#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifndef NOT_USED
#define NOT_USED(a) (void)(a)
#endif

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

static void test_settop(void)
{
        struct stack *s = stack_new(4);

        assert(s);
        stack_pushunsigned(s, 10);
        stack_pushunsigned(s, 20);
        stack_pushunsigned(s, 30);
        stack_settop(s, 1);
        assert(stack_gettop(s) == 1);
        assert(stack_tounsigned(s, 1) == 10);
        assert(s->data[1].type == VT_NONE && s->data[1].value == 0);
        assert(s->data[2].type == VT_NONE && s->data[2].value == 0);
        stack_pushunsigned(s, 40);
        assert(stack_tounsigned(s, -1) == 40);

        stack_settop(s, 0);
        assert(stack_gettop(s) == 0);
        stack_settop(s, 0);
        stack_settop(s, 4);
        assert(stack_gettop(s) == 4);
        for (int i = 1; i <= 4; ++i)
                assert(stack_type(s, i) == VT_NIL);
        stack_settop(s, -2);
        assert(stack_gettop(s) == 3);
        stack_settop(s, -20);
        assert(stack_gettop(s) == 0);
        stack_pushunsigned(s, 50);

        stack_pushvalue(s, 0);
        stack_remove(s, 0);
        stack_insert(s, 0);
        stack_replace(s, 0);
        assert(stack_gettop(s) == 1);
        assert(stack_tounsigned(s, 1) == 50);
        stack_free(s);
}

int main(int argc, char **argv)
{
        unsigned int value;
        struct stack *ts;
        char *p1 = "ABC";
        ptrdiff_t p;

        NOT_USED(argc);
        NOT_USED(argv);

        test_settop();

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
        return 0;
}
