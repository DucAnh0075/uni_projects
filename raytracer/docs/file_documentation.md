# File Documentation

## Core Headers

### performance_utils.h
**Purpose:** Performance optimization utilities for ray tracing

**Description:** This header contains optimizations like ray pre-computation, object pools, and caching mechanisms.

**Key Components:**
- RayGrid: Pre-computed ray directions and origins
- PixelInfoPool: Object pool for memory efficiency
- MaterialCache: Caches material computations
- PixelBatchProcessor: Batch processing for cache locality
- IntersectionCache: Caches intersection results

### render.h
**Purpose:** Main rendering engine and ray tracing algorithms

**Description:** Contains the core ray tracing logic, lighting calculations, and scene rendering functionality.

### math_utils.h
**Purpose:** Mathematical utilities and vector operations

**Description:** Helper functions for vector math, transformations, and geometric calculations.

### object.h
**Purpose:** Scene object definitions and intersection algorithms

**Description:** Defines geometric primitives (spheres, cubes, planes) and CSG operations.

### scene.h
**Purpose:** Scene structure and screen configuration

**Description:** Defines the scene structure, camera setup, and screen parameters.