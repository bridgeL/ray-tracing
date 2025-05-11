#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "hittable.h"
#include "ray.h"
#include "interval.h"

class vertex
{
public:
    vec3 pos;
    double u;
    double v;
    vec3 normal;

    vertex() {}
    vertex(const vec3 &pos) : pos(pos), u(0), v(0), normal(vec3(0, 0, 0)) {}
    vertex(const vec3 &pos, double u, double v, const vec3 &normal) : pos(pos), u(u), v(v), normal(normal) {}
};

class triangle : public hittable
{
public:
    triangle(
        const vertex &p0,
        const vertex &p1,
        const vertex &p2,
        const vec3 &normal,
        shared_ptr<material> mat) : mat(mat),
                                    normal(normal),
                                    vertices({p0, p1, p2}),
                                    b(calculateBBox(p0, p1, p2)) {}

    triangle(
        const vertex &p0,
        const vertex &p1,
        const vertex &p2,
        shared_ptr<material> mat) : mat(mat),
                                    normal(calculateNormal(p0, p1, p2)),
                                    vertices({p0, p1, p2}),
                                    b(calculateBBox(p0, p1, p2)) {}

    bool hit(const ray &r, interval ray_t, hit_record &rec) const override
    {
        vec3 p1 = vertices[0].pos;
        vec3 p2 = vertices[1].pos;
        vec3 p3 = vertices[2].pos;

        vec3 v1 = p1 - p2;
        vec3 v2 = p1 - p3;
        double d = normal.dot(r.direction());

        // 排除平行情况
        if (abs(d) < 1e-16)
            return false;

        double t = normal.dot(p1 - r.origin()) / d;

        // 判断光线是否已被阻挡
        if (!ray_t.contains(t))
            return false;

        // 获取与平面的交点，判断是否在三角形内
        vec3 p = r.at(t);
        if (!insideTriangle(p))
            return false;

        // 记录相交情况
        rec.t = t;
        rec.p = p;
        rec.set_face_normal(r, normal);
        rec.mat = mat;
        rec.u = interpolate(computeBarycentric(p), vertices[0].u, vertices[1].u, vertices[2].u);
        rec.v = interpolate(computeBarycentric(p), vertices[0].v, vertices[1].v, vertices[2].v);

        return true;
    }

    vec3 computeBarycentric(const vec3 &p) const
    {
        vec3 p1 = vertices[0].pos;
        vec3 p2 = vertices[1].pos;
        vec3 p3 = vertices[2].pos;

        vec3 v1 = p2 - p1;
        vec3 v2 = p3 - p1;
        vec3 vp = p - p1;

        // 计算法向量 n = v1 × v2（用于统一分母）
        vec3 n = v1.cross(v2);
        double denom = n.length_squared(); // 分母 = (v1 × v2) · n = |v1 × v2|^2

        // 计算 u 和 v
        double u = vp.cross(v2).dot(n) / denom;
        double v = v1.cross(vp).dot(n) / denom;
        double w = 1 - u - v;

        return vec3(u, v, w);
    }

    bool insideTriangle(const vec3 &p) const
    {
        // 计算点p到三条边的叉积符号
        vec3 p1 = vertices[0].pos;
        vec3 p2 = vertices[1].pos;
        vec3 p3 = vertices[2].pos;

        vec3 c1 = (p - p1).cross(p2 - p1);
        vec3 c2 = (p - p2).cross(p3 - p2);
        vec3 c3 = (p - p3).cross(p1 - p3);

        // 检查符号一致性（与法向量点积）
        double dot1 = c1.dot(normal);
        double dot2 = c2.dot(normal);
        double dot3 = c3.dot(normal);

        // 如果所有点积同号（包括等于0的情况，表示在边上）
        return (dot1 >= 0 && dot2 >= 0 && dot3 >= 0) || (dot1 <= 0 && dot2 <= 0 && dot3 <= 0);
    }

    bbox get_bbox() const override
    {
        return b;
    }

private:
    vertex vertices[3];
    vec3 normal;
    shared_ptr<material> mat;
    bbox b;

    // 提取包围盒计算逻辑
    bbox calculateBBox(const vertex &p0, const vertex &p1, const vertex &p2)
    {
        double xs[3] = {p0.pos.x(), p1.pos.x(), p2.pos.x()};
        double ys[3] = {p0.pos.y(), p1.pos.y(), p2.pos.y()};
        double zs[3] = {p0.pos.z(), p1.pos.z(), p2.pos.z()};

        return bbox(
            interval(std::min({xs[0], xs[1], xs[2]}), std::max({xs[0], xs[1], xs[2]})),
            interval(std::min({ys[0], ys[1], ys[2]}), std::max({ys[0], ys[1], ys[2]})),
            interval(std::min({zs[0], zs[1], zs[2]}), std::max({zs[0], zs[1], zs[2]})));
    }

    // 提取法向量计算逻辑
    vec3 calculateNormal(const vertex &p0, const vertex &p1, const vertex &p2)
    {
        return (p1.pos - p0.pos).cross(p2.pos - p0.pos).normalized();
    }
};

#endif
