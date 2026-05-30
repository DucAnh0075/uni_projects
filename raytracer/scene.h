#pragma once
#include <Eigen/Dense>

class Screen {
 public:
  Eigen::Vector4f origin;
  float width, height;
  int dpi_x, dpi_y;
  float pixel_size_x, pixel_size_y;

  Screen(const Eigen::Vector4f& origin_, float width_, float height_,
         int dpi_x_, int dpi_y_)
      : origin(origin_),
        width(width_),
        height(height_),
        dpi_x(dpi_x_),
        dpi_y(dpi_y_) {
    pixel_size_x = width / (width * dpi_x);
    pixel_size_y = height / (height * dpi_y);
  }
};
#include <limits>
#include <memory>
#include <vector>

#include "light.h"
#include "object.h"
#include "object_tree.h"

class Scene {
 public:
  std::vector<std::shared_ptr<Object>> objects;
  std::vector<Light> lights;
  Eigen::Vector3f backgroundColor;

  Scene() : backgroundColor(0.5f, 0.7f, 1.0f) {}
  Scene(const Eigen::Vector3f& bgColor) : backgroundColor(bgColor) {}

  void addObject(const std::shared_ptr<Object>& obj) { objects.push_back(obj); }
  void addLight(const Light& L) { lights.push_back(L); }

  bool trace(const Eigen::Vector3f& rayOrig, const Eigen::Vector3f& rayDir,
             PixelInfo& hitInfo, float& tMin) const {
    std::shared_ptr<Object> hitObject;
    return trace(rayOrig, rayDir, hitInfo, tMin, hitObject);
  }

  bool trace(const Eigen::Vector3f& rayOrig, const Eigen::Vector3f& rayDir,
             PixelInfo& hitInfo, float& tMin,
             std::shared_ptr<Object>& hitObject) const {
    bool hit = false;
    tMin = std::numeric_limits<float>::max();
    for (const auto& obj : objects) {
      float t;
      if (auto* sphere = dynamic_cast<Sphere*>(obj.get())) {
        if (sphere->intersect(rayOrig, rayDir, t) && t < tMin) {
          tMin = t;
          // Seting intersection point for spheres
          Eigen::Vector3f hitPoint = rayOrig + rayDir * t;
          hitInfo.color = sphere->getColor();
          hitInfo.pos = Position{hitPoint.x(), hitPoint.y(), hitPoint.z()};
          hitInfo.distance = t;
          hitObject = obj;
          hit = true;
        }
      }
      // Add other object types here
      else if (auto* cube = dynamic_cast<Cube*>(obj.get())) {
        // Ray-cube intersection
        Line line{rayOrig, rayDir};
        PixelInfo info = cube->intersectWithLine(line);
        if (info.distance > 0 && info.distance < tMin) {
          tMin = info.distance;
          hitInfo = info; 
          hitObject = obj;
          hit = true;
        }
      } else if (auto* plane = dynamic_cast<HalfSpace*>(obj.get())) {
        // Ray-plane (half-space) intersection
        Line line{rayOrig, rayDir};
        PixelInfo info = plane->intersectWithLine(line);
        if (info.distance > 0 && info.distance < tMin) {
          tMin = info.distance;
          hitInfo = info;
          hitObject = obj;
          hit = true;
        }
      } else if (auto* csg = dynamic_cast<CSGObject*>(obj.get())) {
        Line line{rayOrig, rayDir};
        PixelInfo info = csg->intersectWithLine(line);
        if (info.distance > 0 && info.distance < tMin) {
          tMin = info.distance;
          hitInfo = info;
          hitObject = obj;
          hit = true;
        }
      } else if (auto* tree = dynamic_cast<ObjectTree*>(obj.get())) {
        Line line{rayOrig, rayDir};
        PixelInfo info = tree->intersectWithLine(line);
        if (info.distance > 0 && info.distance < tMin) {
          tMin = info.distance;
          hitInfo = info;
          hitObject = obj;
          hit = true;
        }
      }
    }
    return hit;
  }

  bool isOccluded(const Eigen::Vector3f& origin, const Eigen::Vector3f& dir,
                  float maxDist, float eps = 1e-3f) const {
    for (const auto& obj : objects) {
      float t = 0.0f;
      if (auto* sphere = dynamic_cast<Sphere*>(obj.get())) {
        if (sphere->intersect(origin, dir, t) && t > eps &&
            (std::isinf(maxDist) || t < maxDist - eps))
          return true;
      } else if (auto* cube = dynamic_cast<Cube*>(obj.get())) {
        Line l{origin, dir};
        PixelInfo info = cube->intersectWithLine(l);
        if (info.distance > eps &&
            (std::isinf(maxDist) || info.distance < maxDist - eps))
          return true;
      } else if (auto* plane = dynamic_cast<HalfSpace*>(obj.get())) {
        (void)plane;
        continue;
      } else if (auto* csg = dynamic_cast<CSGObject*>(obj.get())) {
        Line l{origin, dir};
        PixelInfo info = csg->intersectWithLine(l);
        if (info.distance > eps &&
            (std::isinf(maxDist) || info.distance < maxDist - eps))
          return true;
      } else if (auto* tree = dynamic_cast<ObjectTree*>(obj.get())) {
        Line l{origin, dir};
        PixelInfo info = tree->intersectWithLine(l);
        if (info.distance > eps &&
            (std::isinf(maxDist) || info.distance < maxDist - eps))
          return true;
      }
    }
    return false;
  }
};