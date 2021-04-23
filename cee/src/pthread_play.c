//
// Created by lukebayes on 4/23/21.
//

#include "pthread_play.h"
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

typedef struct Channel {
  int limit;
  int in;
  int out;
}Channel;

void *my_thread(void *vargp) {
  Channel *ch = (Channel *)vargp;
  if (ch == NULL) {
    return NULL;
  }

  printf("after check limit: %d\n", ch->limit);
  // printf("inside thread: %d\n", ch->out);
  while (ch->in < ch->limit) {
    printf("thread out: %d\n", ch->in);

    sleep(1);
    ch->out++;
  }
}

int thread_play_start(void) {
  Channel ch = {0};
  ch.in = 0;
  ch.out = 0;
  ch.limit = 5;

  // printf("thread_play_start %d\n", ch->out);
  pthread_t thread_id;
  printf("before thread\n");
  pthread_create(&thread_id, NULL, my_thread, &ch);

  while (ch.out < ch.limit) {
    printf("master in: %d\n", ch.in);
    sleep(1);
    ch.in++;
  }

  /*
  for (int i = 0; i < 5; i++) {
    ch->out = i;
    while (ch->in != i) {
      sleep(1);
    }
  }
 */

  pthread_join(thread_id, NULL);
  printf("after thread %d %d\n", ch.in, ch.out);
  return 0;
}
