#include "level.h"
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <TEXEL/texel.h>
#include "robot.h"

#define tileSize 32.0f

TXL_Texture terrainTex;

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
  solidTop = nextInt(&f);
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
  
  if (!terrainTex.load(TXL_DataPath("terrain.png"), 64, 64)) return 0;
  animTimer = 0;
  return 1;
}

void Level::update() {
  animTimer++;
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
            terrainTex.setClip(0, 32, 0, 32);
            terrainTex.render(i * tileSize + 16 - cX, 360.0f - ((k + height + 1) * tileSize) + 16 - cY);
            terrainTex.setClip(32, 64, 0, 32);
            terrainTex.render(i * tileSize + 16 - cX, 360.0f - ((k + height + 1) * tileSize) + 16 - cY, animTimer * (((i + k + height) % 2) ? -1 : 1));
            break;
          }
          case 'L': {
            terrainTex.setClip(0, 32, 32, 64);
            terrainTex.render(i * tileSize + 16 - cX, 360.0f - ((k + height + 1) * tileSize) + 16 - cY);
            terrainTex.setClip(32, 64, 32, 64);
            char belowType = typeAt(i, k + height);
            if (belowType == 'S' || belowType == 'L') terrainTex.render(i * tileSize + 16 - cX, 360.0f - ((k + height + 1) * tileSize) + 16 - cY, 0);
            char leftType = typeAt(i - 1, k + height + 1);
            if (leftType == 'S' || leftType == 'L') terrainTex.render(i * tileSize + 16 - cX, 360.0f - ((k + height + 1) * tileSize) + 16 - cY, 90);
            char aboveType = typeAt(i, k + height + 2);
            if (aboveType == 'S' || aboveType == 'L') terrainTex.render(i * tileSize + 16 - cX, 360.0f - ((k + height + 1) * tileSize) + 16 - cY, 180);
            char rightType = typeAt(i + 1, k + height + 1);
            if (rightType == 'S' || rightType == 'L') terrainTex.render(i * tileSize + 16 - cX, 360.0f - ((k + height + 1) * tileSize) + 16 - cY, 270);
            break;
          }
        }
      }
      height += terrain[i * depth + j].len;
    }
    if (solidTop) {
      while ((height * tileSize) + cY < 360) {
        terrainTex.setClip(0, 32, 0, 32);
        terrainTex.render(i * tileSize + 16 - cX, 360.0f - ((height + 1) * tileSize) + 16 - cY);
        terrainTex.setClip(32, 64, 0, 32);
        terrainTex.render(i * tileSize + 16 - cX, 360.0f - ((height + 1) * tileSize) + 16 - cY, animTimer * (((i + height) % 2) ? -1 : 1));
        height++;
      }
    }
  }
}

void Level::end() {
  terrainTex.free();
  delete [] terrain;
  terrain = nullptr;
}

void Level::modCam(float &cX, float &cY, float pX, float pY) {
  if (cX < 0.0f) cX = 0.0f;
  if (cX > (length - 20) * tileSize) cX = (length - 20) * tileSize;
  if (cY > 0.0f) cY = 0.0f;
}

bool Level::inTile(float x, float y, char tile) {
  return typeAt(x / tileSize, (360.0f - y) / tileSize + 1) == tile;
}

char Level::typeAt(int x, int y) {
  if (x < 0 || x >= length) return 'S';
  if (y < 0) return 'L';
  
  int h = 0, lH = 0;
  for (int i = 0; i < depth; i++) {
    h += terrain[x * depth + i].len;
    if (y <= h && y >= lH) return terrain[x * depth + i].type;
    lH = h;
  }
  return solidTop ? 'S' : 'E';
}

bool Level::inFloor(float x, float y) {
  return inTile(x, y, 'S');
}

bool Level::inLethal(float x, float y) {
  return inTile(x, y, 'L');
}