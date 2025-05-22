#ifndef VEC3_H
#define VEC3_H

class vec3
{
public:
    double e[3];

    vec3() : e{0, 0, 0} {}
    vec3(double e0, double e1, double e2) : e{e0, e1, e2} {}

    double x() const { return e[0]; }
    double y() const { return e[1]; }
    double z() const { return e[2]; }

    vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }
    double operator[](int i) const { return e[i]; }
    double &operator[](int i) { return e[i]; }

    vec3 &operator+=(const vec3 &v)
    {
        e[0] += v.e[0];
        e[1] += v.e[1];
        e[2] += v.e[2];
        return *this;
    }

    vec3 &operator*=(double t)
    {
        e[0] *= t;
        e[1] *= t;
        e[2] *= t;
        return *this;
    }

    vec3 &operator/=(double t)
    {
        return *this *= 1 / t;
    }

    double length() const
    {
        return std::sqrt(length_squared());
    }

    double length_squared() const
    {
        return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
    }

    bool near_zero() const
    {
        // Return true if the vector is close to zero in all dimensions.
        auto s = 1e-8;
        return (std::fabs(e[0]) < s) && (std::fabs(e[1]) < s) && (std::fabs(e[2]) < s);
    }

    static vec3 random()
    {
        return vec3(random_double(), random_double(), random_double());
    }

    static vec3 random(double min, double max)
    {
        return vec3(random_double(min, max), random_double(min, max), random_double(min, max));
    }

    double dot(const vec3 &v) const
    {
        return e[0] * v.e[0] + e[1] * v.e[1] + e[2] * v.e[2];
    }

    vec3 cross(const vec3 &v) const
    {
        return vec3(e[1] * v.e[2] - e[2] * v.e[1],
                    e[2] * v.e[0] - e[0] * v.e[2],
                    e[0] * v.e[1] - e[1] * v.e[0]);
    }

    vec3 normalized() const
    {
        double len = length();
        return vec3(e[0] / len, e[1] / len, e[2] / len);
    }

    std::string toString() const
    {
        std::ostringstream oss;
        oss << "vec3(" << e[0] << ", " << e[1] << ", " << e[2] << ")";
        return oss.str();
    }
};

std::ostream &operator<<(std::ostream &os, vec3 v)
{
    os << v.toString();
    return os;
}

// Vector Utility Functions

inline vec3 operator+(const vec3 &u, const vec3 &v)
{
    return vec3(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]);
}

inline vec3 operator-(const vec3 &u, const vec3 &v)
{
    return vec3(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]);
}

// cwise Product
inline vec3 operator*(const vec3 &u, const vec3 &v)
{
    return vec3(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]);
}

// scalar
inline vec3 operator*(double t, const vec3 &v)
{
    return vec3(t * v.e[0], t * v.e[1], t * v.e[2]);
}

inline vec3 operator*(const vec3 &v, double t)
{
    return t * v;
}

inline vec3 operator/(const vec3 &v, double t)
{
    return (1 / t) * v;
}

inline vec3 random_unit_vector()
{
    // 方法1：解析变换法（无循环，效率更高）
    double theta = 2 * M_PI * random_double();  // 方位角 [0, 2π)
    double phi = acos(1 - 2 * random_double()); // 极角 [0, π]

    double sin_phi = sin(phi);
    double x = sin_phi * cos(theta);
    double y = sin_phi * sin(theta);
    double z = cos(phi);

    return vec3(x, y, z);
}

inline vec3 reflect(const vec3 &v, const vec3 &n)
{
    return v - 2 * v.dot(n) * n;
}

inline vec3 refract(const vec3 &uv, const vec3 &n, double etai_over_etat)
{
    auto cos_theta = std::fmin((-uv).dot(n), 1.0);
    vec3 r_out_perp = etai_over_etat * (uv + cos_theta * n);
    vec3 r_out_parallel = -std::sqrt(std::fabs(1.0 - r_out_perp.length_squared())) * n;
    return r_out_perp + r_out_parallel;
}

inline vec3 interpolate(const vec3 &weight, const vec3 &p1, const vec3 &p2, const vec3 &p3)
{
    return p1 * weight.e[0] + p2 * weight.e[1] + p3 * weight.e[2];
}

inline double interpolate(const vec3 &weight, const double &v1, const double &v2, const double &v3)
{
    return v1 * weight.e[0] + v2 * weight.e[1] + v3 * weight.e[2];
}

#endif
