#ifndef serverinterh
#define serverinterh

#include "server/comm.h"

bool initInet();
ReadResp *getPlay(const char*);
char sendPlay(WriteReq*);

#endif