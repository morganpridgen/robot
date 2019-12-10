#include "particle.h"
#include <cmath>
#include <TEXEL/texel.h>

struct PNode {
  ParticleInfo data;
  PNode *next;
};
PNode *first;

void updateP(ParticleInfo*);
void renderP(ParticleInfo*, float, float);

bool initParticles() {
  first = nullptr;
  return 1;
}

void updateParticles() {
  PNode *ptr = first, *lPtr;
  while (ptr != nullptr) {
    updateP(&(ptr->data));
    ptr->data.timer--;
    if (ptr->data.timer < 1) {
      if (ptr == first) {
        first = ptr->next;
        delete ptr;
        ptr = first;
      } else {
        lPtr->next = ptr->next;
        delete ptr;
        ptr = lPtr->next;
      }
    } else lPtr = ptr, ptr = ptr->next;
  }
}

void renderParticles(float cX, float cY) {
  PNode *ptr = first;
  while (ptr != nullptr) {
    renderP(&(ptr->data), cX, cY);
    ptr = ptr->next;
  }
}

void endParticles() {
  PNode *ptr = first, *lPtr;
  while (ptr != nullptr) {
    lPtr = ptr;
    ptr = ptr->next;
    delete lPtr;
  }
  first = nullptr;
}

void addParticle(ParticleInfo nP) {
  PNode *nN = new PNode;
  nN->data = nP;
  nN->next = first;
  first = nN;
}

void updateP(ParticleInfo *info) {
  info->x += info->xV;
  info->y += info->yV;
  float a = atan2(info->yV, info->xV), d = sqrt(info->xV * info->xV + info->yV * info->yV) * 0.9f;
  info->xV = d * cos(a), info->yV = d * sin(a);
  /*info->xV *= 0.9f;
  if (fabs(info->xV) < 1.0f) info->xV = 0.0f;
  info->yV *= 0.9f;
  if (fabs(info->yV) < 1.0f) info->yV = 0.0f;*/
}

void renderP(ParticleInfo *info, float cX, float cY) {
  TXL_RenderQuad(info->x - cX, info->y - cY, info->size, info->size, {info->r, info->g, info->b, info->a});
}