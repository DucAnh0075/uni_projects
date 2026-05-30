#pragma once
#include <Eigen/Dense>
#include <vector>

#include "constants.h"
#include "scene.h"

/**
 * @file performance_utils.h
 * @brief Performance optimization utilities for ray tracing
 */

namespace RayTracer {

/** @brief Pre-computed ray grid for performance optimization
 *  @see docs/api_documentation.md for detailed documentation */
class RayGrid {
 private:
  std::vector<Eigen::Vector3f> rayDirections;
  std::vector<Eigen::Vector3f> rayOrigins;
  int width_, height_;

 public:
  /** @brief Initialize the ray grid */
  void initialize(const Screen& screen, const Eigen::Vector4f& observer,
                  int width, int height) {
    width_ = width;
    height_ = height;

    rayDirections.reserve(width * height);
    rayOrigins.reserve(width * height);

    Eigen::Vector3f observer3(observer.x(), observer.y(), observer.z());

    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        float px = screen.origin.x() + (x + 0.5f) * screen.pixel_size_x;
        float py = screen.origin.y() + (y + 0.5f) * screen.pixel_size_y;
        float pz = screen.origin.z();
        Eigen::Vector4f pixel_center(px, py, pz, 1);

        Eigen::Vector4f ray_dir = pixel_center - observer;
        ray_dir.w() = 0;
        Eigen::Vector3f ray_dir3(ray_dir.x(), ray_dir.y(), ray_dir.z());
        ray_dir3.normalize();

        rayDirections.push_back(ray_dir3);
        rayOrigins.push_back(observer3);
      }
    }
  }

  /** @brief Get pre-computed ray direction for pixel (x, y) */
  const Eigen::Vector3f& getRayDirection(int x, int y) const {
    return rayDirections[y * width_ + x];
  }

  /** @brief Get ray origin for pixel (x, y) */
  const Eigen::Vector3f& getRayOrigin(int x, int y) const {
    return rayOrigins[y * width_ + x];
  }

  /** @brief Get width of the ray grid */
  int getWidth() const { return width_; }

  /** @brief Get height of the ray grid */
  int getHeight() const { return height_; }
};

/** @brief Simple object pool for PixelInfo objects
 *  @see docs/api_documentation.md for detailed documentation */
class PixelInfoPool {
 private:
  std::vector<PixelInfo> pool;
  size_t nextIndex = 0;

 public:
  /** @brief Initialize the pool with a given size */
  PixelInfoPool(size_t size = OBJECT_POOL_SIZE) : pool(size) {
    for (auto& pixel : pool) {
      pixel = PixelInfo{Color{0, 0, 0}, Position{0, 0, 0}, 0.0};
    }
  }

  /** @brief Get a PixelInfo object from the pool */
  PixelInfo& acquire() {
    PixelInfo& pixel = pool[nextIndex];
    nextIndex = (nextIndex + 1) % pool.size();
    return pixel;
  }

  /** @brief Reset a PixelInfo object (called when returning to pool) */
  void release(PixelInfo& pixel) {
    pixel = PixelInfo{Color{0, 0, 0}, Position{0, 0, 0}, 0.0};
  }
};

/** @brief Material cache for frequently accessed materials
 *  @see docs/api_documentation.md for detailed documentation */
class MaterialCache {
 private:
  struct CachedMaterial {
    Material material;
    Eigen::Vector3f normalizedAmbient;
    Eigen::Vector3f normalizedDiffuse;
    Eigen::Vector3f normalizedSpecular;
    bool isValid = false;
  };

  std::map<const Material*, CachedMaterial> cache;

 public:
  /** @brief Get or compute cached material properties */
  const CachedMaterial& getCachedMaterial(const Material* mat) {
    auto it = cache.find(mat);
    if (it != cache.end() && it->second.isValid) {
      return it->second;
    }

    CachedMaterial& cached = cache[mat];
    cached.material = *mat;
    cached.normalizedAmbient = mat->Ka;
    cached.normalizedDiffuse = mat->Kd;
    cached.normalizedSpecular = mat->Ks;
    cached.isValid = true;

    return cached;
  }

  /** @brief Clear the cache */
  void clear() { cache.clear(); }
};

/** @brief Batch processor for pixel rendering
 *  @see docs/api_documentation.md for detailed documentation */
class PixelBatchProcessor {
 private:
  struct PixelBatch {
    int startX, startY;
    int width, height;
  };

  std::vector<PixelBatch> batches;

 public:
  /** @brief Initialize batches for given image dimensions */
  void initializeBatches(int imageWidth, int imageHeight,
                         int batchSize = PIXEL_BATCH_SIZE) {
    batches.clear();

    for (int y = 0; y < imageHeight; y += batchSize) {
      for (int x = 0; x < imageWidth; x += batchSize) {
        PixelBatch batch;
        batch.startX = x;
        batch.startY = y;
        batch.width = std::min(batchSize, imageWidth - x);
        batch.height = std::min(batchSize, imageHeight - y);
        batches.push_back(batch);
      }
    }
  }

  /** @brief Get the number of batches */
  size_t getBatchCount() const { return batches.size(); }

  /** @brief Get batch at index */
  const PixelBatch& getBatch(size_t index) const { return batches[index]; }

  /** @brief Process all batches with a given function */
  template <typename ProcessFunc>
  void processAllBatches(ProcessFunc func) {
    for (const auto& batch : batches) {
      func(batch.startX, batch.startY, batch.width, batch.height);
    }
  }
};

/** @brief Intersection cache for scene objects
 *  @see docs/api_documentation.md for detailed documentation */
class IntersectionCache {
 private:
  struct CacheEntry {
    Eigen::Vector3f rayOrigin;
    Eigen::Vector3f rayDirection;
    bool hasIntersection;
    float distance;
    std::shared_ptr<Object> object;
    PixelInfo pixelInfo;
    uint64_t frameNumber = 0;
  };

  std::vector<CacheEntry> cache;
  size_t nextIndex = 0;
  uint64_t currentFrame = 0;

  static constexpr size_t CACHE_SIZE = 1024;

 public:
  IntersectionCache() : cache(CACHE_SIZE) {}

  /** @brief Start a new frame (invalidates old entries) */
  void startNewFrame() { currentFrame++; }

  /** @brief Try to find a cached intersection result */
  bool findCached(const Eigen::Vector3f& rayOrigin,
                  const Eigen::Vector3f& rayDirection, bool& hasIntersection,
                  float& distance, std::shared_ptr<Object>& object,
                  PixelInfo& pixelInfo) {
    for (const auto& entry : cache) {
      if (entry.frameNumber == currentFrame &&
          areEqual(entry.rayOrigin.x(), rayOrigin.x()) &&
          areEqual(entry.rayOrigin.y(), rayOrigin.y()) &&
          areEqual(entry.rayOrigin.z(), rayOrigin.z()) &&
          areEqual(entry.rayDirection.x(), rayDirection.x()) &&
          areEqual(entry.rayDirection.y(), rayDirection.y()) &&
          areEqual(entry.rayDirection.z(), rayDirection.z())) {
        hasIntersection = entry.hasIntersection;
        distance = entry.distance;
        object = entry.object;
        pixelInfo = entry.pixelInfo;
        return true;
      }
    }
    return false;
  }

  /** @brief Cache an intersection result */
  void cacheResult(const Eigen::Vector3f& rayOrigin,
                   const Eigen::Vector3f& rayDirection, bool hasIntersection,
                   float distance, const std::shared_ptr<Object>& object,
                   const PixelInfo& pixelInfo) {
    CacheEntry& entry = cache[nextIndex];
    entry.rayOrigin = rayOrigin;
    entry.rayDirection = rayDirection;
    entry.hasIntersection = hasIntersection;
    entry.distance = distance;
    entry.object = object;
    entry.pixelInfo = pixelInfo;
    entry.frameNumber = currentFrame;

    nextIndex = (nextIndex + 1) % CACHE_SIZE;
  }
};

}