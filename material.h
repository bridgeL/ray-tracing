#ifndef MATERIAL_H
#define MATERIAL_H

class material
{
public:
    virtual ~material() = default;

    virtual bool scatter(
        const ray &r_in, const hit_record &rec, vec3 &attenuation, ray &scattered) const
    {
        return false;
    }
};

class lambertian : public material
{
public:
    lambertian(const vec3 &albedo) : albedo(albedo) {}

    bool scatter(const ray &r_in, const hit_record &rec, vec3 &attenuation, ray &scattered)
        const override
    {
        vec3 scatter_direction = rec.normal + random_unit_vector();

        // Catch degenerate scatter direction
        if (scatter_direction.near_zero())
            scatter_direction = rec.normal;

        scattered = ray(rec.p, scatter_direction);
        attenuation = albedo;
        return true;
    }

private:
    vec3 albedo;
};

class textured_lambertian : public material
{
public:
    textured_lambertian(cv::Mat &image_data) : image_data(image_data) {}

    bool scatter(const ray &r_in, const hit_record &rec, vec3 &attenuation, ray &scattered)
        const override
    {
        vec3 scatter_direction = rec.normal + random_unit_vector();

        // Catch degenerate scatter direction
        if (scatter_direction.near_zero())
            scatter_direction = rec.normal;

        scattered = ray(rec.p, scatter_direction);

        double u = rec.u;
        double v = rec.v;
        attenuation = get_color(u, v);
        return true;
    }

    vec3 get_color(double u, double v) const
    {
        auto u_img = u * image_data.cols;       // width
        auto v_img = (1 - v) * image_data.rows; // height
        auto color = image_data.at<cv::Vec3b>(v_img, u_img);
        return vec3(color[0], color[1], color[2]);
    }

private:
    cv::Mat image_data;
};

class metal : public material
{
public:
    metal(const vec3 &albedo, double fuzz) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}

    bool scatter(const ray &r_in, const hit_record &rec, vec3 &attenuation, ray &scattered)
        const override
    {
        vec3 reflected = reflect(r_in.direction(), rec.normal);
        reflected = reflected.normalized() + (fuzz * random_unit_vector());
        scattered = ray(rec.p, reflected);
        attenuation = albedo;
        return (scattered.direction().dot(rec.normal) > 0);
    }

private:
    vec3 albedo;
    double fuzz;
};

#endif
