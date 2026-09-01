/*
 * Test-local RU value declarations built on top of value_base.
 */

#ifndef T_RU_VALUE_FIXTURE_H
#define T_RU_VALUE_FIXTURE_H

#include <stdbool.h>
#include <stdint.h>

#include <value_base.h>

enum direction {
        TX = 0,
        RX,
};

enum eaxcid_field {
        EAXCID_ODU_PORT_BITMASK    = (1ULL << 0),
        EAXCID_BAND_SECTOR_BITMASK = (1ULL << 1),
        EAXCID_CCID_BITMASK        = (1ULL << 2),
        EAXCID_RU_PORT_BITMASK     = (1ULL << 3),
        EAXCID_ID                  = (1ULL << 4),

        EAXCID_ALL_FIELDS          = EAXCID_ODU_PORT_BITMASK |
                                     EAXCID_BAND_SECTOR_BITMASK |
                                     EAXCID_CCID_BITMASK |
                                     EAXCID_RU_PORT_BITMASK |
                                     EAXCID_ID,
};

struct eaxcid {
        char *name;
        uint16_t o_du_port_bitmask;
        uint16_t band_sector_bitmask;
        uint16_t ccid_bitmask;
        uint16_t ru_port_bitmask;
        uint16_t id;
        struct value_config meta;
        struct list_head list;
};

enum element_field {
        ELEMENT_TRANSPORT_INTERFACE = (1ULL << 0),
        ELEMENT_ALL_FIELDS          = ELEMENT_TRANSPORT_INTERFACE,
};

struct element {
        char *name;
        char *transport_interface;
        struct value_config meta;
        struct list_head list;
};

enum carrier_field {
        CARRIER_ABSOLUTE_FREQUENCY = (1ULL << 0),
        CARRIER_CENTER_BANDWIDTH   = (1ULL << 1),
        CARRIER_CHANNEL_BANDWIDTH  = (1ULL << 2),
        CARRIER_GAIN               = (1ULL << 3),
        CARRIER_ACTIVE             = (1ULL << 4),

        CARRIER_ALL_FIELDS         = CARRIER_ABSOLUTE_FREQUENCY |
                                     CARRIER_CENTER_BANDWIDTH |
                                     CARRIER_CHANNEL_BANDWIDTH |
                                     CARRIER_GAIN |
                                     CARRIER_ACTIVE,
};

struct carrier {
        char *name;
        enum direction dir;
        unsigned long absolute_frequency_center;
        unsigned long center_of_channel_bandwidth;
        unsigned long channel_bandwidth;
        float gain;
        bool active;
        struct value_config meta;
        struct list_head list;
};

enum endpoint_field {
        ENDPOINT_EAXCID         = (1ULL << 0),
        ENDPOINT_TYPE           = (1ULL << 1),
        ENDPOINT_PRB            = (1ULL << 2),
        ENDPOINT_DELAY_MANAGED  = (1ULL << 3),

        ENDPOINT_ALL_FIELDS     = ENDPOINT_EAXCID |
                                  ENDPOINT_TYPE |
                                  ENDPOINT_PRB,
};

struct endpoint {
        char *name;
        enum direction dir;
        struct eaxcid eaxcid;
        uint8_t endpoint_type;
        uint16_t number_of_prb;
        bool non_time_managed_delay_enabled;
        struct value_config meta;
        struct list_head list;
};

struct link {
        char *name;
        struct {
                char *name;
                struct element *value;
        } element;
        struct {
                char *name;
                struct carrier *value;
        } carrier;
        struct {
                char *name;
                struct endpoint *value;
        } endpoint;
        struct value_config meta;
        struct list_head list;
};

struct ru_values {
        struct list_head elements;
        struct list_head tx_carriers;
        struct list_head rx_carriers;
        struct list_head tx_endpoints;
        struct list_head rx_endpoints;
        struct list_head tx_links;
        struct list_head rx_links;
};

extern const struct value_object_ops eaxcid_object_ops;
extern const struct value_meta_ops eaxcid_meta_ops;
extern const struct value_list_ops eaxcid_list_ops;
extern const struct value_object_ops element_object_ops;
extern const struct value_meta_ops element_meta_ops;
extern const struct value_list_ops element_list_ops;
extern const struct value_object_ops carrier_object_ops;
extern const struct value_meta_ops carrier_meta_ops;
extern const struct value_list_ops carrier_list_ops;
extern const struct value_object_ops endpoint_object_ops;
extern const struct value_meta_ops endpoint_meta_ops;
extern const struct value_list_ops endpoint_list_ops;
extern const struct value_object_ops link_object_ops;
extern const struct value_meta_ops link_meta_ops;
extern const struct value_list_ops link_list_ops;

void eaxcid_init(struct eaxcid *e, const char *name);
struct eaxcid *new_eaxcid(const char *name);
void eaxcid_unlink(struct eaxcid *e);
VALUE_OBJECT_WRAPPERS_HEADER(eaxcid);
void eaxcid_fini(struct eaxcid *e);
int eaxcid_copy(struct eaxcid *dst, const struct eaxcid *src);
bool eaxcid_equal(const struct eaxcid *a, const struct eaxcid *b);
bool eaxcid_is_complete(const struct eaxcid *e);
int eaxcid_set_id(struct eaxcid *e, uint16_t id);
int eaxcid_get_id(const struct eaxcid *e, uint16_t *id);
int eaxcid_set_ru_port_bitmask(struct eaxcid *e, uint16_t mask);
uint64_t eaxcid_diff_present_fields(
        const struct eaxcid *a, const struct eaxcid *b);
int eaxcid_merge_present(struct eaxcid *dst, const struct eaxcid *src);
int eaxcid_validate(struct eaxcid *e);
void eaxcid_free(struct eaxcid *e);
struct eaxcid *eaxcid_lookup_id_unlocked(struct list_head *head, uint16_t id);
void eaxcid_add_tail_unlocked(struct list_head *head, struct eaxcid *value);
void eaxcid_foreach_unlocked(
        struct list_head *head,
        void (*fn)(struct eaxcid *value, void *ctx), void *ctx);
void *eaxcid_reduce_unlocked(
        struct list_head *head,
        void *(*fn)(void *acc, struct eaxcid *value, void *ctx),
        void *ctx, void *acc);
int eaxcid_filter_view_unlocked(
        struct list_head *out, struct list_head *in,
        bool (*pred)(const struct eaxcid *value, void *ctx), void *ctx);
void eaxcid_free_list_unlocked(struct list_head *head);
struct value_config *eaxcid_meta(struct eaxcid *value);
const struct value_config *eaxcid_meta_const(const struct eaxcid *value);
bool eaxcid_has_field(const struct eaxcid *value, uint64_t field);
bool eaxcid_meta_has(const struct eaxcid *value, uint64_t field);
bool eaxcid_can_use_field(const struct eaxcid *value, uint64_t field);
bool eaxcid_meta_ready(const struct eaxcid *value);
bool eaxcid_meta_has_any(const struct eaxcid *value, uint64_t fields);
bool eaxcid_meta_has_all(const struct eaxcid *value, uint64_t fields);
bool eaxcid_meta_dirty_any(const struct eaxcid *value, uint64_t fields);
bool eaxcid_meta_invalid_any(const struct eaxcid *value, uint64_t fields);
uint64_t eaxcid_missing_required(const struct eaxcid *value);
bool eaxcid_meta_is_complete(const struct eaxcid *value);
bool eaxcid_meta_complete(const struct eaxcid *value);
uint64_t eaxcid_dirty_fields(const struct eaxcid *value);
bool eaxcid_is_dirty(const struct eaxcid *value);
void eaxcid_mark_present(struct eaxcid *value, uint64_t field);
void eaxcid_meta_mark_present(struct eaxcid *value, uint64_t field);
void eaxcid_mark_dirty(struct eaxcid *value, uint64_t field);
void eaxcid_meta_mark_dirty(struct eaxcid *value, uint64_t field);
void eaxcid_meta_mark_present_dirty(struct eaxcid *value, uint64_t fields);
void eaxcid_meta_mark_invalid(struct eaxcid *value, uint64_t fields);
void eaxcid_meta_clear_invalid(struct eaxcid *value, uint64_t fields);
void eaxcid_meta_accept(struct eaxcid *value);
void eaxcid_clear_field(struct eaxcid *value, uint64_t field);
const char *eaxcid_field_name(uint64_t field);

void element_init(struct element *e, const char *name);
struct element *new_element(const char *name);
VALUE_OBJECT_WRAPPERS_HEADER(element);
void element_fini(struct element *e);
int element_copy(struct element *dst, const struct element *src);
void element_free(struct element *e);
struct element *element_lookup_unlocked(struct list_head *head, const char *key);
struct element *element_lookup_move_front_unlocked(
        struct list_head *head, const char *key);
void element_add_tail_unlocked(struct list_head *head, struct element *value);
void element_free_list_unlocked(struct list_head *head);

void carrier_init(struct carrier *c, const char *name, enum direction dir);
struct carrier *new_carrier(const char *name, enum direction dir);
VALUE_OBJECT_WRAPPERS_HEADER(carrier);
void carrier_fini(struct carrier *c);
void carrier_free(struct carrier *c);
int carrier_set_center_bandwidth(struct carrier *c, unsigned long bandwidth);
int carrier_set_channel_bandwidth(struct carrier *c, unsigned long bandwidth);
int carrier_set_gain(struct carrier *c, float gain);
int carrier_copy(struct carrier *dst, const struct carrier *src);
int carrier_validate(struct carrier *c);
void carrier_accept(struct carrier *c);
struct carrier *carrier_lookup_unlocked(struct list_head *head, const char *key);
struct carrier *carrier_lookup_move_front_unlocked(
        struct list_head *head, const char *key);
void carrier_add_tail_unlocked(struct list_head *head, struct carrier *value);
void carrier_foreach_unlocked(
        struct list_head *head,
        void (*fn)(struct carrier *value, void *ctx), void *ctx);
void *carrier_reduce_unlocked(
        struct list_head *head,
        void *(*fn)(void *acc, struct carrier *value, void *ctx),
        void *ctx, void *acc);
int carrier_filter_view_unlocked(
        struct list_head *out, struct list_head *in,
        bool (*pred)(const struct carrier *value, void *ctx), void *ctx);
void carrier_free_list_unlocked(struct list_head *head);
struct value_config *carrier_meta(struct carrier *value);
bool carrier_has_field(const struct carrier *value, uint64_t field);
bool carrier_meta_has(const struct carrier *value, uint64_t field);
uint64_t carrier_missing_required(const struct carrier *value);
bool carrier_meta_is_complete(const struct carrier *value);
bool carrier_meta_complete(const struct carrier *value);
uint64_t carrier_dirty_fields(const struct carrier *value);
bool carrier_is_dirty(const struct carrier *value);
void carrier_mark_present(struct carrier *value, uint64_t field);
void carrier_meta_mark_present(struct carrier *value, uint64_t field);
void carrier_meta_mark_dirty(struct carrier *value, uint64_t field);
void carrier_clear_field(struct carrier *value, uint64_t field);
const char *carrier_field_name(uint64_t field);

void endpoint_init(struct endpoint *ep, const char *name, enum direction dir);
struct endpoint *new_endpoint(const char *name, enum direction dir);
VALUE_OBJECT_WRAPPERS_HEADER(endpoint);
void endpoint_fini(struct endpoint *ep);
void endpoint_free(struct endpoint *ep);
int endpoint_set_eaxcid(struct endpoint *ep, const struct eaxcid *eaxcid);
int endpoint_get_eaxcid(
        const struct endpoint *ep, const struct eaxcid **eaxcid);
int endpoint_set_eaxcid_id(struct endpoint *ep, uint16_t id);
bool endpoint_has_complete_eaxcid(const struct endpoint *ep);
int endpoint_set_prb(struct endpoint *ep, uint16_t prb);
int endpoint_copy(struct endpoint *dst, const struct endpoint *src);
int endpoint_validate(struct endpoint *ep);
void endpoint_accept(struct endpoint *ep);
struct endpoint *endpoint_lookup_unlocked(
        struct list_head *head, const char *key);
struct endpoint *endpoint_lookup_move_front_unlocked(
        struct list_head *head, const char *key);
void endpoint_add_tail_unlocked(struct list_head *head, struct endpoint *value);
void endpoint_foreach_unlocked(
        struct list_head *head,
        void (*fn)(struct endpoint *value, void *ctx), void *ctx);
void *endpoint_reduce_unlocked(
        struct list_head *head,
        void *(*fn)(void *acc, struct endpoint *value, void *ctx),
        void *ctx, void *acc);
void endpoint_sort_unlocked(
        struct list_head *head,
        int (*fn)(struct endpoint *a, struct endpoint *b, void *ctx),
        void *ctx);
int endpoint_filter_view_unlocked(
        struct list_head *out, struct list_head *in,
        bool (*pred)(const struct endpoint *value, void *ctx), void *ctx);
void endpoint_free_list_unlocked(struct list_head *head);

void link_init(struct link *l, const char *name);
struct link *new_link(const char *name);
VALUE_OBJECT_WRAPPERS_HEADER(link);
void link_fini(struct link *l);
int link_copy(struct link *dst, const struct link *src);
void link_free(struct link *l);
int link_resolve_unlocked(
        struct link *l, struct list_head *elements,
        struct list_head *carriers, struct list_head *endpoints);
bool link_is_resolved(const struct link *l);
int link_validate(struct link *l);
struct link *link_lookup_unlocked(struct list_head *head, const char *key);
struct link *link_lookup_move_front_unlocked(
        struct list_head *head, const char *key);
void link_add_tail_unlocked(struct list_head *head, struct link *value);
void link_foreach_unlocked(
        struct list_head *head,
        void (*fn)(struct link *value, void *ctx), void *ctx);
void *link_reduce_unlocked(
        struct list_head *head,
        void *(*fn)(void *acc, struct link *value, void *ctx),
        void *ctx, void *acc);
int link_filter_view_unlocked(
        struct list_head *out, struct list_head *in,
        bool (*pred)(const struct link *value, void *ctx), void *ctx);
void link_free_list_unlocked(struct list_head *head);

void ru_values_init(struct ru_values *ru);
int ru_values_resolve_unlocked(struct ru_values *ru);
int link_update_bandwidth(struct link *l, unsigned long bandwidth);
int link_update_gain(struct link *l, float gain);
int link_update_eaxcid_id(struct link *l, uint16_t id);

#endif /* T_RU_VALUE_FIXTURE_H */
