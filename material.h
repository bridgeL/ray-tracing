#ifndef MATERIAL_H
#define MATERIAL_H

#include <bitset>
#include "texture.h"

class material
{
public:
    virtual ~material() = default;

    virtual bool scatter(
        const ray &r_in, const hit_record &rec, vec3 &attenuation, ray &scattered) const
    {
        return false;
    }

    virtual bool emit(const ray &r_in, const hit_record &rec, vec3 &emit_color) const
    {
        return false;
    }

    // Sample rate adjustment
    virtual int apply_sample_rate(int sample_num) const
    {
        return sample_num;
    }
};

class lambertian : public material
{
public:
    lambertian(const vec3 &albedo) : tex(make_shared<solid_color>(albedo)) {}
    lambertian(shared_ptr<texture> tex) : tex(tex) {}

    int apply_sample_rate(int sample_num) const override
    {
        sample_num = int(0.2 * sample_num);
        return std::max(1, sample_num);
    }

    bool scatter(const ray &r_in, const hit_record &rec, vec3 &attenuation, ray &scattered)
        const override
    {
        vec3 scatter_direction = rec.normal + random_unit_vector();

        // Catch degenerate scatter direction
        if (scatter_direction.near_zero())
            scatter_direction = rec.normal;

        scattered = ray(rec.p, scatter_direction);
        attenuation = tex->value(rec.u, rec.v);
        return true;
    }

private:
    shared_ptr<texture> tex;
};

vec3 convert_int_to_color(int i, int n)
{
    float phase = interval(0, n).clamp(i) * 4 / n;
    if (phase <= 1)
        return vec3(0, phase * 1, 1); // Phase 1: Blue → Cyan
    if (phase <= 2)
        return vec3(0, 1, (2 - phase) * 1); // Phase 2: Cyan → Green
    if (phase <= 3)
        return vec3((phase - 2) * 1, 1, 0); // Phase 3: Green → Yellow
    return vec3(1, (4 - phase) * 1, 0);     // Phase 4: Yellow → Red
}

class bvh_depth_visual_mat : public material
{
public:
    int h;
    std::string root;

    bvh_depth_visual_mat(int h) : h(h) {}

    bool emit(const ray &r_in, const hit_record &rec, vec3 &emit_color)
        const override
    {
        // Visualize object depth in BVH tree
        // Reddish image indicates BVH tree nodes generally exceed h/2 depth
        emit_color = convert_int_to_color(rec.bvh_depth, h);
        return true;
    }
};

class bvh_group_visual_mat : public material
{
public:
    int h;
    std::string root;

    bvh_group_visual_mat(int n, std::string root) : h(n + root.length()), root(root) {}

    bool emit(const ray &r_in, const hit_record &rec, vec3 &emit_color)
        const override
    {
        // Visualize object grouping from specified root to max depth
        int t1 = 0;
        int t2 = pow(2, h - root.length() + 1) - 2;

        if (rec.bvh_path.length() >= root.length())
        {
            // Must start from specified root
            std::string prefix = rec.bvh_path.substr(0, root.length());
            if (prefix == root)
            {
                // Extract remaining path string, but not exceeding max depth
                int n = rec.bvh_path.length();
                std::string s = rec.bvh_path.substr(root.length(), std::min(n, h) - root.length());

                // Calculate mapping
                t1 = binaryStringToInt2(s);
            }
        }

        emit_color = convert_int_to_color(t1, t2);
        return true;
    }

private:
    // Convert fixed-length binary string to integer
    static int binaryStringToInt(const std::string &binaryStr)
    {
        // Use std::bitset to parse binary string
        std::bitset<32> bits(binaryStr);          // Assume binary string <= 32 bits
        return static_cast<int>(bits.to_ulong()); // Convert to unsigned long, then to int
    }

    // Convert arbitrary-length binary string to integer
    static int binaryStringToInt2(const std::string &s)
    {
        int h = s.length();
        int t0 = pow(2, h) - 2;        // All possibilities for strings shorter than h
        int t1 = binaryStringToInt(s); // Current possibilities for strings of length h
        return t0 + t1;
    }
};

class metal : public material
{
public:
    metal(const vec3 &albedo, double fuzz) : tex(make_shared<solid_color>(albedo)), fuzz(fuzz) {}
    metal(shared_ptr<texture> tex, double fuzz) : tex(tex), fuzz(fuzz) {}

    int apply_sample_rate(int sample_num) const override
    {
        sample_num = int(0.5 * sample_num);
        return std::max(1, sample_num);
    }

    bool scatter(const ray &r_in, const hit_record &rec, vec3 &attenuation, ray &scattered)
        const override
    {
        vec3 reflected = reflect(r_in.direction(), rec.normal);
        reflected = reflected.normalized() + (fuzz * random_unit_vector());
        scattered = ray(rec.p, reflected);
        attenuation = tex->value(rec.u, rec.v);
        return (scattered.direction().dot(rec.normal) > 0);
    }

private:
    double fuzz;
    shared_ptr<texture> tex;
};

class light_mat : public material
{
public:
    light_mat(const vec3 &color, double intensity) : color(color), intensity(intensity) {}

    bool emit(const ray &r_in, const hit_record &rec, vec3 &emit_color)
        const override
    {
        emit_color = color * intensity;
        return true;
    }

private:
    vec3 color;
    double intensity;
};

class glass : public material
{
public:
    glass(double refraction_index) : refraction_index(refraction_index) {}

    bool scatter(const ray &r_in, const hit_record &rec, vec3 &attenuation, ray &scattered)
        const override
    {
        attenuation = vec3(1.0, 1.0, 1.0);
        double ri = rec.front_face ? (1.0 / refraction_index) : refraction_index;

        vec3 unit_direction = r_in.direction().normalized();
        double cos_theta = std::fmin((-unit_direction).dot(rec.normal), 1.0);
        double sin_theta = std::sqrt(1.0 - cos_theta * cos_theta);

        bool cannot_refract = ri * sin_theta > 1.0;
        vec3 direction;

        if (cannot_refract || reflectance(cos_theta, ri) > random_double())
            direction = reflect(unit_direction, rec.normal);
        else
            direction = refract(unit_direction, rec.normal, ri);

        scattered = ray(rec.p, direction);
        return true;
    }

private:
    double refraction_index;

    static double reflectance(double cosine, double refraction_index)
    {
        // Use Schlick's approximation for reflectance
        auto r0 = (1 - refraction_index) / (1 + refraction_index);
        r0 = r0 * r0;
        return r0 + (1 - r0) * std::pow((1 - cosine), 5);
    }
};

class magic_mat : public material
{
public:
    magic_mat(
        const vec3 &lookfrom,
        const vec3 &lookat,
        const vec3 &vup,
        double vfov,
        double focus_dist,
        int image_width, int image_height) : image_height(image_height), image_width(image_width)
    {
        center = lookfrom;

        // Determine viewport dimensions
        auto theta = degrees_to_radians(vfov);
        auto h = std::tan(theta / 2);
        auto viewport_height = 2 * h * focus_dist;
        auto viewport_width = viewport_height * (double(image_width) / image_height);

        // Calculate u,v,w unit basis vectors for camera coordinate frame
        w = (lookfrom - lookat).normalized();
        u = vup.cross(w).normalized();
        v = w.cross(u);

        // Calculate vectors across viewport edges
        vec3 viewport_u = viewport_width * u;   // Horizontal edge vector
        vec3 viewport_v = viewport_height * -v; // Vertical edge vector

        // Calculate pixel-to-pixel delta vectors
        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        // Calculate upper left pixel location
        auto viewport_upper_left = center - (focus_dist * w) - viewport_u / 2 - viewport_v / 2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);
    }

    bool scatter(const ray &r_in, const hit_record &rec, vec3 &attenuation, ray &scattered)
        const override
    {
        if (random_double() < 0.1)
        {
            double fuzz = 0.5;
            vec3 reflected = reflect(r_in.direction(), rec.normal);
            reflected = reflected.normalized() + (fuzz * random_unit_vector());
            scattered = ray(rec.p, reflected);
            attenuation = vec3(0.1, 0.1, 0.1);
            return (scattered.direction().dot(rec.normal) > 0);
        }

        int i = int((rec.u) * image_width);
        int j = int((1 - rec.v) * image_height);

        scattered = get_ray(i, j);
        attenuation = vec3(1, 1, 1);
        return true;
    }

private:
    int image_height;           // Rendered image height
    int image_width;            // Rendered image width
    double pixel_samples_scale; // Color scale factor for pixel samples
    vec3 center;                // Camera center
    vec3 pixel00_loc;          // Location of pixel (0, 0)
    vec3 pixel_delta_u;        // Right pixel offset
    vec3 pixel_delta_v;        // Down pixel offset
    vec3 u, v, w;              // Camera frame basis vectors

    ray get_ray(int i, int j) const
    {
        // Construct camera ray directed at random point around pixel (i,j)
        auto offset = sample_square();
        auto pixel_sample = pixel00_loc + ((i + offset.x()) * pixel_delta_u) + ((j + offset.y()) * pixel_delta_v);

        auto ray_origin = center;
        auto ray_direction = (pixel_sample - ray_origin).normalized();

        return ray(ray_origin, ray_direction);
    }

    vec3 sample_square() const
    {
        // Return random point in [-0.5,-0.5] to [0.5,0.5] unit square
        return vec3(random_double() - 0.5, random_double() - 0.5, 0);
    }
};

#endif
