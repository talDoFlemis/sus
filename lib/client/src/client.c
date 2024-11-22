#include "client/include/client.h"
#include <stdlib.h>
#include <unistd.h>

void client(useconds_t sleep_time_in_us) {
  usleep(sleep_time_in_us);
  exit(EXIT_SUCCESS);
}
