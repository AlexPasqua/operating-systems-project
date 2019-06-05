#ifndef MYFIFO_H
#define MYFIFO_H

#define USR_STRDIM 50 //dim della stringa per user_id
#define SRV_STRDIM 50 //dim per la stringa service

typedef unsigned long server_k;

// struct per la comunicazione tramite FIFOCLIENT e FIFOSERVER
struct Request { char user[USR_STRDIM], service[SRV_STRDIM]; };
struct Response { server_k key; };

#endif
