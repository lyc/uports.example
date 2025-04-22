#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef NOT_USED
#define NOT_USED(a) (void)(a)
#endif

#include "list.h"

int main(int argc, char **argv);

/* ------------------------------------------------------------------------- */
/*                                                                           */
/* ------------------------------------------------------------------------- */

struct node {
        struct node *parent;
        struct node *left;
        struct node *right;
} __attribute__((aligned(sizeof(long))));

struct B {
        int data;
        struct node n;
};

void testB(struct node* n)
{
        struct B *pB;

        pB = (struct B*) container_of(n, struct B, n);
        printf("data of pB is: %d\n", pB->data);
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/* ------------------------------------------------------------------------- */

int main(int argc, char **argv)
{
        struct B tb;

        NOT_USED(argc);
        NOT_USED(argv);

        tb.data = 5;
        testB(&tb.n);

        return 0;
}
