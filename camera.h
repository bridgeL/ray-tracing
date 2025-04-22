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

    double defocus_angle = 0; // Variation angle of rays through each pixel
    double focus_dist = 10;   // Distance from camera lookfrom point to plane of perfect focus

    // 存储像素颜色
    std::vector<Vector3f> frame_buf;

    // 新增鱼眼参数
    bool fisheye_enabled = false; // 是否启用鱼眼
    double fisheye_fov = 180.0;   // 鱼眼视场角（通常180-360度）

    void render(const hittable_list &world, bool display)
    {
        initialize();

        std::cout << "image size: " << image_width << ' ' << image_height << "\nPush any key to cancel\n";

        std::shared_ptr<BVHNode> bvh_tree = world.create_bvh_tree();

        for (int j = 0; j < image_height; j++)
        {
            std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
            for (int i = 0; i < image_width; i++)
            {
                Vector3f pixel_color(0, 0, 0);
                for (int sample = 0; sample < samples_per_pixel; sample++)
                {
                    ray r = get_ray(i, j);

                    // 调试对比
                    // pixel_color += ray_color_without_bvh(r, max_depth, world);
                    pixel_color += ray_color(r, max_depth, bvh_tree);
                }
                // write_color(std::cout, pixel_samples_scale * pixel_color);
                set_pixel(j, i, pixel_samples_scale * pixel_color);
            }

            if (display && j > 0 && j % 10 == 0)
                display_image(1);
        }

        std::clog << "\rDone.                 \n";
    }

    void display_image(int delay)
    {
        cv::Mat image(image_height, image_width, CV_32FC3, frame_buf.data());
        cv::imshow("image", image);
        cv::waitKey(delay);
    }

    void save_image(const std::string &filename)
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
        if (!fisheye_enabled)
        {
            // 原有透视投影逻辑
            auto offset = sample_square();
            auto pixel_sample = pixel00_loc + ((i + offset.x()) * pixel_delta_u) + ((j + offset.y()) * pixel_delta_v);
            auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
            return ray(ray_origin, (pixel_sample - ray_origin).normalized());
        }
        else
        {
            // 鱼眼特效模式
            return get_fisheye_ray(i, j);
        }
    }

    ray get_fisheye_ray(int i, int j) const
    {
        // 将像素坐标归一化到[-1,1]
        double nx = 2.0 * (i + 0.5) / image_width - 1.0;
        // double ny = 2.0 * (j + 0.5) / image_height - 1.0;
        double ny = 1.0 - 2.0 * (j + 0.5) / image_height;
        // 计算极坐标
        double r = sqrt(nx * nx + ny * ny);
        double theta = r * degrees_to_radians(fisheye_fov / 2.0);

        // 防止超出有效范围
        if (r > 1e-6)
        {
            double sin_theta = sin(theta);
            double cos_theta = cos(theta);
            double phi = atan2(ny, nx);

            // 计算射线方向（相机坐标系）
            Vector3f ray_dir = Vector3f(
                sin_theta * cos(phi),
                sin_theta * sin(phi),
                -cos_theta);

            // 转换到世界坐标系
            ray_dir = u * ray_dir.x() + v * ray_dir.y() + w * ray_dir.z();
            return ray(center, ray_dir.normalized());
        }
        return ray(center, -w); // 中心直接向前
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

    Vector3f ray_color(const ray &r, int depth, const std::shared_ptr<BVHNode> &bvh_tree) const
    {
        // If we've exceeded the ray bounce limit, no more light is gathered.
        if (depth <= 0)
            return Vector3f(0, 0, 0);

        hit_record rec;
        bool hit_anything = bvh_tree->hit(r, interval(0.001, infinity), rec);

        if (!hit_anything)
        {
            Vector3f unit_direction = r.direction().normalized();
            auto a = 0.5 * (unit_direction.y() + 1.0);
            return (1.0 - a) * Vector3f(1.0, 1.0, 1.0) + a * Vector3f(0.5, 0.7, 1.0);
        }

        ray scattered;
        Vector3f attenuation;
        if (rec.mat->scatter(r, rec, attenuation, scattered))
            return rec.mat->mix_color(attenuation, ray_color(scattered, depth - 1, bvh_tree));
        return Vector3f(0, 0, 0);
    }

    Vector3f ray_color_without_bvh(const ray &r, int depth, const hittable_list &world) const
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
            return rec.mat->mix_color(attenuation, ray_color_without_bvh(scattered, depth - 1, world));
        return Vector3f(0, 0, 0);
    }
};

#endif
