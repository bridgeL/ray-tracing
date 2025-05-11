// #ifndef OBJ_LOADER_H
// #define OBJ_LOADER_H

// #include <vector>
// #include <iostream>
// #include <fstream>
// #include <sstream>
// #include <Eigen/Eigen>
// #include "triangle.h"
// #include "material.h"

// using namespace Eigen;

// class Face
// {
// public:
//     Vector3i vert_pos;
//     Vector3i normals;
//     Vector3i tex_coords;

//     Face() {}
//     ~Face() {}
// };

// // a simple obj loader that reads the mesh
// class ObjLoader
// {
// public:
//     std::vector<Vector3f> vertices;      // vertices
//     std::vector<Vector3f> normals;       // normals
//     std::vector<Vector2f> tex_coords;    // texture coordinates
//     std::vector<shared_ptr<Face>> faces; // triangles
//     std::vector<shared_ptr<triangle>> triangles;

//     ObjLoader() {
//         transformation = Eigen::Matrix4f::Identity();   // init transformation matrix
//     }
//     ~ObjLoader() {}

//     // this function is defined for spliting three coordinates of a vertex in a face
//     std::vector<std::string> split(const std::string &s, char delimiter)
//     {
//         std::vector<std::string> tokens;
//         std::string token;
//         std::istringstream tokenStream(s);
//         while (std::getline(tokenStream, token, delimiter))
//         {
//             if (!token.empty())
//                 tokens.push_back(token);
//         }
//         return tokens;
//     };

//     inline bool read_obj(const std::string &filepath, const std::string &texturepath)
//     {
//         if (filepath.substr(filepath.size() - 4, 4) != ".obj")
//         {
//             std::cerr << "Error: only obj files are supported. Read aborted." << std::endl;
//             return false;
//         }

//         std::ifstream file(filepath);
//         if (!file.is_open())
//         {
//             std::cerr << "Error: Failed to open \"" << filepath << "\". Read aborted." << std::endl;
//             return false;
//         }

//         auto texture_data = cv::imread(texturepath);
//         cv::cvtColor(texture_data, texture_data, cv::COLOR_RGB2BGR);

//         std::string line;
//         while (std::getline(file, line))
//         {
//             std::istringstream iss(line);
//             std::string prefix;
//             iss >> prefix;
//             if ("v" == prefix)
//             {
//                 Vector3f vertex;
//                 iss >> vertex[0] >> vertex[1] >> vertex[2];
//                 this->vertices.push_back(vertex);
//             }
//             else if ("vn" == prefix)
//             {
//                 Vector3f normal;
//                 iss >> normal[0] >> normal[1] >> normal[2];
//                 this->normals.push_back(normal);
//             }
//             else if ("vt" == prefix)
//             {
//                 Vector2f texcoord;
//                 iss >> texcoord[0] >> texcoord[1];
//                 this->tex_coords.push_back(texcoord);
//             }
//             else if ("f" == prefix)
//             {
//                 auto face = make_shared<Face>();
//                 std::string vert[3];
//                 iss >> vert[0] >> vert[1] >> vert[2];
//                 std::string vert3;

//                 for (int i = 0; i < 3; i++)
//                 {
//                     auto elements = split(vert[i], '/');
//                     face->vert_pos[i] = std::stoi(elements[0]);
//                     face->tex_coords[i] = std::stoi(elements[1]);
//                     // face->normals[i] = std::stoi(elements[2]);
//                 }
//                 // create triangle
//                 auto mat = std::make_shared<textured_lambertian>(texture_data);

//                 auto tri = make_shared<triangle>(
//                     this->vertices[face->vert_pos[0] - 1],
//                     this->vertices[face->vert_pos[1] - 1],
//                     this->vertices[face->vert_pos[2] - 1], // vertices
//                     this->tex_coords[face->tex_coords[0] - 1],
//                     this->tex_coords[face->tex_coords[1] - 1],
//                     this->tex_coords[face->tex_coords[2] - 1],
//                     mat);
//                 // // this->faces.push_back(face);
//                 this->triangles.push_back(tri);

//                 if (iss >> vert3)
//                 {
//                     auto elements = split(vert3, '/');
//                     int vert_pos = std::stoi(elements[0]);
//                     int tex_coords = std::stoi(elements[1]);

//                     auto face2 = make_shared<Face>();

//                     auto tri = make_shared<triangle>(
//                         this->vertices[face->vert_pos[1] - 1],
//                         this->vertices[face->vert_pos[2] - 1],
//                         this->vertices[vert_pos - 1], // vertices
//                         this->tex_coords[face->tex_coords[1] - 1],
//                         this->tex_coords[face->tex_coords[2] - 1],
//                         this->tex_coords[tex_coords - 1],
//                         mat);

//                     // // this->faces.push_back(face);
//                     this->triangles.push_back(tri);
//                 }
//             }
//         }
//         return true;
//     }

//     // ######## Transformation Functions ########
//     inline void set_translate(float x, float y, float z){
//         // calculate the translation matrix and update transformation matrix
//         transformation = get_translation(Vector3f(x, y, z)) * transformation;
//     }

//     inline void set_rotate(float angle, float x, float y, float z){
//         // calculate the rotation matrix and and update transformation matrix
//         transformation = get_rotation(angle, Vector3f(x, y, z)) * transformation;
//     }

//     inline void set_scale(float x, float y, float z){
//         // calculate the scaling matrix and and update transformation matrix
//         Eigen::Matrix4f scaling = Eigen::Matrix4f::Identity();
//         scaling(0, 0) = x;
//         scaling(1, 1) = y;
//         scaling(2, 2) = z;
//         transformation = scaling * transformation;
//     }

//     inline void set_scale(float scale){
//         // calculate the scaling matrix and and update transformation matrix
//         Eigen::Matrix4f scaling = Eigen::Matrix4f::Identity();
//         scaling(0, 0) = scale;
//         scaling(1, 1) = scale;
//         scaling(2, 2) = scale;
//         transformation = scaling * transformation;
//     }

//     inline void apply_transformation(){
//         // apply the transformation matrix to all triangles
//         for (auto &t : triangles)
//         {
//             // apply transformation to each vertex of the triangle
//             for(int i = 0; i < 3; i++){
//                 Vector4f v = Vector4f(t->vertices[i][0], t->vertices[i][1], t->vertices[i][2], 1.0f);
//                 v = transformation * v;
//                 t->vertices[i] = v.head<3>(); // apply transformation and write back
//             }
//         }
//     }

// private:
//     Eigen::Matrix4f transformation;

//     // these helper functions are modified from HW1 codes
//     Eigen::Matrix4f get_translation(const Eigen::Vector3f &translation) {
//         // Calculate a transformation matrix of given translation vector.
//         Eigen::Matrix4f trans = Eigen::Matrix4f::Identity();
//         trans.block<3,1>(0,3) = translation.transpose(); // put translation value into the last column

//         return trans;
//     }

//     Eigen::Matrix4f get_rotation(float rotation_angle, const Eigen::Vector3f &axis) {
//         Eigen::Matrix4f rotation_matrix = Eigen::Matrix4f::Identity();

//         Eigen::Vector3f norm_vector = axis.normalized();  // normalize the axises
//         float nx = norm_vector.x();
//         float ny = norm_vector.y();
//         float nz = norm_vector.z(); // x, y, z value of the normalized axis

//         float rad = rotation_angle * 3.1415 / 180.0f;    // convert the angle from degree to radian
//         float cos = std::cos(rad);   // cos(theta)
//         float sin = std::sin(rad);   // sin(theta)
//         float omc = 1.0f - cos;  // 1 - cos(theta)

//         // put rotation parameters into the matrix
//         Eigen::Matrix3f rotation;
//         rotation << cos + nx*nx*omc,        nx*ny*omc - nz*sin,     nx*nz*omc + ny*sin,
//                     ny*nx*omc + nz*sin,     cos + ny*ny*omc,        ny*nz*omc - nx*sin,
//                     nz*nx*omc - ny*sin,     nz*ny*omc + nx*sin,     cos + nz*nz*omc;

//         // convert the rotation matrix into homogeneous matrix and return
//         rotation_matrix.block<3,3>(0,0) = rotation; // replace the upper left 3*3 block

//         return rotation_matrix;
//     }
// };

// #endif
