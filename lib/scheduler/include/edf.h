#ifndef EDF_H

#define EDF_H

#include "client/include/client.h"

#define EDF_MAX_ITEMS 128

typedef struct {
  ClientProcess *items[EDF_MAX_ITEMS];
  size_t size;
} EDF;

void insert(EDF *edf, ClientProcess *client, const long patience_usec);
ClientProcess *peek(EDF *edf);
ClientProcess *dequeue(EDF *edf, const long patience_usec);

long priority_rank(ClientProcess *client, const long patience_usec);

#endif // !EDF_H
