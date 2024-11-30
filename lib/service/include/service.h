#ifndef SERVICE_H

#define SERVICE_H

#include "reception/include/reception.h"
#include <unistd.h>

typedef struct Service {
  Reception *reception;
} Service;

// Creates a new service using a given reception and attendant
Service *create_new_service(Reception *reception);

// Start a service in a different process
pid_t start_service_process(Service *self);

#endif // !SERVICE_H
