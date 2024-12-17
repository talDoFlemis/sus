#include "attendant/include/attendant.h"
#include "client/include/client.h"
#include "fcntl.h"
#include "semaphore.h"
#include "signal.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/time.h"

Attendant create_attendant(EDF *scheduler, pid_t analist_pid,
                           char *lng_file_path) {
  Attendant att;

  sem_t *sem_scheduler = sem_open("/sem_scheduler", O_RDONLY);
  sem_t *sem_atend = sem_open("/sem_atend", O_RDWR);
  sem_t *sem_block = sem_open("/sem_block", O_WRONLY);

  if (sem_scheduler == SEM_FAILED || sem_atend == SEM_FAILED ||
      sem_block == SEM_FAILED) {
    printf("Failed to create attendant. The sem_nxtclient, sem_atend or "
           "sem_block semaphores may be unavailable.");
    exit(1);
  }

  FILE *lng_file = fopen(lng_file_path, "a");
  if (lng_file == NULL) {
    printf("Failed to open the LNG file.");
    exit(1);
  }

  att.sem_scheduler = sem_scheduler;
  att.sem_atend = sem_atend;
  att.sem_block = sem_block;
  att.scheduler = scheduler;
  att.attended_count = 0;
  att.satisfied_count = 0;

  att.lng_file = lng_file;
  return att;
}

void attend_client(Attendant *att, ClientProcess *client,
                   const long patience_usec) {
  // execute client
  kill(client->pid, SIGCONT);
  sem_wait(att->sem_atend);

  // write client PID in the LNG file
  sem_wait(att->sem_block);
  fwrite(&client->pid, sizeof(pid_t), 1, att->lng_file);
  sem_post(att->sem_block);

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

void start_attedant(Attendant *att, unsigned long patience_usec) {
  struct sigaction sigterm_action;
  memset(&sigterm_action, 0, sizeof(sigterm_action));
  sigterm_action.sa_handler = handle_sigterm;
  sigaction(SIGTERM, &sigterm_action, NULL);

  while (1) {
    // get next client in the scheduler
    sem_wait(att->sem_scheduler);
    ClientProcess *client = dequeue(att->scheduler, patience_usec);
    sem_post(att->sem_scheduler);

    if (client == NULL && stop) {
      // if received a sigterm and all clients have been attended.
      kill(att->analyst_pid, SIGCONT);
      exit(0);
    } else {
      attend_client(att, client, patience_usec);
      // at each 10 clients...
      if (att->attended_count % 10 == 0) {
        kill(att->analyst_pid, SIGCONT);
      }
    }
  }
}
