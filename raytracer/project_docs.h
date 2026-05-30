#pragma once

/**
 * @mainpage Ray Tracer Project
 *
 * @section overview Overview
 * Advanced ray tracing engine with performance optimizations and comprehensive
 * geometric primitive support.
 *
 * @section features Key Features
 * - High-performance ray tracing with pre-computation optimizations
 * - Support for spheres, cubes, planes, and CSG operations
 * - Advanced lighting with reflection, refraction, and multiple light sources
 * - Object pooling and caching for memory efficiency
 * - Batch processing for improved cache locality
 *
 * @section architecture Architecture
 *
 * @subsection core Core Components
 * - **performance_utils.h**: Optimization utilities (ray pre-computation,
 * object pools, caching)
 * - **render.h**: Main rendering engine and ray tracing algorithms
 * - **object.h**: Geometric primitives and intersection algorithms
 * - **scene.h**: Scene structure and camera configuration
 * - **math_utils.h**: Mathematical utilities and vector operations
 *
 * @subsection performance Performance Features
 * - Pre-computed ray grids eliminate repeated calculations
 * - Object pools reduce memory allocation overhead
 * - Material caching avoids redundant computations
 * - Intersection caching for frequently tested rays
 * - Pixel batch processing for better cache locality
 *
 * @author Jokers Gone Wild Team
 * @date 2025
 */