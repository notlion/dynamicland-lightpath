#include "cinder/Rand.h"
#include "cinder/Log.h"
#include "cinder/ImageIo.h"
#include "cinder/app/App.h"

#include "Fx.hpp"

using namespace glm;
using namespace ci;

vec3 Fx::getColor(const ivec2 &coord) const {
  const auto c = clamp(coord, ivec2(0), m_texture_size - 1);
  const auto i = c.y * m_texture_size.x + c.x;
  return m_colors[i];
}

vec3 Fx::getPrevColor(const ivec2 &coord) const {
  const auto c = clamp(coord, ivec2(0), m_texture_size - 1);
  const auto i = c.y * m_texture_size.x + c.x;
  return m_colors_prev[i];
}

const Tile *Fx::getTile(const glm::vec2 &position) const {
  for (const auto &tile : m_tiles) {
    if (tile.bounds.contains(position)) return &tile;
  }
  return nullptr;
}

void Fx::initPrivate(const ivec2 &size, const std::vector<Tile> &tiles) {
  m_colors.resize(size.x * size.y, vec3(0.0f));
  m_colors_prev.resize(size.x * size.y, vec3(0.0f));
  m_colors_next.resize(size.x * size.y, vec3(0.0f));

  m_tiles = tiles;
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

void Fx::updatePrivate(const std::vector<Tile> &tiles) {
  m_tiles = tiles;
}

void Fx::render(double time, int frame_id, const std::vector<vec2> &positions, const Rectf &bounds) {
  m_render_bounds = bounds;

  const auto tex_width = m_texture_size.x;

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


////////////////////////////////////////////////////////////////
// Shaders

void FxTestPattern::init(const glm::ivec2 &size) {
  m_test_image = Surface::create(loadImage(app::loadResource("led_test_pattern.png")));
}

void FxTestPattern::renderPixel(vec3 &color, const vec2 &pos, const ivec2 &coord, double time, int frame_id) {
  const auto &c = m_test_image->getPixel(ivec2(coord.x, m_texture_size.y - coord.y - 1));
  color = vec3(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f);
  color *= mix(0.5f, 1.0f, sin(pos.x + pos.y + time) * 0.5f + 0.5f);
}



void FxPlasma::renderPixel(vec3 &color, const vec2 &pos, const ivec2 &coord, double time, int frame_id) {
  color = (vec3(sin((sin(pos.x) + cos(pos.y)) * 0.235f + time),
                cos((cos(-pos.x * 0.337f) + sin(pos.y * 0.263f)) * 0.821f + time * 0.721f),
                sin((sin(pos.x * 0.0831f) + cos(pos.t * 0.0731f)) * 1.2387f + time * 0.237f)) + 1.0f) * 0.5f;
  color *= 0.5f;
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
