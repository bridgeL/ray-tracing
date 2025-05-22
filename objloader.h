#ifndef OBJ_LOADER_H
#define OBJ_LOADER_H

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include "triangle.h"
#include "material.h"
#include <Eigen/Eigen>

// A simple OBJ loader that reads mesh data
class ObjLoader
{
public:
    std::vector<vec3> v_list;  // Vertex positions
    std::vector<vec3> vn_list;  // Vertex normals
    std::vector<double> vt_u_list;  // Texture U coordinates
    std::vector<double> vt_v_list;  // Texture V coordinates
    std::map<std::string, shared_ptr<material>> materials; // Material/texture map

    std::vector<shared_ptr<triangle>> triangles; // Triangle list
    ObjLoader() {}

    inline bool read_obj(const std::string &filename, const std::string &texturename)
    {
        std::string filepath = "../" + filename;
        std::string texturepath = "../" + texturename;
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

        // Read texture
        auto mat = make_shared<lambertian>(make_shared<image_texture>(texturepath));

        // Default values
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

                // Create triangle
                auto p0 = vertex(v_list[v_idx[0]], vt_u_list[vt_idx[0]], vt_v_list[vt_idx[0]], vn_list[vn_idx[0]]);
                auto p1 = vertex(v_list[v_idx[1]], vt_u_list[vt_idx[1]], vt_v_list[vt_idx[1]], vn_list[vn_idx[1]]);
                auto p2 = vertex(v_list[v_idx[2]], vt_u_list[vt_idx[2]], vt_v_list[vt_idx[2]], vn_list[vn_idx[2]]);

                auto tri = make_shared<triangle>(p0, p1, p2, mat);
                triangles.push_back(tri);

                // Handle quad faces (convert to two triangles)
                std::string vert3;
                if (iss >> vert3)
                {
                    auto result = parse(vert3);
                    auto v_idx3 = result[0];
                    auto vt_idx3 = result[1];
                    auto vn_idx3 = result[2];

                    // Create second triangle
                    auto p3 = vertex(v_list[v_idx3], vt_u_list[vt_idx3], vt_v_list[vt_idx3], vn_list[vn_idx3]);

                    auto tri2 = make_shared<triangle>(p2, p3, p0, mat);
                    triangles.push_back(tri2);
                }
            }
        }
        return true;
    }

    bool read_obj_with_mtl(const std::string &objname, const std::string &mtlname)
    {
        // Pre-processing and file validation
        std::string objpath = "../" + objname;
        std::string mtlpath = "../" + mtlname;
        if (objpath.substr(objpath.size() - 4, 4) != ".obj")
        {
            std::cerr << "Error: only obj files are supported. Read aborted." << std::endl;
            return false;
        }
        if (mtlname.substr(mtlname.size() - 4, 4) != ".mtl")
        {
            std::cerr << "Error: only mtl files are supported. Read aborted." << std::endl;
            return false;
        }
        if (objpath.substr(objpath.size() - 4, 4) != ".obj")
        {
            std::cerr << "Error: only obj files are supported. Read aborted." << std::endl;
            return false;
        }
        std::ifstream file(objpath);
        if (!file.is_open())
        {
            std::cerr << "Error: Failed to open \"" << objpath << "\". Read aborted." << std::endl;
            return false;
        }

        // Read MTL file first
        std::ifstream mtlfile(mtlpath);
        if (!mtlfile.is_open())
        {
            std::cerr << "Error: Failed to open \"" << mtlpath << "\". Read aborted." << std::endl;
            return false;
        }
        std::string line;
        std::string matname;
        std::string texturepath;
        int i = mtlpath.find_last_of("/");
        std::string textureprefix = mtlpath.substr(0, i) + "/";
        while (std::getline(mtlfile, line))
        {
            std::istringstream iss(line);
            std::string prefix;
            iss >> prefix;
            if ("newmtl" == prefix)
            {
                iss >> matname;
            }
            else if ("Kd" == prefix)
            {
                double r, b, g;
                iss >> r >> g >> b;
                auto mat = make_shared<lambertian>(vec3(r, g, b));
                materials[matname] = mat; // Store material in map
            }
            else if ("map_Kd" == prefix)
            {
                iss >> texturepath;
                auto mat = make_shared<lambertian>(make_shared<image_texture>(textureprefix + texturepath));
                materials[matname] = mat; // Store texture in map
            }
        }

        // Create a special material: Material.magic
        materials["Material.magic"] = make_shared<magic_mat>(
            vec3(7, 6, 5),
            vec3(-1, 0.5, -0.9),
            vec3(0, 1, 0),
            10, 10, 800, 600);
        // Alternative magic material configuration (commented out):
        // materials["Material.magic"] = make_shared<magic_mat>(
        //     vec3(-0.45, 1, -0.3),
        //     vec3(-0.8, 0.9, -0.38),
        //     vec3(0, 1, 0),
        //     30, 10, 800, 600);

        materials["laptop"] = make_shared<metal>(vec3(0.1, 0.1, 0.1), 0.8);

        // Read OBJ file
        // Default values
        v_list.push_back(vec3(0, 0, 0));
        vn_list.push_back(vec3(0, 0, 0));
        vt_u_list.push_back(0);
        vt_v_list.push_back(0);
        materials["default"] = make_shared<lambertian>(vec3(0.5, 0.5, 0.5)); // Default material

        std::string current_mat = "default"; // Current material
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

                // u = normalizeUV(u);
                // v = normalizeUV(v);

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

                // Create triangle with current material
                auto p0 = vertex(v_list[v_idx[0]], vt_u_list[vt_idx[0]], vt_v_list[vt_idx[0]], vn_list[vn_idx[0]]);
                auto p1 = vertex(v_list[v_idx[1]], vt_u_list[vt_idx[1]], vt_v_list[vt_idx[1]], vn_list[vn_idx[1]]);
                auto p2 = vertex(v_list[v_idx[2]], vt_u_list[vt_idx[2]], vt_v_list[vt_idx[2]], vn_list[vn_idx[2]]);

                // MODIFIED: Add material support
                shared_ptr<material> mat;
                if (materials.find(current_mat) != materials.end())
                {
                    mat = materials[current_mat];
                }
                else
                {
                    mat = materials["default"];
                }
                auto tri = make_shared<triangle>(p0, p1, p2, mat);
                triangles.push_back(tri);

                // Handle quad faces (convert to two triangles)
                std::string vert3;
                if (iss >> vert3)
                {
                    auto result = parse(vert3);
                    auto v_idx3 = result[0];
                    auto vt_idx3 = result[1];
                    auto vn_idx3 = result[2];

                    // Create second triangle
                    auto p3 = vertex(v_list[v_idx3], vt_u_list[vt_idx3], vt_v_list[vt_idx3], vn_list[vn_idx3]);

                    auto tri2 = make_shared<triangle>(p2, p3, p0, mat);
                    triangles.push_back(tri2);
                }
            }
            else if ("usemtl" == prefix)
            {
                iss >> current_mat; // Update current material
            }
        }
        return true;
    }

    // ######## Transformation Functions ########
    inline void set_translate(float x, float y, float z)
    {
        // Calculate translation matrix and update transformation matrix
        transformation = get_translation(Eigen::Vector3f(x, y, z)) * transformation;
    }

    inline void set_rotate(float angle, vec3 axis)
    {
        // Calculate rotation matrix and update transformation matrix
        transformation = get_rotation(angle, Eigen::Vector3f(axis[0], axis[1], axis[2])) * transformation;
    }

    inline void set_scale(float x, float y, float z)
    {
        // Calculate scaling matrix and update transformation matrix
        Eigen::Matrix4f scaling = Eigen::Matrix4f::Identity();
        scaling(0, 0) = x;
        scaling(1, 1) = y;
        scaling(2, 2) = z;
        transformation = scaling * transformation;
    }

    inline void set_scale(float scale)
    {
        // Calculate uniform scaling matrix and update transformation matrix
        Eigen::Matrix4f scaling = Eigen::Matrix4f::Identity();
        scaling(0, 0) = scale;
        scaling(1, 1) = scale;
        scaling(2, 2) = scale;
        transformation = scaling * transformation;
    }

    inline void apply_transformation()
    {
        // Apply transformation to all triangles
        for (auto &tri : triangles)
        {
            // Transform each vertex of the triangle
            for (int i = 0; i < 3; i++)
            {
                // Get vertex position
                vec3 &pos = tri->vertices[i].pos;

                // Apply transformation
                Eigen::Vector4f a(pos.x(), pos.y(), pos.z(), 1.0);
                a = transformation * a;

                // Update vertex position
                pos = vec3(a[0], a[1], a[2]);
            }

            // Recalculate triangle normal and bounding box
            tri->calculateBBox();
            tri->calculateNormal();
        }

        // Reset transformation matrix to identity
        transformation = Eigen::Matrix4f::Identity();
    }

private:
    Eigen::Matrix4f transformation = Eigen::Matrix4f::Identity();

    // Parse vertex indices from face definition (v/vt/vn format)
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

    // Helper functions for transformation matrices (modified from HW1 codes)
    Eigen::Matrix4f get_translation(const Eigen::Vector3f &translation)
    {
        // Create translation matrix from given vector
        Eigen::Matrix4f trans = Eigen::Matrix4f::Identity();
        trans.block<3, 1>(0, 3) = translation.transpose(); // Set translation components

        return trans;
    }

    Eigen::Matrix4f get_rotation(float rotation_angle, const Eigen::Vector3f &axis)
    {
        Eigen::Matrix4f rotation_matrix = Eigen::Matrix4f::Identity();

        Eigen::Vector3f norm_vector = axis.normalized(); // Normalize rotation axis
        float nx = norm_vector.x();
        float ny = norm_vector.y();
        float nz = norm_vector.z(); // Components of normalized axis

        float rad = rotation_angle * 3.1415 / 180.0; // Convert angle to radians
        float cos = std::cos(rad);                   // cos(theta)
        float sin = std::sin(rad);                   // sin(theta)
        float omc = 1.0 - cos;                       // 1 - cos(theta)

        // Build rotation matrix components
        Eigen::Matrix3f rotation;
        rotation << cos + nx * nx * omc, nx * ny * omc - nz * sin, nx * nz * omc + ny * sin,
            ny * nx * omc + nz * sin, cos + ny * ny * omc, ny * nz * omc - nx * sin,
            nz * nx * omc - ny * sin, nz * ny * omc + nx * sin, cos + nz * nz * omc;

        // Convert to homogeneous coordinates and return
        rotation_matrix.block<3, 3>(0, 0) = rotation; // Set rotation components

        return rotation_matrix;
    }
};

#endif
