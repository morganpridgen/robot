#ifndef levelh
#define levelh

class Level;

#include "robot.h"

class Level {
  private:
    int *terrain;
    int length, depth;
  public:
    bool init(const char*, Robot&);
    void update();
    void render(float, float);
    void end();
    
    void modCam(float&, float&, float, float);
    bool inFloor(float, float);
};

#endif