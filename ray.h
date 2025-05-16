#ifndef RAY_H
#define RAY_H

class ray
{
public:
    ray() {}

    ray(const vec3 &origin, const vec3 &direction) : orig(origin), dir(direction) {}

    const vec3 &origin() const { return orig; }
    const vec3 &direction() const { return dir; }

    vec3 at(double t) const
    {
        return orig + t * dir;
    }

    std::string toString() const
    {
        std::ostringstream oss;
        oss << "ray(origin: " << orig << ", dir: " << dir << ")";
        return oss.str();
    }

private:
    vec3 orig;
    vec3 dir;
};

std::ostream &operator<<(std::ostream &os, ray v)
{
    os << v.toString();
    return os;
}

#endif
