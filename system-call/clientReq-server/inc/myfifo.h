#ifndef MYFIFO_H
#define MYFIFO_H

#define USRID_STRDIM 50 //dim della stringa per user_id
#define SRV_STRDIM 50 //dim per la stringa service
#define K_STRDIM 10 //dim della stringa di key

// struct per la comunicazione tramite FIFOCLIENT e FIFOSERVER
struct Request { char user_id[USRID_STRDIM], service[SRV_STRDIM]; };
struct Response { char key[K_STRDIM]; };

#endif
