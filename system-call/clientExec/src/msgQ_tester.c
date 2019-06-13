#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

#define FTOK_PATH "/tmp/"
#define FTOK_PROJ 99
int main (int argc, char *argv[]){

	int msqid = msgget(ftok(FTOK_PATH, FTOK_PROJ),  IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
	if (msqid == -1)
		printf("An error occured creating the msg queue [%i]\n", errno);

	printf("MSG QUEUE KEY: %i\n", ftok(FTOK_PATH, FTOK_PROJ));

	// message structure definition
	struct mymsg {
		long mtype;
		char mtext[100];
	};


	struct mymsg m;
	size_t mSize =	sizeof(m.mtext);

	if (msgrcv(msqid, &m, mSize, 1, 0) == -1)
		printf("msgrcv failed[%i]\n", errno);

	printf("\n%s\n\n", m.mtext);

	if (msgctl(msqid, IPC_RMID, NULL) == -1)
        printf("msg queue deleting failed...\n");

	return 0;
}
