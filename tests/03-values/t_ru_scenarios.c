/*
 * Integration scenarios for the test-local RU value model.
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "t_ru_value_fixture.h"
#include "t_ru_scenarios.h"

/* -------------------------------------------------------------------------- */
/* Test harness                                                               */
/* -------------------------------------------------------------------------- */

struct sample_event {
        int code;
        struct list_head list;
};

static void sample_event_init(struct sample_event *event, const char *name)
{
        (void)name;
        memset(event, 0, sizeof(*event));
        INIT_LIST_HEAD(&event->list);
}

/*
 * Local test value lifecycle callbacks follow the framework rule: fini releases
 * object-owned state and unlinks the hook; copy preserves dst->list.
 */
static void sample_event_fini_obj(struct sample_event *event)
{
        if (!event)
                return;
        if (!list_empty(&event->list))
                list_del_init(&event->list);
}

static int sample_event_copy_obj(struct sample_event *dst,
                                 const struct sample_event *src)
{
        if (!dst || !src)
                return VALUE_ERR_ARG;
        dst->code = src->code;
        return VALUE_OK;
}

VALUE_OBJECT_WRAPPERS_BIND(sample_event, sample_event_fini_obj,
                           sample_event_copy_obj, free)
VALUE_OBJECT_BIND(sample_event, sample_event_init, sample_event_fini,
                  sample_event_copy, NULL, NULL, NULL)
VALUE_LIST_BIND_NOKEY(sample_event, list, sample_event_copy,
                      sample_event_free)

static struct sample_event *new_sample_event(int code)
{
        struct sample_event *event;

        event = calloc(1, sizeof(*event));
        if (!event)
                return NULL;
        sample_event_init(event, NULL);
        event->code = code;
        return event;
}

static void fill_eaxcid(struct eaxcid *e, uint16_t id, uint16_t ru_mask)
{
        e->o_du_port_bitmask = 0x0001;
        e->band_sector_bitmask = 0x0002;
        e->ccid_bitmask = 0x0004;
        e->ru_port_bitmask = ru_mask;
        e->id = id;
        value_config_mark_present(&e->meta, EAXCID_ALL_FIELDS);
}

struct endpoint_walk {
        int count;
        unsigned int prb_sum;
};

static void count_endpoint_cb(struct endpoint *ep, void *ctx)
{
        struct endpoint_walk *walk = ctx;

        if (!ep || !walk)
                return;
        walk->count++;
        walk->prb_sum += ep->number_of_prb;
}

static void *sum_endpoint_prb_cb(
        void *acc, struct endpoint *ep, void *ctx)
{
        unsigned int *sum = acc;

        (void)ctx;
        if (sum && ep)
                *sum += ep->number_of_prb;
        return acc;
}

static bool endpoint_prb_at_least_cb(const struct endpoint *ep, void *ctx)
{
        const uint16_t *threshold = ctx;

        return ep && threshold && ep->number_of_prb >= *threshold;
}

static int endpoint_prb_cmp_cb(struct endpoint *a, struct endpoint *b,
                               void *ctx)
{
        (void)ctx;
        return (int)a->number_of_prb - (int)b->number_of_prb;
}

struct sample_event_walk {
        int count;
        int sum;
};

static void sample_event_count_cb(struct sample_event *event, void *ctx)
{
        struct sample_event_walk *walk = ctx;

        walk->count++;
        walk->sum += event->code;
}

static void *sample_event_sum_cb(void *acc, struct sample_event *event,
                                 void *ctx)
{
        int *sum = acc;

        (void)ctx;
        *sum += event->code;
        return acc;
}

static bool sample_event_large_cb(const struct sample_event *event, void *ctx)
{
        int *threshold = ctx;

        return event->code >= *threshold;
}

static int sample_event_cmp_cb(struct sample_event *a, struct sample_event *b,
                               void *ctx)
{
        (void)ctx;
        return a->code - b->code;
}

static int count_filters(struct list_head *filters)
{
        int count = 0;
        struct list_head *p;

        list_for_each(p, filters)
                count++;
        return count;
}

static int clone_endpoint_filter_view(
        struct list_head *out, struct list_head *view)
{
        struct list_head *p;

        if (!out || !view)
                return VALUE_ERR_ARG;

        INIT_LIST_HEAD(out);
        list_for_each(p, view) {
                struct value_filter_ref *filter = list_entry(p, struct value_filter_ref, list);
                struct endpoint *src = filter->ptr;
                struct endpoint *clone = new_endpoint(NULL, TX);

                if (!clone) {
                        endpoint_free_list_unlocked(out);
                        return VALUE_ERR_NOMEM;
                }
                if (endpoint_copy(clone, src) != VALUE_OK) {
                        endpoint_free(clone);
                        endpoint_free_list_unlocked(out);
                        return VALUE_ERR_INVALID;
                }
                endpoint_add_tail_unlocked(out, clone);
        }
        return VALUE_OK;
}

static int test_eaxcid_value(void)
{
        struct eaxcid a, b;
        LIST_HEAD(eaxcids);
        struct list_head *prev;
        struct list_head *next;

        eaxcid_init(&a, "eaxcid-a");
        eaxcid_init(&b, NULL);
        list_add_tail(&b.list, &eaxcids);
        prev = b.list.prev;
        next = b.list.next;

        eaxcid_set_ru_port_bitmask(&a, 0x000f);
        eaxcid_set_id(&a, 0x0003);
        a.o_du_port_bitmask = 0x0001;
        a.band_sector_bitmask = 0x0002;
        a.ccid_bitmask = 0x0004;
        value_config_mark_present(&a.meta, EAXCID_ODU_PORT_BITMASK);
        value_config_mark_present(&a.meta, EAXCID_BAND_SECTOR_BITMASK);
        value_config_mark_present(&a.meta, EAXCID_CCID_BITMASK);

        if (!eaxcid_is_complete(&a))
                return 1;
        if (eaxcid_validate(&a) != VALUE_OK)
                return 2;
        if (eaxcid_copy(&b, &a) != VALUE_OK)
                return 3;
        if (!eaxcid_equal(&a, &b))
                return 4;
        if (!value_equal(&eaxcid_object_ops, &a, &b))
                return 128;
        b.id = 0x0009;
        if (value_equal(&eaxcid_object_ops, &a, &b))
                return 129;
        if (b.list.prev != prev || b.list.next != next ||
            prev->next != &b.list || next->prev != &b.list)
                return 5;

        eaxcid_fini(&a);
        eaxcid_fini(&b);
        return 0;
}

static int test_endpoint_embed(void)
{
        struct eaxcid e;
        struct endpoint ep, copy;
        LIST_HEAD(endpoints);
        LIST_HEAD(eaxcids);
        struct list_head *ep_prev;
        struct list_head *ep_next;
        struct list_head *eaxcid_prev;
        struct list_head *eaxcid_next;

        eaxcid_init(&e, "embedded-source");
        endpoint_init(&ep, "ep0", TX);
        endpoint_init(&copy, NULL, TX);
        list_add_tail(&copy.list, &endpoints);
        list_add_tail(&copy.eaxcid.list, &eaxcids);
        ep_prev = copy.list.prev;
        ep_next = copy.list.next;
        eaxcid_prev = copy.eaxcid.list.prev;
        eaxcid_next = copy.eaxcid.list.next;

        e.o_du_port_bitmask = 0x0001;
        e.band_sector_bitmask = 0x0002;
        e.ccid_bitmask = 0x0004;
        e.ru_port_bitmask = 0x000f;
        e.id = 0x0003;
        value_config_mark_present(&e.meta, EAXCID_ALL_FIELDS);

        if (endpoint_set_eaxcid(&ep, &e) != VALUE_OK)
                return 10;
        if (!endpoint_has_complete_eaxcid(&ep))
                return 11;
        if (endpoint_copy(&copy, &ep) != VALUE_OK)
                return 12;
        if (!endpoint_has_complete_eaxcid(&copy))
                return 13;
        if (!eaxcid_equal(&ep.eaxcid, &copy.eaxcid))
                return 14;
        if (copy.list.prev != ep_prev || copy.list.next != ep_next ||
            ep_prev->next != &copy.list || ep_next->prev != &copy.list)
                return 15;
        if (copy.eaxcid.list.prev != eaxcid_prev ||
            copy.eaxcid.list.next != eaxcid_next ||
            eaxcid_prev->next != &copy.eaxcid.list ||
            eaxcid_next->prev != &copy.eaxcid.list)
                return 16;

        eaxcid_fini(&e);
        endpoint_fini(&ep);
        endpoint_fini(&copy);
        return 0;
}

static int test_link_resolve_and_update(void)
{
        struct ru_values ru;
        struct element element;
        struct carrier carrier;
        struct endpoint endpoint;
        struct eaxcid eaxcid;
        struct link link;

        ru_values_init(&ru);
        element_init(&element, "pe0");
        carrier_init(&carrier, "carrier0", TX);
        endpoint_init(&endpoint, "endpoint0", TX);
        eaxcid_init(&eaxcid, "eaxcid0");
        link_init(&link, "link0");

        element.transport_interface = strdup("eth0");
        if (!element.transport_interface)
                return 20;
        value_config_mark_present(&element.meta, ELEMENT_TRANSPORT_INTERFACE);

        eaxcid.o_du_port_bitmask = 0x0001;
        eaxcid.band_sector_bitmask = 0x0002;
        eaxcid.ccid_bitmask = 0x0004;
        eaxcid.ru_port_bitmask = 0x000f;
        eaxcid.id = 0x0001;
        value_config_mark_present(&eaxcid.meta, EAXCID_ALL_FIELDS);
        endpoint_set_eaxcid(&endpoint, &eaxcid);
        endpoint_set_prb(&endpoint, 24);

        link.element.name = strdup("pe0");
        link.carrier.name = strdup("carrier0");
        link.endpoint.name = strdup("endpoint0");
        if (!link.element.name || !link.carrier.name || !link.endpoint.name)
                return 21;

        list_add_tail(&element.list, &ru.elements);
        list_add_tail(&carrier.list, &ru.tx_carriers);
        list_add_tail(&endpoint.list, &ru.tx_endpoints);
        list_add_tail(&link.list, &ru.tx_links);

        if (ru_values_resolve_unlocked(&ru) != VALUE_OK)
                return 22;
        if (!link_is_resolved(&link))
                return 23;
        if (link_update_bandwidth(&link, 100000000UL) != VALUE_OK)
                return 24;
        if (carrier.channel_bandwidth != 100000000UL)
                return 25;
        if (endpoint.number_of_prb != 273)
                return 26;
        if (link_update_gain(&link, 18.5f) != VALUE_OK)
                return 27;
        if (carrier.gain != 18.5f)
                return 28;
        if (link_update_eaxcid_id(&link, 0x0002) != VALUE_OK)
                return 29;
        if (endpoint.eaxcid.id != 0x0002)
                return 30;

        endpoint_accept(&endpoint);
        carrier_accept(&carrier);
        if (endpoint.meta.dirty || endpoint.eaxcid.meta.dirty ||
            carrier.meta.dirty)
                return 31;

        link_fini(&link);
        eaxcid_fini(&eaxcid);
        endpoint_fini(&endpoint);
        carrier_fini(&carrier);
        element_fini(&element);
        return 0;
}

static int test_multi_ru_shape(void)
{
        struct ru_values ru;
        struct element *element;
        struct carrier *tx_carrier0, *tx_carrier1, *rx_carrier0;
        struct endpoint *tx_endpoint0, *tx_endpoint1, *rx_endpoint0;
        struct eaxcid *tx_eaxcid0, *tx_eaxcid1, *rx_eaxcid0;
        struct link *tx_link0, *tx_link1, *rx_link0;

        ru_values_init(&ru);

        element = new_element("pe0");
        tx_carrier0 = new_carrier("tx-carrier0", TX);
        tx_carrier1 = new_carrier("tx-carrier1", TX);
        rx_carrier0 = new_carrier("rx-carrier0", RX);
        tx_endpoint0 = new_endpoint("tx-endpoint0", TX);
        tx_endpoint1 = new_endpoint("tx-endpoint1", TX);
        rx_endpoint0 = new_endpoint("rx-endpoint0", RX);
        tx_eaxcid0 = new_eaxcid("tx-eaxcid0");
        tx_eaxcid1 = new_eaxcid("tx-eaxcid1");
        rx_eaxcid0 = new_eaxcid("rx-eaxcid0");
        tx_link0 = new_link("tx-link0");
        tx_link1 = new_link("tx-link1");
        rx_link0 = new_link("rx-link0");

        if (!element || !tx_carrier0 || !tx_carrier1 || !rx_carrier0 ||
            !tx_endpoint0 || !tx_endpoint1 || !rx_endpoint0 ||
            !tx_eaxcid0 || !tx_eaxcid1 || !rx_eaxcid0 ||
            !tx_link0 || !tx_link1 || !rx_link0)
                return 70;

        element->transport_interface = strdup("eth0");
        tx_link0->element.name = strdup("pe0");
        tx_link0->carrier.name = strdup("tx-carrier0");
        tx_link0->endpoint.name = strdup("tx-endpoint0");
        tx_link1->element.name = strdup("pe0");
        tx_link1->carrier.name = strdup("tx-carrier1");
        tx_link1->endpoint.name = strdup("tx-endpoint1");
        rx_link0->element.name = strdup("pe0");
        rx_link0->carrier.name = strdup("rx-carrier0");
        rx_link0->endpoint.name = strdup("rx-endpoint0");

        if (!element->transport_interface ||
            !tx_link0->element.name || !tx_link0->carrier.name ||
            !tx_link0->endpoint.name ||
            !tx_link1->element.name || !tx_link1->carrier.name ||
            !tx_link1->endpoint.name ||
            !rx_link0->element.name || !rx_link0->carrier.name ||
            !rx_link0->endpoint.name)
                return 72;

        value_config_mark_present(&element->meta, ELEMENT_TRANSPORT_INTERFACE);
        fill_eaxcid(tx_eaxcid0, 0x0001, 0x000f);
        fill_eaxcid(tx_eaxcid1, 0x0002, 0x000f);
        fill_eaxcid(rx_eaxcid0, 0x0101, 0x00ff);
        endpoint_set_eaxcid(tx_endpoint0, tx_eaxcid0);
        endpoint_set_eaxcid(tx_endpoint1, tx_eaxcid1);
        endpoint_set_eaxcid(rx_endpoint0, rx_eaxcid0);
        endpoint_set_prb(tx_endpoint0, 24);
        endpoint_set_prb(tx_endpoint1, 24);
        endpoint_set_prb(rx_endpoint0, 106);

        element_add_tail_unlocked(&ru.elements, element);
        carrier_add_tail_unlocked(&ru.tx_carriers, tx_carrier0);
        carrier_add_tail_unlocked(&ru.tx_carriers, tx_carrier1);
        carrier_add_tail_unlocked(&ru.rx_carriers, rx_carrier0);
        endpoint_add_tail_unlocked(&ru.tx_endpoints, tx_endpoint0);
        endpoint_add_tail_unlocked(&ru.tx_endpoints, tx_endpoint1);
        endpoint_add_tail_unlocked(&ru.rx_endpoints, rx_endpoint0);
        link_add_tail_unlocked(&ru.tx_links, tx_link0);
        link_add_tail_unlocked(&ru.tx_links, tx_link1);
        link_add_tail_unlocked(&ru.rx_links, rx_link0);

        if (carrier_lookup_unlocked(&ru.tx_carriers, "tx-carrier1") !=
            tx_carrier1)
                return 73;
        if (endpoint_lookup_unlocked(&ru.rx_endpoints, "rx-endpoint0") !=
            rx_endpoint0)
                return 74;

        if (ru_values_resolve_unlocked(&ru) != VALUE_OK)
                return 75;

        if (!link_is_resolved(tx_link0) || !link_is_resolved(tx_link1))
                return 76;
        if (tx_link0->element.value != element ||
            tx_link0->carrier.value != tx_carrier0 ||
            tx_link0->endpoint.value != tx_endpoint0)
                return 77;
        if (tx_link1->element.value != element ||
            tx_link1->carrier.value != tx_carrier1 ||
            tx_link1->endpoint.value != tx_endpoint1)
                return 78;
        if (!link_is_resolved(rx_link0))
                return 78;
        if (rx_link0->element.value != element ||
            rx_link0->carrier.value != rx_carrier0 ||
            rx_link0->endpoint.value != rx_endpoint0)
                return 79;

        if (link_update_bandwidth(tx_link0, 40000000UL) != VALUE_OK)
                return 80;
        if (link_update_bandwidth(tx_link1, 100000000UL) != VALUE_OK)
                return 81;
        if (tx_carrier0->channel_bandwidth != 40000000UL ||
            tx_endpoint0->number_of_prb != 106)
                return 82;
        if (tx_carrier1->channel_bandwidth != 100000000UL ||
            tx_endpoint1->number_of_prb != 273)
                return 83;

        if (link_update_eaxcid_id(tx_link1, 0x0009) != VALUE_OK)
                return 84;
        if (tx_endpoint1->eaxcid.id != 0x0009)
                return 85;
        if (tx_eaxcid1->id != 0x0002)
                return 86;

        if (link_update_gain(rx_link0, -3.5f) != VALUE_OK)
                return 87;
        if (rx_carrier0->gain != -3.5f)
                return 88;

        link_free_list_unlocked(&ru.rx_links);
        link_free_list_unlocked(&ru.tx_links);
        endpoint_free_list_unlocked(&ru.rx_endpoints);
        endpoint_free_list_unlocked(&ru.tx_endpoints);
        carrier_free_list_unlocked(&ru.rx_carriers);
        carrier_free_list_unlocked(&ru.tx_carriers);
        element_free_list_unlocked(&ru.elements);
        eaxcid_free(tx_eaxcid0);
        eaxcid_free(tx_eaxcid1);
        eaxcid_free(rx_eaxcid0);
        return 0;
}

static int test_list_lookup(void)
{
        LIST_HEAD(eaxcids);
        struct eaxcid a, b;

        eaxcid_init(&a, "a");
        eaxcid_init(&b, "b");
        eaxcid_set_id(&a, 1);
        eaxcid_set_id(&b, 2);
        list_add_tail(&a.list, &eaxcids);
        list_add_tail(&b.list, &eaxcids);

        if (eaxcid_lookup_id_unlocked(&eaxcids, 2) != &b)
                return 40;
        if (eaxcid_lookup_id_unlocked(&eaxcids, 3))
                return 41;
        if (list_entry(eaxcids.next, struct eaxcid, list) != &a)
                return 42;

        eaxcid_fini(&a);
        eaxcid_fini(&b);
        return 0;
}

static int test_list_functional_wrappers(void)
{
        LIST_HEAD(endpoints);
        LIST_HEAD(view);
        LIST_HEAD(clones);
        struct endpoint *low, *mid, *high, *clone_mid;
        struct endpoint_walk walk = { 0, 0 };
        uint16_t threshold = 100;
        unsigned int reduced_sum = 0;

        low = new_endpoint("ep-low", TX);
        mid = new_endpoint("ep-mid", TX);
        high = new_endpoint("ep-high", TX);
        if (!low || !mid || !high)
                return 90;

        endpoint_set_prb(low, 24);
        endpoint_set_prb(mid, 106);
        endpoint_set_prb(high, 273);
        endpoint_add_tail_unlocked(&endpoints, low);
        endpoint_add_tail_unlocked(&endpoints, mid);
        endpoint_add_tail_unlocked(&endpoints, high);

        endpoint_foreach_unlocked(&endpoints, count_endpoint_cb, &walk);
        if (walk.count != 3 || walk.prb_sum != 403)
                return 91;

        endpoint_reduce_unlocked(
                &endpoints, sum_endpoint_prb_cb, NULL, &reduced_sum);
        if (reduced_sum != 403)
                return 92;

        if (endpoint_lookup_move_front_unlocked(&endpoints, "ep-high") != high)
                return 93;
        if (list_entry(endpoints.next, struct endpoint, list) != high)
                return 94;

        if (endpoint_filter_view_unlocked(
                    &view, &endpoints, endpoint_prb_at_least_cb,
                    &threshold) != VALUE_OK)
                return 95;
        if (count_filters(&view) != 2)
                return 96;
        if (value_filter_view_first(&view) != high)
                return 97;

        if (clone_endpoint_filter_view(&clones, &view) != VALUE_OK)
                return 98;
        value_filter_view_free(&view);

        clone_mid = endpoint_lookup_unlocked(&clones, "ep-mid");
        if (!clone_mid || clone_mid == mid)
                return 99;
        if (clone_mid->number_of_prb != 106)
                return 100;

        endpoint_set_prb(mid, 51);
        if (clone_mid->number_of_prb != 106)
                return 101;

        endpoint_sort_unlocked(&endpoints, endpoint_prb_cmp_cb, NULL);
        if (list_entry(endpoints.next, struct endpoint, list) != low)
                return 130;
        if (list_entry(endpoints.prev, struct endpoint, list) != high)
                return 131;

        endpoint_free_list_unlocked(&clones);
        endpoint_free_list_unlocked(&endpoints);
        return 0;
}

static int test_safe_getters(void)
{
        struct eaxcid eaxcid;
        struct endpoint endpoint;
        const struct eaxcid *child = NULL;
        uint16_t id = 0xffff;

        eaxcid_init(&eaxcid, "getter-eaxcid");
        endpoint_init(&endpoint, "getter-endpoint", TX);

        if (eaxcid_get_id(&eaxcid, &id) != VALUE_ERR_MISSING)
                return 103;
        if (id != 0xffff)
                return 104;

        eaxcid_set_id(&eaxcid, 0x0011);
        if (eaxcid_get_id(&eaxcid, &id) != VALUE_OK)
                return 105;
        if (id != 0x0011)
                return 106;

        if (endpoint_get_eaxcid(&endpoint, &child) != VALUE_ERR_MISSING)
                return 107;
        if (child)
                return 108;

        fill_eaxcid(&eaxcid, 0x0022, 0x000f);
        if (endpoint_set_eaxcid(&endpoint, &eaxcid) != VALUE_OK)
                return 109;
        if (endpoint_get_eaxcid(&endpoint, &child) != VALUE_OK)
                return 110;
        if (child != &endpoint.eaxcid)
                return 111;
        if (eaxcid_get_id(child, &id) != VALUE_OK || id != 0x0022)
                return 112;

        value_config_mark_invalid(&endpoint.eaxcid.meta,
                            EAXCID_ID, "invalid eaxcid id");
        child = NULL;
        if (endpoint_get_eaxcid(&endpoint, &child) != VALUE_ERR_INVALID)
                return 113;
        if (child)
                return 114;

        endpoint_fini(&endpoint);
        eaxcid_fini(&eaxcid);
        return 0;
}

static int test_eaxcid_merge_diff(void)
{
        struct eaxcid base;
        struct eaxcid overlay;
        uint64_t diff;

        eaxcid_init(&base, "base");
        eaxcid_init(&overlay, "overlay");

        base.o_du_port_bitmask = 0x0001;
        base.band_sector_bitmask = 0x0002;
        base.id = 0x0001;
        eaxcid_mark_present(&base, EAXCID_ODU_PORT_BITMASK);
        eaxcid_mark_present(&base, EAXCID_BAND_SECTOR_BITMASK);
        eaxcid_mark_present(&base, EAXCID_ID);
        eaxcid_meta_accept(&base);

        overlay.band_sector_bitmask = 0x0002;
        overlay.ru_port_bitmask = 0x000f;
        overlay.id = 0x0003;
        eaxcid_mark_present(&overlay, EAXCID_BAND_SECTOR_BITMASK);
        eaxcid_mark_present(&overlay, EAXCID_RU_PORT_BITMASK);
        eaxcid_mark_present(&overlay, EAXCID_ID);

        diff = eaxcid_diff_present_fields(&base, &overlay);
        if ((diff & EAXCID_ODU_PORT_BITMASK) == 0)
                return 115;
        if (diff & EAXCID_BAND_SECTOR_BITMASK)
                return 116;
        if ((diff & EAXCID_RU_PORT_BITMASK) == 0)
                return 117;
        if ((diff & EAXCID_ID) == 0)
                return 118;

        if (eaxcid_merge_present(&base, &overlay) != VALUE_OK)
                return 119;
        if (base.o_du_port_bitmask != 0x0001)
                return 120;
        if (base.band_sector_bitmask != 0x0002)
                return 121;
        if (base.ru_port_bitmask != 0x000f)
                return 122;
        if (base.id != 0x0003)
                return 123;
        if (!eaxcid_has_field(&base, EAXCID_RU_PORT_BITMASK))
                return 124;
        if ((eaxcid_dirty_fields(&base) & EAXCID_ID) == 0)
                return 125;
        if (eaxcid_dirty_fields(&base) & EAXCID_BAND_SECTOR_BITMASK)
                return 126;
        if (eaxcid_diff_present_fields(&base, &overlay) !=
            EAXCID_ODU_PORT_BITMASK)
                return 127;

        eaxcid_fini(&overlay);
        eaxcid_fini(&base);
        return 0;
}

static int test_nokey_list_binding(void)
{
        LIST_HEAD(events);
        LIST_HEAD(view);
        struct sample_event *first, *second, *third;
        struct sample_event_walk walk = { 0 };
        int sum = 0;
        int threshold = 20;

        first = new_sample_event(30);
        second = new_sample_event(10);
        third = new_sample_event(20);
        if (!first || !second || !third)
                return 141;

        sample_event_add_tail_unlocked(&events, first);
        sample_event_add_tail_unlocked(&events, second);
        sample_event_add_tail_unlocked(&events, third);

        sample_event_foreach_unlocked(&events, sample_event_count_cb, &walk);
        if (walk.count != 3 || walk.sum != 60)
                return 142;

        sample_event_reduce_unlocked(&events, sample_event_sum_cb, NULL, &sum);
        if (sum != 60)
                return 143;

        if (sample_event_filter_view_unlocked(
                    &view, &events, sample_event_large_cb,
                    &threshold) != VALUE_OK)
                return 144;
        if (count_filters(&view) != 2)
                return 145;
        value_filter_view_free(&view);

        sample_event_sort_unlocked(&events, sample_event_cmp_cb, NULL);
        if (list_entry(events.next, struct sample_event, list) != second)
                return 146;
        if (list_entry(events.prev, struct sample_event, list) != first)
                return 147;

        sample_event_free_list_unlocked(&events);
        return 0;
}

static int test_bindings(void)
{
        struct eaxcid e;
        struct eaxcid clone;
        struct carrier carrier;

        if (eaxcid_object_ops.size != sizeof(struct eaxcid))
                return 50;
        if (eaxcid_meta_ops.meta_offset != offsetof(struct eaxcid, meta))
                return 51;
        if (strcmp(eaxcid_field_name(EAXCID_ID), "eaxc-id"))
                return 52;

        eaxcid_object_ops.init(&e, "bound-eaxcid");
        eaxcid_object_ops.init(&clone, NULL);
        eaxcid_meta_mark_present(&e, EAXCID_ODU_PORT_BITMASK);
        eaxcid_meta_mark_present(&e, EAXCID_BAND_SECTOR_BITMASK);
        eaxcid_meta_mark_present(&e, EAXCID_CCID_BITMASK);
        eaxcid_set_ru_port_bitmask(&e, 0x000f);
        eaxcid_set_id(&e, 0x0007);

        if (!eaxcid_meta_has(&e, EAXCID_ID))
                return 53;
        if (!eaxcid_can_use_field(&e, EAXCID_ID))
                return 54;
        if (eaxcid_missing_required(&e) != 0)
                return 64;
        if (!eaxcid_meta_complete(&e))
                return 65;
        if (!eaxcid_meta_ready(&e))
                return 132;
        if (!eaxcid_meta_has_all(&e, EAXCID_ALL_FIELDS))
                return 133;
        if (!eaxcid_meta_has_any(&e, EAXCID_ID))
                return 134;
        if (eaxcid_object_ops.validate(&e) != VALUE_OK)
                return 55;
        if (eaxcid_object_ops.copy(&clone, &e) != VALUE_OK)
                return 56;
        if (!eaxcid_equal(&e, &clone))
                return 57;

        eaxcid_meta_mark_dirty(&e, EAXCID_ID);
        if (!eaxcid_is_dirty(&e))
                return 58;
        if (eaxcid_dirty_fields(&e) != EAXCID_ID)
                return 66;
        if (!eaxcid_meta_dirty_any(&e, EAXCID_ID))
                return 135;
        eaxcid_meta_mark_invalid(&e, EAXCID_ID);
        if (!eaxcid_meta_invalid_any(&e, EAXCID_ID))
                return 136;
        if (eaxcid_meta_ready(&e))
                return 137;
        eaxcid_meta_clear_invalid(&e, EAXCID_ID);
        if (!eaxcid_meta_ready(&e))
                return 138;
        eaxcid_meta_accept(&e);
        if (eaxcid_is_dirty(&e))
                return 59;
        eaxcid_clear_field(&e, EAXCID_ID);
        if (eaxcid_has_field(&e, EAXCID_ID))
                return 67;
        if ((eaxcid_missing_required(&e) & EAXCID_ID) == 0)
                return 68;
        eaxcid_meta_mark_present_dirty(&e, EAXCID_ID);
        if (!eaxcid_meta_has(&e, EAXCID_ID))
                return 139;
        if (!eaxcid_meta_dirty_any(&e, EAXCID_ID))
                return 140;

        carrier_object_ops.init(&carrier, "bound-carrier");
        carrier_meta_mark_present(&carrier, CARRIER_ACTIVE);
        if (carrier_meta(&carrier) != &carrier.meta)
                return 60;
        if (!carrier_meta_has(&carrier, CARRIER_ACTIVE))
                return 61;
        if (carrier_meta_complete(&carrier))
                return 69;
        carrier_clear_field(&carrier, CARRIER_ACTIVE);
        if (carrier_has_field(&carrier, CARRIER_ACTIVE))
                return 102;
        if (strcmp(carrier_field_name(CARRIER_GAIN), "gain"))
                return 62;
        carrier_object_ops.fini(&carrier);

        if (element_list_ops.size != sizeof(struct element) ||
            endpoint_list_ops.size != sizeof(struct endpoint) ||
            link_list_ops.size != sizeof(struct link))
                return 63;

        eaxcid_object_ops.fini(&e);
        eaxcid_object_ops.fini(&clone);
        return 0;
}

int t_ru_scenarios_run(void)
{
        int rc;

        rc = test_eaxcid_value();
        if (rc)
                return rc;

        rc = test_endpoint_embed();
        if (rc)
                return rc;

        rc = test_link_resolve_and_update();
        if (rc)
                return rc;

        rc = test_multi_ru_shape();
        if (rc)
                return rc;

        rc = test_list_lookup();
        if (rc)
                return rc;

        rc = test_list_functional_wrappers();
        if (rc)
                return rc;

        rc = test_safe_getters();
        if (rc)
                return rc;

        rc = test_eaxcid_merge_diff();
        if (rc)
                return rc;

        rc = test_nokey_list_binding();
        if (rc)
                return rc;

        rc = test_bindings();
        if (rc)
                return rc;

	printf("t_ru_value_model: PASS\n");
        return 0;
}
