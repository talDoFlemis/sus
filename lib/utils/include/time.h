#ifndef TIME_H

#define TIME_H

#include <sys/time.h>

// Get current time in a monotonic clock
void get_now(struct timeval *ts);
// Elapsed time in nanoseconds
long elapsed_time(struct timeval start, struct timeval end);
// Elapsed time in nanoseconds until now
long elapsed_time_until_now(struct timeval start);

#endif // !TIME_H
