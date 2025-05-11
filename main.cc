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

    {
        auto m1 = make_shared<metal>(vec3(0.7, 0.6, 0.5), 0.0);
        world.add(make_shared<sphere>(vec3(-2, 2, 1), 1, m1));
    }

    // {
    //     auto image_data = cv::imread("../model/texture.png");
    //     cv::cvtColor(image_data, image_data, cv::COLOR_RGB2BGR);
    //     image_data = image_data / 255;
    //     auto m2 = make_shared<textured_lambertian>(image_data);
    //     world.add(make_shared<triangle>(
    //         vertex(vec3(-3, 1, 0)),
    //         vertex(vec3(0, 3, 0)),
    //         vertex(vec3(0, 1, -3)),
    //         vec3(0, 0, 0),
    //         m2));
    // }

    for (int a = -11; a < 11; a++)
    {
        for (int b = -11; b < 11; b++)
        {
            auto choose_mat = random_double();
            vec3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

            if ((center - vec3(4, 0.2, 0)).length() > 0.9)
            {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.8)
                {
                    // diffuse
                    auto albedo = vec3(random_double(), random_double(), random_double());
                    sphere_material = make_shared<lambertian>(albedo);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
                else
                {
                    // metal
                    auto albedo = vec3(random_double(0.5, 1), random_double(0.5, 1), random_double(0.5, 1));
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
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
    cam.lookfrom = vec3(10, 2, 3);
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
}
