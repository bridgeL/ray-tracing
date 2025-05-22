# Ray-Tracing

## Project Overview

Our goal is to implement an optimized ray tracer based on **Ray Tracing in One Weekend** framework [^rtweekend], with:

- **Core optimizations**:
  - BVH construction with surface area heuristic (SAH)
  - Parallel acceleration* via OpenMP

[^rtweekend]: P. Shirley et al. *Ray Tracing in One Weekend*. GitHub, 2020. https://github.com/RayTracing/raytracing.github.io

The room model comes from free3d [^free3d]

[^free3d]: Isometric Room. https://free3d.com/3d-model/isometric-room-362653.html

![img](doc/1.png)
![img](doc/2.png)
![img](doc/3.png)
![img](doc/4.png)

---

## Installation

```
sudo apt install -y cmake g++ gdb
sudo apt install libopencv-dev
sudo apt install libeigen3-dev
```

## How to run

```
mkdir build
cd build
cmake .. 
make && ./main -i 0 -sa 1000
```

## Technical Approach

### Base Framework

- **Modified "Ray Tracing in One Weekend"**  
  (https://github.com/RayTracing/raytracing.github.io)
  
- Key extensions:
  - OBJ/GLB model loader
  - BVH construction with surface area heuristic (SAH)
  - PBR material system

### Optimization Pipeline

1. **Pre-processing Stage**:
   - BVH construction with SAH

2. **Ray Tracing Stage**:
   - Parallel ray batches (OpenMP)

---

## Implementation Timeline

| Week | Tasks |
|------|-------|
| 1 | Base framework + OBJ loader |
| 2 | Complex models + BVH optimization |
| 3 | Parallel acceleration + interactive controls |