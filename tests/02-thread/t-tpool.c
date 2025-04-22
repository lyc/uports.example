#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include <thread.h>

#define NOT_USED(a) (void)(a)

/* ------------------------------------------------------------------------- */
/*                                                                           */
/* ------------------------------------------------------------------------- */

int data[3] = { 1, 1, 1};

int func(void *arg)
{
        int *pi = (int *)arg;

        *pi = *pi + 1;
        return 0;
}

int test_tpool()
{
        int i, rc;
        struct tpool *iot, *p;

        printf("===> (%s:%d) tpool_register\n", __func__, __LINE__);
        iot = tpool_register(&default_tpool_operations, 3, "iot");
        p = tpool_find("iot");
        assert(p == iot);

#if 1
        sleep(1);
        printf("===> (%s:%d) tpool_set_handler\n", __func__, __LINE__);
        if ((rc = tpool_set_handler(p, default_tpool_handler)) < 0)
                printf("===> (%s:%d) Oops, tpool_set_handler error(%d)\n",
                       __func__, __LINE__, rc);
#endif

        for (i=0; i<3; i++)
                printf("===> (%s:%d) data[%d] = %d\n",
                       __func__, __LINE__, i, data[i]);

        (p->p_op->dispatch)(p, func, (void *)&data[1]);
        sleep(1);
        for (i=0; i<3; i++)
                printf("===> (%s:%d) data[%d] = %d\n",
                       __func__, __LINE__, i, data[i]);

        (p->p_op->dispatch)(p, func, (void *)&data[0]);
        (p->p_op->dispatch)(p, func, (void *)&data[1]);
        (p->p_op->dispatch)(p, func, (void *)&data[2]);
        sleep(1);
        for (i=0; i<3; i++)
                printf("===> (%s:%d) data[%d] = %d\n",
                       __func__, __LINE__, i, data[i]);

        for (i=0; i<100; i++) {
                if (p) {
                        rc = (p->p_op->dispatch)(p, func, (void *)&data[2]);
                        if (rc < 0) {
                                printf("===> (%s:%d) dispatch(%d) failed, rc=%d.\n", __func__, __LINE__, i, rc);
                                sleep(1);
                        }
                }
                if (i == 75) {
                        rc = tpool_unregister(p, true);
                        if (rc != 0)
                                printf("===> (%s:%d) Oops, tpool_unregister error(%d)\n", __func__, __LINE__, rc);
                        p = NULL;
                }
        }
        for (i=0; i<3; i++)
                printf("===> (%s:%d) data[%d] = %d\n",
                       __func__, __LINE__, i, data[i]);

        rc = tpool_unregister(p, true);
        if (rc != 0)
                printf("===> (%s:%d) Oops, tpool_unregister error(%d)\n",
                       __func__, __LINE__, rc);

        return 0;
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/* ------------------------------------------------------------------------- */

int main( int argc, char *argv[])
{
        NOT_USED(argc);
        NOT_USED(argv);
        printf("welcome to cpool test program, built on %s...\n\n", __TIME__);

        test_tpool();
        return 0;
}
