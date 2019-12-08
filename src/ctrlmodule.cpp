#include "ctrlmodule.h"

PlayerCtrlModule::PlayerCtrlModule() {
  joyX = 0.0f, joyY = 0.0f;
  bJ = 0, lBJ = 0, bR = 0, lBR = 0;
  rPow = 0.0f, rTime = 0;
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



bool RecordedCtrlModule::init(char *path) {
  if (!in.init(path, 'r')) return 0;
  joyX = 0.0f, joyY = 0.0f;
  bJ = 0, lBJ = 0, bR = 0, lBR = 0;
  return 1;
}

RecordedCtrlModule::~RecordedCtrlModule() {
  in.close();
}

void RecordedCtrlModule::update(void *data) {
  lBJ = bJ, lBR = bR;
  if (!in.read(&joyX, sizeof(joyX))) joyX = 0.0f;
  if (!in.read(&joyY, sizeof(joyY))) joyY = 0.0f;
  if (!in.read(&bJ, sizeof(bJ))) bJ = 0;
  if (!in.read(&bR, sizeof(bR))) bR= 0;
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