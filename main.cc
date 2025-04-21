#include "global.h"
#include "camera.h"
#include "triangle.h"
#include "sphere.h"

int main()
{
    hittable_list world;

    auto ground_material = make_shared<lambertian>(Vector3f(0.7, 0.7, 0.7));
    world.add(make_shared<sphere>(Vector3f(0, -1000, 0), 1000, ground_material));

    auto m1 = make_shared<metal>(Vector3f(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<sphere>(Vector3f(-2, 2, 1), 1, m1));

    auto image_data = cv::imread("../model/texture.png");
    cv::cvtColor(image_data, image_data, cv::COLOR_RGB2BGR);
    image_data = image_data / 255;
    auto m2 = make_shared<textured_lambertian>(image_data);
    world.add(make_shared<triangle>(
        Vector3f(-3, 1, 0),
        Vector3f(0, 3, 0),
        Vector3f(0, 1, -3),
        Vector2f(0, 0),
        Vector2f(0, 1),
        Vector2f(1, 1),
        m2));

    for (int a = -11; a < 11; a++)
    {
        for (int b = -11; b < 11; b++)
        {
            auto choose_mat = random_double();
            Vector3f center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

            if ((center - Vector3f(4, 0.2, 0)).norm() > 0.9)
            {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.8)
                {
                    // diffuse
                    auto albedo = Vector3f(random_double(), random_double(), random_double());
                    sphere_material = make_shared<lambertian>(albedo);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
                else
                {
                    // metal
                    auto albedo = Vector3f(random_double(0.5, 1), random_double(0.5, 1), random_double(0.5, 1));
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
    cam.lookfrom = Vector3f(10, 2, 3);
    cam.lookat = Vector3f(0, 1, 0);
    cam.vup = Vector3f(0, 1, 0);

    // cam.defocus_angle = 0.6;
    cam.defocus_angle = 0;
    cam.focus_dist = 10.0;

    cam.render(world);
}
