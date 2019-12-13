#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>

#include "comm.h"

int loop = 1;
char workDir[256];

void cleanEnd(int);
void *handleReq(void*);
void handleRead(int);
char getData(ReadReq, ReadResp*);
void handleWrite(int);
char timeOk(WriteReq);

void cleanEnd(int sig) {
  printf("\nwaiting for threads to exit\n");
  loop = 0;
}

void *handleReq(void *data) {
  int cs = *(int*)data;
  
  char req;
  recv(cs, &req, sizeof(req), 0);
  printf("received command '%c'\n", req);
  if (req == CREAD || req == CWRITE || req == CPING) {
    printf("sending ok\n");
    char resp = COK;
    send(cs, &resp, sizeof(resp), 0);
  } else {
    char resp = CERROR;
    send(cs, &resp, sizeof(resp), 0);
  }
  
  if (req == CREAD) handleRead(cs);
  else if (req == CWRITE) handleWrite(cs);
  
  printf("finished\n");
  shutdown(cs, SHUT_RDWR);
  close(cs);
  return nullptr;
}

void handleRead(int cs) {
  ReadReq rreq;
  for (int i = 0; i < 64; i++) recv(cs, rreq.lvlName + i, sizeof(rreq.lvlName[i]), 0);
  printf("read request for level \"%s\"\n", rreq.lvlName);
  
  ReadResp rresp;
  char resp = getData(rreq, &rresp);
  send(cs, &resp, sizeof(resp), 0);
  if (resp == ROK) {
    send(cs, &rresp.time, sizeof(rresp.time), 0);
    for (int i = 0; i < rresp.time; i++) {
      send(cs, &rresp.data[i].aX, sizeof(rresp.data[i].aX), 0);
      send(cs, &rresp.data[i].aY, sizeof(rresp.data[i].aY), 0);
      send(cs, &rresp.data[i].bJ, sizeof(rresp.data[i].bJ), 0);
      send(cs, &rresp.data[i].bR, sizeof(rresp.data[i].bR), 0);
    }
    delete [] rresp.data;
  } else {
    printf("read error '%c'\n", resp);
    return;
  }
}

char getData(ReadReq rreq, ReadResp* rresp) {
  char path[256], filePath[256];
  sprintf(path, "%s/%s", workDir, rreq.lvlName);
  struct stat info;
  if (stat(path, &info) != 0 || !(info.st_mode & S_IFDIR)) {
    printf("no score available\n");
    return RNOHIGH;
  }
  sprintf(filePath, "%s/time", path);
  FILE *f = fopen(filePath, "rb");
  if (f == nullptr) {
    printf("\"%s\" does not exist\n", filePath);
    return RERROR;
  }
  fread(&rresp->time, sizeof(rresp->time), 1, f);
  fclose(f);
  sprintf(filePath, "%s/play", path);
  f = fopen(filePath, "rb");
  if (f == nullptr) {
    printf("\"%s\" does not exist\n", filePath);
    return RERROR;
  }
  rresp->data = new Inputs[ntohl(rresp->time)];
  for (int i = 0; i < ntohl(rresp->time); i++) {
    fread(&rresp->data[i].aX, sizeof(rresp->data[i].aX), 1, f);
    fread(&rresp->data[i].aY, sizeof(rresp->data[i].aY), 1, f);
    fread(&rresp->data[i].bJ, sizeof(rresp->data[i].bJ), 1, f);
    fread(&rresp->data[i].bR, sizeof(rresp->data[i].bR), 1, f);
  }
  fclose(f);
  return ROK;
}

void handleWrite(int cs) {
  WriteReq wreq;
  for (int i = 0; i < 64; i++) recv(cs, wreq.lvlName + i, sizeof(wreq.lvlName[i]), 0);
  recv(cs, &wreq.time, sizeof(wreq.time), 0);
  printf("write request for level \"%s\"\n", wreq.lvlName);
  char resp = timeOk(wreq);
  send(cs, &resp, sizeof(&resp), 0);
  if (resp == WOK) {
    char path[256], filePath[256];
    sprintf(path, "%s/%s", workDir, wreq.lvlName);
    struct stat info;
    if (stat(path, &info) != 0 || !(info.st_mode & S_IFDIR)) mkdir(path, 0700);
    sprintf(filePath, "%s/time", path);
    FILE *f = fopen(filePath, "wb");
    if (f == nullptr) {
      printf("error opening file \"%s\"\n", filePath);
      return;
    }
    fwrite(&wreq.time, sizeof(wreq.time), 1, f);
    fclose(f);
    sprintf(filePath, "%s/play", path);
    f = fopen(filePath, "wb");
    if (f == nullptr) {
      printf("error opening file \"%s\"\n", filePath);
      return;
    }
    wreq.data = new Inputs[ntohl(wreq.time)];
    for (int i = 0; i < ntohl(wreq.time); i++) {
      recv(cs, &wreq.data[i].aX, sizeof(wreq.data[i].aX), 0);
      recv(cs, &wreq.data[i].aY, sizeof(wreq.data[i].aY), 0);
      recv(cs, &wreq.data[i].bJ, sizeof(wreq.data[i].bJ), 0);
      recv(cs, &wreq.data[i].bR, sizeof(wreq.data[i].bR), 0);
      fwrite(&wreq.data[i].aX, sizeof(wreq.data[i].aX), 1, f);
      fwrite(&wreq.data[i].aY, sizeof(wreq.data[i].aY), 1, f);
      fwrite(&wreq.data[i].bJ, sizeof(wreq.data[i].bJ), 1, f);
      fwrite(&wreq.data[i].bR, sizeof(wreq.data[i].bR), 1, f);
    }
    fclose(f);
    delete [] wreq.data;
  } else {
    printf("write error '%c'\n", resp);
    return;
  }
}

char timeOk(WriteReq wreq) {
  char path[256], filePath[256];
  sprintf(path, "%s/%s", workDir, wreq.lvlName);
  struct stat info;
  if (stat(path, &info) != 0 || !(info.st_mode & S_IFDIR)) return WOK;
  sprintf(filePath, "%s/time", path);
  FILE *f = fopen(filePath, "rb");
  if (f == nullptr) return WOK;
  int curTime;
  fread(&curTime, sizeof(curTime), 1, f);
  fclose(f);
  return ntohl(wreq.time) < ntohl(curTime) ? WOK : WNOHIGH;
}

int main(int argc, char **argv) {
  sprintf(workDir, "%s/robotserverdata", getenv("HOME"));
  struct stat info;
  if (stat(workDir, &info) != 0 || !(info.st_mode & S_IFDIR)) mkdir(workDir, 0700);
  
  signal(SIGINT, cleanEnd);
  int s = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in serverAddr, clientAddr;
  unsigned short port = 22050;
  if (s < 0) {
    printf("error creating socket (%i)\n", errno);
    return -1;
  }
  
  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  serverAddr.sin_port = htons(port);
  if (bind(s, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
    printf("error binding socket (%i)\n", errno);
    return -1;
  }
  
  listen(s, 1);
  int cs, rcs;
  socklen_t cLen = sizeof(clientAddr);
  fcntl(s, F_SETFL, O_NONBLOCK);
  while (loop) {
    cs = accept(s, (sockaddr*)&clientAddr, &cLen);
    if (errno == EAGAIN) {
      errno = 0;
      usleep(1000);
      continue;
    }
    if (accept < 0) {
      printf("error accepting request (%i)\n", errno);
      continue;
    }
    pthread_t tId;
    rcs = cs;
    pthread_create(&tId, nullptr, handleReq, (void*)&rcs);
  }
  pthread_exit(nullptr);
  shutdown(s, SHUT_RDWR);
  close(s);
  return 0;
}