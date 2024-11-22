#include "utils/include/time.h"

void get_now(struct timespec *ts) { clock_gettime(CLOCK_MONOTONIC, ts); }

uint64_t elapsed_time(struct timespec start, struct timespec end) {
  uint64_t diff = end.tv_nsec - start.tv_nsec;
  return diff;
}

uint64_t elapsed_time_until_now(struct timespec start) {
  struct timespec end;
  get_now(&end);
  return elapsed_time(start, end);
}
