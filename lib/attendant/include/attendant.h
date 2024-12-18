#ifndef ATTENDANT_H

#define ATTENDANT_H
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
  pid_t pid_buffer[10];
  size_t pid_buffer_size;
  unsigned long patience_usec;
} Attendant;

Attendant create_attendant(EDF *scheduler, pid_t analist_pid,
                           char *lng_file_path, unsigned long patience_usec);
void start_attedant(Attendant *att);

pthread_t spawn_attendant_thread(Attendant *self);

#endif // !ATTENDANT_H
