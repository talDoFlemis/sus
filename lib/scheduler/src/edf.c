#include "client/include/client.h"
#include "limits.h"
#include "stdio.h"
#include "stdlib.h"
#include "sys/time.h"

#define EDF_MAX_ITEMS 128

long priority_rank(ClientProcess *client, const long patience_usec) {
  long started_usec =
      (client->ts.tv_sec * 1000000) + (client->ts.tv_nsec / 1000);
  long deadline;
  switch (client->priority) {
  case High:
    deadline = started_usec + patience_usec / 2;
    break;
  case Standard:
    deadline = started_usec + patience_usec;
    break;
  }
  struct timeval tv;
  gettimeofday(&tv, NULL);
  long curr_time_usec = (tv.tv_sec * 1000000) + tv.tv_usec;
  long time_to_deadline = curr_time_usec - deadline;
  if (time_to_deadline < client->time_to_attend) {
    return LONG_MAX;
  } else {
    return time_to_deadline;
  }
}

typedef struct {
  ClientProcess *items[EDF_MAX_ITEMS];
  size_t size;
} EDF;

size_t parent(size_t idx) { return (idx - 1) / 2; }
size_t left_child(size_t idx) { return ((2 * idx + 1)); }
size_t right_child(size_t idx) { return ((2 * idx + 2)); }

void shift_up(EDF *edf, size_t idx, const long patience_usec) {
  while (idx > 0 && priority_rank(edf->items[parent(idx)], patience_usec) >
                        priority_rank(edf->items[idx], patience_usec)) {
    // swap child and parent
    ClientProcess *tmp = edf->items[idx];
    edf->items[idx] = edf->items[parent(idx)];
    edf->items[parent(idx)] = tmp;

    idx = parent(idx);
  }
}
void shift_down(EDF *edf, size_t idx, const long patience_usec) {
  size_t min_idx = idx;

  size_t left = left_child(idx);
  if (left < edf->size &&
      priority_rank(edf->items[left], patience_usec) <
          priority_rank(edf->items[min_idx], patience_usec)) {
    min_idx = left;
  }

  size_t right = left_child(idx);
  if (right < edf->size &&
      priority_rank(edf->items[right], patience_usec) <
          priority_rank(edf->items[min_idx], patience_usec)) {
    min_idx = right;
  }

  if (idx != min_idx) {
    ClientProcess *tmp = edf->items[idx];
    edf->items[idx] = edf->items[min_idx];
    edf->items[min_idx] = tmp;

    shift_down(edf, min_idx, patience_usec);
  }
}

void insert(EDF *edf, ClientProcess *client, const long patience_usec) {
  if (edf->size >= EDF_MAX_ITEMS) {
    printf("EDF Scheduler overflow.\n");
    exit(1);
  }

  edf->items[edf->size] = client;
  shift_up(edf, edf->size, patience_usec);

  ++edf->size;
}

ClientProcess *peek(EDF *edf) { return edf->items[0]; }
ClientProcess *dequeue(EDF *edf, const long patience_usec) {
  ClientProcess *next = edf->items[0];

  edf->items[0] = edf->items[edf->size - 1];
  --edf->size;
  shift_down(edf, 0, patience_usec);

  return next;
}
