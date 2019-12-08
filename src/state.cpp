#include "state.h"
#include <cstring>
#include <cmath>
#include <TEXEL/texel.h>

char level[64];

bool PlayState::init() {
  if (!robot.init()) return 0;
  if (!lvl.init(level, robot)) return 0;
  robot.getPos(cX, cY);
  cX -= 320.0f;
  cY -= 180.0f;
  respawnTimer = 0;
  return 1;
}

BaseState *PlayState::update(TXL_Controller *ctrls[4]) {
  robot.update(ctrls[0], lvl);
  lvl.update();
  if (respawnTimer) respawnTimer++;
  if (robot.getDead() && !respawnTimer) respawnTimer = 1;
  if (respawnTimer >= 120) return lvl.isFinished() ? (BaseState*)(new LevelSelectState) : (BaseState*)(new PlayState);
  
  robot.modCam(cX, cY, lvl);
  return nullptr;
}

void PlayState::render() {
  lvl.render(cX, cY);
  robot.render(cX, cY);
}

void PlayState::end() {
  lvl.end();
  robot.end();
}



bool LevelSelectState::init() {
  char tmpLevels[64][64];
  TXL_File f;
  selectedLevel = 0, lvlCount = 0;
  if (!f.init(TXL_DataPath("levels/levellist.txt"), 'r')) return 0;
  for (int i = 0; i < 64; i++) {
    for (int j = 0; j < 64; j++) {
      tmpLevels[i][j] = 0;
      char tmp;
      if (!f.read(&tmp, sizeof(tmp)) || tmp < ' ') break;
      tmpLevels[i][j] = tmp;
    }
    if (tmpLevels[i][0] == 0) break;
    lvlCount++;
  }
  levelList = new char*[lvlCount];
  for (int i = 0; i < lvlCount; i++) {
    levelList[i] = new char[strlen(tmpLevels[i]) + 1];
    strcpy(levelList[i], tmpLevels[i]);
  }
  return 1;
}

BaseState *LevelSelectState::update(TXL_Controller *ctrls[4]) {
  if (ctrls[0]->leftJoyX() < -0.5f && lJX >= -0.5f) selectedLevel--;
  if (ctrls[0]->leftJoyX() > 0.5f && lJX <= 0.5f) selectedLevel++;
  lJX = ctrls[0]->leftJoyX();
  
  while (selectedLevel < 0) selectedLevel += lvlCount;
  while (selectedLevel >= lvlCount) selectedLevel -= lvlCount;
  if (ctrls[0]->buttonRelease(CtrlA)) {
    strcpy(level, levelList[selectedLevel]);
    return new PlayState;
  }
  
  return nullptr;
}

void LevelSelectState::render() {
  TXL_Texture *levelTex = TXL_RenderText(levelList[selectedLevel], 1.0f, 1.0f, 1.0f);
  levelTex->setColorMod(0.625f, 0.625f, 0.625f);
  for (int i = 0; i < 9; i++) levelTex->render(321.0f - (i % 3), 181.0f - (i / 3));
  levelTex->setColorMod(1.0f, 1.0f, 1.0f);
  levelTex->render(320.0f, 180.0f);
}

void LevelSelectState::end() {
  for (int i = 0; i < lvlCount; i++) delete levelList[i];
  delete levelList;
  levelList = nullptr;
}