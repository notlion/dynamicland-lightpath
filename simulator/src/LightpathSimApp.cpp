#include "cinder/Camera.h"
#include "cinder/ImageIo.h"
#include "cinder/Log.h"
#include "cinder/Surface.h"
#include "cinder/Rand.h"
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "OpcClient.h"
#include "Fx.hpp"
#include "FxRipple.hpp"

#include <fcntl.h>
#include <iostream>

// #define CONFIG_NEOPIXEL_88

using namespace ci;
using namespace ci::app;

#if defined(CONFIG_NEOPIXEL_88)
const ivec2 led_pixel_grid_size{ 8, 8 };
const ivec2 led_panel_grid_size{ 1, 1 };
#else
const ivec2 led_pixel_grid_size{ 10, 12 };
const ivec2 led_panel_grid_size{ 4, 2 };
#endif

const ivec2 led_texture_size{ led_pixel_grid_size * led_panel_grid_size };
const int led_pixel_count = led_texture_size.x * led_texture_size.y;
const int led_vertex_count = led_pixel_count * 6; // Two triangles each

const vec2 led_pixel_padding{ 1.0f };
const vec2 led_panel_padding{ 2.0f };
const vec2 led_panel_size = led_pixel_padding * vec2(led_pixel_grid_size);
const vec2 led_pixel_spacing = led_pixel_padding;
const vec2 led_panel_spacing = led_panel_size + led_panel_padding;

const std::string led_shader_vs_src = R"(
#version 150 core

uniform mat4 ciModelViewProjection;
uniform float u_splat_scale;
uniform vec2 u_led_texture_size;

in vec2 ciPosition;
in vec2 ciTexCoord0;
in vec2 ciTexCoord1;

out vec2 v_texcoord_0;
out vec2 v_texcoord_1;

void main() {
  v_texcoord_0 = ciTexCoord0 * u_led_texture_size + 0.5;
  v_texcoord_1 = ciTexCoord1;

  vec2 pos = ciPosition + u_splat_scale * (ciTexCoord1 * 2.0 - 1.0);

  gl_Position = ciModelViewProjection * vec4(pos, 0.0, 1.0);
}
)";

const std::string led_shader_fs_src = R"(
#version 150 core

uniform sampler2DRect u_led_tex;
uniform sampler2D u_splat_tex;
uniform float u_splat_opacity;

in vec2 v_texcoord_0;
in vec2 v_texcoord_1;

out vec4 frag_color;

void main() {
  frag_color = texture(u_led_tex, v_texcoord_0) * texture(u_splat_tex, v_texcoord_1);
  frag_color.a *= u_splat_opacity;
}
)";


class LightpathSimApp : public App {
public:
  static void prepareSettings(Settings *settings);

  void setup() override;
  void resize() override;
  void update() override;
  void draw() override;
  void mouseDown(MouseEvent event) override;
  void mouseUp(MouseEvent event) override;
  void keyDown(KeyEvent event) override;

#if !defined(CONFIG_HEADLESS)
  gl::VboMeshRef m_led_mesh;
  gl::BatchRef m_led_batch;
  gl::TextureRef m_splat_texture;
#endif

  Rectf m_led_bounds;

  std::vector<vec2> m_led_positions;
  std::vector<Tile> m_tiles;

  CameraPersp m_camera;
 
  Fx *m_fx = nullptr;
  FxRipple m_ripple;
  FxPlasma m_plasma;

  kp::opc::ClientRef m_opc_client;

  int m_frame_id = 0;
  vec3 m_mouse_world_pos;
};

void LightpathSimApp::setup() {
  m_led_positions.resize(led_pixel_count);

  std::vector<vec2> positions;
  std::vector<vec2> texcoords;
  std::vector<vec2> mask_texcoords;
  positions.reserve(led_vertex_count);
  texcoords.reserve(led_vertex_count);
  mask_texcoords.reserve(led_vertex_count);

  m_tiles.reserve(led_panel_grid_size.x * led_panel_grid_size.y);

  for (int py = 0; py < led_panel_grid_size.y; ++py) {
    for (int px = 0; px < led_panel_grid_size.x; ++px) {
      m_tiles.emplace_back();
      
      auto &tile = m_tiles.back();
      tile.index = m_tiles.size() - 1;
      tile.bounds = Rectf(vec2(px, py) * led_panel_spacing, vec2(px, py) * led_panel_spacing + led_panel_size);
      tile.bounds -= vec2(led_pixel_spacing * 0.5f);

      for (int y = 0; y < led_pixel_grid_size.y; ++y) {
        for (int x = 0; x < led_pixel_grid_size.x; ++x) {
          mask_texcoords.emplace_back(0.0f, 0.0f);
          mask_texcoords.emplace_back(1.0f, 1.0f);
          mask_texcoords.emplace_back(0.0f, 1.0f);
          mask_texcoords.emplace_back(0.0f, 0.0f);
          mask_texcoords.emplace_back(1.0f, 0.0f);
          mask_texcoords.emplace_back(1.0f, 1.0f);

          const auto position = vec2(px, py) * led_panel_spacing + vec2(x, y) * led_pixel_spacing;
          const auto texcoord = (vec2(px, py) * vec2(led_pixel_grid_size) + vec2(x, y)) / vec2(led_texture_size);

          for (int i = 0; i < 6; ++i) {
            positions.push_back(position);// + Rand::randVec2() * 0.1f);
            texcoords.push_back(texcoord);
          }

          m_led_positions[(py * led_pixel_grid_size.y + y) * led_texture_size.x + px * led_pixel_grid_size.x + x] = position;
        }
      }
    }
  }

  m_led_bounds = Rectf(positions);

  const auto center = m_led_bounds.getCenter();
  for (auto &p : positions) p -= center;
  for (auto &p : m_led_positions) p -= center;
  for (auto &t : m_tiles) t.bounds -= center;

  m_led_bounds -= center;

#if !defined(CONFIG_HEADLESS)
  m_splat_texture = gl::Texture::create(loadImage(loadResource("led_splat_0.png")),
                                        gl::Texture::Format().mipmap());

  m_led_mesh = gl::VboMesh::create(led_vertex_count,
                                   GL_TRIANGLES,
                                   { gl::VboMesh::Layout()
                                         .usage(GL_STATIC_DRAW)
                                         .attrib(geom::Attrib::POSITION, 2)
                                         .attrib(geom::Attrib::TEX_COORD_0, 2)
                                         .attrib(geom::Attrib::TEX_COORD_1, 2) });
  auto led_prog = gl::GlslProg::create(led_shader_vs_src, led_shader_fs_src);
  led_prog->uniform("u_led_tex", 0);
  led_prog->uniform("u_splat_tex", 1);
  led_prog->uniform("u_led_texture_size", vec2(led_texture_size));
  m_led_batch = gl::Batch::create(m_led_mesh, led_prog);

  m_led_mesh->bufferAttrib(geom::Attrib::POSITION, positions);
  m_led_mesh->bufferAttrib(geom::Attrib::TEX_COORD_0, texcoords);
  m_led_mesh->bufferAttrib(geom::Attrib::TEX_COORD_1, mask_texcoords);
#endif

  m_ripple.initPrivate(led_texture_size, m_tiles);
  m_ripple.init(led_texture_size);

  m_plasma.initPrivate(led_texture_size, m_tiles);
  m_plasma.init(led_texture_size);

  m_fx = &m_ripple;

  m_opc_client = kp::opc::Client::create("localhost", 7890, true, false);
}

void LightpathSimApp::resize() {}

void LightpathSimApp::update() {
  const auto time = getElapsedSeconds();

  const auto bounds_size = m_led_bounds.getSize();
  const auto distance = glm::max(bounds_size.x, bounds_size.y) * 2.0f;

  m_camera.setAspectRatio(getWindowAspectRatio());
  m_camera.lookAt(glm::normalize(vec3(0.0f, 1.0f, -10.0f)) * distance, vec3(0.0f), vec3(0.0f, 0.0f, -1.0f));

  for (auto &tile : m_tiles) {
    tile.on_prev = tile.on;
  }

  const auto inputAvailable = [] {
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
    return (FD_ISSET(0, &fds));
  };

  if (inputAvailable()) {
    std::string line;
    std::getline(std::cin, line);
    if (!line.empty()) {
      for (auto it = line.begin(); it != line.end(); ++it) {
        const auto c = *it;
        if (c == '0' || c == '1') {
          m_tiles[std::distance(line.begin(), it)].on = c == '1';
        }
      }
    }
  }

  if (m_fx) {
    m_fx->updatePrivate(m_tiles);
    m_fx->update(time, m_frame_id);
    m_fx->render(time, m_frame_id, m_led_positions, m_led_bounds);
  }

  // Write to Fadecandy
  {
#if defined(CONFIG_NEOPIXEL_88)
    int i = 0;
    for (int y = 7; y >= 0; --y) {
      for (int x = 7; x >= 0; --x) {
        const auto &c = m_fx->getColor(ivec2(x, y));
        m_opc_client->setLED(i++, Colorf(c.r, c.g, c.b));
      }
    }
#else
    ivec2 p, c;
    for (p.y = 0; p.y < led_panel_grid_size.y; ++p.y) {
      for (p.x = 0; p.x < led_panel_grid_size.x; ++p.x) {
        for (c.y = 0; c.y < led_pixel_grid_size.y; ++c.y) {
          for (c.x = 0; c.x < led_pixel_grid_size.x; ++c.x) {
            const auto &color = m_fx->getColor(c);
            const auto cc = p * led_pixel_grid_size + c;
            const auto i = cc.x * led_texture_size.y + cc.y;
            m_opc_client->setLED(i, Colorf(color.r, color.g, color.b));
          }
        }
      }
    }
#endif

    m_opc_client->update();
  }

  m_frame_id++;
}

void LightpathSimApp::draw() {
#if !defined(CONFIG_HEADLESS)
  gl::clear();
  gl::setMatrices(m_camera);

  // gl::rotate(glm::cos(getElapsedSeconds() / glm::pi<float>()) * 0.05f);

  gl::disableDepthRead();
  gl::disableDepthWrite();

  gl::ScopedBlendAdditive scopedBlend;
  gl::ScopedTextureBind scopedLedTex(m_fx->getTexture(), 0);
  gl::ScopedTextureBind scopedSplatTex(m_splat_texture, 1);

  m_led_batch->getGlslProg()->uniform("u_splat_scale", 2.0f);
  m_led_batch->getGlslProg()->uniform("u_splat_opacity", 0.333f);
  m_led_batch->draw();

  int i = 0;
  for (const auto &tile : m_tiles) {
    gl::drawStrokedRect(tile.bounds);
    // gl::drawStringCentered(std::to_string(i++), tile.bounds.getCenter());
  }

  gl::drawStrokedCircle(m_mouse_world_pos, 1.0f, 16);
#endif
}

void LightpathSimApp::mouseDown(MouseEvent event) {
  const auto window_size = getWindowSize();
  const auto mouse_pos = getMousePos();
  const auto ray = m_camera.generateRay(vec2(mouse_pos), window_size);
  
  float t;
  if (ray.calcPlaneIntersection(vec3(0.0f), vec3(0.0f, 0.0f, 1.0f), &t)) {
    m_mouse_world_pos = ray.calcPosition(t);
    const auto pos = vec2(m_mouse_world_pos);
    for (auto &tile : m_tiles) {
      if (tile.bounds.contains(pos)) {
        tile.on = !tile.on;
      }
    }
  }
}

void LightpathSimApp::mouseUp(MouseEvent event) {
}

void LightpathSimApp::keyDown(KeyEvent event) {
  switch (event.getChar()) {
    case '1': m_fx = &m_ripple; break;
    case '2': m_fx = &m_plasma; break;
  }
}


void LightpathSimApp::prepareSettings(Settings *settings) {
  settings->setHighDensityDisplayEnabled();
  settings->setPowerManagementEnabled(false);
  settings->setWindowSize(1280, 720);
}


CINDER_APP(LightpathSimApp, RendererGl(), LightpathSimApp::prepareSettings)
