#include "event.h"
#include "embed_assert.h"

#include <stdlib.h>

/**
 * Create event object
 *
 * @param event Pointer to hold the returned event object
 * @param manual_reset Specify whether the evnet is manual-reset
 * @param initial      Specify the intial state of the event object
 *
 * @return evnet handle, or NULL if failed
 */
embed_status_t 
embed_event_create(embed_event_t **ptr_event, 
        embed_bool_t manual_reset, embed_bool_t initial)
{
    pthread_mutexattr_t attr;
    
    embed_event_t *event;

    event = calloc(1, sizeof(embed_event_t)); 

    if(event == NULL) {
        printf("Calloc embed_event_t failed");
        return EMBED_FAILD;
    }

    /*set mutex attr*/
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
    pthread_mutex_init(&event->mutex, &attr); 
    pthread_mutexattr_destroy(&attr);

    /*init cond*/
    pthread_cond_init(&event->cond, 0);
    event->auto_reset = !manual_reset;
    event->threads_waiting = 0;

    if(initial) {
        event->state = EV_STATE_SET;
        event->threads_to_release = 1;
    } else {
        event->state = EV_STATE_OFF;
        event->threads_to_release = 0;
    }
    *ptr_event = event;

    return EMBED_SUCCESS;
}

static void
event_on_one_release(embed_event_t *event)
{
    if(event->state == EV_STATE_SET) {
        if (event->auto_reset) {
            event->threads_to_release = 0;
            event->state = EV_STATE_OFF;
        } else {
            /* Manual reset remains on */
        }
    } else {
        if (event->auto_reset) {
            /*Only release one*/
            event->threads_to_release = 0;
            event->state = EV_STATE_OFF;
        } else {
            event->threads_to_release--;
            EMBED_ASSERT(event->threads_to_release >= 0, "EMBED_EINVAL");
            if (event->threads_to_release==0)
                event->state = EV_STATE_OFF;
        }
    }
}

/**
 * Wait for event to be signaled
 *
 * @param event The event object 
 *
 * @return  EMBED_SUCCESS or EMBED_FAILD if failed
 */
embed_status_t
embed_event_wait(embed_event_t *event)
{
    pthread_mutex_lock(&event->mutex);
    event->threads_waiting++;
    while(event->state == EV_STATE_OFF)
        pthread_cond_wait(&event->cond, &event->mutex);
    event->threads_waiting--;
    event_on_one_release(event);
    pthread_mutex_unlock(&event->mutex);
    return EMBED_SUCCESS;
}

/**
 * Try wait for event to be signaled
 *
 * @param event The event object 
 *
 * @return  EMBED_SUCCESS or EMBED_FAILD if failed
 */
embed_status_t
embed_event_trywait(embed_event_t *event)
{
    embed_status_t status;

    pthread_mutex_lock(&event->mutex);
    status = event->state == EV_STATE_SET ? EMBED_SUCCESS : EMBED_FAILD;
    if (status == EMBED_SUCCESS)
        event_on_one_release(event);
    pthread_mutex_unlock(&event->mutex);

    return status;
}

/**
 * Set the event object state to signaled. For auto-reset event, this 
 * will only release the first thread that are waiting on the event. For
 * manual reset event, the state remains signaled until the event is reset.
 * If there is no thread waiting on the event, the event object state 
 * remains signaled.
 *
 * @param event	    The event object.
 *
 * @return EMBED_SUCCESS if successfull.
 */
embed_status_t
embed_event_set(embed_event_t *event)
{
    pthread_mutex_lock(&event->mutex);
    event->threads_to_release = 1;
    event->state = EV_STATE_SET;
    if(event->auto_reset) {
        pthread_cond_signal(&event->cond);
    } else {
        pthread_cond_broadcast(&event->cond);
    }
    pthread_mutex_unlock(&event->mutex);
    return EMBED_SUCCESS;
}

/**
 * Set the event object to signaled state to release appropriate number of
 * waiting threads and then reset the event object to non-signaled. For
 * manual-reset event, this function will release all waiting threads. For
 * auto-reset event, this function will only release one waiting thread.
 *
 * @param event	    The event object.
 *
 * @return EMBED_SUCCESS if successfull.
 */
embed_status_t 
embed_event_pulse(embed_event_t *event)
{
    pthread_mutex_lock(&event->mutex);
    if (event->threads_waiting) {
        event->threads_to_release = event->auto_reset ? 1 :
            event->threads_waiting;
        event->state = EV_STATE_PULSED;
        if (event->threads_to_release==1) {
            pthread_cond_signal(&event->cond);
        } else {
            pthread_cond_broadcast(&event->cond);
        }
    }
    pthread_mutex_unlock(&event->mutex);
    return EMBED_SUCCESS;
}

/**
 * Set the event object state to non-signaled.
 *
 * @param event	    The event object.
 *
 * @return EMBED_SUCCESS if successfull.
 */
embed_status_t
embed_event_reset(embed_event_t *event)
{
    pthread_mutex_lock(&event->mutex);    
    event->state = EV_STATE_OFF;
    event->threads_to_release = 0;
    pthread_mutex_unlock(&event->mutex);

    return EMBED_SUCCESS;
}

/**
 * Destroy the event object.
 *
 * @param event	    The event object.
 *
 * @return EMBED_SUCCESS if successfull.
 */
embed_status_t
embed_event_destroy(embed_event_t *event)
{
    pthread_mutex_destroy(&event->mutex);    
    pthread_cond_destroy(&event->cond);

    Free(event);
    return EMBED_SUCCESS;
}

/**
 * Create ready event obj
 *
 * @param ready_event Pointer to hold the returned event object
 *
 * @return EMBED_SUCCESS,or EMBED_FAILD if failed
 */

embed_status_t 
embed_ready_event_create(embed_ready_event_t **ptr_ready_event)
{
    pthread_mutexattr_t attr;
    embed_ready_event_t *ready_event;    

    ready_event = calloc(1, sizeof(embed_ready_event_t));

    if(ready_event == NULL) {
        printf("Calloc embed_ready_notify_t failed");
    }

    /*set mutex attr*/
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
    pthread_mutex_init(&ready_event->mutex, &attr); 
    pthread_mutexattr_destroy(&attr);

    /*init cond*/
    pthread_cond_init(&ready_event->cond, 0);

    *ptr_ready_event = ready_event; 

    return EMBED_SUCCESS;
}

/** Wait for event to be signaled
 *
 * @param event The event object 
 *
 * @return  EMBED_SUCCESS or EMBED_FAILD if failed
 */

embed_status_t 
embed_ready_event_wait(embed_ready_event_t *ready_event)
{
    pthread_mutex_lock(&ready_event->mutex);
    while(ready_event->nready == 0)//防止产生虚假的唤醒
        pthread_cond_wait(&ready_event->cond, &ready_event->mutex);
    pthread_mutex_unlock(&ready_event->mutex); 

    return EMBED_SUCCESS;    
}

/** 
 * If ready == 0, set the event object state to signaled.
 */
embed_status_t 
embed_ready_event_active(embed_ready_event_t *ready_event)
{
    pthread_mutex_lock(&ready_event->mutex);
    if(ready_event->nready == 0) {
        pthread_cond_signal(&ready_event->cond); 
    }
    ready_event->nready++;
    pthread_mutex_unlock(&ready_event->mutex);

    return EMBED_SUCCESS; 
}

/**
 * Update ready_event nready
 */
embed_status_t 
embed_ready_event_update(embed_ready_event_t *ready_event)
{
    pthread_mutex_lock(&ready_event->mutex);
    ready_event->nready--;
    pthread_mutex_unlock(&ready_event->mutex);

    return EMBED_SUCCESS;
}

/**
 * Detroy embed_ready_notify_t object
 */
embed_status_t
embed_ready_event_destroy(embed_ready_event_t *ready_event)
{
    pthread_mutex_destroy(&ready_event->mutex);
    pthread_cond_destroy(&ready_event->cond);

    Free(ready_event);

    return EMBED_SUCCESS;
}

embed_status_t 
embed_rwlock_create(embed_rwlock_t **rw_mutex)
{
    embed_status_t status;

    embed_rwlock_t *rwlock;

    rwlock = calloc(1, sizeof(embed_rwlock_t));

    if(rwlock == NULL) {
        return EMBED_FAILD;
    }

    status = pthread_rwlock_init(&rwlock->rwlock, NULL);   

    if(status != EMBED_SUCCESS)
        return status;

    *rw_mutex = rwlock;

    return EMBED_SUCCESS;
}

embed_status_t 
embed_rwlock_lock_read(embed_rwlock_t *rw_mutex)
{
    embed_status_t status;

    status = pthread_rwlock_rdlock(&rw_mutex->rwlock);

    if(status != EMBED_SUCCESS) 
        return status;

    return EMBED_SUCCESS;
}

embed_status_t 
embed_rwlock_lock_write(embed_rwlock_t *rw_mutex)
{
    embed_status_t status;    

    status = pthread_rwlock_wrlock(&rw_mutex->rwlock);

    if(status != EMBED_SUCCESS)
        return status;

    return EMBED_SUCCESS;
}

embed_status_t 
embed_rwlock_unlock_read(embed_rwlock_t *rw_mutex)
{
    return embed_rwlock_unlock_write(rw_mutex);
}

embed_status_t 
embed_rwlock_unlock_write(embed_rwlock_t *rw_mutex)
{
    embed_status_t status;    

    status = pthread_rwlock_unlock(&rw_mutex->rwlock);

    if(status != EMBED_SUCCESS)
        return status;

    return EMBED_SUCCESS;
}

embed_status_t 
embed_rwlock_lock_destroy(embed_rwlock_t *rw_mutex)
{
    embed_status_t status;    

    status = pthread_rwlock_destroy(&rw_mutex->rwlock);

    if(status != EMBED_SUCCESS)
        return status;

    return EMBED_SUCCESS;
}
