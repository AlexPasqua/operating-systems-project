#ifndef SHAMEM_H
#define SHAMEM_H

#include <stdbool.h>
#include "myfifo.h"

#define SHM_DIM 5

typedef const unsigned short cus_t;

// struct che descrive un'entry del segmento di shared memory
typedef struct Entry {
  char user[USR_STRDIM];
  server_k key;
  unsigned long timestamp;
} Entry;

// dichiarazione funzioni
void del_old_entries(Entry *shmptr, cus_t time_limit);
bool ts_too_old(const long unsigned entry_ts, cus_t time_limit);

#endif
