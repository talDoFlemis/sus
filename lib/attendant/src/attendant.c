#include "attendant/include/attendant.h"
#include "client/include/client.h"
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>

Attendant *create_attendant(EDF *scheduler, pid_t analist_pid,
                            char *lng_file_path, unsigned long patience_usec,
                            sem_t *sem_scheduler, sem_t *sem_block,
                            sem_t *sem_atend) {
  Attendant *att = malloc(sizeof(Attendant));
  assert(att != NULL && "failed to allocate memory for attendant");

  FILE *lng_file = fopen(lng_file_path, "w");
  assert(lng_file != NULL && "failed to open lng file");

  att->sem_scheduler = sem_scheduler;
  att->sem_atend = sem_atend;
  att->sem_block = sem_block;
  att->scheduler = scheduler;
  att->attended_count = 0;
  att->satisfied_count = 0;
  att->lng_file = lng_file;
  att->patience_usec = patience_usec;
  return att;
}

void attend_client(Attendant *att, ClientProcess *client,
                   const long patience_usec) {
  assert(client != NULL && "attending null client");

  // execute client
  kill(client->pid, SIGCONT);
  sem_wait(att->sem_atend);

  // add PID to the buffer
  att->pid_buffer[att->pid_buffer_size] = client->pid;
  ++att->pid_buffer_size;

  ++att->attended_count;

  // calculate satisfaction
  struct timeval tv;
  gettimeofday(&tv, NULL);
  long curr_time_usec = (tv.tv_sec * 1000000) + tv.tv_usec;
  long started_usec =
      (client->ts.tv_sec * 1000000) + (client->ts.tv_nsec / 1000);
  long time_span = curr_time_usec - started_usec;
  if (time_span <= patience_usec) {
    ++att->satisfied_count;
  }

  // free client
  free(client);
}

extern atomic_int client_stream_ended;

void start_attedant(Attendant *att) {
  while (1) {
    // get next client in the scheduler
    sem_wait(att->sem_scheduler);
    ClientProcess *client = dequeue(att->scheduler, att->patience_usec);
    sem_post(att->sem_scheduler);

    if (client == NULL && atomic_load(&client_stream_ended)) {
      // if received a sigterm and all clients have been attended.
      if (att->pid_buffer_size > 0) {
        sem_wait(att->sem_block);
        fwrite(att->pid_buffer, sizeof(pid_t), att->pid_buffer_size,
               att->lng_file);
        fflush(att->lng_file);
        sem_post(att->sem_block);

        kill(att->analyst_pid, SIGCONT);
      }

      // Wait for analyst to finish processing
      int analyst_status;
      waitpid(att->analyst_pid, &analyst_status, 0);
      assert(WIFEXITED(analyst_status) == 1 && "analyst didn't exit normally");
      assert(WEXITSTATUS(analyst_status) == EXIT_SUCCESS &&
             "analyst didn't exit with success");

      fclose(att->lng_file);
      sem_close(att->sem_block);
      sem_close(att->sem_atend);
      sem_close(att->sem_scheduler);
      exit(0);
    } else if (client == NULL) {
      continue;
    } else {
      attend_client(att, client, att->patience_usec);
      // at each 10 clients...
      if (att->pid_buffer_size == 10) {
        sem_wait(att->sem_block);
        fwrite(att->pid_buffer, sizeof(pid_t), 10, att->lng_file);
        fflush(att->lng_file);
        sem_post(att->sem_block);

        att->pid_buffer_size = 0;

        kill(att->analyst_pid, SIGCONT);
      }
    }
  }
}

void *attendant_thread_wrapper(void *ptr) {
  Attendant *self = (Attendant *)ptr;
  start_attedant(self);
  return NULL;
}

pthread_t spawn_attendant_thread(Attendant *self) {
  pthread_t t;

  int thread_status =
      pthread_create(&t, NULL, attendant_thread_wrapper, (void *)self);
  assert(thread_status == 0 && "failed to create reception thread");

  return t;
};
