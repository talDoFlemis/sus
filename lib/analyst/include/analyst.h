#ifndef ANALYST_H

#define ANALYST_H

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

typedef struct Analyst {
  uint64_t current_line_number;
  uint8_t max_number_of_int_to_read;
  char *lng_file_path;
  FILE *output_stream;
} Analyst;

Analyst create_analyst(char *lng_file_path, uint8_t max_number_of_int_read,
                       FILE *output_stream);

void start_analyst(Analyst *self);

pid_t create_analyst_process(Analyst *self);

#endif // !ANALYST_H
