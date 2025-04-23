#ifndef MATERIAL_H
#define MATERIAL_H

class material
{
public:
    virtual ~material() = default;

    virtual bool scatter(
        const ray &r_in, const hit_record &rec, Vector3f &attenuation, ray &scattered) const
    {
        return false;
    }

    virtual Vector3f mix_color(const Vector3f &origin, const Vector3f &new_color) const
    {
        return Vector3f(0, 0, 0);
    }
};

class lambertian : public material
{
public:
    lambertian(const Vector3f &albedo) : albedo(albedo) {}

    bool scatter(const ray &r_in, const hit_record &rec, Vector3f &attenuation, ray &scattered)
        const override
    {
        Vector3f scatter_direction = rec.normal + random_unit_vector();

        // Catch degenerate scatter direction
        if (near_zero_vector(scatter_direction))
            scatter_direction = rec.normal;

        scattered = ray(rec.p, scatter_direction);
        attenuation = albedo;
        return true;
    }

    Vector3f mix_color(const Vector3f &origin, const Vector3f &new_color)
        const override
    {
        return origin.cwiseProduct(new_color);
    }

private:
    Vector3f albedo;
};

class textured_lambertian : public material
{
public:
    textured_lambertian(cv::Mat &image_data) : image_data(image_data) {}

    bool scatter(const ray &r_in, const hit_record &rec, Vector3f &attenuation, ray &scattered)
        const override
    {
        Vector3f scatter_direction = rec.normal + random_unit_vector();

        // Catch degenerate scatter direction
        if (near_zero_vector(scatter_direction))
            scatter_direction = rec.normal;

        scattered = ray(rec.p, scatter_direction);

        float u = rec.texture_coord[0];
        float v = rec.texture_coord[1];
        attenuation = get_color(u, v);
        return true;
    }

    Vector3f get_color(float u, float v) const
    {
        auto u_img = u * image_data.cols;       // width
        auto v_img = (1 - v) * image_data.rows; // height
        auto color = image_data.at<cv::Vec3b>(v_img, u_img);
        return Vector3f(color[0], color[1], color[2]) / 255;
    }

    Vector3f mix_color(const Vector3f &origin, const Vector3f &new_color)
        const override
    {
        return origin.cwiseProduct(new_color);
        // return origin * 0.5 + new_color * 0.5;
    }

private:
    cv::Mat image_data;
};

class metal : public material
{
public:
    metal(const Vector3f &albedo, double fuzz) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}

    bool scatter(const ray &r_in, const hit_record &rec, Vector3f &attenuation, ray &scattered)
        const override
    {
        Vector3f reflected = reflect(r_in.direction(), rec.normal);
        reflected = reflected.normalized() + (fuzz * random_unit_vector());
        scattered = ray(rec.p, reflected);
        attenuation = albedo;
        return (scattered.direction().dot(rec.normal) > 0);
    }

    Vector3f mix_color(const Vector3f &origin, const Vector3f &new_color)
        const override
    {
        return origin.cwiseProduct(new_color);
    }

private:
    Vector3f albedo;
    double fuzz;
};

#endif
