#ifndef HITTABLE_H
#define HITTABLE_H

class material;

class hit_record
{
public:
    Vector3f p;      // hit point
    Vector3f normal; // face normal
    shared_ptr<material> mat;
    float t;
    Vector2f texture_coord; // uv coordinate
};

class hittable
{
public:
    virtual ~hittable() = default;

    virtual bool hit(const ray &r, interval ray_t, hit_record &rec) const = 0;

    bbox box;
};

#endif
