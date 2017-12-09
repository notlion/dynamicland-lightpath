#include "cinder/Vector.h"
#include "cinder/Rect.h"
#include "cinder/gl/Texture.h"

class Fx {
protected:
  std::vector<glm::vec3> m_colors;
  std::vector<glm::vec3> m_colors_prev;
  std::vector<glm::vec3> m_colors_next;

  ci::Rectf m_render_bounds;

  ci::gl::TextureRef m_texture;

public:
  virtual ~Fx() = default;

  ci::gl::TextureRef &getTexture() {
    return m_texture;
  }

  glm::vec3 getColor(const glm::ivec2 &coord);
  glm::vec3 getPrevColor(const glm::ivec2 &coord);

  void render(double time, int frame_id, const std::vector<glm::vec2> &positions, const ci::Rectf &bounds);

  virtual void init(const glm::ivec2 &size);
  virtual void update(double time, int frame_id);
  virtual void renderPixel(glm::vec3 &color, const glm::vec2 &pos, const glm::ivec2 &coord, double time, int frame_id) = 0;
};

class FxTest : public Fx {
public:
  void renderPixel(glm::vec3 &color, const glm::vec2 &pos, const glm::ivec2 &coord, double time, int frame_id) override;
};

class FxRipple : public Fx {
  glm::vec3 m_random_color;
  glm::vec2 m_random_pos;
  float m_random_radius;

public:
  void update(double time, int frame_id) override;
  void renderPixel(glm::vec3 &color, const glm::vec2 &pos, const glm::ivec2 &coord, double time, int frame_id) override;
};

class FxPlasma : public Fx {
public:
  void renderPixel(glm::vec3 &color, const glm::vec2 &pos, const glm::ivec2 &coord, double time, int frame_id) override;
};
