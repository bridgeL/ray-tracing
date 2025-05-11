#include "global.h"
#include "camera.h"
#include "triangle.h"
#include "sphere.h"
#include "timer.h"
#include "objloader.h"

int main()
{
    hittable_list world;

    {
        world.add(make_shared<sphere>(
            vec3(0, -1000, 0), 1000,
            make_shared<lambertian>(vec3(0.7, 0.7, 0.7))));
    }

    {
        auto loader = ObjLoader();
        // loader.read_obj("../model/cow.obj", "../model/cow.png");
        loader.read_obj("../model/tiny_desktop.obj", "../model/cow.png");
        loader.set_rotate(150, vec3(0, 1, 0));
        loader.set_scale(0.9);
        loader.set_translate(1, 1, 0);
        loader.apply_transformation();

        for (size_t i = 0; i < loader.triangles.size(); i++)
            world.add(loader.triangles[i]);
    }

    {
        world.add(make_shared<sphere>(
            vec3(-2, 2, 0), 1,
            make_shared<metal>(vec3(0.7, 0.6, 0.5), 0.0)));
    }

    camera cam;

    cam.aspect_ratio = 4.0 / 3.0;
    cam.image_width = 512;
    cam.samples_per_pixel = 10;
    cam.max_depth = 5;

    // 高分辨率显示屏请调节此参数
    cam.screen_scale = 2.0;

    cam.vfov = 20;
    cam.lookfrom = vec3(0, 20, 20);
    cam.lookat = vec3(0, 1, 0);
    cam.vup = vec3(0, 1, 0);

    // cam.defocus_angle = 0.6;
    cam.defocus_angle = 0;
    cam.focus_dist = 10.0;

    ScopedTimer timer("Render time: ");

    // create bvh tree
    world.create_bvh_tree(1);

    // rendering
    cam.initialize();
    cam.render(world, true);

    timer.stop_timer();

    cam.screen.save("output.png");
    cam.screen.display(0);
}
