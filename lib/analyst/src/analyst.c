#include "analyst/include/analyst.h"
#include <assert.h>
#include <sched.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

Analyst create_analyst(char *lng_file_path, uint8_t max_number_of_int_to_read,
                       FILE *output_stream) {
  assert(lng_file_path != NULL && "empty lng file path");

  Analyst analyst = {
      .current_line_number = 0,
      .lng_file_path = lng_file_path,
      .max_number_of_int_to_read = max_number_of_int_to_read,
      .output_stream = output_stream,
  };

  return analyst;
}

void start_analyst(Analyst *self) {
  FILE *lng_file = fopen(self->lng_file_path, "rb");
  assert(lng_file != NULL && "failed to open lng file");

  raise(SIGSTOP);

  pid_t buffer[self->max_number_of_int_to_read];
  unsigned long remaining_int;

  do {
    remaining_int =
        fread(buffer, sizeof(pid_t), self->max_number_of_int_to_read, lng_file);
    for (uint8_t i = 0; i < remaining_int; i++) {
      fprintf(self->output_stream, "%d\n", buffer[i]);
    }

    if (remaining_int == self->max_number_of_int_to_read) {
      raise(SIGSTOP);
    }

  } while (remaining_int == self->max_number_of_int_to_read);

  fflush(self->output_stream);

  int fclose_status = fclose(lng_file);
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
