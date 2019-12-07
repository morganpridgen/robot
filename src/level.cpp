#include "level.h"
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <TEXEL/texel.h>
#include "robot.h"

#define tileSize 32.0f

TXL_Texture groundTex;

int nextInt(TXL_File *f) {
  int out = 0;
  char tmp = 0;
  bool readNum = 0;
  while (1) {
    if (!f->read(&tmp, sizeof(tmp))) {
      if (readNum) return out;
      else return -1;
    }
    if (tmp >= '0' && tmp <= '9') readNum = 1;
    else if (readNum) return out;
    if (readNum) {
      out *= 10;
      out += tmp - '0';
    }
  }
}

bool Level::init(const char *name, Robot &robot) {
  TXL_File f;
  char path[256], root[256];
  sprintf(root, "levels/%s", name);
  sprintf(path, "%s/terrain.txt", root);
  if (!f.init(TXL_DataPath(path), 'r')) return 0;
  length = nextInt(&f);
  depth = nextInt(&f);
  terrain = new TileSpan[length * depth];
  for (int i = 0; i < length * depth; i++) {
    do f.read(&(terrain[i].type), sizeof(terrain[i].type));
    while (terrain[i].type <= ' ');
    terrain[i].len = nextInt(&f);
  }
  f.close();
  sprintf(path, "%s/robot.txt", root);
  if (!f.init(TXL_DataPath(path), 'r')) return 0;
  int rX, rY;
  rX = nextInt(&f);
  rY = nextInt(&f);
  robot.setPos(rX * tileSize + tileSize / 2.0f, 360.0f - (rY * tileSize));
  
  //if (!groundTex.load(TXL_DataPath("terrain.png"), 32, 32)) return 0;
  return 1;
}

void Level::update() {
  
}

void Level::render(float cX, float cY) {
  for (int i = cX / tileSize; i < (cX + 640.0f) / tileSize; i++) {
    int height = 0;
    for (int j = 0; j < depth; j++) {
      if (terrain[i * depth + j].type == 'E') {
        height += terrain[i * depth + j].len;
        continue;
      }
      for (int k = 0; k < terrain[i * depth + j].len; k++) {
        switch (terrain[i * depth + j].type) {
          case 'S': {
            TXL_RenderQuad(i * tileSize + 16 - cX, 360.0f - ((k + height + 1) * tileSize) + 16 - cY, tileSize, tileSize, {1.0f, 1.0f, 1.0f, 1.0f});
            break;
          }
          case 'L': {
            TXL_RenderQuad(i * tileSize + 16 - cX, 360.0f - ((k + height + 1) * tileSize) + 16 - cY, tileSize, tileSize, {1.0f, 1.0f, 0.0f, 1.0f});
            break;
          }
        }
      }
      height += terrain[i * depth + j].len;
    }
    if (depth % 2 == 0) {
      while ((height * tileSize) + cY < 360) { // height * tileSize - cY < 360
        TXL_RenderQuad(i * tileSize + 16 - cX, 360.0f - ((height + 1) * tileSize) + 16 - cY, tileSize, tileSize, {1.0f, 1.0f, 1.0f, 1.0f});
        height++;
      }
    }
  }
}

void Level::end() {
  groundTex.free();
  delete [] terrain;
  terrain = nullptr;
}

void Level::modCam(float &cX, float &cY, float pX, float pY) {
  int wPX = pX / tileSize;
  int wPY = (360.0f - pY) / tileSize;
  if (cX < 0.0f) cX = 0.0f;
  if (cX > (length - 20) * tileSize) cX = (length - 20) * tileSize;
  
  int wPH = 0;
  for (int i = 0; i < depth; i++) {
    if (wPH + terrain[wPX * depth + i].len > wPY) break;
    wPH += terrain[wPX * depth + i].len;
  }
  if (wPY - wPH < 8) {
    float camTarget = 90.0f - wPH * tileSize;
    cY += (camTarget - cY) / 8.0f;
    if (cY > 0.0f) cY = 0.0f;
  }
}

bool Level::inFloor(float x, float y) {
  int pX = x / tileSize;
  int pY = (360.0f - y) / tileSize + 1;
  int h = 0, lH = 0;
  
  if (pX < 0 || pX >= length || pY < 0) return 1;
  
  for (int i = 0; i < depth; i++) {
    h += terrain[pX * depth + i].len;
    if (pY <= h && pY >= lH) return terrain[pX * depth + i].type == 'S';
    lH = h;
  }
  return depth % 2 == 0;
}