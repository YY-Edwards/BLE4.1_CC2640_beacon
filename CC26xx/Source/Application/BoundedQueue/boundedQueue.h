#ifndef _BOUNDED_QUEUE_H_
#define _BOUNDED_QUEUE_H_


#ifdef __cplusplus
extern "C"
{
#endif
  

#include "stdbool.h"
#include "stdlib.h"
#include "string.h"

#define QUEUEDEEP 10
#define DATADEEP  64
typedef enum
{
    queue_ok,
    queue_null,
    element_null,
    queue_full,
    queue_empty,
}QueueSta_t;

#pragma   pack(1)
typedef struct
{
  char  data[DATADEEP];
  int   len ;
}fixed_size_packet_t;

typedef struct
{
   int   len ;
  char  *data;
}dyn_size_packet_t;

typedef struct
{
    fixed_size_packet_t         queue_array[QUEUEDEEP];
    unsigned short              head;
    unsigned short              tail;

    
} ring_queue_t;

typedef struct
{
    unsigned short head;
    unsigned short tail;
    unsigned short data_size;
    unsigned short queue_deep;
    dyn_size_packet_t *queue_point;
   
} dyn_ring_queue_t;

//typedef ring_queue_t* RingQueue_t;
typedef dyn_ring_queue_t* RingQueue_t;

//bool init_queue(RingQueue_t ring_queue);
RingQueue_t create_queue(unsigned int deep, unsigned int size);
bool take_from_queue(RingQueue_t ring_queue, void *buf, int *len, bool erase);
bool push_to_queue(RingQueue_t ring_queue, void *buf, int len);
void clear_queue(RingQueue_t ring_queue);
bool queue_is_empty(RingQueue_t ring_queue);

//typedef struct
//{
//    unsigned short deep;  
//    unsigned short elementsize;
//    unsigned short front;
//    unsigned short rear;
//    unsigned short count;
//    unsigned short index;
//    unsigned char * store;
//} QueueStr_t;

#pragma   pack()
//typedef QueueStr_t * Queue_t;
//
//
//Queue_t QueueCreate(unsigned short deep, unsigned short elementsize );
//QueueSta_t QueueDelete(Queue_t queue);
//QueueSta_t QueuePush(Queue_t queue, void * element);
//QueueSta_t QueuePull(Queue_t queue, void * element);


#ifdef __cplusplus
}
#endif


#endif