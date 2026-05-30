/**
 * @file object.cpp
 * @brief Implementation of geometric objects and ray intersection algorithms
 * @see docs/api_documentation.md#geometric-objects
 */

#include "object.h"

#include <algorithm>
#include <cmath>
#include <iostream>

#include "constants.h"
#include "math_utils.h"

Sphere::Sphere(const Eigen::Vector3f& c, float r, const Eigen::Vector3f& col) {
  pos = Position{c.x(), c.y(), c.z()};
  color = Color{static_cast<int>(col.x()), static_cast<int>(col.y()),
                static_cast<int>(col.z())};
  radius = r;
  material = Material();
}

Sphere::Sphere(const Eigen::Vector3f& c, float r, const Eigen::Vector3f& col,
               const Material& mat) {
  pos = Position{c.x(), c.y(), c.z()};
  color = Color{static_cast<int>(col.x()), static_cast<int>(col.y()),
                static_cast<int>(col.z())};
  radius = r;
  material = mat;
}

bool Sphere::intersect(const Eigen::Vector3f& rayOrig,
                       const Eigen::Vector3f& rayDir, float& t) const {
  using namespace RayTracer;

  Eigen::Vector3f center(pos.x, pos.y, pos.z);
  Eigen::Vector3f oc = rayOrig - center;

  float a = rayDir.dot(rayDir);
  float b = 2.0f * oc.dot(rayDir);
  float c = oc.dot(oc) - radius * radius;

  auto solutions = solveQuadratic(a, b, c);
  if (!solutions) {
    return false;
  }

  float t1 = solutions->first;
  float t2 = solutions->second;

  if (t1 > MIN_INTERSECTION_DISTANCE) {
    t = t1;
    return true;
  } else if (t2 > MIN_INTERSECTION_DISTANCE) {
    t = t2;
    return true;
  }

  return false;
}

PixelInfo Sphere::intersectWithLine(const Line&) {
  return PixelInfo{color, pos, 0.0};
}

bool Sphere::contains(const Eigen::Vector3f& P) const {
  Eigen::Vector3f C(pos.x, pos.y, pos.z);
  return (P - C).squaredNorm() <= radius * radius + 1e-6f;
}

bool Sphere::rayInterval(const Line& line, float& tEnter, float& tExit) const {
  Eigen::Vector3f C(pos.x, pos.y, pos.z);
  Eigen::Vector3f oc = line.origin - C;
  float b = oc.dot(line.direction);
  float c = oc.dot(oc) - radius * radius;
  float disc = b * b - c;
  if (disc < 0) return false;
  float s = std::sqrt(std::max(0.0f, disc));
  float t1 = -b - s;
  float t2 = -b + s;
  if (t1 > t2) std::swap(t1, t2);
  tEnter = t1;
  tExit = t2;
  return tExit >= 0.0f;
}

PixelInfo Cube::intersectWithLine(const Line& line) {
  using namespace RayTracer;

  Eigen::Vector3f minCorner(pos.x - size / 2.0f, pos.y - size / 2.0f,
                            pos.z - size / 2.0f);
  Eigen::Vector3f maxCorner(pos.x + size / 2.0f, pos.y + size / 2.0f,
                            pos.z + size / 2.0f);
  auto intersection =
      rayAABBIntersection(line.origin, line.direction, minCorner, maxCorner);

  if (!intersection) {
    return PixelInfo{Color{0, 0, 0}, Position{0, 0, 0}, 0.0};
  }

  float tNear = intersection->first;
  float tFar = intersection->second;

  float t;
  if (tNear > MIN_INTERSECTION_DISTANCE) {
    t = tNear;
  } else if (tFar > MIN_INTERSECTION_DISTANCE) {
    t = tFar;
  } else {
    return PixelInfo{Color{0, 0, 0}, Position{0, 0, 0}, 0.0};
  }

  Eigen::Vector3f hit = line.origin + t * line.direction;

  if (!isFinite(hit)) {
    return PixelInfo{Color{0, 0, 0}, Position{0, 0, 0}, 0.0};
  }

  return PixelInfo{color, Position{hit.x(), hit.y(), hit.z()}, t};
}

bool Cube::contains(const Eigen::Vector3f& P) const {
  float half = static_cast<float>(size * 0.5);
  return (P.x() >= pos.x - half && P.x() <= pos.x + half &&
          P.y() >= pos.y - half && P.y() <= pos.y + half &&
          P.z() >= pos.z - half && P.z() <= pos.z + half);
}

bool Cube::rayInterval(const Line& line, float& tEnter, float& tExit) const {
  using namespace RayTracer;

  float half = static_cast<float>(size * 0.5f);
  Eigen::Vector3f minCorner(pos.x - half, pos.y - half, pos.z - half);
  Eigen::Vector3f maxCorner(pos.x + half, pos.y + half, pos.z + half);

  auto intersection =
      rayAABBIntersection(line.origin, line.direction, minCorner, maxCorner);

  if (!intersection) {
    return false;
  }

  tEnter = intersection->first;
  tExit = intersection->second;
  return tExit >= 0.0f;
}

PixelInfo HalfSpace::intersectWithLine(const Line& line) {
  using namespace RayTracer;

  float denom = normal.dot(line.direction);
  if (isZero(denom)) {
    return PixelInfo{Color{0, 0, 0}, Position{0, 0, 0}, 0.0};
  }

  float t = (normal.dot(normal * offset - line.origin)) / denom;
  if (t > MIN_INTERSECTION_DISTANCE) {
    Eigen::Vector3f hit = line.origin + t * line.direction;
    if (!isFinite(hit)) {
      return PixelInfo{Color{0, 0, 0}, Position{0, 0, 0}, 0.0};
    }

    return PixelInfo{color, Position{hit.x(), hit.y(), hit.z()}, t};
  }
  return PixelInfo{Color{0, 0, 0}, Position{0, 0, 0}, 0.0};
}

bool HalfSpace::contains(const Eigen::Vector3f& P) const {
  return normal.dot(P) >= offset - 1e-6f;
}

bool HalfSpace::rayInterval(const Line& line, float& tEnter,
                            float& tExit) const {
  float denom = normal.dot(line.direction);
  float numer = offset - normal.dot(line.origin);
  if (std::abs(denom) < 1e-6f) {
    if (numer <= 0) {
      tEnter = -std::numeric_limits<float>::infinity();
      tExit = std::numeric_limits<float>::infinity();
      return true;
    }
    return false;
  }
  float t = numer / denom;
  if (denom < 0) {
    tEnter = t;
    tExit = std::numeric_limits<float>::infinity();
  } else {
    tEnter = -std::numeric_limits<float>::infinity();
    tExit = t;
  }
  return true;
}

Color colorMul(const Color& c, float f) {
  return Color{static_cast<int>(std::clamp(c.r * f, 0.0f, 255.0f)),
               static_cast<int>(std::clamp(c.g * f, 0.0f, 255.0f)),
               static_cast<int>(std::clamp(c.b * f, 0.0f, 255.0f))};
}

// ---- CSG implementation ----
static bool combineIntervals(CSGOp op, float aEnter, float aExit, bool aValid,
                             float bEnter, float bExit, bool bValid,
                             float& outEnter, float& outExit) {
  if (op == CSGOp::Union) {
    if (!aValid && !bValid) return false;
    if (!aValid) {
      outEnter = bEnter;
      outExit = bExit;
      return true;
    }
    if (!bValid) {
      outEnter = aEnter;
      outExit = aExit;
      return true;
    }
    outEnter = std::min(aEnter, bEnter);
    outExit = std::max(aExit, bExit);
    return outExit >= std::max(0.0f, outEnter);
  } else if (op == CSGOp::Intersection) {
    if (!aValid || !bValid) return false;
    outEnter = std::max(aEnter, bEnter);
    outExit = std::min(aExit, bExit);
    return outExit >= std::max(0.0f, outEnter);
  } else {  // Difference: A \ B
    if (!aValid) return false;
    if (!bValid) {
      outEnter = aEnter;
      outExit = aExit;
      return outExit >= std::max(0.0f, outEnter);
    }
    if (aExit <= bEnter || bExit <= aEnter) {
      outEnter = aEnter;
      outExit = aExit;
      return outExit >= std::max(0.0f, outEnter);
    }
    if (aEnter < bEnter) {
      outEnter = aEnter;
      outExit = bEnter;
      return outExit >= std::max(0.0f, outEnter);
    }
    return false;
  }
}

bool CSGObject::contains(const Eigen::Vector3f& P) const {
  bool inA = A->contains(P);
  bool inB = B->contains(P);
  switch (op) {
    case CSGOp::Union:
      return inA || inB;
    case CSGOp::Intersection:
      return inA && inB;
    case CSGOp::Difference:
      return inA && !inB;
  }
  return false;
}

bool CSGObject::rayInterval(const Line& line, float& tEnter,
                            float& tExit) const {
  float a0, a1, b0, b1;
  bool aV = A->rayInterval(line, a0, a1);
  bool bV = B->rayInterval(line, b0, b1);
  return combineIntervals(op, a0, a1, aV, b0, b1, bV, tEnter, tExit);
}

PixelInfo CSGObject::intersectWithLine(const Line& line) {
  float t0, t1;
  if (!rayInterval(line, t0, t1))
    return PixelInfo{Color{0, 0, 0}, Position{0, 0, 0}, 0.0};
  float t = (t0 > 1e-5f) ? t0 : t1;
  if (t <= 1e-5f) return PixelInfo{Color{0, 0, 0}, Position{0, 0, 0}, 0.0};

  Eigen::Vector3f hit = line.origin + t * line.direction;
  lastHitComponent = A;
  return PixelInfo{A->getColor(), Position{hit.x(), hit.y(), hit.z()}, t};
}
