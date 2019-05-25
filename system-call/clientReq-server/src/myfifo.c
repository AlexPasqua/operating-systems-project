#include <unistd.h>

#include "myfifo.h"
#include "errExit.h"

char *fifo_chread(int fifoFD, char *buf, unsigned int size){
  int bR = read(fifoFD, buf, size);
  if (bR == -1)
    errExit("Server failed to perdorm a read from FIFOSERVER");
  else if (bR != USR_STRDIM)
    errExit("Looks like server didn't received a struct Request correctly");
}
