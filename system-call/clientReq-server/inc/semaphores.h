#ifndef SEMAPHORES_H
#define SEMAPHORES_H

// definition of the union semun
union semun {
  int val;
  struct semid_ds *buf;
  unsigned short *array;
};

void semOp (int semid, unsigned short sem_num, short sem_op);

#endif
