#include "global.h"
#include "camera.h"
#include "triangle.h"
#include "sphere.h"
#include "timer.h"
#include "objloader.h"
#include "config.hpp"

int main(int argc, char *argv[])
{
// 检查 OpenMP 是否可用
#ifdef _OPENMP
    std::cout << "OpenMP enabled (max threads: " << omp_get_max_threads() << ")\n";
#else
    std::cout << "OpenMP not available\n";
#endif

    Config config;
    parse_args(config, argc, argv, 1);

    if (config.help)
    {
        print_help();
        return 0;
    }

    ScopedTimer timer;
    hittable_list world;

    world.add(make_shared<sphere>(vec3(-1.2, 0.9, -1), 0.2, make_shared<glass>(1.5)));
    world.add(make_shared<sphere>(vec3(-1.1, 0.637, -0.8), 0.01, make_shared<metal>(vec3(1, 0.2, 0.2), 0.3)));
    world.add(make_shared<sphere>(vec3(-1.1, 0.637, -0.7), 0.01, make_shared<metal>(vec3(0.2, 0.2, 1), 0.3)));
    world.add(make_shared<sphere>(vec3(-1.15, 0.637, -0.6), 0.01, make_shared<metal>(vec3(0.2, 0.2, 0.2), 0.3)));
    world.add(make_shared<sphere>(vec3(-1.2, 0.7, -0.6), 0.03, make_shared<light_mat>(vec3(1, 1, 1), 20)));
    world.add(make_shared<sphere>(vec3(0.196, 0.6, -1.39), 0.03, make_shared<light_mat>(vec3(1, 1, 1), 40)));

    ObjLoader loader;

    std::srand(42);
    timer.start_timer("Load");
    // loader.read_obj("model/cow.obj", "model/cow2.png");
    loader.read_obj_with_mtl("model/room/room.obj", "model/room/room.mtl");
    timer.stop_timer();

    timer.start_timer("Tranformation");
    loader.set_rotate(config.rotate_degree, vec3(0, 1, 0));
    // loader.set_scale(0.9);
    // loader.set_translate(0.1, 0.1, 0);
    loader.apply_transformation();
    timer.stop_timer();

    for (size_t i = 0; i < loader.triangles.size(); i++)
        world.add(loader.triangles[i]);

    camera cam;

    cam.image_width = 900;
    cam.image_height = 520;
    cam.samples_per_pixel = config.sample_num;
    cam.max_depth = config.max_depth;

    // 黑色背景
    cam.background_color = vec3(1, 1, 1);

    // 高分辨率显示屏请调节此参数
    cam.screen_scale = 1.0;
    cam.screen_name = "image";

    cam.lookfrom = config.camera_lookfrom;
    cam.lookat = config.camera_lookat;
    cam.vfov = config.camera_vfov;
    cam.vup = vec3(0, 1, 0);

    cam.focus_dist = 10.0;

    if (config.bvh_depth_visual)
        cam.mat = make_shared<bvh_depth_visual_mat>(config.bvh_depth_visual_h);
    else if (config.bvh_group_visual)
        cam.mat = make_shared<bvh_group_visual_mat>(config.bvh_group_visual_h, config.bvh_group_visual_root);
    else
        cam.mat = nullptr;

    show_config(config);
    std::cout << "Objects number: " << world.objects.size() << std::endl;
    std::cout << "image size: " << cam.image_width << 'x' << cam.image_height << std::endl;

    // create bvh tree
    bool sah = config.bvh_sah;
    timer.start_timer("BVH build");
    world.create_bvh_tree(5, sah ? BVHSplitMethod::SAH : BVHSplitMethod::MIDDLE);
    timer.stop_timer();

    // rendering
    timer.start_timer("Render");
    cam.initialize();
    cam.render(world, true, config.use_openmp, config.use_sample_rate);
    timer.stop_timer();

    cam.screen.save("output.png");
    cam.screen.display(1);

    while (config.ci)
    {
        parse_cin_input(config);
        show_config(config);

        if (config.help)
        {
            print_help();
            continue;
        }

        std::cout << "Objects number: " << world.objects.size() << std::endl;
        std::cout << "image size: " << cam.image_width << 'x' << cam.image_height << std::endl;

        if (config.bvh_depth_visual)
            cam.mat = make_shared<bvh_depth_visual_mat>(config.bvh_depth_visual_h);
        else if (config.bvh_group_visual)
            cam.mat = make_shared<bvh_group_visual_mat>(config.bvh_group_visual_h, config.bvh_group_visual_root);
        else
            cam.mat = nullptr;

        cam.samples_per_pixel = config.sample_num;
        cam.max_depth = config.max_depth;
        cam.lookfrom = config.camera_lookfrom;
        cam.lookat = config.camera_lookat;
        cam.vfov = config.camera_vfov;

        if (config.bvh_sah != sah)
        {
            sah = config.bvh_sah;
            timer.start_timer("BVH build");
            world.create_bvh_tree(5, sah ? BVHSplitMethod::SAH : BVHSplitMethod::MIDDLE);
            timer.stop_timer();
        }

        timer.start_timer("Render");
        cam.initialize();
        cam.render(world, true, config.use_openmp, config.use_sample_rate);
        timer.stop_timer();

        cam.screen.save("output.png");
        cam.screen.display(1);
    }
}
