#ifndef SERVICE_H

#define SERVICE_H

#include "reception/include/reception.h"
#include "attendant/include/attendant.h"
#include <unistd.h>

typedef struct Service {
  Reception *reception;
  Attendant attendant;
} Service;

// Creates a new service using a given reception and attendant
Service *create_new_service(Reception *reception, Attendant attendant);

// Start a service in a different process
pid_t start_service_process(Service *self);

#endif // !SERVICE_H
