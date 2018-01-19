#pragma once

#include "Fx.hpp"

class FxRipple : public Fx {
  glm::vec3 m_random_color;
  glm::vec2 m_random_pos;
  float m_random_radius;
  float m_opacity;

public:
  void init(const glm::ivec2 &size) override;
  void update(double time, int frame_id) override;
  void renderPixel(glm::vec3 &color, const glm::vec2 &pos, const glm::ivec2 &coord, double time, int frame_id) override;
};
