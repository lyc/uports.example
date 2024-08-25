#ifndef __TREE_H__
#define __TREE_H__

/* ------------------------------------------------------------------------- */
/* borrow from Linux kernel ...                                              */
/* ------------------------------------------------------------------------- */

#undef NULL
#define NULL ((void *)0)

#undef offsetof
#ifdef __compiler_offsetof
#define offsetof(TYPE,MEMBER) __compiler_offsetof(TYPE,MEMBER)
#else
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})

/* ------------------------------------------------------------------------- */
/* binary tree node ...                                                      */
/* ------------------------------------------------------------------------- */

struct bi_node {
        struct bi_node *bi_parent; /* parent */
        struct bi_node *bi_left;   /* left - child */
        struct bi_node *bi_right;  /* right - sibling */
};

struct bi_root {
        struct bi_node *bi_node;
};

#define BI_ROOT (struct bi_root) { NULL, }
#define bi_entry(ptr, type, member) container_of(ptr, type, member)

#define INIT_BI_ROOT(root)  ((root)->bi_node = NULL)
#define BI_EMPTY_ROOT(root)  ((root)->bi_node == NULL)

static inline void bi_link_node(
        struct bi_node *node, struct bi_node *parent, struct bi_node **bi_link)
{
        node->bi_parent = parent;
        node->bi_left = node->bi_right = NULL;

        *bi_link = node;
}

struct bi_node *bi_leftmost(struct bi_node *n);
struct bi_node *bi_rightmost(struct bi_node *n);
void bi_replace_node(struct bi_node *victim, struct bi_node *new);

typedef int (bi_helper)(struct bi_node *n, void *data);
void bi_preorder(struct bi_node *n, bi_helper op, void *data);
void bi_inorder(struct bi_node *n, bi_helper op, void *data);
void bi_postorder(struct bi_node *n, bi_helper op, void *data);

enum {
        BI_CONTINUE = 0,
        BI_STOP,
};

int bi_preorder_ex(struct bi_node *n, bi_helper op, void *data);
int bi_inorder_ex(struct bi_node *n, bi_helper op, void *data);
int bi_postorder_ex(struct bi_node *n, bi_helper op, void *data);

/*
 *  In-order operation
 */

void bi_erase(struct bi_root *root, struct bi_node *node);

struct bi_node *bi_next(struct bi_node *n);
struct bi_node *bi_prev(struct bi_node *n);
struct bi_node *bi_first(struct bi_root *root);
struct bi_node *bi_last(struct bi_root *root);

#endif /* __TREE_H__ */
