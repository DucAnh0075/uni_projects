# Development Plan (Iterative with Tests)

## I1 — Foundation

Vec4, Ray, Mat4 (Trans/Rot/Scale, inverse for basic transformations)

Unit-Tests: Dot/Norm/Normalize, Transformation inverses

## I2 — Camera & Image

Camera.generateRay (Screen/P/DPI), Image (+PPM/PNG)

Test: Grid/Gradient

## I3 — Primitives & Intersection

Sphere, Plane, Scene.traceNearest

Tests: analytical cases, parallel cases

## I4 — Phong (without shadows)

Material, PointLight, Color (+⊕, clamp)

Test: Lambert hemisphere

## I5 — Shadows (hard)

Shadow ray with epsilon

Test: Sphere+Plane, correct umbra

## I6 — Reflection

reflect, recursive shade, maxDepth

Test: Mirror sphere

## I7 — Transmission

refract (Snell, TIR), ior

Test: Glass sphere (n'≈1.5)

## I8 — Transformations

Transformed(child,M,M⁻¹)

Test: Scaled/rotated sphere/plane

## I9 — Quality/Optimization (optional)

Supersampling (2×2), parallel scanlines

Simple BVH (AABB) if many objects

## Deliverables per Iteration

Brief note (½ page): Features, Tests

1 Example scene + Reference image