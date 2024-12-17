#include "attendant/include/attendant.h"
#include "client/include/client.h"
#include "fcntl.h"
#include "semaphore.h"
#include "signal.h"
#include "stdio.h"
#include "stdlib.h"
#include "sys/time.h"

Attendant create_attendant(EDF *scheduler, pid_t analist_pid,
                           char *lng_file_path) {
  Attendant att;

  sem_t *nxtclient = sem_open("/sem_nxtclient", O_RDONLY);
  sem_t *atend = sem_open("/sem_atend", O_RDWR);
  sem_t *block = sem_open("/sem_block", O_WRONLY);

  if (nxtclient == SEM_FAILED || atend == SEM_FAILED || block == SEM_FAILED) {
    printf("Failed to create attendant. The sem_nxtclient, sem_atend or "
           "sem_block semaphores may be unavailable.");
    exit(1);
  }

  FILE *lng_file = fopen(lng_file_path, "a");
  if (lng_file == NULL) {
    printf("Failed to open the LNG file.");
    exit(1);
  }

  att.sem_nxtclient = nxtclient;
  att.sem_atend = atend;
  att.sem_block = block;
  att.scheduler = scheduler;
  att.attended_count = 0;
  att.satisfied_count = 0;

  att.lng_file = lng_file;
  return att;
}

void attend_next(Attendant *att, const long patience_usec) {
  // get next client in the scheduler
  sem_wait(att->sem_nxtclient);
  ClientProcess *client = dequeue(att->scheduler, patience_usec);
  sem_post(att->sem_nxtclient);

  // if there is no more clients wake up the analyst
  // TODO: fix scenario where the analist consumes < 10 PIDs and, in further
  // iterations, the analist will consume < 10 PIDs each time.
  // HOW: the analist should only be allowed to consume < 10 PIDs in the last
  // iteration.
  if (client == NULL) {
    kill(att->analyst_pid, SIGCONT);
    return;
  }

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

  // wake the analyst at each 10 clients
  if (att->attended_count % 10 == 0) {
    kill(att->analyst_pid, SIGCONT);
  }
}
