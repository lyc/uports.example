#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "tree.h"

/*
 *  Binary operation...
 */

struct bi_node *bi_leftmost(struct bi_node *n)
{
        assert(n);

        while (n->bi_left)
                n = n->bi_left;
        return n;
}

struct bi_node *bi_rightmost(struct bi_node *n)
{
        assert(n);

        while (n->bi_right)
                n = n->bi_right;
        return n;
}

void bi_replace_node(struct bi_node *victim, struct bi_node *new)
{
        if (victim->bi_parent) {
                if (victim == victim->bi_parent->bi_left)
                        victim->bi_parent->bi_left = new;
                else
                        victim->bi_parent->bi_right = new;
        }
        victim->bi_left->bi_parent = new;
        victim->bi_right->bi_parent = new;
        new->bi_parent = victim->bi_parent;
        new->bi_left = victim->bi_left;
        new->bi_right = victim->bi_right;
}

/*
 * NOTE:
 *  The node to be detached must be either no children or only has one child,
 *  this limitation must be care by it caller.
 */
struct bi_node *bi_detach_node(struct bi_node *node)
{
        struct bi_node *child;

        /* find the child pointer, it may be NULL */
        if (node->bi_left)
                child = node->bi_left;
        else
                child = node->bi_right;

        /* adujst child's parent pointer */
        if (child)
                child->bi_parent = node->bi_parent;

        /* adujst node parent's children pointer */
        if (node->bi_parent) {
                if (node == node->bi_parent->bi_left)
                        node->bi_parent->bi_left = child;
                else
                        node->bi_parent->bi_right = child;
        }
        return child;
}

void bi_preorder(struct bi_node *n, bi_helper op, void *data)
{
        if (n) {
                if (op)
                        (op)(n, data);
                bi_preorder(n->bi_left, op, data);
                bi_preorder(n->bi_right, op, data);
        }
}

void bi_inorder(struct bi_node *n, bi_helper op, void *data)
{
        if (n) {
                bi_inorder(n->bi_left, op, data);
                if (op)
                        (op)(n, data);
                bi_inorder(n->bi_right, op, data);
        }
}

void bi_postorder(struct bi_node *n, bi_helper op, void *data)
{

        if (n) {
                bi_postorder(n->bi_left, op, data);
                bi_postorder(n->bi_right, op, data);
                if (op)
                        (op)(n, data);
        }
}

int bi_preorder_ex(struct bi_node *n, bi_helper op, void *data)
{
        if (n) {
                if (op)
                        if ((op)(n, data) == BI_STOP)
                                return BI_STOP;
                if (bi_preorder_ex(n->bi_left, op, data) == BI_STOP)
                        return BI_STOP;
                if (bi_preorder_ex(n->bi_right, op, data) == BI_STOP)
                        return BI_STOP;
        }
        return BI_CONTINUE;
}

int bi_inorder_ex(struct bi_node *n, bi_helper op, void *data)
{
        if (n) {
                if (bi_inorder_ex(n->bi_left, op, data) == BI_STOP)
                        return BI_STOP;
                if (op)
                        if ((op)(n, data) == BI_STOP)
                                return BI_STOP;
                if (bi_inorder_ex(n->bi_right, op, data) == BI_STOP)
                        return BI_STOP;
        }
        return BI_CONTINUE;
}

int bi_postorder_ex(struct bi_node *n, bi_helper op, void *data)
{

        if (n) {
                if (bi_postorder_ex(n->bi_left, op, data) == BI_STOP)
                        return BI_STOP;
                if (bi_postorder_ex(n->bi_right, op, data) == BI_STOP)
                        return BI_STOP;
                if (op)
                        if ((op)(n, data) == BI_STOP)
                                return BI_STOP;
        }
        return BI_CONTINUE;
}

/*
 *  In-order operation...
 */

struct bi_node *bi_next(struct bi_node *n)
{
        struct bi_node *node;

        if (!n)
                return NULL;

        if (n->bi_right)
                return bi_leftmost(n->bi_right);

        node = n->bi_parent;
        while (node && n == node->bi_right) {
                n = node;
                node = node->bi_parent;
        }
        return node;
}

struct bi_node *bi_prev(struct bi_node *n)
{
        struct bi_node *node;

        if (!n)
                return NULL;

        if (n->bi_left)
                return bi_rightmost(n->bi_left);

        node = n->bi_parent;
        while (node && n == node->bi_left) {
                n = node;
                node = node->bi_parent;
        }
        return node;
}

struct bi_node *bi_first(struct bi_root *root)
{
        if (BI_EMPTY_ROOT(root))
                return NULL;
        return bi_leftmost(root->bi_node);
}

struct bi_node *bi_last(struct bi_root *root)
{
        if (BI_EMPTY_ROOT(root))
                return NULL;
        return bi_rightmost(root->bi_node);
}

void bi_erase(struct bi_root *root, struct bi_node *node)
{
        struct bi_node *n;

        if (!node)
                return;

        /*
         * The node need to be erased has three possible situations:
         *
         *   case 1: the node has no children (it is leaf)
         *   case 2: the node hos only one child
         *   case 3: the node has two children (both left and right)
         *
         * In case 1 or case 2, erase the node is straightforward,
         * just detach itself by re-link its parent and child.
         *
         * In case 3, one of BST properity is "the predecessor or successor
         * of this node must be in either case 1 or case 2", by using this
         * properity, erase case 3 node can become below three steps:
         *
         *   step 1: find its successor, and
         *   step 2: detach its successor from BST, then
         *   step 3: replace this to be erased node by its successor
         */

        if (node->bi_left && node->bi_right) {
                /* case 3, perform above three steps.
                 *
                 * please be noted that successor (returned by bi_leftmost)
                 * will become BST's new root candidate.
                 */
                n = bi_leftmost(node->bi_right);
                bi_detach_node(n);
                bi_replace_node(node, n);
        } else {
                /* case 1 and 2, just detach itself.
                 *
                 * save node's child pointer (returned by bi_detach_node)
                 * in case if it's BST new root candidate.
                 */
                n = bi_detach_node(node);
        }

        /* reset BST's root in case erase root node */
        if (node == root->bi_node)
                root->bi_node = n;
}
