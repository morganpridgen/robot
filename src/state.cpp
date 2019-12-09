#include "state.h"
#include <cstring>
#include <cmath>
#include <climits>
#include <TEXEL/texel.h>

char level[64];
bool highScoreReplay = 0;

bool GameState::init() {
  if (!robot.init()) return 0;
  if (!lvl.init(level, robot)) return 0;
  robot.getPos(cX, cY);
  cX -= 320.0f;
  cY -= 180.0f;
  respawnTimer = 0, gameTimer = 0, timerStart = 0;
  ctrlModule = nullptr;
  return 1;
}

bool GameState::engine() {
  if (fabs(ctrlModule->jX()) > 0.125f || fabs(ctrlModule->jY()) > 0.125f || ctrlModule->jump()) timerStart = 1;
  robot.update(ctrlModule, lvl);
  lvl.update();
  if (respawnTimer) respawnTimer++;
  if (robot.getDead() && !respawnTimer) respawnTimer = 1;
  if (respawnTimer >= 120) return 1;
  
  robot.modCam(cX, cY, lvl);
  gameTimer += !lvl.isFinished() && timerStart;
  return 0;
}

void GameState::render() {
  lvl.render(cX, cY);
  robot.render(cX, cY);
  
  char timer[9];
  sprintf(timer, "%02i:%05.2f", gameTimer / 3600, fmod(gameTimer, 3600) / 60.0f);
  TXL_Texture *timerTex = TXL_RenderText(timer, 1.0f, 1.0f, 1.0f);
  timerTex->setColorMod(0.625f, 0.625f, 0.625f);
  for (int i = 0; i < 9; i++) timerTex->render(641.0f - 16.0f - timerTex->width() / 2.0f - (i % 3), 17.0f + timerTex->height() / 2.0f - (i / 3));
  timerTex->setColorMod(1.0f, 1.0f, 1.0f);
  timerTex->render(640.0f - 16.0f - timerTex->width() / 2.0f, 16.0f + timerTex->height() / 2.0f);
  delete timerTex;
}

void GameState::end() {
  delete ctrlModule;
  lvl.end();
  robot.end();
}



bool PlayState::init() {
  if (!GameState::init()) return 0;
  ctrlModule = new PlayerCtrlModule;
  if (!recording.init(TXL_SavePath(".tmp"), 'w')) return 0;
  lastFinish = 0, highScore = 0;
  return 1;
}

BaseState *PlayState::update(TXL_Controller *ctrls[4]) {
  if (!lvl.isFinished()) {
    ctrlModule->update(ctrls[0]);
    float jX = ctrlModule->jX(), jY = ctrlModule->jY();
    bool jump = ctrlModule->jump(), run = ctrlModule->run();
    if (timerStart) {
      recording.write(&jX, sizeof(jX));
      recording.write(&jY, sizeof(jY));
      recording.write(&jump, sizeof(jump));
      recording.write(&run, sizeof(run));
    }
  } else if (!lastFinish) {
    TXL_File f;
    char path[64];
    sprintf(path, "%s-time", level);
    if (!f.init(TXL_SavePath(path), 'r')) highScore = 1;
    else {
      int highTime;
      f.read(&highTime, sizeof(highTime));
      highScore = gameTimer < highTime;
      f.close();
    }
    highScoreReplay = highScore;
  }
  if (engine()) {
    if (lvl.isFinished()) return highScore ? (BaseState*)(new ReplayState) : (BaseState*)(new LevelSelectState);
    return new PlayState;
  }
  return nullptr;
}

void PlayState::render() {
  GameState::render();
  if (highScore) {
    TXL_Texture *winTex = TXL_RenderText("New Record!", 1.0f, 1.0f, 1.0f);
    winTex->setColorMod(0.625f, 0.625f, 0.625f);
    for (int i = 0; i < 9; i++) winTex->render(321.0f - (i % 3), 181.0f - (i / 3));
    winTex->setColorMod(1.0f, 1.0f, 1.0f);
    winTex->render(320.0f, 180.0f);
  }
}

void PlayState::end() {
  recording.close();
  if (highScore) {
    char path[64];
    TXL_File f;
    sprintf(path, "%s-time", level);
    if (f.init(TXL_SavePath(path), 'w')) {
      f.write(&gameTimer, sizeof(gameTimer));
      f.close();
    }
    if (recording.init(TXL_SavePath(".tmp"), 'r')) {
      sprintf(path, "%s-play", level);
      if (f.init(TXL_SavePath(path), 'w')) {
        char tmp;
        while (recording.read(&tmp, sizeof(tmp))) f.write(&tmp, sizeof(tmp));
        f.close();
      }
      recording.close();
    }
  }
  GameState::end();
}



bool ReplayState::init() {
  if (!GameState::init()) return 0;
  ctrlModule = new RecordedCtrlModule;
  if (!((RecordedCtrlModule*)ctrlModule)->init(level)) return 0;
  return 1;
}

BaseState *ReplayState::update(TXL_Controller *ctrls[4]) {
  ctrlModule->update(nullptr);
  if (engine()) return new LevelSelectState;
  if (((RecordedCtrlModule*)ctrlModule)->timeLeft() < -300) {
    TXL_File f;
    char path[64];
    sprintf(path, "%s-time", level);
    if (f.init(TXL_SavePath(path), 'w')) {
      int tmp = INT_MAX;
      f.write(&tmp, sizeof(tmp));
      f.close();
    }
    return new LevelSelectState;
  }
  return nullptr;
}

void ReplayState::render() {
  GameState::render();
  if (highScoreReplay) {
    TXL_Texture *winTex = TXL_RenderText("New Record!", 1.0f, 1.0f, 1.0f);
    winTex->setColorMod(0.625f, 0.625f, 0.625f);
    for (int i = 0; i < 9; i++) winTex->render(321.0f - (i % 3), 181.0f - (i / 3));
    winTex->setColorMod(1.0f, 1.0f, 1.0f);
    winTex->render(320.0f, 180.0f);
  }
}

void ReplayState::end() {
  GameState::end();
  highScoreReplay = 0;
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
  
  char path[64];
  sprintf(path, "%s-time", levelList[0]);
  if (f.init(TXL_SavePath(path), 'r')) {
    f.read(&stageTime, sizeof(stageTime));
    f.close();
  } else stageTime = INT_MAX;
  return 1;
}

BaseState *LevelSelectState::update(TXL_Controller *ctrls[4]) {
  if (ctrls[0]->leftJoyX() < -0.5f && lJX >= -0.5f) selectedLevel--;
  if (ctrls[0]->leftJoyX() > 0.5f && lJX <= 0.5f) selectedLevel++;
  lJX = ctrls[0]->leftJoyX();
  
  while (selectedLevel < 0) selectedLevel += lvlCount;
  while (selectedLevel >= lvlCount) selectedLevel -= lvlCount;
  
  if (lSL != selectedLevel) {
    TXL_File f;
    char path[64];
    sprintf(path, "%s-time", levelList[selectedLevel]);
    if (f.init(TXL_SavePath(path), 'r')) {
      f.read(&stageTime, sizeof(stageTime));
      f.close();
    } else stageTime = INT_MAX;
  }
  if (ctrls[0]->buttonRelease(CtrlA)) {
    strcpy(level, levelList[selectedLevel]);
    return new PlayState;
  }
  if (ctrls[0]->buttonRelease(CtrlX)) {
    strcpy(level, levelList[selectedLevel]);
    return new ReplayState;
  }
  
  lSL = selectedLevel;
  return nullptr;
}

void LevelSelectState::render() {
  TXL_Texture *levelTex = TXL_RenderText(levelList[selectedLevel], 1.0f, 1.0f, 1.0f);
  levelTex->setColorMod(0.625f, 0.625f, 0.625f);
  for (int i = 0; i < 9; i++) levelTex->render(321.0f - (i % 3), 181.0f - (i / 3));
  levelTex->setColorMod(1.0f, 1.0f, 1.0f);
  levelTex->render(320.0f, 180.0f);
  delete levelTex;
  
  char time[32] = "No record";
  if (stageTime != INT_MAX) sprintf(time, "Fastest time: %02i:%05.2f", stageTime / 3600, fmod(stageTime, 3600) / 60.0f);
  TXL_Texture *timeTex = TXL_RenderText(time, 1.0f, 1.0f, 1.0f);
  timeTex->setColorMod(0.625f, 0.625f, 0.625f);
  for (int i = 0; i < 9; i++) timeTex->render(321.0f - (i % 3), 271.0f - (i / 3));
  timeTex->setColorMod(1.0f, 1.0f, 1.0f);
  timeTex->render(320.0f, 270.0f);
}

void LevelSelectState::end() {
  for (int i = 0; i < lvlCount; i++) delete levelList[i];
  delete levelList;
  levelList = nullptr;
}