#ifndef SCREEN_H
#define SCREEN_H

#include <opencv2/opencv.hpp>
#include "interval.h"
#include "vec3.h"

class Screen
{
private:
  unsigned char *image_buffer;
  int image_width;
  int image_height;

public:
  Screen() {}
  Screen(int width, int height) : image_width(width), image_height(height)
  {
    image_buffer = new unsigned char[width * height * 3];
  }

  void clear()
  {
    for (int i = 0; i < image_width * image_height * 3; i++)
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
    int index = (y * image_width + x) * 3; // 每像素3字节RGB
    // 使用BGR顺序存储，绘图时无需转换
    image_buffer[index + 0] = bbyte;
    image_buffer[index + 1] = gbyte;
    image_buffer[index + 2] = rbyte;
  }

  void display(int delay)
  {
    cv::Mat image(image_height, image_width, CV_8UC3, image_buffer);
    cv::imshow("image", image);
    cv::waitKey(delay);
  }

  void save(const std::string &filepath)
  {
    cv::Mat image(image_height, image_width, CV_8UC3, image_buffer);
    cv::imwrite(filepath, image);
    cv::imshow("image", image);
    cv::waitKey(0);
  }
};

#endif
