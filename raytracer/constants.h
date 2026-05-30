#pragma once
#include <cmath>

/**
 * @file constants.h
 * @brief Centralized constants and configuration values
 */

namespace RayTracer {
// Mathematical Constants
constexpr float EPSILON = 1e-6f;
constexpr float LARGE_EPSILON = 1e-3f;
constexpr float PI = 3.14159265359f;
constexpr float TWO_PI = 6.28318530718f;
constexpr float INV_PI = 0.31830988618f;

// Color Constants
constexpr float MAX_COLOR_VALUE = 255.0f;
constexpr float INV_MAX_COLOR = 1.0f / 255.0f;

// Rendering Constants
constexpr int MAX_RAY_DEPTH = 5;
constexpr int DEFAULT_RAY_DEPTH = 2;
constexpr float MIN_RAY_CONTRIBUTION = 0.01f;

// Material Constants
constexpr float DEFAULT_SHININESS = 32.0f;
constexpr float DEFAULT_AMBIENT = 0.3f;
constexpr float DEFAULT_DIFFUSE = 1.0f;
constexpr float DEFAULT_SPECULAR = 0.5f;
constexpr float DEFAULT_REFRACTIVE_INDEX = 1.0f;

// Lighting Constants
constexpr float AMBIENT_INTENSITY = 0.12f;
constexpr float SHADOW_BIAS = 1e-3f;
constexpr float LIGHT_ATTENUATION_CONSTANT = 1.0f;
constexpr float LIGHT_ATTENUATION_LINEAR = 0.0f;
constexpr float LIGHT_ATTENUATION_QUADRATIC = 0.0f;

// Geometric Constants
constexpr float MIN_INTERSECTION_DISTANCE = 1e-5f;
constexpr float MAX_INTERSECTION_DISTANCE = 1e6f;

// Visualization Constants
constexpr int DEFAULT_GRID_SPACING = 1;
constexpr float DEFAULT_AXIS_LENGTH = 2.0f;
constexpr int RAY_VISUALIZATION_STRIDE = 16;
constexpr int MAX_FLOW_ARROWS = 1000;

// Performance Constants
constexpr size_t OBJECT_POOL_SIZE = 1000;
constexpr int PIXEL_BATCH_SIZE = 64;

// JSON Parsing Constants
constexpr size_t MAX_JSON_DEPTH = 32;
constexpr size_t JSON_BUFFER_SIZE = 8192;

// Screen and Camera Constants
constexpr int DEFAULT_DPI = 240;
constexpr float DEFAULT_SCREEN_WIDTH = 8.0f;
constexpr float DEFAULT_SCREEN_HEIGHT = 4.5f;

// Error Codes
enum class ErrorCode {
  SUCCESS = 0,
  FILE_NOT_FOUND = 1,
  INVALID_JSON = 2,
  INVALID_GEOMETRY = 3,
  MEMORY_ALLOCATION_FAILED = 4,
  INVALID_PARAMETERS = 5
};

// Utility Functions
/**
 * @brief Check if a floating point value is effectively zero
 */
constexpr bool isZero(float value) { return std::abs(value) < EPSILON; }

/**
 * @brief Check if two floating point values are equal within epsilon
 */
constexpr bool areEqual(float a, float b) { return std::abs(a - b) < EPSILON; }

/**
 * @brief Clamp a value between min and max
 */
constexpr float clamp(float value, float min, float max) {
  return (value < min) ? min : (value > max) ? max : value;
}

/**
 * @brief Convert degrees to radians
 */
constexpr float degToRad(float degrees) { return degrees * PI / 180.0f; }

/**
 * @brief Convert radians to degrees
 */
constexpr float radToDeg(float radians) { return radians * 180.0f / PI; }
}  // namespace RayTracer