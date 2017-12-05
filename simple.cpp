// Simple example effect:
// Draws a noise pattern modulated by an expanding sine wave.

#include <math.h>

#include "lib/color.h"
#include "lib/effect.h"
#include "lib/effect_runner.h"

class MyEffect : public Effect {
public:
  MyEffect() {}

  float time = 0.0f;

  void beginFrame(const FrameInfo &f) override {
    time = std::fmod(time + f.timeDelta * 0.05f, 2.0f);
  }

  void shader(Vec3 &rgb, const PixelInfo &p) const override {
    if (time < 1.0f) {
      hsv2rgb(rgb, time, 1.0f, std::fmod(time * 3.0f, 1.0f));
    }
    // else if (time < 2.0f) {
    //   float t = std::fmod(time * 3.0f, 1.0f);
    //   hsv2rgb(rgb, t, 1.0f, int(p.point[0]) % 2 == (int(t * 4) % 2) ? 1.0f : 0.0f);
    // }
    else {
      float t = std::fmod(time * 2.0f, 1.0f);
      hsv2rgb(rgb, t + p.point[0] * 0.05f, 1.0f, 1.0f);
    }
    // float scale = 0.1f;
    // float h = std::fmod(2.0f + std::sin(cycle + scale * p.point[0]) + std::sin(cycle + scale * p.point[2]), 1.0f);
    // hsv2rgb(rgb, h, 1.0f, 0.5f);
  }
};

int main(int argc, char **argv) {
  EffectRunner r;

  MyEffect e;
  r.setEffect(&e);

  // Defaults, overridable with command line options
  r.setMaxFrameRate(60);
  r.setLayout("layouts/grid12x10.json");

  return r.main(argc, argv);
}