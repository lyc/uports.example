#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "stack.h"

/* -------------------------------------------------------------------------- */
/* stack ...                                                                  */
/* -------------------------------------------------------------------------- */

struct stack *stack_new(size_t size)
{
        struct stack *stack;

        stack = calloc(1, sizeof(struct stack));
        if (!stack)
                return NULL;

        stack->size = size;
        stack->data = calloc(size, sizeof(struct tvalue));
        if (!stack->data) {
                free(stack);
                return NULL;
        }
        return stack;
}

void stack_free(struct stack *stack)
{
        if (!stack)
                return;

        free(stack->data);
        free(stack);
}

#define TOP (s->index-1)
#define I2I(indices, idx)                     \
        if (indices > 0) {                    \
                idx = indices - 1;            \
        } else if (indices < 0) {             \
                idx = indices + s->index;     \
        }

static void slot_empty(struct stack *s, int idx)
{
        struct tvalue *tv = &s->data[idx];

        if (tv->type != VT_NONE) {
                tv->value = 0L; /* FIXME: free value */
                tv->type = VT_NONE;
        }
}

static struct tvalue slot_get(struct stack *s, int idx)
{
        return s->data[idx];
}

static bool slot_is_empty(struct tvalue *tv)
{
        return (tv->type == VT_NONE) && (tv->value == 0L);
}

static void slot_set(struct stack *s, int idx, struct tvalue *v)
{
        assert(slot_is_empty(&s->data[idx]));
        s->data[idx] = *v;
}

static void slot_copy(struct stack *s, int to, int from)
{
        assert(slot_is_empty(&s->data[to]));
        s->data[to] = s->data[from];
}

static void slot_copy_nil(struct stack *s, int idx)
{
        struct tvalue *tv = &s->data[idx];

        assert(slot_is_empty(tv));
        tv->type = VT_NIL;
        tv->value = 0L;
}

void stack_pushnil(struct stack *s)
{
        struct tvalue tv;

        assert(s->index < s->size);

        tv.type = VT_NIL;
        tv.value = 0L;
        slot_set(s, s->index, &tv);

        /* update index */
        s->index++;
}

void stack_pushinteger(struct stack *s, ptrdiff_t i)
{
        struct tvalue tv;

        assert(s->index < s->size);

        tv.type = VT_PTR;
        tv.value = i;
        slot_set(s, s->index, &tv);

        /* update index */
        s->index++;
}

void stack_pushunsigned(struct stack *s, unsigned int u)
{
        struct tvalue tv;

        assert(s->index < s->size);

        tv.type = VT_NUMBER;
        tv.value = u;
        slot_set(s, s->index, &tv);

        /* update index */
        s->index++;
}

enum value_type stack_type(struct stack *s, int indices)
{
        int idx;

        I2I(indices, idx);
        assert(idx >= 0 && idx < s->size);
        return s->data[idx].type;
}

const char *stack_typename(struct stack *s, enum value_type type)
{
        (void)(s);

        switch (type) {
        case VT_NIL: return "VT_NIL";
        case VT_BOOL: return "VT_BOOL";
        case VT_NUMBER: return "VT_NUMBER";
        case VT_PTR: return "VT_PTR";
        case VT_STRING: return "VT_STRING";
        case VT_NONE: return NULL;
        }
        return NULL;
}

ptrdiff_t stack_tointeger(struct stack *s, int indices)
{
        int idx;
        struct tvalue *tv;

        I2I(indices, idx);
        assert(idx >= 0 && idx < s->size);
        tv = &s->data[idx];
        assert(!slot_is_empty(tv));
        assert(tv->type == VT_PTR);
        return tv->value;
}

unsigned int stack_tounsigned(struct stack *s, int indices)
{
        int idx;
        struct tvalue *tv;

        I2I(indices, idx);
        assert(idx >= 0 && idx < s->size);
        tv = &s->data[idx];
        assert(!slot_is_empty(tv));
        assert(tv->type == VT_NUMBER);
        return (unsigned int)tv->value;
}

int stack_gettop(struct stack *s)
{
        return s->index;
}

void stack_settop(struct stack *s, int indices)
{
        int idx, i;

        /* index convert & range check */
        I2I(indices, idx);
        if (idx < 0)
                idx = 0;
        else if (idx >= s->size)
                idx = s->size - 1;

        /* no change is needed if indices (new top) equal to top */
        if (idx == TOP)
                return;

        /* check if values need to adjust ... */
        if (idx > TOP) {
                /* pushing nils */
                for (i=idx; i>TOP; i--)
                        slot_copy_nil(s, i);
        } else if (idx < TOP) {
                /* popping values */
                for (i=TOP; i<idx; i--)
                        slot_empty(s, i);
        }

        /* update index */
        s->index = idx + 1;
}

void stack_pushvalue(struct stack *s, int indices)
{
        int idx;

        if (s->index <= 0)
                return;

        /* index convert */
        I2I(indices, idx);
        if (idx >= s->index || idx < 0)
                return;

        /* slot should be empty */
        assert(slot_is_empty(&s->data[s->index]));

        /* popping a copy value at indices */
        slot_copy(s, s->index, idx);

        /* update index */
        s->index++;
}

void stack_remove(struct stack *s, int indices)
{
        int idx, i;

        if (s->index <= 0)
                return;

        /* index convert */
        I2I(indices, idx);
        if (idx >= s->index || idx < 0)
                return;

        /* remove data */
        slot_empty(s, idx);

        /* check if "shifting down" is needed */
        if (idx != TOP) {
                for (i=idx; i<TOP; i++) {
                        /* shift down */
                        slot_copy(s, i, i+1);
                        slot_empty(s, i+1);
                }
        }

        /* update index */
        s->index--;
}

void stack_insert(struct stack *s, int indices)
{
        int idx, i;
        struct tvalue value;

        if (s->index <= 0)
                return;

        /* index convert */
        I2I(indices, idx);
        if (idx >= s->index || idx < 0)
                return;

        /* no change is needed if indices equal to top */
        if (idx == TOP)
                return;

        /* popping the value at top */
        value = slot_get(s, TOP);
        slot_empty(s, TOP);

        /* check if "shifting up" is needed */
        for (i=TOP-1; i>=idx; i--) {
                slot_copy(s, i+1, i);
                slot_empty(s, i);
        }

        /* insert popping value into indices */
        slot_set(s, idx, &value);

        /* NOTE: index no need to change */
}

void stack_replace(struct stack *s, int indices)
{
        int idx;
        struct tvalue value;

        if (s->index <= 0)
                return;

        /* index convert */
        I2I(indices, idx);
        if (idx >= s->index || idx < 0)
                return;

        /* no change is needed if indices equal to top */
        if (idx == TOP)
                return;

        /* popping the value at top */
        value = slot_get(s, TOP);
        slot_empty(s, TOP);

        /* replacing  */
        slot_empty(s, idx);
        slot_set(s, idx, &value);

        /* update index */
        s->index--;
}

void stack_copy(struct stack *s, int from, int to)
{
        if (from == to)
                return;

        stack_pushvalue(s, from);
        if (to < 0)
                stack_replace(s, to-1);
        else
                stack_replace(s, to);
}
