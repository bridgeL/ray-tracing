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
    bbox(const interval &x, const interval &y, const interval &z) : x(x), y(y), z(z) {}

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

    interval get(int axis) const
    {
        return axis == 0   ? x
               : axis == 1 ? y
                           : z;
    }

    // 计算包围盒的表面积
    double surface_area() const
    {
        if (x.size() == 0 || y.size() == 0 || z.size() == 0)
            return 0.0; // 空包围盒的表面积为0

        double dx = x.size();
        double dy = y.size();
        double dz = z.size();

        // 表面积 = 2*(dx*dy + dx*dz + dy*dz)
        return 2.0 * (dx * dy + dx * dz + dy * dz);
    }

    // 射线相交检测
    bool hit(const ray &r, interval ray_t) const
    {
        for (int i = 0; i < 3; i++)
        {
            double invD = 1.0 / r.direction()[i];
            double t0 = (get(i).min - r.origin()[i]) * invD;
            double t1 = (get(i).max - r.origin()[i]) * invD;
            if (invD < 0.0)
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
            << "  x: " << x << "\n"
            << "  y: " << y << "\n"
            << "  z: " << z << "\n"
            << "]";
        return oss.str();
    }
};

std::ostream &operator<<(std::ostream &os, bbox v)
{
    os << v.toString();
    return os;
}

const bbox bbox::empty = bbox(interval::empty, interval::empty, interval::empty);
const bbox bbox::universe = bbox(interval::universe, interval::universe, interval::universe);

#endif
