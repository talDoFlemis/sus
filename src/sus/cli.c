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
    {"number-of-clients", 'n', "NUMBER", 0,
     "Number of clients that will be scheduled."},
    {"max-number-of-processes", 'p', "NUMBER", 0,
     "Max number of concurrent client processes."},
    {"path-to-client-process", 'c', "CLIENT_PROCESS", 0, "Path to client process."},
    {"patience-in-us", 't', "NUMBER", 0, "Patience for the project in usec."},
    {"seed", 's', "NUMBER", 0, "Seed for random number generation."},
    {"lng-file-path", 'l', "FILE_PATH", 0, "Path to LNG file."},
    {"max-number-of-ints-to-read", 'm', "NUMBER", 0,
     "Max number of integers a analyst can read at once."},
    {0}};

struct arguments {
  uint64_t number_of_clients;
  uint8_t max_number_of_processes;
  uint64_t patience_in_us;
  char *path_to_client_process;
  char *path_to_lng_file;
  uint8_t max_number_of_int_to_read;
  uint64_t seed;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = state->input;
  switch (key) {
  case 'n':
    if (arg) arguments->number_of_clients = strtoull(arg, NULL, 10);
    break;
  case 'p':
    if (arg) arguments->max_number_of_processes = (uint8_t)strtoul(arg, NULL, 10);
    break;
  case 'c':
    arguments->path_to_client_process = arg;
    break;
  case 't':
    if (arg) arguments->patience_in_us = strtoull(arg, NULL, 10);
    break;
  case 's':
    if (arg) arguments->seed = strtoull(arg, NULL, 10);
    break;
  case 'l':
    arguments->path_to_lng_file = arg;
    break;
  case 'm':
    if (arg) arguments->max_number_of_int_to_read = (uint8_t)strtoul(arg, NULL, 10);
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
  arguments->patience_in_us = 100;
  arguments->path_to_client_process = "./client";
  arguments->seed = 42069;
  arguments->path_to_lng_file = "./lng";
  arguments->max_number_of_int_to_read = 10;
}
