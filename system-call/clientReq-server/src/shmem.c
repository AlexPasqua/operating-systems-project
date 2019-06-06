#include <stdbool.h>
#include <time.h>
#include <stdio.h>

#include "shmem.h"
#include "myfifo.h"

// implementazione funzioni ----------------------------------------------------
void del_old_entries(Entry *shmptr, cus_t time_limit){
  for (int i = 0; i < SHM_DIM; i++){
    if ((shmptr + i)->key != 0 && ts_too_old((shmptr + i)->timestamp, time_limit))
      (shmptr + i)->key = 0;

    else if ((shmptr + i)->key == 0)  //se ne trova una entry libera (probabilmente a causa di clienExec)
      return;
  }
}

//------------------------------------------------------------------------------
bool ts_too_old(const long unsigned entry_ts, cus_t time_limit){
  if ((time(NULL) - time_limit) >= entry_ts)
    return true;

  return false;
}
