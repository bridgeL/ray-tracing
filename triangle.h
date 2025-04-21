#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "hittable.h"
#include "ray.h"
#include "interval.h"

class triangle : public hittable
{
public:
    triangle(const Vector3f &p0, const Vector3f &p1, const Vector3f &p2,
             shared_ptr<material> mat)
        : mat(mat)
    {
        vertices[0] = p0;
        vertices[1] = p1;
        vertices[2] = p2;

        // Compute normal once for the triangle
        normal = (p1 - p0).cross(p2 - p0).normalized();
    }

    triangle(const Vector3f &p0, const Vector3f &p1, const Vector3f &p2,
             const Vector3f &normal,
             shared_ptr<material> mat)
        : mat(mat), normal(normal)
    {
        vertices[0] = p0;
        vertices[1] = p1;
        vertices[2] = p2;
    }

    triangle(const Vector3f &p0, const Vector3f &p1, const Vector3f &p2,
             const Vector2f &t0, const Vector2f &t1, const Vector2f &t2,
             shared_ptr<material> mat)
        : mat(mat)
    {
        vertices[0] = p0;
        vertices[1] = p1;
        vertices[2] = p2;

        texture_coords[0] = t0;
        texture_coords[1] = t1;
        texture_coords[2] = t2;

        // Compute normal once for the triangle
        normal = (p1 - p0).cross(p2 - p0).normalized();
    }

    triangle(const Vector3f &p0, const Vector3f &p1, const Vector3f &p2,
             const Vector2f &t0, const Vector2f &t1, const Vector2f &t2,
             const Vector3f &normal,
             shared_ptr<material> mat)
        : mat(mat), normal(normal)
    {
        vertices[0] = p0;
        vertices[1] = p1;
        vertices[2] = p2;

        texture_coords[0] = t0;
        texture_coords[1] = t1;
        texture_coords[2] = t2;
    }

    bbox get_bbox() const override
    {
        return bbox(
            Vector3f(
                std::min({vertices[0][0], vertices[1][0], vertices[2][0]}),
                std::min({vertices[0][1], vertices[1][1], vertices[2][1]}),
                std::min({vertices[0][2], vertices[1][2], vertices[2][2]})),
            Vector3f(
                std::max({vertices[0][0], vertices[1][0], vertices[2][0]}),
                std::max({vertices[0][1], vertices[1][1], vertices[2][1]}),
                std::max({vertices[0][2], vertices[1][2], vertices[2][2]})));
    }

    bool hit(const ray &r, interval ray_t, hit_record &rec) const override
    {
        Vector3f p1 = vertices[0];
        Vector3f p2 = vertices[1];
        Vector3f p3 = vertices[2];

        Vector3f v1 = p1 - p2;
        Vector3f v2 = p1 - p3;
        float d = normal.dot(r.direction());
        if (abs(d) < 1e-16)
            return false;

        float t = normal.dot(p1 - r.origin()) / d;
        if (t < 0)
            return false;

        // 获取与平面的交点
        Vector3f p = r.at(t);

        // 获取与交点的距离
        if (!ray_t.contains(t))
            return false;

        // 判断是否在三角形内
        if (!insideTriangle(p))
            return false;

        // 记录相交情况
        rec.t = t;
        rec.p = p;
        rec.normal = normal;
        rec.mat = mat;
        rec.texture_coord = interpolate(computeBarycentric(p), texture_coords[0], texture_coords[1], texture_coords[2]);

        return true;
    }

    Vector3f computeBarycentric(const Vector3f &p) const
    {
        Vector3f p1 = vertices[0];
        Vector3f p2 = vertices[1];
        Vector3f p3 = vertices[2];

        Vector3f v1 = p2 - p1;
        Vector3f v2 = p3 - p1;
        Vector3f vp = p - p1;

        // 计算法向量 n = v1 × v2（用于统一分母）
        Vector3f n = v1.cross(v2);
        float denom = n.squaredNorm(); // 分母 = (v1 × v2) · n = |v1 × v2|^2

        // 计算 u 和 v
        float u = vp.cross(v2).dot(n) / denom;
        float v = v1.cross(vp).dot(n) / denom;
        float w = 1 - u - v;

        return Vector3f(u, v, w);
    }

    bool insideTriangle(const Vector3f &p) const
    {
        // 计算点p到三条边的叉积符号
        Vector3f p1 = vertices[0];
        Vector3f p2 = vertices[1];
        Vector3f p3 = vertices[2];

        Vector3f c1 = (p - p1).cross(p2 - p1);
        Vector3f c2 = (p - p2).cross(p3 - p2);
        Vector3f c3 = (p - p3).cross(p1 - p3);

        // 检查符号一致性（与法向量点积）
        float dot1 = c1.dot(normal);
        float dot2 = c2.dot(normal);
        float dot3 = c3.dot(normal);

        // 如果所有点积同号（包括等于0的情况，表示在边上）
        return (dot1 >= 0 && dot2 >= 0 && dot3 >= 0) || (dot1 <= 0 && dot2 <= 0 && dot3 <= 0);
    }

private:
    Vector3f vertices[3];
    Vector2f texture_coords[3];
    Vector3f normal;
    shared_ptr<material> mat;
};

#endif
