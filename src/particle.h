#ifndef particleh
#define particleh

struct ParticleInfo {
  float x, y, xV, yV;
  int timer;
  float r, g, b, a;
  float size;
};

bool initParticles();
void updateParticles();
void renderParticles(float, float);
void endParticles();
void addParticle(ParticleInfo);

#endif