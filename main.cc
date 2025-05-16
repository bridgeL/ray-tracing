#include "global.h"
#include "camera.h"
#include "triangle.h"
#include "sphere.h"
#include "timer.h"
#include "objloader.h"
#include "config.hpp"

int main(int argc, char *argv[])
{

    Config config = parse_args(argc, argv);

    if (config.help)
    {
        print_help();
        return 0;
    }

    show_config(config);

    ScopedTimer timer;
    hittable_list world;

    ObjLoader loader;

    // timer.start_timer("Load: ");
    // loader.read_obj("model/cow.obj", "model/cow2.png");
    // // loader.read_obj_with_mtl("model/room/room.obj", "model/room/room.mtl");
    // timer.stop_timer();

    // timer.start_timer("Tranformation: ");
    // loader.set_rotate(config.rotate_degree, vec3(0, 1, 0));
    // // loader.set_scale(0.9);
    // // loader.set_translate(0.1, 0.1, 0);
    // loader.apply_transformation();

    // timer.stop_timer();

    // auto mat = make_shared<bvh_visualization_mat>();
    // for (size_t i = 0; i < loader.triangles.size(); i++)
    // {
    //     if (config.bvh_visual)
    //         loader.triangles[i]->mat = mat;
    //     world.add(loader.triangles[i]);
    // }

    world.add(make_shared<sphere>(vec3(0, 0, 0), 1, make_shared<lambertian>(vec3(1, 0, 1))));
    // world.add(make_shared<sphere>(vec3(0, 0, 0), 1, make_shared<bvh_visualization_mat>(40)));

    std::cout << "Objects number: " << world.objects.size() << std::endl;

    camera cam;

    cam.aspect_ratio = 1.0 / 1.0;
    cam.image_width = 200;
    cam.samples_per_pixel = config.sample_num;
    cam.max_depth = config.max_depth;

    cam.background_color = vec3(1, 1, 1);

    // 高分辨率显示屏请调节此参数
    cam.screen_scale = 1.0;
    cam.screen_name = "image";

    cam.vfov = config.camera_vfov;
    cam.vup = vec3(0, 1, 0);

    // cam.defocus_angle = 0.6;
    cam.defocus_angle = 0;
    cam.focus_dist = 10.0;

    // create bvh tree
    timer.start_timer("BVH: ");
    world.create_bvh_tree(1, BVHSplitMethod::MIDDLE);
    timer.stop_timer();

    // rendering
    timer.start_timer("Render: ");
    cam.initialize();
    cam.render(world, true);
    timer.stop_timer();

    cam.screen.save("output.png");
    cam.screen.display(0);
}
