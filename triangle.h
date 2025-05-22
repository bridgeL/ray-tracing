#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "hittable.h"
#include "ray.h"
#include "interval.h"

class vertex
{
public:
    vec3 pos;    // Vertex position
    double u;    // Texture coordinate u
    double v;    // Texture coordinate v
    vec3 normal; // Vertex normal

    vertex() {}
    vertex(const vec3 &pos) : pos(pos), u(-1), v(-1), normal(vec3(0, 0, 0)) {}
    vertex(const vec3 &pos, double u, double v, const vec3 &normal) : pos(pos), u(u), v(v), normal(normal) {}

    std::string toString() const
    {
        std::ostringstream oss;
        oss << "Vertex {\n"
            << "  pos: (" << pos.x() << ", " << pos.y() << ", " << pos.z() << ")\n"
            << "  uv: (" << u << ", " << v << ")\n"
            << "  normal: (" << normal.x() << ", " << normal.y() << ", " << normal.z() << ")\n"
            << "}";
        return oss.str();
    }
};

std::ostream &operator<<(std::ostream &os, vertex v)
{
    os << v.toString();
    return os;
}

class triangle : public hittable
{
public:
    vertex vertices[3]; // Triangle vertices
    vec3 normal;        // Face normal
    shared_ptr<material> mat; // Material
    bbox b;             // Bounding box

    triangle(
        const vertex &p0,
        const vertex &p1,
        const vertex &p2,
        const vec3 &normal,
        shared_ptr<material> mat) : mat(mat),
                                    normal(normal),
                                    vertices({p0, p1, p2})
    {
        calculateBBox();
    }

    triangle(
        const vertex &p0,
        const vertex &p1,
        const vertex &p2,
        shared_ptr<material> mat) : mat(mat),
                                    vertices({p0, p1, p2})
    {
        calculateBBox();
        calculateNormal();
    }

    bool hit(const ray &r, interval ray_t, hit_record &rec) const override
    {
        vec3 p1 = vertices[0].pos;
        vec3 p2 = vertices[1].pos;
        vec3 p3 = vertices[2].pos;

        vec3 v1 = p1 - p2;
        vec3 v2 = p1 - p3;
        double d = normal.dot(r.direction());

        // Exclude parallel cases (ray parallel to triangle plane)
        if (abs(d) < 1e-16)
            return false;

        double t = normal.dot(p1 - r.origin()) / d;

        // Check if ray is already blocked
        if (!ray_t.contains(t))
            return false;

        // Get intersection point with plane and check if inside triangle
        vec3 p = r.at(t);
        if (!insideTriangle(p))
            return false;

        // Record intersection details
        rec.t = t;
        rec.p = p;
        rec.set_face_normal(r, normal);
        rec.mat = mat;
        rec.u = interpolate(computeBarycentric(p), vertices[0].u, vertices[1].u, vertices[2].u);
        rec.v = interpolate(computeBarycentric(p), vertices[0].v, vertices[1].v, vertices[2].v);

        return true;
    }

    bbox get_bbox() const override
    {
        return b;
    }

    std::string toString() const
    {
        std::ostringstream oss;
        oss << "Triangle {\n";
        for (int i = 0; i < 3; ++i)
        {
            oss << "  v" << i << ": " << vertices[i] << "\n";
        }
        oss << "  face_normal: (" << normal.x() << ", " << normal.y() << ", " << normal.z() << ")\n"
            << "}";
        return oss.str();
    }

    // Calculate bounding box
    void calculateBBox()
    {
        const vertex &p0 = vertices[0];
        const vertex &p1 = vertices[1];
        const vertex &p2 = vertices[2];

        double xs[3] = {p0.pos.x(), p1.pos.x(), p2.pos.x()};
        double ys[3] = {p0.pos.y(), p1.pos.y(), p2.pos.y()};
        double zs[3] = {p0.pos.z(), p1.pos.z(), p2.pos.z()};

        b = bbox(
            interval(std::min({xs[0], xs[1], xs[2]}), std::max({xs[0], xs[1], xs[2]})).pad(1e-8),
            interval(std::min({ys[0], ys[1], ys[2]}), std::max({ys[0], ys[1], ys[2]})).pad(1e-8),
            interval(std::min({zs[0], zs[1], zs[2]}), std::max({zs[0], zs[1], zs[2]})).pad(1e-8));
    }

    // Calculate face normal
    void calculateNormal()
    {
        const vertex &p0 = vertices[0];
        const vertex &p1 = vertices[1];
        const vertex &p2 = vertices[2];

        normal = (p1.pos - p0.pos).cross(p2.pos - p0.pos).normalized();
    }

private:
    vec3 computeBarycentric(const vec3 &p) const
    {
        vec3 p1 = vertices[0].pos;
        vec3 p2 = vertices[1].pos;
        vec3 p3 = vertices[2].pos;

        // Compute vectors
        vec3 v0 = p2 - p1;
        vec3 v1 = p3 - p1;
        vec3 v2 = p - p1;

        // Compute cross product areas
        float d00 = v0.dot(v0);
        float d01 = v0.dot(v1);
        float d11 = v1.dot(v1);
        float d20 = v2.dot(v0);
        float d21 = v2.dot(v1);

        float denom = d00 * d11 - d01 * d01;

        // Prevent division by zero
        if (denom == 0.0)
            return vec3(-1, -1, -1); // Return special value for invalid case

        float v = (d11 * d20 - d01 * d21) / denom;
        float w = (d00 * d21 - d01 * d20) / denom;
        float u = 1 - v - w;

        return vec3(u, v, w);
    }

    bool insideTriangle(const vec3 &p) const
    {
        // Compute cross products between point p and triangle edges
        vec3 p1 = vertices[0].pos;
        vec3 p2 = vertices[1].pos;
        vec3 p3 = vertices[2].pos;

        vec3 c1 = (p - p1).cross(p2 - p1);
        vec3 c2 = (p - p2).cross(p3 - p2);
        vec3 c3 = (p - p3).cross(p1 - p3);

        // Check sign consistency (dot product with normal)
        double dot1 = c1.dot(normal);
        double dot2 = c2.dot(normal);
        double dot3 = c3.dot(normal);

        // If all dot products have same sign (including zero for edge cases)
        return (dot1 >= 0 && dot2 >= 0 && dot3 >= 0) || (dot1 <= 0 && dot2 <= 0 && dot3 <= 0);
    }
};

std::ostream &operator<<(std::ostream &os, triangle v)
{
    os << v.toString();
    return os;
}

#endif
