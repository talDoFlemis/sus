#include "analyst/include/analyst.h"
#include "attendant/include/attendant.h"
#include "cli.c"
#include "reception/include/reception.h"
#include "service/include/service.h"
#include <argp.h>
#include <assert.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>

int main(int argc, char *argv[]) {
  struct timeval start_time, end_time;
  gettimeofday(&start_time, NULL);

  struct arguments arguments;

  set_default_arguments(&arguments);
  argp_parse(&argp, argc, argv, 0, 0, &arguments);

  // Initiate reception
  printf("Arguments:\n");
  printf("Number of clients: %lu\n", arguments.number_of_clients);
  printf("Max number of processes: %u\n", arguments.max_number_of_processes);
  printf("Path to client process: %s\n", arguments.path_to_client_process);
  printf("Due date in us: %lu\n", arguments.patience_in_us);
  printf("Seed: %lu\n", arguments.seed);
  printf("Path to LNG file: %s\n", arguments.path_to_lng_file);
  printf("Max number of integers to read: %u\n",
         arguments.max_number_of_int_to_read);

  printf("Creating unamed semaphores...\n");
  sem_t sem_scheduler;
  sem_init(&sem_scheduler, 0, 1);
  assert(&sem_scheduler != SEM_FAILED &&
         "failed to create semaphore for scheduler");

  sem_t *sem_block = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE,
                          MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  assert(sem_block != MAP_FAILED &&
         "failed to create shared semaphore for block");
  sem_init(sem_block, 1, 1);
  assert(sem_block != SEM_FAILED && "failed to create semaphore for block");

  printf("Creating named semaphore for block...\n");
  sem_t *sem_atend = sem_open("/sem_atend", O_RDWR);

  if (sem_atend == SEM_FAILED) {
    sem_unlink("/sem_atend");
    sem_atend = sem_open("/sem_atend", O_CREAT | O_RDWR);
    assert(sem_atend != SEM_FAILED &&
           "failed to create named semaphore for atend");
  }

  printf("Creating scheduler...\n");
  EDF *scheduler = create_edf();

  printf("Creating analyst process...\n");
  Analyst *analyst =
      create_analyst(arguments.path_to_lng_file,
                     arguments.max_number_of_int_to_read, sem_block);
  pid_t analyst_pid = create_analyst_process(analyst);
  printf("Analyst PID: %d\n", analyst_pid);
  printf("Read integers processed by analyst on /proc/%d/fd/1\n", analyst_pid);
  printf("\tTIP: tail -f /proc/%d/fd/1\n", analyst_pid);

  printf("Creating reception...\n");
  Reception *reception = create_new_reception(
      arguments.number_of_clients, arguments.max_number_of_processes,
      arguments.path_to_client_process, scheduler, &sem_scheduler,
      arguments.patience_in_us);

  printf("Creating attendant...\n");
  Attendant *attendant = create_attendant(
      scheduler, analyst_pid, arguments.path_to_lng_file,
      arguments.patience_in_us, &sem_scheduler, sem_block, sem_atend);

  printf("Creating service...\n");
  Service *service = create_new_service(reception, attendant);
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

  gettimeofday(&end_time, NULL);
  long int elapsed_time_in_usec =
      ((end_time.tv_sec * 1000000) + end_time.tv_usec) -
      ((start_time.tv_sec * 1000000) + start_time.tv_usec);
  printf("Service finished processing all clients in %lu"
         "us\n",
         elapsed_time_in_usec);

  struct rusage usage;
  getrusage(RUSAGE_SELF, &usage);

  double user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1e6;
  double system_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1e6;

  printf("Time in user mode: %.6f s\n", user_time);
  printf("Time in kernel mode: %.6f s\n", system_time);
  printf("Block output operations: %ld times\n", usage.ru_oublock);
  printf("Page faults: %ld times\n", usage.ru_majflt);
  printf("Voluntary context switches: %ld times\n", usage.ru_nvcsw);
  printf("Involuntary context switches: %ld times\n", usage.ru_nivcsw);

  printf("Destroying semaphores...\n");
  sem_destroy(&sem_scheduler);
  sem_destroy(sem_block);
  sem_close(sem_atend);
  sem_unlink("/sem_atend");

  return EXIT_SUCCESS;
}
