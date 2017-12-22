#include "cinder/Rect.h"
#include "cinder/Serial.h"
#include "cinder/Vector.h"
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

  const std::vector<glm::vec3> &getColors() const {
    return m_colors;
  }

  glm::vec3 getColor(const glm::ivec2 &coord);
  glm::vec3 getPrevColor(const glm::ivec2 &coord);

  void initPrivate(const glm::ivec2 &size);
  void render(double time, int frame_id, const std::vector<glm::vec2> &positions, const ci::Rectf &bounds);

  virtual void init(const glm::ivec2 &size);
  virtual void update(double time, int frame_id);
  virtual void renderPixel(glm::vec3 &color, const glm::vec2 &pos, const glm::ivec2 &coord, double time, int frame_id) = 0;
  virtual void pluck(const glm::vec2 &pos);
};


class FxTest : public Fx {
public:
  void renderPixel(glm::vec3 &color, const glm::vec2 &pos, const glm::ivec2 &coord, double time, int frame_id) override;
};

class FxPlasma : public Fx {
public:
  void renderPixel(glm::vec3 &color, const glm::vec2 &pos, const glm::ivec2 &coord, double time, int frame_id) override;
};

class FxRipple : public Fx {
  glm::vec3 m_random_color;
  glm::vec2 m_random_pos;
  float m_random_radius;
  float m_opacity;

public:
  bool auto_pluck = true;

  void update(double time, int frame_id) override;
  void pluck(const glm::vec2 &pos) override;
  void renderPixel(glm::vec3 &color, const glm::vec2 &pos, const glm::ivec2 &coord, double time, int frame_id) override;
};

class FxSensorTest : public Fx {
  ci::SerialRef m_sensor;

  int m_sensor_min = std::numeric_limits<int>::max();
  int m_sensor_max = 0;
  int m_sensor_value;

public:
  void init(const glm::ivec2 &size) override;
  void update(double time, int frame_id) override;
  void renderPixel(glm::vec3 &color, const glm::vec2 &pos, const glm::ivec2 &coord, double time, int frame_id) override;
};
