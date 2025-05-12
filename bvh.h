#ifndef BVH_H
#define BVH_H

// BVH节点类型
enum class BVHNodeType
{
    LEAF,
    INTERNAL
};

class BVHNode : public hittable
{
public:
    BVHNodeType type;
    bbox b;
    shared_ptr<BVHNode> left;
    shared_ptr<BVHNode> right;
    std::vector<shared_ptr<hittable>> objects; // 中间节点不存值，只有叶子节点会存

    // 递归构建函数
    BVHNode(std::vector<shared_ptr<hittable>> &src_objects,
            size_t start, size_t end,
            int max_leaf_size)
    {
        // 1. 计算当前节点包围盒
        b = bbox::empty;
        for (size_t i = start; i < end; i++)
            b = bbox(b, src_objects[i]->get_bbox());

        // 2. 判断是否终止递归
        size_t object_count = end - start;
        if (object_count <= max_leaf_size)
        {
            type = BVHNodeType::LEAF;
            objects.assign(src_objects.begin() + start,
                           src_objects.begin() + end);
            return;
        }

        // 3. 选择最佳分割轴和位置
        int axis = b.longest_axis();
        auto comparator = (axis == 0)   ? box_x_compare
                          : (axis == 1) ? box_y_compare
                                        : box_z_compare;

        size_t object_span = end - start;

        // 4. 分割对象
        std::sort(src_objects.begin() + start,
                  src_objects.begin() + end,
                  comparator);

        // 简单平分
        auto mid = start + object_span / 2;

        type = BVHNodeType::INTERNAL;
        left = make_shared<BVHNode>(src_objects, start, mid, max_leaf_size);
        right = make_shared<BVHNode>(src_objects, mid, end, max_leaf_size);
    }

    bool hit(const ray &r, interval ray_t, hit_record &rec) const override
    {
        if (!b.hit(r, ray_t))
            return false;

        if (type == BVHNodeType::LEAF)
        {
            bool hit_anything = false;
            for (const auto &object : objects)
            {
                if (object->hit(r, ray_t, rec))
                {
                    hit_anything = true;
                    ray_t.max = rec.t;
                }
            }
            return hit_anything;
        }

        rec.depth += 1;
        bool hit_left = left->hit(r, ray_t, rec);
        bool hit_right = right->hit(r, interval(ray_t.min, hit_left ? rec.t : ray_t.max), rec);

        return hit_left || hit_right;
    }

    bbox get_bbox() const override
    {
        return b;
    }

private:
    // 比较函数
    static bool box_x_compare(const shared_ptr<hittable> a,
                              const shared_ptr<hittable> b)
    {
        return a->get_bbox().x.min < b->get_bbox().x.min;
    }

    static bool box_y_compare(const shared_ptr<hittable> a,
                              const shared_ptr<hittable> b)
    {
        return a->get_bbox().y.min < b->get_bbox().y.min;
    }

    static bool box_z_compare(const shared_ptr<hittable> a,
                              const shared_ptr<hittable> b)
    {
        return a->get_bbox().z.min < b->get_bbox().z.min;
    }
};

vec3 convert_int_to_color(int v, int n)
{
    float r = interval(0, n).clamp(v) * 2 / n;
    // while value growing: blue -> green -> red
    return r < 1.0 ? vec3(0, r, 1 - r) : vec3(r - 1, 2 - r, 0);
}

#endif
