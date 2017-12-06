#include "cinder/Rand.h"

#include "Fx.hpp"

using namespace glm;
using namespace ci;

vec3 Fx::getColor(const ivec2 &coord) {
  const auto &s = m_texture->getSize();
  const auto c = glm::clamp(coord, ivec2(0), s - 1);
  return m_colors[c.y * s.x + c.x];
}

vec3 Fx::getPrevColor(const ivec2 &coord) {
  const auto &s = m_texture->getSize();
  const auto c = glm::clamp(coord, ivec2(0), s - 1);
  return m_colors_prev[c.y * s.x + c.x];
}

void Fx::init(const ivec2 &size) {
  m_colors.resize(size.x * size.y);
  m_colors_prev.resize(size.x * size.y);
  m_colors_next.resize(size.x * size.y);

  m_texture = gl::Texture::create(size.x,
                                  size.y,
                                  gl::Texture::Format()
                                      .target(GL_TEXTURE_RECTANGLE)
                                      .minFilter(GL_NEAREST)
                                      .magFilter(GL_NEAREST)
                                      .internalFormat(GL_RGB32F));
}

void Fx::update(double time, int frame_id) {}

void Fx::render(double time, int frame_id, const std::vector<vec2> &positions, const Rectf &bounds) {
  m_render_bounds = bounds;

  for (int i = 0; i < m_colors_next.size(); ++i) {
    renderPixel(m_colors_next[i],
                positions[i],
                ivec2(i % m_texture->getWidth(), i / m_texture->getHeight()),
                time,
                frame_id);
  }

  m_texture->update(Surface32f(&m_colors_next.front().x,
                               m_texture->getWidth(),
                               m_texture->getHeight(),
                               m_texture->getWidth() * sizeof(float) * 3,
                               SurfaceChannelOrder::RGB));

  m_colors_prev = m_colors;
  m_colors = m_colors_next;
}


////////////////////////////////////////////////////////////////
// Shaders

void FxTest::renderPixel(vec3 &color, const vec2 &pos, const ivec2 &coord, double time, int frame_id) {
  color = vec3((coord.x + frame_id / 10) % 2);
}



void FxPlasma::renderPixel(vec3 &color, const vec2 &pos, const ivec2 &coord, double time, int frame_id) {
  color = (vec3(sin((sin(pos.x) + cos(pos.y)) * 0.235f + time),
                cos((cos(-pos.x * 0.337f) + sin(pos.y * 0.263f)) * 0.821f + time * 0.721f),
                sin((sin(pos.x * 0.0831f) + cos(pos.t * 0.0731f)) * 1.2387f + time * 0.237f)) + 1.0f) * 0.5f;
}



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
  1.0f / glm::sqrt(2.0f),
  1.0f,
  1.0f / glm::sqrt(2.0f),
  1.0f,
  1.0f / glm::sqrt(2.0f),
  1.0f,
  1.0f / glm::sqrt(2.0f)
};

void FxRipple::update(double time, int frame_id) {
  if (frame_id % 30 == 0) {
    m_random_pos = vec2(randFloat(m_render_bounds.getX1(), m_render_bounds.getX2()),
                        randFloat(m_render_bounds.getY1(), m_render_bounds.getY2()));
    m_random_color = vec3(randFloat(), randFloat(), randFloat());
  }
}

void FxRipple::renderPixel(vec3 &color, const vec2 &pos, const ivec2 &coord, double time, int frame_id) {
  if (frame_id < 2) {
    color = vec3(0.1f);
  }
  else {
    const float friction = 0.25f;
    const float gravity = 0.9f;
    const float transmission = 0.025f;
    const float tightness = 0.075f;
    
    const vec3 &prev = getPrevColor(coord);
    vec3 vel = color - prev;
    vel *= 1.0f - friction;
    color += vel;

    // for (int i = 0; i < 8; ++i) {
    //   const auto crd = coord + neighbor_directions[i];
    //   const auto &c = getColor(crd);
    //   const auto &cp = getPrevColor(crd);
    //   vel += (c - color) * neighbor_strengths[i] * tightness;
    //   vel += (c - cp) * neighbor_strengths[i] * transmission;
    // }

    // color *= gravity;
    
    // color += vec3(0.01f);

    float brush = glm::smoothstep(1.0f, 0.0f, glm::distance(pos, m_random_pos) * 0.1f);
    //if (brush > 0.0f) {
      color += 0.001f * pos.x;// * vec3(brush);
      // printf("%f %f %f\n", brush, vel.x, color.r);
    //}
  }
}