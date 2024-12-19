#ifndef CLIENT_H

#define CLIENT_H

#include <sys/time.h>
#include <sys/types.h>

typedef enum ClientPriority {
  High,
  Standard,
} ClientPriority;

// Client Process Tuple
typedef struct ClientProcess {
  // Client PID for later SIGCONT
  pid_t pid;
  // Date of arrive of a client
  struct timeval ts;
  // Time to attend
  int time_to_attend;
  // Client priority
  ClientPriority priority;
} ClientProcess;

#endif // !CLIENT_H
