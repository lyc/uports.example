#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "list.h"
#include "hash.h"

#ifndef NOT_USED
#define NOT_USED(a) (void)(a)
#endif

int main(int argc, char **argv);
void hlist_demo(void);

/* ------------------------------------------------------------------------- */
/*                                                                           */
/* ------------------------------------------------------------------------- */

enum notify_id {
        NT_UNKNOWN = -1,

        /* real notification */
        NT_SYNCHRONIZATION_STATE_CHANGE,   /* synchronization-state-change */
        NT_PTP_STATE_CHANGE,               /* ptp-state-change */
        NT_ALARM_NOTIF,                    /* alarm-notif */
        NT_SUPERVISION_NOTIFICATION,       /* supervision-notification */
        NT_MEASUREMENT_RESULT_STATS,       /* measurement-result-stats */
        NT_TX_ARRAY_CARRIERS_STATE_CHANGE, /* tx-array-carriers-state-change */
        NT_RX_ARRAY_CARRIERS_STATE_CHANGE, /* rx-array-carriers-state-change */
        NT_DOWNLOAD_EVENT,                 /* download-event */
        NT_INSTALL_EVENT,                  /* install-event */
        NT_ACTIVATE_EVENT,                 /* activate-event */
        NT_NETCONF_CONFIG_CHANGE,          /* netconf-config-change */
        NT_NETCONF_SESSION_START,          /* netconf-session-start */
        NT_NETCONF_SESSION_END,            /* netconf-session-end */

        NT_FAKE_START,

        /* fake notification */
#if 0
        NT_RADIO_PARAMETERS_GET,           /* radio-parameters-get */
#endif
        NT_RADIO_PARAMETERS_SET,           /* radio-parameters-set */
        NT_PTP_STATE_CHECK,                /* ptp-state-check */

        NT_MAXIMUM,
};

#define NOTIF_ID_SLOT    ((((NT_MAXIMUM)+7)/8)*8)
#define NOTIF_HASH_SLOT  (NOTIF_ID_SLOT*3)
#define NOTIF_SN_MAXIMUM 30000

struct node {
        struct {
                int id;
                int sn;
        } nstats[NOTIF_ID_SLOT];
};


int node_notify_stat_init(struct node *node)
{
        int i;

        for (i=0; i<NOTIF_ID_SLOT; i++) {
                node->nstats[i].id = -1;
                node->nstats[i].sn = 0;
        }
        return 0;
}

int node_notify_stat_lookup(struct node *node, int nid)
{
        int i;

        for (i=0; i<NOTIF_ID_SLOT; i++) {
                if (node->nstats[i].id == -1)
                        break;
                if (node->nstats[i].id == nid)
                        break;
        }

        /* full space */
        if (i >= NOTIF_ID_SLOT)
                return -1;

        if (node->nstats[i].id == -1) {
                node->nstats[i].id = nid;
                printf("new incoming: ns[%d] = (%d,%d)...\n",
                       i, node->nstats[i].id, node->nstats[i].sn);
        }

        node->nstats[i].sn += 1;
        return node->nstats[i].sn;
}

struct notify_info {
        int id;
        char *name;
        int sn;
};

typedef unsigned int (*hash_helper)(char *str);

struct notify_head {
        hash_helper func;
        int size;
        struct hlist_head *hash;
        struct list_head list;
};

struct notify_entry {
        struct list_head l_list;
        struct hlist_node l_hash;
        struct notify_info *info;
};

struct notify_head *notify_head_create(int size, hash_helper func)
{
        int i;
        struct notify_head *nh;

        nh = calloc(1, sizeof(struct notify_head));
        if (!nh)
                return NULL;

        nh->func = func;
        nh->size = size;
        nh->hash = calloc(size, sizeof(struct notify_info));
        if (!nh->hash) {
                free(nh);
                return NULL;
        }

        INIT_LIST_HEAD(&nh->list);
        for (i=0; i<nh->size; i++)
                INIT_HLIST_HEAD(&nh->hash[i]);

        return nh;
}

void notify_head_destroy(struct notify_head *nh)
{
        struct list_head *p, *n;
        struct notify_entry *ne;

        if (nh) {
                list_for_each_safe(p, n, &nh->list) {
                        ne = list_entry(p, struct notify_entry, l_list);
                        list_del(p);
                        free(ne);
                }

                if (nh->hash)
                        free(nh->hash);
                free(nh);
        }
}

int notify_head_setup(struct notify_head *nh, struct notify_info *info)
{
        int i;
        unsigned int h;
        struct notify_info *ni;
        struct notify_entry *ne;

        for (i=0; i<NT_MAXIMUM; i++) {
                ni = &info[i];
                if (ni->id == NT_FAKE_START)
                        continue;
                ne = calloc(1, sizeof(struct notify_entry));
                if (ne == NULL) {
                        printf("Oops, fail to allocate notify_entry...\n");
                        exit(-1);
                }

                ne->info = ni;
                list_add_tail(&ne->l_list, &nh->list);
                h = (nh->func)(ni->name) % nh->size;
                hlist_add_head(&ne->l_hash, &nh->hash[h]);
                printf("===> hash(%s) = %d\n", ni->name, h);
        }
        return 0;
}

struct notify_info *notify_head_lookup(struct notify_head *nh, char *notif)
{
        unsigned int h;
        struct notify_entry *ne;
        struct hlist_node *ph;

        h = (nh->func)(notif) % nh->size;
        hlist_for_each(ph, &nh->hash[h]) {
                ne = hlist_entry(ph, struct notify_entry, l_hash);
                if (!strcmp(notif, ne->info->name))
                        return ne->info;
        }
        return NULL;
}

struct notify_info notify_table[NT_MAXIMUM] = {
        { NT_SYNCHRONIZATION_STATE_CHANGE,   "synchronization-state-change", 0 },
        { NT_PTP_STATE_CHANGE,               "ptp-state-change",             0 },
        { NT_ALARM_NOTIF,                    "alarm-notif",                  0 },
        { NT_SUPERVISION_NOTIFICATION,       "supervision-notification",     0 },
        { NT_MEASUREMENT_RESULT_STATS,       "measurement-result-stats",     0 },
        { NT_TX_ARRAY_CARRIERS_STATE_CHANGE, "tx-array-carriers-state-change", 0 },
        { NT_RX_ARRAY_CARRIERS_STATE_CHANGE, "rx-array-carriers-state-change", 0 },
        { NT_DOWNLOAD_EVENT,                 "download-event",               0 },
        { NT_INSTALL_EVENT,                  "install-event",                0 },
        { NT_ACTIVATE_EVENT,                 "activate-event",               0 },
        { NT_NETCONF_CONFIG_CHANGE,          "netconf-config-change",        0 },
        { NT_NETCONF_SESSION_START,          "netconf-session-start",        0 },
        { NT_NETCONF_SESSION_END,            "netconf-session-end",          0 },

        { NT_FAKE_START,                     "",                             0 },

#if 0
        { NT_RADIO_PARAMETERS_GET,           "radio-parameters-get",         0 },
#endif
        { NT_RADIO_PARAMETERS_SET,           "radio-parameters-set",         0 },
        { NT_PTP_STATE_CHECK,                "ptp-state-check",              0 },
};

struct notify {
        struct node *node;
        struct notify_info *notify;
        int sn;
};

int notify_demo()
{
        struct node node;
        struct notify_head *nh;
        struct notify_info *ni;
        uint32_t sn = 0xffffffff;

        int i, ns;
        char *msg[] = {
                "alarm-notif",
                "tx-array-carriers-state-change",
                "download-event",
                "alarm-notif",
                "ABCDE",
                "supervision-notification",
                "supervision-notification",
        };

        printf("sizeof(unsigned int) = %ld\n", sizeof(unsigned int));
        printf("sizeof(uint32_t) = %ld\n", sizeof(uint32_t));
        printf("sn = %08x\n", sn);
        sn++;
        printf("sn = %08x\n", sn);
        sn++;
        printf("sn = %08x\n", sn);
        printf("\n");

        printf("NOTIF_ID_SLOT = %d\n", NOTIF_ID_SLOT);
        printf("NOTIF_HASH_SLOT = %d\n", NOTIF_HASH_SLOT);
        printf("NOTIF_SN_MAXIMUM = %d\n", NOTIF_SN_MAXIMUM);
        printf("NT_SYNCHRONIZATION_STATE_CHANGE = %d\n",
               NT_SYNCHRONIZATION_STATE_CHANGE);
        printf("NT_FAKE_START = %d\n", NT_FAKE_START);
        printf("NT_MAXIMUM = %d\n", NT_MAXIMUM);
        printf("notif[NT_SYNCHRONIZATION_STATE_CHANGE].name = %s\n",
               notify_table[NT_SYNCHRONIZATION_STATE_CHANGE].name);


        printf("\n");

        nh = notify_head_create(NOTIF_HASH_SLOT, simple_hash);
        if (!nh)
                return -1;
        notify_head_setup(nh, (struct notify_info *)&notify_table);

        node_notify_stat_init(&node);

        for (i=0; i<(int)(sizeof(msg)/sizeof(char*)); i++) {
                ni = notify_head_lookup(nh, msg[i]);
                if (ni) {
                        printf("id(%s) = %d\n", msg[i], ni->id);

                        ns = node_notify_stat_lookup(&node, ni->id);
                        if (ns >= 0)
                                printf("stat(%d) = %d\n", ni->id, ns);
                }
        }

        notify_head_destroy(nh);
        return 0;
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/* ------------------------------------------------------------------------- */

typedef int (*http_header_op)(char *name, void* data);

struct http_header {
        char *name;
        http_header_op op;
};

int no_op(char *name, void *data)
{
        NOT_USED(data);

        printf("Oops, %s didn't support yet...\n", name);
        return 0;
}

int op_cookie(char *name, void *data)
{
        NOT_USED(name);
        NOT_USED(data);

        printf("op_cookie has been called...\n");
        return 0;
}


struct http_header http_header_table[] = {
        /* general-header */
        { "Cache-Control", no_op },
        { "Connection", no_op },
        { "Date", no_op },
        { "Pragma", no_op },
        { "Trailer", no_op },
        { "Transfer-Encoding", no_op },
        { "Upgrade", no_op },
        { "Via", no_op },
        { "Warning", no_op },

        /* request-header */
        { "Accept", no_op },
        { "Accept-Charset", no_op },
        { "Accept-Encoding", no_op },
        { "Accept-Language", no_op },
        { "Authorization", no_op },
        { "Expect", no_op },
        { "From", no_op },
        { "Host", no_op },
        { "If-Match", no_op },
        { "If-Modified-Since", no_op },
        { "If-None-Match", no_op },
        { "If-Range", no_op },
        { "If-Unmodified-Since", no_op },
        { "Max-Forwards", no_op },
        { "Proxy-Authorization", no_op },
        { "Range", no_op },
        { "Referer", no_op },
        { "TE", no_op },
        { "User-Agent", no_op },


        /* response-header */
        { "Accept-Ranges", no_op },
        { "Age", no_op },
        { "ETag", no_op },
        { "Location", no_op },
        { "Proxy-Authentiate", no_op },
        { "Retry-After", no_op },
        { "Server", no_op },
        { "Vary", no_op },
        { "WWW-Authenticate", no_op },

        /* entity-header */
        { "Content-Encoding", no_op },
        { "Content-Language", no_op },
        { "Content-Length", no_op },
        { "Content-Location", no_op },
        { "Content-MD5", no_op },
        { "Content-Range", no_op },
        { "Content-Type", no_op },
        { "Expires", no_op },
        { "Last-Modified", no_op },

        /* cookie-header */
        { "Set-Cookie", no_op },
        { "Cookie", op_cookie },
};

void show_hash_table(char *msg[], unsigned int nr,
                     unsigned int *table, unsigned int size,
                     char *name, unsigned int (*function)(char*))
{
        unsigned int i, h;
        unsigned one, two;

        for (i=0; i<size; i++)
                table[i] = 0;

        for (i=0; i<nr; i++) {
                h = (function)(msg[i]) % size;
                table[h]++;
        }

        for (i=0, one=0, two=0; i<size; i++)
                if (table[i] == 1) {
                        one++;
                } else if (table[i] >= 2) {
                        two++;
                }

        printf("\nhash function: %s (%d, %d, %d)\n", name, size, one, two);
        for (i=0; i<size;) {
                if (i%32 == 0)
                        printf("%04d: ", i);
                printf("%02d ", table[i]);
                i++;
                if (i%32 == 0)
                        printf("\n");
        }
}

void try_hash(struct http_header *table, unsigned int nr, unsigned int size)
{
        unsigned int i, *htable;
        char **ntable;

        static struct {
                char *name;
                unsigned int (*function)(char *);
        } hash_function_table[] = {
                { "simple_hash", simple_hash },
                { "RS_hash", RS_hash },
                { "JS_hash", JS_hash },
                { "PJW_hash", PJW_hash },
                { "ELF_hash", ELF_hash },
                { "BKDR_hash", BKDR_hash },
                { "SDBM_hash", SDBM_hash },
                { "DJB_hash", DJB_hash },
                { "AP_hash", AP_hash },
                { "CRC_hash", CRC_hash },
                { NULL, NULL }
        };

        ntable = (char **)malloc(sizeof(char*)*nr);
        if (ntable == NULL) {
                printf("Oops, no available memory for ntable...\n");
                exit(-1);
        }
        for (i=0; i<nr; i++)
                ntable[i] = table[i].name;

        htable = (unsigned int*)malloc (sizeof(unsigned int)*size);
        if (htable == NULL) {
                printf("Oops, no available memory for htable...\n");
                exit(-1);
        }

        for (i=0; hash_function_table[i].name; i++)
                show_hash_table(
                        ntable, nr, htable, size,
                        hash_function_table[i].name,
                        hash_function_table[i].function);

        free(htable);
        free(ntable);
}

struct B {
        struct list_head i_list;
        struct hlist_node i_hash;
        struct http_header *i_header;
};

#define HASH_SIZE 256

struct list_head hlist;
struct hlist_head hhash[HASH_SIZE];

void hlist_demo()
{
        int i, nr = sizeof(http_header_table)/sizeof(struct http_header);
        struct B *pB;
        struct list_head *pl, *pn;
        unsigned int h;
        struct hlist_node *ph;

        char *msg[] = {
                "Transfer-Encoding",
                "Cookie",
                "ABCDE",
                "Content-Encoding",
        };

        printf("conut of http header is: %d\n", nr);
        try_hash(http_header_table, nr, HASH_SIZE);
        printf("\n");

        INIT_LIST_HEAD(&hlist);
        for (i=0; i<HASH_SIZE; i++)
                INIT_HLIST_HEAD(&hhash[i]);

        for (i=0; i<nr; i++) {
                pB = (struct B*) malloc(sizeof(struct B));
                if (pB == NULL) {
                        printf("Oops, no available memory for pB...\n");
                        exit(-1);
                }
                pB->i_header = &http_header_table[i];

                list_add_tail(&pB->i_list, &hlist);

                h = simple_hash(pB->i_header->name) % HASH_SIZE;
                hlist_add_head(&pB->i_hash, &hhash[h]);
        }

/*
        list_for_each(pl, &hlist) {
                pB = list_entry(pl, struct B, i_list);
                (pB->i_header->op)(pB->i_header->name, NULL);
        }
        printf("\n");
*/

        for (i=0; i<(int)(sizeof(msg)/sizeof(char*)); i++) {
                h = simple_hash(msg[i]) % HASH_SIZE;
/*
                if (hlist_empty(&hhash[h]))
                        continue;
*/
                hlist_for_each(ph, &hhash[h]) {
                        pB = hlist_entry(ph, struct B, i_hash);
                        if (!strcmp(msg[i], pB->i_header->name))
                                (pB->i_header->op)(pB->i_header->name, NULL);
                }
        }

        list_for_each_safe(pl, pn, &hlist) {
                pB = list_entry(pl, struct B, i_list);
                list_del(pl);
                free(pB);
        }
}


/* ------------------------------------------------------------------------- */
/*                                                                           */
/* ------------------------------------------------------------------------- */
#define PSDN_CONN_HASH

struct psdn_connection {
	struct list_head list;
#ifdef PSDN_CONN_HASH
	struct hlist_node hash;
#endif
	uint8_t dest[6];
	uint8_t gateway[6];
	uint16_t port;
	uint16_t vid;
	bool tag;
};

struct psdn_connection_head {
	struct list_head list;
#ifdef PSDN_CONN_HASH
	int hash_size;
	struct hlist_head *hash;
#endif
};

#ifdef PSDN_CONN_HASH
unsigned int my_simple_hash(uint8_t *str)
{
	int i;
	register uint8_t hash;
	register uint8_t *p;

	for(i = 0, hash = 0, p = (uint8_t *)str; i < 6; p++, i++)
	        hash = 31 * hash + *p;

	return (hash & 0x7FFFFFFF);
}
#endif

struct psdn_connection_head *create_psdn_connection_head(int size)
{
	struct psdn_connection_head *head;

	head = malloc(sizeof(struct psdn_connection_head));
	if (head == NULL)
		return NULL;

#ifdef PSDN_CONN_HASH
	head->hash_size = size;
	head->hash = malloc(sizeof(struct hlist_head) * size);
#else
	NOT_USED(size);
#endif
	INIT_LIST_HEAD(&head->list);
	return head;
}

int destroy_psdn_connection_head(struct psdn_connection_head *head)
{
	struct list_head *p, *pn;
	struct psdn_connection *c;

	if (head == NULL)
		return -1;

        list_for_each_safe(p, pn, &head->list) {
                c = list_entry(p, struct psdn_connection, list);
                list_del(p);
                free(c);
        }

#ifdef PSDN_CONN_HASH
	if (head->hash)
		free(head->hash);
#endif
	free(head);
	return 0;
}

struct psdn_connection *psdn_connection_find(
	struct psdn_connection_head *head, uint8_t *mac)
{
	struct psdn_connection *c;
#ifdef PSDN_CONN_HASH
	struct hlist_node *ph;
	unsigned int h;

	h = my_simple_hash(mac) % head->hash_size;

        hlist_for_each(ph, &head->hash[h]) {
                c = hlist_entry(ph, struct psdn_connection, hash);
#else
	struct list_head *p;
        list_for_each(p, &head->list) {
                c = hlist_entry(p, struct psdn_connection, list);
#endif
		if (memcmp(c->dest, mac, 6) == 0)
			return c;
	}
	return NULL;
}

struct psdn_connection *psdn_connection_add(
	struct psdn_connection_head *head, struct psdn_connection *src)
{
	struct 	psdn_connection *c;

	c = malloc(sizeof(struct psdn_connection));
	if (c == NULL)
		return NULL;

	memcpy(c->dest, src->dest, 6);
	memcpy(c->gateway, src->gateway, 6);

	list_add_tail(&c->list, &head->list);

#ifdef PSDN_CONN_HASH
	unsigned int h;
	h = my_simple_hash(c->dest) % head->hash_size;
	hlist_add_head(&c->hash, &head->hash[h]);
#endif
	return c;
}

struct psdn_connection *psdn_connection_del(
	struct psdn_connection_head *head, uint8_t *mac)
{
	struct psdn_connection *c;

	c = psdn_connection_find(head, mac);
	if(c){
#ifdef PSDN_CONN_HASH
		//list_del(&c->list);
		hlist_del(&c->hash);
#endif
		list_del(&c->list);
		free(c);
	}

	return c;
}

struct psdn_connection *psdn_connection_update(
	struct psdn_connection_head *head, struct psdn_connection *src)
{
	struct psdn_connection *c;

	c = psdn_connection_find(head, src->dest);
	if (c)
		memcpy(c->gateway, src->gateway, 6);
	else
		c = psdn_connection_add(head, src);
	return c;
}

uint8_t NO_MAC[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

void connection_dump(struct psdn_connection *c)
{
	printf("dest = %02x:%02x:%02x:%02x:%02x:%02x",
	       c->dest[0], c->dest[1], c->dest[2],
	       c->dest[3], c->dest[4], c->dest[5]);
	printf(", ");
	printf("gateway = %02x:%02x:%02x:%02x:%02x:%02x",
	       c->gateway[0], c->gateway[1], c->gateway[2],
	       c->gateway[3], c->gateway[4], c->gateway[5]);
	printf("\n");

}

void psdn_connection_dump(struct psdn_connection_head *head)
{
	struct psdn_connection *c;
	struct list_head *p;

	printf("<-------------------------------\n");
        list_for_each(p, &head->list) {
                c = list_entry(p, struct psdn_connection, list);
		connection_dump(c);
	}
	printf("------------------------------->\n");
}

int psdn_connection_test()
{
	struct A {
		uint8_t dest[6];
		uint8_t gateway[6];
	} tA[] = {
		{ { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc },
		  { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc } },
		{ { 0x00, 0x50, 0x40, 0x00, 0x93, 0x01 },
		  { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc } },
		{ { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc },
		  { 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff } },
		{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
		  { 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff } },
	};
	int i = 0;
	struct psdn_connection_head *head;
	struct psdn_connection src;

	head = create_psdn_connection_head(31);
	if (head == NULL)
		return -1;

	while (true) {
		if (memcmp(tA[i].dest, NO_MAC, 6) == 0)
			break;

		memcpy(src.dest, tA[i].dest, 6);
		memcpy(src.gateway, tA[i].gateway, 6);

		printf("\n");
		connection_dump(&src);
		psdn_connection_update(head, &src);
		psdn_connection_dump(head);
		i++;
	}

	printf("\ndelete:\n");
	psdn_connection_del(head, tA[1].dest);
	printf("dest = %02x:%02x:%02x:%02x:%02x:%02x",
	       tA[1].dest[0], tA[1].dest[1], tA[1].dest[2],
	       tA[1].dest[3], tA[1].dest[4], tA[1].dest[5]);
	printf(", ");
	printf("gateway = %02x:%02x:%02x:%02x:%02x:%02x",
	       tA[1].gateway[0], tA[1].gateway[1], tA[1].gateway[2],
	       tA[1].gateway[3], tA[1].gateway[4], tA[1].gateway[5]);
	printf("\n");

	psdn_connection_dump(head);

	destroy_psdn_connection_head(head);

	return 0;
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/* ------------------------------------------------------------------------- */

int main(int argc, char **argv)
{
        NOT_USED(argc);
        NOT_USED(argv);

//        printf("\nrunning hlist_demo...\n\n");
//        hlist_demo();

//        printf("\nrunning psdn_connection_test...\n\n");
//	psdn_connection_test();

        printf("\nrunning notif_demo...\n\n");
        notify_demo();

        return 0;
}
