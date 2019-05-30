#include "../inc/errExit.h" /* metto il percorso altrimenti ho difficoltà
a compilarlo in clientExec */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

void errExit(const char *msg) {
    perror(msg);
    exit(1);
}
