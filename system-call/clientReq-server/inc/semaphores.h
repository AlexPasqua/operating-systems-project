#ifndef SEMAPHORES_H
#define SEMAPHORES_H

#define SRVSEM 0
#define CLIMUTEX 1

// definizione della union semun
union semun {
  int val;
  struct semid_ds *buf;
  unsigned short *array;
};

void semOp (int semid, unsigned short sem_num, short sem_op);

#endif
