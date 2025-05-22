#include "Scene.hpp"
#include <chrono>
#include "s1_preprocessing.hpp"
#include "s2_modelling.hpp"
#include "s3_rendering.hpp"
#include "s4_postprocessing.hpp"
#include "Material.hpp"

int main(int argc, char** argv)
{

    Scene scene(256, 256);

    if (argc>=2){
        scene.width = static_cast<int>(4.0f / 3.0f * (int)atoi(argv[1]));
        scene.height = (int)atoi(argv[1]);
    }
    else{
        scene.width = static_cast<int>(4.0f / 3.0f * 256);
        scene.height = 256;
    }

    if (argc>=3){
        scene.spp = (int)atoi(argv[2]); 
    }
    else{
        scene.spp = 32;
    }
    // Stage 1

    std::cout << "Resolution: " << scene.width << " x " << scene.height << "\n";

    auto start = std::chrono::system_clock::now();

    s1_preprocessing(); // Do Nothing

    std::vector<Material*> materials;
    std::map<std::string, void * > opened_images;

    // Stage 2
    s2_modelling(scene, materials, opened_images);

    // Stage 3
    s3_rendering(scene);

    // Stage 4
    s4_postprocessing(); // Do Nothing

    auto stop = std::chrono::system_clock::now();

    
    std::cout << "Render complete: \n";
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << " hours\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() << " minutes\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() << " seconds\n";


    for (auto& material : materials) {
        delete material; // Free the memory allocated for each material
    }


    return 0;
}