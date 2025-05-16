#include "Scene.hpp"
#include <chrono>
#include "s1_preprocessing.hpp"
#include "s2_modelling.hpp"
#include "s3_rendering.hpp"
#include "s4_postprocessing.hpp"

int main(int argc, char** argv)
{

    Scene scene(256, 256);

    if (argc>=2){
        scene.width = (int)atoi(argv[1]);
        scene.height = (int)atoi(argv[1]);
    }
    else{
        scene.width = 256;
        scene.height = 256;
    }

    if (argc>=3){
        scene.spp = (int)atoi(argv[1]);
    }
    else{
        scene.spp = 32;
    }
    // Stage 1
    s1_preprocessing(); // Do Nothing

    // Stage 2
    s2_modelling(scene);

    // Stage 3
    auto start = std::chrono::system_clock::now();
    s3_rendering(scene);
    auto stop = std::chrono::system_clock::now();

    // Stage 4
    s4_postprocessing(); // Do Nothing

    
    std::cout << "Render complete: \n";
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << " hours\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() << " minutes\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() << " seconds\n";

    return 0;
}