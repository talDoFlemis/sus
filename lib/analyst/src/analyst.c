#include "analyst/include/analyst.h"
#include <assert.h>
#include <fcntl.h>
#include <sched.h>
#include <semaphore.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

Analyst *create_analyst(char *lng_file_path, uint8_t max_number_of_int_to_read,
                        sem_t *sem_block) {
  assert(lng_file_path != NULL && "empty lng file path");
  assert(max_number_of_int_to_read > 0 &&
         "max number of int to read must be greater than 0");
  Analyst *analyst = malloc(sizeof(Analyst));
  assert(analyst != NULL && "failed to allocate memory for analyst");

  FILE *lng_file = fopen(lng_file_path, "a+");
  assert(lng_file != NULL && "failed to open lng file");

  analyst->max_number_of_int_to_read = max_number_of_int_to_read;
  analyst->sem_block = sem_block;
  analyst->lng_file = lng_file;

  return analyst;
}

void start_analyst(Analyst *self) {
  raise(SIGSTOP);

  pid_t buffer[self->max_number_of_int_to_read];
  unsigned long remaining_int;

  do {
    sem_wait(self->sem_block);

    remaining_int = fread(buffer, sizeof(pid_t),
                          self->max_number_of_int_to_read, self->lng_file);
    for (uint8_t i = 0; i < remaining_int; i++) {
      printf("%d\n", buffer[i]);
    }

    sem_post(self->sem_block);
    rewind(self->lng_file);

    if (remaining_int == self->max_number_of_int_to_read) {
      raise(SIGSTOP);
    }

  } while (remaining_int == self->max_number_of_int_to_read);

  int fclose_status = fclose(self->lng_file);
  assert(fclose_status == 0 && "failed to close lng file");
};

pid_t create_analyst_process(Analyst *self) {
  pid_t pid = fork();
  assert(pid >= 0 && "failed to spawn analyst process");

  // Child process
  if (pid == 0) {
    start_analyst(self);
    exit(EXIT_SUCCESS);
  }
  // Parent process
  else {
    return pid;
  };
};
