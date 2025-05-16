//
// Created by goksu on 2/25/20.
//

#include <fstream>
#include <sstream>
#include "Scene.hpp"
#include "Renderer.hpp"
#include "Material.hpp"
#ifdef _OPENMP
    #include <omp.h>
#endif

// @param deg: angle in degrees
// @return: angle in radians
inline float deg2rad(const float& deg) { return deg * M_PI / 180.0; }

// A small constant used to offset rays slightly to prevent 
// self-intersections due to floating point inaccuracies (used elsewhere)
const float EPSILON = 1e-2;
const float SMALL_EPSILON = 1e-6f;



// The main render function. This where we iterate over all pixels in the image,
// generate primary rays and cast these rays into the scene. The content of the
// framebuffer is saved to a file.
// 
// @param scene: the scene to be rendered
void Renderer::Render(const Scene& scene)
{
    // Initialise a framebuffer
    std::vector<Vector3f> framebuffer(scene.width * scene.height);

    // Assume in screen space, the camera is at (0,0,1) and the screen is at (0,0,0)
    // The camera is looking down the +z axis.
    // Scale is the half-height of the screen at z = 1
    float scale = tan(deg2rad(scene.fov * 0.5));

    // The aspect ratio of the image
    float imageAspectRatio = scene.width / (float)scene.height;

    // The camera is behind the Cornell box which is centered at z = 0
    Vector3f eye_pos(278, 273, -800);

    // Print out samples per pixel
    std::cout << "SPP: " << scene.spp << "\n";

    // Record the progress of the rendering
    float progress = 0.0f;

    #pragma omp parallel for num_threads(8) // use multi-threading for speedup if openmp is available
    // Each thread will process one row of the images
    for (uint32_t j = 0; j < scene.height; ++j) {
        for (uint32_t i = 0; i < scene.width; ++i) {
            // Loop through each pixel in the image

            // Compute the flatten 1D index of the current pixel in the framebuffer
            int m = i + j * scene.width;


            if(scene.spp==1){
                // i (x) is going from left to right
                // j (y) is going from top to bottom
                // We want x to go from right to left,
                // y to go from bottom to top,
                // and z to go from forward to backward

                // TODO: task 1.2 pixel projection
                // use a right-hand coordinate system where +x is left, +y is up and +z is forward

                float offset_x = 0.5f;
                float offset_y = 0.5f;

                // Normalised Device Coordinates (NDC) in the range [0, 1]
                float ndc_x = (i + offset_x) / scene.width;
                float ndc_y = (j + offset_y) / scene.height;

                // NDC in the range [-1, 1]
                ndc_x = 2.0f * ndc_x - 1.0f;
                ndc_y = 2.0f * ndc_y - 1.0f;

                // The camera-space coordinates of the pixel on the z=1 plane
                float px = - ndc_x * scale * imageAspectRatio;
                float py = - ndc_y * scale;

                // The ray direction in the camera space, i.e., originating at (0,0,0) with a direction of (px, py, 1)
                Vector3f dir_screen = Vector3f(px, py, 1.0f).normalized();

                // The ray direction in the world space, i.e., originating at (eye_pos) with a direction of (dir)
                // The camera’s “look” axes are aligned with the world axes, no extra rotation is needed
                Ray ray_world(eye_pos, dir_screen);

                // Cast the ray into the scene and get the color at the intersection point
                framebuffer[m] = scene.castRay(ray_world, 0, false);

            }else {
                // TODO: task 4 multi-sampling
                // use a right-hand coordinate system where +x is left, +y is up and +z is forward
                // use scene.spp to determine the number of samples per pixel

                Vector3f colour(0.f);

                // Loop through each sample in the pixel
                // Sample each pixel scene.spp times
                for (int s = 0; s < scene.spp; ++s) {

                    float offset_x = get_random_float();
                    float offset_y = get_random_float();

                    // Normalised Device Coordinates (NDC) in the range [0, 1]
                    float ndc_x = (i + offset_x) / scene.width;
                    float ndc_y = (j + offset_y) / scene.height;

                    // NDC in the range [-1, 1]
                    ndc_x = 2.0f * ndc_x - 1.0f;
                    ndc_y = 2.0f * ndc_y - 1.0f;

                    // The camera-space coordinates of the pixel on the z=1 plane
                    float px = - ndc_x * scale * imageAspectRatio;
                    float py = - ndc_y * scale;

                    // The ray direction in the camera space, i.e., originating at (0,0,0) with a direction of (px, py, 1)
                    Vector3f dir_screen = Vector3f(px, py, 1.0f).normalized();

                    // The ray direction in the world space, i.e., originating at (eye_pos) with a direction of (dir)
                    // The camera’s “look” axes are aligned with the world axes, no extra rotation is needed
                    Ray ray_world(eye_pos, dir_screen);

                    // Cast the ray into the scene and get the color at the intersection point
                    colour += scene.castRay(ray_world, 0, false);
                }
                // Average the colour
                framebuffer[m] = colour / (float)scene.spp;
            }
        }
        progress += 1.0f / (float)scene.height;
        UpdateProgress(progress);
    }
    UpdateProgress(1.f);

    // ========================== Save the image ========================== //

    // Create a filename like "binary_task1.ppm"
    std::stringstream ss;
    ss << "binary_task" << TASK_N<<".ppm";
    std::string str = ss.str();
    const char* file_name = str.c_str();

    // Open the file for writing
    // P6 means binary PPM, 255 means max color value
    FILE* fp = fopen(file_name, "wb");
    (void)fprintf(fp, "P6\n%d %d\n255\n", scene.width, scene.height);

    // Write the pixel data to the file
    for (auto i = 0; i < scene.height * scene.width; ++i) {
        static unsigned char color[3];
        // Convert from [0,1] to [0,255]
        // Apply gamma correction ^0.6 to brighten dark areas
        color[0] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].x), 0.6f));
        color[1] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].y), 0.6f));
        color[2] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].z), 0.6f));
        fwrite(color, 1, 3, fp);
    }
    fclose(fp);
}
