#include <stdbool.h>
#include <time.h>

#include "shmem.h"
#include "myfifo.h"

// implementazione funzioni ----------------------------------------------------
void del_old_entries(Entry *shmptr, cus_t time_limit, cus_t entry_limit){
  for (int i = 0; i <= entry_limit; i++){
    if ((shmptr + i)->key != 0 && ts_too_old((shmptr + i)->timestamp, time_limit))
      (shmptr + i)->key = 0;
  }
}

//------------------------------------------------------------------------------
bool ts_too_old(const long unsigned ts, cus_t time_limit){
  if ((time(NULL) - time_limit) >= ts)
    return true;

  return false;
}
