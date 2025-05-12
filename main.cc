#include "global.h"
#include "camera.h"
#include "triangle.h"
#include "sphere.h"
#include "timer.h"
#include "objloader.h"

int main(int argc, char *argv[])
{

    // 检查参数数量（程序名 + 至少1个参数）
    if (argc < 3)
    {
        std::cerr << "usage: " << argv[0] << " <rotate degree> <camera position choice>" << std::endl;
        return 1; // 非零返回值表示错误
    }

    float degree = std::stof(argv[1]);
    int camera_choice = std::stoi(argv[2]);

    ScopedTimer timer;

    hittable_list world;

    // {
    //     world.add(make_shared<sphere>(
    //         vec3(0, -1000, 0), 1000,
    //         make_shared<lambertian>(vec3(0.7, 0.7, 0.7))));
    // }

    auto loader = ObjLoader();
    // loader.read_obj("model/cow.obj", "model/cow2.png");
    // loader.read_obj("model/room/room.obj", "model/cow.png");

    timer.start_timer("Load time: ");
    loader.read_obj_with_mtl("model/room/room.obj", "model/room/room.mtl");
    timer.stop_timer();

    timer.start_timer("Tranformation time: ");
    loader.set_rotate(degree, vec3(0, 1, 0));
    // loader.set_scale(0.9);
    // loader.set_translate(0.1, 0.1, 0);
    loader.apply_transformation();

    timer.stop_timer();

    for (size_t i = 0; i < loader.triangles.size(); i++)
        world.add(loader.triangles[i]);

    camera cam;

    cam.aspect_ratio = 1.0 / 1.0;
    cam.image_width = 600;
    cam.samples_per_pixel = 500;
    cam.max_depth = 20;

    cam.background_color = vec3(1, 1, 1);

    // 高分辨率显示屏请调节此参数
    cam.screen_scale = 1.0;
    cam.screen_name = "image2";

    cam.vfov = camera_choice == 0 ? 60 : 20;
    switch (camera_choice)
    {
    case 0:
        cam.lookfrom = vec3(0, 1, 0);
        cam.lookat = vec3(-1, 1, 0);
        break;
    case 1:
        cam.lookfrom = vec3(7, 6, 5);
        cam.lookat = vec3(-1, 0.5, -0.5);
    default:
        break;
    }

    cam.vup = vec3(0, 1, 0);

    // cam.defocus_angle = 0.6;
    cam.defocus_angle = 0;
    cam.focus_dist = 10.0;

    // create bvh tree
    timer.start_timer("BVH time: ");
    world.create_bvh_tree(1);
    timer.stop_timer();

    // rendering
    timer.start_timer("Render time: ");
    cam.initialize();
    cam.render(world, true);
    timer.stop_timer();

    cam.screen.save("output.png");
    cam.screen.display(0);
}
