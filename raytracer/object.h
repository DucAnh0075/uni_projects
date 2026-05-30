#pragma once
#include <Eigen/Dense>
#include <memory>

struct Color {
  int r, g, b;
};

struct Position {
  float x, y, z;
};

struct Line {
  Eigen::Vector3f origin;
  Eigen::Vector3f direction;
};

struct PixelInfo {
  Color color;
  Position pos;
  double distance;
};

struct Material {
  Eigen::Vector3f Ka;  // Ambient
  Eigen::Vector3f Kd;  // Diffuse
  Eigen::Vector3f Ks;  // Specular
  float shininess;
  Eigen::Vector3f Kr;  // Reflection
  Eigen::Vector3f Kt;  // Transmission
  float refractive_index;
  Material()
      : Ka(0.3f, 0.3f, 0.3f),
        Kd(1.0f, 1.0f, 1.0f),
        Ks(0.5f, 0.5f, 0.5f),
        shininess(32.0f),
        Kr(0.0f, 0.0f, 0.0f),
        Kt(0.0f, 0.0f, 0.0f),
        refractive_index(1.0f) {}

  Material(const Eigen::Vector3f& ka, const Eigen::Vector3f& kd,
           const Eigen::Vector3f& ks, float shine,
           const Eigen::Vector3f& kr = Eigen::Vector3f::Zero(),
           const Eigen::Vector3f& kt = Eigen::Vector3f::Zero(), float n = 1.0f)
      : Ka(ka),
        Kd(kd),
        Ks(ks),
        shininess(shine),
        Kr(kr),
        Kt(kt),
        refractive_index(n) {}
};

class Object {
 protected:
  Position pos;
  Color color;
  Material material;

 public:
  virtual ~Object() = default;

  const Position& getPos() const { return pos; }
  void setPosition(const Position& newPos) { pos = newPos; }
  const Color& getColor() const { return color; }
  virtual const Material& getMaterial() const { return material; }

  virtual PixelInfo intersectWithLine(const Line&) = 0;
  virtual bool contains(const Eigen::Vector3f& P) const = 0;
  virtual bool rayInterval(const Line& line, float& tEnter,
                           float& tExit) const = 0;
};

/**
 * @brief A sphere object
 */
class Sphere : public Object {
 public:
  float radius;

  /**
   * @brief Create a sphere
   * @param c Center position
   * @param r Radius
   * @param col Color
   */
  Sphere(const Eigen::Vector3f& c, float r, const Eigen::Vector3f& col);
  Sphere(const Eigen::Vector3f& c, float r, const Eigen::Vector3f& col,
         const Material& mat);

  bool intersect(const Eigen::Vector3f& rayOrig, const Eigen::Vector3f& rayDir,
                 float& t) const;
  PixelInfo intersectWithLine(const Line& line) override;
  bool contains(const Eigen::Vector3f& P) const override;
  bool rayInterval(const Line& line, float& tEnter,
                   float& tExit) const override;
};

class Cube : public Object {
 protected:
  double size;

 public:
  Cube(const Eigen::Vector3f& c, double s, const Eigen::Vector3f& col) {
    pos = Position{c.x(), c.y(), c.z()};
    color = Color{static_cast<int>(col.x()), static_cast<int>(col.y()),
                  static_cast<int>(col.z())};
    size = s;
    material = Material();
  }

  Cube(const Eigen::Vector3f& c, double s, const Eigen::Vector3f& col,
       const Material& mat) {
    pos = Position{c.x(), c.y(), c.z()};
    color = Color{static_cast<int>(col.x()), static_cast<int>(col.y()),
                  static_cast<int>(col.z())};
    size = s;
    material = mat;
  }
  PixelInfo intersectWithLine(const Line& line) override;
  bool contains(const Eigen::Vector3f& P) const override;
  bool rayInterval(const Line& line, float& tEnter,
                   float& tExit) const override;
};

class HalfSpace : public Object {
 protected:
  Eigen::Vector3f normal;
  float offset;

 public:
  HalfSpace(const Eigen::Vector3f& n, float r, const Eigen::Vector3f& col) {
    normal = n.normalized();
    offset = r;
    pos =
        Position{normal.x() * offset, normal.y() * offset, normal.z() * offset};
    color = Color{static_cast<int>(col.x()), static_cast<int>(col.y()),
                  static_cast<int>(col.z())};
    material = Material();
  }

  HalfSpace(const Eigen::Vector3f& n, float r, const Eigen::Vector3f& col,
            const Material& mat) {
    normal = n.normalized();
    offset = r;
    pos =
        Position{normal.x() * offset, normal.y() * offset, normal.z() * offset};
    color = Color{static_cast<int>(col.x()), static_cast<int>(col.y()),
                  static_cast<int>(col.z())};
    material = mat;
  }
  PixelInfo intersectWithLine(const Line& line) override;
  const Eigen::Vector3f& getNormal() const { return normal; }

  bool contains(const Eigen::Vector3f& P) const override;
  bool rayInterval(const Line& line, float& tEnter,
                   float& tExit) const override;
};

Color colorMul(const Color& c, float f);

enum class CSGOp { Union, Intersection, Difference };  // Difference = A \ B

class CSGObject : public Object {
  std::shared_ptr<Object> A;
  std::shared_ptr<Object> B;
  CSGOp op;
  mutable std::shared_ptr<Object> lastHitComponent;

 public:
  CSGObject(const std::shared_ptr<Object>& a, const std::shared_ptr<Object>& b,
            CSGOp operation, const Eigen::Vector3f& col)
      : A(a), B(b), op(operation), lastHitComponent(nullptr) {
    color = Color{static_cast<int>(col.x()), static_cast<int>(col.y()),
                  static_cast<int>(col.z())};
    pos = a ? a->getPos() : Position{0, 0, 0};
  }
  const Material& getMaterial() const override {
    if (lastHitComponent) {
      return lastHitComponent->getMaterial();
    }
    return material;
  }

  void setMaterial(const Material& mat) { material = mat; }

  PixelInfo intersectWithLine(const Line& line) override;
  bool contains(const Eigen::Vector3f& P) const override;
  bool rayInterval(const Line& line, float& tEnter,
                   float& tExit) const override;
};