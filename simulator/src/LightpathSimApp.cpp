#include "cinder/Camera.h"
#include "cinder/ImageIo.h"
#include "cinder/Log.h"
#include "cinder/Surface.h"
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;

const ivec2 led_pixel_grid_size{ 10, 14 };
const ivec2 led_panel_grid_size{ 4, 2 };
const ivec2 led_texture_size{ led_pixel_grid_size * led_panel_grid_size };
const int led_pixel_count = led_texture_size.x * led_texture_size.y;
const int led_vertex_count = led_pixel_count * 6; // Two triangles each

const vec2 led_pixel_padding{ 1.0f };
const vec2 led_panel_padding{ 2.0f };
const vec2 led_pixel_spacing = led_pixel_padding;
const vec2 led_panel_spacing = led_pixel_spacing * vec2(led_pixel_grid_size) + led_panel_padding;

const std::string led_shader_vs_src = R"(
#version 150 core

uniform mat4 ciModelViewProjection;
uniform float u_splat_scale;

in vec2 ciPosition;
in vec2 ciTexCoord0;
in vec2 ciTexCoord1;

out vec2 v_texcoord_0;
out vec2 v_texcoord_1;

void main() {
  v_texcoord_0 = ciTexCoord0;
  v_texcoord_1 = ciTexCoord1;

  vec2 pos = ciPosition + u_splat_scale * (ciTexCoord1 * 2.0 - 1.0);

  gl_Position = ciModelViewProjection * vec4(pos, 0.0, 1.0);
}
)";

const std::string led_shader_fs_src = R"(
#version 150 core

uniform sampler2D u_led_tex;
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

  void processPixel(vec3 &color, const vec2 &pos, const ivec2 &coord);

  gl::VboMeshRef m_led_mesh;
  gl::BatchRef m_led_batch;

  gl::TextureRef m_splat_texture;
  gl::TextureRef m_led_texture;

  Rectf m_led_bounds;

  std::vector<vec2> m_led_positions;
  std::vector<vec3> m_led_colors;
  std::vector<vec3> m_led_colors_prev;

  Surface32fRef m_led_color_surf;

  CameraPersp m_camera;
};

void LightpathSimApp::setup() {
  m_led_texture = gl::Texture::create(
      led_texture_size.x,
      led_texture_size.y,
      gl::Texture::Format().minFilter(GL_NEAREST).magFilter(GL_NEAREST).internalFormat(GL_RGB32F));
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
  m_led_batch = gl::Batch::create(m_led_mesh, led_prog);

  m_led_positions.resize(led_pixel_count);
  m_led_colors.resize(led_pixel_count);

  std::vector<vec2> positions;
  std::vector<vec2> texcoords;
  std::vector<vec2> mask_texcoords;
  positions.reserve(led_vertex_count);
  texcoords.reserve(led_vertex_count);
  mask_texcoords.reserve(led_vertex_count);

  for (int py = 0; py < led_panel_grid_size.y; ++py) {
    for (int px = 0; px < led_panel_grid_size.x; ++px) {
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
            positions.push_back(position);
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

  m_led_bounds -= center;

  m_led_mesh->bufferAttrib(geom::Attrib::POSITION, positions);
  m_led_mesh->bufferAttrib(geom::Attrib::TEX_COORD_0, texcoords);
  m_led_mesh->bufferAttrib(geom::Attrib::TEX_COORD_1, mask_texcoords);

  m_led_color_surf = Surface32f::create(&m_led_colors.front().x,
                                        led_texture_size.x,
                                        led_texture_size.y,
                                        led_texture_size.x * sizeof(float) * 3,
                                        SurfaceChannelOrder::RGB);
}

void LightpathSimApp::resize() {}

void LightpathSimApp::update() {
  for (int i = 0; i < led_pixel_count; ++i) {
    processPixel(m_led_colors[i], m_led_positions[i], ivec2(i % led_texture_size.x, i / led_texture_size.x));
  }
  m_led_texture->update(*m_led_color_surf);
}

void LightpathSimApp::processPixel(vec3 &color, const vec2 &pos, const ivec2 &coord) {
  const auto time = getElapsedSeconds() * 2.0f;

  using namespace glm;

  color = (vec3(sin((sin(pos.x) + cos(pos.y)) * 0.235f + time),
                cos((cos(-pos.x * 0.337f) + sin(pos.y * 0.263f)) * 0.821f + time * 0.721f),
                sin((sin(pos.x * 0.0831f) + cos(pos.t * 0.0731f)) * 1.2387f + time * 0.237f)) + 1.0f) * 0.5f;
}

void LightpathSimApp::draw() {
  const auto bounds_size = m_led_bounds.getSize();
  const auto distance = glm::max(bounds_size.x, bounds_size.y) * 1.4f;

  m_camera.setAspectRatio(getWindowAspectRatio());
  m_camera.lookAt(glm::normalize(vec3(0.0f, 1.0f, 10.0f)) * distance, vec3(0.0f), vec3(0.0f, 0.0f, 1.0f));
  
  gl::clear();
  gl::setMatrices(m_camera);

  gl::rotate(glm::cos(getElapsedSeconds() / glm::pi<float>()) * 0.05f);

  gl::disableDepthRead();
  gl::disableDepthWrite();

  gl::ScopedBlendAdditive scopedBlend;
  gl::ScopedTextureBind scopedLedTex(m_led_texture, 0);
  gl::ScopedTextureBind scopedSplatTex(m_splat_texture, 1);

  m_led_batch->getGlslProg()->uniform("u_splat_scale", 2.0f);
  m_led_batch->getGlslProg()->uniform("u_splat_opacity", 0.333f);
  m_led_batch->draw();
}

void LightpathSimApp::prepareSettings(Settings *settings) {
  settings->setHighDensityDisplayEnabled(true);
}

CINDER_APP(LightpathSimApp, RendererGl(RendererGl::Options().msaa(8)), LightpathSimApp::prepareSettings)
