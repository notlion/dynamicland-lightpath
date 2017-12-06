#include "cinder/Vector.h"
#include "cinder/Rect.h"
#include "cinder/gl/Texture.h"

class Fx {
protected:
  std::vector<glm::vec3> m_colors;
  std::vector<glm::vec3> m_colors_prev;

  ci::Rectf m_render_bounds;

  ci::gl::TextureRef m_texture;

public:
  virtual ~Fx() = default;

  ci::gl::TextureRef &getTexture() {
    return m_texture;
  }

  glm::vec3 getColor(const glm::ivec2 &coord);
  glm::vec3 getPrevColor(const glm::ivec2 &coord);

  void render(double time, uint32_t frame_id, const std::vector<glm::vec2> &positions, const ci::Rectf &bounds);

  virtual void init(const glm::ivec2 &size);
  virtual void update(double time, uint32_t frame_id);
  virtual void renderPixel(glm::vec3 &color, const glm::vec2 &pos, const glm::ivec2 &coord, double time, uint32_t frame_id) = 0;
};

class FxRipple : public Fx {
public:
  void renderPixel(glm::vec3 &color, const glm::vec2 &pos, const glm::ivec2 &coord, double time, uint32_t frame_id) override;
};

class FxPlasma : public Fx {
public:
  void renderPixel(glm::vec3 &color, const glm::vec2 &pos, const glm::ivec2 &coord, double time, uint32_t frame_id) override;
};
