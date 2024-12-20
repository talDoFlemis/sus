#ifndef RECEPTION_H

#define RECEPTION_H

#include "scheduler/include/edf.h"
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>

typedef enum ReceptionMode {
  Live,
  Batch,
} ReceptionMode;

typedef struct Reception {
  uint8_t max_number_of_processes;
  uint64_t number_of_clients;
  ReceptionMode mode;
  char *path_to_client_process;
  EDF *scheduler;
  long patience_usec;
} Reception;

Reception *create_new_reception(uint64_t number_of_clients,
                                uint8_t max_number_of_processes,
                                char *path_to_client_process, EDF *scheduler,
                                long patience_usec);

void start_reception(Reception *self);

pthread_t spawn_reception_thread(Reception *self);

#endif // !RECEPTION_H
