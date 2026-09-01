#include <stdint.h>
#include <stdio.h>

#include <value_base.h>

#include "t_support.h"

enum pair_fields {
	PAIR_COORD = 1ULL << 0,
	PAIR_LEVEL = 1ULL << 1,
};

struct pair {
	int x;
	int y;
	int level;
	struct value_config meta;
};

VALUE_META_BIND(pair, meta, NULL);

static void pair_init(struct pair *pair)
{
	pair->x = 0;
	pair->y = 0;
	pair->level = 0;
	value_config_init(&pair->meta, PAIR_COORD);
}

static void pair_set_coord(struct pair *pair, int x, int y)
{
	VALUE_FIELD_SET_CHANGED2(pair, pair, x, y, PAIR_COORD, x, y);
}

static void pair_set_level(struct pair *pair, int level)
{
	VALUE_FIELD_SET_CHANGED(pair, pair, level, PAIR_LEVEL, level);
}

static int pair_compare_coord(const struct pair *lhs, const struct pair *rhs)
{
	VALUE_FIELD_COMPARE2(pair, lhs, rhs, x, y, PAIR_COORD);
	return 0;
}

static int pair_compare_level(const struct pair *lhs, const struct pair *rhs)
{
	VALUE_FIELD_COMPARE(pair, lhs, rhs, level, PAIR_LEVEL);
	return 0;
}

static int test_changed_macros(void)
{
	struct pair pair;

	pair_init(&pair);
	T_CHECK(!pair_meta_has(&pair, PAIR_COORD));
	pair_set_coord(&pair, 3, 4);
	T_CHECK(pair.x == 3 && pair.y == 4);
	T_CHECK(pair_meta_has(&pair, PAIR_COORD));
	T_CHECK(pair_meta_dirty_any(&pair, PAIR_COORD));

	pair_meta_accept(&pair);
	pair_set_coord(&pair, 3, 4);
	T_CHECK(!pair_meta_dirty_any(&pair, PAIR_COORD));
	pair_set_coord(&pair, 3, 5);
	T_CHECK(pair_meta_dirty_any(&pair, PAIR_COORD));

	pair_set_level(&pair, 7);
	T_CHECK(pair.level == 7 && pair_meta_has(&pair, PAIR_LEVEL));
	return 0;
}

static int test_compare_macros(void)
{
	struct pair a;
	struct pair b;
	struct pair c;

	pair_init(&a);
	pair_init(&b);
	pair_init(&c);
	pair_set_coord(&a, 1, 9);
	pair_set_coord(&b, 2, 0);
	pair_set_coord(&c, 2, 1);

	T_CHECK(pair_compare_coord(&a, &b) < 0);
	T_CHECK(pair_compare_coord(&b, &a) > 0);
	T_CHECK(pair_compare_coord(&b, &c) < 0);
	T_CHECK(pair_compare_coord(&a, &c) < 0);

	pair_set_level(&a, 10);
	pair_set_level(&b, 11);
	T_CHECK(pair_compare_level(&a, &b) < 0);
	T_CHECK(pair_compare_level(&b, &a) > 0);
	return 0;
}

static int test_metadata_transitions(void)
{
	struct value_config meta;
	uint64_t required = PAIR_COORD | PAIR_LEVEL;

	value_config_init(&meta, required);
	T_CHECK(value_config_missing_required(&meta) == required);
	T_CHECK(!value_config_complete(&meta));
	value_config_mark_present_dirty(&meta, PAIR_COORD);
	T_CHECK(value_config_has_any(&meta, required));
	T_CHECK(!value_config_has_all(&meta, required));
	T_CHECK(value_config_dirty_fields(&meta) == PAIR_COORD);
	T_CHECK(value_config_is_dirty(&meta));
	value_config_mark_present(&meta, PAIR_LEVEL);
	T_CHECK(value_config_complete(&meta) && value_config_ready(&meta));

	value_config_mark_invalid(&meta, PAIR_COORD, "bad coordinate");
	T_CHECK(value_config_invalid_any(&meta, PAIR_COORD));
	T_CHECK(!value_config_ready(&meta));
	T_CHECK(meta.validated == PAIR_COORD);
	T_CHECK(meta.validation_count == 1);
	value_config_mark_valid(&meta, PAIR_COORD);
	T_CHECK(!value_config_invalid_any(&meta, PAIR_COORD));
	T_CHECK(value_config_ready(&meta));

	value_config_mark_invalid_fields(&meta, required);
	T_CHECK(value_config_invalid_any(&meta, required));
	value_config_clear_invalid(&meta, required);
	T_CHECK(!value_config_invalid_any(&meta, required));
	value_config_accept(&meta);
	T_CHECK(!value_config_is_dirty(&meta));
	value_config_clear_field(&meta, PAIR_LEVEL);
	T_CHECK(!value_config_has(&meta, PAIR_LEVEL));
	T_CHECK(value_config_missing_required(&meta) == PAIR_LEVEL);
	T_CHECK((meta.validated & PAIR_LEVEL) == 0);
	return 0;
}

int main(void)
{
	T_RUN(test_changed_macros);
	T_RUN(test_compare_macros);
	T_RUN(test_metadata_transitions);
	puts("t_value_macros: PASS");
	return 0;
}
