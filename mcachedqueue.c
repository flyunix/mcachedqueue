#include "mcachedqueue.h"
#include "embed_assert.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

int 
mcached_queue_init(mcached_queue_t *queue, int item_size, int item_cnt)
{
    EMBED_ASSERT_RETURN(queue, EMBED_FAILD);
    
    queue->idle_list = &(queue->_idle_list);
    queue->used_list = &(queue->_used_list);

    INIT_LIST_HEAD(queue->idle_list);
    INIT_LIST_HEAD(queue->used_list);

    queue->max_item_cnt = 0;
    queue->used_item_cnt = 0;

    queue->mlattr = &(queue->_mlattr);
    queue->mlock = &(queue->_mlock);

    if(pthread_mutexattr_init(queue->mlattr) != 0) {
        ("pthread_mutexaddr_init failed:%s", strerror(errno));
        return -1;
    }
    
    if(pthread_mutexattr_settype(queue->mlattr, PTHREAD_MUTEX_RECURSIVE_NP) != 0) {
        printf("pthread_mutexattr_settype failed:%s", strerror(errno));
        return -1;
    }

    if(pthread_mutex_init(queue->mlock, queue->mlattr) != 0) {
        printf("pthread_mutex_init failed:%s", strerror(errno));
        return -1;
    }

    if(embed_ready_event_create(&(queue->ready_event)) != 0) {
        printf("embed_ready_event_create failed");
        return -1;
    }

    queue->mem_cached = calloc(item_cnt, item_size);

    if(queue->mem_cached == NULL) {
        printf("calloc mem for mcached_queue_t failed");
        return -1;
    }

    int i = 0;
    struct list_head *pos = NULL;

    for (; i < item_cnt; i++) {
        pos = (struct list_head*)(queue->mem_cached + i * item_size);
        assert(pos);

        list_add_tail(pos, queue->idle_list);
    }

    return 0;
}

int
mcached_queue_destroy(mcached_queue_t *queue)
{
    assert(queue);

    queue->max_item_cnt = 0;
    queue->used_item_cnt = 0;

    pthread_mutex_destroy(queue->mlock);
    pthread_mutexattr_destroy(queue->mlattr);
    
    if(queue->mem_cached) {
        free(queue->mem_cached);
        queue->mem_cached = NULL;
    }

    return 0;
}

/*
 * @breif:add new_item to list->used_list
 *
 * @note:new_item MUST get from, list->idle_list,and MUST lock list
 */

int
mcached_queue_add(mcached_queue_t *queue, struct list_head *new_item)
{
    assert(queue && new_item);

    MCACHED_QUEUE_LOCK(queue);

    list_move(new_item, queue->used_list);

    queue->used_item_cnt++;
    queue->max_item_cnt--;
    
    embed_ready_event_active(queue->ready_event);

    MCACHED_QUEUE_UNLOCK(queue);

    return 0;
}

void
mcached_queue_del(mcached_queue_t *queue, struct list_head *del_item)
{
    assert(queue && del_item);

    MCACHED_QUEUE_LOCK(queue);

    list_move(del_item, queue->idle_list);

    queue->used_item_cnt--;
    queue->max_item_cnt++;
    
    embed_ready_event_update(queue->ready_event);

    MCACHED_QUEUE_UNLOCK(queue);
}

int
mcached_queue_get_idle_item(mcached_queue_t *queue, struct list_head **item)
{
    assert(queue&& item); 

    int ret = 0;
    MCACHED_QUEUE_LOCK(queue);

    //list is full
    if(IS_IDLE_LIST_EMPTY(queue)) {
        printf("idle list is full\n");
        *item = NULL;
        ret = -1;
        goto ret_err;
    }

    *item = queue->idle_list->next;

ret_err:
    MCACHED_QUEUE_UNLOCK(queue);
    return ret;
}

void 
mcached_queue_traverse(mcached_queue_t *queue, traverse_item_cb item_handler)
{
    assert(queue && item_handler);    

    struct list_head *pos, *n;

    embed_ready_event_wait(queue->ready_event);

    MCACHED_QUEUE_LOCK(queue);

    list_for_each_safe(pos, n, queue->used_list) {
        item_handler(queue, pos);
    }   

    MCACHED_QUEUE_UNLOCK(queue);
}

/*
 * @breif:遍历队列,根据find_index查找对应的表项
 *
 * @func:mcached_queue_find
 *
 * @param:queue:被查找的队列
 *
 * @param:find_index:查找对象的依据
 *
 * @param:compared:比较函数
 *
 * @param:find_item:用于保存查找到的表项,如果没有找到其为NULL
 *
 * @return: success
 */

int 
mcached_queue_find(
        mcached_queue_t *queue, 
        void *find_index,
        find_compared_cb compared, 
        struct list_head **find_item
        )
{
    assert(queue && compared && find_index && find_item);

    struct list_head *pos, *n;

    MCACHED_QUEUE_LOCK(queue);

    list_for_each_safe(pos, n, queue->used_list) {
        if(compared(pos, find_index)) {
            *find_item = pos;
            goto err_ret;
        }  
    }

    *find_item = NULL;

err_ret:
    MCACHED_QUEUE_UNLOCK(queue);
    return 0;
}
