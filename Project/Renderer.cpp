//
// Created by goksu on 2/25/20.
//

#include <fstream>
#include <sstream>
#include "Scene.hpp"
#include "Renderer.hpp"
#include "pcg.h"
#include "parallel.h"
#include "progress_reporter.h"
#include "Material.hpp"
#ifdef _OPENMP
    #include <omp.h>
#endif


inline float deg2rad(const float& deg) { return deg * M_PI / 180.0; }

const float EPSILON = 1e-2;

// The main render function. This where we iterate over all pixels in the image,
// generate primary rays and cast these rays into the scene. The content of the
// framebuffer is saved to a file.
void Renderer::Render(const Scene& scene)
{
    std::vector<Vector3f> framebuffer(scene.width * scene.height);

    float scale = tan(deg2rad(scene.fov * 0.5));
    float imageAspectRatio = scene.width / (float)scene.height;
    Vector3f eye_pos(278, 273, -800);

    std::cout << "SPP: " << scene.spp << "\n";

    float progress = 0.0f;

    constexpr int tile_size = 16;
    int num_tiles_x = (IMAGE_W + tile_size - 1) / tile_size;
    int num_tiles_y = (IMAGE_H + tile_size - 1) / tile_size;
    auto totalProgress = num_tiles_x * num_tiles_y;

    parallel_for([&](const Vector2i &tile) {
        pcg32_state rng = init_pcg32(tile[1] * num_tiles_x + tile[0]);
        int x0 = tile[0] * tile_size;
        int x1 = min(x0 + tile_size, IMAGE_W);
        int y0 = tile[1] * tile_size;
        int y1 = min(y0 + tile_size, IMAGE_H);

        for (uint32_t j = y0; j < y1; ++j) {
            for (uint32_t i = x0; i < x1; ++i) {

                int m = i + j * scene.width;
                Vector3f total_buffer = Vector3f(0,0,0);
                    for ( int k =0;k<scene.spp;k++) {
                        float offset_x = next_pcg32_real<double>(rng);
                        float offset_y = next_pcg32_real<double>(rng);
                        float x = (2 * (i + offset_x) / (float)scene.width - 1)* scale * imageAspectRatio;
                        float y = (1 - 2 * (j + offset_y) / (float)scene.height) * scale;
                        Vector3f dir = normalize(Vector3f(-x, y, 1.0f));
                        total_buffer += scene.castRay(Ray(eye_pos, dir), 0, rng);
                    }

                    framebuffer[m] = total_buffer/(float)scene.spp;
            }
        }
        progress += 1.0f / (float)totalProgress;
        UpdateProgress(progress);

    }, Vector2i(num_tiles_x, num_tiles_y));
    UpdateProgress(1.f);

    // save framebuffer to file
    std::stringstream ss;
    ss << "binary_task" << "Final_Project"<<".ppm";
    std::string str = ss.str();
    const char* file_name = str.c_str();
    FILE* fp = fopen(file_name, "wb");
    (void)fprintf(fp, "P6\n%d %d\n255\n", scene.width, scene.height);
    for (auto i = 0; i < scene.height * scene.width; ++i) {
        static unsigned char color[3];
        color[0] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].x), 0.6f));
        color[1] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].y), 0.6f));
        color[2] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].z), 0.6f));
        fwrite(color, 1, 3, fp);
    }
    fclose(fp);
}
