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
  m_tile_hue_offset.resize(m_tiles.size(), 0.0f);
}

void FxRipple::update(double time, int frame_id) {
  bool any_tiles_on = false;

  for (const auto &tile : m_tiles) {
    any_tiles_on |= tile.on;
    if (tile.on && !tile.on_prev) {
      m_tile_on_time[tile.index] = time;
      m_tile_hue_offset[tile.index] = randFloat();
    }
  }

  if (!any_tiles_on && randInt(50) == 0) {
    m_droplet_center.x = randFloat(m_render_bounds.getX1(), m_render_bounds.getX2());
    m_droplet_center.y = randFloat(m_render_bounds.getY1(), m_render_bounds.getY2());
    m_droplet_color = vec3(Colorf(CM_HSV, randFloat(), 1.0f, 1.0f));
    float r = randFloat();
    r *= r;
    m_droplet_opacity = mix(0.5f, 1.0f, r);
    m_droplet_radius = mix(1.0f, 3.0f, m_droplet_opacity);
  }
  else {
    m_droplet_opacity = 0.0f;
  }
}

inline float circle(const vec2 &pos, const vec2 &center, float radius, float feather) {
  return smoothstep(radius, radius - feather, distance(pos, center));
}

void FxRipple::renderPixel(vec3 &color, const vec2 &pos, const ivec2 &coord, double time, int frame_id) {
  const float friction = 0.19f;
  const float gravity = 0.99f;
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

    if (tile.on) {
      const auto pulse = glm::smoothstep(1.0f, 0.0f, glm::fract(t * 0.5f));
      const auto col = vec3(Colorf(CM_HSV, glm::fract(m_tile_hue_offset[tile.index] + t * 0.1f), 1.0f, 1.0f));
      const auto r = mix(1.0f, 4.0f, pulse);
      const auto a = circle(pos, tile.bounds.getCenter(), r, r);
      color = mix(color, col, a * mix(0.1f, 1.0f, pulse));
    }
    else if (tile.on_prev) {
    }
  }

  if (m_droplet_opacity > 0.0f) {
    const auto a = circle(pos, m_droplet_center, m_droplet_radius, m_droplet_radius);
    color += m_droplet_color * (m_droplet_opacity * a);
  }
}
