#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include <tree.h>

#ifndef NOT_USED
#define NOT_USED(a) (void)(a)
#endif

/* ------------------------------------------------------------------------- */
/* BST ...                                                                   */
/* ------------------------------------------------------------------------- */

/*
 *  Binary-search tree...
 */

struct bstree {
        struct bi_node node;

        char key[32];
};

// typedef int (bi_comparer)(struct bi_node *a, struct bi_node *b);

int bst_compare(char *a, char *b)
{
        int ai, bi;

        ai = atoi(a);
        bi = atoi(b);
        if (ai < bi)
                return -1;
        else if (ai > bi)
                return 1;
        else
                return 0;
}

struct bstree *bstree_search(struct bi_root *root, char *key)
{
        int result;
        struct bi_node *n;
        struct bstree *t;

        n = root->bi_node;
        while (n) {
                t = container_of(n, struct bstree, node);
                result = bst_compare(key, t->key);

                if (result < 0) {
                        n = n->bi_left;
                } else if (result > 0) {
                        n = n->bi_right;
                } else {
                        printf("=> found node: %s\n", t->key);
                        return t;
                }
        }
        printf("=> couldn't found node: %s\n", key);
        return NULL;
}

int bstree_add(struct bi_root *root, struct bstree *my)
{
        int result;
        struct bi_node **n = &root->bi_node, *parent = NULL;

        /* Figure out where to put new node */
        while (*n) {
                result = bst_compare(my->key,
                                 bi_entry(*n, struct bstree, node)->key);

                parent = *n;
                if (result < 0)
                        n = &((*n)->bi_left);
                else if (result > 0)
                        n = &((*n)->bi_right);
                else
                        return -1;
        }

        /* Add new node and rebalance tree. */
        printf("=> add node: %s\n", my->key);
        bi_link_node(&my->node, parent, n);

        return 0;
}

void bstree_remove(struct bi_root *root, char *key)
{
        struct bstree *t;

        t = bstree_search(root, key);
        if (t) {
                printf("=> delete node: %s\n", t->key);
                bi_erase(root, &t->node);
                free(t);
        }
}

int output(struct bi_node *n, void *data)
{
        struct bstree *t;

        NOT_USED(data);
        t = bi_entry(n, struct bstree, node);
        printf("%s ", t->key);
        return 0;
}

int BST_test(int argc, char *argv[])
{
        int i;
        struct bstree *t;

        /*
         *                 12
         *                /  \
         *               2    18
         *              / \     \
         *            -4   8     21
         *                /     /  \
         *               6     19   25
         *                         /
         *                       22
         *                         \
         *                          23
         */
        char *str_add[] = {
                "12", "2", "18", "-4", "8", "6", "21", "19", "25", "22", "23",
                NULL,
        };
        char *str_delete[] = {
                "-4", "15", "8", "18", "21", "12", "50", "2", "6", "19", "25",
                "22", "0", "23", "-4", NULL,
        };
        struct bi_root root;
        struct bi_node *n;

        NOT_USED(argc);
        NOT_USED(argv);

        INIT_BI_ROOT(&root);

        if (BI_EMPTY_ROOT(&root))
                printf("===> empty bi_root...\n");

        t = bstree_search(&root, "1");
        if (t)
                printf("found node with key<%s>\n", t->key);

        for (i=0; str_add[i]; i++) {
                t = malloc(sizeof(struct bstree));
                if (!t) {
                        printf("Oops, unable to allocate memory!\n");
                        return -1;
                }
                strcpy(t->key, str_add[i]);
                bstree_add(&root, t);
        }

        printf("=====> pre-order ...\n");
        bi_preorder(root.bi_node, output, NULL);
        printf("\n");

        printf("=====> in-order ...\n");
        bi_inorder(root.bi_node, output, NULL);
        printf("\n");

        printf("=====> post-order ...\n");
        bi_postorder(root.bi_node, output, NULL);
        printf("\n");

        printf("=====> forward iterate ...\n");
        for (n=bi_first(&root); n; n=bi_next(n))
                printf("%s ", bi_entry(n, struct bstree, node)->key);
        printf("\n");

        printf("=====> backword iterate ...\n");
        for (n=bi_last(&root); n; n=bi_prev(n))
                printf("%s ", bi_entry(n, struct bstree, node)->key);
        printf("\n");

        printf("\n=====> erase test ...\n");
        for (i=0; str_delete[i]; i++)
                bstree_remove(&root, str_delete[i]);

        printf("\n=====> final check ...\n");
        if (BI_EMPTY_ROOT(&root))
                printf("===> empty bi_root...\n");

        return 0;
}

/* ------------------------------------------------------------------------- */
/* main ...                                                                  */
/* ------------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
        NOT_USED(argc);
        NOT_USED(argv);

        BST_test(argc, argv);
        return 0;
}
