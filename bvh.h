#ifndef BVH_H
#define BVH_H

// BVH 分割策略
enum class BVHSplitMethod
{
    MIDDLE, // 中位数分割
    SAH     // SAH 优化分割
};

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
    int depth;
    std::string path;

    // 递归构建函数
    BVHNode(std::vector<shared_ptr<hittable>> &src_objects,
            size_t start, size_t end,
            int max_leaf_size,
            BVHSplitMethod split_method = BVHSplitMethod::SAH, // 默认使用SAH
            int depth = 0, std::string path = "") : depth(depth), path(path)
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

        // 3. 根据策略选择分割方法
        size_t split_pos = start;
        int split_axis = b.longest_axis(); // 默认使用最长轴

        if (split_method == BVHSplitMethod::SAH)
        {
            // 尝试SAH分割，如果失败则使用中位数
            if (!try_sah_split(src_objects, start, end, split_axis, split_pos))
            {
                middle_split(src_objects, start, end, split_axis, split_pos);
            }
        }
        else
        {
            // 始终使用中位数分割
            middle_split(src_objects, start, end, split_axis, split_pos);
        }

        // 4. 递归构建子节点（保持相同的分割策略）
        type = BVHNodeType::INTERNAL;
        left = make_shared<BVHNode>(src_objects, start, split_pos,
                                    max_leaf_size, split_method,
                                    depth + 1, path + "0");
        right = make_shared<BVHNode>(src_objects, split_pos, end,
                                     max_leaf_size, split_method,
                                     depth + 1, path + "1");
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
                    rec.bvh_depth = depth;
                    rec.bvh_path = path;
                }
            }
            return hit_anything;
        }

        bool hit_left = left->hit(r, ray_t, rec);
        bool hit_right = right->hit(r, interval(ray_t.min, hit_left ? rec.t : ray_t.max), rec);

        return hit_left || hit_right;
    }

    bbox get_bbox() const override
    {
        return b;
    }

private:
    // 中位数分割（辅助函数）
    void middle_split(std::vector<shared_ptr<hittable>> &objects,
                      size_t start, size_t end,
                      int axis, size_t &split_pos)
    {
        auto comparator = (axis == 0)   ? box_x_compare
                          : (axis == 1) ? box_y_compare
                                        : box_z_compare;

        std::sort(objects.begin() + start, objects.begin() + end, comparator);
        split_pos = start + (end - start) / 2;
    }

    // 尝试SAH分割（辅助函数）
    bool try_sah_split(std::vector<shared_ptr<hittable>> &objects,
                       size_t start, size_t end,
                       int &best_axis, size_t &best_split_pos)
    {
        constexpr int SAH_BUCKETS = 12;
        constexpr float COST_TRAVERSAL = 2.0f; // 遍历左右子节点的总开销
        constexpr float COST_INTERSECTION = 1;
        int BUCKETS_SIZE = int((end - start) / SAH_BUCKETS) + 1;

        struct BucketInfo
        {
            int count = 0;
            bbox bounds = bbox::empty;
        };

        float min_cost = std::numeric_limits<float>::max();
        bool found_good_split = false;

        for (size_t axis = 0; axis < 3; axis++)
        {
            BucketInfo buckets[SAH_BUCKETS];
            auto comparator = (axis == 0)   ? box_x_compare
                              : (axis == 1) ? box_y_compare
                                            : box_z_compare;
            std::sort(objects.begin() + start, objects.begin() + end, comparator);

            // 填充桶
            for (size_t i = start; i < end; ++i)
            {
                int bucket_idx = int((i - start) / BUCKETS_SIZE);
                buckets[bucket_idx].count++;
                buckets[bucket_idx].bounds = bbox(buckets[bucket_idx].bounds, objects[i]->get_bbox());
            }

            // 计算每个分割位置的代价
            for (int i = 1; i < SAH_BUCKETS; ++i)
            {
                bbox left_box = bbox::empty, right_box = bbox::empty;
                int left_count = 0, right_count = 0;

                // 计算左侧
                for (int j = 0; j < i; ++j)
                {
                    left_box = bbox(left_box, buckets[j].bounds);
                    left_count += buckets[j].count;
                }

                // 计算右侧
                for (int j = i; j < SAH_BUCKETS; ++j)
                {
                    right_box = bbox(right_box, buckets[j].bounds);
                    right_count += buckets[j].count;
                }

                // 计算代价（避免除零和无效包围盒）
                float left_area = left_box.surface_area();
                float right_area = right_box.surface_area();
                float total_area = b.surface_area();

                float cost = COST_TRAVERSAL + (left_count * left_area + right_count * right_area) / (total_area + 1e-8) * COST_INTERSECTION;

                // 更新最佳分割
                if (cost < min_cost && left_count > 0 && right_count > 0)
                {
                    min_cost = cost;
                    best_axis = axis;
                    best_split_pos = start + left_count; // 实际物体数量
                    found_good_split = true;
                }
            }
        }

        // 应用排序（仅当找到有效分割）
        if (found_good_split)
        {
            auto comparator = (best_axis == 0)   ? box_x_compare
                              : (best_axis == 1) ? box_y_compare
                                                 : box_z_compare;
            std::sort(objects.begin() + start, objects.begin() + end, comparator);
        }

        return found_good_split;
    }

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

#endif
