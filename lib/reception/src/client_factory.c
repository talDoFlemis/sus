#include "reception/include/client_factory.h"
#include "client/include/client.h"
#include <assert.h>
#include <signal.h>
#include <stdlib.h>

pid_t spawn_new_sleepy_client_process(u_int64_t sleep_time_in_us) {
  pid_t pid = fork();
  assert(pid >= 0 && "failed to spawn client process");

  // Child process
  if (pid == 0) {
    int raise_value = raise(SIGSTOP);
    assert(raise_value == 0 && "failed to sigstop child process");

    client(sleep_time_in_us);

    exit(EXIT_SUCCESS);
  }
  // Parent process
  else {
    return pid;
  }
}

ClientFactory create_client_factory(uint64_t sleep_time_in_us) {
  ClientFactory client_factory = {
      .sleep_time_in_us = sleep_time_in_us,
  };
  return client_factory;
}

ClientProcess create_client_process(ClientFactory factory) {
  int random_number = (double)rand() / (double)RAND_MAX;
  ClientPriority client_priority;

  if (random_number < 0.5) {
    client_priority = High;
  } else {
    client_priority = Standard;
  }

  pid_t client_pid = spawn_new_sleepy_client_process(factory.sleep_time_in_us);

  ClientProcess client_process = {
      .pid = client_pid,
      .priority = client_priority,
      .random_number = random_number * 10e4,
  };

  return client_process;
}

ClientProcess *create_client_processe_in_bulk(ClientFactory factory,
                                              uint64_t num_processes) {
  assert(num_processes > 0 && "number of processes must be greater than 0");

  ClientProcess *client_processes =
      malloc(sizeof(ClientProcess) * num_processes);
  assert(client_processes != NULL &&
         "failed to create client_processes container");

  for (int i = 0; i < num_processes; i++) {
    client_processes[i] = create_client_process(factory);
  }

  return client_processes;
}
