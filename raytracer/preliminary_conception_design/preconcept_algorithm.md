# Primary Rays

Pixel center: dx=1/dpiX, dy=1/dpiY, Pij = P + ((i+0.5)dx, (j+0.5)dy, 0)

ray = (eye, normalize(Pij - eye))

hit = scene.traceNearest(ray) → smallest t>0

Color: hit ? shade(hit, ray, 0) : background

# Shading (Phong + Reflection/Transmission)

Ea = Ka ⊙ Ia

For each visible light L:

Ldir = normalize(L - p), Shadow ray with epsilon offset

Diffuse: Ed = Kd ⊙ IL * max(0, Ldir·N)

Specular: Es = Ks ⊙ IL * (max(0, reflect(-Ldir,N)·V))^α

Elights ⟵ Elights ⊕ (Ed ⊕ Es) (⊕: 1 - (1-a)(1-b) channel-wise)

Recursion (if depth < maxDepth):

Reflection: R = reflect(I,N) → Er = Kr ⊙ trace(p+εN, R)

Transmission (Snell, TIR-Check): T = refract(I,N,n,n') → Et = Kt ⊙ trace(p-εN, T)

Final color: E = clamp( Ea ⊕ Elights ⊕ Er ⊕ Et )

# Intersection Tests (core only)

Sphere: SC=C-S, Δ = r² + (d·SC)² - ||SC||²; t = (d·SC) ± √Δ, take smallest t>0, N=(p-C)/r

Plane: if N·d=0 → parallel; else t=(r - N·S)/ (N·d), t>0, N constant

Transformation: before intersection r' = M⁻¹ r, afterwards point via M, normal via (M⁻¹)ᵀ (normalize)