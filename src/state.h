#ifndef stateh
#define stateh
#include <TEXEL/texel.h>
#include "robot.h"
#include "level.h"

class BaseState {
  public:
    virtual bool init() = 0;
    virtual BaseState *update(TXL_Controller*[4]) = 0;
    virtual void render() = 0;
    virtual void end() = 0;
};

class PlayState : public BaseState {
  private:
    Robot robot;
    Level lvl;
    float cX, cY;
    int endTimer;
  public:
    virtual bool init();
    virtual BaseState *update(TXL_Controller*[4]);
    virtual void render();
    virtual void end();
};

class LevelSelectState : public BaseState {
  private:
    char **levelList;
    int selectedLevel, lvlCount;
    float lJX;
  public:
    virtual bool init();
    virtual BaseState *update(TXL_Controller*[4]);
    virtual void render();
    virtual void end();
};

class InitState : public BaseState {
  public:
    virtual bool init() {return 1;}
    virtual BaseState *update(TXL_Controller *ctrls[4]) {return new LevelSelectState;}
    virtual void render() {}
    virtual void end() {}
};

#endif