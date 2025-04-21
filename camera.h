#ifndef CAMERA_H
#define CAMERA_H

class camera
{
public:
    double aspect_ratio = 1.0;  // Ratio of image width over height
    int image_width = 100;      // Rendered image width in pixel count
    int samples_per_pixel = 10; // Count of random samples for each pixel
    int max_depth = 10;         // Maximum number of ray bounces into scene

    double vfov = 90;                      // Vertical view angle (field of view)
    Vector3f lookfrom = Vector3f(0, 0, 0); // Point camera is looking from
    Vector3f lookat = Vector3f(0, 0, -1);  // Point camera is looking at
    Vector3f vup = Vector3f(0, 1, 0);      // Camera-relative "up" direction
    std::vector<Vector3f> frame_buf;

    double defocus_angle = 0; // Variation angle of rays through each pixel
    double focus_dist = 10;   // Distance from camera lookfrom point to plane of perfect focus

    void render(const hittable &world)
    {
        initialize();

        std::cout << "P3\n"
                  << image_width << ' ' << image_height << "\n255\n";

        for (int j = 0; j < image_height; j++)
        {
            std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
            for (int i = 0; i < image_width; i++)
            {
                Vector3f pixel_color(0, 0, 0);
                for (int sample = 0; sample < samples_per_pixel; sample++)
                {
                    ray r = get_ray(i, j);
                    pixel_color += ray_color(r, max_depth, world);
                }
                // write_color(std::cout, pixel_samples_scale * pixel_color);
                set_pixel(j, i, pixel_samples_scale * pixel_color);
            }
            if (j > 0 && j % 10 == 0)
                display();
        }

        std::clog << "\rDone.                 \n";

        display();
        save("output.png");
        cv::waitKey();
    }

    void display()
    {
        cv::Mat image(image_height, image_width, CV_32FC3, frame_buf.data());
        cv::imshow("image", image);
        cv::waitKey(1);
    }

    void save(const std::string &filename)
    {
        cv::Mat image(image_height, image_width, CV_32FC3, frame_buf.data());
        image.convertTo(image, CV_8UC3, 255.0f);
        cv::imwrite(filename, image);
    }

    void set_pixel(int i, int j, const Vector3f &pixel_color)
    {
        float r = float(pixel_color.x());
        float g = float(pixel_color.y());
        float b = float(pixel_color.z());

        // Write out the pixel color components.
        frame_buf[image_width * i + j] = {b, g, r};
    }

private:
    int image_height;           // Rendered image height
    double pixel_samples_scale; // Color scale factor for a sum of pixel samples
    Vector3f center;            // Camera center
    Vector3f pixel00_loc;       // Location of pixel 0, 0
    Vector3f pixel_delta_u;     // Offset to pixel to the right
    Vector3f pixel_delta_v;     // Offset to pixel below
    Vector3f u, v, w;           // Camera frame basis vectors
    Vector3f defocus_disk_u;    // Defocus disk horizontal radius
    Vector3f defocus_disk_v;    // Defocus disk vertical radius

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
        Vector3f viewport_u = viewport_width * u;   // Vector across viewport horizontal edge
        Vector3f viewport_v = viewport_height * -v; // Vector down viewport vertical edge

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

        // resize frame buffer
        frame_buf.resize(image_width * image_height);
        std::fill(frame_buf.begin(), frame_buf.end(), Vector3f{0, 0, 0});
    }

    ray get_ray(int i, int j) const
    {
        // Construct a camera ray originating from the defocus disk and directed at a randomly
        // sampled point around the pixel location i, j.

        auto offset = sample_square();
        auto pixel_sample = pixel00_loc + ((i + offset.x()) * pixel_delta_u) + ((j + offset.y()) * pixel_delta_v);

        auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
        auto ray_direction = pixel_sample - ray_origin;

        return ray(ray_origin, ray_direction);
    }

    Vector3f sample_square() const
    {
        // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
        return Vector3f(random_double() - 0.5, random_double() - 0.5, 0);
    }

    Vector3f random_in_unit_disk() const
    {
        while (true)
        {
            auto p = Vector3f(random_double(-1, 1), random_double(-1, 1), 0);
            if (p.squaredNorm() < 1)
                return p;
        }
    }

    Vector3f sample_disk(double radius) const
    {
        // Returns a random point in the unit (radius 0.5) disk centered at the origin.
        return radius * random_in_unit_disk();
    }

    Vector3f defocus_disk_sample() const
    {
        // Returns a random point in the camera defocus disk.
        auto p = random_in_unit_disk();
        return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }

    Vector3f ray_color(const ray &r, int depth, const hittable &world) const
    {
        // If we've exceeded the ray bounce limit, no more light is gathered.
        if (depth <= 0)
            return Vector3f(0, 0, 0);

        hit_record rec;
        bool hit_anything = world.hit(r, interval(0.001, infinity), rec);

        if (!hit_anything)
        {
            Vector3f unit_direction = r.direction().normalized();
            auto a = 0.5 * (unit_direction.y() + 1.0);
            return (1.0 - a) * Vector3f(1.0, 1.0, 1.0) + a * Vector3f(0.5, 0.7, 1.0);
        }

        ray scattered;
        Vector3f attenuation;
        if (rec.mat->scatter(r, rec, attenuation, scattered))
            return rec.mat->mix_color(attenuation, ray_color(scattered, depth - 1, world));
        return Vector3f(0, 0, 0);
    }
};

#endif
