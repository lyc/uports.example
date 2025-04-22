#ifndef __THREAD_H__
#define __THREAD_H__

#include <pthread.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>

#include "list.h"

/* ------------------------------------------------------------------------- */
/* reader/writer lock support                                                */
/* ------------------------------------------------------------------------- */

struct rw_lock {
        pthread_mutex_t         mutex;
        pthread_cond_t          cond;
        int                     reader;
        int                     writer;
};

void rw_lock_init(struct rw_lock *rw);
int rw_rlock(struct rw_lock *rw);      /* reader lock */
int rw_runlock(struct rw_lock *rw);    /* reader unlock */
int rw_wlock(struct rw_lock *rw);      /* writer lock */
int rw_wunlock(struct rw_lock *rw);    /* writer unlock */

/* ------------------------------------------------------------------------- */
/* struct tpool                                                              */
/* ------------------------------------------------------------------------- */

enum {
        TPOOL_FULL      = -1,
        TPOOL_CLOSED    = -2,
        TPOOL_MEM_ERROR = -3,
};

typedef int (*tpool_worker)(void *arg);

struct work {
        struct list_head        list;
        tpool_worker            func;
        void                    *arg;
};

typedef void *(*tpool_handler)(void *arg);

struct tpool {
        struct list_head        p_list;
        char                    p_name[32];

        int                     p_num;       /* the number of  worker */
        pthread_t               *p_tid;
        tpool_handler           p_handler;

        pthread_mutex_t         p_lock;
        pthread_cond_t          p_empty;
        pthread_cond_t          p_not_empty;
        pthread_cond_t          p_not_full;
        struct list_head        p_runq;
        int                     p_level;    /* buffer water level */
        int                     p_closed;
        int                     p_shutdown;

        struct tpool_operations *p_op;
        void                    *p_priv;
};

struct tpool_operations {
        int (*construct)(struct tpool *tpool);
        int (*destroy)(struct tpool *tpool);
        int (*dispatch)(struct tpool *tpool, tpool_worker func, void *arg);
        int (*close)(struct tpool *tpool);

};

struct tpool *tpool_register(
        struct tpool_operations *op, int num, const char *name);
struct tpool *tpool_find(const char *name);
int tpool_set_handler(struct tpool *tpool, tpool_handler handler);
int tpool_unregister(struct tpool *tpool, bool finish);

extern struct tpool_operations default_tpool_operations;
void *default_tpool_handler(void *arg);

/* ------------------------------------------------------------------------- */
/* struct cport                                                              */
/* ------------------------------------------------------------------------- */

#define PORT_MODE_BLOCK     0
#define PORT_MODE_NONBLOCK  1

typedef void *(*cport_handler)(void *arg);

struct cport {
        struct list_head        p_list;
        char                    p_name[32];

        int                     p_mode;
        long                    p_sleep; /* nano seconds */

        /* Normally, cport need to have its own thread handler
         * and corresponding control block, right now this control block
         * embedded in p_priv.
         */
        pthread_t               p_tid;
        cport_handler           p_handler;

        struct cport_operations *p_op;
        void                    *p_priv;
};

struct cport_operations {
        int (*create)(struct cport* p);
        int (*destroy)(struct cport* p);
        int (*set_handler_control)(struct cport* p, void *op, void *args);
        ssize_t (*read)(struct cport* p, void* buf, size_t num);
        ssize_t (*write)(struct cport* p, const void* buf, size_t num);
        int (*close)(struct cport* p);
};

struct cport *cport_register(
        struct cport_operations *op, int mode, const char *name);
struct cport *cport_find(const char *name);
int cport_set_handler(struct cport *p,
                      cport_handler handler, void *op, void *args);
int cport_unregister(struct cport *p);

/* ------------------------------------------------------------------------- */
/* owdp (oneway datagram cport)                                               */
/* ------------------------------------------------------------------------- */

struct owdp_control {
        pthread_mutex_t         c_lock;
        pthread_cond_t          c_not_empty;
        struct list_head        c_runq;
        int                     c_level;   /* buffer water level */
        int                     c_closed;
	struct timespec         c_wait; /* for pthread_cond_timewait */

        struct owdp_operations  *c_op;
        void                    *c_priv;
};

struct owdp_operations {
        void (*timeout_pending_prepare)(struct cport *p);
        int (*prepare_timeout)(struct timespec *ts);
        void (*execute_timeout_pending)(struct cport *p);
        int (*func)(struct cport *p, void *buf, int num);
};

int default_prepare_timeout(struct timespec *ts);
void default_execute_timeout_pending(struct cport *p);
extern struct owdp_operations default_owdp_control_operations;

void *default_owdp_handler(void *arg);
extern struct cport_operations default_owdp_operations;

#endif /* __THREAD_H__ */
