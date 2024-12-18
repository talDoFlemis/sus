#include "attendant/include/attendant.h"
#include "client/include/client.h"
#include "fcntl.h"
#include "semaphore.h"
#include "signal.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/time.h"
#include <assert.h>
#include <pthread.h>

Attendant *create_attendant(EDF *scheduler, pid_t analist_pid,
                            char *lng_file_path, unsigned long patience_usec,
                            sem_t *sem_scheduler) {
  Attendant *att = malloc(sizeof(Attendant));

  sem_t *sem_atend = sem_open("/sem_atend", O_RDWR);
  sem_t *sem_block = sem_open("/sem_block", O_WRONLY);

  if (sem_atend == SEM_FAILED || sem_block == SEM_FAILED) {
    printf("Failed to create attendant. The sem_nxtclient, sem_atend or "
           "sem_block semaphores may be unavailable.");
    exit(1);
  }

  FILE *lng_file = fopen(lng_file_path, "w");
  if (lng_file == NULL) {
    printf("Failed to open the LNG file.");
    exit(1);
  }

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
}

volatile sig_atomic_t stop = 0;

void handle_sigterm(int signum) { stop = 1; }

void start_attedant(Attendant *att) {
  struct sigaction sigterm_action;
  memset(&sigterm_action, 0, sizeof(sigterm_action));
  sigterm_action.sa_handler = handle_sigterm;
  sigaction(SIGTERM, &sigterm_action, NULL);

  while (1) {
    // get next client in the scheduler
    sem_wait(att->sem_scheduler);
    ClientProcess *client = dequeue(att->scheduler, att->patience_usec);
    sem_post(att->sem_scheduler);

    if (client == NULL && stop) {
      // if received a sigterm and all clients have been attended.
      sem_wait(att->sem_block);
      fwrite(att->pid_buffer, sizeof(pid_t), att->pid_buffer_size,
             att->lng_file);
      sem_post(att->sem_block);

      kill(att->analyst_pid, SIGCONT);
      exit(0);
    } else {
      attend_client(att, client, att->patience_usec);
      // at each 10 clients...
      if (att->pid_buffer_size == 10) {
        sem_wait(att->sem_block);
        fwrite(att->pid_buffer, sizeof(pid_t), 10, att->lng_file);
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
