#ifndef roboth
#define roboth

#include <TEXEL/texel.h>

class Robot;

#include "level.h"

struct RobotInfo {
  float x, y, xV, yV, gY;
  bool grounded, djCharge;
};

class Robot {
  private:
    TXL_Texture robotTex;
    RobotInfo info;
    bool dead;
    void motionCalc(TXL_Controller*);
    void colCalc(TXL_Controller*, Level&);
    bool isInFloor(float, float, Level&);
  public:
    bool init();
    void update(TXL_Controller*, Level&);
    void render(float, float);
    void end();
    
    void getPos(float &x, float &y) {x = info.x, y = info.y;}
    void setPos(float x, float y) {info.x = x, info.y = y;}
    void modCam(float&, float&, Level&);
    bool getDead() {return dead;}
};

#endif