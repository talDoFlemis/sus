#include "utils/include/time.h"
#include <stdlib.h>
#include <sys/time.h>

void get_now(struct timeval *ts) { gettimeofday(ts, NULL); }

long elapsed_time(struct timeval start, struct timeval end) {
  return ((end.tv_sec * 1000000) + end.tv_usec) -
         ((start.tv_sec * 1000000) + start.tv_usec);
}

long elapsed_time_until_now(struct timeval start) {
  struct timeval end;
  gettimeofday(&end, NULL);
  return elapsed_time(start, end);
}
