#include "serverinter.h"
#include <cstdio>
#include <cstring>
#include <TEXEL/texel.h>

char address[22];

bool initInet() {
  if (!TXL_IsFile(TXL_SavePath("serverip"))) return 0;
  TXL_File f;
  if (!f.init(TXL_SavePath("serverip"), 'r')) return 0;
  for (int i = 0; i < 23; i++) {
    if (f.read(address + i, sizeof(address[i])) == 0) {
      address[i] = 0;
      break;
    }
  }
  f.close();
  TXL_Socket s;
  if (!s.init(address)) return 0;
  char req = CPING;
  s.write(&req, sizeof(req));
  char resp;
  s.read(&resp, sizeof(resp));
  s.end();
  return resp == COK;
}

ReadResp *getPlay(const char *lvl) {
  static ReadResp rresp;
  static Inputs *data = nullptr;
  if (data) {
    delete [] data;
    data = nullptr;
  }
  ReadReq rreq;
  TXL_Socket s;
  if (!s.init(address)) return nullptr;
  char req = CREAD;
  s.write(&req, sizeof(req));
  char resp;
  s.read(&resp, sizeof(resp));
  if (resp == COK) {
    strcpy(rreq.lvlName, lvl);
    for (int i = 0; i < 64; i++) s.write(rreq.lvlName + i, sizeof(rreq.lvlName[i]));
    s.read(&resp, sizeof(resp));
    if (resp == ROK) {
      s.read(&rresp.time, sizeof(rresp.time));
      TXL_FlipEndian(&rresp.time, sizeof(rresp.time));
      data = new Inputs[rresp.time];
      rresp.data = data;
      for (int i = 0; i < rresp.time; i++) {
        s.read(&data[i].aX, sizeof(data[i].aX));
        s.read(&data[i].aY, sizeof(data[i].aY));
        s.read(&data[i].bJ, sizeof(data[i].bJ));
        s.read(&data[i].bR, sizeof(data[i].bR));
        TXL_FlipEndian(&data[i].aX, sizeof(data[i].aX));
        TXL_FlipEndian(&data[i].aY, sizeof(data[i].aY));
        TXL_FlipEndian(&data[i].bJ, sizeof(data[i].bJ));
        TXL_FlipEndian(&data[i].bR, sizeof(data[i].bR));
      }
    }
  }
  s.end();
  return &rresp;
}

char sendPlay(WriteReq *wreq) {
  TXL_Socket s;
  if (!s.init(address)) return CERROR;
  char req = CWRITE;
  s.write(&req, sizeof(req));
  char resp;
  s.read(&resp, sizeof(resp));
  if (resp == COK) {
    for (int i = 0; i < 64; i++) s.write(wreq->lvlName + i, sizeof(wreq->lvlName[i]));
    TXL_FlipEndian(&wreq->time, sizeof(wreq->time));
    s.write(&wreq->time, sizeof(wreq->time));
    TXL_FlipEndian(&wreq->time, sizeof(wreq->time));
    s.read(&resp, sizeof(resp));
    if (resp == WOK) {
      for (int i = 0; i < wreq->time; i++) {
        TXL_FlipEndian(&wreq->data[i].aX, sizeof(wreq->data[i].aX));
        TXL_FlipEndian(&wreq->data[i].aY, sizeof(wreq->data[i].aY));
        TXL_FlipEndian(&wreq->data[i].bJ, sizeof(wreq->data[i].bJ));
        TXL_FlipEndian(&wreq->data[i].bR, sizeof(wreq->data[i].bR));
        s.write(&wreq->data[i].aX, sizeof(wreq->data[i].aX));
        s.write(&wreq->data[i].aY, sizeof(wreq->data[i].aY));
        s.write(&wreq->data[i].bJ, sizeof(wreq->data[i].bJ));
        s.write(&wreq->data[i].bR, sizeof(wreq->data[i].bR));
        TXL_FlipEndian(&wreq->data[i].aX, sizeof(wreq->data[i].aX));
        TXL_FlipEndian(&wreq->data[i].aY, sizeof(wreq->data[i].aY));
        TXL_FlipEndian(&wreq->data[i].bJ, sizeof(wreq->data[i].bJ));
        TXL_FlipEndian(&wreq->data[i].bR, sizeof(wreq->data[i].bR));
      }
    }
  }
  s.end();
  return resp;
}