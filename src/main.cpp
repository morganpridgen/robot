#include <TEXEL/texel.h>
#include "state.h"
#include "serverinter.h"

bool init();
void update();
void render();
void end();

TXL_Display disp;
TXL_Controller *ctrls[4];
BaseState *state;
bool loop = 1;
bool stateChange = 0;

int main(int argc, char **argv) {
  if (init()) {
    while (loop) {
      loop = TXL_Events(&disp);
      
      do {
        stateChange = 0;
        update();
      } while (stateChange);
      render();
      disp.refresh();
    }
    end();
    return 0;
  }
  return 1;
}

bool init() {
  if (!TXL_Init()) return 0;
  TXL_InitPaths("robot");
  if (!TXL_LoadFont(TXL_DataPath("font.png"))) return 0;
  
  if (!disp.init("Robot")) return 0;
  for (int i = 0; i < 4; i++) {
    ctrls[i] = new TXL_Controller;
    if (!ctrls[i]->init()) {
      delete ctrls[i];
      ctrls[i] = nullptr;
      if (i == 0) {
        ctrls[i] = new TXL_Keyboard;
        ctrls[i]->init();
      }
    }
  }
  
  state = new InitState;
  if (!state->init()) return 0;
  return 1;
}

void update() {
  for (int i = 0; i < 4; i++) TXL_ManageController(ctrls[i]);
  BaseState *newState = state->update(ctrls);
  if (newState) {
    state->end();
    delete state;
    state = newState;
    if (!state->init()) loop = 0;
    stateChange = 1;
    activeServer = initInet();
  }
}

void render() {
  state->render();
}

void end() {
  TXL_UnloadFont();
  if (state) {
    state->end();
    delete state;
  }
  for (int i = 0; i < 4; i++) {
    if (ctrls[i]) {
      ctrls[i]->end();
      delete ctrls[i];
    }
  }
  disp.end();
  TXL_EndSound();
  TXL_End();
}