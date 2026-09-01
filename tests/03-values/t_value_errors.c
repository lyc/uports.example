/*
 * Invalid arguments, failure cleanup, and borrowed filter lifetime.
 */

#include "t_support.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <value_base.h>

struct sample {
        char *name;
        int value;
        struct list_head list;
};

static void sample_fini_obj(struct sample *sample)
{
        free(sample->name);
        sample->name = NULL;
}

static int sample_copy_obj(struct sample *dst, const struct sample *src)
{
        char *name;

        name = src->name ? strdup(src->name) : NULL;
        if (src->name && !name)
                return VALUE_ERR_NOMEM;

        free(dst->name);
        dst->name = name;
        dst->value = src->value;
        return VALUE_OK;
}

static void sample_free_obj(struct sample *sample)
{
        free(sample);
}

static struct sample *sample_new(const char *name, int value)
{
        struct sample *sample;

        sample = calloc(1, sizeof(*sample));
        if (!sample)
                return NULL;
        sample->name = name ? strdup(name) : NULL;
        if (name && !sample->name) {
                free(sample);
                return NULL;
        }
        sample->value = value;
        INIT_LIST_HEAD(&sample->list);
        return sample;
}

VALUE_OBJECT_WRAPPERS_BIND(sample, sample_fini_obj, sample_copy_obj,
                           sample_free_obj)
VALUE_OBJECT_BIND(sample, NULL, sample_fini, sample_copy, NULL, NULL, NULL)
VALUE_LIST_BIND(sample, name, list, sample_copy, sample_free)

struct memory_probe {
        size_t calls;
        size_t live;
        size_t fail_call;
};

static struct memory_probe *copy_probe;

static void *probe_calloc(size_t nmemb, size_t size, void *ctx)
{
        struct memory_probe *probe = ctx;
        void *ptr;

        probe->calls++;
        if (probe->fail_call && probe->calls == probe->fail_call)
                return NULL;

        ptr = calloc(nmemb, size);
        if (ptr)
                probe->live++;
        return ptr;
}

static void probe_dealloc(void *ptr, void *ctx)
{
        struct memory_probe *probe = ctx;

        if (ptr) {
                T_REQUIRE(probe->live > 0);
                probe->live--;
        }
        free(ptr);
}

struct fragile {
        char *first;
        char *second;
        struct list_head list;
};

static void fragile_fini_obj(struct fragile *fragile)
{
        free(fragile->first);
        free(fragile->second);
        fragile->first = NULL;
        fragile->second = NULL;
}

static int fragile_copy_obj(struct fragile *dst, const struct fragile *src)
{
        char *first = NULL;
        char *second = NULL;

        if (src->first) {
                first = probe_calloc(
                        strlen(src->first) + 1, 1, copy_probe);
                if (!first)
                        return VALUE_ERR_NOMEM;
                strcpy(first, src->first);
        }
        if (src->second) {
                second = probe_calloc(
                        strlen(src->second) + 1, 1, copy_probe);
                if (!second) {
                        probe_dealloc(first, copy_probe);
                        return VALUE_ERR_NOMEM;
                }
                strcpy(second, src->second);
        }

        fragile_fini_obj(dst);
        dst->first = first;
        dst->second = second;
        return VALUE_OK;
}

static void fragile_free_obj(struct fragile *fragile)
{
        free(fragile);
}

VALUE_OBJECT_WRAPPERS_BIND(fragile, fragile_fini_obj, fragile_copy_obj,
                           fragile_free_obj)

static bool sample_all(const struct sample *sample, void *ctx)
{
        (void)sample;
        (void)ctx;
        return true;
}

static void *sum_filter_values(void *acc, void *object, void *ctx)
{
        struct sample *sample = object;
        int *sum = acc;

        (void)ctx;
        *sum += sample->value;
        return sum;
}

static int sample_value_desc(void *left, void *right, void *ctx)
{
        const struct sample *a = left;
        const struct sample *b = right;

        (void)ctx;
        return b->value - a->value;
}

static const char *sample_null_key(const void *obj)
{
        (void)obj;
        return NULL;
}

static void count_sample(struct sample *sample, void *ctx)
{
        int *count = ctx;

        (void)sample;
        (*count)++;
}

static void *keep_acc(void *acc, struct sample *sample, void *ctx)
{
        (void)sample;
        (void)ctx;
        return acc;
}

struct map_probe {
        size_t calls;
        size_t fail_call;
        int failed;
};

static struct list_head *clone_until_failure(struct sample *sample, void *ctx)
{
        struct map_probe *probe = ctx;
        struct sample *clone;

        probe->calls++;
        if (probe->failed || probe->calls == probe->fail_call) {
                probe->failed = 1;
                return NULL;
        }

        clone = sample_new(NULL, 0);
        if (!clone || sample_copy(clone, sample) != VALUE_OK) {
                sample_free(clone);
                probe->failed = 1;
                return NULL;
        }
        return &clone->list;
}

static int test_invalid_arguments(void)
{
        LIST_HEAD(head);
        LIST_HEAD(out);
        struct sample *sample = sample_new("one", 1);
        struct value_memory_ops invalid_memory = { 0 };
        struct value_list_ops null_key_ops = sample_list_ops;
        int count = 0;
        int acc = 7;

        if (!sample)
                return 1;

        value_config_init(NULL, 0);
        if (value_memory_set(NULL) != VALUE_ERR_ARG)
                return 2;
        if (value_memory_set(&invalid_memory) != VALUE_ERR_ARG)
                return 3;
        if (value_hook(NULL, &sample_list_ops) ||
            value_hook(sample, NULL) ||
            value_hook_const(NULL, &sample_list_ops) ||
            value_entry(NULL, &sample_list_ops))
                return 4;
        if (value_list_lookup_unlocked(NULL, &sample_list_ops, "one") ||
            value_list_lookup_unlocked(&head, NULL, "one") ||
            value_list_lookup_unlocked(&head, &sample_list_ops, NULL))
                return 5;

        sample_add_tail_unlocked(&head, sample);
        null_key_ops.key = sample_null_key;
        if (value_list_lookup_unlocked(&head, &null_key_ops, "one"))
                return 6;
        list_del_init(&sample->list);

        sample_add_tail_unlocked(NULL, sample);
        sample_add_tail_unlocked(&head, NULL);
        if (!list_empty(&head))
                return 7;

        value_list_foreach_unlocked(NULL, &sample_list_ops,
                                    (value_each_fn)count_sample, &count);
        value_list_foreach_unlocked(&head, NULL,
                                    (value_each_fn)count_sample, &count);
        value_list_foreach_unlocked(&head, &sample_list_ops, NULL, &count);
        if (count)
                return 8;
        if (value_list_reduce_unlocked(
                    NULL, &sample_list_ops, (value_reduce_fn)keep_acc,
                    NULL, &acc) != &acc)
                return 9;
        if (value_list_map_hook_unlocked(
                    &out, NULL, &sample_list_ops,
                    (value_hook_map_fn)clone_until_failure, NULL) != &out)
                return 10;
        if (value_list_remove_if_unlocked(
                    NULL, &sample_list_ops, (value_pred_fn)sample_all,
                    NULL) != 0)
                return 11;
        if (value_list_count(NULL) != 0)
                return 12;
        if (value_list_filter_view_unlocked(
                    NULL, &head, &sample_list_ops,
                    (value_pred_fn)sample_all, NULL) != VALUE_ERR_ARG)
                return 13;
        if (sample_copy(NULL, sample) != VALUE_ERR_ARG ||
            sample_copy(sample, NULL) != VALUE_ERR_ARG)
                return 14;

        sample_fini(NULL);
        sample_free(NULL);
        value_list_sort_unlocked(NULL, &sample_list_ops, NULL, NULL);
        value_list_free_unlocked(NULL, &sample_list_ops);
        value_filter_view_free(NULL);
        value_filter_view_foreach(NULL, NULL, NULL);
        if (value_filter_view_nth(NULL, 0) != NULL)
                return 15;
        if (value_filter_view_clone(NULL, &head) != VALUE_ERR_ARG ||
            value_filter_view_clone(&head, NULL) != VALUE_ERR_ARG ||
            value_filter_view_clone(&head, &head) != VALUE_ERR_ARG)
                return 16;
        if (value_filter_view_reduce(
                    NULL, (value_reduce_fn)keep_acc, NULL, &acc) != &acc ||
            value_filter_view_reduce(&head, NULL, NULL, &acc) != &acc)
                return 17;
        if (value_filter_view_map(NULL, NULL, NULL, NULL) != NULL)
                return 18;

        sample_free(sample);
        return 0;
}

static int test_filter_failure_and_lifetime(void)
{
        LIST_HEAD(samples);
        LIST_HEAD(filters);
        LIST_HEAD(clone);
        struct memory_probe probe = {
                .fail_call = 2,
        };
        struct value_memory_ops memory = {
                .calloc_fn = probe_calloc,
                .dealloc_fn = probe_dealloc,
                .ctx = &probe,
        };
        struct sample *first;
        int sum = 0;

        sample_add_tail_unlocked(&samples, sample_new("one", 1));
        sample_add_tail_unlocked(&samples, sample_new("two", 2));
        sample_add_tail_unlocked(&samples, sample_new("three", 3));
        if (sample_count(&samples) != 3)
                return 20;

        if (value_memory_set(&memory) != VALUE_OK)
                return 21;
        if (sample_filter_view_unlocked(
                    &filters, &samples, sample_all, NULL) != VALUE_ERR_NOMEM)
                return 22;
        if (!list_empty(&filters) || probe.live != 0)
                return 23;
        if (sample_count(&samples) != 3)
                return 24;

        probe.calls = 0;
        probe.fail_call = 0;
        if (sample_filter_view_unlocked(
                    &filters, &samples, sample_all, NULL) != VALUE_OK)
                return 25;
        if (probe.live != 3)
                return 26;

        first = value_filter_view_first(&filters);
        if (!first || strcmp(first->name, "one"))
                return 27;
        if (value_filter_view_nth(&filters, 0) != first ||
            value_filter_view_nth(&filters, 1) !=
                    sample_nth_unlocked(&samples, 1) ||
            value_filter_view_nth(&filters, 2) !=
                    sample_nth_unlocked(&samples, 2) ||
            value_filter_view_nth(&filters, 3) != NULL)
                return 30;
        if (value_filter_view_reduce(
                    &filters, sum_filter_values, NULL, &sum) != &sum ||
            sum != 6)
                return 31;

        if (value_filter_view_clone(&clone, &filters) != VALUE_OK ||
            probe.live != 6 || value_filter_view_nth(&clone, 0) != first ||
            value_filter_view_nth(&clone, 2) !=
                    value_filter_view_nth(&filters, 2))
                return 32;
        value_filter_view_sort(&clone, sample_value_desc, NULL);
        if (((struct sample *)value_filter_view_nth(&clone, 0))->value != 3 ||
            ((struct sample *)value_filter_view_nth(&clone, 2))->value != 1 ||
            value_filter_view_first(&filters) != first)
                return 35;
        value_filter_view_free(&clone);
        if (probe.live != 3 || value_filter_view_first(&filters) != first)
                return 33;

        probe.calls = 0;
        probe.fail_call = 2;
        if (value_filter_view_clone(&clone, &filters) != VALUE_ERR_NOMEM ||
            !list_empty(&clone) || probe.live != 3)
                return 34;

        value_memory_reset();
        value_filter_view_free(&filters);
        if (probe.live != 0 || sample_count(&samples) != 3)
                return 28;
        if (strcmp(first->name, "one") || first->value != 1)
                return 29;

        sample_free_list_unlocked(&samples);
        return 0;
}

static int test_partial_map_cleanup(void)
{
        LIST_HEAD(samples);
        LIST_HEAD(clones);
        struct map_probe probe = {
                .fail_call = 2,
        };

        sample_add_tail_unlocked(&samples, sample_new("one", 1));
        sample_add_tail_unlocked(&samples, sample_new("two", 2));
        sample_add_tail_unlocked(&samples, sample_new("three", 3));

        sample_map_unlocked(
                &clones, clone_until_failure, &probe, &samples);
        if (!probe.failed || probe.calls != 3)
                return 30;
        if (sample_count(&clones) != 1 || sample_count(&samples) != 3)
                return 31;
        if (sample_lookup_unlocked(&clones, "one") ==
            sample_lookup_unlocked(&samples, "one"))
                return 32;

        sample_free_list_unlocked(&clones);
        sample_free_list_unlocked(&samples);
        return 0;
}

static int test_partial_copy_cleanup(void)
{
        struct memory_probe probe = {
                .fail_call = 2,
        };
        struct fragile src = {
                .first = "first",
                .second = "second",
        };
        struct fragile dst = { 0 };

        INIT_LIST_HEAD(&src.list);
        INIT_LIST_HEAD(&dst.list);
        copy_probe = &probe;

        if (fragile_copy(&dst, &src) != VALUE_ERR_NOMEM)
                return 40;
        if (probe.live != 0 || dst.first || dst.second)
                return 41;
        if (!list_empty(&dst.list) || strcmp(src.first, "first") ||
            strcmp(src.second, "second"))
                return 42;

        copy_probe = NULL;
        return 0;
}

int main(void)
{
        int rc;

        rc = test_invalid_arguments();
        if (rc)
                return rc;
        rc = test_filter_failure_and_lifetime();
        if (rc)
                return rc;
        rc = test_partial_map_cleanup();
        if (rc)
                return rc;
        rc = test_partial_copy_cleanup();
        if (rc)
                return rc;

	printf("t_value_errors: PASS\n");
        return 0;
}
