#include "FxRipple.hpp"

#include "cinder/Rand.h"
#include "cinder/Log.h"

using namespace glm;
using namespace ci;

const ivec2 neighbor_directions[8]{
  ivec2{  0, -1 },
  ivec2{  1, -1 },
  ivec2{  1,  0 },
  ivec2{  1,  1 },
  ivec2{  0,  1 },
  ivec2{ -1,  1 },
  ivec2{ -1,  0 },
  ivec2{ -1, -1 }
};

const float neighbor_strengths[8]{
  1.0f,
  1.0f / sqrt(2.0f),
  1.0f,
  1.0f / sqrt(2.0f),
  1.0f,
  1.0f / sqrt(2.0f),
  1.0f,
  1.0f / sqrt(2.0f)
};

void FxRipple::init(const ivec2 &size) {
  m_tile_on_time.resize(m_tiles.size(), 0.0);
}

void FxRipple::update(double time, int frame_id) {
  for (const auto &tile : m_tiles) {
    if (tile.on && !tile.on_prev) {
      CI_LOG_D("start: " << tile.index);
      m_tile_on_time[tile.index] = time;
    }
  }
}

inline float circle(const vec2 &pos, const vec2 &center, float radius, float feather) {
  return smoothstep(radius, radius - feather, distance(pos, center));
}

void FxRipple::renderPixel(vec3 &color, const vec2 &pos, const ivec2 &coord, double time, int frame_id) {
  const float friction = 0.2f;
  const float gravity = 0.995f;
  const float transmission = 0.0275f;
  const float tightness = 0.075f;
  
  const vec3 &prev = getPrevColor(coord);
  vec3 vel = color - prev;
  vel *= 1.0f - friction;

  for (int i = 0; i < 8; ++i) {
    const auto crd = coord + neighbor_directions[i];
    const auto &c = getColor(crd);
    const auto &cp = getPrevColor(crd);
    vel += (c - color) * neighbor_strengths[i] * tightness;
    vel += (c - cp) * neighbor_strengths[i] * transmission;
  }
  
  color += vel;
  color *= gravity;

  for (const auto &tile : m_tiles) {
    const auto t = float(time - m_tile_on_time[tile.index]);
    auto tw2 = clamp(t * 0.1f, 0.0f, 1.0f);
    tw2 *= tw2;
    if (tile.on) {
      const auto tw = clamp(t * 0.5f, 0.0f, 1.0f);
      const auto r = 2.0f;
      const auto v = vec2(mix(10.0f, 3.0f, tw), 0.0f);
      for (int i = 0; i < 3; ++i) {
        const auto c = rotate(v, (two_pi<float>() / 3.0f) * float(i) - t * 10.0f) + tile.bounds.getCenter();
        auto col = vec3(Colorf(CM_HSV, glm::fract(float(i) * 0.025f + t * 0.1f), 1.0f, 1.0f));
        float a = tw * circle(pos, c, r, r * 0.5f);
        color = mix(color, col, a * 0.25f);
        color = mix(color, vec3(1.0f), a * (1.0f - (cos(20.0f * t * tw2) * 0.5f + 0.5f)));
      }
    }
    else if (tile.on_prev) {
      float a = circle(pos, tile.bounds.getCenter(), 10.0f, 10.0f);
      color += 4.0f * a * tw2 * vec3(Colorf(CM_HSV, glm::fract(t * 0.1f), 1.0f, 1.0f));
    }
  }
}
