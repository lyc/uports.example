#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <value_base.h>

#include "t_support.h"

struct filter_item {
	int id;
	struct list_head source_hook;
	struct list_head map_hook;
};

struct allocation_probe {
	size_t calls;
	size_t live;
	size_t fail_call;
};

static void *probe_calloc(size_t nmemb, size_t size, void *context)
{
	struct allocation_probe *probe = context;
	void *memory;

	probe->calls++;
	if (probe->fail_call && probe->calls == probe->fail_call)
		return NULL;
	memory = calloc(nmemb, size);
	if (memory)
		probe->live++;
	return memory;
}

static void probe_free(void *memory, void *context)
{
	struct allocation_probe *probe = context;

	if (memory) {
		if (!probe->live) {
			fprintf(stderr, "allocator provenance mismatch\n");
			abort();
		}
		probe->live--;
	}
	free(memory);
}

static bool item_is_even(const void *object, void *context)
{
	const struct filter_item *item = object;

	(void)context;
	return (item->id % 2) == 0;
}

static void count_item(void *object, void *context)
{
	struct filter_item *item = object;
	int *sum = context;

	*sum += item->id;
}

static void *sum_item(void *accumulator, void *object, void *context)
{
	struct filter_item *item = object;
	int *sum = accumulator;

	(void)context;
	*sum += item->id;
	return sum;
}

static int item_desc(void *left, void *right, void *context)
{
	const struct filter_item *a = left;
	const struct filter_item *b = right;

	(void)context;
	return (a->id < b->id) - (a->id > b->id);
}

static struct list_head *item_map_hook(void *object, void *context)
{
	struct filter_item *item = object;

	(void)context;
	return &item->map_hook;
}

static const struct value_list_ops item_list_ops = {
	.hook_offset = offsetof(struct filter_item, source_hook),
	.size = sizeof(struct filter_item),
};

static void init_items(struct filter_item *items, size_t count,
	struct list_head *source)
{
	size_t index;

	INIT_LIST_HEAD(source);
	for (index = 0; index < count; index++) {
		items[index].id = (int)index + 1;
		INIT_LIST_HEAD(&items[index].source_hook);
		INIT_LIST_HEAD(&items[index].map_hook);
		list_add_tail(&items[index].source_hook, source);
	}
}

static int test_view_operations(void)
{
	struct filter_item items[5];
	struct allocation_probe first_probe = { 0 };
	struct allocation_probe second_probe = { 0 };
	struct value_memory_ops first_memory = {
		.calloc_fn = probe_calloc,
		.dealloc_fn = probe_free,
		.ctx = &first_probe,
	};
	struct value_memory_ops second_memory = {
		.calloc_fn = probe_calloc,
		.dealloc_fn = probe_free,
		.ctx = &second_probe,
	};
	LIST_HEAD(source);
	LIST_HEAD(view);
	LIST_HEAD(clone);
	LIST_HEAD(mapped);
	int sum = 0;

	init_items(items, 5, &source);
	T_CHECK(value_memory_set(&first_memory) == VALUE_OK);
	T_CHECK(value_list_filter_view_unlocked(&view, &source,
		&item_list_ops, item_is_even, NULL) == VALUE_OK);
	T_CHECK(first_probe.live == 2);
	T_CHECK(value_filter_view_first(&view) == &items[1]);
	T_CHECK(value_filter_view_nth(&view, 1) == &items[3]);
	T_CHECK(value_filter_view_nth(&view, 2) == NULL);

	value_filter_view_foreach(count_item, &sum, &view);
	T_CHECK(sum == 6);
	sum = 0;
	T_CHECK(value_filter_view_reduce(&view, sum_item, NULL, &sum) == &sum);
	T_CHECK(sum == 6);

	T_CHECK(value_memory_set(&second_memory) == VALUE_OK);
	T_CHECK(value_filter_view_clone(&clone, &view) == VALUE_OK);
	T_CHECK(first_probe.live == 2 && second_probe.live == 2);
	value_filter_view_sort(&clone, item_desc, NULL);
	T_CHECK(value_filter_view_nth(&clone, 0) == &items[3]);
	T_CHECK(value_filter_view_nth(&clone, 1) == &items[1]);
	T_CHECK(value_filter_view_first(&view) == &items[1]);

	T_CHECK(value_filter_view_map(&mapped, &view,
		item_map_hook, NULL) == &mapped);
	T_CHECK(list_count(&mapped) == 2);
	T_CHECK(list_entry(mapped.next, struct filter_item, map_hook) == &items[1]);
	while (!list_empty(&mapped))
		list_del_init(mapped.next);

	value_filter_view_free(&view);
	T_CHECK(first_probe.live == 0 && second_probe.live == 2);
	T_CHECK(items[1].id == 2 && items[3].id == 4);
	T_CHECK(value_list_count(&source) == 5);
	value_filter_view_free(&clone);
	T_CHECK(second_probe.live == 0);
	value_memory_reset();
	return 0;
}

static int test_allocation_rollback(void)
{
	struct filter_item items[4];
	struct allocation_probe probe = { .fail_call = 2 };
	struct value_memory_ops memory = {
		.calloc_fn = probe_calloc,
		.dealloc_fn = probe_free,
		.ctx = &probe,
	};
	LIST_HEAD(source);
	LIST_HEAD(view);

	init_items(items, 4, &source);
	T_CHECK(value_memory_set(&memory) == VALUE_OK);
	T_CHECK(value_list_filter_view_unlocked(&view, &source,
		&item_list_ops, item_is_even, NULL) == VALUE_ERR_NOMEM);
	T_CHECK(list_empty(&view));
	T_CHECK(probe.live == 0);
	T_CHECK(value_list_count(&source) == 4);
	value_memory_reset();
	return 0;
}

static int test_clone_rollback(void)
{
	struct filter_item items[4];
	struct allocation_probe source_probe = { 0 };
	struct allocation_probe clone_probe = { .fail_call = 2 };
	struct value_memory_ops source_memory = {
		.calloc_fn = probe_calloc,
		.dealloc_fn = probe_free,
		.ctx = &source_probe,
	};
	struct value_memory_ops clone_memory = {
		.calloc_fn = probe_calloc,
		.dealloc_fn = probe_free,
		.ctx = &clone_probe,
	};
	LIST_HEAD(source);
	LIST_HEAD(view);
	LIST_HEAD(clone);

	init_items(items, 4, &source);
	T_CHECK(value_memory_set(&source_memory) == VALUE_OK);
	T_CHECK(value_list_filter_view_unlocked(&view, &source,
		&item_list_ops, item_is_even, NULL) == VALUE_OK);
	T_CHECK(source_probe.live == 2);
	T_CHECK(value_memory_set(&clone_memory) == VALUE_OK);
	T_CHECK(value_filter_view_clone(&clone, &view) == VALUE_ERR_NOMEM);
	T_CHECK(list_empty(&clone) && clone_probe.live == 0);
	value_memory_reset();
	value_filter_view_free(&view);
	T_CHECK(source_probe.live == 0);
	return 0;
}

int main(void)
{
	T_RUN(test_view_operations);
	T_RUN(test_allocation_rollback);
	T_RUN(test_clone_rollback);
	puts("t_value_filter_ref: PASS");
	return 0;
}
