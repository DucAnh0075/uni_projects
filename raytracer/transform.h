/**
 * @file transform.h
 * @brief 3D transformation matrices and operations
 */

#pragma once
#include <Eigen/Dense>
#include <memory>

#include "math_utils.h"
#include "object.h"

using Vec3 = Eigen::Vector3f;
using Vec4 = Eigen::Vector4f;
using Matrix4 = Eigen::Matrix4f;

/** @brief Transform class using Eigen matrices
 *  @see docs/api_documentation.md for detailed documentation */
class Transform {
 private:
  Matrix4 matrix;
  Matrix4 inverse;

 public:
  Transform() {
    matrix = Matrix4::Identity();
    inverse = Matrix4::Identity();
  }

  Transform(const Matrix4& m) : matrix(m) { inverse = m.inverse(); }

  static Transform translation(float tx, float ty, float tz) {
    Matrix4 m = Matrix4::Identity();
    m(0, 3) = tx;
    m(1, 3) = ty;
    m(2, 3) = tz;
    return Transform(m);
  }

  static Transform rotationX(float degrees) {
    degrees = std::fmod(degrees, 360.0f);
    if (degrees < 0.0f) degrees += 360.0f;
    float radians = degrees * M_PI / 180.0f;

    Matrix4 m = Matrix4::Identity();
    float c = std::cos(radians);
    float s = std::sin(radians);
    m(1, 1) = c;
    m(1, 2) = -s;
    m(2, 1) = s;
    m(2, 2) = c;
    return Transform(m);
  }

  static Transform rotationXDegrees(float degrees) {
    return rotationX(degrees);
  }

  static Transform rotationY(float degrees) {
    degrees = std::fmod(degrees, 360.0f);
    if (degrees < 0.0f) degrees += 360.0f;
    float radians = degrees * M_PI / 180.0f;

    Matrix4 m = Matrix4::Identity();
    float c = std::cos(radians);
    float s = std::sin(radians);
    m(0, 0) = c;
    m(0, 2) = s;
    m(2, 0) = -s;
    m(2, 2) = c;
    return Transform(m);
  }

  static Transform rotationYDegrees(float degrees) {
    return rotationY(degrees);
  }

  static Transform rotationZ(float degrees) {
    degrees = std::fmod(degrees, 360.0f);
    if (degrees < 0.0f) degrees += 360.0f;
    float radians = degrees * M_PI / 180.0f;

    Matrix4 m = Matrix4::Identity();
    float c = std::cos(radians);
    float s = std::sin(radians);
    m(0, 0) = c;
    m(0, 1) = -s;
    m(1, 0) = s;
    m(1, 1) = c;
    return Transform(m);
  }

  static Transform rotationZDegrees(float degrees) {
    return rotationZ(degrees);
  }

  static Transform scaling(float sx, float sy, float sz) {
    Matrix4 m = Matrix4::Identity();
    m(0, 0) = sx;
    m(1, 1) = sy;
    m(2, 2) = sz;
    return Transform(m);
  }

  Transform operator*(const Transform& other) const {
    return Transform(matrix * other.matrix);
  }

  Vec3 transformPoint(const Vec3& p) const {
    Vec4 p4(p.x(), p.y(), p.z(), 1.0f);
    Vec4 result = matrix * p4;
    return RayTracer::vec4ToVec3(result);
  }

  Vec3 transformVector(const Vec3& v) const {
    Vec4 v4(v.x(), v.y(), v.z(), 0.0f);
    Vec4 result = matrix * v4;
    return RayTracer::vec4ToVec3(result);
  }

  Vec3 transformNormal(const Vec3& n) const {
    Eigen::Matrix3f normalMatrix = inverse.block<3, 3>(0, 0).transpose();
    return (normalMatrix * n).normalized();
  }

  Vec3 transformPointInverse(const Vec3& p) const {
    Vec4 p4(p.x(), p.y(), p.z(), 1.0f);
    Vec4 result = inverse * p4;
    return RayTracer::vec4ToVec3(result);
  }

  Vec3 transformVectorInverse(const Vec3& v) const {
    Vec4 v4(v.x(), v.y(), v.z(), 0.0f);
    Vec4 result = inverse * v4;
    return RayTracer::vec4ToVec3(result);
  }

  Line transformLine(const Line& line) const {
    Vec3 newOrigin = transformPointInverse(line.origin);
    Vec3 newDirection = transformVectorInverse(line.direction);
    return Line{newOrigin, newDirection.normalized()};
  }

  const Matrix4& getMatrix() const { return matrix; }
  const Matrix4& getInverse() const { return inverse; }
};