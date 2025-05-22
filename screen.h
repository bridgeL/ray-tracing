#ifndef SCREEN_H
#define SCREEN_H

#include <opencv2/opencv.hpp>
#include "interval.h"
#include "vec3.h"

class Screen
{
private:
    unsigned char *image_buffer; // Buffer for storing image data
    int width;                   // Image width in pixels
    int height;                  // Image height in pixels
    double scale;                // Display scaling factor
    std::string name;            // Window name for display

public:
    Screen() {}
    Screen(int width, int height, double scale, std::string name) : width(width), height(height), scale(scale), name(name)
    {
        image_buffer = new unsigned char[width * height * 3];
    }

    void clear()
    {
        // Initialize all pixels to black (0)
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

        // Apply gamma correction to each color channel
        r = linear_to_gamma(r);
        g = linear_to_gamma(g);
        b = linear_to_gamma(b);

        // Clamp values to [0, 0.999] range and convert to [0,255]
        static const interval intensity(0.000, 0.999);
        int rbyte = int(256 * intensity.clamp(r));
        int gbyte = int(256 * intensity.clamp(g));
        int bbyte = int(256 * intensity.clamp(b));

        // Write to image_buffer, converting 2D coordinates to 1D offset
        int index = (y * width + x) * 3; // 3 bytes per pixel (RGB)
        // Store in BGR order (OpenCV default), no conversion needed for display
        image_buffer[index + 0] = bbyte;
        image_buffer[index + 1] = gbyte;
        image_buffer[index + 2] = rbyte;
    }

    void display(int delay = 1)
    {
        // 1. Create OpenCV image from buffer
        cv::Mat image(height, width, CV_8UC3, image_buffer);

        // 2. Scale image according to display scale factor
        cv::Mat scaled_image;
        cv::resize(image, scaled_image, cv::Size(), scale, scale);

        // 3. Display the scaled image
        cv::imshow(name, scaled_image);
        cv::waitKey(delay);
    }

    void save(const std::string &filepath)
    {
        // Create OpenCV image and save to file
        cv::Mat image(height, width, CV_8UC3, image_buffer);
        cv::imwrite(filepath, image);
    }
};

#endif
