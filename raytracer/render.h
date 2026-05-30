/**
 * @file render.h
 * @brief Main rendering engine and ray tracing algorithms
 */

#ifndef RENDER_H
#define RENDER_H

#include <Eigen/Dense>
#include <chrono>
#include <cmath>
#include <opencv2/opencv.hpp>

#include "constants.h"
#include "light.h"
#include "math_utils.h"
#include "performance_utils.h"
#include "scene.h"
#include "transform.h"

using Vec3 = Eigen::Vector3f;
using Vec4 = Eigen::Vector4f;

/** @brief Logarithmic light combination operator: E1 ⊕ E2 = 1 - (1-E1)(1-E2) */
inline Vec3 combineLights(const Vec3& E1, const Vec3& E2) {
  return Vec3::Ones() - (Vec3::Ones() - E1).cwiseProduct(Vec3::Ones() - E2);
}

/** @brief Reflection vector calculation: R = I - 2(I·N)N */
inline Vec3 computeReflection(const Vec3& incident, const Vec3& normal) {
  return incident - 2.0f * incident.dot(normal) * normal;
}

/** @brief Refraction vector calculation using Snell's law */
inline Vec3 computeRefraction(const Vec3& incident, const Vec3& normal,
                              float n1, float n2) {
  float ratio = n1 / n2;
  float cosI = -normal.dot(incident);
  float sinT2 = ratio * ratio * (1.0f - cosI * cosI);
  if (sinT2 > 1.0f) return Vec3::Zero();  // Total internal reflection
  float cosT = std::sqrt(1.0f - sinT2);
  return ratio * incident + (ratio * cosI - cosT) * normal;
}

/** @brief Get surface normal for any object type */
inline Vec3 getSurfaceNormal(const std::shared_ptr<Object>& obj,
                             const Vec3& hitPoint) {
  if (auto* sphere = dynamic_cast<const Sphere*>(obj.get())) {
    Vec3 center(sphere->getPos().x, sphere->getPos().y, sphere->getPos().z);
    return (hitPoint - center).normalized();
  } else if (auto* plane = dynamic_cast<const HalfSpace*>(obj.get())) {
    return plane->getNormal();
  } else if (auto* cube = dynamic_cast<const Cube*>(obj.get())) {
    Vec3 center(cube->getPos().x, cube->getPos().y, cube->getPos().z);
    Vec3 local = hitPoint - center;
    float ax = std::abs(local.x()), ay = std::abs(local.y()),
          az = std::abs(local.z());
    float maxAxis = std::max({ax, ay, az});
    if (ax == maxAxis) return Vec3((local.x() > 0) ? 1 : -1, 0, 0);
    if (ay == maxAxis) return Vec3(0, (local.y() > 0) ? 1 : -1, 0);
    return Vec3(0, 0, (local.z() > 0) ? 1 : -1);
  }
  return Vec3(0, 1, 0);
}

class Renderer {
 private:
  static Vec3 traceRay(const Scene& scene, const Vec3& rayOrigin,
                       const Vec3& rayDirection, int depth,
                       RayTracer::PixelInfoPool& pixelPool,
                       RayTracer::MaterialCache& materialCache,
                       RayTracer::IntersectionCache& intersectionCache,
                       int maxDepth = RayTracer::DEFAULT_RAY_DEPTH) {
    if (depth >= maxDepth) {
      return scene.backgroundColor;
    }

    float t;
    PixelInfo& hitInfo = pixelPool.acquire();
    std::shared_ptr<Object> hitObject;

    if (!scene.trace(rayOrigin, rayDirection, hitInfo, t, hitObject) ||
        !hitObject) {
      pixelPool.release(hitInfo);
      return scene.backgroundColor;
    }

    Vec3 hitPoint(hitInfo.pos.x, hitInfo.pos.y, hitInfo.pos.z);
    Vec3 normal = getSurfaceNormal(hitObject, hitPoint);
    Vec3 viewDir = (rayOrigin - hitPoint).normalized();
    const Material& mat = hitObject->getMaterial();

    if (normal.dot(viewDir) < 0) {
      normal = -normal;
    }

    Vec3 objectColor = RayTracer::colorToVec3(hitInfo.color);
    Vec3 ambient = objectColor * RayTracer::AMBIENT_INTENSITY;

    Vec3 diffuse = Vec3::Zero();
    Vec3 specular = Vec3::Zero();

    for (const auto& light : scene.lights) {
      Vec3 lightVec = light.getLightVector(hitPoint);
      float distToLight = (light.getType() == Light::Type::Point)
                              ? lightVec.norm()
                              : std::numeric_limits<float>::infinity();

      if (distToLight <= RayTracer::MIN_INTERSECTION_DISTANCE) continue;

      Vec3 lightDir = lightVec.normalized();
      float ndotl = std::max(0.0f, normal.dot(lightDir));
      if (ndotl <= 0.0f) continue;

      Vec3 shadowOrig = hitPoint + RayTracer::SHADOW_BIAS * normal;
      bool blocked = scene.isOccluded(
          shadowOrig, lightDir,
          std::isfinite(distToLight) ? distToLight
                                     : std::numeric_limits<float>::infinity(),
          RayTracer::SHADOW_BIAS);
      if (blocked) continue;

      float lightFactor = light.getAttenuatedIntensity(hitPoint);

      Vec3 diffuseContrib = objectColor * ndotl * lightFactor;
      diffuse = combineLights(diffuse, diffuseContrib);
      Vec3 reflectDir = computeReflection(-lightDir, normal);
      float RdotV = std::max(0.0f, reflectDir.dot(viewDir));
      float Ks = 0.25f;
      float alpha = 32.0f;
      Vec3 specularContrib =
          Vec3(1.0f, 1.0f, 1.0f) * Ks * std::pow(RdotV, alpha) * lightFactor;
      specular = combineLights(specular, specularContrib);
    }

    Vec3 reflection = Vec3::Zero();
    Vec3 refraction = Vec3::Zero();

    if (depth < maxDepth) {
      if (mat.Kr.maxCoeff() > 0.01f) {
        Vec3 reflectDir = computeReflection(-viewDir, normal);
        Vec3 reflectedColor =
            traceRay(scene, hitPoint + normal * 0.001f, reflectDir, depth + 1,
                     pixelPool, materialCache, intersectionCache, maxDepth);
        reflection = Vec3(mat.Kr.x() * reflectedColor.x(),
                          mat.Kr.y() * reflectedColor.y(),
                          mat.Kr.z() * reflectedColor.z());
      }
      if (mat.Kt.maxCoeff() > 0.01f) {
        Vec3 refractDir =
            computeRefraction(-viewDir, normal, 1.0f, mat.refractive_index);
        if (refractDir != Vec3::Zero()) {
          Vec3 refractedColor =
              traceRay(scene, hitPoint - normal * 0.001f, refractDir, depth + 1,
                       pixelPool, materialCache, intersectionCache, maxDepth);
          refraction = Vec3(mat.Kt.x() * refractedColor.x(),
                            mat.Kt.y() * refractedColor.y(),
                            mat.Kt.z() * refractedColor.z());
        }
      }
    }
    Vec3 finalColor = ambient + diffuse + specular + reflection + refraction;

    pixelPool.release(hitInfo);

    return finalColor.cwiseMin(1.0f).cwiseMax(0.0f);
  }

  static Vec3 traceRay(const Scene& scene, const Vec3& rayOrigin,
                       const Vec3& rayDirection, int depth,
                       int maxDepth = RayTracer::DEFAULT_RAY_DEPTH) {
    RayTracer::PixelInfoPool tempPool(10);
    RayTracer::MaterialCache tempCache;
    RayTracer::IntersectionCache tempIntersectionCache;
    return traceRay(scene, rayOrigin, rayDirection, depth, tempPool, tempCache,
                    tempIntersectionCache, maxDepth);
  }

 public:
  static void renderScene(cv::Mat& image, const Scene& scene,
                          const Screen& screen, const Vec4& observer,
                          const Vec3& camera, bool showRays, bool showLightRays,
                          bool showPlaneFlow, int planeFlowStride,
                          bool showPositionMarkers) {
    int width = screen.width * screen.dpi_x;
    int height = screen.height * screen.dpi_y;

    RayTracer::RayGrid rayGrid;
    RayTracer::PixelInfoPool pixelPool(RayTracer::OBJECT_POOL_SIZE);
    RayTracer::MaterialCache materialCache;
    RayTracer::PixelBatchProcessor batchProcessor;
    RayTracer::IntersectionCache intersectionCache;

    auto startTime = std::chrono::high_resolution_clock::now();

    rayGrid.initialize(screen, observer, width, height);

    Vec3 planeNormal(0, 1, 0);
    float planeOffset = 0.0f;
    for (const auto& obj : scene.objects) {
      if (auto plane = dynamic_cast<HalfSpace*>(obj.get())) {
        planeNormal = plane->getNormal();
        Line testLine{Vec3(0, 0, 0), planeNormal};
        PixelInfo info = plane->intersectWithLine(testLine);
        planeOffset = info.distance;
        break;
      }
    }

    bool hasPointLight = false;
    Vec3 pointLightPos(0, 0, 0);
    bool hasDirLight = false;
    Vec3 dirLightDir(0, -1, 0);
    for (const auto& L : scene.lights) {
      if (!hasPointLight && L.getType() == Light::Type::Point) {
        pointLightPos = L.getPosition();
        hasPointLight = true;
      }
      if (!hasDirLight && L.getType() == Light::Type::Directional) {
        dirLightDir = L.getDirection();
        hasDirLight = true;
      }
    }
    int lx = 0, ly = 0;
    if (hasPointLight) {
      lx = (int)((pointLightPos.x() - screen.origin.x()) / (screen.width) *
                 width);
      ly = (int)((pointLightPos.y() - screen.origin.y()) / (screen.height) *
                 height);
    }

    auto projectToScreen = [&](const Vec3& P, int& outX, int& outY) -> bool {
      Vec3 O = RayTracer::vec4ToVec3(observer);
      float denom = (P.z() - O.z());
      if (std::abs(denom) < 1e-6f) return false;
      float t_proj = (screen.origin.z() - O.z()) / denom;
      if (t_proj <= 0.0f) return false;
      Vec3 S = O + t_proj * (P - O);
      int sx = (int)((S.x() - screen.origin.x()) / screen.pixel_size_x);
      int sy = (int)((S.y() - screen.origin.y()) / screen.pixel_size_y);
      if (sx < 0 || sx >= width || sy < 0 || sy >= height) return false;
      outX = sx;
      outY = height - 1 - sy;
      return true;
    };

    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        Vec3 rayOrigin = rayGrid.getRayOrigin(x, y);
        Vec3 rayDirection = rayGrid.getRayDirection(x, y);

        Vec3 finalColor = traceRay(scene, rayOrigin, rayDirection, 0, pixelPool,
                                   materialCache, intersectionCache,
                                   RayTracer::MAX_RAY_DEPTH);
        finalColor = RayTracer::denormalizeColor(
            RayTracer::clampVector(finalColor, 0.0f, 1.0f));

        int row = height - 1 - y;
        image.at<cv::Vec3b>(row, x) =
            cv::Vec3b((uchar)finalColor.z(), (uchar)finalColor.y(),
                      (uchar)finalColor.x());

        if (showRays && x % RayTracer::RAY_VISUALIZATION_STRIDE == 0 &&
            y % RayTracer::RAY_VISUALIZATION_STRIDE == 0) {
          cv::Point startPt((int)(camera.x() * screen.dpi_x + width / 2),
                            (int)(camera.y() * screen.dpi_y + height / 2));
          cv::Point endPt(x, height - 1 - y);
          cv::line(image, startPt, endPt, cv::Scalar(0, 0, 255), 1);
        }
        if (showLightRays && hasPointLight &&
            x % RayTracer::RAY_VISUALIZATION_STRIDE == 0 &&
            y % RayTracer::RAY_VISUALIZATION_STRIDE == 0) {
          cv::line(image, cv::Point(lx, ly), cv::Point(x, height - 1 - y),
                   cv::Scalar(255, 255, 255), 1);
        }
      }
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    auto renderTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime);

#ifdef DEBUG_BUILD
    std::cout << "[PERFORMANCE] Rendering completed in " << renderTime.count()
              << "ms" << std::endl;
    std::cout << "[PERFORMANCE] Pixels rendered: " << (width * height)
              << std::endl;
    std::cout << "[PERFORMANCE] Pixels per second: "
              << (width * height * 1000.0 / renderTime.count()) << std::endl;
#endif

    if (hasPointLight) {
      int fly = height - 1 - ly;
      cv::circle(image, cv::Point(lx, fly), 16, cv::Scalar(0, 255, 255), -1);
      cv::circle(image, cv::Point(lx, fly), 18, cv::Scalar(0, 0, 0), 2);
      cv::line(image, cv::Point(lx - 12, fly), cv::Point(lx + 12, fly),
               cv::Scalar(0, 0, 0), 2);
      cv::line(image, cv::Point(lx, fly - 12), cv::Point(lx, fly + 12),
               cv::Scalar(0, 0, 0), 2);
    }

    if (showPlaneFlow) {
      Vec3 n = planeNormal;
      int offset = planeFlowStride / 2;
      size_t drawn = 0;
      for (int y = offset; y < height; y += planeFlowStride) {
        for (int x = offset; x < width; x += planeFlowStride) {
          float px = screen.origin.x() + (x + 0.5f) * screen.pixel_size_x;
          float py = screen.origin.y() + (y + 0.5f) * screen.pixel_size_y;
          float pz = screen.origin.z();
          Vec4 pixel_center(px, py, pz, 1);
          Vec4 ray_dir = pixel_center - observer;
          ray_dir.w() = 0;
          Vec3 ray_dir3 = RayTracer::vec4ToVec3(ray_dir);
          ray_dir3.normalize();
          float t;
          PixelInfo info;
          if (!scene.trace(camera, ray_dir3, info, t)) continue;
          if (std::abs(
                  planeNormal.dot(Vec3(info.pos.x, info.pos.y, info.pos.z)) -
                  planeOffset) > RayTracer::LARGE_EPSILON)
            continue;
          Vec3 P = camera + ray_dir3 * t;
          Vec3 L = hasPointLight
                       ? (pointLightPos - P)
                       : (hasDirLight ? (-dirLightDir) : Vec3(0, 0, 0));
          Vec3 n_used = (n.dot(L) < 0.0f) ? -n : n;
          Vec3 L_tan = L - n_used * n_used.dot(L);
          float len = L_tan.norm();
          if (len < RayTracer::MIN_INTERSECTION_DISTANCE) continue;
          Vec3 dir = L_tan / len;
          float stepWorld = 0.8f;
          Vec3 Q = P + stepWorld * dir;
          int x1, y1;
          if (!projectToScreen(Q, x1, y1)) continue;
          cv::arrowedLine(image, cv::Point(x, height - 1 - y),
                          cv::Point(x1, y1), cv::Scalar(255, 0, 255), 1,
                          cv::LINE_AA, 0, 0.25);
          if (++drawn >= RayTracer::MAX_FLOW_ARROWS) break;
        }
        if (drawn >= RayTracer::MAX_FLOW_ARROWS) break;
      }
    }

    if (showPositionMarkers) {
      for (const auto& obj : scene.objects) {
        Position pos = obj->getPos();
        Vec3 posVec(pos.x, pos.y, pos.z);
        int px, py;
        if (projectToScreen(posVec, px, py)) {
          cv::circle(image, cv::Point(px, py), 6, cv::Scalar(255, 255, 0), 2);
          cv::circle(image, cv::Point(px, py), 3, cv::Scalar(0, 255, 255), -1);

          std::string posText = "(" + std::to_string((int)pos.x) + "," +
                                std::to_string((int)pos.y) + "," +
                                std::to_string((int)pos.z) + ")";
          cv::putText(image, posText, cv::Point(px + 8, py - 8),
                      cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(255, 255, 0),
                      1);
        }
      }
    }
  }
};

#endif