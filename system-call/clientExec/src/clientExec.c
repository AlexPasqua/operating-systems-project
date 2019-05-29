#include <stdlib.h>
#include <stdio.h>

#include "errExit.h"

int main (int argc, char *argv[]) {
  // in questo caso è possibile che il servizio non abbia argomenti -> non farà nulla
  if (argc < 3)
    errExit("Usage: ./clientExec <user_id> <server_key> <args>");

  return 0;
}
