#include "global.h"
#include "camera.h"
#include "triangle.h"
#include "sphere.h"
#include "timer.h"
#include "objloader.h"

int main()
{
    hittable_list world;

    auto ground_material = make_shared<lambertian>(Vector3f(0.7, 0.7, 0.7));
    world.add(make_shared<sphere>(Vector3f(0, -1000, 0), 1000, ground_material));

    {
        auto loader = make_shared<ObjLoader>();
        loader->read_obj("../model/desktop_table.obj", "../model/r.png");

        for (size_t i = 0; i < loader->triangles.size(); i++)
            world.add(loader->triangles[i]);
    }

    {
        auto m1 = make_shared<metal>(Vector3f(0.7, 0.6, 0.5), 0.0);
        world.add(make_shared<sphere>(Vector3f(-2, 2, 1), 1, m1));
    }

    {
        auto image_data = cv::imread("../model/r.png");
        cv::cvtColor(image_data, image_data, cv::COLOR_RGB2BGR);
        auto m2 = make_shared<textured_lambertian>(image_data);
        world.add(make_shared<triangle>(
            Vector3f(-3, 1, 0),
            Vector3f(0, 3, 0),
            Vector3f(0, 1, -3),
            Vector2f(0, 0),
            Vector2f(0, 1),
            Vector2f(1, 1),
            m2));
    }

    camera cam;

    // cam.aspect_ratio      = 16.0 / 9.0;
    cam.aspect_ratio = 1.0 / 1.0;
    // cam.image_width       = 1200;
    cam.image_width = 512;
    // cam.samples_per_pixel = 10;
    cam.samples_per_pixel = 10;
    // cam.max_depth         = 20;
    cam.max_depth = 5;

    cam.vfov = 20;
    cam.lookfrom = Vector3f(10, 2, 3);
    cam.lookat = Vector3f(0, 1, 0);
    cam.vup = Vector3f(0, 1, 0);

    // cam.defocus_angle = 0.6;
    cam.defocus_angle = 0;
    cam.focus_dist = 10.0;

    ScopedTimer timer("Render time: ");
    cam.render(world, true);
    timer.stop_timer();

    cam.save_image("output.png");
    cam.display_image(0);
}
