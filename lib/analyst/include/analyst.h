#ifndef ANALYST_H

#define ANALYST_H

#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

typedef struct Analyst {
  uint8_t max_number_of_int_to_read;
  sem_t *sem_block;
  FILE *lng_file;
} Analyst;

Analyst *create_analyst(char *lng_file_path, uint8_t max_number_of_int_read,
                       sem_t *sem_block);

void start_analyst(Analyst *self);

pid_t create_analyst_process(Analyst *self);

#endif // !ANALYST_H
