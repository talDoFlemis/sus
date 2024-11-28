#ifndef SERVICE_H

#define SERVICE_H

#include <unistd.h>

typedef struct Service {
} Service;

// Creates a new service using a given reception and attendant
Service *create_new_service();

// Start a service in a different process
pid_t start_service_process(Service *service);

#endif // !SERVICE_H
