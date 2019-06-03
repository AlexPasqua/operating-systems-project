#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "../../clientReq-server/inc/errExit.h"

int main (int argc, char *argv[]) {
  if (argc < 2)
    errExit("Usage: ./salva <file name> <args>");

  int fd = open(argv[1],  O_CREAT | O_TRUNC | O_RDWR);
  if (fd == -1)
    errExit("salva: failed to open the file");

  for (int i = 2; i < argc; i++){
    int size = 1;
    for ( ; argv[i][size - 1] != '\0'; size++); //conto la lunghezza della stringa
    if (write(fd, argv[i], size) != size)
      errExit("salva: write on file failed");
  }

  return 0;
}
