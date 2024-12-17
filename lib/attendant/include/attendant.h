#include "scheduler/include/edf.h"
#include "semaphore.h"
#include "stdio.h"

typedef struct {
  sem_t *sem_scheduler;
  sem_t *sem_atend;
  sem_t *sem_block;
  EDF *scheduler;
  unsigned int attended_count;
  unsigned int satisfied_count;
  pid_t analyst_pid;
  FILE *lng_file;
} Attendant;

Attendant create_attendant(EDF *scheduler, pid_t analist_pid,
                           char *lng_file_path);
void attend_next(Attendant *att, const long patience_usec);
