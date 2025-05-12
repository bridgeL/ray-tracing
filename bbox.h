#ifndef BBOX_H
#define BBOX_H

#include "interval.h"

class bbox
{
public:
    interval x;
    interval y;
    interval z;

    bbox() {}
    bbox(const interval &x, const interval &y, const interval &z) : x(x.pad(0.001)), y(y.pad(0.001)), z(z.pad(0.001)) {}

    // merge two bounding boxes
    bbox(const bbox &a, const bbox &b) : x(a.x, b.x), y(a.y, b.y), z(a.z, b.z) {}

    int longest_axis() const
    {
        if (x.size() > y.size() && x.size() > z.size())
            return 0;
        if (y.size() > z.size())
            return 1;
        return 2;
    }

    // 新增：射线相交检测
    bool hit(const ray &r, interval ray_t) const
    {
        interval m[3] = {x, y, z};
        for (int i = 0; i < 3; i++)
        {
            double invD = 1.0f / r.direction()[i];
            double t0 = (m[i].min - r.origin()[i]) * invD;
            double t1 = (m[i].max - r.origin()[i]) * invD;
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

    std::string toString() const
    {
        std::ostringstream oss;
        oss << "bbox[\n"
            << "  x: " << x.toString() << "\n"
            << "  y: " << y.toString() << "\n"
            << "  z: " << z.toString() << "\n"
            << "]";
        return oss.str();
    }
};

const bbox bbox::empty = bbox(interval::empty, interval::empty, interval::empty);
const bbox bbox::universe = bbox(interval::universe, interval::universe, interval::universe);

#endif
