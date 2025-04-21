#ifndef BVH_H
#define BVH_H

// BVH节点类型
enum class BVHNodeType
{
    LEAF,
    INTERNAL
};

class BVHNode
{
public:
    BVHNodeType type;
    bbox box;
    shared_ptr<BVHNode> left;
    shared_ptr<BVHNode> right;
    std::vector<shared_ptr<hittable>> objects; // 中间节点不存值，只有叶子节点会存

    // 叶子节点构造函数
    BVHNode(std::vector<std::shared_ptr<hittable>> objs)
        : type(BVHNodeType::LEAF), objects(std::move(objs))
    {
        // 计算叶子节点的包围盒
        box = bbox::empty;
        for (const auto &obj : objects)
        {
            box.expand_box(obj->box);
        }
    }

    // 递归构建函数
    BVHNode(std::vector<std::shared_ptr<hittable>> src_objects,
            size_t start, size_t end,
            int max_leaf_size = 5)
    {
        // 1. 计算当前节点包围盒
        box = bbox::empty;
        for (size_t i = start; i < end; i++)
        {
            box.expand_box(src_objects[i]->box);
        }

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
        int axis = box.longest_axis();
        auto comparator = (axis == 0)   ? box_x_compare
                          : (axis == 1) ? box_y_compare
                                        : box_z_compare;

        size_t object_span = end - start;

        // 4. 分割对象
        if (object_span <= 2)
        {
            // 简单平分
            std::sort(src_objects.begin() + start,
                      src_objects.begin() + end,
                      comparator);
            auto mid = start + object_span / 2;
            left = std::make_shared<BVHNode>(src_objects, start, mid);
            right = std::make_shared<BVHNode>(src_objects, mid, end);
        }
        else
        {
            // SAH优化分割
            auto [split_axis, split_pos] = find_best_split(src_objects, start, end);
            auto mid = partition_objects(src_objects, start, end, split_axis, split_pos);

            left = std::make_shared<BVHNode>(src_objects, start, mid);
            right = std::make_shared<BVHNode>(src_objects, mid, end);
        }

        type = BVHNodeType::INTERNAL;
    }

    bool hit(const ray &r, interval ray_t, hit_record &rec) const
    {
        if (!box.hit(r, ray_t))
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

        bool hit_left = left->hit(r, ray_t, rec);
        bool hit_right = right->hit(r, interval(ray_t.min, hit_left ? rec.t : ray_t.max), rec);

        return hit_left || hit_right;
    }

private:
    // 比较函数
    static bool box_x_compare(const std::shared_ptr<hittable> a,
                              const std::shared_ptr<hittable> b)
    {
        return a->box.min.x() < b->box.min.x();
    }

    static bool box_y_compare(const std::shared_ptr<hittable> a,
                              const std::shared_ptr<hittable> b)
    {
        return a->box.min.y() < b->box.min.y();
    }

    static bool box_z_compare(const std::shared_ptr<hittable> a,
                              const std::shared_ptr<hittable> b)
    {
        return a->box.min.z() < b->box.min.z();
    }

    // SAH分割优化
    std::pair<int, float> find_best_split(
        const std::vector<std::shared_ptr<hittable>> &objects,
        size_t start, size_t end)
    {
        constexpr int BINS = 12; // 通常12-32个bin足够
        constexpr float traversal_cost = 1.0f;
        constexpr float intersection_cost = 1.2f;

        float best_cost = std::numeric_limits<float>::infinity();
        int best_axis = 0;
        float best_split = 0.0f;

        // 计算全局包围盒
        bbox global_box;
        for (size_t i = start; i < end; ++i)
        {
            global_box.expand_box(objects[i]->box);
        }

        // 遍历三个轴
        for (int axis = 0; axis < 3; ++axis)
        {
            // 跳过过小的轴
            if (global_box.max[axis] - global_box.min[axis] < 0.001f)
                continue;

            // 初始化bins
            struct Bin
            {
                bbox bounds;
                int count = 0;
            };
            std::vector<Bin> bins(BINS);

            // 填充bins
            float scale = BINS / (global_box.max[axis] - global_box.min[axis]);
            for (size_t i = start; i < end; ++i)
            {
                int bin_idx = std::min(BINS - 1,
                                       static_cast<int>((objects[i]->box.center()[axis] - global_box.min[axis]) * scale));
                bins[bin_idx].bounds.expand_box(objects[i]->box);
                bins[bin_idx].count++;
            }

            // 计算前缀和后缀信息
            std::vector<bbox> left_accum(BINS);
            std::vector<int> left_count(BINS, 0);
            bbox left_box;
            int left_total = 0;

            for (int i = 0; i < BINS; ++i)
            {
                left_box.expand_box(bins[i].bounds);
                left_accum[i] = left_box;
                left_total += bins[i].count;
                left_count[i] = left_total;
            }

            std::vector<bbox> right_accum(BINS);
            std::vector<int> right_count(BINS, 0);
            bbox right_box;
            int right_total = 0;

            for (int i = BINS - 1; i >= 0; --i)
            {
                right_box.expand_box(bins[i].bounds);
                right_accum[i] = right_box;
                right_total += bins[i].count;
                right_count[i] = right_total;
            }

            // 评估分割位置
            for (int i = 1; i < BINS; ++i)
            {
                float cost = traversal_cost +
                             (left_accum[i - 1].surface_area() * left_count[i - 1] +
                              right_accum[i].surface_area() * right_count[i]) *
                                 intersection_cost;

                if (cost < best_cost)
                {
                    best_cost = cost;
                    best_axis = axis;
                    best_split = global_box.min[axis] + (i / static_cast<float>(BINS)) *
                                                            (global_box.max[axis] - global_box.min[axis]);
                }
            }
        }

        return {best_axis, best_split};
    }

    size_t partition_objects(
        std::vector<std::shared_ptr<hittable>> &objects,
        size_t start, size_t end,
        int axis, float split_pos)
    {
        auto mid = std::partition(
            objects.begin() + start,
            objects.begin() + end,
            [axis, split_pos](const auto &obj)
            {
                return obj->box.center()[axis] < split_pos;
            });
        return mid - objects.begin();
    }
};

#endif
