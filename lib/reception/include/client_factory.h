#ifndef CLIENT_FACTORY_H

#define CLIENT_FACTORY_H

#include <stdint.h>
#include <unistd.h>

// Factory
typedef struct ClientFactory {
  uint64_t sleep_time_in_us;
} ClientFactory;

ClientFactory create_client_factory(uint64_t sleep_time_in_us);

typedef enum ClientPriority {
  High,
  Standard,
} ClientPriority;

// Client Process Tuple
typedef struct ClientProcess {
  // Client PID for later SIGCONT
  pid_t pid;
  // Small random number to be used in the client process
  int8_t random_number;
  // Client priority
  ClientPriority priority;
} ClientProcess;

// Create a new client process, send a sigstop to it and return its
// client_process
ClientProcess create_client_process(ClientFactory factory);
// Create a new client process in bulk
ClientProcess *create_client_processe_in_bulk(ClientFactory factory,
                                              uint64_t num_processes);

#endif // !CLIENT_FACTORY_H
