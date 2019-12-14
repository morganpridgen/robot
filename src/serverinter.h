#ifndef serverinterh
#define serverinterh

#include "robotserver/comm.h"

bool initInet();
ReadResp *getPlay(const char*);
char sendPlay(WriteReq*);

#endif