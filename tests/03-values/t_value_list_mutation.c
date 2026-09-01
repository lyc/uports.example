/*
 * Value list sorting and removal verification.
 *
 * Proves sort_TYPE_list and remove_if_TYPE_list: generated
 * list operations from VALUE_LIST_BIND / VALUE_LIST_BIND_NOKEY.
 *
 * Per VALUES_FRAMEWORK_DESIGN.md section 7:
 *   - sort_TYPE_list: in-place sort via comparison callback
 *   - remove_if_TYPE_list: remove matching entries, returns count
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <value_base.h>

#include "t_support.h"

/* ------------------------------------------------------------------------- */
/* struct score — string-keyed, sortable, removable                          */
/* ------------------------------------------------------------------------- */

struct score {
        char *name;
        int points;
        struct list_head list;
};

static void score_fini_obj(struct score *s)
{
        if (s->name) free(s->name);
}

static int score_copy_obj(struct score *dst, const struct score *src)
{
        char *name = src->name ? strdup(src->name) : NULL;

        if (src->name && !name)
                return VALUE_ERR_NOMEM;
        free(dst->name);
        dst->name = name;
        dst->points = src->points;
        return VALUE_OK;
}

static void score_free_obj(struct score *s)
{
        free(s);
}

static struct score *new_score(const char *name, int points)
{
        struct score *s = calloc(1, sizeof(struct score));
        if (!s)
                return NULL;
        s->name = strdup(name);
        s->points = points;
        INIT_LIST_HEAD(&s->list);
        return s;
}

VALUE_OBJECT_WRAPPERS_BIND(score, score_fini_obj, score_copy_obj,
                           score_free_obj)
VALUE_OBJECT_BIND(score, NULL, score_fini, score_copy, NULL, NULL, NULL)
VALUE_LIST_BIND(score, name, list, score_copy, score_free)

/* ------------------------------------------------------------------------- */
/* test                                                                      */
/* ------------------------------------------------------------------------- */

static void _dump_score_cb(struct score *s, void *data)
{
        int *i = data;
        printf("  [%d] %s: %d\n", (*i)++, s->name, s->points);
}

static int _sort_by_points_desc(struct score *a, struct score *b,
                                void *data)
{
        (void)data;
        if (a->points > b->points) return -1;
        if (a->points < b->points) return 1;
        return 0;
}

static int _sort_by_name_asc(struct score *a, struct score *b, void *data)
{
        (void)data;
        return strcmp(a->name, b->name);
}

static const char *score_nullable_key(const void *obj)
{
        const struct score *score = obj;

        return score->name;
}

static bool _is_failing(const struct score *s, void *data)
{
        int threshold = *(int *)data;
        return s->points < threshold;
}

static bool score_order_is(struct list_head *scores,
	const char *const *names, size_t count)
{
	size_t index;

	if (score_count(scores) != count)
		return false;
	for (index = 0; index < count; index++) {
		struct score *score = score_nth_unlocked(scores, index);

		if (!score || strcmp(score->name, names[index]) != 0)
			return false;
	}
	return true;
}

static int test_list_edges(void)
{
	LIST_HEAD(scores);
	struct score *first;
	struct score *second;
	int threshold = 101;

	T_CHECK(score_count(&scores) == 0);
	T_CHECK(score_nth_unlocked(&scores, 0) == NULL);
	T_CHECK(score_lookup_unlocked(&scores, "same") == NULL);
	score_sort_unlocked(&scores, _sort_by_name_asc, NULL);
	T_CHECK(score_remove_if_unlocked(&scores, _is_failing,
		&threshold) == 0);

	first = new_score("same", 10);
	second = new_score("same", 20);
	T_CHECK(first && second);
	score_add_tail_unlocked(&scores, first);
	T_CHECK(score_count(&scores) == 1);
	score_sort_unlocked(&scores, _sort_by_name_asc, NULL);
	T_CHECK(score_nth_unlocked(&scores, 0) == first);
	score_add_tail_unlocked(&scores, second);
	T_CHECK(score_lookup_unlocked(&scores, "same") == first);
	T_CHECK(score_remove_if_unlocked(&scores, _is_failing,
		&threshold) == 2);
	T_CHECK(list_empty(&scores));
	return 0;
}

int main(int argc, char *argv[])
{
        (void)argc;
        (void)argv;

        LIST_HEAD(scores);
	int idx;

	T_RUN(test_list_edges);

        /* populate: add in random order */
        score_add_tail_unlocked(&scores, new_score("charlie", 75));
        score_add_tail_unlocked(&scores, new_score("alice", 92));
        score_add_tail_unlocked(&scores, new_score("bob", 58));
        score_add_tail_unlocked(&scores, new_score("diana", 88));
        score_add_tail_unlocked(&scores, new_score("eve", 45));
        score_add_tail_unlocked(&scores, new_score("frank", 95));

        printf("original order (tail-add):\n");
        idx = 0;
		score_foreach_unlocked(&scores, _dump_score_cb, &idx);

        /* sort by points descending */
        score_sort_unlocked(&scores, _sort_by_points_desc, NULL);
        printf("\nsorted by points (desc):\n");
        idx = 0;
		score_foreach_unlocked(&scores, _dump_score_cb, &idx);
		{
			static const char *const expected[] = {
				"frank", "alice", "diana", "charlie", "bob", "eve",
			};

			T_CHECK(score_order_is(&scores, expected, 6));
		}
        {
                struct score *first = list_entry(scores.next,
                                                  struct score, list);
                T_CHECK(first->points == 95);
        }

        /* sort by name ascending */
        score_sort_unlocked(&scores, _sort_by_name_asc, NULL);
        printf("\nsorted by name (asc):\n");
		idx = 0;
		score_foreach_unlocked(&scores, _dump_score_cb, &idx);
		{
			static const char *const expected[] = {
				"alice", "bob", "charlie", "diana", "eve", "frank",
			};

			T_CHECK(score_order_is(&scores, expected, 6));
		}
        {
                struct score *first = list_entry(scores.next,
                                                  struct score, list);
                T_CHECK(strcmp(first->name, "alice") == 0);
        }

        /* generated key sort: custom key callbacks may return NULL */
        {
                struct value_list_ops nullable_ops = score_list_ops;
                struct score *null_key = score_lookup_unlocked(
                        &scores, "alice");
                char *saved_name = null_key->name;

                nullable_ops.key = score_nullable_key;
                null_key->name = NULL;
                value_list_sort_key_unlocked(&scores, &nullable_ops);
                T_CHECK(score_nth_unlocked(&scores, 0) == null_key);
                null_key->name = saved_name;
        }

        /* remove_if: failing scores (< 60) */
        {
                int threshold = 60;
                size_t removed = score_remove_if_unlocked(
                        &scores, _is_failing, &threshold);
                printf("\nremoved %zu failing scores (< 60):\n", removed);
                T_CHECK(removed == 2);
                idx = 0;
                score_foreach_unlocked(&scores, _dump_score_cb, &idx);
                T_CHECK(score_count(&scores) == 4);
        }

        /* verify removed entries are gone */
        {
                T_CHECK(score_lookup_unlocked(&scores, "bob") == NULL);
                T_CHECK(score_lookup_unlocked(&scores, "eve") == NULL);
                T_CHECK(score_lookup_unlocked(&scores, "alice") != NULL);
                printf("\nlookup: bob=NULL eve=NULL alice=found: OK\n");
        }

        /* remove_if on empty result set */
        {
                int threshold = 0;
                size_t removed = score_remove_if_unlocked(
                        &scores, _is_failing, &threshold);
                T_CHECK(removed == 0);
                printf("remove_if with no matches: removed=%zu: OK\n",
                       removed);
        }

        score_free_list_unlocked(&scores);

	printf("\nt_value_list_mutation: PASS\n");
        return 0;
}
