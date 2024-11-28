#ifndef SCHEDULER_H

#define SCHEDULER_H

#include "reception/include/client_factory.h"

// Scheduler trait definition
typedef struct {
  void (*const add_new_client)(void *self, ClientProcess *client_process);
  ClientProcess *(*const get_next_client)(void *self);
} Scheduler;

// Scheduler trait instance
typedef struct {
  void *self;
  Scheduler const *tc;
} Schedulable;

#define impl_scheduler(T, Name, add_f) Schedulable Name(T x) {
char *(*const show_)(T e) = (add_f);
(void)show_;
static Scheduler const tc = {.show = (char *(*const)(void *))(add_f)};
return (Schedulable){.tc = &tc, .self = x};
}

#endif // !SCHEDULER_H
