#ifndef GLOBAL_H
#define GLOBAL_H

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>
#include <vector>
#include <algorithm>
#include <opencv2/opencv.hpp>

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

inline double linear_to_gamma(double linear_component)
{
    if (linear_component > 0)
        return std::sqrt(linear_component);

    return 0;
}

cv::Mat read_image(const std::string &filepath)
{
    auto bgr_image = cv::imread(filepath); // 默认读取为BGR格式
    cv::Mat rgb_image;
    cv::cvtColor(bgr_image, rgb_image, cv::COLOR_BGR2RGB);
    return rgb_image;
}

double normalizeUV(double uv)
{
    double wrapped = std::fmod(uv, 1.0);
    if (wrapped < 0)
        wrapped += 1.0; // 处理负值
    return wrapped;
}

// Common Headers

#include "vec3.h"
#include "interval.h"
#include "ray.h"
#include "bbox.h"
#include "hittable.h"
#include "hittable_list.h"
#include "material.h"
#include "bvh.h"

#endif
