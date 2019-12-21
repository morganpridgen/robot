#ifndef stateh
#define stateh
#include <TEXEL/texel.h>
#include "robot.h"
#include "level.h"
#include "ctrlmodule.h"

extern bool activeServer;

class BaseState {
  public:
    virtual bool init() = 0;
    virtual BaseState *update(TXL_Controller*[4]) = 0;
    virtual void render() = 0;
    virtual void end() = 0;
};

class GameState : public BaseState {
  protected:
    Robot robot;
    CtrlModule *ctrlModule;
    Level lvl;
    float cX, cY;
    int respawnTimer, gameTimer;
    bool timerStart;
    virtual bool engine();
    virtual void renderHud();
  public:
    virtual bool init();
    virtual BaseState *update(TXL_Controller*[4]) = 0;
    virtual void render();
    virtual void end();
};

class PlayState : public GameState {
  protected:
    TXL_File recording;
    bool lastFinish, highScore, firstLoop;
    Robot enemyRobot;
    CtrlModule *enemyModule;
    virtual void renderHud();
  public:
    virtual bool init();
    virtual BaseState *update(TXL_Controller*[4]);
    virtual void render();
    virtual void end();
};

class ReplayState : public GameState {
  protected:
    int skipLoop;
    virtual void renderHud();
  public:
    virtual bool init();
    virtual BaseState *update(TXL_Controller*[4]);
    virtual void render();
    virtual void end();
};

class InternetReplayState : public ReplayState {
  public:
    virtual bool init();
    virtual BaseState *update(TXL_Controller*[4]);
};

class LevelSelectState : public BaseState {
  private:
    char **levelList;
    int selectedLevel, lSL, lvlCount;
    float lJX;
    int stageTime;
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