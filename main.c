#include "mcachedqueue.h"

typedef struct {
    struct list_head list;
    int a;
}test_item_t;

void traverse_item(mcached_queue_t* mcached_queue, struct list_head *new_item)
{
    //test_item_t *item = list_entry(new_item, test_item_t, list);
    test_item_t *item = (test_item_t*)new_item;

    if(item == NULL) {
        printf("list_entry failed");
    }

    printf("a = %d\n", item->a); 

    if(item->a == 5) {
        mcached_queue_del(mcached_queue, new_item);
        printf("\n\n");
    }
}

int main(void)
{
    mcached_queue_t _mcached_queue, *mcached_queue;

    mcached_queue = &_mcached_queue;

    //Init mcached_queue
    mcached_queue_init(mcached_queue, sizeof(test_item_t), 10);

    printf("mcached_queue_init success \n");

    struct list_head *new_item;

    int i = 0;

    //Add data into mcached_queue
    for (;i < 10; i++) {
        mcached_queue_get_idle_item(mcached_queue, &new_item);

        if(new_item == NULL) {
            printf("mcached_queue_get failed\n");
            return -1;
        }

        test_item_t *item = (test_item_t*)new_item;

        if(item == NULL) {
            printf("list_entry failed");
        }

        item->a = i;

        mcached_queue_add(mcached_queue, new_item);
    }

    //traverse macached_queue
    mcached_queue_traverse(mcached_queue, traverse_item);

    return 0;
}
