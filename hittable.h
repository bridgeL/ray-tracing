#ifndef HITTABLE_H
#define HITTABLE_H

#include "bbox.h"

class material;

class hit_record
{
public:
    vec3 p;      // hit point
    vec3 normal; // face normal
    shared_ptr<material> mat;
    double t;
    double u;
    double v;
    bool front_face;

    void set_face_normal(const ray &r, const vec3 &outward_normal)
    {
        // Sets the hit record normal vector.
        // NOTE: the parameter `outward_normal` is assumed to have unit length.

        front_face = r.direction().dot(outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class hittable
{
public:
    virtual ~hittable() = default;

    virtual bool hit(const ray &r, interval ray_t, hit_record &rec) const = 0;

    virtual bbox get_bbox() const = 0;
};

#endif
