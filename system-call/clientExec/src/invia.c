#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <string.h>

#include "../../clientReq-server/inc/errExit.h"

struct mymsg {
  long mtype;
  char mtext[100];
};

int main (int argc, char *argv[]) {
  if (argc < 2)
    errExit("Usage: ./invia <message queue key> <message>");

  key_t key = (key_t)atoi(argv[1]);
  if (key < 0)
    errExit("La chiave dev'essere positiva");

  int msgid = msgget(key, S_IRUSR | S_IWUSR);
  if (msgid == -1)
    errExit("invia: msgget failed");

  struct mymsg message;
  message.mtype = 1;
  sprintf(message.mtext, "%s", "");
  for (int i = 2; i < argc; i++){
    strcat(message.mtext, argv[i]);
    strcat(message.mtext, " ");
  }

  if (msgsnd(msgid, &message, sizeof(message.mtext), 0) == -1)
    errExit("invia: msgsnd failed");

  printf("Messaggio: %s\n", message.mtext);

  /*if (msgctl(msgid, IPC_RMID, NULL) == -1)
    errExit("invia: failed to remove the message queue");*/

  return 0;
}
