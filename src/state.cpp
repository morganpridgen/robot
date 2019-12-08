#include "state.h"
#include <cstring>
#include <cmath>
#include <TEXEL/texel.h>

char level[64];

bool GameState::init() {
  if (!robot.init()) return 0;
  if (!lvl.init(level, robot)) return 0;
  robot.getPos(cX, cY);
  cX -= 320.0f;
  cY -= 180.0f;
  respawnTimer = 0;
  ctrlModule = nullptr;
  return 1;
}

bool GameState::engine() {
  robot.update(ctrlModule, lvl);
  lvl.update();
  if (respawnTimer) respawnTimer++;
  if (robot.getDead() && !respawnTimer) respawnTimer = 1;
  if (respawnTimer >= 120) return 1;
  
  robot.modCam(cX, cY, lvl);
  return 0;
}

void GameState::render() {
  lvl.render(cX, cY);
  robot.render(cX, cY);
}

void GameState::end() {
  delete ctrlModule;
  lvl.end();
  robot.end();
}



bool PlayState::init() {
  if (!GameState::init()) return 0;
  ctrlModule = new PlayerCtrlModule;
  if (!recording.init(TXL_SavePath("tmp.bin"), 'w')) return 0;
  return 1;
}

BaseState *PlayState::update(TXL_Controller *ctrls[4]) {
  if (!lvl.isFinished()) {
    ctrlModule->update(ctrls[0]);
    float jX = ctrlModule->jX(), jY = ctrlModule->jY();
    bool jump = ctrlModule->jump(), run = ctrlModule->run();
    recording.write(&jX, sizeof(jX));
    recording.write(&jY, sizeof(jY));
    recording.write(&jump, sizeof(jump));
    recording.write(&run, sizeof(run));
  }
  if (engine()) return lvl.isFinished() ? (BaseState*)(new ReplayState) : (BaseState*)(new PlayState);
  return nullptr;
}

void PlayState::render() {
  GameState::render();
}

void PlayState::end() {
  recording.close();
  GameState::end();
}



bool ReplayState::init() {
  if (!GameState::init()) return 0;
  ctrlModule = new RecordedCtrlModule;
  if (!((RecordedCtrlModule*)ctrlModule)->init(TXL_SavePath("tmp.bin"))) return 0;
  return 1;
}

BaseState *ReplayState::update(TXL_Controller *ctrls[4]) {
  ctrlModule->update(nullptr);
  if (engine()) return new LevelSelectState;
  return nullptr;
}

void ReplayState::render() {
  GameState::render();
}

void ReplayState::end() {
  GameState::end();
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