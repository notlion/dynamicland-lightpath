#pragma once

#include "Fx.hpp"

class FxRipple : public Fx {
  std::vector<double> m_tile_on_time;

public:
  void init(const glm::ivec2 &size) override;
  void update(double time, int frame_id) override;
  void renderPixel(glm::vec3 &color, const glm::vec2 &pos, const glm::ivec2 &coord, double time, int frame_id) override;
};
