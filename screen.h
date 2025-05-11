#ifndef SCREEN_H
#define SCREEN_H

#include <opencv2/opencv.hpp>
#include "interval.h"
#include "vec3.h"

class Screen
{
private:
    unsigned char *image_buffer;
    int width;
    int height;
    double scale;

public:
    Screen() {}
    Screen(int width, int height, double scale) : width(width), height(height), scale(scale)
    {
        image_buffer = new unsigned char[width * height * 3];
    }

    void clear()
    {
        for (int i = 0; i < width * height * 3; i++)
        {
            image_buffer[i] = 0;
        }
    }

    void set_color(int x, int y, vec3 pixel_color)
    {
        auto r = pixel_color.x();
        auto g = pixel_color.y();
        auto b = pixel_color.z();

        // Apply gamma correction
        r = linear_to_gamma(r);
        g = linear_to_gamma(g);
        b = linear_to_gamma(b);

        // Clamp to [0, 0.999] and convert to [0,255]
        static const interval intensity(0.000, 0.999);
        int rbyte = int(256 * intensity.clamp(r));
        int gbyte = int(256 * intensity.clamp(g));
        int bbyte = int(256 * intensity.clamp(b));

        // 写入 image_buffer，注意行列要换算成1D偏移
        int index = (y * width + x) * 3; // 每像素3字节RGB
        // 使用BGR顺序存储，绘图时无需转换
        image_buffer[index + 0] = bbyte;
        image_buffer[index + 1] = gbyte;
        image_buffer[index + 2] = rbyte;
    }

    void display(int delay = 1)
    {
        // 1. 创建图像对象
        cv::Mat image(height, width, CV_8UC3, image_buffer);

        // 2. 按比例缩放图像
        cv::Mat scaled_image;
        cv::resize(image, scaled_image, cv::Size(), scale, scale);

        // 3. 显示图像
        cv::imshow("image", scaled_image);
        cv::waitKey(delay);
    }

    void save(const std::string &filepath)
    {
        cv::Mat image(height, width, CV_8UC3, image_buffer);
        cv::imwrite(filepath, image);
    }
};

#endif
