#ifndef RAY_H
#define RAY_H

class ray
{
public:
    ray() {}

    ray(const Vector3f &origin, const Vector3f &direction) : orig(origin), dir(direction) {}

    const Vector3f &origin() const { return orig; }
    const Vector3f &direction() const { return dir; }

    Vector3f at(double t) const
    {
        return orig + t * dir;
    }

private:
    Vector3f orig;
    Vector3f dir;
};

#endif
