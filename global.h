#ifndef GLOBAL_H
#define GLOBAL_H

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>
#include <opencv2/opencv.hpp>
#include <Eigen/Eigen>

using namespace Eigen;

// C++ Std Usings

using std::make_shared;
using std::shared_ptr;

// Constants

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Utility Functions

inline double degrees_to_radians(double degrees)
{
    return degrees * pi / 180.0;
}

inline double random_double()
{
    // Returns a random real in [0,1).
    return std::rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max)
{
    // Returns a random real in [min,max).
    return min + (max - min) * random_double();
}

// 生成一个随机的单位向量（均匀分布在单位球面上）
inline Vector3f random_unit_vector()
{
    while (true)
    {
        // 在 [-1, 1) 范围内随机生成
        Vector3f p(random_double(-1, 1), random_double(-1, 1), random_double(-1, 1));

        // 计算长度的平方
        float lensq = p.squaredNorm();

        // 避免接近零的情况
        // 避免8个角落方向的概率高于平均值
        if (lensq > 1e-16f && lensq <= 1.0f)
            return p.normalized(); // 归一化并返回
    }
}

inline bool near_zero_vector(Vector3f v)
{
    return v.squaredNorm() < 1e-16f;
}

inline Vector3f reflect(const Vector3f &v, const Vector3f &n)
{
    return v - 2.0f * v.dot(n) * n;
}

inline Vector3f interpolate(const Vector3f &weight, const Vector3f &p1, const Vector3f &p2, const Vector3f &p3)
{
    return weight[0] * p1 + weight[1] * p2 + weight[2] * p3;
}

inline Vector2f interpolate(const Vector3f &weight, const Vector2f &p1, const Vector2f &p2, const Vector2f &p3)
{
    return weight[0] * p1 + weight[1] * p2 + weight[2] * p3;
}

// Common Headers

#include "interval.h"
#include "ray.h"
#include "hittable.h"
#include "hittable_list.h"
#include "material.h"

#endif
