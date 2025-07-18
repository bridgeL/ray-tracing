#ifndef CONFIG_H
#define CONFIG_H

#include <iostream>
#include <vector>
#include <string>
#include "vec3.h"

// Configuration structure
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

// Print help message
void print_help()
{
    std::cout << "Usage: ./main [options]\n"
              << "Options:\n"
              << std::left
              << "  " << std::setw(16) << "-h" << "Show this help message\n"
              << "  " << std::setw(16) << "-v N" << "Set cam vfov\n"
              << "  " << std::setw(16) << "-d N" << "Set depth\n"
              << "  " << std::setw(16) << "-sa N" << "Set sample number\n"
              << "  " << std::setw(16) << "-rd N" << "Set rotate degree\n"
              << "  " << std::setw(16) << "-c0 x y z" << "Set camera look from\n"
              << "  " << std::setw(16) << "-c1 x y z" << "Set camera look at\n"
              << "  " << std::setw(16) << "-i N" << "Use preset configuration N\n"
              << "  " << std::setw(16) << "-bvd N" << "Enable BVH depth visualization and set depth\n"
              << "  " << std::setw(16) << "-bvg <str> N" << "Enable BVH group visualization and set root and depth\n"
              << "  " << std::setw(16) << "-sah 0" << "Disable BVH SAH algorithm\n"
              << "  " << std::setw(16) << "-mp 0" << "Disable OpenMP\n"
              << "  " << std::setw(16) << "-ci" << "Enable continuous input\n"
              << "  " << std::setw(16) << "-sr 0" << "Disable dynamic sample rate\n";
}

void show_config(Config config)
{
    // Display current configuration
    std::cout << "Current configuration:\n"
              << "    Depth: " << config.max_depth << "\n"
              << "    Samples: " << config.sample_num << "\n"
              << "        Dynamic sample rate: " << (config.use_sample_rate ? "ON " : "OFF ") << "\n"
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

// Parse command line arguments
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

        // Handle continuous input for BVH group visualization
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

        // Handle view field of view
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
            case 0:
                std::cout << "Based on Preset 0: laptop" << std::endl;
                config.camera_lookfrom = vec3(-0.45, 1, -0.3);
                config.camera_lookat = vec3(-0.8, 0.9, -0.38);
                config.camera_vfov = 30;
                break;

            case 1:
                std::cout << "Based on Preset 1: overall bvh-sah" << std::endl;
                config.camera_lookfrom = vec3(7, 6, 5);
                config.camera_lookat = vec3(-1, 0.5, -0.5);
                config.camera_vfov = 20;
                break;

            case 2:
                std::cout << "Based on Preset 2: local area bvh-sah" << std::endl;
                config.camera_lookfrom = vec3(0, 1, 0);
                config.camera_lookat = vec3(-1, 1, 0);
                config.camera_vfov = 60;
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

// Parse cin input into argv format
void parse_cin_input(Config &config)
{
    std::string line;
    std::getline(std::cin, line); // Read entire line

    // Split input into tokens (similar to command line args)
    std::vector<std::string> tokens;
    std::istringstream iss(line);
    std::string token;
    while (iss >> token)
        tokens.push_back(token);

    // Convert to argc and argv format
    std::vector<char *> argv;
    for (auto &token : tokens)
        argv.push_back(const_cast<char *>(token.c_str()));

    // Call parse_args
    parse_args(config, argv.size(), argv.data(), 0);
}

#endif
