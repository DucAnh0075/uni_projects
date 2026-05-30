/**
 * @file math_utils.h
 * @brief Mathematical utilities and vector operations
 * @see docs/api_documentation.md#mathematical-utilities
 */

#pragma once
#include <Eigen/Dense>
#include <cmath>
#include <limits>
#include <optional>

#include "constants.h"

using Vec3 = Eigen::Vector3f;
using Vec4 = Eigen::Vector4f;

/** @brief Forward declaration of Color struct */
struct Color;

namespace RayTracer {

/** @brief Safe division with fallback value */
inline float safeDivide(float numerator, float denominator,
                        float fallback = 0.0f) {
  return (std::abs(denominator) > EPSILON) ? numerator / denominator : fallback;
}

/** @brief Safe division returning optional */
inline std::optional<float> safeDivideOptional(float numerator,
                                               float denominator) {
  if (std::abs(denominator) > EPSILON) {
    return numerator / denominator;
  }
  return std::nullopt;
}

/** @brief Safe square root that handles negative inputs */
inline float safeSqrt(float value) {
  return (value >= 0.0f) ? std::sqrt(value) : 0.0f;
}

/** @brief Safe normalization of a vector */
template <typename VectorType>
bool safeNormalize(VectorType& vector) {
  float length = vector.norm();
  if (length > EPSILON) {
    vector /= length;
    return true;
  }
  vector.setZero();
  return false;
}

/** @brief Safe normalization returning normalized copy */
template <typename VectorType>
VectorType safeNormalized(const VectorType& vector) {
  float length = vector.norm();
  if (length > EPSILON) {
    return vector / length;
  }
  return VectorType::Zero();
}

/** @brief Check if a floating point number is finite (not NaN or infinity) */
inline bool isFinite(float value) { return std::isfinite(value); }

/** @brief Check if a vector contains only finite values */
template <typename VectorType>
bool isFinite(const VectorType& vector) {
  for (int i = 0; i < vector.size(); ++i) {
    if (!std::isfinite(vector[i])) {
      return false;
    }
  }
  return true;
}

/** @brief Clamp a vector's components between min and max */
template <typename VectorType>
auto clampVector(const VectorType& vector, float minVal, float maxVal) ->
    typename VectorType::PlainObject {
  using PlainType = typename VectorType::PlainObject;
  PlainType result = vector;
  for (int i = 0; i < vector.size(); ++i) {
    result[i] = clamp(result[i], minVal, maxVal);
  }
  return result;
}

/** @brief Safe color normalization (0-255) -> (0-1) */
inline Eigen::Vector3f normalizeColor(const Eigen::Vector3f& color) {
  return color / 255.0f;
}

/** @brief Safe color denormalization (0-1) -> (0-255) */
inline Eigen::Vector3f denormalizeColor(const Eigen::Vector3f& color) {
  return clampVector(color * 255.0f, 0.0f, 255.0f);
}

/** @brief Solve quadratic equation ax² + bx + c = 0 */
inline std::optional<std::pair<float, float>> solveQuadratic(float a, float b,
                                                             float c) {
  if (std::abs(a) < EPSILON) {
    if (std::abs(b) < EPSILON) {
      return std::nullopt;
    }
    float x = -c / b;
    return std::make_pair(x, x);
  }

  float discriminant = b * b - 4 * a * c;
  if (discriminant < 0) {
    return std::nullopt;
  }

  float sqrtDisc = std::sqrt(discriminant);
  float x1 = (-b - sqrtDisc) / (2 * a);
  float x2 = (-b + sqrtDisc) / (2 * a);

  return std::make_pair(std::min(x1, x2), std::max(x1, x2));
}

/** @brief Safe ray-AABB intersection with proper handling of edge cases */
inline std::optional<std::pair<float, float>> rayAABBIntersection(
    const Eigen::Vector3f& rayOrigin, const Eigen::Vector3f& rayDir,
    const Eigen::Vector3f& boxMin, const Eigen::Vector3f& boxMax) {
  float tNear = -std::numeric_limits<float>::infinity();
  float tFar = std::numeric_limits<float>::infinity();

  for (int i = 0; i < 3; ++i) {
    if (std::abs(rayDir[i]) < EPSILON) {
      if (rayOrigin[i] < boxMin[i] || rayOrigin[i] > boxMax[i]) {
        return std::nullopt;
      }
    } else {
      float t1 = (boxMin[i] - rayOrigin[i]) / rayDir[i];
      float t2 = (boxMax[i] - rayOrigin[i]) / rayDir[i];

      if (t1 > t2) std::swap(t1, t2);

      tNear = std::max(tNear, t1);
      tFar = std::min(tFar, t2);

      if (tNear > tFar) {
        return std::nullopt;
      }
    }
  }

  return std::make_pair(tNear, tFar);
}

/** @brief Convert Color struct to normalized Vec3 */
inline Eigen::Vector3f colorToVec3(const Color& color) {
  return Eigen::Vector3f(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f);
}

/** @brief Convert normalized Vec3 to Color struct */
inline Color vec3ToColor(const Eigen::Vector3f& vec) {
  Eigen::Vector3f clamped = clampVector(vec * 255.0f, 0.0f, 255.0f);
  return Color{static_cast<int>(clamped.x()), static_cast<int>(clamped.y()),
               static_cast<int>(clamped.z())};
}

/** @brief Convert Vec3 to Eigen::Vector3f directly (identity function since
 * Vec3 is already Eigen::Vector3f) */
inline Eigen::Vector3f vec3ToVector3f(const Vec3& vec) {
  return vec;  // Vec3 is already Eigen::Vector3f, so just return it
}

/** @brief Convert Vec4 to Vec3 by extracting x, y, z components */
inline Vec3 vec4ToVec3(const Vec4& vec) {
  return Vec3(vec.x(), vec.y(), vec.z());
}

}  // namespace RayTracer