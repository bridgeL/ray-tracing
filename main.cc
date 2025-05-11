#include "global.h"
#include "camera.h"
#include "triangle.h"
#include "sphere.h"
#include "timer.h"
#include "objloader.h"

int main()
{
    hittable_list world;

    auto ground_material = make_shared<lambertian>(vec3(0.7, 0.7, 0.7));
    world.add(make_shared<sphere>(vec3(0, -1000, 0), 1000, ground_material));

    // {
    //     auto loader = make_shared<ObjLoader>();
    //     loader->read_obj("../model/cow.obj", "../model/cow.png");
    //     loader->set_scale(1.5);
    //     // loader->set_translate(0, 1, 0);
    //     loader->apply_transformation();

    //     for (size_t i = 0; i < loader->triangles.size(); i++)
    //         world.add(loader->triangles[i]);
    // }

    {
        auto m1 = make_shared<metal>(vec3(0.7, 0.6, 0.5), 0.0);
        world.add(make_shared<sphere>(vec3(-2, 2, 0), 1, m1));
    }

    {
        auto image_data = cv::imread("../model/cow.png");
        auto m2 = make_shared<textured_lambertian>(image_data);
        world.add(make_shared<triangle>(
            vertex(vec3(0.1, 0, 0), 0, 0, vec3(0, 0, 0)),
            vertex(vec3(0, 3, 0), 0, 1, vec3(0, 0, 0)),
            vertex(vec3(0, 0, 3), 1, 1, vec3(0, 0, 0)),
            m2));
    }

    camera cam;

    cam.aspect_ratio = 4.0 / 3.0;
    cam.image_width = 1024;
    cam.samples_per_pixel = 100;
    cam.max_depth = 5;

    cam.screen_scale = 1.0;

    cam.vfov = 20;
    cam.lookfrom = vec3(0, 2, 10);
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
