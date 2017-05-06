#ifndef __MCACHEDLIST_H_
#define __MCACHEDLIST_H_

#include "list.h"
#include "event.h"
#include "assert.h"

#include <pthread.h>

typedef struct 
{
    struct list_head _idle_list;
    struct list_head _used_list;

    struct list_head *idle_list;
    struct list_head *used_list;

    char   *mem_cached;

    int    max_item_cnt;
    int    used_item_cnt;

    pthread_mutex_t _mlock;
    pthread_mutex_t *mlock;

    pthread_mutexattr_t _mlattr;
    pthread_mutexattr_t *mlattr;

    embed_ready_event_t *ready_event;
}mcached_queue_t;

typedef void (*traverse_item_cb)(mcached_queue_t *mcached_queue, struct list_head *item);
typedef bool (*find_compared_cb)(struct list_head *queue_item,  void *find_item);

#define IS_IDLE_LIST_EMPTY(list) \
    (\
     ((list)->used_item_cnt >= (list)->max_item_cnt) \
     &&\
     ((list_empty((list)->idle_list)))\
    ) \

#define MCACHED_QUEUE_LOCK(queue) \
do \
{\
    pthread_mutex_lock((queue)->mlock);\
}while(0)

#define MCACHED_QUEUE_UNLOCK(queue) \
do \
{\
    pthread_mutex_unlock((queue)->mlock);\
}while(0)


int
mcached_queue_init(mcached_queue_t *queue, int item_size, int item_cnt);

int 
mcached_queue_destroy(mcached_queue_t *queue);

int
mcached_queue_add(mcached_queue_t *queue, struct list_head *new_item);

void
mcached_queue_del(mcached_queue_t *queue, struct list_head *del_item);

int
mcached_queue_get_idle_item(mcached_queue_t *queue, struct list_head **item);

void 
mcached_queue_traverse(mcached_queue_t *queue, traverse_item_cb item_handler);

int 
mcached_queue_find(
        mcached_queue_t *queue, 
        void *find_index,
        find_compared_cb compared, 
        struct list_head **find_item
        );

#endif

