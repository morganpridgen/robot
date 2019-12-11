#include "robot.h"
#include <cmath>
#include "particle.h"

#define colXOff (6.0f)
#define limbXOff (3.0f)
#define limbYOff (3.0f)

TXL_Square jumpSnd = {1760, 1.0f, 8.0f, 2};
TXL_Square djSnd = {3520, 0.5f, 4.0f, 2};
TXL_Noise landSnd = {0.25f, 2.0f, 0, 8, 0};
TXL_Noise bumpSnd = {0.25f, 2.0f, 0, 4, 1};
TXL_Noise explodeSnd = {1.0f, 4.0f, 0, 16, 1};

bool Robot::init() {
  if (!robotTex.load(TXL_DataPath("robot.png"), 24, 16)) return 0;
  info.x = 320.0f, info.y = 180.0f, info.xV = 0.0f, info.yV = 0.0f, info.gY = 0.0f;
  info.grounded = 0;
  info.djCharge = 0;
  dead = 0, finished = 0, dir = 0;
  lLR = 0.0f, rLR = 0.0f, tLLR = 0.0f, tRLR = 0.0f, lAnim = 0.0f;
  isReplay = 0;
  deathTimer = 0;
  return 1;
}

void Robot::update(CtrlModule *ctrl, Level &lvl) {
  isReplay = ctrl->isReplay();
  if (!dead) {
    if (!finished) {
      info.xV += ((ctrl->jX() * 4.0f * float(1 + ctrl->run())) - info.xV) / 8.0f;
      if (ctrl->jump() && !ctrl->lJump()) {
        if (info.grounded) {
          info.yV = -16.0f;
          info.grounded = 0;
          ctrl->rumble(0.75f, 125);
          if (!isReplay) TXL_PlaySound(jumpSnd);
        } else if (info.djCharge) {
          info.yV = -12.0f;
          info.djCharge = 0;
          ctrl->rumble(0.625f, 125);
          if (!isReplay) {
            TXL_PlaySound(djSnd);
            for (int i = 0; i < 3; i++) {
              for (int j = 0; j < 5; j++) {
                addParticle({info.x, info.y, (1 + i) * cos((j + 2) * 3.14f / 8.0f), (2 + i) * sin((j + 2) * 3.14f / 8.0f), 30, 1.0f, float(i) / 2.0f, 0.0f, 0.5f, 2.0f});
              }
            }
          }
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
    lUpdate();
  } else if (!isReplay && deathTimer < 15) {
    if (deathTimer % 2) TXL_PlaySound(explodeSnd);
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 32; j++) {
        if ((i + j + deathTimer) % 8 == 0)addParticle({info.x, info.y, (2 + i) * cos((j + deathTimer % 2) * 3.14f / 16.0f), (2 + i) * sin((j + deathTimer % 2) * 3.14f / 16.0f), 30, 1.0f, float(i) / 2.0f, 0.0f, 0.5f, 2.0f});
      }
    }
    deathTimer++;
  }
}

void Robot::motionCalc(CtrlModule *ctrl) {
  if (!info.grounded) {
    if (info.yV < 16.0f) info.yV += 1.0f + float(1 - (ctrl->jump() || !info.djCharge));
  }
  if (fabs(info.xV) < 0.25f) info.xV = 0.0f;
}

void Robot::colCalc(CtrlModule *ctrl, Level &lvl) {
  if (isInFloor(0.0f, 0.0f, lvl)) {
    bool floor = 0;
    for (int i = 0; i < 128; i++) {
      if (!isInFloor(0.0f, 0.0f - i, lvl)) { // floor check
        ctrl->rumble(fmin(fabs(info.yV) / 24.0f, 1.0f), 125);
        if (!isReplay) TXL_PlaySound(landSnd);
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
        if (!isReplay && info.xV > 3.0f) TXL_PlaySound(bumpSnd);
        info.x -= i + 1;
        info.xV = 0.0f;
        break;
      }
      if (!isInFloor(0.0f + i, 0.0f, lvl)) { // left wall check
        ctrl->rumble(fmin(fabs(info.xV) / 6.0f, 1.0f), 125);
        if (!isReplay && info.xV < -3.0f) TXL_PlaySound(bumpSnd);
        info.x += i + 1;
        info.xV = 0.0f;
        break;
      }
      if (!isInFloor(0.0f, 0.0f + i, lvl)) { // ceiling check
        ctrl->rumble(fmin(fabs(info.yV) / 24.0f, 1.0f), 125);
        if (!isReplay) TXL_PlaySound(bumpSnd);
        info.y += i + 1;
        info.yV = round(info.yV * -0.5f);
        break;
      }
    }
  }
  if (!isInFloor(0.0f, 2.0f, lvl)) info.grounded = 0;
  if (lvl.inLethal(info.x, info.y)) dead = 1;
  if (lvl.inGoal(info.x, info.y)) finished = 1;
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
  if (deathTimer >= 15) return;
  robotTex.setColorMod(float(2 - isReplay) / 2.0f);
  robotTex.setClip(16, 24, 0, 16);
  robotTex.render(info.x - cX - limbXOff, info.y - cY - 8 + limbYOff, lLR);
  robotTex.setClip(24, 16, 0, 16);
  robotTex.render(info.x - cX + limbXOff, info.y - cY - 8 + limbYOff, rLR);
  robotTex.setClip(16 * dir, 16 * !dir, 0, 16);
  robotTex.render(info.x - cX, info.y - cY - 8.0f + (info.xV / 8.0f) * (sin(lAnim * 1.57f / 8.0f) + cos(lAnim * 1.57f / 8.0f)));
}

void Robot::end() {
  robotTex.free();
}

void Robot::modCam(float &cX, float &cY, Level &lvl) {
  cX += (info.x - cX - float(320 + 80 * (info.xV / -8))) / 8.0f;
  cY += (info.gY - cY - 225.0f) / 8.0f;
  lvl.modCam(cX, cY, info.x, info.y);
}

void Robot::lUpdate() {
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
}