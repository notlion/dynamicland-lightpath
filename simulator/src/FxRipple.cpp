#include "FxRipple.hpp"

#include "cinder/Rand.h"

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
}

void FxRipple::update(double time, int frame_id) {
}

void FxRipple::renderPixel(vec3 &color, const vec2 &pos, const ivec2 &coord, double time, int frame_id) {
  const auto &tile = getTile(pos);

  if (tile && tile->on) {
    color = vec3(Colorf(CM_HSV, vec3(randFloat(), 1.0f, 1.0f)));
    color *= 0.5f;
  }
  else {
    const float friction = 0.2f;
    const float gravity = 0.995f;
    const float transmission = 0.029f;
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
    
    // float brush = smoothstep(m_random_radius, 0.0f, distance(pos, m_random_pos));
    // color += m_random_color * (brush * m_opacity);
  }
}
