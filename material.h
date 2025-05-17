#ifndef MATERIAL_H
#define MATERIAL_H

#include <bitset>
#include "texture.h"

class material
{
public:
    virtual ~material() = default;

    virtual bool scatter(
        const ray &r_in, const hit_record &rec, vec3 &attenuation, ray &scattered) const
    {
        return false;
    }

    virtual bool emit(const ray &r_in, const hit_record &rec, vec3 &emit_color) const
    {
        return false;
    }
};

class lambertian : public material
{
public:
    lambertian(const vec3 &albedo) : tex(make_shared<solid_color>(albedo)) {}
    lambertian(shared_ptr<texture> tex) : tex(tex) {}

    bool scatter(const ray &r_in, const hit_record &rec, vec3 &attenuation, ray &scattered)
        const override
    {
        vec3 scatter_direction = rec.normal + random_unit_vector();

        // Catch degenerate scatter direction
        if (scatter_direction.near_zero())
            scatter_direction = rec.normal;

        scattered = ray(rec.p, scatter_direction);
        attenuation = tex->value(rec.u, rec.v);
        return true;
    }

private:
    shared_ptr<texture> tex;
};

vec3 convert_int_to_color(int i, int n)
{
    float phase = interval(0, n).clamp(i) * 4 / n;
    if (phase <= 1)
        return vec3(0, phase * 1, 1); // 阶段1: 蓝 → 青
    if (phase <= 2)
        return vec3(0, 1, (2 - phase) * 1); // 阶段2: 青 → 绿
    if (phase <= 3)
        return vec3((phase - 2) * 1, 1, 0); // 阶段3: 绿 → 黄
    return vec3(1, (4 - phase) * 1, 0);     // 阶段4: 黄 → 红
}

class bvh_depth_visual_mat : public material
{
public:
    int h;
    std::string root;

    bvh_depth_visual_mat(int h) : h(h) {}

    bool emit(const ray &r_in, const hit_record &rec, vec3 &emit_color)
        const override
    {
        // 展示各个对象在bvh树中的深度
        // 如果画面偏红，说明bvh tree的节点深度普遍超过 h/2
        emit_color = convert_int_to_color(rec.bvh_depth, h);
        return true;
    }
};

class bvh_group_visual_mat : public material
{
public:
    int h;
    std::string root;

    bvh_group_visual_mat(int n, std::string root) : h(n + root.length()), root(root) {}

    bool emit(const ray &r_in, const hit_record &rec, vec3 &emit_color)
        const override
    {
        // 展示从指定起点 到 指定最大深度处，各个对象分组的情况
        int t1 = 0;
        int t2 = pow(2, h - root.length() + 1) - 2;

        if (rec.bvh_path.length() >= root.length())
        {
            // 必须从指定起点开始
            std::string prefix = rec.bvh_path.substr(0, root.length());
            if (prefix == root)
            {
                // 截取剩余部分的字符串，但不超过最大探查深度
                int n = rec.bvh_path.length();
                std::string s = rec.bvh_path.substr(root.length(), std::min(n, h) - root.length());

                // 计算映射
                t1 = binaryStringToInt2(s);
            }
        }

        emit_color = convert_int_to_color(t1, t2);
        return true;
    }

private:
    // s是固定同一长度的01字符串，将他一一对应映射到整数空间
    static int binaryStringToInt(const std::string &binaryStr)
    {
        // 使用 std::bitset 解析二进制字符串
        std::bitset<32> bits(binaryStr);          // 假设二进制字符串不超过 32 位
        return static_cast<int>(bits.to_ulong()); // 转换为 unsigned long，再转 int
    }

    // s是任意长度的01字符串，将他一一对应映射到整数空间
    static int binaryStringToInt2(const std::string &s)
    {
        int h = s.length();
        int t0 = pow(2, h) - 2;        // 小于h长度的s，所有的可能数
        int t1 = binaryStringToInt(s); // 等于h长度的s，到目前位置的可能数
        return t0 + t1;
    }
};

class metal : public material
{
public:
    metal(const vec3 &albedo, double fuzz) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}

    bool scatter(const ray &r_in, const hit_record &rec, vec3 &attenuation, ray &scattered)
        const override
    {
        vec3 reflected = reflect(r_in.direction(), rec.normal);
        reflected = reflected.normalized() + (fuzz * random_unit_vector());
        scattered = ray(rec.p, reflected);
        attenuation = albedo;
        return (scattered.direction().dot(rec.normal) > 0);
    }

private:
    vec3 albedo;
    double fuzz;
};

#endif
