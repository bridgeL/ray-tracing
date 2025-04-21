#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"

class sphere : public hittable
{
public:
    sphere(const Vector3f &center, float radius, shared_ptr<material> mat)
        : center(center), radius(std::fmax(0, radius)), mat(mat) {}

    bool hit(const ray &r, interval ray_t, hit_record &rec) const override
    {
        Vector3f oc = center - r.origin();
        auto a = r.direction().squaredNorm();
        auto h = r.direction().dot(oc);
        auto c = oc.squaredNorm() - radius * radius;

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
        rec.normal = (rec.p - center) / radius;
        rec.mat = mat;

        return true;
    }

private:
    Vector3f center;
    float radius;
    shared_ptr<material> mat;
};

#endif
