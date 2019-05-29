#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main (int argc, char *argv[]) {
  /* Il testo dice che bisogna stampare TUTTI gli argomenti da riga di comando,
  quindi stampo anche il nome */

  char result[200] = "";

  for (int i = 1; i < argc; i++){
    strcat(result, argv[i]);
    strcat(result, " ");
  }

  printf("%s", result);

  return 0;
}
