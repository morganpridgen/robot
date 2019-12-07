#include "robot.h"
#include <cmath>

#define colXOff (6.0f)

bool Robot::init() {
  //if (!robotTex.load(TXL_DataPath("robot.png"), 16, 16)) return 0;
  info.x = 320.0f, info.y = 180.0f, info.xV = 0.0f, info.yV = 0.0f;
  info.grounded = 0;
  return 1;
}

void Robot::update(TXL_Controller *ctrl, Level &lvl) {
  info.xV += ((ctrl->leftJoyX() * 4.0f * float(1 + ctrl->buttonPress(CtrlW))) - info.xV) / 8.0f;
  if (info.grounded && ctrl->buttonClick(CtrlS)) {
    info.yV = -16.0f;
    info.grounded = 0;
  }
  
  motionCalc();
  for (int i = 0; i < 4; i++) {
    info.x += info.xV / 4.0f;
    info.y += info.yV / 4.0f;
    colCalc(lvl);
  }
}

void Robot::motionCalc() {
  if (!info.grounded) {
    info.yV += 1.0f;
    if (info.xV * info.xV + info.yV * info.yV > 576.0f) {
      float r = atan2(info.yV, info.xV);
      info.xV = 24.0f * cos(r);
      info.yV = 24.0f * sin(r);
    }
  }
  if (fabs(info.xV) < 0.5f) info.xV = 0.0f;
}

void Robot::colCalc(Level &lvl) {
  if (isInFloor(0.0f, 0.0f, lvl)) {
    bool floor = 0;
    for (int i = 0; i < 128; i++) {
      if (!isInFloor(0.0f, 0.0f - i, lvl)) { // floor check
        info.y -= i + 1;
        info.yV = 0.0f;
        info.grounded = 1;
        floor = 1;
        break;
      }
      if (!isInFloor(0.0f - i, 0.0f, lvl)) { // left wall check
        info.x -= i + 1;
        info.xV = 0;
        break;
      }
      if (!isInFloor(0.0f + i, 0.0f, lvl)) { // right wall check
        info.x += i + 1;
        info.xV = 0;
        break;
      }
      if (!isInFloor(0.0f, 0.0f + i, lvl)) { // ceiling check
        info.y += i + 1;
        info.yV = round(info.yV * -0.5f);
        info.xV = 0;
        break;
      }
    }
  }
  if (!isInFloor(0.0f, 2.0f, lvl)) info.grounded = 0;
}

bool Robot::isInFloor(float xOff, float yOff, Level &lvl) {
  return lvl.inFloor(info.x - colXOff + xOff, info.y + yOff) || lvl.inFloor(info.x + colXOff + xOff, info.y + yOff);
}

void Robot::render(float cX, float cY) {
  TXL_RenderQuad(info.x - cX, info.y - cY, 16, 16, {0.75f, 0.75f, 0.75f, 1.0f});
}

void Robot::end() {
  robotTex.free();
}

void Robot::modCam(float &cX, float &cY, Level &lvl) {
  cX += (info.x - cX - 320.0f) / 8.0f;
  lvl.modCam(cX, cY, info.x, info.y);
}