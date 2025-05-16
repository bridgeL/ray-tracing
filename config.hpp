#include <iostream>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <iomanip>

// 配置结构体
struct Config
{
    int camera_vfov = 20;                     // -v
    int max_depth = 20;                       // -d
    int sample_num = 10;                      // -sa
    int rotate_degree = 0;                    // -rd
    vec3 camera_lookfrom = vec3(7, 6, 5);     // -c0
    vec3 camera_lookat = vec3(-1, 0.5, -0.5); // -c1
    int preset_id = -1;                       // -i
    bool bvh_visual = false;                  // -bv
    int bvh_h = 20;
    bool bvh_sah = true;
    bool help = false;
};

// 打印帮助信息
void print_help()
{
    std::cout << "Usage: ./main [options]\n"
              << "Options:\n"
              << std::left
              << "  " << std::setw(8) << "-h" << "Show this help message\n"
              << "  " << std::setw(8) << "-v N" << "Set cam vfov (default: 20)\n"
              << "  " << std::setw(8) << "-d N" << "Set depth (default: 20)\n"
              << "  " << std::setw(8) << "-sa N" << "Set sample number (default: 10)\n"
              << "  " << std::setw(8) << "-rd N" << "Set rotate degree (default: 0)\n"
              << "  " << std::setw(8) << "-c0 x y z" << "Set camera look from (default: 0 1 0)\n"
              << "  " << std::setw(8) << "-c1 x y z" << "Set camera look at (default: -1 1 0)\n"
              << "  " << std::setw(8) << "-i N" << "Use preset configuration N (default: -1)\n"
              << "  " << std::setw(8) << "-bv N" << "Enable BVH visualization and set visual depth\n"
              << "  " << std::setw(8) << "-sah" << "Enable BVH SAH algorithm\n";
}

void show_config(Config config)
{
    // 使用配置参数
    std::cout << "Current configuration:\n"
              << "  Depth: " << config.max_depth << "\n"
              << "  Samples: " << config.sample_num << "\n"
              << "  Rotation: " << config.rotate_degree << " degrees\n"
              << "  Camera: look from: " << config.camera_lookfrom << ", look at: "
              << config.camera_lookat << "\n"
              << "  Preset: " << config.preset_id << "\n"
              << "  BVH SAH: " << (config.bvh_sah ? "ON " : "OFF ") << "\n"
              << "  BVH Visual: " << (config.bvh_visual ? "ON " : "OFF ")
              << config.bvh_h << "\n";
}

// 解析命令行参数
Config parse_args(int argc, char *argv[])
{
    Config config;

    for (int i = 1; i < argc;)
    {
        std::string arg = argv[i];

        if (arg == "-h")
        {
            config.help = true;
            break;
        }

        if (arg == "-sa")
        {
            config.sample_num = std::stoi(argv[i + 1]);
            i += 2;
        }
        else if (arg == "-v")
        {
            config.camera_vfov = std::stoi(argv[i + 1]);
            i += 2;
        }
        else if (arg == "-d")
        {
            config.max_depth = std::stoi(argv[i + 1]);
            i += 2;
        }
        else if (arg == "-rd")
        {
            config.rotate_degree = std::stoi(argv[i + 1]);
            i += 2;
        }
        else if (arg == "-c0")
        {
            config.camera_lookfrom = {
                std::stof(argv[i + 1]),
                std::stof(argv[i + 2]),
                std::stof(argv[i + 3])};
            i += 4;
        }
        else if (arg == "-c1")
        {
            config.camera_lookat = {
                std::stof(argv[i + 1]),
                std::stof(argv[i + 2]),
                std::stof(argv[i + 3])};
            i += 4;
        }
        else if (arg == "-i")
        {
            config.preset_id = std::stoi(argv[i + 1]);
            i += 2;
        }
        else if (arg == "-bv")
        {
            config.bvh_visual = true;
            config.bvh_h = std::stoi(argv[i + 1]);
            i += 2;
        }
        else if (arg == "-sah")
        {
            config.bvh_sah = true;
            i++;
        }
    }

    switch (config.preset_id)
    {
    // local area sah
    case 0:
        std::cout << "Preset: local area bvh-sah" << std::endl;
        config.camera_lookfrom = vec3(0, 1, 0);
        config.camera_lookat = vec3(-1, 1, 0);
        config.camera_vfov = 60;
        config.sample_num = 10;
        config.max_depth = 20;
        config.rotate_degree = 0;
        config.bvh_sah = true;
        config.bvh_visual = false;
        break;

    // overall sah
    case 1:
        std::cout << "Preset: overall bvh-sah" << std::endl;
        config.camera_lookfrom = vec3(7, 6, 5);
        config.camera_lookat = vec3(-1, 0.5, -0.5);
        config.camera_vfov = 20;
        config.sample_num = 10;
        config.max_depth = 20;
        config.rotate_degree = 0;
        config.bvh_sah = true;
        config.bvh_visual = false;
        break;

    // overall middle
    case 2:
        std::cout << "Preset: overall bvh-middle" << std::endl;
        config.camera_lookfrom = vec3(7, 6, 5);
        config.camera_lookat = vec3(-1, 0.5, -0.5);
        config.camera_vfov = 20;
        config.sample_num = 10;
        config.max_depth = 20;
        config.rotate_degree = 0;
        config.bvh_sah = false;
        config.bvh_visual = false;
        break;

    // bvh visual middle
    case 12:
        std::cout << "Preset: bvh visual bvh-middle" << std::endl;
        config.camera_lookfrom = vec3(7, 6, 5);
        config.camera_lookat = vec3(-1, 0.5, -0.5);
        config.camera_vfov = 20;
        config.sample_num = 1;
        config.max_depth = 1;
        config.rotate_degree = 0;
        config.bvh_visual = true;
        config.bvh_sah = false;
        config.bvh_h = 20;
        break;

    // bvh visual sah
    case 13:
        std::cout << "Preset: bvh visual bvh-sah" << std::endl;
        config.camera_lookfrom = vec3(7, 6, 5);
        config.camera_lookat = vec3(-1, 0.5, -0.5);
        config.camera_vfov = 20;
        config.sample_num = 1;
        config.max_depth = 1;
        config.rotate_degree = 0;
        config.bvh_visual = true;
        config.bvh_sah = true;
        config.bvh_h = 20;
        break;

    default:
        break;
    }

    return config;
}
