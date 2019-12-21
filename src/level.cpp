#include "level.h"
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <TEXEL/texel.h>
#include "robot.h"
#include "particle.h"

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
  sprintf(path, "%s/points.txt", root);
  if (!f.init(TXL_DataPath(path), 'r')) return 0;
  int rX, rY;
  rX = nextInt(&f);
  rY = nextInt(&f);
  robot.setPos(rX * tileSize + tileSize / 2.0f, 360.0f - (rY * tileSize));
  gX = nextInt(&f);
  gY = nextInt(&f);
  f.close();
  
  if (!terrainTex.load(TXL_DataPath("terrain.png"), 64, 64)) return 0;
  animTimer = 0;
  lCX = 0.0f, lCY = 0.0f;
  return 1;
}

void Level::update() {
  for (int i = lCX / tileSize; i < (lCX + 640.0f) / tileSize; i++) {
    int height = 0;
    for (int j = 0; j < depth; j++) {
      for (int k = 0; k < terrain[i * depth + j].len; k++) {
        switch (terrain[i * depth + j].type) {
          case 'L': {
            if (animTimer % 32 == 0) {
              for (int l = 0; l < 8; l++) {
                addParticle({i * tileSize + 16, 360.0f - ((k + height + 1) * tileSize) + 16, 2.0f * cos(l * 3.14f / 4.0f), 2.0f * sin(l * 3.14f / 4.0f), 15, 1.0f, 1.0f, 0.5f, 0.75f, 2.0f});
              }
            }
          }
        }
      }
      height += terrain[i * depth + j].len;
    }
  }
  if (fabs(gX * tileSize + 16 - lCX - 320.0f) < 320.0f && fabs(360.0f - gY * tileSize - 16 - lCY - 180.0f)) {
    for (int i = 0; i < 8; i++) {
      addParticle({gX * tileSize + 16, 360.0f - gY * tileSize - 16, 2.0f * cos(3.14f * (i / 4.0f + animTimer / 64.0f)), 2.0f * sin(3.14f * (i / 4.0f + animTimer / 64.0f)), 15, 1.0f, 1.0f, 1.0f, 0.25f, 2.0f});
    }
  }
  animTimer++;
}

void Level::render(float cX, float cY) {
  terrainTex.setColorMod(0.25f, 0.125f, 0.0f);
  for (int i = cX / tileSize; i < (cX + 2560.0f) / tileSize + 1; i++) {
    for (int j = cY / tileSize; j < (1440.0f - cY) / tileSize + 1; j++) {
      if (i % 2 && j % 2) {
        terrainTex.setClip(0, 32, 0, 32);
        terrainTex.render(i * (tileSize / 4.0f) - cX / 4.0f, 360.0f - (j * tileSize / 4.0f) - cY / 4.0f, 0.5f, 0.5f);
        terrainTex.setClip(32, 64, 0, 32);
        if ((i / 2 + j / 2) % 2) terrainTex.render(i * (tileSize / 4.0f) - cX / 4.0f, 360.0f - (j * tileSize / 4.0f) - cY / 4.0f, 0.5f, 0.5f, animTimer / 2);
      }
    }
  }
  
  for (int i = cX / tileSize; i < (cX + 640.0f) / tileSize; i++) {
    int height = 0;
    for (int j = 0; j < depth; j++) {
      if (terrain[i * depth + j].type == 'E') {
        height += terrain[i * depth + j].len;
        continue;
      }
      for (int k = 0; k < terrain[i * depth + j].len; k++) {
        float tX = i * tileSize + 16 - cX, tY = 360.0f - ((k + height + 1) * tileSize) + 16 - cY;
        switch (terrain[i * depth + j].type) {
          case 'S': {
            float b = sin((i + k + height) * 3.14f / 16.0f) / 8.0f + 0.875f;
            terrainTex.setColorMod(b / 2.0f, b / 3.0f, 0);
            terrainTex.setClip(0, 32, 0, 32);
            terrainTex.render(tX, tY);
            terrainTex.setClip(32, 64, 0, 32);
            terrainTex.render(tX, tY, animTimer * (((i + k + height) % 2) ? -1 : 1));
            break;
          }
          case 'L': {
            terrainTex.setColorMod(0.875f, 0.875f, 0.875f);
            terrainTex.setClip(0, 32, 32, 64);
            terrainTex.render(tX, tY);
            terrainTex.setClip(32, 64, 32, 64);
            char belowType = typeAt(i, k + height);
            if (belowType == 'S' || belowType == 'L') terrainTex.render(tX, tY, 0);
            char leftType = typeAt(i - 1, k + height + 1);
            if (leftType == 'S' || leftType == 'L') terrainTex.render(tX, tY, 90);
            char aboveType = typeAt(i, k + height + 2);
            if (aboveType == 'S' || aboveType == 'L') terrainTex.render(tX, tY, 180);
            char rightType = typeAt(i + 1, k + height + 1);
            if (rightType == 'S' || rightType == 'L') terrainTex.render(tX, tY, 270);
            break;
          }
        }
      }
      height += terrain[i * depth + j].len;
    }
    if (solidTop) {
      while ((height * tileSize) + cY < 360) {
        float b = sin((i + height) * 3.14f / 16.0f) / 8.0f + 0.875f;
        terrainTex.setColorMod(b / 2.0f, b / 3.0f, 0);
        terrainTex.setClip(0, 32, 0, 32);
        terrainTex.render(i * tileSize + 16 - cX, 360.0f - ((height + 1) * tileSize) + 16 - cY);
        terrainTex.setClip(32, 64, 0, 32);
        terrainTex.render(i * tileSize + 16 - cX, 360.0f - ((height + 1) * tileSize) + 16 - cY, animTimer * (((i + height) % 2) ? -1 : 1));
        height++;
      }
    }
  }
  terrainTex.setColorMod(1.0f, 1.0f, 1.0f);
  terrainTex.setClip(32, 64, 0, 32);
  terrainTex.render(gX * tileSize + 16 - cX, 360.0f - gY * tileSize - 16 - cY, animTimer);
  terrainTex.render(gX * tileSize + 16 - cX, 360.0f - gY * tileSize - 16 - cY, -animTimer);
  terrainTex.setClip(0, 32, 32, 64);
  terrainTex.render(gX * tileSize + 16 - cX, 360.0f - gY * tileSize - 16 - cY);
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
  lCX = cX, lCY = cY;
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

bool Level::inGoal(float x, float y) {
  return int(x / tileSize) == gX && int((360.0f - y) / tileSize) == gY;
}