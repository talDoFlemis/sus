#ifndef TIME_H

#define TIME_H

#include <stdint.h>
#include <time.h>

// Get current time in a monotonic clock
void get_now(struct timespec *ts);
// Elapsed time in nanoseconds
uint64_t elapsed_time(struct timespec start, struct timespec end);
// Elapsed time in nanoseconds until now
uint64_t elapsed_time_until_now(struct timespec start);

#endif // !TIME_H
