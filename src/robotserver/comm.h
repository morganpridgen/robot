#ifndef commh
#define commh

/*
  // > to server
  // < from server
  
  > (char)req
  < (char)resp
  
  if (resp == COK) {
    if (req == CREAD) {
      for (i = 0; i < 64; i++) > (ReadReq)rreq.lvlName[i]
      < (char)ok
      if (ok == ROK) {
        < (ReadResp)rresp.time
        for (i = 0; i < rresp.time; i++) < (ReadResp)rresp.data[i]
      }
    } else if (req == CWRITE) {
      for (i = 0; i < 64; i++) > (WriteReq)wreq.lvlName[i]
      > (WriteReq)wreq.time
      < (char)ok
      if (ok == WOK) {
        for (i = 0; i < wreq.time; i++) > (WriteReq)wreq.data[i]
      }
    }
  }
*/

#define CREAD 'R'
#define CWRITE 'W'
#define CPING 'Q'
#define COK 'K'
#define CERROR 'X'

#define ROK 'K'
#define RNOHIGH 'B'
#define RERROR 'X'

#define WOK 'K'
#define WNOHIGH 'B'
#define WERROR 'X'

const char sig[] = "RBTSVR00";

struct Inputs {
  float aX, aY;
  bool bJ, bR;
};



struct ReadReq {
  char lvlName[64];
};

struct ReadResp {
  int time;
  Inputs *data;
};



struct WriteReq {
  char lvlName[64];
  int time;
  Inputs *data;
};

#endif