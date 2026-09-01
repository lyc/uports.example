/*
 * Core typed list verification for the value macro framework.
 *
 * Covers VALUE_OBJECT_BIND, VALUE_LIST_BIND, sorting, lookup,
 * move-to-front, filtering, mapping, reducing, and owned-list cleanup.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_support.h"

#include <value_base.h>

#define RFORMAT "%38s:       "

/* ------------------------------------------------------------------------- */
/* keyed carrier value                                                      */
/* ------------------------------------------------------------------------- */

struct carrier {
        char *name;
        struct list_head list;
};

static void _carrier_fini(struct carrier *c)
{
        if (c->name)
                free(c->name);
}

static int _carrier_copy(struct carrier *dst, const struct carrier *src)
{
	char *name = src->name ? strdup(src->name) : NULL;

	if (src->name && !name)
		return VALUE_ERR_NOMEM;
	free(dst->name);
	dst->name = name;
	return VALUE_OK;
}

static void _carrier_free(struct carrier *c)
{
        free(c);
}

static struct carrier *new_carrier(const char *name)
{
        struct carrier *c;

        c = calloc(1, sizeof(struct carrier));
        if (!c)
                return NULL;
	if (name) {
		c->name = strdup(name);
		if (!c->name) {
			free(c);
			return NULL;
		}
	}
        INIT_LIST_HEAD(&c->list);
        return c;
}

VALUE_OBJECT_WRAPPERS_BIND(carrier, _carrier_fini, _carrier_copy,
                           _carrier_free)
VALUE_OBJECT_BIND(carrier, NULL, carrier_fini, carrier_copy,
                  NULL, NULL, NULL)
VALUE_LIST_BIND(carrier, name, list, carrier_copy, carrier_free)

/* ------------------------------------------------------------------------- */
/* keyed name/text value                                                    */
/* ------------------------------------------------------------------------- */

struct item {
        char *name;
        char *text;
        struct list_head list;
};

static void _item_fini(struct item *i)
{
        if (i->name)
                free(i->name);
        if (i->text)
                free(i->text);
}

static int _item_copy(struct item *dst, const struct item *src)
{
	char *name = src->name ? strdup(src->name) : NULL;
	char *text = src->text ? strdup(src->text) : NULL;

	if ((src->name && !name) || (src->text && !text)) {
		free(text);
		free(name);
		return VALUE_ERR_NOMEM;
	}
	free(dst->text);
	free(dst->name);
	dst->name = name;
	dst->text = text;
	return VALUE_OK;
}

static void _item_free(struct item *i)
{
        free(i);
}

static struct item *new_item(const char *name)
{
        struct item *i;

        i = calloc(1, sizeof(struct item));
        if (!i)
                return NULL;
	if (name) {
		i->name = strdup(name);
		if (!i->name) {
			free(i);
			return NULL;
		}
	}
        INIT_LIST_HEAD(&i->list);
        return i;
}

VALUE_OBJECT_WRAPPERS_BIND(item, _item_fini, _item_copy, _item_free)
VALUE_OBJECT_BIND(item, NULL, item_fini, item_copy, NULL, NULL, NULL)
VALUE_LIST_BIND(item, name, list, item_copy, item_free)

/* ------------------------------------------------------------------------- */
/* callbacks                                                                */
/* ------------------------------------------------------------------------- */

static void _dump_carrier_cb(struct carrier *c, void *data)
{
        (void)data;
        printf(RFORMAT, c->name);
        printf("(%p)", c);
        printf("\n");
}

static bool _name_starts_vowel_cb(const struct carrier *c, void *data)
{
        char ch;

        (void)data;
        if (!c->name)
                return false;
        ch = c->name[0];
        return (ch == 'a' || ch == 'e' || ch == 'i' ||
                ch == 'o' || ch == 'u');
}

static int _carrier_name_cmp_cb(
        struct carrier *a, struct carrier *b, void *data)
{
        (void)data;
        return strcmp(a->name, b->name);
}

static void _dump_filter_cb(void *ptr, void *data)
{
        struct carrier *c = (struct carrier *)ptr;

        (void)data;
        printf(RFORMAT, c->name);
        printf("\n");
}

static struct list_head *_carrier_to_item_cb(struct carrier *c, void *data)
{
        struct item *item;
        char *p;
        int i, len, diff;

        (void)data;
        item = new_item(c->name);
        if (!item)
                return NULL;
        item->text = strdup(c->name);
        len = strlen(item->text);
        diff = 'a' - 'A';
        for (p = item->text, i = 0; i < len; i++, p++) {
                if (*p >= 'a')
                        *p -= diff;
        }
        return &item->list;
}

static void _dump_item_cb(struct item *i, void *data)
{
        (void)data;
        printf(RFORMAT, i->name);
        printf("(%p, %s)\n", i, i->text);
}

static void *_count_char_cb(void *carry, struct item *i, void *data)
{
        int *cnt = (int *)carry;
        int len, j;
        char *p, c = ((char *)data)[0];

        len = strlen(i->name);
        for (p = i->name, j = 0; j < len; j++, p++)
                if (*p == c)
                        *cnt += 1;
        return carry;
}

/* ------------------------------------------------------------------------- */
/* main                                                                      */
/* ------------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
        (void)argc;
        (void)argv;

        struct carrier *c;

        LIST_HEAD(lt);

        /* Preserve the legacy unsorted input sequence, then use the new
         * framework's generic sort operation to produce the same order. */
        carrier_add_tail_unlocked(&lt, new_carrier("pinapple"));
        carrier_add_tail_unlocked(&lt, new_carrier("guava"));
        carrier_add_tail_unlocked(&lt, new_carrier("apple"));
        carrier_add_tail_unlocked(&lt, new_carrier("orange"));
        carrier_add_tail_unlocked(&lt, new_carrier("banana"));
        carrier_sort_unlocked(&lt, _carrier_name_cmp_cb, NULL);

        /* old: add_value(new_value("peach"), VF_FIRST, &lt)
         * new: insert at head to match VF_FIRST behavior */
        c = new_carrier("peach");
        list_add(&c->list, &lt);

        T_CHECK(carrier_nth_unlocked(&lt, 0) == c);
        T_CHECK(!strcmp(carrier_nth_unlocked(&lt, 4)->name, "orange"));
        T_CHECK(carrier_nth_unlocked(&lt, 5));
        T_CHECK(!carrier_nth_unlocked(&lt, 6));

        carrier_foreach_unlocked(&lt, _dump_carrier_cb, NULL);

        /* old: add_value with VF_NOEXISTED — skip if already exists
         * new: use carrier_lookup to check first */
        c = new_carrier("peach");
        if (carrier_lookup_unlocked(&lt, "peach")) {
                printf(RFORMAT, "-----");
                printf("add_value(..., VF_NOEXISTED)\n");
                carrier_free(c);
        }

        /* old: lookup_value */
        c = carrier_lookup_unlocked(&lt, "orange");
        if (c) {
                printf(RFORMAT, "-----");
                printf("lookup_value found orange\n");
                carrier_foreach_unlocked(&lt, _dump_carrier_cb, NULL);
        }

        /* old: lookup_value_ex (move-to-front)
         * new: value_list_lookup_move_front_unlocked */
        c = carrier_lookup_move_front_unlocked(&lt, "orange");
        if (c) {
                printf(RFORMAT, "-----");
                printf("lookup_value_ex found orange\n");
                carrier_foreach_unlocked(&lt, _dump_carrier_cb, NULL);
        }

        printf(RFORMAT, "-----");
        printf("\n");

        LIST_HEAD(filters);
        carrier_filter_view_unlocked(
                &filters, &lt, _name_starts_vowel_cb, NULL);
        value_filter_view_foreach(_dump_filter_cb, NULL, &filters);
        value_filter_view_free(&filters);

        LIST_HEAD(maps);
        carrier_map_unlocked(&maps, _carrier_to_item_cb, NULL, &lt);
        item_foreach_unlocked(&maps, _dump_item_cb, NULL);
        int cnt = 0;
        item_reduce_unlocked(&maps, _count_char_cb, "a", &cnt);
        printf("total of %s have: %d\n", "a", cnt);
        item_free_list_unlocked(&maps);

        carrier_free_list_unlocked(&lt);

        return 0;
}
