#include <iostream>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <iomanip>

// 配置结构体
struct Config
{
    int camera_vfov = 20;                   // -v
    int max_depth = 40;                     // -d
    int sample_num = 10;                    // -sa
    int rotate_degree = 0;                  // -rd
    vec3 camera_lookfrom = {1, 1, 1};       // -c0
    vec3 camera_lookat = {0, 0, 0};         // -c1
    int preset_id = -1;                     // -i
    bool bvh_depth_visual = false;          // -bvd
    int bvh_depth_visual_h = 20;            //
    bool bvh_group_visual = false;          // -bvg
    std::string bvh_group_visual_root = ""; //
    int bvh_group_visual_h = 20;            //
    bool use_openmp = true;                 // -mp
    bool ci = false;                        // -ci
    bool use_sample_rate = true;            // -sr
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
              << "  " << std::setw(8) << "-bvd N" << "Enable BVH depth visualization and set depth\n"
              << "  " << std::setw(8) << "-bvg <str> N" << "Enable BVH group visualization and set root and depth\n"
              << "  " << std::setw(8) << "-sah 0" << "Disable BVH SAH algorithm\n"
              << "  " << std::setw(8) << "-mp 0" << "Disable OpenMP\n"
              << "  " << std::setw(8) << "-ci" << "Enable continuous input\n"
              << "  " << std::setw(8) << "-sr 0" << "Disable sample rate\n";
}

void show_config(Config config)
{
    // 使用配置参数
    std::cout << "Current configuration:\n"
              << "    Depth: " << config.max_depth << "\n"
              << "    Samples: " << config.sample_num << "\n"
              << "        Sample rate: " << (config.use_sample_rate ? "ON " : "OFF ") << "\n"
              << "    Rotation: " << config.rotate_degree << " degrees\n"
              << "    Camera: " << "\n"
              << "        Look from: " << config.camera_lookfrom << "\n"
              << "        Look at: " << config.camera_lookat << "\n"
              << "        vFov: " << config.camera_vfov << "\n"
              << "    Preset: " << config.preset_id << "\n"
              << "    OpenMP: " << (config.use_openmp ? "ON " : "OFF ") << "\n"
              << "    BVH: " << (config.bvh_sah ? "SAH " : "MIDDLE ") << "\n"
              << "    BVH Depth Visual: " << (config.bvh_depth_visual ? "ON " : "OFF ") << config.bvh_depth_visual_h << "\n"
              << "    BVH Group Visual: " << (config.bvh_group_visual ? "ON " : "OFF ") << config.bvh_group_visual_h << " \"" << config.bvh_group_visual_root << "\"\n"
              << "    Continuous input: " << (config.ci ? "ON " : "OFF ") << "\n";
}

// 解析命令行参数
void parse_args(Config &config, int argc, char *argv[], int start)
{
    for (int i = start; i < argc;)
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
        else if (arg == "-sr")
        {
            config.use_sample_rate = std::stoi(argv[i + 1]);
            i += 2;
        }

        else if (arg == "-ci")
        {
            config.ci = true;
            i++;
        }

        // ci
        else if (arg == "0" && config.ci && config.bvh_group_visual)
        {
            config.bvh_group_visual_root += "0";
            i++;
        }
        else if (arg == "1" && config.ci && config.bvh_group_visual)
        {
            config.bvh_group_visual_root += "1";
            i++;
        }
        else if (arg == "2" && config.ci && config.bvh_group_visual)
        {
            config.bvh_group_visual_root = config.bvh_group_visual_root.substr(0, config.bvh_group_visual_root.length() - 1);
            i++;
        }

        // vfov
        else if (arg == "-v")
        {
            config.camera_vfov = std::stoi(argv[i + 1]);
            i += 2;
        }
        else if (arg == "-mp")
        {
            config.use_openmp = std::stoi(argv[i + 1]);
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

            switch (config.preset_id)
            {
            // local area sah
            case 0:
                std::cout << "Based on Preset 0: local area bvh-sah" << std::endl;
                config.camera_lookfrom = vec3(0, 1, 0);
                config.camera_lookat = vec3(-1, 1, 0);
                config.camera_vfov = 60;
                break;

            // overall middle
            case 1:
                std::cout << "Based on Preset 1: overall bvh-middle" << std::endl;
                config.camera_lookfrom = vec3(7, 6, 5);
                config.camera_lookat = vec3(-1, 0.5, -0.5);
                config.camera_vfov = 20;
                config.bvh_sah = false;
                break;

            // overall sah
            case 2:
                std::cout << "Based on Preset 2: overall bvh-sah" << std::endl;
                config.camera_lookfrom = vec3(7, 6, 5);
                config.camera_lookat = vec3(-1, 0.5, -0.5);
                config.camera_vfov = 20;
                break;

            case 3:
                std::cout << "Based on Preset 3: laptop" << std::endl;
                config.camera_lookfrom = vec3(-0.45, 1, -0.3);
                config.camera_lookat = vec3(-0.8, 0.9, -0.38);
                config.camera_vfov = 30;
                break;

            case 4:
                std::cout << "Based on Preset 4: desk" << std::endl;
                config.camera_lookfrom = vec3(0, 0.7, 2);
                config.camera_lookat = vec3(-1.1, 0.7, -0.5);
                config.camera_vfov = 10;
                break;

            case 10:
                std::cout << "Based on Preset 10: bvh visual local area bvh-sah" << std::endl;
                config.camera_lookfrom = vec3(0, 1, 0);
                config.camera_lookat = vec3(-1, 1, 0);
                config.camera_vfov = 60;
                config.sample_num = 50;
                break;

            // bvh visual middle
            case 11:
                std::cout << "Based on Preset 11: bvh visual bvh-middle" << std::endl;
                config.camera_lookfrom = vec3(7, 6, 5);
                config.camera_lookat = vec3(-1, 0.5, -0.5);
                config.camera_vfov = 20;
                config.sample_num = 50;
                config.bvh_sah = false;
                break;

            // bvh visual sah
            case 12:
                std::cout << "Based on Preset 12: bvh visual bvh-sah" << std::endl;
                config.camera_lookfrom = vec3(7, 6, 5);
                config.camera_lookat = vec3(-1, 0.5, -0.5);
                config.camera_vfov = 20;
                config.sample_num = 50;
                break;

            default:
                break;
            }
        }
        else if (arg == "-bvd")
        {
            config.bvh_depth_visual = true;
            config.bvh_group_visual = false;
            config.bvh_depth_visual_h = std::stoi(argv[i + 1]);
            if (config.bvh_depth_visual_h == 0)
                config.bvh_depth_visual = false;
            i += 2;
        }
        else if (arg == "-bvg")
        {
            config.bvh_group_visual = true;
            config.bvh_depth_visual = false;
            config.bvh_group_visual_h = std::stoi(argv[i + 1]);
            std::string s = argv[i + 2];
            config.bvh_group_visual_root = s == "." ? "" : s;
            if (config.bvh_group_visual_h == 0)
                config.bvh_group_visual_h = false;
            i += 3;
        }
        else if (arg == "-sah")
        {
            config.bvh_sah = std::stoi(argv[i + 1]);
            i += 2;
        }
        else
        {
            std::cout << "E: Unexpected Input: " << arg << std::endl;
            break;
        }
    }
}

// 将 cin 输入拆分为 argv 格式
void parse_cin_input(Config &config)
{
    std::string line;
    std::getline(std::cin, line); // 读取整行输入

    // 拆分输入为单词（类似命令行参数）
    std::vector<std::string> tokens;
    std::istringstream iss(line);
    std::string token;
    while (iss >> token)
        tokens.push_back(token);

    // 转换为 argc 和 argv 格式
    std::vector<char *> argv;
    for (auto &token : tokens)
        argv.push_back(const_cast<char *>(token.c_str()));

    // 调用 parse_args
    parse_args(config, argv.size(), argv.data(), 0);
}
