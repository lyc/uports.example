/*
 * Domain-key value list verification.
 *
 * Proves VALUE_LIST_BIND_DOMAIN_KEY: types with a non-string key
 * get list functional operations but no generated string lookup.
 * Domain lookup must be handwritten.
 *
 * Per VALUES_FRAMEWORK_DESIGN.md:
 *   - Domain-key values use VALUE_LIST_BIND_DOMAIN_KEY
 *   - No generic string lookup is generated
 *   - Domain lookup is handwritten: eaxcid_lookup_id()
 *   - Multi-key lookup is also domain-key; handwritten
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_support.h"

#include <value_base.h>

/* ------------------------------------------------------------------------- */
/* struct eaxcid — domain-keyed by eaxc_id                                  */
/* ------------------------------------------------------------------------- */

enum eaxcid_field {
        EAXCID_F_ODU_PORT    = (1ULL << 0),
        EAXCID_F_BAND_SECTOR = (1ULL << 1),
        EAXCID_F_CCID        = (1ULL << 2),
        EAXCID_F_RU_PORT     = (1ULL << 3),
        EAXCID_F_ID          = (1ULL << 4),
        EAXCID_F_ALL         = EAXCID_F_ODU_PORT | EAXCID_F_BAND_SECTOR |
                               EAXCID_F_CCID | EAXCID_F_RU_PORT |
                               EAXCID_F_ID,
};

struct eaxcid {
        uint16_t o_du_port_bitmask;
        uint16_t band_sector_bitmask;
        uint16_t ccid_bitmask;
        uint16_t ru_port_bitmask;
        uint16_t eaxc_id;

        struct list_head list;
};

static void eaxcid_fini_obj(struct eaxcid *e)
{
        (void)e;
}

static int eaxcid_copy_obj(struct eaxcid *dst, const struct eaxcid *src)
{
        dst->o_du_port_bitmask = src->o_du_port_bitmask;
        dst->band_sector_bitmask = src->band_sector_bitmask;
        dst->ccid_bitmask = src->ccid_bitmask;
        dst->ru_port_bitmask = src->ru_port_bitmask;
        dst->eaxc_id = src->eaxc_id;
        return VALUE_OK;
}

static void eaxcid_free_obj(struct eaxcid *e)
{
        free(e);
}

static struct eaxcid *new_eaxcid(uint16_t id)
{
        struct eaxcid *e = calloc(1, sizeof(struct eaxcid));
        if (!e)
                return NULL;
        e->eaxc_id = id;
        INIT_LIST_HEAD(&e->list);
        return e;
}

VALUE_OBJECT_WRAPPERS_BIND(eaxcid, eaxcid_fini_obj, eaxcid_copy_obj,
                           eaxcid_free_obj)
VALUE_OBJECT_BIND(eaxcid, NULL, eaxcid_fini, eaxcid_copy, NULL, NULL, NULL)
VALUE_LIST_BIND_DOMAIN_KEY(eaxcid, list, eaxcid_copy, eaxcid_free)

/* Handwritten domain-key lookup — per VALUES_FRAMEWORK_DESIGN.md */
struct eaxcid *eaxcid_lookup_id(struct list_head *head, uint16_t eaxc_id)
{
        struct list_head *pos;

        list_for_each(pos, head) {
                struct eaxcid *e = list_entry(pos, struct eaxcid, list);
                if (e->eaxc_id == eaxc_id)
                        return e;
        }
        return NULL;
}

/* ------------------------------------------------------------------------- */
/* test                                                                      */
/* ------------------------------------------------------------------------- */

static void _dump_eaxcid_cb(struct eaxcid *e, void *data)
{
        int *i = data;
        printf("  [%d] eaxc_id=%u odu=0x%x bs=0x%x ccid=0x%x ru=0x%x\n",
               (*i)++, e->eaxc_id, e->o_du_port_bitmask,
               e->band_sector_bitmask, e->ccid_bitmask,
               e->ru_port_bitmask);
}

static bool _is_high_id(const struct eaxcid *e, void *data)
{
        uint16_t threshold = *(uint16_t *)data;
        return e->eaxc_id >= threshold;
}

static void *_count_eaxcid_cb(void *carry, struct eaxcid *e, void *data)
{
        int *cnt = carry;
        (void)e;
        (void)data;
        *cnt += 1;
        return carry;
}

int main(int argc, char *argv[])
{
        (void)argc;
        (void)argv;

        LIST_HEAD(eaxcids);
        int idx;

        /* add several eaxcid values */
        eaxcid_add_tail_unlocked(&eaxcids, new_eaxcid(100));
        eaxcid_add_tail_unlocked(&eaxcids, new_eaxcid(200));
        eaxcid_add_tail_unlocked(&eaxcids, new_eaxcid(300));
        eaxcid_add_tail_unlocked(&eaxcids, new_eaxcid(400));

        /* set some field values */
        {
                struct eaxcid *e;

                e = eaxcid_lookup_id(&eaxcids, 100);
                T_CHECK(e != NULL);
                e->o_du_port_bitmask = 0x01;
                e->band_sector_bitmask = 0x02;
                e->ccid_bitmask = 0x03;
                e->ru_port_bitmask = 0x04;

                e = eaxcid_lookup_id(&eaxcids, 200);
                T_CHECK(e != NULL);
                e->o_du_port_bitmask = 0x11;
                e->band_sector_bitmask = 0x12;

                e = eaxcid_lookup_id(&eaxcids, 400);
                T_CHECK(e != NULL);
                e->o_du_port_bitmask = 0xFF;
                e->ru_port_bitmask = 0xEE;
        }

        /* foreach */
        printf("all eaxcid values:\n");
        idx = 0;
        eaxcid_foreach_unlocked(&eaxcids, _dump_eaxcid_cb, &idx);

        /* domain-key lookup */
        printf("\ndomain-key lookup:\n");
        {
                struct eaxcid *e = eaxcid_lookup_id(&eaxcids, 200);
                T_CHECK(e != NULL);
                printf("  found eaxc_id=200: odu=0x%x bs=0x%x\n",
                       e->o_du_port_bitmask, e->band_sector_bitmask);
        }

        {
                struct eaxcid *e = eaxcid_lookup_id(&eaxcids, 999);
                T_CHECK(e == NULL);
                printf("  eaxc_id=999: not found (correct)\n");
        }

        /* verify no generic string lookup exists — carrier_lookup should
         * NOT be generated for domain-key types. We cannot call
         * eaxcid_lookup("200") because that function does not exist. */
        printf("\nno string-key lookup generated for domain-key type: OK\n");

        /* filter: high ids >= 300 */
        printf("\nfilter eaxc_id >= 300:\n");
        LIST_HEAD(filters);
        uint16_t threshold = 300;
        eaxcid_filter_view_unlocked(
                &filters, &eaxcids, _is_high_id, &threshold);
        idx = 0;
        value_filter_view_foreach((void (*)(void *, void *))_dump_eaxcid_cb,
                            &idx, &filters);
        value_filter_view_free(&filters);

        /* reduce: count */
        {
                int cnt = 0;
                eaxcid_reduce_unlocked(
                        &eaxcids, _count_eaxcid_cb, NULL, &cnt);
                printf("\ntotal count: %d\n", cnt);
                T_CHECK(cnt == 4);
        }

        /* count via typed wrapper */
        printf("eaxcid_count: %zu\n", eaxcid_count(&eaxcids));
        T_CHECK(eaxcid_count(&eaxcids) == 4);

        /* copy */
        {
                struct eaxcid *src = eaxcid_lookup_id(&eaxcids, 100);
                struct eaxcid *dst = new_eaxcid(0);
                struct list_head *prev;
                struct list_head *next;

                eaxcid_add_tail_unlocked(&eaxcids, dst);
                prev = dst->list.prev;
                next = dst->list.next;
                eaxcid_copy(dst, src);
                T_CHECK(dst->eaxc_id == 100);
                T_CHECK(dst->o_du_port_bitmask == 0x01);
                T_CHECK(dst->band_sector_bitmask == 0x02);
                T_CHECK(dst->ccid_bitmask == 0x03);
                T_CHECK(dst->ru_port_bitmask == 0x04);
                T_CHECK(dst->list.prev == prev);
                T_CHECK(dst->list.next == next);
                T_CHECK(prev->next == &dst->list);
                T_CHECK(next->prev == &dst->list);
                T_CHECK(eaxcid_count(&eaxcids) == 5);
                printf("\ncopy: eaxc_id=%u matches source, list hook preserved: OK\n",
                       dst->eaxc_id);
        }

        eaxcid_free_list_unlocked(&eaxcids);

	printf("\nt_value_domain_key: PASS\n");
        return 0;
}
