#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"

class sphere : public hittable
{
public:
    shared_ptr<material> mat;

    sphere(const vec3 &center, double radius, shared_ptr<material> mat)
        : center(center), radius(std::fmax(0, radius)), mat(mat)
    {
        b = bbox(
            interval(center.x() - radius, center.x() + radius),
            interval(center.y() - radius, center.y() + radius),
            interval(center.z() - radius, center.z() + radius));
    }

    bool hit(const ray &r, interval ray_t, hit_record &rec) const override
    {
        vec3 oc = center - r.origin();
        auto a = r.direction().length_squared();
        auto h = r.direction().dot(oc);
        auto c = oc.length_squared() - radius * radius;

        auto discriminant = h * h - a * c;
        if (discriminant < 0)
            return false;

        auto sqrtd = std::sqrt(discriminant);

        // Find the nearest root that lies in the acceptable range.
        auto root = (h - sqrtd) / a;
        if (!ray_t.surrounds(root))
        {
            root = (h + sqrtd) / a;
            if (!ray_t.surrounds(root))
                return false;
        }

        rec.t = root;
        rec.p = r.at(rec.t);
        vec3 outward_normal = (rec.p - center) / radius;
        rec.set_face_normal(r, outward_normal);

        // TODO:
        rec.u = 0;
        rec.v = 0;

        rec.mat = mat;

        return true;
    }

    bbox get_bbox() const override
    {
        return b;
    }

private:
    vec3 center;
    double radius;
    bbox b;
};

#endif
