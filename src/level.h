#ifndef levelh
#define levelh

class Level;

#include "robot.h"

struct TileSpan {
  char type;
  int len;
};

class Level {
  private:
    TileSpan *terrain;
    int length, depth;
    bool solidTop;
    bool inTile(float, float, char);
    char typeAt(int, int);
    int animTimer;
  public:
    bool init(const char*, Robot&);
    void update();
    void render(float, float);
    void end();
    
    void modCam(float&, float&, float, float);
    bool inFloor(float, float);
    bool inLethal(float, float);
};

#endif