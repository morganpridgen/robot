#ifndef roboth
#define roboth

#include <TEXEL/texel.h>

class Robot;

#include "level.h"
#include "ctrlmodule.h"

struct RobotInfo {
  float x, y, xV, yV, gY;
  bool grounded, djCharge;
};

class Robot {
  private:
    TXL_Texture robotTex;
    RobotInfo info;
    bool dead, finished, dir;
    void motionCalc(CtrlModule*);
    void colCalc(CtrlModule*, Level&);
    bool isInFloor(float, float, Level&);
    float lLR, rLR, tLLR, tRLR, lAnim;
    bool lLOff, rLOff;
    void lUpdate();
    bool isReplay;
    int deathTimer;
  public:
    bool init();
    void update(CtrlModule*, Level&);
    void render(float, float);
    void end();
    
    void getPos(float &x, float &y) {x = info.x, y = info.y;}
    void setPos(float x, float y) {info.x = x, info.y = y;}
    void modCam(float&, float&, Level&);
    bool getDead() {return dead;}
    bool getFinished() {return finished;}
};

#endif