#include "robot.h"
#include <cmath>

#define colXOff (6.0f)
#define limbXOff (3.0f)
#define limbYOff (3.0f)

bool Robot::init() {
  if (!robotTex.load(TXL_DataPath("robot.png"), 24, 16)) return 0;
  info.x = 320.0f, info.y = 180.0f, info.xV = 0.0f, info.yV = 0.0f, info.gY = 0.0f;
  info.grounded = 0;
  info.djCharge = 0;
  dead = 0, dir = 0;
  return 1;
}

void Robot::update(TXL_Controller *ctrl, Level &lvl) {
  if (!dead) {
    info.xV += ((ctrl->leftJoyX() * 4.0f * float(1 + ctrl->buttonPress(CtrlW))) - info.xV) / 8.0f;
    if (ctrl->buttonClick(CtrlS)) {
      if (info.grounded) {
        info.yV = -16.0f;
        info.grounded = 0;
        ctrl->rumble(0.75f, 125);
      } else if (info.djCharge) {
        info.yV = -12.0f;
        info.djCharge = 0;
        ctrl->rumble(0.625f, 125);
      }
    }
  } else info.xV += info.xV / -8.0f;
  if (info.xV > 0) dir = 0;
  if (info.xV < 0) dir = 1;
  
  motionCalc(ctrl);
  for (int i = 0; i < 4; i++) {
    info.x += info.xV / 4.0f;
    info.y += info.yV / 4.0f;
    colCalc(ctrl, lvl);
  }
}

void Robot::motionCalc(TXL_Controller *ctrl) {
  if (!info.grounded) {
    if (info.yV < 16.0f) info.yV += 1.0f + float(1 - (ctrl->buttonPress(CtrlS) || !info.djCharge));
  }
  if (fabs(info.xV) < 0.25f) info.xV = 0.0f;
}

void Robot::colCalc(TXL_Controller *ctrl, Level &lvl) {
  if (isInFloor(0.0f, 0.0f, lvl)) {
    bool floor = 0;
    for (int i = 0; i < 128; i++) {
      if (!isInFloor(0.0f, 0.0f - i, lvl)) { // floor check
        ctrl->rumble(fmin(fabs(info.yV) / 24.0f, 1.0f), 125);
        info.y -= i + 1;
        info.gY = info.y;
        info.yV = 0.0f;
        info.grounded = 1;
        info.djCharge = 1;
        floor = 1;
        break;
      }
      if (!isInFloor(0.0f - i, 0.0f, lvl)) { // right wall check
        ctrl->rumble(fmin(fabs(info.xV) / 6.0f, 1.0f), 125);
        info.x -= i + 1;
        info.xV = 0.0f;
        break;
      }
      if (!isInFloor(0.0f + i, 0.0f, lvl)) { // left wall check
        ctrl->rumble(fmin(fabs(info.xV) / 6.0f, 1.0f), 125);
        info.x += i + 1;
        info.xV = 0.0f;
        break;
      }
      if (!isInFloor(0.0f, 0.0f + i, lvl)) { // ceiling check
        ctrl->rumble(fmin(fabs(info.yV) / 24.0f, 1.0f), 125);
        info.y += i + 1;
        info.yV = round(info.yV * -0.5f);
        break;
      }
    }
  }
  if (!isInFloor(0.0f, 2.0f, lvl)) info.grounded = 0;
  if (lvl.inLethal(info.x, info.y)) dead = 1;
  if (dir) {
    rLOff = (!lvl.inFloor(info.x + limbXOff - 1, info.y + 2.0f)) && !lLOff;
    lLOff = (!lvl.inFloor(info.x - limbXOff + 1, info.y + 2.0f)) && !rLOff;
  } else {
    lLOff = (!lvl.inFloor(info.x - limbXOff + 1, info.y + 2.0f)) && !rLOff;
    rLOff = (!lvl.inFloor(info.x + limbXOff - 1, info.y + 2.0f)) && !lLOff;
  }
}

bool Robot::isInFloor(float xOff, float yOff, Level &lvl) {
  return lvl.inFloor(info.x - colXOff + xOff, info.y + yOff) || lvl.inFloor(info.x + colXOff + xOff, info.y + yOff);
}

void Robot::render(float cX, float cY) {
  if (!info.grounded) {
    lAnim = 0;
    tLLR = 30.0f, tRLR = -30.0f;
  } else {
    if (info.xV > 0.0f) {
      lAnim += info.xV / 8.0f;
      tLLR = 30.0f * sin(lAnim * 1.57f / 4.0f);
      tRLR = 30.0f * cos(lAnim * 1.57f / 4.0f);
    } else if (info.xV < 0.0f) {
      lAnim -= info.xV / 8.0f;
      tLLR = -30.0f * cos(lAnim * 1.57f / 4.0f);
      tRLR = -30.0f * sin(lAnim * 1.57f / 4.0f);
    } else {
      lAnim = 0;
      tLLR = -30.0f * lLOff;
      tRLR = 30.0f * rLOff;
    }
  }
  lLR += (tLLR - lLR) / 4.0f;
  rLR += (tRLR - rLR) / 4.0f;
  
  //TXL_RenderQuad(info.x - cX, info.y - cY - colXOff, 16, 16, {0.75f, 0.75f, 0.75f, 1.0f});
  robotTex.setClip(16, 24, 0, 16);
  robotTex.render(info.x - cX - limbXOff, info.y - cY - 8 + limbYOff, lLR);
  robotTex.setClip(24, 16, 0, 16);
  robotTex.render(info.x - cX + limbXOff, info.y - cY - 8 + limbYOff, rLR);
  robotTex.setClip(16 * dir, 16 * !dir, 0, 16);
  robotTex.render(info.x - cX, info.y - cY - 8.0f + (sin(lAnim * 1.57f / 8.0f) + cos(lAnim * 1.57f / 8.0f)));
}

void Robot::end() {
  robotTex.free();
}

void Robot::modCam(float &cX, float &cY, Level &lvl) {
  cX += (info.x - cX - float(320 + 80 * (info.xV / -8))) / 8.0f;
  cY += (info.gY - cY - 225.0f) / 8.0f;
  lvl.modCam(cX, cY, info.x, info.y);
}