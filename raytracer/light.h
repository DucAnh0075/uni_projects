#pragma once
#include <Eigen/Dense>

#include "object.h"

class Light {
 public:
  enum class Type { Point, Directional };

 private:
  Type type_;
  Eigen::Vector3f position_;
  Eigen::Vector3f direction_;
  Color color_{255, 255, 255};
  float intensity_ = 1.0f;
  bool castsShadows_ = true;

  float attenC_ = 1.0f;  // Constant attenuation
  float attenL_ = 0.0f;  // Linear attenuation
  float attenQ_ = 0.0f;  // Quadratic attenuation

  Light(Type t, const Eigen::Vector3f& pos, const Eigen::Vector3f& dir,
        const Color& col, float intensity, bool castsShadows, float attenC,
        float attenL, float attenQ)
      : type_(t),
        position_(pos),
        direction_(dir),
        color_(col),
        intensity_(intensity),
        castsShadows_(castsShadows),
        attenC_(attenC),
        attenL_(attenL),
        attenQ_(attenQ) {
    if (type_ == Type::Directional) {
      if (direction_.norm() > 0) direction_.normalize();
    }
  }

 public:
  static Light createPointLight(const Eigen::Vector3f& position,
                                const Color& color = {255, 255, 255},
                                float intensity = 1.0f,
                                bool castsShadows = true, float attenC = 1.0f,
                                float attenL = 0.0f, float attenQ = 0.0f) {
    return Light(Type::Point, position, Eigen::Vector3f::Zero(), color,
                 intensity, castsShadows, attenC, attenL, attenQ);
  }

  static Light createDirectionalLight(const Eigen::Vector3f& direction,
                                      const Color& color = {255, 255, 255},
                                      float intensity = 1.0f,
                                      bool castsShadows = true) {
    return Light(Type::Directional, Eigen::Vector3f::Zero(), direction, color,
                 intensity, castsShadows, 1.0f, 0.0f, 0.0f);
  }

  Type getType() const { return type_; }
  const Eigen::Vector3f& getPosition() const { return position_; }
  const Eigen::Vector3f& getDirection() const { return direction_; }
  const Color& getColor() const { return color_; }
  float getIntensity() const { return intensity_; }
  bool getCastsShadows() const { return castsShadows_; }

  Eigen::Vector3f getLightVector(const Eigen::Vector3f& point) const {
    if (type_ == Type::Point) {
      return position_ - point;
    } else {
      return -direction_;
    }
  }

  float getAttenuatedIntensity(const Eigen::Vector3f& point) const {
    if (type_ == Type::Point) {
      float distance = (position_ - point).norm();
      return intensity_ /
             (attenC_ + attenL_ * distance + attenQ_ * distance * distance);
    } else {
      return intensity_;
    }
  }
};