#ifndef CAMERA_H
#define CAMERA_H

#include "screen.h"

class camera
{
public:
    double aspect_ratio = 1.0;  // Ratio of image width over height
    int image_width = 100;      // Rendered image width in pixel count
    int samples_per_pixel = 10; // Count of random samples for each pixel
    int max_depth = 10;         // Maximum number of ray bounces into scene

    double screen_scale = 1.0;
    std::string screen_name = "image";
    vec3 background_color = vec3(1, 1, 1);

    double vfov = 90;              // Vertical view angle (field of view)
    vec3 lookfrom = vec3(0, 0, 0); // Point camera is looking from
    vec3 lookat = vec3(0, 0, -1);  // Point camera is looking at
    vec3 vup = vec3(0, 1, 0);      // Camera-relative "up" direction

    double defocus_angle = 0; // Variation angle of rays through each pixel
    double focus_dist = 10;   // Distance from camera lookfrom point to plane of perfect focus

    Screen screen;

    void render(const hittable_list &world, bool display, bool use_openmp)
    {

#pragma omp parallel for schedule(dynamic) if (use_openmp)
        for (int j = 0; j < image_height; j++)
        {
            std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
            for (int i = 0; i < image_width; i++)
            {
                // 展示一条渐变色带，帮助我们分析bvh tree到底有多深，各个像素点又是位于多深的位置被命中的
                if (j < 10)
                {
                    screen.set_color(i, j, convert_int_to_color(i, image_width));
                    continue;
                }

                vec3 pixel_color(0, 0, 0);
                for (int sample = 0; sample < samples_per_pixel; sample++)
                {
                    ray r = get_ray(i, j);
                    pixel_color += ray_color(r, max_depth, world);
                }
                screen.set_color(i, j, pixel_samples_scale * pixel_color);
            }

            if (display && j > 0 && j % 10 == 0)
                screen.display(1);
        }

        std::clog << "\rDone.                 \n";
    }

    void initialize()
    {
        image_height = int(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;

        pixel_samples_scale = 1.0 / samples_per_pixel;

        center = lookfrom;

        // Determine viewport dimensions.
        auto theta = degrees_to_radians(vfov);
        auto h = std::tan(theta / 2);
        auto viewport_height = 2 * h * focus_dist;
        auto viewport_width = viewport_height * (double(image_width) / image_height);

        // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
        w = (lookfrom - lookat).normalized();
        u = vup.cross(w).normalized();
        v = w.cross(u);

        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        vec3 viewport_u = viewport_width * u;   // Vector across viewport horizontal edge
        vec3 viewport_v = viewport_height * -v; // Vector down viewport vertical edge

        // Calculate the horizontal and vertical delta vectors from pixel to pixel.
        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        // Calculate the location of the upper left pixel.
        auto viewport_upper_left = center - (focus_dist * w) - viewport_u / 2 - viewport_v / 2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

        // Calculate the camera defocus disk basis vectors.
        auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;

        // create screen
        screen = Screen(image_width, image_height, screen_scale, screen_name);
        screen.clear();

        std::cout << "image size: " << image_width << ' ' << image_height << std::endl;
    }

private:
    int image_height;           // Rendered image height
    double pixel_samples_scale; // Color scale factor for a sum of pixel samples
    vec3 center;                // Camera center
    vec3 pixel00_loc;           // Location of pixel 0, 0
    vec3 pixel_delta_u;         // Offset to pixel to the right
    vec3 pixel_delta_v;         // Offset to pixel below
    vec3 u, v, w;               // Camera frame basis vectors
    vec3 defocus_disk_u;        // Defocus disk horizontal radius
    vec3 defocus_disk_v;        // Defocus disk vertical radius

    ray get_ray(int i, int j) const
    {
        // Construct a camera ray originating from the defocus disk and directed at a randomly
        // sampled point around the pixel location i, j.

        auto offset = sample_square();
        auto pixel_sample = pixel00_loc + ((i + offset.x()) * pixel_delta_u) + ((j + offset.y()) * pixel_delta_v);

        auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
        auto ray_direction = (pixel_sample - ray_origin).normalized();

        return ray(ray_origin, ray_direction);
    }

    vec3 sample_square() const
    {
        // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
        return vec3(random_double() - 0.5, random_double() - 0.5, 0);
    }

    vec3 random_in_unit_disk() const
    {
        while (true)
        {
            auto p = vec3(random_double(-1, 1), random_double(-1, 1), 0);
            if (p.length_squared() < 1)
                return p;
        }
    }

    vec3 sample_disk(double radius) const
    {
        // Returns a random point in the unit (radius 0.5) disk centered at the origin.
        return radius * random_in_unit_disk();
    }

    vec3 defocus_disk_sample() const
    {
        // Returns a random point in the camera defocus disk.
        auto p = random_in_unit_disk();
        return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }

    vec3 ray_color(const ray &r, int depth, const hittable_list &world) const
    {
        // If we've exceeded the ray bounce limit, no more light is gathered.
        if (depth <= 0)
            return vec3(0, 0, 0);

        hit_record rec;
        bool hit_anything = world.hit(r, interval(0.001, infinity), rec);

        if (!hit_anything)
            return background_color;

        vec3 emit_color;
        bool can_emit = rec.mat->emit(r, rec, emit_color);
        if (can_emit)
            return emit_color;

        ray scattered;
        vec3 attenuation;

        bool can_scatter = rec.mat->scatter(r, rec, attenuation, scattered);

        if (can_scatter)
            return attenuation * ray_color(scattered, depth - 1, world);

        return vec3(0, 0, 0);
    }
};

#endif
