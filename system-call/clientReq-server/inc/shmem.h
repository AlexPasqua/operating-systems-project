#ifndef SHAMEM_H
#define SHAMEM_H

#include "myfifo.h"

#define SHM_DIM 100

// struct che descrive un'entry del segmento di shared memory
struct Entry {
  char user[USR_STRDIM];
  server_k key;
  unsigned long timestamp;
};

#endif