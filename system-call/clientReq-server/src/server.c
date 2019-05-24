#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include "errExit.h"

int main (int argc, char *argv[]) {
  // blocco tutti i signal tranne SIGTERM
  sigset_t signal_set;
  if (sigfillset(&signal_set) == -1)
    errExit("sigfillset failed");

  if (sigdelset(&signal_set, SIGTERM) == -1)
    errExit("sigdelset failed");

  if (sigprocmask(SIG_SETMASK, &signal_set, NULL) == -1)
    errExit("sigprocmask failed");

  // --> crea FIFOSERVER





  // --> elimina FIFOSERVER
  return 0;
}
