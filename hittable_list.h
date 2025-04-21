#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H

#include "hittable.h"
#include "bvh.h"

class hittable_list
{
public:
    std::vector<shared_ptr<hittable>> objects;

    hittable_list() {}
    hittable_list(shared_ptr<hittable> object) { add(object); }

    void clear() { objects.clear(); }

    void add(shared_ptr<hittable> object)
    {
        objects.push_back(object);
    }

    shared_ptr<BVHNode> create_bvh_tree() const
    {
        return std::make_shared<BVHNode>(objects, 0, objects.size());
    }
};

#endif
