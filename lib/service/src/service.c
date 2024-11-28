#include "service/include/service.h"
#include <assert.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdlib.h>

const char *attendant_semaphore_path = "/sem_atend";
const char *lng_semaphore_path = "/sem_block";

Service *create_new_service() {
  Service *service = malloc(sizeof(Service));
  return service;
};

pid_t start_service_process(Service *service) {
  // Open attendant semaphore
  sem_t *sem_attendant =
      sem_open(attendant_semaphore_path, O_CREAT | O_EXCL, 0644, 1);
  int status;

  assert(sem_attendant != SEM_FAILED &&
         "failed to create semaphore for attendant");

  // Open lng semaphore
  sem_t *sem_lng = sem_open(lng_semaphore_path, O_CREAT | O_EXCL, 0644, 1);
  assert(sem_lng != SEM_FAILED && "failed to create semaphore for lng");

  pid_t pid = fork();
  assert(pid >= 0 && "failed to spawn service process");

  // Child process
  if (pid == 0) {
    exit(EXIT_SUCCESS);
  }
  // Parent process
  else {
    return pid;
  }
}
