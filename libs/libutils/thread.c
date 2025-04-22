#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "thread.h"

#ifndef NOT_USED
#define NOT_USED(x) (void)(x)
#endif

/* ------------------------------------------------------------------------- */
/* reader/writer lock support                                                */
/*                                                                           */
/* ------------------------------------------------------------------------- */

void rw_lock_init(struct rw_lock *rw)
{
        pthread_mutex_init(&(rw->mutex), NULL);
        pthread_cond_init(&(rw->cond), NULL);
        rw->reader = 0;
        rw->writer = 0;
}

int rw_rlock(struct rw_lock *rw)
{
        pthread_mutex_lock(&(rw->mutex));
        while (rw->writer) {
                pthread_cond_wait(&(rw->cond), &(rw->mutex));
        }
        rw->reader++;
        pthread_mutex_unlock(&(rw->mutex));
        return 0;
}

int rw_runlock(struct rw_lock *rw)
{
        pthread_mutex_lock(&(rw->mutex));
        if (rw->reader == 0) {
                pthread_mutex_unlock(&(rw->mutex));
                return -1;
        } else {
                rw->reader--;
                if (rw->reader == 0)
                        pthread_cond_signal(&(rw->cond));
                pthread_mutex_unlock(&(rw->mutex));
                return 0;
        }
}

int rw_wlock(struct rw_lock *rw)
{
        pthread_mutex_lock(&(rw->mutex));
        while (rw->writer || rw->reader) {
                pthread_cond_wait(&(rw->cond), &(rw->mutex));
        }
        rw->writer++;
        pthread_mutex_unlock(&(rw->mutex));
        return 0;
}

int rw_wunlock(struct rw_lock *rw)
{
        pthread_mutex_lock(&(rw->mutex));
        if (rw->writer == 0) {
                pthread_mutex_unlock(&(rw->mutex));
                return -1;
        } else {
                rw->writer = 0;
                pthread_cond_broadcast(&(rw->cond));
                pthread_mutex_unlock(&(rw->mutex));
                return 0;
        }
}

/* ------------------------------------------------------------------------- */
/* tpool function...                                                         */
/*                                                                           */
/* ------------------------------------------------------------------------- */

LIST_HEAD(tpool_head);
pthread_mutex_t tpool_mutex = PTHREAD_MUTEX_INITIALIZER;

static int runq_size(struct list_head *head)
{
        int cnt = 0;
        struct list_head *p;

        list_for_each(p, head)
                cnt++;
        return cnt;
};

struct tpool *tpool_register(
        struct tpool_operations *op, int num, const char *name)
{
        struct tpool *tpool;

        tpool = calloc(1, sizeof(struct tpool));
        if (tpool == NULL)
                return NULL;

        tpool->p_op = op;
        strcpy(tpool->p_name, name);
        tpool->p_num = num;
        tpool->p_tid = calloc(tpool->p_num, sizeof(pthread_t));
        if (!tpool->p_tid) {
                free(tpool);
                return NULL;
        }

#if 0
        if (tpool->p_op->construct)
                (tpool->p_op->construct)(p);
#else
        pthread_mutex_init(&tpool->p_lock, NULL);
        pthread_cond_init(&tpool->p_empty, NULL);
        pthread_cond_init(&tpool->p_not_empty, NULL);
        pthread_cond_init(&tpool->p_not_full, NULL);
        INIT_LIST_HEAD(&tpool->p_runq);
        tpool->p_level = 0;
        tpool->p_closed = 0;
        tpool->p_shutdown = 0;
#endif

        /* attach tpool node ... */
        pthread_mutex_lock(&tpool_mutex);
        list_add_tail(&tpool->p_list, &tpool_head);
        pthread_mutex_unlock(&tpool_mutex);

        printf("---> (%s:%d) new tpool(%s)\n",
               __func__, __LINE__, tpool->p_name);
        return tpool;
}

struct tpool *tpool_find(const char *name)
{
        struct list_head *p;
        struct tpool *tpool;

        pthread_mutex_lock(&tpool_mutex);
        list_for_each(p, &tpool_head) {
                tpool = list_entry(p, struct tpool, p_list);
                if (!strcmp(tpool->p_name, name)) {
                        pthread_mutex_unlock(&tpool_mutex);
                        return tpool;
                }
        }
        pthread_mutex_unlock(&tpool_mutex);
        return NULL;
}

int tpool_set_handler(struct tpool *tpool, tpool_handler handler)
{
        int i;

        assert(tpool != NULL);

        for (i=0; i<tpool->p_num; i++)
                if (tpool->p_tid[i]) /* handler already running... */
                        return -1;

        if (!handler)
                return -2;

        pthread_mutex_lock(&tpool_mutex);
        tpool->p_handler = handler;

        for (i=0; i<tpool->p_num; i++) {
                if (pthread_create(&(tpool->p_tid[i]),
                                   NULL, tpool->p_handler, (void*)tpool) != 0) {
                        pthread_mutex_unlock(&tpool_mutex);
                        return -3;
                }
        }
        pthread_mutex_unlock(&tpool_mutex);
        return 0;
}

int tpool_unregister(struct tpool *tpool, bool finish)
{
        int i;
        struct list_head *p, *n;
        struct work *work;

        if (!tpool)
                return -1;

        pthread_mutex_lock(&tpool->p_lock);
        if (tpool->p_closed || tpool->p_shutdown) {
                pthread_mutex_unlock(&tpool->p_lock);
                return -2;
        }

        /* detach tpool node ... */
        pthread_mutex_lock(&tpool_mutex);
        assert(tpool->p_list.next != NULL && tpool->p_list.prev != NULL);
        list_del(&tpool->p_list);
        pthread_mutex_unlock(&tpool_mutex);

        /* stop all tpool worker thread ... */
#if 0
        if (tpool->p_op->close)
                (tpool->p_op->close)(p);
#else
        tpool->p_closed = 1;

        /* if the finish flag is set, wait for workers to drain queue */
        if (finish) {
                while (!list_empty(&tpool->p_runq))
                        pthread_cond_wait(&tpool->p_empty, &tpool->p_lock);
        }

        tpool->p_shutdown = 1;

        pthread_mutex_unlock(&tpool->p_lock);
        pthread_cond_broadcast(&tpool->p_not_empty);
        pthread_cond_broadcast(&tpool->p_not_full);
        for (i=0; i<tpool->p_num; i++)
                pthread_join(tpool->p_tid[i], NULL);
#endif

        /* cleanup tpool node ... */
#if 0
        if (tpool->p_op->destroy)
                (tpool->p_op->destroy)(p);
#else
        free(tpool->p_tid);
        list_for_each_safe(p, n, &tpool->p_runq) {
                work = list_entry(p, struct work, list);
                list_del(p);
                free(work);
        }
#endif

        printf("---> (%s:%d) delete tpool(%s)\n",
               __func__, __LINE__, tpool->p_name);
        free(tpool);
        return 0;
}

/* ------------------------------------------------------------------------- */
/* default tpool operations                                                  */
/*                                                                           */
/* ------------------------------------------------------------------------- */

static int default_tpool_construct(struct tpool *p)
{
        NOT_USED(p);
        return 0;
}
static int default_tpool_destroy(struct tpool *p)
{
        NOT_USED(p);
        return 0;
}
static int default_tpool_dispatch(
        struct tpool *tpool, tpool_worker func, void *arg)
{
        int cnt;
        struct work *work;

        pthread_mutex_lock(&tpool->p_lock);
        cnt = runq_size(&tpool->p_runq);

        if (cnt == tpool->p_num) {
                pthread_mutex_unlock(&tpool->p_lock);
                return TPOOL_FULL;
        }

        if (tpool->p_shutdown || tpool->p_closed) {
                pthread_mutex_unlock(&tpool->p_lock);
                return TPOOL_CLOSED;
        }

        work = calloc(1, sizeof(struct work));
        if (!work) {
                pthread_mutex_unlock(&tpool->p_lock);
                return TPOOL_MEM_ERROR;
        }

        work->func = func;
        work->arg = arg;
        list_add_tail(&work->list, &tpool->p_runq);
        if (cnt == 0)
                pthread_cond_signal(&tpool->p_not_empty);

        pthread_mutex_unlock(&tpool->p_lock);
        return 0;
}

static int default_tpool_close(struct tpool *p)
{
        NOT_USED(p);
        return 0;
}

struct tpool_operations default_tpool_operations = {
        .construct = default_tpool_construct,
        .destroy   = default_tpool_destroy,
        .dispatch  = default_tpool_dispatch,
        .close     = default_tpool_close,
};

void *default_tpool_handler(void *arg)
{
        int cnt;
        struct tpool *tpool;
        struct work *work;

        tpool = (struct tpool*)arg;
        printf("---> (%s:%d) tpool(%s) worker start\n",
               __func__, __LINE__, tpool->p_name);
        while (true) {
                pthread_mutex_lock(&tpool->p_lock);
                while (list_empty(&tpool->p_runq) && !tpool->p_shutdown)
                        pthread_cond_wait(&tpool->p_not_empty, &tpool->p_lock);

                if (tpool->p_shutdown) {
                        pthread_mutex_unlock(&tpool->p_lock);
                        printf("---> (%s:%d) pthread_exit\n",__func__, __LINE__);
#if 1
                        break;
#else
                        /* FIXME:
                         *
                         *   There are many valgrind error report if we use
                         *   pthread_exit, why?
                         */
                        pthread_exit(NULL);
#endif
                }

                /* get first one work */
                if (!list_empty(&tpool->p_runq)) {
                        work = list_entry(
                                tpool->p_runq.next, struct work, list);
                        list_del(tpool->p_runq.next);
                }

                cnt = runq_size(&tpool->p_runq);
                if (cnt == tpool->p_num - 1)
                        pthread_cond_signal(&tpool->p_not_full);
                if (cnt == 0)
                        pthread_cond_signal(&tpool->p_empty);
                pthread_mutex_unlock(&tpool->p_lock);

                (work->func)(work->arg);
                free(work);
        }
        return NULL;
}

/* ------------------------------------------------------------------------- */
/* struct my_buffer                                                          */
/*                                                                           */
/* ------------------------------------------------------------------------- */

#define BUFFER_NEW     0
#define BUFFER_ATTACH  1
#define BUFFER_COPY    2

struct my_buffer {
        struct list_head list;
        size_t size;
        int rwi;
        char *data;
};

struct my_buffer *my_buffer_new(char *src, int size, int flag);
struct my_buffer *my_buffer_free(struct my_buffer *buf, bool flag);

struct my_buffer *my_buffer_new(char *src, int size, int flag)
{
        struct my_buffer *b;

        b = (struct my_buffer*)calloc(sizeof(struct my_buffer), 1);
        if (b == NULL)
                return NULL;

        switch (flag) {
        case BUFFER_NEW:
        case BUFFER_COPY:
                b->data = (char*)malloc(size);
                if (b->data == NULL) {
                        free(b);
                        return NULL;
                }
                if (src != NULL)
                        memcpy(b->data, src, size);
                break;
        case BUFFER_ATTACH:
                b->data = src;
                break;
        }
        b->size = size;
        return b;
}

struct my_buffer *my_buffer_free(struct my_buffer *buf, bool flag)
{
        if (buf == NULL)
                return NULL;

        if (buf->data != NULL && flag == true)
                free(buf->data);
        free(buf);
        return NULL;
}

/* ------------------------------------------------------------------------- */
/* cport function...                                                         */
/*                                                                           */
/* ------------------------------------------------------------------------- */

LIST_HEAD(cport_head);
pthread_mutex_t cport_mutex = PTHREAD_MUTEX_INITIALIZER;

struct cport *cport_register(
        struct cport_operations *op, int mode, const char *name)
{
        struct cport *p;

        p = (struct cport*)malloc(sizeof(struct cport));
        if (p == NULL)
                return NULL;

        memset(p, 0x0, sizeof(struct cport));
        p->p_op = op;
        p->p_mode = mode;
        strncpy(p->p_name, name, sizeof(p->p_name)-1);
        if (p->p_op->create)
                (p->p_op->create)(p);
        if (p->p_op->set_handler_control)
                (p->p_op->set_handler_control)(p, NULL, NULL);

        pthread_mutex_lock(&cport_mutex);
        list_add_tail(&p->p_list, &cport_head);
        assert(p->p_list.next != NULL && p->p_list.prev != NULL);
        pthread_mutex_unlock(&cport_mutex);
        printf("===> (%s:%d) new cport(%s)\n", __func__, __LINE__, p->p_name);
        return p;
}

int cport_unregister(struct cport *p)
{
        assert(p != NULL);

        /* detach cport node ... */
        pthread_mutex_lock(&cport_mutex);
        assert(p->p_list.next != NULL && p->p_list.prev != NULL);
        list_del(&p->p_list);
        pthread_mutex_unlock(&cport_mutex);

        /* stop cport thread ... */
        if (p->p_op->close) {
                (p->p_op->close)(p);
        }

        /* cleanup cport node ... */
        if (p->p_op->destroy)
                (p->p_op->destroy)(p);
        printf("===> (%s:%d) delete cport(%s)\n", __func__, __LINE__, p->p_name);
        free(p);
        return 0;
}

struct cport *cport_find(const char *name)
{
        struct list_head *l;
        struct cport *p;

        pthread_mutex_lock(&cport_mutex);
        list_for_each(l, &cport_head) {
                p = list_entry(l, struct cport, p_list);
                if (!strcmp(p->p_name, name)) {
                        pthread_mutex_unlock(&cport_mutex);
                        return p;
                }
        }
        pthread_mutex_unlock(&cport_mutex);
        return NULL;
}

int cport_set_handler(struct cport *p,
                      cport_handler handler, void *op, void *args)
{
        assert(p != NULL);

        if (p->p_tid) /* handler already running... */
                return -1;

        if (!handler)
                return -2;

        p->p_handler = handler;
        if (p->p_op->set_handler_control)
                (p->p_op->set_handler_control)(p, op, args);
        return pthread_create(&p->p_tid, NULL, p->p_handler, (void*)p);
}

/* ------------------------------------------------------------------------- */
/* owdp (oneway datagram cport)                                              */
/*                                                                           */
/* ------------------------------------------------------------------------- */

void default_timeout_pending_prepare(struct cport *p)
{
        struct owdp_control *c;

        assert(p != NULL);
        c = (struct owdp_control*)p->p_priv;
        assert(c != NULL);
}

int default_prepare_timeout(struct timespec* ts)
{
        NOT_USED(ts);
        return -1;
}

void default_execute_timeout_pending(struct cport *p)
{
        struct owdp_control *c;

        assert(p != NULL);
        c = (struct owdp_control*)p->p_priv;
        assert(c != NULL);
}

struct owdp_operations default_owdp_control_operations = {
        .timeout_pending_prepare = default_timeout_pending_prepare,
        .prepare_timeout         = default_prepare_timeout,
        .execute_timeout_pending = default_execute_timeout_pending,
        .func                    = NULL,
};

static int default_owdp_create(struct cport *p)
{
        struct owdp_control *c;

        if (!p->p_priv) {
                c = malloc(sizeof(struct owdp_control));
                if (c == NULL)
                        return -1;

                memset(c, '\0', sizeof(struct owdp_control));
                pthread_mutex_init(&c->c_lock, NULL);
                pthread_cond_init(&c->c_not_empty, NULL);
                INIT_LIST_HEAD(&c->c_runq);
                c->c_level = 0;
                c->c_closed = 0;

                c->c_op = NULL;
                c->c_priv = NULL;

                p->p_priv = c;
        }
        return 0;
}

static int default_owdp_destroy(struct cport *p)
{
        struct owdp_control *c;
        struct my_buffer *b;

        assert(p != NULL);
        if (p->p_priv != NULL) {
                c = (struct owdp_control*)p->p_priv;
                while (!list_empty(&c->c_runq)) {
                        b = list_entry(c->c_runq.next, struct my_buffer, list);
                        list_del(c->c_runq.next);
                        if (b == NULL)
                                my_buffer_free(b, true);
                }
                free(c);
        }
        return 0;
}

static int default_owdp_set_handler_control(
        struct cport *p, void *op, void *args)
{
        struct owdp_control *c;

        assert(p != NULL);
        c = (struct owdp_control*)p->p_priv;
        assert(c != NULL);

        c->c_op = (struct owdp_operations*)op;
        c->c_priv = args;
        if (c->c_op && c->c_op->timeout_pending_prepare)
                (c->c_op->timeout_pending_prepare)(p);
        return 0;
}

/* defalult_owdp_read (consumer)
 *
 */

static ssize_t default_owdp_read(struct cport *cp, void *buf, size_t num)
{
        struct my_buffer *b = NULL;
        struct owdp_control *c;
        struct timespec ts;

        assert(cp != NULL);
        c = (struct owdp_control*)cp->p_priv;
        assert(c != NULL);

        pthread_mutex_lock(&c->c_lock);
        if (list_empty(&c->c_runq)) {
                if (cp->p_mode == PORT_MODE_NONBLOCK) {
                        pthread_mutex_unlock(&c->c_lock);
                        ts.tv_sec = 0;
                        ts.tv_nsec = cp->p_sleep;
                        nanosleep(&ts, NULL);
                        return 0;
                }
                if (c->c_op->prepare_timeout &&
                    (c->c_op->prepare_timeout)(&c->c_wait) >= 0)
                        pthread_cond_timedwait(
                                &c->c_not_empty, &c->c_lock, &c->c_wait);
                else
                        pthread_cond_wait(&c->c_not_empty, &c->c_lock);
        }

        if (c->c_closed == 1) {
                pthread_mutex_unlock(&c->c_lock);
                return -1;
        }

        if (!list_empty(&c->c_runq)) {
                b = list_entry(c->c_runq.next, struct my_buffer, list);
                list_del(c->c_runq.next);
        }
        pthread_mutex_unlock(&c->c_lock);

        if (b == NULL)
                return 0;
        *((void**)buf) = b->data;
        num = b->size;
//        printf("===> (%s) read data: %ld...\n", __func__, num);
        my_buffer_free(b, false);
        return num;
}

/* defalult_owdp_write (producer)
 *
 *   other threads use this function to send datagram to cport.
 */
static ssize_t default_owdp_write(struct cport *p, const void *buf, size_t num)
{
        struct my_buffer *b;
        struct owdp_control *c;

        assert(p != NULL);
        c = (struct owdp_control*)p->p_priv;
        assert(c != NULL);

        /* copy data ... */
        b = my_buffer_new((char*)buf, num, BUFFER_COPY);
        if (b == NULL)
                return -1;

        pthread_mutex_lock(&c->c_lock);

//        printf("===> (%s) write data...\n", __func__);
        list_add_tail(&b->list, &c->c_runq);
        if (!list_empty(&c->c_runq))
                pthread_cond_signal(&c->c_not_empty);
        pthread_mutex_unlock(&c->c_lock);
        return num;
}

static int default_owdp_close(struct cport *p)
{
        struct owdp_control *c;

        assert(p != NULL);
        c = (struct owdp_control*)p->p_priv;
        assert(c != NULL);

        pthread_mutex_lock(&c->c_lock);
        c->c_closed = 1;
        pthread_mutex_unlock(&c->c_lock);
        pthread_cond_broadcast(&c->c_not_empty);
        pthread_join(p->p_tid, NULL);

        return 0;
}

struct cport_operations default_owdp_operations = {
        .create              = default_owdp_create,
        .destroy             = default_owdp_destroy,
        .set_handler_control = default_owdp_set_handler_control,
        .read                = default_owdp_read,
        .write               = default_owdp_write,
        .close               = default_owdp_close,
};

void *default_owdp_handler(void *arg)
{
        struct cport *p = (struct cport*)arg;
        struct owdp_control *c;
        void *dg;
        int num = 0;

        assert(p != NULL);
        c = (struct owdp_control*)p->p_priv;
        assert(c != NULL);

        for (;c->c_closed == 0;) {
                if (c->c_op->execute_timeout_pending)
                        (c->c_op->execute_timeout_pending)(p);
                if (p->p_op->read) {
                        /* use data pointer pass directly... */
                        num = (p->p_op->read)(p, (void*)&dg, 0);
                }
                if (num == -1 && c->c_closed == 1)
                        break;
                if (num > 0) {
                        if (c->c_op->func) {
//                                printf("===> (%s) call ...\n", __func__);
                                (c->c_op->func)(p, dg, num);
                        }
                        free(dg);
                }
        }
        return NULL;
}
