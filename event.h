#ifndef _OS_H_
#define _OS_H_

#include "type.h"

#include <pthread.h>

/**
 * @defgroup EMBED_HAS_REDAY_NOTIFY_OBJ
 * @{
 *
 */

/**
 * This moudle provides abstraction to Reday event mechanism
 * Ready event obj can be used for synchronizatino among (queue list
 * and so on)
 */
    
#define Free(ptr) \
    do {\
        if(ptr){ \
            free(ptr);\
            ptr = NULL;\
        }\
    }while(0)\

typedef struct{
    pthread_mutex_t mutex;
    pthread_cond_t  cond;

    uint16 nready;
}embed_ready_event_t;

embed_status_t embed_ready_event_create(embed_ready_event_t **ready_event);

embed_status_t embed_ready_event_wait(embed_ready_event_t *ready_event);

embed_status_t embed_ready_event_active(embed_ready_event_t *ready_event);

embed_status_t embed_ready_event_update(embed_ready_event_t *ready_event);

embed_status_t embed_ready_event_destroy(embed_ready_event_t *ready_event);
/**
 *
 * @}
 */


/**
 * @defgroup EMBED_HAS_EVENT_OBJ
 * @{
 */

/**
 *  This moudle provides abstraction to event object(Linux).
 *  Event object can be used for synchronization among threads
 */

typedef enum 
{
    EV_STATE_OFF,
    EV_STATE_SET,
    EV_STATE_PULSED
}embed_event_state;

typedef struct 
{
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    embed_event_state state;

    bool auto_reset;    
    uint16 threads_waiting;
    uint16 threads_to_release;
}embed_event_t;

embed_status_t embed_event_create(embed_event_t **event, 
        embed_bool_t manual_reset, embed_bool_t initial);

embed_status_t embed_evnet_wait(embed_event_t *event);

embed_status_t embed_event_trywait(embed_event_t *event);

embed_status_t embed_event_set(embed_event_t *event);

embed_status_t embed_event_pulse(embed_event_t *event);

embed_status_t embed_event_reset(embed_event_t *event);

embed_status_t embed_event_destroy(embed_event_t *event);

/**
 * @defgroup EMBED_HAS_RWLOCK_OBJ
 * @{
 *
 */

/**
 * This moudle provides abstraction to POSIX rwlock
 */
typedef struct {
    pthread_rwlock_t rwlock;
}embed_rwlock_t;

embed_status_t embed_rwlock_create(embed_rwlock_t **rw_mutex);

embed_status_t embed_rwlock_lock_read(embed_rwlock_t *rw_mutex);

embed_status_t embed_rwlock_lock_write(embed_rwlock_t *rw_mutex);

embed_status_t embed_rwlock_unlock_read(embed_rwlock_t *rw_mutex);

embed_status_t embed_rwlock_unlock_write(embed_rwlock_t *rw_mutex);

embed_status_t embed_rwlock_lock_destroy(embed_rwlock_t *rw_mutex);

#endif
