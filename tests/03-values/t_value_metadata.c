/*
 * Value metadata verification.
 *
 * Proves VALUE_META_BIND: metadata state tracking with
 * present/required/dirty/invalid fields.
 *
 * Per VALUES_FRAMEWORK_DESIGN.md section 8:
 *   - TYPE_meta_has, TYPE_meta_complete, TYPE_meta_ready
 *   - TYPE_meta_has_any, TYPE_meta_has_all
 *   - TYPE_meta_dirty_any, TYPE_meta_invalid_any
 *   - TYPE_meta_mark_present, TYPE_meta_mark_dirty
 *   - TYPE_meta_mark_present_dirty
 *   - TYPE_meta_mark_invalid, TYPE_meta_clear_invalid
 *   - TYPE_meta_accept
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_support.h"

#include <value_base.h>

/* ------------------------------------------------------------------------- */
/* struct sensor — string-keyed with metadata                                */
/* ------------------------------------------------------------------------- */

enum sensor_field {
        SENSOR_F_NAME    = (1ULL << 0),
        SENSOR_F_UNIT    = (1ULL << 1),
        SENSOR_F_VALUE   = (1ULL << 2),
        SENSOR_F_MIN     = (1ULL << 3),
        SENSOR_F_MAX     = (1ULL << 4),
        SENSOR_F_ALL     = SENSOR_F_NAME | SENSOR_F_UNIT |
                           SENSOR_F_VALUE | SENSOR_F_MIN | SENSOR_F_MAX,
        SENSOR_F_REQUIRED = SENSOR_F_NAME | SENSOR_F_UNIT | SENSOR_F_VALUE,
};

struct sensor {
        char *name;
        char *unit;
        double value;
        double min;
        double max;

        struct value_config meta;
        struct list_head list;
};

static void sensor_fini_obj(struct sensor *s)
{
        if (s->name) free(s->name);
        if (s->unit) free(s->unit);
}

static int sensor_copy_obj(struct sensor *dst, const struct sensor *src)
{
        char *name = src->name ? strdup(src->name) : NULL;
        char *unit = src->unit ? strdup(src->unit) : NULL;

        if ((src->name && !name) || (src->unit && !unit)) {
                free(unit);
                free(name);
                return VALUE_ERR_NOMEM;
        }
        free(dst->name);
        free(dst->unit);
        dst->name = name;
        dst->unit = unit;
        dst->value = src->value;
        dst->min = src->min;
        dst->max = src->max;
        dst->meta = src->meta;
        return VALUE_OK;
}

static void sensor_free_obj(struct sensor *s)
{
        free(s);
}

static struct sensor *new_sensor(const char *name)
{
        struct sensor *s = calloc(1, sizeof(struct sensor));
        if (!s)
                return NULL;
        if (name)
                s->name = strdup(name);
        value_config_init(&s->meta, SENSOR_F_REQUIRED);
        INIT_LIST_HEAD(&s->list);
        return s;
}

VALUE_OBJECT_WRAPPERS_BIND(sensor, sensor_fini_obj, sensor_copy_obj,
                           sensor_free_obj)
VALUE_OBJECT_BIND(sensor, NULL, sensor_fini, sensor_copy, NULL, NULL, NULL)
VALUE_LIST_BIND(sensor, name, list, sensor_copy, sensor_free)
VALUE_META_BIND(sensor, meta, NULL)

int main(int argc, char *argv[])
{
        (void)argc;
        (void)argv;

        struct sensor *s;

        printf("=== metadata state tracking ===\n\n");

        /* create sensor with required fields defined */
        s = new_sensor("temperature");
        T_CHECK(s != NULL);

        /* initially: no fields present, not complete */
        T_CHECK(!sensor_meta_has(s, SENSOR_F_NAME));
        T_CHECK(!sensor_meta_complete(s));
        T_CHECK(!sensor_meta_ready(s));

        /* mark fields present one by one */
        sensor_meta_mark_present(s, SENSOR_F_NAME);
        T_CHECK(sensor_meta_has(s, SENSOR_F_NAME));
        T_CHECK(!sensor_meta_complete(s));

        sensor_meta_mark_present(s, SENSOR_F_UNIT);
        T_CHECK(sensor_meta_has_any(s, SENSOR_F_NAME | SENSOR_F_UNIT));
        T_CHECK(!sensor_meta_complete(s));

        sensor_meta_mark_present(s, SENSOR_F_VALUE);
        T_CHECK(sensor_meta_has_all(s, SENSOR_F_NAME | SENSOR_F_UNIT |
                                          SENSOR_F_VALUE));
        T_CHECK(sensor_meta_complete(s));
        T_CHECK(sensor_meta_ready(s));
        printf("1. required fields marked: complete=%d ready=%d\n",
               sensor_meta_complete(s), sensor_meta_ready(s));

        /* mark optional fields */
        sensor_meta_mark_present(s, SENSOR_F_MIN);
        sensor_meta_mark_present(s, SENSOR_F_MAX);
        T_CHECK(sensor_meta_has(s, SENSOR_F_MIN));
        T_CHECK(sensor_meta_has(s, SENSOR_F_MAX));
        printf("2. optional fields marked: has_min=%d has_max=%d\n",
               sensor_meta_has(s, SENSOR_F_MIN),
               sensor_meta_has(s, SENSOR_F_MAX));

        /* dirty tracking */
        T_CHECK(!sensor_meta_dirty_any(s, SENSOR_F_ALL));
        sensor_meta_mark_dirty(s, SENSOR_F_VALUE);
        T_CHECK(sensor_meta_dirty_any(s, SENSOR_F_VALUE));
        T_CHECK(!sensor_meta_dirty_any(s, SENSOR_F_NAME));
        printf("3. dirty tracking: value_dirty=%d name_dirty=%d\n",
               sensor_meta_dirty_any(s, SENSOR_F_VALUE),
               sensor_meta_dirty_any(s, SENSOR_F_NAME));

        /* mark_present_dirty: both present and dirty */
        sensor_meta_mark_present_dirty(s, SENSOR_F_MAX);
        T_CHECK(sensor_meta_has(s, SENSOR_F_MAX));
        T_CHECK(sensor_meta_dirty_any(s, SENSOR_F_MAX));
        printf("4. mark_present_dirty: has_max=%d max_dirty=%d\n",
               sensor_meta_has(s, SENSOR_F_MAX),
               sensor_meta_dirty_any(s, SENSOR_F_MAX));

        /* accept: clears all dirty flags */
        sensor_meta_accept(s);
        T_CHECK(!sensor_meta_dirty_any(s, SENSOR_F_ALL));
        printf("5. after accept: any_dirty=%d\n",
               sensor_meta_dirty_any(s, SENSOR_F_ALL));

        /* invalid tracking */
        T_CHECK(!sensor_meta_invalid_any(s, SENSOR_F_ALL));
        sensor_meta_mark_invalid(s, SENSOR_F_VALUE);
        T_CHECK(sensor_meta_invalid_any(s, SENSOR_F_VALUE));
        T_CHECK(!sensor_meta_ready(s));
        printf("6. invalid: value_invalid=%d ready=%d\n",
               sensor_meta_invalid_any(s, SENSOR_F_VALUE),
               sensor_meta_ready(s));

        /* clear invalid */
        sensor_meta_clear_invalid(s, SENSOR_F_VALUE);
        T_CHECK(!sensor_meta_invalid_any(s, SENSOR_F_VALUE));
        T_CHECK(sensor_meta_ready(s));
        printf("7. after clear_invalid: value_invalid=%d ready=%d\n",
               sensor_meta_invalid_any(s, SENSOR_F_VALUE),
               sensor_meta_ready(s));

        /* NULL safety */
        T_CHECK(!sensor_meta_has(NULL, SENSOR_F_NAME));
        T_CHECK(!sensor_meta_complete(NULL));
        T_CHECK(!sensor_meta_ready(NULL));
        printf("8. NULL safety: all return false\n");

        /* copy preserves metadata */
        {
                struct sensor *dst = new_sensor("copy");
                LIST_HEAD(sensors);
                struct list_head *prev;
                struct list_head *next;

                sensor_add_tail_unlocked(&sensors, s);
                sensor_add_tail_unlocked(&sensors, dst);
                prev = dst->list.prev;
                next = dst->list.next;
                sensor_meta_mark_dirty(dst, SENSOR_F_NAME);
                sensor_copy(dst, s);
                T_CHECK(sensor_meta_has(dst, SENSOR_F_VALUE));
                T_CHECK(!sensor_meta_dirty_any(dst, SENSOR_F_ALL));
                T_CHECK(dst->list.prev == prev);
                T_CHECK(dst->list.next == next);
                T_CHECK(prev->next == &dst->list);
                T_CHECK(next->prev == &dst->list);
                printf("9. copy preserves metadata: has_value=%d dirty=%d\n",
                       sensor_meta_has(dst, SENSOR_F_VALUE),
                       sensor_meta_dirty_any(dst, SENSOR_F_ALL));
                sensor_free_list_unlocked(&sensors);
                s = NULL;
        }

        sensor_free(s);

	printf("\nt_value_metadata: PASS\n");
        return 0;
}
