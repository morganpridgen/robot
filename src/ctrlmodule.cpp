#include "ctrlmodule.h"
#include <climits>

PlayerCtrlModule::PlayerCtrlModule() {
  joyX = 0.0f, joyY = 0.0f;
  bJ = 0, lBJ = 0, bR = 0, lBR = 0;
  rPow = 0.0f, rTime = 0;
}

void PlayerCtrlModule::write(TXL_File &f) {
  f.write(&joyX, sizeof(joyX));
  f.write(&joyY, sizeof(joyY));
  f.write(&bJ, sizeof(bJ));
  f.write(&bR, sizeof(bR));
}

void PlayerCtrlModule::update(void *data) {
  TXL_Controller *ctrl = (TXL_Controller*)data;
  joyX = ctrl->leftJoyX(), joyY = ctrl->leftJoyY();
  lBJ = bJ;
  bJ = ctrl->buttonPress(CtrlS);
  lBR = bR;
  bR = ctrl->buttonPress(CtrlW);
  if (rTime) {
    ctrl->rumble(rPow, rTime);
    rPow = 0.0f, rTime = 0;
  }
}

float PlayerCtrlModule::jX() {
  return joyX;
}

float PlayerCtrlModule::jY() {
  return joyY;
}

bool PlayerCtrlModule::jump() {
  return bJ;
}

bool PlayerCtrlModule::lJump() {
  return lBJ;
}

bool PlayerCtrlModule::run() {
  return bR;
}

bool PlayerCtrlModule::lRun() {
  return lBR;
}

void PlayerCtrlModule::rumble(float nRPow, int nRTime) {
  rPow = nRPow, rTime = nRTime;
}



bool RecordedCtrlModule::init(char *level) {
  TXL_File f;
  char path[64];
  sprintf(path, "%s-time", level);
  if (!f.init(TXL_SavePath(path), 'r')) return 0;
  f.read(&playLen, sizeof(playLen));
  f.close();
  if (playLen == INT_MAX) return 0;
  inputs = new Input[playLen];
  sprintf(path, "%s-play", level);
  if (!f.init(TXL_SavePath(path), 'r')) return 0;
  for (int i = 0; i < playLen; i++) {
    f.read(&inputs[i].jX, sizeof(inputs[i].jX));
    f.read(&inputs[i].jY, sizeof(inputs[i].jY));
    f.read(&inputs[i].bJ, sizeof(inputs[i].bJ));
    f.read(&inputs[i].bR, sizeof(inputs[i].bR));
  }
  f.close();
  timer = 0;
  
  joyX = 0.0f, joyY = 0.0f;
  bJ = 0, lBJ = 0, bR = 0, lBR = 0;
  return 1;
}

void RecordedCtrlModule::update(void *data) {
  lBJ = bJ, lBR = bR;
  if (timer < playLen) {
    joyX = inputs[timer].jX;
    joyY = inputs[timer].jY;
    bJ = inputs[timer].bJ;
    bR = inputs[timer].bR;
  } else {
    joyX = 0.0f, joyY = 0.0f;
    bJ = 0, bR = 0;
  }
  timer++;
}

float RecordedCtrlModule::jX() {
  return joyX;
}

float RecordedCtrlModule::jY() {
  return joyY;
}

bool RecordedCtrlModule::jump() {
  return bJ;
}

bool RecordedCtrlModule::lJump() {
  return lBJ;
}

bool RecordedCtrlModule::run() {
  return bR;
}

bool RecordedCtrlModule::lRun() {
  return lBR;
}

void RecordedCtrlModule::rumble(float nRPow, int nRTime) {
  
}