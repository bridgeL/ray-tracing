#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H

#include "hittable.h"
#include "bvh.h"

class hittable_list : public hittable
{
public:
    std::vector<shared_ptr<hittable>> objects;
    shared_ptr<BVHNode> bvh_tree;
    bbox b;

    hittable_list() : b(bbox::empty) {}

    void clear() { objects.clear(); }

    void add(shared_ptr<hittable> object)
    {
        objects.push_back(object);
        b = bbox(b, object->get_bbox());
    }

    void create_bvh_tree(int max_leaf_size = 5, BVHSplitMethod split_method = BVHSplitMethod::MIDDLE)
    {
        bvh_tree = make_shared<BVHNode>(objects, 0, objects.size(), max_leaf_size, split_method);
    }

    bool hit(const ray &r, interval ray_t, hit_record &rec) const override
    {
        if (!bvh_tree)
        {
            std::cerr << "Error: BVH tree not created. Call create_bvh_tree() first." << std::endl;
            return false;
        }

        return bvh_tree->hit(r, ray_t, rec);
    }

    bbox get_bbox() const override
    {
        return b;
    }
};

#endif
