#ifndef RECEPTION_H

#define RECEPTION_H

#include "client/include/client.h"
#include <pthread.h>
#include <stdint.h>

typedef enum ReceptionMode {
  Live,
  Batch,
} ReceptionMode;

typedef struct Reception {
  uint8_t max_number_of_processes;
  uint64_t number_of_clients;
  ClientProcess *queue;
  ReceptionMode mode;
  uint8_t remaining_clients_on_queue;
  uint8_t queue_size;
} Reception;

Reception *create_new_reception(uint64_t number_of_clients,
                                uint8_t max_number_of_processes);

void *start_reception(Reception *self);

pthread_t spawn_reception_thread(Reception *self);

#endif // !RECEPTION_H
