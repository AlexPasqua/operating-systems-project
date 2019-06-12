#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <errno.h>

#define FTOK_PATH "/dev"
#define FTOK_PROJ 99

// message structure definition 
struct mymsg { 
	long mtype; 
	char mtext[100]; 
};

int main (int argc, char *argv[]) {
    printf("Hi, I'm Invia program!\n");
    int i;
    //get the msg queue ID
    //it 's the first argument
    int msqid = msgget((key_t)atoi(argv[3]), 0);
    if (msqid == -1)
    	printf("An error occured opening msqid[%i]\n", errno);
    
    struct mymsg m;

    //message type
    m.mtype = 1;
    //init msg body
    sprintf(m.mtext, "%s", "");

    //load msg to msg queue
    for (i = 4; i < argc; i++){
    	strcat(m.mtext, argv[i]);
    	strcat(m.mtext, " ");
    }
    printf("MSG:\nBody:%s\n", m.mtext);

    if (msgsnd(msqid, &m, sizeof(m.mtext), 0) == -1)
    	printf("An error occured sending the message\n");

    return 0;
}
