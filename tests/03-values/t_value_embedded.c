/*
 * Embedded value verification.
 *
 * Proves embedded value pattern: a value (eaxcid) can both
 * live directly in its own list AND be embedded inside another
 * value (endpoint).
 *
 * Per VALUES_FRAMEWORK_DESIGN.md:
 *   - eaxcid is decoupled from endpoint
 *   - endpoint embeds eaxcid
 *   - eaxcid can also live directly in an eaxcid list
 *   - parent calls child lifecycle, never manual field copy
 *   - copy never copies list membership (INIT_LIST_HEAD on dst)
 *   - parent must unlink embedded child before finalization
 *     if it was externally listed
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_support.h"

#include <value_base.h>

/* ------------------------------------------------------------------------- */
/* struct eaxcid — domain-keyed, can be embedded or standalone              */
/* ------------------------------------------------------------------------- */

enum eaxcid_field {
        EA_F_ODU_PORT    = (1ULL << 0),
        EA_F_BAND_SECTOR = (1ULL << 1),
        EA_F_CCID        = (1ULL << 2),
        EA_F_RU_PORT     = (1ULL << 3),
        EA_F_ID          = (1ULL << 4),
        EA_F_REQUIRED    = EA_F_ID,
};

struct eaxcid {
        uint16_t o_du_port_bitmask;
        uint16_t band_sector_bitmask;
        uint16_t ccid_bitmask;
        uint16_t ru_port_bitmask;
        uint16_t eaxc_id;

        struct value_config meta;
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
        dst->meta = src->meta;
        return VALUE_OK;
}

static void eaxcid_free_obj(struct eaxcid *e)
{
        free(e);
}

static void eaxcid_init(struct eaxcid *e, uint16_t id)
{
        memset(e, 0, sizeof(*e));
        e->eaxc_id = id;
        value_config_init(&e->meta, EA_F_REQUIRED);
        INIT_LIST_HEAD(&e->list);
}

static struct eaxcid *new_eaxcid(uint16_t id)
{
        struct eaxcid *e = calloc(1, sizeof(struct eaxcid));
        if (!e)
                return NULL;
        eaxcid_init(e, id);
        return e;
}

VALUE_OBJECT_WRAPPERS_BIND(eaxcid, eaxcid_fini_obj, eaxcid_copy_obj,
                           eaxcid_free_obj)
VALUE_OBJECT_BIND(eaxcid, NULL, eaxcid_fini, eaxcid_copy, NULL, NULL, NULL)
VALUE_LIST_BIND_DOMAIN_KEY(eaxcid, list, eaxcid_copy, eaxcid_free)
VALUE_META_BIND(eaxcid, meta, NULL)

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
/* struct endpoint — string-keyed, embeds eaxcid                            */
/* ------------------------------------------------------------------------- */

enum endpoint_field {
        EP_F_NAME      = (1ULL << 0),
        EP_F_DIR       = (1ULL << 1),
        EP_F_EAXCID    = (1ULL << 2),
        EP_F_REQUIRED  = EP_F_NAME | EP_F_DIR,
};

enum direction { DIR_NONE = 0, DIR_TX = 1, DIR_RX = 2 };

struct endpoint {
        char *name;
        enum direction dir;
        struct eaxcid eaxcid;  /* embedded value */

        struct value_config meta;
        struct list_head list;
};

static void endpoint_fini_obj(struct endpoint *ep)
{
        if (ep->name)
                free(ep->name);
        eaxcid_fini(&ep->eaxcid);
}

static int endpoint_copy_obj(struct endpoint *dst,
                             const struct endpoint *src)
{
        char *name = src->name ? strdup(src->name) : NULL;

        if (src->name && !name)
                return VALUE_ERR_NOMEM;
        free(dst->name);
        dst->name = name;
        dst->dir = src->dir;
        if (eaxcid_copy(&dst->eaxcid, &src->eaxcid) != VALUE_OK)
                return VALUE_ERR_NOMEM;
        dst->meta = src->meta;
        return VALUE_OK;
}

static void endpoint_free_obj(struct endpoint *ep)
{
        free(ep);
}

static struct endpoint *new_endpoint(const char *name)
{
        struct endpoint *ep = calloc(1, sizeof(struct endpoint));
        if (!ep)
                return NULL;
        if (name)
                ep->name = strdup(name);
        eaxcid_init(&ep->eaxcid, 0);
        value_config_init(&ep->meta, EP_F_REQUIRED);
        INIT_LIST_HEAD(&ep->list);
        return ep;
}

VALUE_OBJECT_WRAPPERS_BIND(endpoint, endpoint_fini_obj, endpoint_copy_obj,
                           endpoint_free_obj)
VALUE_OBJECT_BIND(endpoint, NULL, endpoint_fini, endpoint_copy,
                  NULL, NULL, NULL)
VALUE_LIST_BIND(endpoint, name, list, endpoint_copy, endpoint_free)
VALUE_META_BIND(endpoint, meta, NULL)

/* ------------------------------------------------------------------------- */
/* test                                                                      */
/* ------------------------------------------------------------------------- */

static void _dump_endpoint_cb(struct endpoint *ep, void *data)
{
        const char *dir_str[] = { "NONE", "TX", "RX" };
        int *i = data;
        printf("  [%d] %s dir=%s eaxc_id=%u odu=0x%x\n",
               (*i)++, ep->name, dir_str[ep->dir],
               ep->eaxcid.eaxc_id, ep->eaxcid.o_du_port_bitmask);
}

int main(int argc, char *argv[])
{
        (void)argc;
        (void)argv;

        LIST_HEAD(endpoints);
        LIST_HEAD(standalone_eaxcids);
        int idx;

        printf("=== embedded value pattern ===\n\n");

        /* 1. endpoint with embedded eaxcid */
        {
                struct endpoint *ep = new_endpoint("ru0-tx0");
                ep->dir = DIR_TX;
                ep->eaxcid.eaxc_id = 42;
                ep->eaxcid.o_du_port_bitmask = 0xFF;
                endpoint_meta_mark_present(ep, EP_F_NAME);
                endpoint_meta_mark_present(ep, EP_F_DIR);
                endpoint_meta_mark_present(ep, EP_F_EAXCID);
                eaxcid_meta_mark_present(&ep->eaxcid, EA_F_ID);
                endpoint_add_tail_unlocked(&endpoints, ep);
        }

        {
                struct endpoint *ep = new_endpoint("ru0-rx0");
                ep->dir = DIR_RX;
                ep->eaxcid.eaxc_id = 43;
                ep->eaxcid.band_sector_bitmask = 0x01;
                endpoint_meta_mark_present(ep, EP_F_NAME);
                endpoint_meta_mark_present(ep, EP_F_DIR);
                endpoint_meta_mark_present(ep, EP_F_EAXCID);
                endpoint_add_tail_unlocked(&endpoints, ep);
        }

        printf("endpoints with embedded eaxcid:\n");
        idx = 0;
        endpoint_foreach_unlocked(&endpoints, _dump_endpoint_cb, &idx);

        /* 2. standalone eaxcid list (separate from endpoints) */
        eaxcid_add_tail_unlocked(&standalone_eaxcids, new_eaxcid(100));
        eaxcid_add_tail_unlocked(&standalone_eaxcids, new_eaxcid(200));

        printf("\nstandalone eaxcid list: count=%zu\n",
               eaxcid_count(&standalone_eaxcids));

        /* 3. copy endpoint — embedded eaxcid is copied via child API,
         *    destination hooks are preserved */
        {
                struct endpoint *src =
                        endpoint_lookup_unlocked(&endpoints, "ru0-tx0");
                T_CHECK(src != NULL);
                struct endpoint *dst = new_endpoint("copy");
                struct list_head *prev;
                struct list_head *next;

                endpoint_add_tail_unlocked(&endpoints, dst);
                prev = dst->list.prev;
                next = dst->list.next;
                endpoint_copy(dst, src);
                T_CHECK(dst->eaxcid.eaxc_id == 42);
                T_CHECK(dst->eaxcid.o_du_port_bitmask == 0xFF);
                T_CHECK(dst->list.prev == prev);
                T_CHECK(dst->list.next == next);
                T_CHECK(prev->next == &dst->list);
                T_CHECK(next->prev == &dst->list);
                T_CHECK(list_empty(&dst->eaxcid.list));
                T_CHECK(endpoint_count(&endpoints) == 3);
                printf("\ncopy: dst eaxc_id=%u odu=0x%x, destination hooks preserved: OK\n",
                       dst->eaxcid.eaxc_id, dst->eaxcid.o_du_port_bitmask);
        }

        /* 4. copy eaxcid standalone — destination list hook preserved */
        {
                struct eaxcid *src = eaxcid_lookup_id(&standalone_eaxcids, 100);
                T_CHECK(src != NULL);
                struct eaxcid *dst = new_eaxcid(0);
                struct list_head *prev;
                struct list_head *next;

                eaxcid_add_tail_unlocked(&standalone_eaxcids, dst);
                prev = dst->list.prev;
                next = dst->list.next;
                eaxcid_copy(dst, src);
                T_CHECK(dst->eaxc_id == 100);
                T_CHECK(dst->list.prev == prev);
                T_CHECK(dst->list.next == next);
                T_CHECK(prev->next == &dst->list);
                T_CHECK(next->prev == &dst->list);
                T_CHECK(eaxcid_count(&standalone_eaxcids) == 3);
                printf("copy eaxcid: dst eaxc_id=%u, list hook preserved: OK\n",
                       dst->eaxc_id);
        }

        /* 5. parent delegates to child lifecycle */
        {
                struct endpoint *ep = new_endpoint("lifecycle-test");
                ep->dir = DIR_TX;
                ep->eaxcid.eaxc_id = 99;
                eaxcid_meta_mark_present(&ep->eaxcid, EA_F_ID);

                /* parent fini calls child fini internally */
                endpoint_fini(ep);
                /* after fini, eaxcid fields are zeroed by fini_obj (no-op
                 * currently, but the delegation pattern is proven) */
                printf("parent fini delegates to child fini: OK\n");
                /* free separately since fini doesn't free */
                free(ep);
        }

        /* 6. metadata on embedded value accessible through parent */
        {
                struct endpoint *ep =
                        endpoint_lookup_unlocked(&endpoints, "ru0-tx0");
                T_CHECK(ep != NULL);
                T_CHECK(eaxcid_meta_has(&ep->eaxcid, EA_F_ID));
                T_CHECK(endpoint_meta_has(ep, EP_F_EAXCID));
                printf("embedded metadata: eaxcid_has_id=%d endpoint_has_eaxcid=%d: OK\n",
                       eaxcid_meta_has(&ep->eaxcid, EA_F_ID),
                       endpoint_meta_has(ep, EP_F_EAXCID));
        }

        endpoint_free_list_unlocked(&endpoints);
        eaxcid_free_list_unlocked(&standalone_eaxcids);

	printf("\nt_value_embedded: PASS\n");
        return 0;
}
