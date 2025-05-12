#ifndef INTERVAL_H
#define INTERVAL_H

class interval
{
public:
    double min, max;

    interval() {}
    interval(double min, double max) : min(min), max(max) {}

    // merge two intervals
    interval(const interval &a, const interval &b)
    {
        min = a.min < b.min ? a.min : b.min;
        max = a.max > b.max ? a.max : b.max;
    }

    double centroid() const
    {
        return (min + max) / 2;
    }

    double size() const
    {
        return max - min;
    }

    bool contains(double x) const
    {
        return min <= x && x <= max;
    }

    bool surrounds(double x) const
    {
        return min < x && x < max;
    }

    interval pad(double s) const
    {
        return interval(min - s, max + s);
    }

    double clamp(double x) const
    {
        if (x < min)
            return min;
        if (x > max)
            return max;
        return x;
    }

    static const interval empty, universe;

    std::string toString() const
    {
        return "[" + std::to_string(min) + ", " + std::to_string(max) + "]";
    }
};

const interval interval::empty = interval(+infinity, -infinity);
const interval interval::universe = interval(-infinity, +infinity);

#endif
