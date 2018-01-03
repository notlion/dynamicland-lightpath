#include "cinder/Rand.h"
#include "cinder/Log.h"

#include "Fx.hpp"

using namespace glm;
using namespace ci;

vec3 Fx::getColor(const ivec2 &coord) {
  const auto c = clamp(coord, ivec2(0), m_texture_size - 1);
  return m_colors[c.y * m_texture_size.x + c.x];
}

vec3 Fx::getPrevColor(const ivec2 &coord) {
  const auto c = clamp(coord, ivec2(0), m_texture_size - 1);
  return m_colors_prev[c.y * m_texture_size.x + c.x];
}

void Fx::initPrivate(const ivec2 &size) {
  m_colors.resize(size.x * size.y, vec3(0.0f));
  m_colors_prev.resize(size.x * size.y, vec3(0.0f));
  m_colors_next.resize(size.x * size.y, vec3(0.0f));

  m_texture_size = size;

#if !defined(CONFIG_HEADLESS)
  m_texture = gl::Texture::create(m_texture_size.x,
                                  m_texture_size.y,
                                  gl::Texture::Format()
                                      .target(GL_TEXTURE_RECTANGLE)
                                      .minFilter(GL_NEAREST)
                                      .magFilter(GL_NEAREST)
                                      .internalFormat(GL_RGB32F));
#endif
}

void Fx::render(double time, int frame_id, const std::vector<vec2> &positions, const Rectf &bounds) {
  m_render_bounds = bounds;

  const auto tex_width = m_texture_size.y;

  for (int i = 0; i < m_colors_next.size(); ++i) {
    renderPixel(m_colors_next[i], positions[i], ivec2(i % tex_width, i / tex_width), time, frame_id);
  }

#if !defined(CONFIG_HEADLESS)
  m_texture->update(Surface32f(&m_colors_next.front().x,
                               m_texture->getWidth(),
                               m_texture->getHeight(),
                               m_texture->getWidth() * sizeof(float) * 3,
                               SurfaceChannelOrder::RGB));
#endif

  std::copy(m_colors.begin(), m_colors.end(), m_colors_prev.begin());
  std::copy(m_colors_next.begin(), m_colors_next.end(), m_colors.begin());
}

void Fx::init(const ivec2 &size) {}
void Fx::update(double time, int frame_id) {}
void Fx::pluck(const vec2 &pos) {}


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
  1.0f / sqrt(2.0f),
  1.0f,
  1.0f / sqrt(2.0f),
  1.0f,
  1.0f / sqrt(2.0f),
  1.0f,
  1.0f / sqrt(2.0f)
};

void FxRipple::update(double time, int frame_id) {
  if (auto_pluck) {
    if (frame_id % 40 == 0) {
      pluck(vec2(randFloat(m_render_bounds.getX1(), m_render_bounds.getX2()),
                 randFloat(m_render_bounds.getY1(), m_render_bounds.getY2())));
    }
    if (frame_id % 40 == 20) {
      m_opacity = 0.0f;
    }
  }
}

void FxRipple::pluck(const vec2 &pos) {
  m_random_pos = pos;
  m_random_color = vec3(Colorf(CM_HSV, vec3(randFloat(), 1.0f, 1.0f)));
  m_random_radius = randFloat(2.0f, 3.0f);
  m_opacity = 0.03f;
}

void FxRipple::renderPixel(vec3 &color, const vec2 &pos, const ivec2 &coord, double time, int frame_id) {
  const float friction = 0.2f;
  const float gravity = 0.99f;
  const float transmission = 0.025f;
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
  
  float brush = smoothstep(m_random_radius, 0.0f, distance(pos, m_random_pos));
  color += m_random_color * (brush * m_opacity);
}



void FxSensorTest::init(const ivec2 &size) {
  auto device = Serial::findDeviceByNameContains("cu.usbserial");
  if (!device.getName().empty()) {
    m_sensor = Serial::create(device, 9600);
  }
}

void FxSensorTest::update(double time, int frame_id) {
  if (m_sensor) {
    std::array<uint8_t, 256> bytes;
    const auto len = m_sensor->readAvailableBytes(bytes.data(), bytes.size());

    for (std::size_t i = 0; i < len; ++i) {
      m_sensor_min = min(int(bytes[i]), m_sensor_min);
      m_sensor_max = max(int(bytes[i]), m_sensor_max);
      m_sensor_value = int(bytes[i]);
    }
  }
}

void FxSensorTest::renderPixel(vec3 &color, const vec2 &pos, const ivec2 &coord, double time, int frame_id) {
  if (coord.y < 4) {
    color = vec3(float(int(m_sensor_value) - m_sensor_min) / float(m_sensor_max - m_sensor_min), 0.0f, 0.0f);
  }
  else if (coord.y < 8) {
    color = vec3(0.0f, float(m_sensor_value) / 255.0f, 0.0f);
  }
  else {
    color = vec3(0.0f, 0.0f, float(m_sensor_value) / 127.0f);
  }
}
