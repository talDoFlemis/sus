#include "reception/include/reception.h"
#include "client/include/client.h"
#include "scheduler/include/edf.h"
#include "utils/include/time.h"
#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

extern atomic_int client_stream_ended;
extern pthread_mutex_t scheduler_mutex;
atomic_int stop_reception = ATOMIC_VAR_INIT(0);

ClientProcess *create_client_process(Reception *self) {
  int random_number = (double)rand() / (double)RAND_MAX;
  ClientPriority client_priority;

  if (random_number < 0.5) {
    client_priority = High;
  } else {
    client_priority = Standard;
  }

  pid_t pid = fork();
  assert(pid >= 0 && "failed to spawn client process");

  // Child process
  if (pid == 0) {
    int exec_status = execv(self->path_to_client_process, NULL);
    assert(exec_status != 0 && "failed to exec client process");

    exit(EXIT_SUCCESS);
  }
  // Parent process
  else {
    ClientProcess *client_process = malloc(sizeof(ClientProcess));
    client_process->pid = pid;
    client_process->priority = client_priority;

    int status;
    waitpid(client_process->pid, &status, WUNTRACED);
    assert(WIFSTOPPED(status) == 1 && "client process didn't stopped");

    FILE *demanda = fopen("demanda.txt", "r");
    assert(demanda != NULL && "failed to open demanda file");

    int time_to_attend;
    int fscanf_status = fscanf(demanda, "%d", &time_to_attend);
    assert(fscanf_status == 1 &&
           "failed to read single integer from demanda file");

    client_process->time_to_attend = time_to_attend;
    get_now(&client_process->ts);

    int fclose_status = fclose(demanda);
    assert(fclose_status == 0 && "failed to close demanda file");

    return client_process;
  }
}

Reception *create_new_reception(uint64_t number_of_clients,
                                uint8_t max_number_of_processes,
                                char *path_to_client_process, EDF *scheduler,
                                long patience_usec) {
  assert(number_of_clients >= 0 &&
         "number of clients should be greater than or equal to 0");
  assert(max_number_of_processes >= 1 &&
         "max number of clients should be greater than or equal to 1");

  Reception *reception = malloc(sizeof(Reception));
  assert(reception != NULL && "failed to create reception");

  uint64_t size_of_queue = max_number_of_processes;
  if (number_of_clients != 0 && number_of_clients < max_number_of_processes) {
    size_of_queue = number_of_clients;
  }

  ReceptionMode mode = Live;
  if (number_of_clients != 0) {
    mode = Batch;
  }

  reception->max_number_of_processes = max_number_of_processes;
  reception->number_of_clients = number_of_clients;
  reception->mode = mode;
  reception->path_to_client_process = path_to_client_process;
  reception->scheduler = scheduler;
  reception->patience_usec = patience_usec;

  return reception;
};

void add_new_client_process(Reception *self) {
  pthread_mutex_lock(&scheduler_mutex);

  ClientProcess *client_process = create_client_process(self);
  insert(self->scheduler, client_process, self->patience_usec);

  pthread_mutex_unlock(&scheduler_mutex);
}

void *thread_wrapper(void *ptr) {
  Reception *self = (Reception *)ptr;
  start_reception(self);
  return NULL;
}

void *input_thread(void *arg) {
  Reception *r_args = (Reception *)arg;

  while (1) {
    char input;
    input = getchar();

    if (input == 's' || input == 'S') {
      atomic_store(&stop_reception, 1);
      break;
    }

    while (getchar() != '\n')
      ;
  }

  return NULL;
}

void start_reception(Reception *self) {
  if (self->mode == Batch) {
    while (self->number_of_clients != 0) {

      while (self->scheduler->size == EDF_MAX_ITEMS)
        continue;

      add_new_client_process(self);

      self->number_of_clients--;
    }
    atomic_store(&client_stream_ended, 1);
  } else {
    pthread_t t;
    int thread_status = pthread_create(&t, NULL, input_thread, (void *)self);
    assert(thread_status == 0 && "failed to input thread");

    while (!atomic_load(&stop_reception)) {
      while (self->scheduler->size == EDF_MAX_ITEMS)
        continue;
      add_new_client_process(self);
    }
    atomic_store(&client_stream_ended, 2);
  }
};

pthread_t spawn_reception_thread(Reception *self) {
  pthread_t t;

  int thread_status = pthread_create(&t, NULL, thread_wrapper, (void *)self);
  assert(thread_status == 0 && "failed to create reception thread");

  return t;
};
