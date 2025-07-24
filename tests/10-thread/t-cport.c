#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/time.h>

#include <thread.h>

#define NOT_USED(a) (void)(a)

/* ------------------------------------------------------------------------- */
/*                                                                           */
/* ------------------------------------------------------------------------- */

void platform_startup()
{
}

void platform_shutdown()
{
}

/* ------------------------------------------------------------------------- */
/* TEST_1                                                                    */
/* ------------------------------------------------------------------------- */

int aaa = 0;
char obuf[64];

int my_func( struct cport* p, void *buf, int num)
{
        char *pb = (char*)buf;

        aaa++;
        if (aaa%5 == 0) {
                sprintf(obuf, "<=== %s:%d\n", p->p_name, aaa);
                (p->p_op->write)(p, (void*)obuf, strlen(obuf));
        }


        pb[num-1] = '\0';
        printf("[%s]: got %s\n", p->p_name, pb);
        return 0;
}

#if 0
struct owdp_operations my_owdp_control_operations = {
        .prepare_timeout         = default_prepare_timeout,
        .execute_timeout_pending = default_execute_timeout_pending,
        .func                    = my_func,
};
#endif

#if 0
/* ------------------------------------------------------------------------- */
/* TEST_1                                                                    */
/* ------------------------------------------------------------------------- */

int my_func2( struct cport* p, void *buf, int num)
{
        char *pb = (char*)buf;

        pb[num] = '\0';
        printf("[%s]: got %s", p->p_name, pb);
        return 0;
}

struct owdp_operations my_owdp_control_operations = {
        .prepare_timeout         = default_prepare_timeout,
        .execute_timeout_pending = default_execute_timeout_pending,
        .func                    = my_func2,
};
#endif

/* ------------------------------------------------------------------------- */
/*                                                                           */
/* ------------------------------------------------------------------------- */

#define MAXLINE 512

#if 1
int my_test1_prepare_timeout(struct timespec* ts)
{
        struct timeval t;

        gettimeofday(&t, NULL);

        ts->tv_nsec = t.tv_usec * 1000;
        ts->tv_sec = t.tv_sec + 3;
        return 0;
}

int my_test1_cnt;

void my_test1_execute_timeout_pending(struct cport *p)
{
        int *pi;
        struct owdp_control *c;

        assert(p != NULL);
        c = (struct owdp_control*)p->p_priv;
        assert(c != NULL);

        pi = (int *)c->c_priv;
        printf("===> (%s:%d) cnt = %d\n", __func__, __LINE__, *pi);
        *pi = *pi + 1;
}

struct owdp_operations my_control_1 = {
        .prepare_timeout         = my_test1_prepare_timeout,
        .execute_timeout_pending = my_test1_execute_timeout_pending,
        .func                    = my_func,
};

struct owdp_operations my_control_2 = {
        .prepare_timeout         = default_prepare_timeout,
        .execute_timeout_pending = default_execute_timeout_pending,
        .func                    = my_func,
};
#endif


void test_owdp_cport()
{
        struct cport *p, *p1, *p2;
        char line[MAXLINE];

//        struct cport_operation my_test1_operation = default_owdp_operation;

#if 1
        p = cport_register(&default_owdp_operations, PORT_MODE_BLOCK, "TEST_1");
        assert(p != NULL);
        p1 = cport_find( "TEST_1");
        assert(p == p1);
//        my_control_1 = default_owdp_control_operation;
//        my_control_1.func = my_func;
        cport_set_handler(p1, default_owdp_handler,
                          &my_control_1, (void*)&my_test1_cnt);

        p = cport_register(&default_owdp_operations, PORT_MODE_BLOCK, "TEST_2");
        assert(p != NULL);
        p2 = cport_find("TEST_2");
        assert(p == p2);
        my_control_2 = default_owdp_control_operations;
        my_control_2.func = my_func;
        cport_set_handler(p2, default_owdp_handler, &my_control_2, NULL);
#else
        /* FIXME: owdp_operation instance shouldn't be as local variable */
        struct owdp_operations my_control = default_owdp_control_operations;
        my_control.func = my_func;

        p = cport_register(&default_owdp_operations, PORT_MODE_BLOCK, "TEST_1");
        assert(p != NULL);
        p1 = cport_find("TEST_1");
        assert(p == p1);
        cport_set_handler(p1, default_owdp_handler, &my_control, NULL);

        p = cport_register(&default_owdp_operations, PORT_MODE_BLOCK, "TEST_2");
        assert(p != NULL);
        p2 = cport_find("TEST_2");
        assert(p == p2);
        cport_set_handler(p2, default_owdp_handler, &my_control, NULL);
#endif

        for (;;) {
                if (fgets(line, MAXLINE-1, stdin) != NULL) {
                        printf("==> %s<%d>", line, (int)strlen(line));
                        if (strstr(line, "quit") != NULL)
                                break;
                        (p1->p_op->write)(p1, (void*)line, strlen( line));
                        (p2->p_op->write)(p2, (void*)line, strlen( line));
                }
        }
        cport_unregister(p1);
        cport_unregister(p2);
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/* ------------------------------------------------------------------------- */

int main( int argc, char *argv[])
{
        NOT_USED(argc);
        NOT_USED(argv);
        printf("welcome to cport test program, built on %s...\n\n", __TIME__);

        platform_startup();

        test_owdp_cport();

        platform_shutdown();
        return 0;
}
