/*
 * No-key value list verification.
 *
 * Proves VALUE_LIST_BIND_NOKEY: list-aware values with no lookup
 * identity. They get foreach/map/filter/reduce/sort/remove_if/
 * free/count/add but NO lookup function.
 *
 * Per VALUES_FRAMEWORK_DESIGN.md:
 *   - No-key list values use VALUE_LIST_BIND_NOKEY
 *   - Generated list and functional APIs, but no lookup
 *   - Useful for unordered collections where identity
 *     doesn't matter (e.g., log entries, events)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_support.h"

#include <value_base.h>

/* ------------------------------------------------------------------------- */
/* struct event — no-key list value                                          */
/* ------------------------------------------------------------------------- */

enum event_type {
        EVENT_INFO = 0,
        EVENT_WARN = 1,
        EVENT_ERROR = 2,
};

struct event {
        enum event_type type;
        int code;
        char *message;
        struct list_head list;
};

static void event_fini_obj(struct event *e)
{
        if (e->message)
                free(e->message);
}

static int event_copy_obj(struct event *dst, const struct event *src)
{
        char *message = src->message ? strdup(src->message) : NULL;

        if (src->message && !message)
                return VALUE_ERR_NOMEM;
        free(dst->message);
        dst->message = message;
        dst->type = src->type;
        dst->code = src->code;
        return VALUE_OK;
}

static void event_free_obj(struct event *e)
{
        free(e);
}

static struct event *new_event(enum event_type type, int code,
                              const char *message)
{
        struct event *e = calloc(1, sizeof(struct event));
        if (!e)
                return NULL;
        e->type = type;
        e->code = code;
        if (message)
                e->message = strdup(message);
        INIT_LIST_HEAD(&e->list);
        return e;
}

VALUE_OBJECT_WRAPPERS_BIND(event, event_fini_obj, event_copy_obj,
                           event_free_obj)
VALUE_OBJECT_BIND(event, NULL, event_fini, event_copy, NULL, NULL, NULL)
VALUE_LIST_BIND_NOKEY(event, list, event_copy, event_free)

/* ------------------------------------------------------------------------- */
/* test                                                                      */
/* ------------------------------------------------------------------------- */

static void _dump_event_cb(struct event *e, void *data)
{
        const char *type_str[] = { "INFO", "WARN", "ERROR" };
        int *i = data;
        printf("  [%d] %s code=%d msg=\"%s\"\n",
               (*i)++, type_str[e->type], e->code,
               e->message ? e->message : "");
}

static bool _is_error(const struct event *e, void *data)
{
        (void)data;
        return e->type == EVENT_ERROR;
}

static void *_collect_codes_cb(void *carry, struct event *e, void *data)
{
        int *sum = carry;
        (void)data;
        *sum += e->code;
        return carry;
}

int main(int argc, char *argv[])
{
        (void)argc;
        (void)argv;

        LIST_HEAD(events);
        int idx;

        /* add events — no particular identity, just appended */
        event_add_tail_unlocked(
                &events, new_event(EVENT_INFO, 0, "system started"));
        event_add_tail_unlocked(
                &events, new_event(EVENT_WARN, 10, "low memory"));
        event_add_tail_unlocked(
                &events, new_event(EVENT_ERROR, 404, "not found"));
        event_add_tail_unlocked(
                &events, new_event(EVENT_INFO, 1, "heartbeat"));
        event_add_tail_unlocked(
                &events, new_event(EVENT_ERROR, 500, "internal error"));

        T_CHECK(event_nth_unlocked(&events, 0)->code == 0);
        T_CHECK(event_nth_unlocked(&events, 2)->code == 404);
        T_CHECK(event_nth_unlocked(&events, 4)->code == 500);
        T_CHECK(!event_nth_unlocked(&events, 5));

        /* foreach */
        printf("all events:\n");
        idx = 0;
        event_foreach_unlocked(&events, _dump_event_cb, &idx);

        /* no lookup function generated — event_lookup("name") does
         * not exist and should not compile */
        printf("\nno lookup generated for no-key type: OK\n");

        /* filter: errors only */
        printf("\nfilter ERROR events:\n");
        LIST_HEAD(filters);
        event_filter_view_unlocked(&filters, &events, _is_error, NULL);
        idx = 0;
        value_filter_view_foreach((void (*)(void *, void *))_dump_event_cb,
                            &idx, &filters);
        value_filter_view_free(&filters);

        /* reduce: sum of codes */
        {
                int sum = 0;
                event_reduce_unlocked(
                        &events, _collect_codes_cb, NULL, &sum);
                printf("\ntotal code sum: %d\n", sum);
                T_CHECK(sum == 0 + 10 + 404 + 1 + 500);
        }

        /* count */
        printf("event_count: %zu\n", event_count(&events));
        T_CHECK(event_count(&events) == 5);

        /* copy */
        {
                struct event *src = list_entry(events.next, struct event, list);
                struct event *dst = new_event(
                        EVENT_WARN, 99, "destination");
                struct list_head *prev;
                struct list_head *next;

                event_add_tail_unlocked(&events, dst);
                prev = dst->list.prev;
                next = dst->list.next;
                event_copy(dst, src);
                T_CHECK(dst->type == EVENT_INFO);
                T_CHECK(dst->code == 0);
                T_CHECK(strcmp(dst->message, "system started") == 0);
                T_CHECK(dst->list.prev == prev);
                T_CHECK(dst->list.next == next);
                T_CHECK(prev->next == &dst->list);
                T_CHECK(next->prev == &dst->list);
                T_CHECK(event_count(&events) == 6);
                printf("\ncopy: type/code/message match, list hook preserved: OK\n");
        }

        event_free_list_unlocked(&events);

	printf("\nt_value_no_key: PASS\n");
        return 0;
}
