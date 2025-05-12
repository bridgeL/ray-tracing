#ifndef TEXTURE_H
#define TEXTURE_H

#include "vec3.h"
#include <opencv2/opencv.hpp>

class texture
{
public:
    virtual ~texture() = default;

    virtual vec3 value(double u, double v) const = 0;
};

class solid_color : public texture
{
public:
    solid_color(const vec3 &albedo) : albedo(albedo) {}
    solid_color(double red, double green, double blue) : solid_color(vec3(red, green, blue)) {}

    vec3 value(double u, double v) const override
    {
        return albedo;
    }

private:
    vec3 albedo;
};

class image_texture : public texture
{
public:
    image_texture(const std::string &filename)
    {
        std::string filepath = "../" + filename; // 在filename前添加"../"
        image = cv::imread(filepath);
        if (image.empty())
        {
            std::cerr << "Error: Failed to load image '" << filepath << "'." << std::endl;
        }
    }

    vec3 value(double u, double v) const override
    {
        // If we have no texture data, then return solid cyan as a debugging aid.
        if (image.rows <= 0)
            return vec3(1, 0, 1);

        // Clamp input texture coordinates to [0,1] x [1,0]
        u = interval(0, 1).clamp(u);
        v = 1.0 - interval(0, 1).clamp(v); // Flip V to image coordinates

        auto i = int(v * image.rows);
        auto j = int(u * image.cols);
        auto pixel = image.at<cv::Vec3b>(i, j);
        return vec3(pixel[2] / 255.0, pixel[1] / 255.0, pixel[0] / 255.0);
    }

private:
    cv::Mat image;
};

#endif
