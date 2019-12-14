#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>

#include "comm.h"

int loop = 1;
char workDir[256];

struct ClientInfo {
  int cs, id;
};
void cleanEnd(int);
void *handleReq(void*);
void handleRead(ClientInfo);
char getData(ClientInfo, ReadReq, ReadResp*);
void handleWrite(ClientInfo);
char timeOk(ClientInfo, WriteReq);

void cleanEnd(int sig) {
  printf("\nwaiting for threads to exit\n");
  loop = 0;
}

void *handleReq(void *data) {
  ClientInfo client = *(ClientInfo*)data;
  
  for (int i = 0; i < 8; i++) send(client.cs, sig + i, sizeof(sig[i]), 0);
  char req;
  if (recv(client.cs, &req, sizeof(req), 0) > 0) {
    printf("(%010i) received command '%c'\n", client.id, req);
    if (req == CREAD || req == CWRITE || req == CPING) {
      printf("(%010i) sending ok\n", client.id);
      char resp = COK;
      send(client.cs, &resp, sizeof(resp), 0);
    } else {
      char resp = CERROR;
      send(client.cs, &resp, sizeof(resp), 0);
    }
    
    if (req == CREAD) handleRead(client);
    else if (req == CWRITE) handleWrite(client);
  }
  
  printf("(%010i) finished\n", client.id);
  shutdown(client.cs, SHUT_RDWR);
  close(client.cs);
  return nullptr;
}

void handleRead(ClientInfo client) {
  ReadReq rreq;
  for (int i = 0; i < 64; i++) recv(client.cs, rreq.lvlName + i, sizeof(rreq.lvlName[i]), 0);
  printf("(%010i)read request for level \"%s\"\n", client.id, rreq.lvlName);
  
  ReadResp rresp;
  char resp = getData(client, rreq, &rresp);
  send(client.cs, &resp, sizeof(resp), 0);
  if (resp == ROK) {
    send(client.cs, &rresp.time, sizeof(rresp.time), 0);
    for (int i = 0; i < rresp.time; i++) {
      send(client.cs, &rresp.data[i].aX, sizeof(rresp.data[i].aX), 0);
      send(client.cs, &rresp.data[i].aY, sizeof(rresp.data[i].aY), 0);
      send(client.cs, &rresp.data[i].bJ, sizeof(rresp.data[i].bJ), 0);
      send(client.cs, &rresp.data[i].bR, sizeof(rresp.data[i].bR), 0);
    }
    delete [] rresp.data;
  } else {
    printf("(%010i) read error '%c'\n", client.id, resp);
    return;
  }
}

char getData(ClientInfo client, ReadReq rreq, ReadResp* rresp) {
  char path[256], filePath[256];
  sprintf(path, "%s/%s", workDir, rreq.lvlName);
  struct stat info;
  if (stat(path, &info) != 0 || !(info.st_mode & S_IFDIR)) {
    printf("(%010i) no score available\n", client.id);
    return RNOHIGH;
  }
  sprintf(filePath, "%s/time", path);
  FILE *f = fopen(filePath, "rb");
  if (f == nullptr) {
    printf("(%010i) \"%s\" does not exist\n", client.id, filePath);
    return RERROR;
  }
  fread(&rresp->time, sizeof(rresp->time), 1, f);
  fclose(f);
  sprintf(filePath, "%s/play", path);
  f = fopen(filePath, "rb");
  if (f == nullptr) {
    printf("(%010i) \"%s\" does not exist\n", client.id, filePath);
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

void handleWrite(ClientInfo client) {
  WriteReq wreq;
  for (int i = 0; i < 64; i++) recv(client.cs, wreq.lvlName + i, sizeof(wreq.lvlName[i]), 0);
  recv(client.cs, &wreq.time, sizeof(wreq.time), 0);
  printf("(%010i) write request for level \"%s\"\n", client.id, wreq.lvlName);
  char resp = timeOk(client, wreq);
  send(client.cs, &resp, sizeof(&resp), 0);
  if (resp == WOK) {
    char path[256], filePath[256];
    sprintf(path, "%s/%s", workDir, wreq.lvlName);
    struct stat info;
    if (stat(path, &info) != 0 || !(info.st_mode & S_IFDIR)) mkdir(path, 0700);
    sprintf(filePath, "%s/time", path);
    FILE *f = fopen(filePath, "wb");
    if (f == nullptr) {
      printf("(%010i) error opening file \"%s\"\n", client.id, filePath);
      return;
    }
    fwrite(&wreq.time, sizeof(wreq.time), 1, f);
    fclose(f);
    sprintf(filePath, "%s/play", path);
    f = fopen(filePath, "wb");
    if (f == nullptr) {
      printf("(%010i) error opening file \"%s\"\n", client.id, filePath);
      return;
    }
    wreq.data = new Inputs[ntohl(wreq.time)];
    for (int i = 0; i < ntohl(wreq.time); i++) {
      recv(client.cs, &wreq.data[i].aX, sizeof(wreq.data[i].aX), 0);
      recv(client.cs, &wreq.data[i].aY, sizeof(wreq.data[i].aY), 0);
      recv(client.cs, &wreq.data[i].bJ, sizeof(wreq.data[i].bJ), 0);
      recv(client.cs, &wreq.data[i].bR, sizeof(wreq.data[i].bR), 0);
      fwrite(&wreq.data[i].aX, sizeof(wreq.data[i].aX), 1, f);
      fwrite(&wreq.data[i].aY, sizeof(wreq.data[i].aY), 1, f);
      fwrite(&wreq.data[i].bJ, sizeof(wreq.data[i].bJ), 1, f);
      fwrite(&wreq.data[i].bR, sizeof(wreq.data[i].bR), 1, f);
    }
    fclose(f);
    delete [] wreq.data;
  } else {
    printf("(%010i) write error '%c'\n", client.id, resp);
    return;
  }
}

char timeOk(ClientInfo client, WriteReq wreq) {
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
  int cs, cId = 0;
  ClientInfo rcs;
  socklen_t cLen = sizeof(clientAddr);
  fcntl(s, F_SETFL, O_NONBLOCK);
  while (loop) {
    cs = accept(s, (sockaddr*)&clientAddr, &cLen);
    if (errno == EAGAIN) {
      errno = 0;
      usleep(1000);
      continue;
    }
    char *cAddr = inet_ntoa(clientAddr.sin_addr);    
    if (accept < 0) {
      printf("error accepting request from \"%s\" (%i)\n", cAddr, errno);
      continue;
    }
    pthread_t tId;
    rcs.cs = cs, rcs.id = cId;
    printf("(%010i) new connection from \"%s\"\n", rcs.id, cAddr);
    pthread_create(&tId, nullptr, handleReq, (void*)&rcs);
    cId++;
  }
  pthread_exit(nullptr);
  shutdown(s, SHUT_RDWR);
  close(s);
  return 0;
}
