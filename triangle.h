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

        box = create_bbox();
    }

    triangle(const Vector3f &p0, const Vector3f &p1, const Vector3f &p2,
             const Vector3f &normal,
             shared_ptr<material> mat)
        : mat(mat), normal(normal)
    {
        vertices[0] = p0;
        vertices[1] = p1;
        vertices[2] = p2;

        box = create_bbox();
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

        box = create_bbox();
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

        box = create_bbox();
    }

    bbox create_bbox()
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
    
        // 计算向量
        Vector3f v0 = p2 - p1;
        Vector3f v1 = p3 - p1;
        Vector3f v2 = p  - p1;
    
        // 计算叉积面积
        float d00 = v0.dot(v0);
        float d01 = v0.dot(v1);
        float d11 = v1.dot(v1);
        float d20 = v2.dot(v0);
        float d21 = v2.dot(v1);
    
        float denom = d00 * d11 - d01 * d01;
    
        // 防止除零
        if (denom == 0.0f)
            return Vector3f(-1.0f, -1.0f, -1.0f);  // 或者根据你的需求返回特殊值
    
        float v = (d11 * d20 - d01 * d21) / denom;
        float w = (d00 * d21 - d01 * d20) / denom;
        float u = 1.0f - v - w;
    
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

// private:
    Vector3f vertices[3];
    Vector2f texture_coords[3];
    Vector3f normal;
    shared_ptr<material> mat;
};

#endif
