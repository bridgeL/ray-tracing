#ifndef OBJ_LOADER_H
#define OBJ_LOADER_H

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include "triangle.h"
#include "material.h"
#include <Eigen/Eigen>

// a simple obj loader that reads the mesh
class ObjLoader
{
public:
    std::vector<vec3> v_list;
    std::vector<vec3> vn_list;
    std::vector<double> vt_u_list;
    std::vector<double> vt_v_list;

    std::vector<shared_ptr<triangle>> triangles; // triangles
    ObjLoader() {}

    inline bool read_obj(const std::string &filename, const std::string &texturename)
    {
        std::string filepath = "../" + filename;
        if (filepath.substr(filepath.size() - 4, 4) != ".obj")
        {
            std::cerr << "Error: only obj files are supported. Read aborted." << std::endl;
            return false;
        }

        std::ifstream file(filepath);
        if (!file.is_open())
        {
            std::cerr << "Error: Failed to open \"" << filepath << "\". Read aborted." << std::endl;
            return false;
        }

        // 读取texture
        auto mat = make_shared<lambertian>(make_shared<image_texture>(texturename));

        // 默认缺省值
        v_list.push_back(vec3(0, 0, 0));
        vn_list.push_back(vec3(0, 0, 0));
        vt_u_list.push_back(0);
        vt_v_list.push_back(0);

        std::string line;
        while (std::getline(file, line))
        {
            std::istringstream iss(line);
            std::string prefix;
            iss >> prefix;
            if ("v" == prefix)
            {
                vec3 pos;
                iss >> pos[0] >> pos[1] >> pos[2];
                v_list.push_back(pos);
            }
            else if ("vn" == prefix)
            {
                vec3 normal;
                iss >> normal[0] >> normal[1] >> normal[2];
                vn_list.push_back(normal);
            }
            else if ("vt" == prefix)
            {
                double u, v;
                iss >> u >> v;

                u = normalizeUV(u);
                v = normalizeUV(v);

                vt_u_list.push_back(u);
                vt_v_list.push_back(v);
            }
            else if ("f" == prefix)
            {
                std::string vert[3];
                iss >> vert[0] >> vert[1] >> vert[2];

                int v_idx[3] = {0, 0, 0};
                int vn_idx[3] = {0, 0, 0};
                int vt_idx[3] = {0, 0, 0};
                for (int i = 0; i < 3; i++)
                {
                    auto result = parse(vert[i]);
                    v_idx[i] = result[0];
                    vt_idx[i] = result[1];
                    vn_idx[i] = result[2];
                }

                // create triangle
                auto p0 = vertex(v_list[v_idx[0]], vt_u_list[vt_idx[0]], vt_v_list[vt_idx[0]], vn_list[vn_idx[0]]);
                auto p1 = vertex(v_list[v_idx[1]], vt_u_list[vt_idx[1]], vt_v_list[vt_idx[1]], vn_list[vn_idx[1]]);
                auto p2 = vertex(v_list[v_idx[2]], vt_u_list[vt_idx[2]], vt_v_list[vt_idx[2]], vn_list[vn_idx[2]]);

                auto tri = make_shared<triangle>(p0, p1, p2, mat);
                triangles.push_back(tri);

                // quad
                std::string vert3;
                if (iss >> vert3)
                {
                    auto result = parse(vert3);
                    auto v_idx3 = result[0];
                    auto vt_idx3 = result[1];
                    auto vn_idx3 = result[2];

                    // create triangle
                    auto p3 = vertex(v_list[v_idx3], vt_u_list[vt_idx3], vt_v_list[vt_idx3], vn_list[vn_idx3]);

                    auto tri2 = make_shared<triangle>(p2, p3, p0, mat);
                    triangles.push_back(tri2);
                }
            }
        }
        return true;
    }

    // ######## Transformation Functions ########
    inline void set_translate(float x, float y, float z)
    {
        // calculate the translation matrix and update transformation matrix
        transformation = get_translation(Eigen::Vector3f(x, y, z)) * transformation;
    }

    inline void set_rotate(float angle, vec3 axis)
    {
        // calculate the rotation matrix and and update transformation matrix
        transformation = get_rotation(angle, Eigen::Vector3f(axis[0], axis[1], axis[2])) * transformation;
    }

    inline void set_scale(float x, float y, float z)
    {
        // calculate the scaling matrix and and update transformation matrix
        Eigen::Matrix4f scaling = Eigen::Matrix4f::Identity();
        scaling(0, 0) = x;
        scaling(1, 1) = y;
        scaling(2, 2) = z;
        transformation = scaling * transformation;
    }

    inline void set_scale(float scale)
    {
        // calculate the scaling matrix and and update transformation matrix
        Eigen::Matrix4f scaling = Eigen::Matrix4f::Identity();
        scaling(0, 0) = scale;
        scaling(1, 1) = scale;
        scaling(2, 2) = scale;
        transformation = scaling * transformation;
    }

    inline void apply_transformation()
    {
        // 遍历所有三角形
        for (auto &tri : triangles)
        {
            // 遍历三角形的三个顶点
            for (int i = 0; i < 3; i++)
            {
                // 获取顶点坐标
                vec3 &pos = tri->vertices[i].pos;

                // 应用变换
                Eigen::Vector4f a(pos.x(), pos.y(), pos.z(), 1.0f);
                a = transformation * a;

                // 更新顶点坐标
                pos = vec3(a[0], a[1], a[2]);
            }

            // 重新计算三角形法向量和bounding box
            tri->calculateBBox();
            tri->calculateNormal();
        }

        transformation = Eigen::Matrix4f::Identity();
    }

private:
    Eigen::Matrix4f transformation = Eigen::Matrix4f::Identity();

    // this function is defined for spliting three coordinates of a vertex in a face
    std::vector<int> parse(const std::string &s)
    {
        std::vector<int> result;

        char delimiter = '/';
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, delimiter))
        {
            if (!token.empty())
                result.push_back(std::stoi(token));
            else
                result.push_back(0);
        }
        return result;
    };

    // these helper functions are modified from HW1 codes
    Eigen::Matrix4f get_translation(const Eigen::Vector3f &translation)
    {
        // Calculate a transformation matrix of given translation vector.
        Eigen::Matrix4f trans = Eigen::Matrix4f::Identity();
        trans.block<3, 1>(0, 3) = translation.transpose(); // put translation value into the last column

        return trans;
    }

    Eigen::Matrix4f get_rotation(float rotation_angle, const Eigen::Vector3f &axis)
    {
        Eigen::Matrix4f rotation_matrix = Eigen::Matrix4f::Identity();

        Eigen::Vector3f norm_vector = axis.normalized(); // normalize the axises
        float nx = norm_vector.x();
        float ny = norm_vector.y();
        float nz = norm_vector.z(); // x, y, z value of the normalized axis

        float rad = rotation_angle * 3.1415 / 180.0f; // convert the angle from degree to radian
        float cos = std::cos(rad);                    // cos(theta)
        float sin = std::sin(rad);                    // sin(theta)
        float omc = 1.0f - cos;                       // 1 - cos(theta)

        // put rotation parameters into the matrix
        Eigen::Matrix3f rotation;
        rotation << cos + nx * nx * omc, nx * ny * omc - nz * sin, nx * nz * omc + ny * sin,
            ny * nx * omc + nz * sin, cos + ny * ny * omc, ny * nz * omc - nx * sin,
            nz * nx * omc - ny * sin, nz * ny * omc + nx * sin, cos + nz * nz * omc;

        // convert the rotation matrix into homogeneous matrix and return
        rotation_matrix.block<3, 3>(0, 0) = rotation; // replace the upper left 3*3 block

        return rotation_matrix;
    }
};

#endif
