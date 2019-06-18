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

  int fd = open(argv[1],  O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
  if (fd == -1)
    errExit("salva: failed to open the file");

  char result[200];
  sprintf(result, "%s", "");
  for (int i = 2; i < argc; i++){
    strcat(result, argv[i]);
    strcat(result, " ");
  }

  unsigned int size = strlen(result);
  if (write(fd, result, size) != size)
        errExit("salva: write on file failed");

  return 0;
}
