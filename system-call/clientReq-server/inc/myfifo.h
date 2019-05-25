#ifndef MYFIFO_H
#define MYFIFO_H

#define USR_STRDIM 50 //dim della stringa per user_id
#define SRV_STRDIM 50 //dim per la stringa service
#define K_STRDIM 10 //dim della stringa di key

// struct per la comunicazione tramite FIFOCLIENT e FIFOSERVER
struct Request { char user[USR_STRDIM], service[SRV_STRDIM]; };
struct Response { char key[K_STRDIM]; };

char *fifo_chread(int fifoFD, char *buf, unsigned int size);

#endif
