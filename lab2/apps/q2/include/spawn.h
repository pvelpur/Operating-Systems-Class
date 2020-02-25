#ifndef __USERPROG__
#define __USERPROG__

typedef struct circular_buf {
  char data [5];
  int head;
  int tail;
} circular_buf;

#define FILENAME_PRODUCER "producer.dlx.obj"
#define FILENAME_CONSUMER "consumer.dlx.obj"

#endif
