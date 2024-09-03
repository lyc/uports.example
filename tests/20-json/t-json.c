#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

#include <json-c/json.h>

#ifndef NOT_USED
#define NOT_USED(a) (void)(a)
#endif

/* -------------------------------------------------------------------------- */
/* JSON create test (without file)                                            */
/* -------------------------------------------------------------------------- */

//#define SMS_CREATE
#define SIMPLE_CREATE

void json_create_test_manually()
{
        printf("\n====== JSON create test (manually) ======================\n");

#if defined(SIMPLE_CREATE)
        struct json_object *jso, *jstr, *jarray;

        jso = json_object_new_object();

        jstr = json_object_new_string("Joys of Programming");

        jarray = json_object_new_array();
        json_object_array_add(jarray, json_object_new_string("asm"));
        json_object_array_add(jarray, json_object_new_string("c"));
        json_object_array_add(jarray, json_object_new_string("c++"));
        json_object_array_add(jarray, json_object_new_string("javascript"));
        json_object_array_add(jarray, json_object_new_string("lisp"));

        json_object_object_add(jso, "Site Name", jstr);
        json_object_object_add(jso, "Categories", jarray);
        json_object_object_add(jso, "Key", json_object_new_string("Oops"));
#elif defined(SMS_CREATE)
        struct json_object *jso, *jo1, *jo2, *ja;

        jso = json_object_new_object();

        jo1 = json_object_new_object();
        json_object_object_add(jo1, "name", json_object_new_string("xyz"));
        json_object_object_add(jo1, "algorithm", json_object_new_string("RSA"));
        json_object_object_add(jo1, "key-data", json_object_new_string("01010"));
        jo2 = json_object_new_object();
        json_object_object_add(jo2, "name", json_object_new_string("abc"));
        json_object_object_add(jo2, "algorithm", json_object_new_string("DES"));
        json_object_object_add(jo2, "key-data", json_object_new_string("01010"));
        ja = json_object_new_array();
        json_object_array_add(ja, jo1);
        json_object_array_add(ja, jo2);
        json_object_object_add(jso, "ssh-key", ja);
//        json_object_put(ja);
//        ja = NULL;
//        printf("JSON object put ja...\n");
#endif

        printf("JSON string => %s\n", json_object_to_json_string(jso));
        if (jso) {
                json_object_put(jso);
        }
}

struct json_object *create_json_object_from_string(const char *str)
{
        struct json_object *jso;

        jso = json_tokener_parse(str);
        return jso;
}

void json_create_test_from_string()
{
        const char *tp[] = {
                "",
                "\0",
                "\"null\"",
                "1",
                "1.2",
                "true",
                "\"true\"",
                "\"this is a string\"",
                "[\"xml\", \"json\"]",
                "{\"lang\" : \"lisp\"}",
                "[\"lang\" : \"lisp\"]",
                "[ { \"lang\" : \"lisp\" } ]",
                "[ \"abc\": { \"lang\" : \"lisp\" } ]",
                "{\"sitename\" : \"joys of programming\", \"categories\" : [ \"c\" , [\"c++\" , \"c\" ], \"java\", \"PHP\" ], \"author-details\": { \"admin\": false, \"name\" : \"Joys of Programming\", \"Number of Posts\" : 10 }}",
        };

        unsigned int i;
        struct json_object *jso;

        printf("\n====== JSON create test (from string) ===================\n");

        for (i=0; i<sizeof(tp)/sizeof(const char*); i++) {
                printf("[%02d]: ", i);
                jso = create_json_object_from_string(tp[i]);
                if (jso) {
                        printf("JSON string => %s\n",
                               json_object_to_json_string(jso));
                        json_object_put(jso); /* free json_object ... */
                } else
                        printf("\n");
        }
}

/* -------------------------------------------------------------------------- */
/* JSON key search test                                                       */
/* -------------------------------------------------------------------------- */

struct tokener {
        int alloc;
        char buf[1024], sep[8];

        int argc;
        char **argv;
};

struct tokener *new_tokener(char *sep, int alloc);
int free_tokener(struct tokener *tk);
int tokener_feed(struct tokener *tk, char *buf);

struct tokener *new_tokener(char *sep, int alloc)
{
        struct tokener *tk;

        assert(strlen(sep) < 8);

        tk = calloc(1, sizeof(struct tokener));
        if (!tk)
                return NULL;

        tk->alloc = alloc;
        strcpy(tk->sep, sep);
        tk->argv = calloc(alloc,sizeof(char *));
        return tk;
}

int free_tokener(struct tokener *tk)
{
        if (tk->argv)
                free(tk->argv);
        free(tk);
        return 0;
}

int tokener_feed(struct tokener *tk, char *buf)
{
        int i;
        char *p, *save;

        assert(strlen(buf) < 1024);

        for (i=0; i<tk->alloc; i++)
                tk->argv[i] = NULL;
        tk->argc = 0;
        strcpy(tk->buf, buf);

        i = 0;
        p = strtok_r(tk->buf, tk->sep, &save);
        while (p) {
                tk->argv[i] = p;
                i++;
                if (i >= tk->alloc)
                        break;
                p = strtok_r(NULL, tk->sep, &save);
        }
        if (i < tk->alloc) {
                tk->argc = i;
                return 0;
        } else
                return -1;
}

const char *find_key_in_json(struct json_object *jso, const char *key)
{
        const char *kv;
        struct json_object_iterator ji, je;
        struct json_object *jv;

        kv = NULL;
        ji = json_object_iter_begin(jso);
        je = json_object_iter_end(jso);
        while (!json_object_iter_equal(&ji, &je)) {
                if (!strcmp(json_object_iter_peek_name(&ji), key)) {
                        jv = json_object_iter_peek_value(&ji);
                        if (json_object_get_type(jv) == json_type_string) {
                                kv = json_object_get_string(jv);
                                break;
                        }
                }
                json_object_iter_next(&ji);
        }
        return kv;
}

int find_key_in_json_ex(char *buf, struct json_object *jso, struct tokener *t)
{
        const char *k;
        int i;

        buf[0] = '\0';
        for (i=0; i<t->argc; i++) {
                k = find_key_in_json(jso, t->argv[i]);
                if (!k)
                        break;
                if (i > 0)
                        strcat(buf, "_");
                strcat(buf, k);
        }
        return 0;
}

char key_string[] = "Key 2th";

void json_find_key_test()
{
        struct json_object *jso, *jstr, *jarray;
        struct tokener *t;
        int i;
        int abc = 12;

        printf("\n====== JSON find key test ===============================\n");

        jso = json_object_new_object();
        jstr = json_object_new_string("Joys of Programming");
        json_object_object_add(jso, "2th", json_object_new_string("another"));

        jarray = json_object_new_array();
        json_object_array_add(jarray, json_object_new_string("asm"));
        json_object_array_add(jarray, json_object_new_string("c"));
        json_object_array_add(jarray, json_object_new_string("c++"));
        json_object_array_add(jarray, json_object_new_string("javascript"));
        json_object_array_add(jarray, json_object_new_string("lisp"));

        json_object_object_add(jso, "integer", json_object_new_int(abc));
        json_object_object_add(jso, "Site Name", jstr);
        json_object_object_add(jso, "Categories", jarray);
        json_object_object_add(jso, "Key", json_object_new_string("Oops"));

        printf("JSON string => %s\n", json_object_to_json_string(jso));

        /* iterator test to find key ... */
#if 0
        struct json_object_iterator it;
        struct json_object_iterator itEnd;
        const char *pk;
        struct json_object *jv;

        it = json_object_iter_begin(jso);
        itEnd = json_object_iter_end(jso);
        while (!json_object_iter_equal(&it, &itEnd)) {
                pk = json_object_iter_peek_name(&it);
                printf("%s\n", pk);
                if (!strcmp(pk, "Key")) {
                        jv = json_object_iter_peek_value(&it);
                        printf("Got key...\n");
                        _dummy_json_op_value(jv);
                        break;
                }
                json_object_iter_next(&it);
        }
#else
        char mk[128];
        const char *key;
        key = find_key_in_json(jso, "Key");
        if (key)
                printf("Got key: %s...\n", key);
        else
                printf("Oops, can't find key in JSON object\n");

        t = new_tokener(" ", 5);
        tokener_feed(t, key_string);
        for (i=0; i<t->argc; i++)
                printf("===> %s\n", t->argv[i]);
        find_key_in_json_ex(mk, jso, t);
        printf("mk = %s\n", mk);
        free_tokener(t);

        t = new_tokener(" ", 2);
        tokener_feed(t, "Key");
        find_key_in_json_ex(mk, jso, t);
        printf("mk = %s\n", mk);
        free_tokener(t);
#endif

        if (jso)
                json_object_put(jso);
}

/* -------------------------------------------------------------------------- */
/* JSON create test (with file)                                               */
/* -------------------------------------------------------------------------- */

ssize_t get_filesize(int fd)
{
        off_t cur, len;

        assert(fd > 0);

        cur = lseek(fd, 0L, SEEK_CUR);
        len = lseek(fd, 0L, SEEK_END);
        assert(cur == lseek(fd, cur, SEEK_SET));

        return (ssize_t)len;
}

int read_file(const char *fn, ssize_t *size, char **buf)
{
        int fd;
        ssize_t ns, nr, i;

        fd = open(fn, O_RDONLY);
        if (fd < 0) {
                printf("Oops, file %s open failed...\n", fn);
                return -1;
        }

        ns = (ssize_t)get_filesize(fd);
        printf("file size of %s: %ld\n", fn, (unsigned long)ns);
        *size = (int)ns;

        *buf = malloc(ns+1);
        if (*buf == NULL) {
                close(fd);
                printf ("Memory allocate fail...\n");
                return -1;
        }

        i = 0;
        while (1) {
                if (ns > 1024)
                        nr = 1024;
                else
                        nr = ns;
                nr = read(fd, *buf+i, nr);
                i += nr;
                ns -= nr;
                if (ns == 0)
                        break;
        }
        (*buf)[i] = '\0';

        close(fd);
        return 0;
}

int copy_file(char *from, char *to)
{
        int fd1, fd2;
        int ns;
        char buf[1024];

        fd1 = open(from, O_RDONLY);
        fd2 = creat(to, S_IRUSR|S_IWUSR);

        ns = read(fd1, buf, 420);
        while (1) {
                ns = read(fd1, buf, 1024);
                if (ns == 0)
                        break;
                write(fd2, buf, ns);
        }

        close(fd1);
        close(fd2);
        return 0;
}

struct json_object *create_json_object_from_file(const char *fn)
{
        char *buf;
        int fd, nr;
        enum json_tokener_error jerr;
        struct json_tokener *tok;
        struct json_object *jso = NULL;

        fd = open(fn, O_RDONLY);
        if (fd < 0) {
                printf("[%s]: Oops, file %s open failed...\n", __func__, fn);
                return jso;
        }

        buf = (char*)malloc(1024);
        if (!buf) {
                printf("[%s]: Oops, can't allocate memory...\n",  __func__);
                close(fd);
                return jso;
        }

        tok = json_tokener_new();
        do {
                nr = read(fd, buf, 1024);
                if (nr == 0)
                        break;
                jso = json_tokener_parse_ex(tok, buf, nr);
                jerr = json_tokener_get_error(tok);
        } while (jerr == json_tokener_continue);
        if (jerr != json_tokener_success) {
                printf("[%s]: Oops, json token parse failure...\n", __func__);
                assert(jso != NULL);
        }
        json_tokener_free(tok);

        free(buf);
        close(fd);
        return jso;
}

void json_create_test_from_file(char *file)
{
        struct json_object *jso = NULL;

        printf("\n====== JSON create test (from file) =====================\n");

#if 1
        int rc;
        char *buf;
        ssize_t ns;

        rc = read_file(file, &ns, &buf);
        if (rc >= 0) {
                printf("Input file: => %s\n", buf);
                jso = create_json_object_from_string((const char*)buf);
                free(buf);
        }
#else
        jso = create_json_object_from_file(file);
#endif
        if (jso) {
                printf("JSON string => %s\n", json_object_to_json_string(jso));
                json_object_put(jso); /* free json_object ... */
        }
}

/* -------------------------------------------------------------------------- */
/* JSON parse engine                                                          */
/* -------------------------------------------------------------------------- */

struct json_parse_operations {
        int (*do_null)(struct json_object *jso, char *key, void *data);
        int (*do_boolean)(struct json_object *jso, char *key, void *data);
        int (*do_int)(struct json_object *jso, char *key, void *data);
        int (*do_double)(struct json_object *jso, char *key, void *data);
        int (*do_string)(struct json_object *jso, char *key, void *data);
        int (*do_array)(struct json_object *jso, char *key, void *data);
        int (*do_object)(struct json_object *jso, char *key, void *data);
};

int do_json_parse(
        struct json_object *jso, struct json_parse_operations *op, void *data);

int do_json_parse(
        struct json_object *jso, struct json_parse_operations *op, void *data)
{
        int rc;
	enum json_type type;

        assert(jso != NULL);

        type = json_object_get_type(jso);
        switch (type) {
                case json_type_null:
                        if (op->do_null)
                                rc = (op->do_null)(jso, NULL, data);
                        break;
                case json_type_int:
                        if (op->do_int)
                                rc = (op->do_int)(jso, NULL, data);
                        break;
                case json_type_boolean:
                        if (op->do_boolean)
                                rc = (op->do_boolean)(jso, NULL, data);
                        break;
                case json_type_double:
                        if (op->do_double)
                                rc = (op->do_double)(jso, NULL, data);
                        break;
                case json_type_string:
                        if (op->do_string)
                                rc = (op->do_string)(jso, NULL, data);
                        break;
                case json_type_array:
                        if (op->do_array)
                                rc = (op->do_array)(jso, NULL, data);
                        break;
                case json_type_object:
                        if (op->do_object)
                                rc = (op->do_object)(jso, NULL, data);
                        break;
        }
        return rc;
}

/* -------------------------------------------------------------------------- */
/* JSON parse test (by dummy_json_node_op)                                    */
/* -------------------------------------------------------------------------- */


struct dummy_ctx {
        int ident;
        char space[128];
};

static char *ispace(struct dummy_ctx *ctx)
{
        memset(ctx->space, '=', ctx->ident*4);
        ctx->space[ctx->ident*4] = '\0';
        strcat(ctx->space, "==>");
        return ctx->space;
}

static int _dummy_json_op_value(struct json_object *jso, char *key, void *data);
static int _dummy_json_op_object(struct json_object *jso, char *key, void *data);
static int _dummy_json_op_array(struct json_object *jso, char *key, void *data);

static int _dummy_json_op_value(struct json_object *jso, char *key, void *data)
{
        enum json_type type;
        struct dummy_ctx *ctx = (struct dummy_ctx *)data;

        type = json_object_get_type(jso);
        switch (type) {
        case json_type_int:
                printf("%s json_value_int: (%s : %d)\n",
                       ispace(ctx), key ? key : " ",
                       json_object_get_int(jso));
                break;
        case json_type_boolean:
                printf("%s json_value_boolean: (%s : %s)\n",
                       ispace(ctx), key ? key : " ",
                       json_object_get_boolean(jso) ? "true" : "false");
                break;
        case json_type_double:
                printf("%s json_value_double: (%s : %lf)\n",
                       ispace(ctx), key ? key : " ",
                       json_object_get_double(jso));
                break;
        case json_type_string:
                printf("%s json_value_string: (%s : %s)\n",
                       ispace(ctx), key ? key : " ",
                       json_object_get_string(jso));
                break;
        case json_type_null:
                printf("%s json_value_null: (%s :)\n",
                       ispace(ctx), key ? key : " ");
                break;
        case json_type_array:
        case json_type_object:
                printf("Oops, should not go to here...\n");
                break;
        }
        return 0;
}

static int _dummy_json_op_array(struct json_object *jso, char *key, void *data)
{
        int rc;
        enum json_type type;
        struct json_object *jval;
        struct dummy_ctx *ctx = (struct dummy_ctx *)data;
        size_t i;

        printf("%s json_type_ARRAY: (%s)\n",
               ispace(ctx), key ? key : " ");

        ctx->ident++;
        for (i=0; i<json_object_array_length(jso); i++) {
                jval = json_object_array_get_idx(jso, i);
                type = json_object_get_type(jval);

		switch (type) {
                case json_type_null:
                case json_type_int:
                case json_type_boolean:
                case json_type_double:
                case json_type_string:
                        rc = _dummy_json_op_value(jval, NULL, ctx);
                        break;
                case json_type_array:
                        rc = _dummy_json_op_array(jval, NULL, ctx);
                        break;
                case json_type_object:
                        rc = _dummy_json_op_object(jval, NULL, ctx);
                        break;
		}
        }
        ctx->ident--;
        return  rc;
}

static int _dummy_json_op_object(struct json_object *jso, char *key, void *data)
{
        int rc;
	enum json_type type;
        struct dummy_ctx *ctx = (struct dummy_ctx *)data;

        printf("%s json_type_OBJECT: (%s)\n",
               ispace(ctx), key ? key : " ");

        ctx->ident++;
	json_object_object_foreach(jso, jkey, jval) {
		type = json_object_get_type(jval);

		switch (type) {
                case json_type_null:
                case json_type_int:
                case json_type_boolean:
                case json_type_double:
                case json_type_string:
                        rc = _dummy_json_op_value(jval, jkey, ctx);
                        break;
                case json_type_array:
                        rc = _dummy_json_op_array(jval, jkey, ctx);
                        break;
                case json_type_object:
                        rc = _dummy_json_op_object(jval, jkey, ctx);
                        break;
		}
	}
        ctx->ident--;
        return rc;
}

struct json_parse_operations dummy_json_node_op = {
        .do_null    = _dummy_json_op_value,
        .do_boolean = _dummy_json_op_value,
        .do_int     = _dummy_json_op_value,
        .do_double  = _dummy_json_op_value,
        .do_string  = _dummy_json_op_value,
        .do_array   = _dummy_json_op_array,
        .do_object  = _dummy_json_op_object,
};

void json_parse_test_by_dummy_json_node_op(char *file)
{
        struct json_object *jso;
        struct dummy_ctx dummy;

        printf("\n====== JSON parse test (by dummy_json_op) ===============\n");

        jso = create_json_object_from_file(file);
        if (jso) {
                printf("JSON string => %s\n", json_object_to_json_string(jso));

                dummy.ident = 0;
                do_json_parse(jso, &dummy_json_node_op, &dummy);
                json_object_put(jso); /* free json_object ... */
        }
}

/* -------------------------------------------------------------------------- */
/* JSON userdata test                                                         */
/* -------------------------------------------------------------------------- */

typedef void (*json_userdata_cb)(
#if 0
        struct lyd_node *node,
#endif
        struct json_object *jso, void *userdata);

struct userdata {
        bool hotspot;

        union {
                int i;
                bool b;

                double f;
                char *s;
        } data;
};

struct userdata *new_json_userdata(
        struct json_object *jso, bool hotspot, void *data);
void free_json_userdata(struct json_object *jso, void *userdata);

struct userdata *new_json_userdata(
        struct json_object *jso, bool hotspot, void *data)
{
        enum json_type type;
        struct userdata *u;

        u = calloc(1, sizeof(struct userdata));
        if (!u)
                return NULL;

        u->hotspot = hotspot;
        if (data) {
                type = json_object_get_type(jso);
                switch (type) {
                case json_type_int:
                        u->data.i = *((int *)data);
                        printf("----->    new_json_userdata: %d\n", u->data.i);
                        break;
                case json_type_boolean:
                        u->data.b = *((bool *)data);
                        printf("----->    new_json_userdata: %s\n",
                               u->data.b ?  "true" : "false");
                        break;
                case json_type_double:
                        u->data.f = *((double *)data);
                        printf("----->    new_json_userdata: %lf\n", u->data.f);
                        break;
                case json_type_string:
                        u->data.s = strdup((const char *)data);
                        printf("----->    new_json_userdata: %s\n", u->data.s);
                        break;
                case json_type_null:
                case json_type_array:
                case json_type_object:
                        break;
                }
        } else
                printf("----->    new_json_userdata: %s\n", "");

        json_object_set_userdata(jso, (void *)u, free_json_userdata);
        return u;
}

void free_json_userdata(struct json_object *jso, void *userdata)
{
        enum json_type type;
        struct userdata *u = (struct userdata *)userdata;

        type = json_object_get_type(jso);
        switch (type) {
        case json_type_int:
                printf("----->    free_json_userdata: %d\n", u->data.i);
                break;
        case json_type_boolean:
                printf("----->    free_json_userdata: %s\n",
                       u->data.b ?  "true" : "false");
                break;
        case json_type_double:
                printf("----->    free_json_userdata: %lf\n", u->data.f);
                break;
        case json_type_string:
                printf("----->    free_json_userdata: %s\n", u->data.s);
                free(u->data.s);
                break;
        case json_type_null:
        case json_type_array:
        case json_type_object:
                break;
        }
        free(u);
}

static int _ck_json_op_value(struct json_object *jso, char *key, void *data);
static int _ck_json_op_object(struct json_object *jso, char *key, void *data);
static int _ck_json_op_array(struct json_object *jso, char *key, void *data);

static int _ck_json_op_value(struct json_object *jso, char *key, void *data)
{
        struct userdata *u;
        enum json_type type;

        NOT_USED(key);
        NOT_USED(data);

        u = (struct userdata *)json_object_get_userdata(jso);
        if (u)
                printf("-----> %sfound_json_userdata(%s)",
                       u->hotspot ? "[*]" : "   ", key);

        type = json_object_get_type(jso);
        switch (type) {
        case json_type_int:
                if (u && u->hotspot)
                        printf("(int): %d", u->data.i);
                break;
        case json_type_boolean:
                if (u && u->hotspot)
                        printf("(boolean): %s", u->data.b ?  "true" : "false");
                break;
        case json_type_double:
                if (u && u->hotspot)
                        printf("(double): %lf", u->data.f);
                break;
        case json_type_string:
                if (u && u->hotspot)
                        printf("(string): %s", u->data.s);
                break;
        case json_type_null:
                break;
        case json_type_array:
        case json_type_object:
                break;
        }
        if (u)
                printf("\n");
        return 0;
}

static int _ck_json_op_array(struct json_object *jso, char *key, void *data)
{
        int rc;
        struct userdata *u;
        enum json_type type;
        struct json_object *jval;
        size_t i;

        NOT_USED(key);

        u = json_object_get_userdata(jso);
        if (u)
                printf("-----> %sfound_json_userdata(<array>:%s)\n",
                       u->hotspot ? "[*]" : "   ", key);

        for (i=0; i<json_object_array_length(jso); i++) {
                jval = json_object_array_get_idx(jso, i);
                type = json_object_get_type(jval);

		switch (type) {
                case json_type_null:
                case json_type_int:
                case json_type_boolean:
                case json_type_double:
                case json_type_string:
                        rc = _ck_json_op_value(jval, NULL, data);
                        break;
                case json_type_array:
                        rc = _ck_json_op_array(jval, NULL, data);
                        break;
                case json_type_object:
                        rc = _ck_json_op_object(jval, NULL, data);
                        break;
		}
        }
        return  rc;
}

static int _ck_json_op_object(struct json_object *jso, char *key, void *data)
{
        int rc;
        struct userdata *u;
	enum json_type type;

        NOT_USED(key);

        u = json_object_get_userdata(jso);
        if (u)
                printf("-----> %sfound_json_userdata(<object>:%s)\n",
                       u->hotspot ? "[*]" : "   ", key);

	json_object_object_foreach(jso, jkey, jval) {
		type = json_object_get_type(jval);

		switch (type) {
                case json_type_null:
                case json_type_int:
                case json_type_boolean:
                case json_type_double:
                case json_type_string:
                        rc = _ck_json_op_value(jval, jkey, data);
                        break;
                case json_type_array:
                        rc = _ck_json_op_array(jval, jkey, data);
                        break;
                case json_type_object:
                        rc = _ck_json_op_object(jval, jkey, data);
                        break;
		}
	}
        return rc;
}

struct json_parse_operations ck_json_node_op = {
        .do_null    = _ck_json_op_value,
        .do_boolean = _ck_json_op_value,
        .do_int     = _ck_json_op_value,
        .do_double  = _ck_json_op_value,
        .do_string  = _ck_json_op_value,
        .do_array   = _ck_json_op_array,
        .do_object  = _ck_json_op_object,
};

void json_userdata_test(char *file)
{
        int abc = 12;
        struct json_object *jso, *jstr, *jarray, *j;

        printf("\n====== JSON userdata test ===============================\n");

        if (!file) {
                jso = json_object_new_object();
                jstr = json_object_new_string("Joys of Programming");
                new_json_userdata(jstr, true, NULL); /* <=== */
                json_object_object_add(jso, "2th",
                                       json_object_new_string("another"));

                jarray = json_object_new_array();
                json_object_array_add(jarray, json_object_new_string("asm"));
                json_object_array_add(jarray, json_object_new_string("c"));
                json_object_array_add(jarray, json_object_new_string("c++"));
                json_object_array_add(jarray, json_object_new_string("javasccript"));

                json_object_array_add(jarray, json_object_new_string("lisp"));

                j = json_object_new_int(abc);
                new_json_userdata(j, false, (void *)&abc); /* <=== */
                json_object_object_add(jso, "integer", j);

                json_object_object_add(jso, "Site Name", jstr);
                json_object_object_add(jso, "Categories", jarray);
                json_object_object_add(jso, "Key",
                                       json_object_new_string("Oops"));
        } else {
                jso = create_json_object_from_file(file);
        }

        printf("\n----------------------------------------\n");
        printf("JSON string => %s\n", json_object_to_json_string(jso));

        printf("\n----------------------------------------\n");
        do_json_parse(jso, &ck_json_node_op, NULL);

        if (jso)
                json_object_put(jso);
}

/* -------------------------------------------------------------------------- */
/*                                                                            */
/* -------------------------------------------------------------------------- */

struct json_object * jsonutil_vget(struct json_object * jobj, va_list ap)
{
	char *name = va_arg(ap, char *);
	int idx = -1;

	if (json_object_get_type(jobj) == json_type_array) {
		if (idx >= 0) {
			jobj = json_object_array_get_idx(jobj, idx);
                }
	} else if (json_object_get_type(jobj) == json_type_object) {
                if (name && *name) {
                        struct json_object *jval;
                        idx = va_arg(ap, int);

			if (json_object_object_get_ex(jobj, name, &jval)) {
                                jobj = jval;

				if (json_object_get_type(jobj) ==
                                    json_type_array) {
					if (idx >= 0) {
						jobj = json_object_array_get_idx(
                                                        jobj, idx);
                                        }
				} else if (json_object_get_type(jobj) ==
                                           json_type_object) {
                                } else {
					idx = -1;
				}
			}
                }
	} else {
                idx = -1;
	}

        return (idx < 0) ? jobj : (jobj) ? jsonutil_vget(jobj, ap) : NULL;
}

struct json_object * jsonutil_get(struct json_object * jobj, ...)
{
        va_list ap;
        struct json_object * r;

        va_start(ap, jobj);
        r = jsonutil_vget(jobj, ap);
        va_end(ap);
        return r;
}

int jsonutil_get_int(struct json_object * jobj, int *num, ...)
{
        va_list ap;
        struct json_object * r;
	int ret = -1;

        va_start(ap, num);
        r = jsonutil_vget(jobj, ap);
        va_end(ap);

	if (r) {
		*num = json_object_get_int(r);
		ret = 0;
	} else
		*num = 0;

	return ret;
}

void print_json(struct json_object *jso)
{
        enum json_type type;

        if (!jso) {
                printf("=====> Oops, invalid json object");
                return;
        }

        type = json_object_get_type(jso);
        switch (type) {
        case json_type_int:
                printf("json_value_int: %d\n",
                       json_object_get_int(jso));
                break;
        case json_type_boolean:
                printf("json_value_boolean: %s\n",
                       json_object_get_boolean(jso) ? "true" : "false");
                break;
        case json_type_double:
                printf("json_value_double:%lf\n",
                       json_object_get_double(jso));
                break;
        case json_type_string:
                printf("json_value_string: %s\n",
                       json_object_get_string(jso));
                break;
        case json_type_null:
                break;
        case json_type_array:
                printf("ARRAY: %s\n", json_object_to_json_string(jso));
                break;
        case json_type_object:
                printf("OBJECT: %s\n", json_object_to_json_string(jso));
                break;
        }
}

void jsonutil_test(char *file)
{
        struct json_object *jso, *jnew, *jout;

        printf("\n====== JSON util test = =================================\n");

        jso = create_json_object_from_file(file);

        printf("\n----------------------------------------\n");
        printf("JSON string => %s\n", json_object_to_json_string(jso));

        printf("\n----------------------------------------\n");
        if (strstr(file,"o-ran-uplane-conf")) {
                jout = jsonutil_get(
                        jso,
                        "o-ran-uplane-conf:user-plane-configuration", 0,
                        "endpoint-prach-group", 0,
                        "supported-prach-preamble-formats", 2);
                if (jout)
                        print_json(jout);

                jnew = jsonutil_get(
                        jso,
                        "o-ran-uplane-conf:user-plane-configuration", -1);
                if (jnew)
                        print_json(jnew);

                jout = jsonutil_get(
                        jnew,
                        "static-low-level-tx-endpoints", 1,
                        "name", -1);
                if (jout)
                        print_json(jout);
        }

        if (jso)
                json_object_put(jso);
}

/* -------------------------------------------------------------------------- */
/*                                                                            */
/* -------------------------------------------------------------------------- */

typedef struct json_object * (*jsonutil_get_cb)(struct json_object * jobj, ...);
typedef struct json_object * (*jsonutil_vget_cb)(
        struct json_object * jobj, va_list ap);

bool check_if_modified_string(
        struct json_object *jcfg, struct json_object *jinput,
        jsonutil_vget_cb vget, bool modify, ...)
{
        bool rc = false;
        va_list ap;
        struct json_object *jc, *ji;

        va_start(ap, modify);
        jc = (vget)(jcfg, ap);
        va_end(ap);

        va_start(ap, modify);
        ji = (vget)(jinput, ap);
        va_end(ap);

        if (jc && ji) {
                if (strcmp(json_object_to_json_string(jc),
                           json_object_to_json_string(ji))) {
                        rc = true;
                        if (modify)
                                json_object_set_string(
                                        jc, json_object_get_string(ji));
                }
        }
        return rc;
}

bool check_if_update_ptp_lock_state(
        struct json_object *operational, struct json_object *input,
        jsonutil_get_cb get)
{
        bool rc = false;
        struct json_object *jc, *ji;

        jc = (get)(operational, "ptp-status", 0, "lock-state", -1);
        ji = (get)(input, "ptp-status", 0, "lock-state", -1);
        if (jc && ji) {
                if (strcmp(json_object_to_json_string(jc),
                           json_object_to_json_string(ji))) {
                        json_object_set_string(jc, json_object_get_string(ji));
                        rc = true;
                }
        }
        return rc;
}

bool check_if_update_sync_state(
        struct json_object *operational, struct json_object *input,
        jsonutil_get_cb get)
{
        bool rc = false;
        struct json_object *jc, *ji;

        jc = (get)(operational, "sync-status", 0, "sync-state", -1);
        ji = (get)(input, "sync-status", 0, "sync-state", -1);
        if (jc && ji) {
                if (strcmp(json_object_to_json_string(jc),
                           json_object_to_json_string(ji))) {
                        json_object_set_string(jc, json_object_get_string(ji));
                        rc = true;
                }
        }
        return rc;
}

void send_ptp_state_change_notification(
        struct json_object *operational, jsonutil_get_cb get)
{
        struct json_object *jso;
#if 1
        jso = (get)(operational, "ptp-status", 0, "lock-state", -1);
        printf("===> /sync/ptp-status/lock-state change to: %s...\n",
               json_object_get_string(jso));
#else
        /* TODO: send NETCONF notification */
#endif
}

void send_sync_state_change_notification(
        struct json_object *operational, jsonutil_get_cb get)
{
        struct json_object *jso;
#if 1
        jso = (get)(operational, "sync-status", 0, "sync-state", -1);
        printf("===> /sync/sync-status/sync-state change to: %s...\n",
               json_object_get_string(jso));
#else
        /* TODO: send NETCONF notification */
#endif
}

void check_if_state_chage(
        struct json_object *operational, const char *inputfile,
        jsonutil_get_cb get)
{
        struct json_object *jinput1, *jinput;

        /*
         * TODO:
         *   check if input file has been modified since last time checking
         */

        jinput1 = create_json_object_from_file(inputfile);
        if (!jinput1)
                return;

        jinput = (get)(jinput1, "o-ran-sync:sync", -1);
        if (jinput) {
#if 0
                printf("==========> start check_if_modified_string\n");
                if (check_if_modified_string(
                        operational, jinput, jsonutil_vget, false,
                        "ptp-status", 0, "lock-state", -1))
                        send_ptp_state_change_notification(operational, get);
                printf("==========> end check_if_modified_string\n");
#endif
                if (check_if_update_ptp_lock_state(operational, jinput, get))
                        send_ptp_state_change_notification(operational, get);
                if (check_if_update_sync_state(operational, jinput, get))
                        send_sync_state_change_notification(operational, get);

                json_object_put(jinput1);
        }
}

void jsonutil_test2(char *cfgfile, char *inputfile)
{
        struct json_object *jcfg1, *jcfg;

        printf("\n====== JSON util test2 ==================================\n");

        jcfg1 = create_json_object_from_file(cfgfile);
        jcfg = jsonutil_get(jcfg1, "o-ran-sync:sync", -1);
        printf("\n----------------------------------------\n");
        print_json(jcfg);

        check_if_state_chage(jcfg, inputfile, jsonutil_get);

        if (jcfg1)
                json_object_put(jcfg1);
}

/* -------------------------------------------------------------------------- */
/*                                                                            */
/* -------------------------------------------------------------------------- */

enum vendor_code {
        VENDOR_UNKNOWN       = 0x0000, /* 00000000 - 00000000 */
        VENDOR_RU            = 0x0001, /* 00000000 - 00000001 */
        VENDOR_WNC_RU        = 0x0003, /* 00000000 - 00000011 */
        VENDOR_FOXCONN_RU    = 0x0005, /* 00000000 - 00000101 */
        VENDOR_PEGA_RU       = 0x0007, /* 00000000 - 00000111 */
        VENDOR_LIONS_RU      = 0x0009, /* 00000000 - 00001001 */
        VENDOR_FHM           = 0x0100, /* 00000001 - 00000000 */
        VENDOR_UFISPACE_FHM  = 0x0300, /* 00000011 - 00000000 */
        VENDOR_O1            = 0x8000, /* 10000000 - 00000000 */
};

struct json_object *JSTACK[10];
int ji;

struct json_object *pop_object(struct json_object *jso)
{
        if (ji < 0)
                return NULL;
        assert(jso == JSTACK[ji-1]);
        printf("=====> (%s,%d) pop JSTACK[%d]=%p, ",
               __func__, __LINE__, ji-1, JSTACK[ji-1]);
        JSTACK[ji--] = NULL;
        printf("return JSTACK[%d] = %p\n", ji-1, JSTACK[ji-1]);
        return JSTACK[ji-1];
}

struct json_object *push_top_object()
{
        struct json_object *jso;

        jso = json_object_new_object();
        if (!jso)
                return NULL;
        printf("=====> (%s,%d) new top object = %p\n", __func__, __LINE__, jso);
        ji = 0;
        printf("=====> (%s,%d) push JSTACK[%d] = %p\n",
               __func__, __LINE__, ji, jso);
        JSTACK[ji++] = jso;
        return jso;
}

struct json_object *push_cells(struct json_object *jso)
{
        struct json_object *jarray;

        jarray = json_object_new_array();
        if (!jarray)
                return NULL;
        printf("=====> (%s,%d) new Cell array = %p\n",
               __func__, __LINE__, jarray);
        json_object_object_add(jso, "cells", jarray);
        printf("=====> (%s,%d) push JSTACK[%d] = %p\n",
               __func__, __LINE__, ji, jarray);
        JSTACK[ji++] = jarray;
        return jarray;
}

struct json_object *push_add_cell(struct json_object *jso, char *name)
{
        struct json_object *cell;

        assert(json_object_get_type(jso) == json_type_array);

        cell = json_object_new_object();
        if (!cell)
                return NULL;
        printf("=====> (%s,%d) new Cell = %p\n", __func__, __LINE__, cell);
        json_object_object_add(cell, "name", json_object_new_string(name));
        json_object_array_add(jso, cell);
        printf("=====> (%s,%d) push JSTACK[%d] = %p\n",
               __func__, __LINE__, ji, cell);
        JSTACK[ji++] = cell;
        return cell;
}

struct json_object *push_nodes(struct json_object *jso)
{
        struct json_object *jarray;

        jarray = json_object_new_array();
        if (!jarray)
                return NULL;
        printf("=====> (%s,%d) new node array = %p\n",
               __func__, __LINE__, jarray);
        json_object_object_add(jso, "nodes", jarray);
        printf("=====> (%s,%d) push JSTACK[%d] = %p\n",
               __func__, __LINE__, ji, jarray);
        JSTACK[ji++] = jarray;
        return jarray;
}

struct json_object *push_add_node(
        struct json_object *jso, enum vendor_code vendor, char *name)
{
        char *p;
        struct json_object *node;

        assert(json_object_get_type(jso) == json_type_array);

        if (vendor & VENDOR_RU)
                p = "RU";
        else if (vendor & VENDOR_FHM)
                p = "FHM";
        else
                return NULL;

        node = json_object_new_object();
        if (!node)
                return NULL;
        printf("=====> (%s,%d) new %s node = %p\n", __func__, __LINE__, p, node);
        json_object_object_add(node, "name", json_object_new_string(name));
        json_object_object_add(node, "type", json_object_new_string(p));
        json_object_array_add(jso, node);
        printf("=====> (%s,%d) push JSTACK[%d] = %p\n",
               __func__, __LINE__, ji, node);
        JSTACK[ji++] = node;
        return node;
}

/*
 * cell(0)
 *    RU(0-R) 192.168.100.1
 * cell(1)
 *     FHM(F0-I) 192.168.100.150
 *        RU(1-R) 192.168.100.2
 *        RU(2-R) 192.168.100.3
 */

void json_test_ru(char *file)
{
        struct json_object *jso = NULL;

        printf("\n====== JSON RU create test (manually) ====================\n");

#if 0
        jso = create_json_object_from_file(file);
#else
        NOT_USED(file);

        jso = push_top_object();
          jso = push_cells(jso);

            jso = push_add_cell(jso, "C0");
              jso = push_nodes(jso);
                jso = push_add_node(jso, VENDOR_RU, "0");
                json_object_object_add(jso, "ip", json_object_new_string("192.168.100.1"));
                jso = pop_object(jso); /* node */
              jso = pop_object(jso); /* nodes */
            jso = pop_object(jso); /* cell */

            jso = push_add_cell(jso, "C1");
              jso = push_nodes(jso);
                jso = push_add_node(jso, VENDOR_FHM, "F0");
                json_object_object_add(jso, "ip", json_object_new_string("192.168.100.150"));
                  jso = push_nodes(jso);
                    jso = push_add_node(jso, VENDOR_RU, "1");
                    json_object_object_add(jso, "ip", json_object_new_string("192.168.100.2"));
                  jso = pop_object(jso); /* nodes */
                    jso = push_add_node(jso, VENDOR_RU, "2");
                    json_object_object_add(jso, "ip", json_object_new_string("192.168.100.3"));
                  jso = pop_object(jso); /* nodes */
                jso = pop_object(jso); /* fhm */
              jso = pop_object(jso); /* nodes */
            jso = pop_object(jso); /* cell */

          jso = pop_object(jso); /* cells */
        jso = pop_object(jso); /* top */
        printf("=====> (%s,%d) ji=%d, jso=%p\n", __func__, __LINE__, ji, jso);

        while (true) {
                struct json_object *jt;
                jt = pop_object(jso);
                if (jt == NULL)
                        break;
                jso = jt;
        }
        printf("=====> (%s,%d) ji=%d, jso=%p\n", __func__, __LINE__, ji, jso);
#endif

        if (jso) {
                printf("JSON string => %s\n", json_object_to_json_string(jso));
                json_object_put(jso); /* free json_object ... */
        }
}

/* -------------------------------------------------------------------------- */
/*                                                                            */
/* -------------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
        NOT_USED(argc);
	printf("JSON-C test: %s\n", argv[0]);

        /* ----- Create test (without file) --------------------------------- */

//        json_create_test_manually();
//        json_create_test_from_string();

        /* ----- Key search test -------------------------------------------- */

//        json_find_key_test();

        /* ----- Create test (with file) ------------------------------------ */

//        json_create_test_from_file(argv[1]);

        /* ----- Parsing test ----------------------------------------------- */

//        json_parse_test_by_dummy_json_node_op(argv[1]);
//        json_parse_test_by_libyang_json_node_op(argv[1]);
//        json_parse_test_by_hotspot_json_node_op(argv[1]);

        /* ----- Userdata test ---------------------------------------------- */
#if 0
        if (argc > 1)
                json_userdata_test(argv[1]);
        else
                json_userdata_test(NULL);
#endif

        /* ----- jsonutil test ---------------------------------------------- */
        /* use o-ran-uplane-conf.cfg as argv[1] */
#if 0
        if (argc > 1)
                jsonutil_test(argv[1]);
        else
                jsonutil_test(NULL);
#endif

        /* ----- jsonutil2 test --------------------------------------------- */
        /* use o-ran-sync.cfg as argv[1] */
        /* use tmp-sync.json as argv[2] */
#if 0
        jsonutil_test2(argv[1], argv[2]);
#endif

        /* ----- json RU test --------------------------------------------- */
        /* use ru.json as argv[1] */
        json_test_ru(argv[1]);

        return 0;
}
