#ifndef ctrlmoduleh
#define ctrlmoduleh

#include <TEXEL/texel.h>

class CtrlModule {
  public:
    virtual float jX() = 0;
    virtual float jY() = 0;
    virtual bool jump() = 0;
    virtual bool lJump() = 0;
    virtual bool run() = 0;
    virtual bool lRun() = 0;
    virtual void rumble(float, int) = 0;
    virtual void update(void*) = 0;
    virtual bool isReplay() = 0;
};

class PlayerCtrlModule : public CtrlModule {
  protected:
    float joyX, joyY;
    bool bJ, lBJ, bR, lBR;
    float rPow;
    int rTime;
  public:
    PlayerCtrlModule();
    void write(TXL_File&);
    
    virtual float jX();
    virtual float jY();
    virtual bool jump();
    virtual bool lJump();
    virtual bool run();
    virtual bool lRun();
    virtual void rumble(float, int);
    virtual void update(void*);
    virtual bool isReplay() {return 0;}
};

class RecordedCtrlModule : public CtrlModule {
  protected:
    struct Input {
      float jX, jY;
      bool bJ, bR;
    };
    Input *inputs;
    int playLen, timer;
    float joyX, joyY;
    bool bJ, lBJ, bR, lBR;
  public:
    bool init(char*);
    ~RecordedCtrlModule();
    int timeLeft() {return playLen - timer;}
    
    virtual float jX();
    virtual float jY();
    virtual bool jump();
    virtual bool lJump();
    virtual bool run();
    virtual bool lRun();
    virtual void rumble(float, int);
    virtual void update(void*);
    virtual bool isReplay() {return 1;}
};

#endif