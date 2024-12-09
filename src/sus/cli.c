#include <argp.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

const char *argp_program_version = "0.1.0";
const char *argp_program_bug_address =
    "André Willyan <willyanpublic@protonmail.com> and Said Rodrigues "
    "<coderflemis@gmail.com>";
static char doc[] = "SUS - Simple Unix Sort";
static char args_doc[] = "";

static struct argp_option options[] = {
    {"number-of-clients", 'n', 0, 0,
     "Number of clients that will be scheduled."},
    {"max-number-of-processes", 'p', 0, 0,
     "Max number of concurrent client processes."},
    {"path-to-client-process", 'c', 0, 0, "Path to client process."},
    {"due-date-in-us", 'd', 0, 0, "Due date for the project."},
    {"seed", 's', 0, 0, "Seed for random number generation."},
    {0}};

struct arguments {
  uint64_t number_of_clients;
  uint8_t max_number_of_processes;
  uint64_t due_date_in_us;
  char *path_to_client_process;
  uint64_t seed;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = state->input;
  switch (key) {
  case 'n':
    arguments->number_of_clients = atoi(arg);
    break;
  case 'p':
    arguments->max_number_of_processes = atoi(arg);
    break;
  case 'c':
    arguments->path_to_client_process = arg;
    break;
  case 'd':
    arguments->due_date_in_us = atoi(arg);
    break;
  case 's':
    arguments->seed = atoi(arg);
    break;
  case ARGP_KEY_ARG:
    return 0;
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};

static void set_default_arguments(struct arguments *arguments) {
  arguments->number_of_clients = 1;
  arguments->max_number_of_processes = 4;
  arguments->due_date_in_us = 0;
  arguments->path_to_client_process = "./client";
  arguments->seed = 42069;
}
