#ifndef SHAMEM_H
#define SHAMEM_H

#define SHM_DIM 20

#include "myfifo.h"

// struct che descrive un'entry del segmento di shared memory
struct Entry {
  char user[USR_STRDIM];
  server_k key;
  unsigned long timestamp;
};

#endif
