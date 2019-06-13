#ifndef SEMAPHORES_H
#define SEMAPHORES_H

#define SRVSEM 0
#define CLIMUTEX 1
#define SEMTYPE_FIFO 0
#define SEMTYPE_SHM 1

// definizione della union semun
union semun {
  int val;
  struct semid_ds *buf;
  unsigned short *array;
};

// dichiarazione funcioni
int crt_semaphores(char *calling_prog, short type);  //crea un semaphores set per le fifo o shm
void semOp (int semid, unsigned short sem_num, short sem_op);

#endif
