#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "list.h"

#ifndef NOT_USED
#define NOT_USED(a) (void)(a)
#endif

int main(int argc, char **argv);
void list_demo(void);

/* ------------------------------------------------------------------------- */
/*                                                                           */
/* ------------------------------------------------------------------------- */
struct A {
        struct list_head _lh;
        int data;
};

LIST_HEAD(head);
struct list_head *p_cur;

int first(struct list_head *h)
{
        p_cur = h->next;
        if (p_cur == h)
                return -1;
        return container_of(p_cur, struct A, _lh)->data;
}

int next(struct list_head *h)
{
        p_cur = p_cur->next;
        if (p_cur == h) {
                p_cur = NULL;
                return -1;
        }
        return container_of(p_cur, struct A, _lh)->data;
}

void list_demo()
{
        int t;
        struct A *pA;
        struct list_head *p;
        struct list_head *n;

        pA = (struct A*)malloc(sizeof(struct A));
        pA->data = 10;
        list_add_tail(&pA->_lh, &head);

        list_for_each(p, &head) {
                pA = list_entry(p, struct A, _lh);
                printf("%d ", pA->data);
        }
        printf("\n");

        pA = (struct A*)malloc(sizeof(struct A));
        pA->data = 6;
        list_add_tail(&pA->_lh, &head);

        pA = (struct A*)malloc(sizeof(struct A));
        pA->data = 3;
        list_add_tail(&pA->_lh, &head);

        list_for_each_entry(pA, &head, _lh)
                printf("%d ", pA->data);
        printf("\n");

        t = first(&head);
        if (t != 1) {
                printf("==> (first) %d\n", t);
                while (1) {
                        t = next(&head);
                        if (t == -1)
                                break;
                        printf("==> (next) %d\n", t);
                }
        }

        list_for_each_safe(p, n, &head) {
                pA = list_entry(p, struct A, _lh);
                list_del(p);
                free(pA);
        }

        if (!list_empty(&head))
                printf("Oops! list is not empty\n");
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/* ------------------------------------------------------------------------- */

LIST_HEAD(t1);
int max_count;

struct string_entry {
        struct list_head list;
        char *key;
        char *value;
};

int list_cleanup(struct list_head *head)
{
        struct string_entry *pe;
        struct list_head *p;
        struct list_head *n;

        list_for_each_safe(p, n, head) {
                pe = list_entry(p, struct string_entry, list);
                list_del(p);
                if (pe->key)
                        free(pe->key);
                if (pe->value)
                        free(pe->value);
                free(pe);
        }
        return 0;
}

int list_dump(struct list_head *head)
{
        struct string_entry *pe;
        struct list_head *p;

        list_for_each(p, head) {
                pe = list_entry(p, struct string_entry, list);
                printf("(%s-%s) ", pe->key ? pe->key : "null", pe->value);
        }
        printf("\n");
        return 0;
}

int add_value(struct list_head *head, char *k, char *v)
{
        struct list_head* p;
        struct string_entry *pe;
        int updated;

        if (list_empty(head)) {
                /* new add ... */
                pe = calloc(1, sizeof(struct string_entry));
                if (max_count != 1)
                        pe->key = strdup(k);
                pe->value = strdup(v);
                list_add_tail(&pe->list, head);
        } else {
                if (max_count == 1) {
                        /* update (only one) ... */
                        pe = list_first_entry(head, struct string_entry, list);
                        if (pe->value)
                                free(pe->value);
                        pe->value = strdup(v);
                } else {
                        /* loop check then update or insert ... */
                        updated = 0;
                        list_for_each(p, head) {
                                pe = list_entry(p, struct string_entry, list);
                                if (!strcmp(k, pe->key)) {
                                        if (pe->value)
                                                free(pe->value);
                                        pe->value = strdup(v);
                                        updated = 1;
                                        break;
                                }
                        }
                        if (!updated) {
                                pe = calloc(1, sizeof(struct string_entry));
                                if (max_count != 1)
                                        pe->key = strdup(k);
                                pe->value = strdup(v);
                                list_add_tail(&pe->list, head);
                        }
                }
        }
        return 0;
}

int remove_value(struct list_head *head, char *k, char *v)
{
        struct string_entry *pe;
        struct list_head* p;
        struct list_head *n;
        int rc;

        NOT_USED(v);

        if (list_empty(head))
                return -1;

        if (max_count != 1) {
                if (!k)
                        return -3;
        }

        if (max_count == 1) {
                assert (list_is_singular(head));
                list_cleanup(head);
                rc = 0;
        } else {
                rc = -2;
                list_for_each_safe(p, n, head) {
                        pe = list_entry(p, struct string_entry, list);
                        if (!strcmp(k, pe->key)) {
                                list_del(p);
                                if (pe->key)
                                        free(pe->key);
                                if (pe->value)
                                        free(pe->value);
                                free(pe);
                                rc = 0;
                                break;
                        }
                }
        }
        return rc;
}

struct list_head *lookup_value(struct list_head *head, char *k)
{
        struct string_entry *pe;
        struct list_head* p;
        struct list_head *n;

        if (list_empty(head))
                return NULL;

        if (max_count != 1) {
                if (!k)
                        return NULL;
        }

        if (max_count == 1) {
                assert (list_is_singular(head));
                return head->next;
        } else {
                list_for_each_safe(p, n, head) {
                        pe = list_entry(p, struct string_entry, list);
                        if (!strcmp(k, pe->key))
                                return p;
                }
        }
        return NULL;
}

char k1[] = "2";

void list_test_t1(struct list_head *head)
{
        add_value(head, "0", "wan");
        add_value(head, "1", "lan");
        add_value(head, "1", "wan");
        add_value(head, "0", "wifi");
        remove_value(head, "1", "wan");
        add_value(head, "2", "wan");
}

void list_test_t1_more(struct list_head *head)
{
        struct list_head *p;
        struct string_entry *pe;
        char *k;

        k = k1;
        p = lookup_value(head, k);
        if (p) {
                pe = list_entry(p, struct string_entry, list);
                printf("=> found: (%s: %s)\n",
                        pe->key ? pe->key : "null", pe->value);
        } else
                printf("=> can't found entry with key value (%s)\n",
                        k ? k : "null");

        k = NULL;
        p = lookup_value(head, k);
        if (p) {
                pe = list_entry(p, struct string_entry, list);
                printf("=> found: (%s: %s)\n",
                        pe->key ? pe->key : "null", pe->value);
        } else
                printf("=> can't found entry with key value (%s)\n",
                        k ? k : "null");
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/* ------------------------------------------------------------------------- */
#if 0
struct psdn_connection {
	struct list_head list;
	uint8_t dest[6];
	uint8_t gateway[6];
	uint16_t port;
	uint16_t vid;
	bool tag;
};

LIST_HEAD(psdn);

struct list_head *psdn_connection_find(struct list_head *head, uint8_t *mac)
{
	struct psdn_connection *c;
	struct list_head *p;

        list_for_each(p, head) {
                c = list_entry(p, struct psdn_connection, list);
		if (memcmp(c->dest, mac, 6) == 0)
			return p;
	}
	return NULL;
}

struct psdn_connection *psdn_connection_add(
	struct list_head *head, uint8_t *dest, uint8_t *gateway)
{
	struct 	psdn_connection *c;

	c = malloc(sizeof(struct psdn_connection));
	if (c == NULL)
		return NULL;

	memcpy(c->dest, dest, 6);
	memcpy(c->gateway, gateway, 6);

	list_add_tail(&c->list, head);
	return c;
}

struct psdn_connection *psdn_connection_update(
	struct list_head *head, uint8_t *dest, uint8_t *gateway)
{
	struct psdn_connection *c;
	struct list_head *p;

	p = psdn_connection_find(head, dest);
	if (p) { /* found */
                c = list_entry(p, struct psdn_connection, list);
		memcpy(c->gateway, gateway, 6);
	} else
		c = psdn_connection_add(head, dest, gateway);

	return c;
}

uint8_t NO_MAC[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

void connection_dump(uint8_t *dest, uint8_t *gateway)
{
	printf("dest = %02x:%02x:%02x:%0x2:%02x:%02x",
	       dest[0], dest[1], dest[2],
	       dest[3], dest[4], dest[5]);
	printf(", ");
	printf("gateway = %02x:%02x:%02x:%0x2:%02x:%02x",
	       gateway[0], gateway[1], gateway[2],
	       gateway[3], gateway[4], gateway[5]);
	printf("\n");
}

void psdn_connection_dump(struct list_head *head)
{
	struct psdn_connection *c;
	struct list_head *p;

	printf("<-------------------------------\n");
        list_for_each(p, head) {
                c = list_entry(p, struct psdn_connection, list);
		connection_dump(c->dest, c->gateway);
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
	int i;

	i = 0;
	while (true) {
		if (memcmp(tA[i].dest, NO_MAC, 6) == 0)
			break;
		printf("\n");
		connection_dump(tA[i].dest, tA[i].gateway);
		psdn_connection_update(&psdn, tA[i].dest, tA[i].gateway);
		psdn_connection_dump(&psdn);
		i++;
	}
	return 0;
}

#endif
/* ------------------------------------------------------------------------- */
/*                                                                           */
/* ------------------------------------------------------------------------- */

int main(int argc, char **argv)
{

        NOT_USED(argc);
        NOT_USED(argv);

        printf("running list_demo...\n\n");
        list_demo();

        max_count = 1;
        list_test_t1(&t1);
        list_test_t1_more(&t1);
        list_dump(&t1);
        list_cleanup(&t1);
        list_test_t1_more(&t1);
        max_count = 20;
        list_test_t1(&t1);
        list_test_t1_more(&t1);
        list_dump(&t1);
        list_cleanup(&t1);
        list_test_t1_more(&t1);

//	psdn_connection_test();

        return 0;
}
