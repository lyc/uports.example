/*
 * Test-local RU value model implementation.
 *
 * RU Agent specific value model prototype built on top of value_base.
 */

#include <stdlib.h>
#include <string.h>

#include "t_ru_value_fixture.h"

/* -------------------------------------------------------------------------- */
/* eaxcid: separated reusable embedded/list-aware value                       */

void eaxcid_init(struct eaxcid *e, const char *name)
{
        memset(e, 0, sizeof(*e));
        if (name)
                e->name = strdup(name);
        value_config_init(&e->meta, EAXCID_ALL_FIELDS);
        INIT_LIST_HEAD(&e->list);
}

struct eaxcid *new_eaxcid(const char *name)
{
        struct eaxcid *e;

        e = calloc(1, sizeof(*e));
        if (!e)
                return NULL;
        eaxcid_init(e, name);
        return e;
}

void eaxcid_unlink(struct eaxcid *e)
{
        if (!e)
                return;
        if (!list_empty(&e->list))
                list_del_init(&e->list);
}

/*
 * Lifecycle callback criteria:
 * - release owned storage only; object storage is freed by the wrapper
 * - leave the object harmless for repeated cleanup paths
 * - unlink this standalone/embedded hook by explicit model rule
 */
static void eaxcid_fini_obj(struct eaxcid *e)
{
        if (!e)
                return;
        eaxcid_unlink(e);
        free(e->name);
        e->name = NULL;
}

/*
 * Copy callback criteria:
 * - accept an already-initialized destination
 * - deep-copy owned strings and scalar metadata
 * - preserve the destination list hook by never assigning it from src
 * - keep dst unchanged on allocation failure
 */
static int eaxcid_copy_obj(struct eaxcid *dst, const struct eaxcid *src)
{
        char *name = NULL;

        if (!dst || !src)
                return VALUE_ERR_ARG;
        if (dst == src)
                return VALUE_OK;

        if (src->name) {
                name = strdup(src->name);
                if (!name)
                        return VALUE_ERR_NOMEM;
        }

        free(dst->name);
        dst->name = name;
        dst->o_du_port_bitmask = src->o_du_port_bitmask;
        dst->band_sector_bitmask = src->band_sector_bitmask;
        dst->ccid_bitmask = src->ccid_bitmask;
        dst->ru_port_bitmask = src->ru_port_bitmask;
        dst->id = src->id;
        dst->meta = src->meta;

        return VALUE_OK;
}

bool eaxcid_equal(const struct eaxcid *a,
                         const struct eaxcid *b)
{
        if (!a || !b)
                return false;

        return a->o_du_port_bitmask == b->o_du_port_bitmask &&
               a->band_sector_bitmask == b->band_sector_bitmask &&
               a->ccid_bitmask == b->ccid_bitmask &&
               a->ru_port_bitmask == b->ru_port_bitmask &&
               a->id == b->id;
}

bool eaxcid_is_complete(const struct eaxcid *e)
{
        return e && value_config_complete(&e->meta);
}

int eaxcid_set_id(struct eaxcid *e, uint16_t id)
{
        if (!e)
                return VALUE_ERR_ARG;

        if (!value_config_has(&e->meta, EAXCID_ID)) {
                e->id = id;
                value_config_mark_present(&e->meta, EAXCID_ID);
        } else if (e->id != id) {
                e->id = id;
                value_config_mark_dirty(&e->meta, EAXCID_ID);
        }
        return VALUE_OK;
}

int eaxcid_get_id(const struct eaxcid *e, uint16_t *id)
{
        if (!e || !id)
                return VALUE_ERR_ARG;
        if (!value_config_has(&e->meta, EAXCID_ID))
                return VALUE_ERR_MISSING;
        if (!value_config_can_use(&e->meta, EAXCID_ID))
                return VALUE_ERR_INVALID;

        *id = e->id;
        return VALUE_OK;
}

int eaxcid_set_ru_port_bitmask(struct eaxcid *e, uint16_t mask)
{
        if (!e)
                return VALUE_ERR_ARG;

        if (!value_config_has(&e->meta, EAXCID_RU_PORT_BITMASK)) {
                e->ru_port_bitmask = mask;
                value_config_mark_present(&e->meta, EAXCID_RU_PORT_BITMASK);
        } else if (e->ru_port_bitmask != mask) {
                e->ru_port_bitmask = mask;
                value_config_mark_dirty(&e->meta, EAXCID_RU_PORT_BITMASK);
        }
        return VALUE_OK;
}

uint64_t eaxcid_diff_present_fields(
        const struct eaxcid *a, const struct eaxcid *b)
{
        uint64_t fields;
        uint64_t diff = 0;

        if (!a || !b)
                return UINT64_MAX;

        fields = a->meta.present | b->meta.present;

        if ((fields & EAXCID_ODU_PORT_BITMASK) &&
            (!value_config_has(&a->meta, EAXCID_ODU_PORT_BITMASK) ||
             !value_config_has(&b->meta, EAXCID_ODU_PORT_BITMASK) ||
             a->o_du_port_bitmask != b->o_du_port_bitmask))
                diff |= EAXCID_ODU_PORT_BITMASK;

        if ((fields & EAXCID_BAND_SECTOR_BITMASK) &&
            (!value_config_has(&a->meta, EAXCID_BAND_SECTOR_BITMASK) ||
             !value_config_has(&b->meta, EAXCID_BAND_SECTOR_BITMASK) ||
             a->band_sector_bitmask != b->band_sector_bitmask))
                diff |= EAXCID_BAND_SECTOR_BITMASK;

        if ((fields & EAXCID_CCID_BITMASK) &&
            (!value_config_has(&a->meta, EAXCID_CCID_BITMASK) ||
             !value_config_has(&b->meta, EAXCID_CCID_BITMASK) ||
             a->ccid_bitmask != b->ccid_bitmask))
                diff |= EAXCID_CCID_BITMASK;

        if ((fields & EAXCID_RU_PORT_BITMASK) &&
            (!value_config_has(&a->meta, EAXCID_RU_PORT_BITMASK) ||
             !value_config_has(&b->meta, EAXCID_RU_PORT_BITMASK) ||
             a->ru_port_bitmask != b->ru_port_bitmask))
                diff |= EAXCID_RU_PORT_BITMASK;

        if ((fields & EAXCID_ID) &&
            (!value_config_has(&a->meta, EAXCID_ID) ||
             !value_config_has(&b->meta, EAXCID_ID) ||
             a->id != b->id))
                diff |= EAXCID_ID;

        return diff;
}

int eaxcid_merge_present(struct eaxcid *dst, const struct eaxcid *src)
{
        if (!dst || !src)
                return VALUE_ERR_ARG;

        if (value_config_has(&src->meta, EAXCID_ODU_PORT_BITMASK)) {
                if (!value_config_has(&dst->meta, EAXCID_ODU_PORT_BITMASK)) {
                        dst->o_du_port_bitmask = src->o_du_port_bitmask;
                        value_config_mark_present(&dst->meta,
                                            EAXCID_ODU_PORT_BITMASK);
                } else if (dst->o_du_port_bitmask !=
                           src->o_du_port_bitmask) {
                        dst->o_du_port_bitmask = src->o_du_port_bitmask;
                        value_config_mark_dirty(&dst->meta,
                                          EAXCID_ODU_PORT_BITMASK);
                }
        }
        if (value_config_has(&src->meta, EAXCID_BAND_SECTOR_BITMASK)) {
                if (!value_config_has(&dst->meta, EAXCID_BAND_SECTOR_BITMASK)) {
                        dst->band_sector_bitmask = src->band_sector_bitmask;
                        value_config_mark_present(&dst->meta,
                                            EAXCID_BAND_SECTOR_BITMASK);
                } else if (dst->band_sector_bitmask !=
                           src->band_sector_bitmask) {
                        dst->band_sector_bitmask = src->band_sector_bitmask;
                        value_config_mark_dirty(&dst->meta,
                                          EAXCID_BAND_SECTOR_BITMASK);
                }
        }
        if (value_config_has(&src->meta, EAXCID_CCID_BITMASK)) {
                if (!value_config_has(&dst->meta, EAXCID_CCID_BITMASK)) {
                        dst->ccid_bitmask = src->ccid_bitmask;
                        value_config_mark_present(&dst->meta, EAXCID_CCID_BITMASK);
                } else if (dst->ccid_bitmask != src->ccid_bitmask) {
                        dst->ccid_bitmask = src->ccid_bitmask;
                        value_config_mark_dirty(&dst->meta, EAXCID_CCID_BITMASK);
                }
        }
        if (value_config_has(&src->meta, EAXCID_RU_PORT_BITMASK))
                eaxcid_set_ru_port_bitmask(dst, src->ru_port_bitmask);
        if (value_config_has(&src->meta, EAXCID_ID))
                eaxcid_set_id(dst, src->id);
        return VALUE_OK;
}

int eaxcid_validate(struct eaxcid *e)
{
        if (!e)
                return VALUE_ERR_ARG;

        if (!eaxcid_is_complete(e)) {
                value_config_mark_invalid(&e->meta,
                                       value_config_missing_required(&e->meta),
                                       "missing required eaxcid fields");
                return VALUE_ERR_MISSING;
        }

        value_config_mark_valid(&e->meta, EAXCID_ALL_FIELDS);
        return VALUE_OK;
}

static const char *eaxcid_field_label(uint64_t field)
{
        switch (field) {
        case EAXCID_ODU_PORT_BITMASK:
                return "odu-port-bitmask";
        case EAXCID_BAND_SECTOR_BITMASK:
                return "band-sector-bitmask";
        case EAXCID_CCID_BITMASK:
                return "ccid-bitmask";
        case EAXCID_RU_PORT_BITMASK:
                return "ru-port-bitmask";
        case EAXCID_ID:
                return "eaxc-id";
        default:
                return "unknown";
        }
}

VALUE_OBJECT_WRAPPERS_BIND(eaxcid, eaxcid_fini_obj, eaxcid_copy_obj, free)
VALUE_OBJECT_BIND(eaxcid, eaxcid_init, eaxcid_fini,
                  eaxcid_copy, eaxcid_validate, NULL, eaxcid_equal)
VALUE_META_BIND(eaxcid, meta, eaxcid_field_label)
VALUE_LIST_BIND_DOMAIN_KEY(eaxcid, list, eaxcid_copy, eaxcid_free)

struct eaxcid *eaxcid_lookup_id_unlocked(struct list_head *head, uint16_t id)
{
        struct list_head *pos;
        struct eaxcid *value;

        if (!head)
                return NULL;

        list_for_each(pos, head) {
                value = list_entry(pos, struct eaxcid, list);
                if (eaxcid_has_field(value, EAXCID_ID) && value->id == id)
                        return value;
        }

        return NULL;
}

/* -------------------------------------------------------------------------- */
/* element                                                                    */

void element_init(struct element *e, const char *name)
{
        memset(e, 0, sizeof(*e));
        if (name)
                e->name = strdup(name);
        value_config_init(&e->meta, ELEMENT_ALL_FIELDS);
        INIT_LIST_HEAD(&e->list);
}

struct element *new_element(const char *name)
{
        struct element *e;

        e = calloc(1, sizeof(*e));
        if (!e)
                return NULL;
        element_init(e, name);
        return e;
}

/*
 * Lifecycle callback criteria:
 * - release all owned strings
 * - do not free object storage
 * - unlink the outer hook before finalization by this model's rule
 */
static void element_fini_obj(struct element *e)
{
        if (!e)
                return;
        if (!list_empty(&e->list))
                list_del_init(&e->list);
        free(e->name);
        free(e->transport_interface);
        e->name = NULL;
        e->transport_interface = NULL;
}

/*
 * Copy callback criteria:
 * - copy into an initialized destination without changing dst->list
 * - deep-copy both owned strings
 * - copy metadata as part of the value snapshot
 * - keep dst unchanged on allocation failure
 */
static int element_copy_obj(struct element *dst, const struct element *src)
{
        char *name = NULL;
        char *transport_interface = NULL;

        if (!dst || !src)
                return VALUE_ERR_ARG;
        if (dst == src)
                return VALUE_OK;

        if (src->name) {
                name = strdup(src->name);
                if (!name)
                        return VALUE_ERR_NOMEM;
        }
        if (src->transport_interface) {
                transport_interface = strdup(src->transport_interface);
                if (!transport_interface) {
                        free(name);
                        return VALUE_ERR_NOMEM;
                }
        }

        free(dst->name);
        free(dst->transport_interface);
        dst->name = name;
        dst->transport_interface = transport_interface;
        dst->meta = src->meta;
        return VALUE_OK;
}

static const char *element_field_label(uint64_t field)
{
        switch (field) {
        case ELEMENT_TRANSPORT_INTERFACE:
                return "transport-interface";
        default:
                return "unknown";
        }
}

VALUE_OBJECT_WRAPPERS_BIND(element, element_fini_obj, element_copy_obj, free)
VALUE_OBJECT_BIND(element, element_init, element_fini,
                  element_copy, NULL, NULL, NULL)
VALUE_META_BIND(element, meta, element_field_label)
VALUE_LIST_BIND(element, name, list, element_copy, element_free)

/* -------------------------------------------------------------------------- */
/* carrier                                                                    */

void carrier_init(struct carrier *c, const char *name,
                         enum direction dir)
{
        memset(c, 0, sizeof(*c));
        if (name)
                c->name = strdup(name);
        c->dir = dir;
        value_config_init(&c->meta, CARRIER_ALL_FIELDS);
        INIT_LIST_HEAD(&c->list);
}

struct carrier *new_carrier(const char *name, enum direction dir)
{
        struct carrier *c;

        c = calloc(1, sizeof(*c));
        if (!c)
                return NULL;
        carrier_init(c, name, dir);
        return c;
}

/*
 * Lifecycle callback criteria:
 * - release owned name
 * - do not free object storage
 * - preserve a harmless state after unlinking this model's outer hook
 */
static void carrier_fini_obj(struct carrier *c)
{
        if (!c)
                return;
        if (!list_empty(&c->list))
                list_del_init(&c->list);
        free(c->name);
        c->name = NULL;
}

static void carrier_init_object(struct carrier *c, const char *name)
{
        carrier_init(c, name, TX);
}

int carrier_set_center_bandwidth(
        struct carrier *c, unsigned long bandwidth)
{
        if (!c)
                return VALUE_ERR_ARG;

        if (!value_config_has(&c->meta, CARRIER_CENTER_BANDWIDTH)) {
                c->center_of_channel_bandwidth = bandwidth;
                value_config_mark_present(&c->meta, CARRIER_CENTER_BANDWIDTH);
        } else if (c->center_of_channel_bandwidth != bandwidth) {
                c->center_of_channel_bandwidth = bandwidth;
                value_config_mark_dirty(&c->meta, CARRIER_CENTER_BANDWIDTH);
        }
        return VALUE_OK;
}

int carrier_set_channel_bandwidth(
        struct carrier *c, unsigned long bandwidth)
{
        if (!c)
                return VALUE_ERR_ARG;

        if (!value_config_has(&c->meta, CARRIER_CHANNEL_BANDWIDTH)) {
                c->channel_bandwidth = bandwidth;
                value_config_mark_present(&c->meta, CARRIER_CHANNEL_BANDWIDTH);
        } else if (c->channel_bandwidth != bandwidth) {
                c->channel_bandwidth = bandwidth;
                value_config_mark_dirty(&c->meta, CARRIER_CHANNEL_BANDWIDTH);
        }
        return VALUE_OK;
}

int carrier_set_gain(struct carrier *c, float gain)
{
        if (!c)
                return VALUE_ERR_ARG;

        if (!value_config_has(&c->meta, CARRIER_GAIN)) {
                c->gain = gain;
                value_config_mark_present(&c->meta, CARRIER_GAIN);
        } else if (c->gain != gain) {
                c->gain = gain;
                value_config_mark_dirty(&c->meta, CARRIER_GAIN);
        }
        return VALUE_OK;
}

/*
 * Copy callback criteria:
 * - copy a full value snapshot into an initialized destination
 * - preserve dst->list
 * - deep-copy owned name
 * - copy metadata and scalar configuration fields together
 */
static int carrier_copy_obj(struct carrier *dst,
                            const struct carrier *src)
{
        char *name = NULL;

        if (!dst || !src)
                return VALUE_ERR_ARG;
        if (dst == src)
                return VALUE_OK;
        if (src->name) {
                name = strdup(src->name);
                if (!name)
                        return VALUE_ERR_NOMEM;
        }

        free(dst->name);
        dst->name = name;
        dst->dir = src->dir;
        dst->absolute_frequency_center = src->absolute_frequency_center;
        dst->center_of_channel_bandwidth = src->center_of_channel_bandwidth;
        dst->channel_bandwidth = src->channel_bandwidth;
        dst->gain = src->gain;
        dst->active = src->active;
        dst->meta = src->meta;
        return VALUE_OK;
}

static int VALUE_UNUSED carrier_merge_present(struct carrier *dst,
                                             const struct carrier *src)
{
        if (!dst || !src || dst->dir != src->dir)
                return VALUE_ERR_ARG;

        if (value_config_has(&src->meta, CARRIER_CENTER_BANDWIDTH))
                carrier_set_center_bandwidth(
                        dst, src->center_of_channel_bandwidth);
        if (value_config_has(&src->meta, CARRIER_CHANNEL_BANDWIDTH))
                carrier_set_channel_bandwidth(dst, src->channel_bandwidth);
        if (value_config_has(&src->meta, CARRIER_GAIN))
                carrier_set_gain(dst, src->gain);
        return VALUE_OK;
}

int carrier_validate(struct carrier *c)
{
        if (!c)
                return VALUE_ERR_ARG;
        if (!value_config_complete(&c->meta))
                return VALUE_ERR_MISSING;
        value_config_mark_valid(&c->meta, CARRIER_ALL_FIELDS);
        return VALUE_OK;
}

static const char *carrier_field_label(uint64_t field)
{
        switch (field) {
        case CARRIER_ABSOLUTE_FREQUENCY:
                return "absolute-frequency-center";
        case CARRIER_CENTER_BANDWIDTH:
                return "center-of-channel-bandwidth";
        case CARRIER_CHANNEL_BANDWIDTH:
                return "channel-bandwidth";
        case CARRIER_GAIN:
                return "gain";
        case CARRIER_ACTIVE:
                return "active";
        default:
                return "unknown";
        }
}

VALUE_OBJECT_WRAPPERS_BIND(carrier, carrier_fini_obj, carrier_copy_obj, free)
VALUE_OBJECT_BIND(carrier, carrier_init_object, carrier_fini,
                  carrier_copy, carrier_validate, NULL, NULL)
VALUE_META_BIND(carrier, meta, carrier_field_label)
VALUE_LIST_BIND(carrier, name, list, carrier_copy, carrier_free)

/* -------------------------------------------------------------------------- */
/* endpoint                                                                   */

void endpoint_init(struct endpoint *ep, const char *name,
                          enum direction dir)
{
        memset(ep, 0, sizeof(*ep));
        if (name)
                ep->name = strdup(name);
        ep->dir = dir;
        eaxcid_init(&ep->eaxcid, NULL);
        value_config_init(&ep->meta, ENDPOINT_ALL_FIELDS);
        INIT_LIST_HEAD(&ep->list);
}

struct endpoint *new_endpoint(const char *name, enum direction dir)
{
        struct endpoint *ep;

        ep = calloc(1, sizeof(*ep));
        if (!ep)
                return NULL;
        endpoint_init(ep, name, dir);
        return ep;
}

/*
 * Lifecycle callback criteria:
 * - release the owned name and embedded eaxcid
 * - preserve the outer model rule that embedded eaxcid links are unlinked
 * - do not free endpoint storage
 */
static void endpoint_fini_obj(struct endpoint *ep)
{
        if (!ep)
                return;
        if (!list_empty(&ep->list))
                list_del_init(&ep->list);
        eaxcid_unlink(&ep->eaxcid);
        eaxcid_fini(&ep->eaxcid);
        free(ep->name);
        ep->name = NULL;
}

static void endpoint_init_object(struct endpoint *ep, const char *name)
{
        endpoint_init(ep, name, TX);
}

int endpoint_set_eaxcid(struct endpoint *ep,
                               const struct eaxcid *eaxcid)
{
        if (!ep || !eaxcid)
                return VALUE_ERR_ARG;

        if (!value_config_has(&ep->meta, ENDPOINT_EAXCID)) {
                eaxcid_copy(&ep->eaxcid, eaxcid);
                value_config_mark_present(&ep->meta, ENDPOINT_EAXCID);
        } else if (!eaxcid_equal(&ep->eaxcid, eaxcid)) {
                eaxcid_copy(&ep->eaxcid, eaxcid);
                value_config_mark_dirty(&ep->meta, ENDPOINT_EAXCID);
        }
        return VALUE_OK;
}

int endpoint_get_eaxcid(
        const struct endpoint *ep, const struct eaxcid **eaxcid)
{
        if (!ep || !eaxcid)
                return VALUE_ERR_ARG;
        if (!value_config_has(&ep->meta, ENDPOINT_EAXCID))
                return VALUE_ERR_MISSING;
        if (!value_config_can_use(&ep->meta, ENDPOINT_EAXCID))
                return VALUE_ERR_INVALID;
        if (ep->eaxcid.meta.invalid)
                return VALUE_ERR_INVALID;
        if (!eaxcid_is_complete(&ep->eaxcid))
                return VALUE_ERR_MISSING;

        *eaxcid = &ep->eaxcid;
        return VALUE_OK;
}

int endpoint_set_eaxcid_id(struct endpoint *ep, uint16_t id)
{
        if (!ep)
                return VALUE_ERR_ARG;

        if (eaxcid_set_id(&ep->eaxcid, id) == VALUE_OK) {
                if (!value_config_has(&ep->meta, ENDPOINT_EAXCID))
                        value_config_mark_present(&ep->meta, ENDPOINT_EAXCID);
                else
                        value_config_mark_dirty(&ep->meta, ENDPOINT_EAXCID);
                return VALUE_OK;
        }
        return VALUE_ERR_INVALID;
}

bool endpoint_has_complete_eaxcid(const struct endpoint *ep)
{
        return ep &&
               value_config_can_use(&ep->meta, ENDPOINT_EAXCID) &&
               eaxcid_is_complete(&ep->eaxcid);
}

static bool VALUE_UNUSED endpoint_is_ready_for_edit_config(
        const struct endpoint *ep)
{
        return ep &&
               value_config_complete(&ep->meta) &&
               endpoint_has_complete_eaxcid(ep);
}

int endpoint_set_prb(struct endpoint *ep, uint16_t prb)
{
        if (!ep)
                return VALUE_ERR_ARG;

        if (!value_config_has(&ep->meta, ENDPOINT_PRB)) {
                ep->number_of_prb = prb;
                value_config_mark_present(&ep->meta, ENDPOINT_PRB);
        } else if (ep->number_of_prb != prb) {
                ep->number_of_prb = prb;
                value_config_mark_dirty(&ep->meta, ENDPOINT_PRB);
        }
        return VALUE_OK;
}

/*
 * Copy callback criteria:
 * - preserve dst->list and the destination embedded eaxcid hook
 * - deep-copy owned name
 * - delegate embedded eaxcid copy through its lifecycle contract
 * - copy metadata and scalar endpoint state as one snapshot
 */
static int endpoint_copy_obj(struct endpoint *dst,
                             const struct endpoint *src)
{
        char *name = NULL;
        int rc;

        if (!dst || !src)
                return VALUE_ERR_ARG;
        if (dst == src)
                return VALUE_OK;
        if (src->name) {
                name = strdup(src->name);
                if (!name)
                        return VALUE_ERR_NOMEM;
        }

        rc = eaxcid_copy(&dst->eaxcid, &src->eaxcid);
        if (rc != VALUE_OK) {
                free(name);
                return rc;
        }

        free(dst->name);
        dst->name = name;
        dst->dir = src->dir;
        dst->endpoint_type = src->endpoint_type;
        dst->number_of_prb = src->number_of_prb;
        dst->non_time_managed_delay_enabled =
                src->non_time_managed_delay_enabled;
        dst->meta = src->meta;
        return VALUE_OK;
}

static int VALUE_UNUSED endpoint_merge_present(struct endpoint *dst,
                                              const struct endpoint *src)
{
        if (!dst || !src || dst->dir != src->dir)
                return VALUE_ERR_ARG;

        if (value_config_has(&src->meta, ENDPOINT_EAXCID))
                endpoint_set_eaxcid(dst, &src->eaxcid);
        if (value_config_has(&src->meta, ENDPOINT_PRB))
                endpoint_set_prb(dst, src->number_of_prb);
        return VALUE_OK;
}

int endpoint_validate(struct endpoint *ep)
{
        if (!ep)
                return VALUE_ERR_ARG;
        if (!value_config_complete(&ep->meta))
                return VALUE_ERR_MISSING;
        if (!endpoint_has_complete_eaxcid(ep))
                return VALUE_ERR_MISSING;
        value_config_mark_valid(&ep->meta, ENDPOINT_ALL_FIELDS);
        return VALUE_OK;
}

static const char *endpoint_field_label(uint64_t field)
{
        switch (field) {
        case ENDPOINT_EAXCID:
                return "eaxcid";
        case ENDPOINT_TYPE:
                return "endpoint-type";
        case ENDPOINT_PRB:
                return "number-of-prb";
        case ENDPOINT_DELAY_MANAGED:
                return "non-time-managed-delay-enabled";
        default:
                return "unknown";
        }
}

VALUE_OBJECT_WRAPPERS_BIND(endpoint, endpoint_fini_obj,
                           endpoint_copy_obj, free)
VALUE_OBJECT_BIND(endpoint, endpoint_init_object, endpoint_fini,
                  endpoint_copy, endpoint_validate, NULL, NULL)
VALUE_META_BIND(endpoint, meta, endpoint_field_label)
VALUE_LIST_BIND(endpoint, name, list, endpoint_copy, endpoint_free)

/* -------------------------------------------------------------------------- */
/* link                                                                       */

void link_init(struct link *l, const char *name)
{
        memset(l, 0, sizeof(*l));
        if (name)
                l->name = strdup(name);
        value_config_init(&l->meta, 0);
        INIT_LIST_HEAD(&l->list);
}

struct link *new_link(const char *name)
{
        struct link *l;

        l = calloc(1, sizeof(*l));
        if (!l)
                return NULL;
        link_init(l, name);
        return l;
}

/*
 * Lifecycle callback criteria:
 * - release owned identity strings only
 * - borrowed element/carrier/endpoint pointers are not owned
 * - unlink this link's outer hook and leave a valid empty hook
 */
static void link_fini_obj(struct link *l)
{
        if (!l)
                return;
        if (!list_empty(&l->list))
                list_del_init(&l->list);
        free(l->name);
        free(l->element.name);
        free(l->carrier.name);
        free(l->endpoint.name);
        memset(l, 0, sizeof(*l));
        INIT_LIST_HEAD(&l->list);
}

/*
 * Copy callback criteria:
 * - preserve dst->list
 * - deep-copy owned identity strings
 * - shallow-copy borrowed resolved pointers by documented pointer identity
 * - copy metadata as part of the value snapshot
 * - keep dst unchanged on allocation failure
 */
static int link_copy_obj(struct link *dst, const struct link *src)
{
        char *name = NULL;
        char *element_name = NULL;
        char *carrier_name = NULL;
        char *endpoint_name = NULL;

        if (!dst || !src)
                return VALUE_ERR_ARG;
        if (dst == src)
                return VALUE_OK;

        if (src->name) {
                name = strdup(src->name);
                if (!name)
                        return VALUE_ERR_NOMEM;
        }
        if (src->element.name) {
                element_name = strdup(src->element.name);
                if (!element_name)
                        goto err;
        }
        if (src->carrier.name) {
                carrier_name = strdup(src->carrier.name);
                if (!carrier_name)
                        goto err;
        }
        if (src->endpoint.name) {
                endpoint_name = strdup(src->endpoint.name);
                if (!endpoint_name)
                        goto err;
        }

        free(dst->name);
        free(dst->element.name);
        free(dst->carrier.name);
        free(dst->endpoint.name);
        dst->name = name;
        dst->element.name = element_name;
        dst->element.value = src->element.value;
        dst->carrier.name = carrier_name;
        dst->carrier.value = src->carrier.value;
        dst->endpoint.name = endpoint_name;
        dst->endpoint.value = src->endpoint.value;
        dst->meta = src->meta;
        return VALUE_OK;

err:
        free(name);
        free(element_name);
        free(carrier_name);
        free(endpoint_name);
        return VALUE_ERR_NOMEM;
}

/*
 * Resolve names to existing values.
 *
 * This replaces the current ad-hoc _fill_link_cb pattern and makes the pointer
 * graph construction explicit.
 */
int link_resolve_unlocked(
        struct link *l,
        struct list_head *elements,
        struct list_head *carriers,
        struct list_head *endpoints)
{
        if (!l)
                return VALUE_ERR_ARG;

        if (l->element.name)
                l->element.value =
                        element_lookup_unlocked(elements, l->element.name);
        if (l->carrier.name)
                l->carrier.value =
                        carrier_lookup_unlocked(carriers, l->carrier.name);
        if (l->endpoint.name)
                l->endpoint.value =
                        endpoint_lookup_unlocked(endpoints, l->endpoint.name);

        if (!l->element.value || !l->carrier.value || !l->endpoint.value)
                return VALUE_ERR_MISSING;
        return VALUE_OK;
}

bool link_is_resolved(const struct link *l)
{
        return l && l->element.value && l->carrier.value && l->endpoint.value;
}

int link_validate(struct link *l)
{
        if (!l)
                return VALUE_ERR_ARG;
        return link_is_resolved(l) ? VALUE_OK : VALUE_ERR_MISSING;
}

VALUE_OBJECT_WRAPPERS_BIND(link, link_fini_obj, link_copy_obj, free)
VALUE_OBJECT_BIND(link, link_init, link_fini,
                  link_copy, link_validate, NULL, NULL)
VALUE_META_BIND(link, meta, NULL)
VALUE_LIST_BIND(link, name, list, link_copy, link_free)

/* -------------------------------------------------------------------------- */
/* Operations over existing values                                            */

void ru_values_init(struct ru_values *ru)
{
        INIT_LIST_HEAD(&ru->elements);
        INIT_LIST_HEAD(&ru->tx_carriers);
        INIT_LIST_HEAD(&ru->rx_carriers);
        INIT_LIST_HEAD(&ru->tx_endpoints);
        INIT_LIST_HEAD(&ru->rx_endpoints);
        INIT_LIST_HEAD(&ru->tx_links);
        INIT_LIST_HEAD(&ru->rx_links);
}

static int resolve_link_list_unlocked(
        struct list_head *links,
        struct list_head *elements,
        struct list_head *carriers,
        struct list_head *endpoints)
{
        struct list_head *p;
        int errors = 0;

        list_for_each(p, links) {
                struct link *l = value_entry(p, &link_list_ops);

                if (link_resolve_unlocked(l, elements, carriers, endpoints) < 0)
                        errors++;
        }
        return errors ? VALUE_ERR_MISSING : VALUE_OK;
}

/*
 * Validate and prune existing C/U-plane values.
 *
 * This captures the current validate_cuplane_values() idea:
 *   1. resolve link names to pointers
 *   2. remove incomplete links
 *   3. remove carriers/endpoints not referenced by remaining links
 *
 * Removal is omitted here to keep this prototype compact; the important point
 * is that the operation is now named and structured around value semantics.
 */
int ru_values_resolve_unlocked(struct ru_values *ru)
{
        int rc;

        rc = resolve_link_list_unlocked(&ru->tx_links, &ru->elements,
                                        &ru->tx_carriers, &ru->tx_endpoints);
        if (rc < 0)
                return rc;

        rc = resolve_link_list_unlocked(&ru->rx_links, &ru->elements,
                                        &ru->rx_carriers, &ru->rx_endpoints);
        return rc;
}

/*
 * O1/H-MPlane runtime update examples.
 *
 * These replace direct writes such as:
 *   c->channel_bandwidth = bw;
 *   ep->e_axcid.eaxc_id = id;
 *
 * with value setters that update metadata consistently.
 */

static uint16_t bandwidth_to_prb(unsigned long bandwidth)
{
        if (bandwidth >= 100000000UL)
                return 273;
        if (bandwidth >= 40000000UL)
                return 106;
        return 24;
}

int link_update_bandwidth(struct link *l, unsigned long bandwidth)
{
        if (!link_is_resolved(l))
                return VALUE_ERR_MISSING;

        carrier_set_channel_bandwidth(l->carrier.value, bandwidth);
        endpoint_set_prb(l->endpoint.value, bandwidth_to_prb(bandwidth));
        return VALUE_OK;
}

int link_update_gain(struct link *l, float gain)
{
        if (!link_is_resolved(l))
                return VALUE_ERR_MISSING;

        return carrier_set_gain(l->carrier.value, gain);
}

int link_update_eaxcid_id(struct link *l, uint16_t id)
{
        if (!link_is_resolved(l))
                return VALUE_ERR_MISSING;

        return endpoint_set_eaxcid_id(l->endpoint.value, id);
}

/*
 * Compare default endpoint against RU endpoint using child value semantics.
 */
static bool VALUE_UNUSED endpoint_eaxcid_matches_default(
        const struct endpoint *ru_ep,
        const struct endpoint *default_ep)
{
        uint16_t ru_port;

        if (!ru_ep || !default_ep)
                return false;

        if (!endpoint_has_complete_eaxcid(ru_ep) ||
            !endpoint_has_complete_eaxcid(default_ep))
                return false;

        /*
         * Preserves the current validation rule shape:
         *   ru_port = ru endpoint ru_port_bitmask & ru endpoint eaxc id
         *   compare with default endpoint eaxc id
         */
        ru_port = ru_ep->eaxcid.ru_port_bitmask & ru_ep->eaxcid.id;
        return ru_port == default_ep->eaxcid.id;
}

/*
 * Accept current values as baseline after initial parse or successful
 * edit-config.
 */
void endpoint_accept(struct endpoint *ep)
{
        if (!ep)
                return;
        value_config_accept(&ep->meta);
        value_config_accept(&ep->eaxcid.meta);
}

void carrier_accept(struct carrier *c)
{
        if (!c)
                return;
        value_config_accept(&c->meta);
}
