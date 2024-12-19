#include "service/include/service.h"
#include "attendant/include/attendant.h"
#include "reception/include/reception.h"
#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdatomic.h>
#include <stdlib.h>

const char *attendant_semaphore_path = "/sem_atend";
const char *lng_semaphore_path = "/sem_block";

Service *create_new_service(Reception *reception, Attendant *attendant) {
  Service *service = malloc(sizeof(Service));
  service->reception = reception;
  service->attendant = attendant;
  return service;
};

atomic_int client_stream_ended = ATOMIC_VAR_INIT(0);

pid_t start_service_process(Service *self) {
  pid_t pid = fork();
  assert(pid >= 0 && "failed to spawn service process");

  // Child process
  if (pid == 0) {
    pthread_t reception_thread = spawn_reception_thread(self->reception);
    pthread_t attendant_thread = spawn_attendant_thread(self->attendant);

    int join_status = pthread_join(reception_thread, NULL);
    assert(join_status != -1 && "failed to join reception thread on service");

    int join_status2 = pthread_join(attendant_thread, NULL);
    assert(join_status2 != -1 && "failed to join attendant thread on service");

    exit(EXIT_SUCCESS);
  }
  // Parent process
  else {
    return pid;
  }
}
