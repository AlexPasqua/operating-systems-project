#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "errExit.h"
#include "myfifo.h"


int main (int argc, char *argv[]) {
  // blocco tutti i signal tranne SIGTERM
  sigset_t signal_set;
  if (sigfillset(&signal_set) == -1)
    errExit("sigfillset failed");

  if (sigdelset(&signal_set, SIGTERM) == -1)
    errExit("sigdelset failed");

  if (sigprocmask(SIG_SETMASK, &signal_set, NULL) == -1)
    errExit("sigprocmask failed");


  // creo FIFOSERVER, apro FIFOSERVER
  char *fifoserv_pathname = "/tmp/FIFOSERVER";
  char *fifocli_pathname = "/tmp/FIFOCLIENT";
  if (mkfifo(fifoserv_pathname, S_IRUSR | S_IWUSR) == -1)
    errExit("mkfifo (FIFOSERVER) failed");

  int fifoserver = open(fifoserv_pathname, O_RDONLY);
  if (fifoserver == -1)
    errExit("Server failed to open FIFOSERVER in read-only mode");


  // continua a controllare richieste dei client
  while (1){
    // apro FIFOCLIENT
    int fifoclient = open(fifocli_pathname, O_WRONLY);
    if (fifoclient == -1)
      errExit("Server failed to open FIFOCLIENT in write-only mode");



     // chiudo FIFOCLIENT
     if (close(fifoclient) == -1)
       errExit("Server failed to close FIFOCLIENT");
  }


  // chiudo ed elimino FIFOSERVER
  if (close(fifoserver) == -1)
    errExit("Server failed to close FIFOSERVER");

  if (unlink(fifoserv_pathname) != 0)
    errExit("Server failed to unlink FIFOSERVER");

  return 0;
}
