#include "cli.c"
#include "reception/include/reception.h"
#include "service/include/service.h"
#include "utils/include/time.h"
#include <argp.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>

int main(int argc, char *argv[]) {
  struct timespec start;
  get_now(&start);

  struct arguments arguments;

  set_default_arguments(&arguments);
  argp_parse(&argp, argc, argv, 0, 0, &arguments);

  // Initiate reception
  printf("Arguments:\n");
  printf("Number of clients: %lu\n", arguments.number_of_clients);
  printf("Max number of processes: %u\n", arguments.max_number_of_processes);
  printf("Path to client process: %s\n", arguments.path_to_client_process);
  printf("Due date in us: %lu\n", arguments.due_date_in_us);
  printf("Seed: %lu\n", arguments.seed);

  printf("Creating reception...\n");
  Reception *reception = create_new_reception(arguments.number_of_clients,
                                              arguments.max_number_of_processes,
                                              arguments.path_to_client_process);

  // Create service
  printf("Creating service...\n");
  Service *service = create_new_service(reception);
  pid_t service_pid = start_service_process(service);

  printf("Service PID: %d\n", service_pid);
  if (arguments.number_of_clients == 0)
    printf("Send 's' to /proc/%d/fd/0 to stop the service", service_pid);

  printf("Waiting for service to finish processing all clients...\n");
  int service_status;

  waitpid(service_pid, &service_status, 0);
  assert(WIFEXITED(service_status) == 1 && "service didn't exit normally");
  assert(WEXITSTATUS(service_status) == EXIT_SUCCESS &&
         "service didn't exit with success");

  uint64_t duration_time_in_ns = elapsed_time_until_now(start);

  printf("Service finished processing all clients in %lu ns\n",
         duration_time_in_ns);

  return EXIT_SUCCESS;
}
