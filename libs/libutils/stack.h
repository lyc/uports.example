#ifndef __STACK_H__
#define __STACK_H__

#include <stddef.h>
#include <stdbool.h>

/* -------------------------------------------------------------------------- */
/* struct stack                                                               */
/* -------------------------------------------------------------------------- */

enum value_type {
        VT_NONE = 0,
        VT_NIL,
        VT_BOOL,
        VT_NUMBER,
        VT_PTR,
        VT_STRING,
};

struct tvalue {
        enum value_type type;
        ptrdiff_t value;
};

/* -------------------------------------------------------------------------- */
/* struct stack                                                               */
/* -------------------------------------------------------------------------- */

#define SLOT_DEFAULT_SIZE 20

struct stack {
        int size;
        int index;
        struct tvalue *data;
};

struct stack *stack_new(size_t size);
void stack_free(struct stack *stack);

void stack_pushnil(struct stack *s);
void stack_pushinteger(struct stack *s, ptrdiff_t i);
void stack_pushunsigned(struct stack *s, unsigned int u);
#if 0
/* 0 pushs false, anything else push true */
void stack_pushboolean(struct stack *s, int boolean);
/* for string with embedded zeros */
void stack_pushlstring(struct stack *s, const char *str, size_t u);
/* this just calls pushstring with strlen(str) */
void stack_pushstring(struct stack *s, const char *str);
#endif

enum value_type stack_type(struct stack *s, int indices);
const char *stack_typename(struct stack *s, enum value_type type);

ptrdiff_t stack_tointeger(struct stack *s, int indices);
unsigned int stack_tounsigned(struct stack *s, int indices);
#if 0
#endif

int stack_gettop(struct stack *s); /* index of top element */
/* sets the new top, popping values or pushing nils */
void stack_settop(struct stack *s, int indices);
/* pushes a copy of the value at indices */
void stack_pushvalue(struct stack *s, int indices);
/* remove the value at indices, shiftint down */
void stack_remove(struct stack *s, int indices);
/* pops the value at the top and inserts into indices, shifting up */
void stack_insert(struct stack *s, int indices);
/* pops the value at the top and inserts into indices, replaceing what is there*/
void stack_replace(struct stack *s, int indices);
/* copy the value at "from" to "to", replaceing what is there */
void stack_copy(struct stack *s, int from, int to);

#endif /* __STACK_H__ */
