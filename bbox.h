#ifndef BBOX_H
#define BBOX_H

class bbox
{
public:
    Vector3f min;
    Vector3f max;

    // Default bbox is empty
    bbox() : min(Vector3f(infinity, infinity, infinity)), max(Vector3f(-infinity, -infinity, -infinity)) {}

    bbox(Vector3f min, Vector3f max) : min(min), max(max) {}

    Vector3f center() const
    {
        return 0.5f * (min + max);
    }

    void expand_box(const bbox &b)
    {
        for (size_t i = 0; i < 3; i++)
        {
            if (b.max[i] > max[i])
                max[i] = b.max[i];
            if (b.min[i] < min[i])
                min[i] = b.min[i];
        }
    }

    float surface_area() const
    {
        Vector3f d = max - min;
        return 2.0f * (d.x() * d.y() + d.x() * d.z() + d.y() * d.z());
    }

    int longest_axis() const
    {
        Vector3f d = max - min;
        if (d.x() > d.y() && d.x() > d.z())
            return 0;
        return (d.y() > d.z()) ? 1 : 2;
    }

    // 新增：射线相交检测
    bool hit(const ray &r, interval ray_t) const
    {
        for (int i = 0; i < 3; i++)
        {
            float invD = 1.0f / r.direction()[i];
            float t0 = (min[i] - r.origin()[i]) * invD;
            float t1 = (max[i] - r.origin()[i]) * invD;
            if (invD < 0.0f)
                std::swap(t0, t1);
            ray_t.min = t0 > ray_t.min ? t0 : ray_t.min;
            ray_t.max = t1 < ray_t.max ? t1 : ray_t.max;
            if (ray_t.max <= ray_t.min)
                return false;
        }
        return true;
    }

    static const bbox empty, universe;
};

const bbox bbox::empty = bbox(Vector3f(infinity, infinity, infinity), Vector3f(-infinity, -infinity, -infinity));
const bbox bbox::universe = bbox(Vector3f(-infinity, -infinity, -infinity), Vector3f(infinity, infinity, infinity));

#endif
