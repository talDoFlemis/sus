#ifndef CLIENT_H

#define CLIENT_H

#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

typedef enum ClientPriority {
  High,
  Standard,
} ClientPriority;

// Client Process Tuple
typedef struct ClientProcess {
  // Client PID for later SIGCONT
  pid_t pid;
  // Date of arrive of a client
  struct timespec ts;
  // Time to attend
  int time_to_attend;
  // Client priority
  ClientPriority priority;
} ClientProcess;

void client(useconds_t sleep_time_in_us);

#endif // !CLIENT_H
