#ifndef MATERIAL_H
#define MATERIAL_H

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
};

class lambertian : public material
{
public:
    lambertian(const vec3 &albedo) : tex(make_shared<solid_color>(albedo)) {}
    lambertian(shared_ptr<texture> tex) : tex(tex) {}

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

class bvh_visualization_mat : public material
{
public:
    static int h;

    bvh_visualization_mat(const vec3 &albedo) : tex(make_shared<solid_color>(albedo)) {}
    bvh_visualization_mat(shared_ptr<texture> tex) : tex(tex) {}

    bool scatter(const ray &r_in, const hit_record &rec, vec3 &attenuation, ray &scattered)
        const override
    {
        vec3 scatter_direction = rec.normal + random_unit_vector();

        // Catch degenerate scatter direction
        if (scatter_direction.near_zero())
            scatter_direction = rec.normal;

        scattered = ray(rec.p, scatter_direction);

        // show hit depth at bvh tree
        // 如果画面偏红，说明bvh tree的节点深度普遍超过 h/2
        // ln(949496)≈13.763
        attenuation = convert_int_to_color(rec.depth, h);
        return true;
    }

    static void set_h(int _h)
    {
        h = _h;
    }

private:
    shared_ptr<texture> tex;
};

int bvh_visualization_mat::h = 100;

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
