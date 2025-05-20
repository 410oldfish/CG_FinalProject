# pragma one
#include "Scene.hpp"
#include "Triangle.hpp"
#include "Sphere.hpp"
#include <opencv2/opencv.hpp>
#include <global.hpp>
#include <Eigen/Dense>
#include <map>
#include <string>


inline void s2_modelling(Scene& scene, std::vector<Material*>& loaded_materials, 
    std::map<std::string, void * >& opened_images)
{

    // ================================== MATERIALS ==================================

    // // Diffuse Materials
    // Material* pink = new Material();
    // loaded_materials.push_back(pink);
    // pink->rho_map_implicit = [](Vector2f uv) {return Vector3f(0.72f, 0.48f, 0.56f);};
    // pink->kd_map_implicit = [](Vector2f uv) {return Vector3f(0.8f, 0.8f, 0.8f);};
    // pink->ks_map_implicit = [](Vector2f uv) {return Vector3f(0.2f, 0.2f, 0.2f);};
    // pink->ior_map_implicit = [](Vector2f uv) {return 1.46f;};
    // pink->opaqueness_map_implicit = [](Vector2f uv) {return 1.f;};


    // Material* blue = new Material();
    // loaded_materials.push_back(blue);
    // blue->rho_map_implicit = [](Vector2f uv) {return Vector3f(0.2f, 0.6f, 0.86f);};
    // blue->kd_map_implicit = [](Vector2f uv) {return Vector3f(1.0f, 1.0f, 1.0f);};
    // blue->ks_map_implicit = [](Vector2f uv) {return Vector3f(0.0f, 0.0f, 0.0f);};
    // blue->ior_map_implicit = [](Vector2f uv) {return 1.46f;};
    // blue->opaqueness_map_implicit = [](Vector2f uv) {return 1.f;};


    // Material* green = new Material();
    // loaded_materials.push_back(green);
    // green->rho_map_implicit = [](Vector2f uv) {return Vector3f(0.5f, 0.7f, 0.13f);};
    // green->kd_map_implicit = [](Vector2f uv) {return Vector3f(0.8f, 0.8f, 0.8f);};
    // green->ks_map_implicit = [](Vector2f uv) {return Vector3f(0.2f, 0.2f, 0.2f);};
    // green->ior_map_implicit = [](Vector2f uv) {return 1.46f;};
    // green->opaqueness_map_implicit = [](Vector2f uv) {return 1.f;};


    Material* white = new Material();
    loaded_materials.push_back(white);
    white->rho_map_implicit = [](Vector2f uv) {return Vector3f(0.48f, 0.45f, 0.4f);}; // always return white
    white->kd_map_implicit = [](Vector2f uv) {return 0.8f * Vector3f(1);};
    white->ks_map_implicit = [](Vector2f uv) {return 0.2f * Vector3f(1);};
    white->ior_map_implicit = [](Vector2f uv) {return 5.f;};
    white->opaqueness_map_implicit = [](Vector2f uv) {return 1.f;};


    // Material* red = new Material();
    // loaded_materials.push_back(red);
    // red->rho_map_implicit = [](Vector2f uv) {return Vector3f(0.8f, 0.2f, 0.0f);}; // always return red
    // red->kd_map_implicit = [](Vector2f uv) {return Vector3f(0.8f, 0.8f, 0.8f);};
    // red->ks_map_implicit = [](Vector2f uv) {return Vector3f(0.2f, 0.2f, 0.2f);};
    // red->ior_map_implicit = [](Vector2f uv) {return 1.46f;};
    // red->opaqueness_map_implicit = [](Vector2f uv) {return 1.f;};
    

    // Glass Material
    // std::shared_ptr<Material> glass = std::make_shared<Material>(GLASS, Vector3f(0.9f, 0.9f, 0.9f));
    Material* glass = new Material();
    loaded_materials.push_back(glass);
    glass->rho_map_implicit = [](Vector2f uv) {return Vector3f(0.9f, 0.9f, 0.9f);}; // always return glass
    glass->kd_map_implicit = [](Vector2f uv) {return Vector3f(0.0f, 0.0f, 0.0f);}; // no diffuse reflection
    glass->ks_map_implicit = [](Vector2f uv) {return Vector3f(1.0f, 1.0f, 1.0f);}; // full specular reflection
    glass->ior_map_implicit = [](Vector2f uv) {return 1.5f;}; // refractive index
    glass->opaqueness_map_implicit = [](Vector2f uv) {return 0.0f;}; // fully transparent

    

    // // std::shared_ptr<Material> mirror = std::make_shared<Material>(GLASS, Vector3f(0.9f, 0.9f, 0.9f));
    // Material* mirror = new Material();
    // loaded_materials.push_back(mirror);
    // mirror->rho_map_implicit = [](Vector2f uv) {return Vector3f(0.9f, 0.9f, 0.9f);}; // always return mirror
    // mirror->kd_map_implicit = [](Vector2f uv) {return Vector3f(0.0f, 0.0f, 0.0f);}; // no diffuse reflection
    // mirror->ks_map_implicit = [](Vector2f uv) {return Vector3f(1.0f, 1.0f, 1.0f);}; // full specular reflection
    // mirror->ior_map_implicit = [](Vector2f uv) {return 100.f;}; // refractive index
    // mirror->opaqueness_map_implicit = [](Vector2f uv) {return 1.0f;}; // fully opaque
    

    // // Emissive Material
    // // std::shared_ptr<Material> light = std::make_shared<Material>(EMIT, Vector3f(1));
    // Material* light = new Material();
    // loaded_materials.push_back(light);
    // // light->rho_map_implicit = [](Vector2f uv) {return Vector3f(0.8,0.8,0.8);}; // always return light
    // light->m_type = EMIT;
    // // Yellow light
    // light->m_emission= 60 * Vector3f(1.f, 1.f, 1.f); // emission color of the light
    // // light->kd_map_implicit = [](Vector2f uv) {return Vector3f(0.8f, 0.8f, 0.8f);};
    // // light->ks_map_implicit = [](Vector2f uv) {return Vector3f(0.2f, 0.2f, 0.2f);};
    // // light->ior_map_implicit = [](Vector2f uv) {return 1.5f;};
    // // light->opaqueness_map_implicit = [](Vector2f uv) {return 1.f;}; // fully opaque
    


    // std::shared_ptr<Material> frosted_glass = std::make_shared<Material>(GLASS, Vector3f(0.5f, 0.7f, 0.13f));
    Material* frosted_glass = new Material();
    frosted_glass->rho_map_implicit = [](Vector2f uv) {return Vector3f(0.5f, 0.7f, 0.13f);}; // always return frosted glass
    loaded_materials.push_back(frosted_glass);
    frosted_glass->kd_map_implicit = [](Vector2f uv) {return 0.2f * Vector3f(1);}; // no diffuse reflection
    frosted_glass->ks_map_implicit = [](Vector2f uv) {return 0.8f * Vector3f(1);}; // full specular reflection
    frosted_glass->ior_map_implicit = [](Vector2f uv) {return 1.5f;}; // refractive index
    frosted_glass->opaqueness_map_implicit = [](Vector2f uv) {return 0.2f;}; // fully transparent



    // // std::shared_ptr<Material> frosted_glass = std::make_shared<Material>(GLASS, Vector3f(0.5f, 0.7f, 0.13f));
    // Material* pink_glass = new Material();
    // pink_glass->rho_map_implicit = [](Vector2f uv) {return Vector3f(0.5f, 0.7f, 0.13f);}; // always return frosted glass
    // loaded_materials.push_back(frosted_glass);
    // pink_glass->kd_map_implicit = [](Vector2f uv) {return 0.1f * Vector3f(1);}; // no diffuse reflection
    // pink_glass->ks_map_implicit = [](Vector2f uv) {return 0.9f * Vector3f(1);}; // full specular reflection
    // pink_glass->ior_map_implicit = [](Vector2f uv) {return 1.5f;}; // refractive index
    // pink_glass->opaqueness_map_implicit = [](Vector2f uv) {return 0.2f;}; // fully transparent



    // // std::shared_ptr<Material> gold = std::make_shared<Material>(GLASS, Vector3f(255.f/255.f, 215.f/255.f, 0.0f));
    // Material* gold = new Material();
    // loaded_materials.push_back(gold);
    // gold->rho_map_implicit = [](Vector2f uv) {return Vector3f(1.f, 1.f, 0.0f);}; // always return gold
    // gold->kd_map_implicit = [](Vector2f uv) {return Vector3f(0.5f, 0.5f, 0.5f);}; // no diffuse reflection
    // gold->ks_map_implicit = [](Vector2f uv) {return Vector3f(0.5f, 0.5f, 0.5f);}; // full specular reflection
    // gold->ior_map_implicit = [](Vector2f uv) {return 100.f;}; // refractive index
    // gold->opaqueness_map_implicit = [](Vector2f uv) {return 1.f;}; // fully opaque
    

    // // std::shared_ptr<Material> stone = std::make_shared<Material>(DIFFUSE, Vector3f(0.2f, 0.6f, 0.86f));
    // Material* stone = new Material();
    // loaded_materials.push_back(stone);
    // stone->rho_map_implicit = [](Vector2f uv) {return Vector3f(0.2f, 0.6f, 0.86f);}; // always return stone
    // stone->kd_map_implicit = [](Vector2f uv) {return Vector3f(1.0f, 1.0f, 1.0f);};
    // stone->ks_map_implicit = [](Vector2f uv) {return Vector3f(0.0f, 0.0f, 0.0f);};
    // stone->ior_map_implicit = [](Vector2f uv) {return 1.5f;};
    // stone->opaqueness_map_implicit = [](Vector2f uv) {return 1.f;}; // fully opaque
    


    // Material* yellow = new Material(DIFFUSE, Vector3f(1, 1, 1));
    // loaded_materials.push_back(yellow);
    // yellow->rho_map = loadImageAsMatrix("../models/bob-the-duck/bob_diffuse.png");
    // yellow->kd_map_implicit = [](Vector2f uv) {return Vector3f(0.8f, 0.8f, 0.8f);};
    // yellow->ks_map_implicit = [](Vector2f uv) {return Vector3f(0.2f, 0.2f, 0.2f);};
    // yellow->ior_map_implicit = [](Vector2f uv) {return 1.5f;};
    // yellow->opaqueness_map_implicit = [](Vector2f uv) {return 1.f;}; // fully opaque



    // Material* mfloor = new Material(DIFFUSE, Vector3f(0));
    // loaded_materials.push_back(mfloor);


    // mfloor->rho_map_implicit = [](Vector2f uv) -> Vector3f {
    // float scale = 5;
    // float u = fmodf(uv.x * scale, 1.0f);
    // float v = fmodf(uv.y * scale, 1.0f);
    // bool pattern = (u > 0.5f) ^ (v > 0.5f); // XOR checker
    // // Return two alternating colors
    // return pattern
    //     ? Vector3f(0.815f, 0.235f, 0.031f)
    //     : Vector3f(0.937f, 0.937f, 0.231f);
    // };

    // // UV
    // mfloor->rho_map_implicit = [](Vector2f uv) {
    // return Vector3f(uv.x, uv.y, 1.0f - uv.x);
    // };

    // // // Radar
    // mfloor->rho_map_implicit = [](Vector2f uv) {
    //     Vector2f center(0.5f, 0.5f);
    //     float dist = (uv - center).norm(); // Euclidean distance
    //     return lerp(Vector3f(1, 1, 1), Vector3f(0, 0, 0), dist * 2.0f); // white center to black edge
    // };

    // Stripes
    // mfloor->rho_map_implicit = [](Vector2f uv) {
    // float stripes = sin(uv.x * 40.0f);
    // return stripes > 0 ? Vector3f(0.7, 0.3, 0.3) : Vector3f(0.3, 0.3, 0.7);
    // };







    // ================================== LIGHTS ===============================

    Material* lightUpMaterial = new Material();
    loaded_materials.push_back(lightUpMaterial);
    lightUpMaterial->m_type = EMIT;
    lightUpMaterial->m_emission= 60 * Vector3f(1.f, 1.f, 1.f);
    std::unique_ptr<MeshTriangle> lightUp = std::make_unique<MeshTriangle>("../models/LightUp.obj", Vector3f(0), lightUpMaterial, loaded_materials, opened_images);

    Material* lightRightMaterial = new Material();
    loaded_materials.push_back(lightRightMaterial);
    lightRightMaterial->m_type = EMIT;
    lightRightMaterial->m_emission= 10 * Vector3f(1.f, 1.f, 1.f);
    std::unique_ptr<MeshTriangle> lightRight = std::make_unique<MeshTriangle>("../models/LightRight.obj", Vector3f(0), lightRightMaterial, loaded_materials, opened_images);
    

    // Material* lightLeftMaterial = new Material();
    // loaded_materials.push_back(lightLeftMaterial);
    // lightLeftMaterial->m_type = EMIT;
    // lightLeftMaterial->m_emission= 10 * Vector3f(1.f, 1.f, 1.f);
    // std::unique_ptr<MeshTriangle> lightLeft = std::make_unique<MeshTriangle>("../models/LightLeft.obj", Vector3f(0), lightLeftMaterial, loaded_materials, opened_images);

    Material* lightLampMaterial = new Material();
    loaded_materials.push_back(lightLampMaterial);
    lightLampMaterial->m_type = EMIT;
    lightLampMaterial->m_emission= 5 * Vector3f(1.f, 1.f, 1.f);
    std::unique_ptr<MeshTriangle> lightLamp = std::make_unique<MeshTriangle>("../models/LightLamp.obj", Vector3f(0), lightLampMaterial, loaded_materials, opened_images);


    Material* sun1Material = new Material();
    loaded_materials.push_back(sun1Material);
    sun1Material->m_type = EMIT;
    sun1Material->m_emission= 30 * Vector3f(1.f, 1.f, 1.f);
    std::unique_ptr<MeshTriangle> sun1 = std::make_unique<MeshTriangle>("../models/Sun1.obj", Vector3f(0), sun1Material, loaded_materials, opened_images);

    Material* sun2Material = new Material();
    loaded_materials.push_back(sun2Material);
    sun2Material->m_type = EMIT;
    sun2Material->m_emission= 30 * Vector3f(1.f, 1.f, 1.f);
    std::unique_ptr<MeshTriangle> sun2 = std::make_unique<MeshTriangle>("../models/Sun2.obj", Vector3f(0), sun2Material, loaded_materials, opened_images);

    
    Material* sunBigMaterial = new Material();
    loaded_materials.push_back(sunBigMaterial);
    sunBigMaterial->m_type = EMIT;
    sunBigMaterial->m_emission= 200 * Vector3f(1.f, 1.f, 1.f);
    std::unique_ptr<MeshTriangle> sunBig = std::make_unique<MeshTriangle>("../models/SunBig.obj", Vector3f(0), sunBigMaterial, loaded_materials, opened_images);

    // =========================== OBJECTS ============================
    

    std::unique_ptr<MeshTriangle> room = std::make_unique<MeshTriangle>("../models/room_teto.obj", Vector3f(0), white, loaded_materials, opened_images);
    // std::unique_ptr<MeshTriangle> room = std::make_unique<MeshTriangle>("../models/room_nobody_here.obj", Vector3f(0), white, loaded_materials, opened_images);



    scene.Add(std::move(room));
    scene.Add(std::move(lightUp));
    scene.Add(std::move(sun1));
    scene.Add(std::move(sun2));
    scene.Add(std::move(sunBig));
    scene.Add(std::move(lightRight));
    scene.Add(std::move(lightLamp));





    // ============================= SPHERES ============================


    std::unique_ptr<Sphere> green_sphere = std::make_unique<Sphere>(Vector3f(200, 60, 400), 60, glass);
    scene.Add(std::move(green_sphere));



    Material* mfloor = new Material(DIFFUSE, Vector3f(0.0f, 0.0f, 0.0f));
    loaded_materials.push_back(mfloor);


    mfloor->rho_map_implicit = [](Vector2f uv) -> Vector3f {
    float scale = 5;
    float u = fmodf(uv.x * scale, 1.0f);
    float v = fmodf(uv.y * scale, 1.0f);
    bool pattern = (u > 0.5f) ^ (v > 0.5f); // XOR checker
    // Return two alternating colors
    return pattern
        ? Vector3f(0.815f, 0.235f, 0.031f)
        : Vector3f(0.937f, 0.937f, 0.231f);
    };

    // // UV
    // mfloor->rho_map_implicit = [](Vector2f uv) {
    // return Vector3f(uv.x, uv.y, 1.0f - uv.x);
    // };

    // // // Radar
    // mfloor->rho_map_implicit = [](Vector2f uv) {
    //     Vector2f center(0.5f, 0.5f);
    //     float dist = (uv - center).norm(); // Euclidean distance
    //     return lerp(Vector3f(1, 1, 1), Vector3f(0, 0, 0), dist * 2.0f); // white center to black edge
    // };

    // Stripes
    // mfloor->rho_map_implicit = [](Vector2f uv) {
    // float stripes = sin(uv.x * 40.0f);
    // return stripes > 0 ? Vector3f(0.7, 0.3, 0.3) : Vector3f(0.3, 0.3, 0.7);
    // };


    // mfloor->rho_map_implicit = [](Vector2f uv) -> Vector3f {
    // uv = uv * 10.0f; // scale UVs for tiling

    // // Simple wave-based displacement
    // float wave1 = std::sin(uv.x + std::sin(uv.y));
    // float wave2 = std::cos(uv.y + std::cos(uv.x * 1.5f));
    // float ripples = std::sin(10.0f * (uv.x * uv.y));

    // // Combine and normalize
    // float height = 0.5f * wave1 + 0.5f * wave2 + 0.2f * ripples;
    // height = 0.5f + 0.5f * height; // map to [0, 1]

    // // Optional color modulation (blue water + specular highlight)
    // Vector3f waterColor = Vector3f(0.0f, 0.4f, 0.7f);       // deep water
    // Vector3f highlight  = Vector3f(0.8f, 0.9f, 1.0f);       // caustic shimmer
    // return lerp(waterColor, highlight, height);
    // };



//     mfloor->rho_map_implicit = [](Vector2f uv) -> Vector3f {
//     // 1) Tile frequency
//     uv = uv * 10.0f;

//     // 2) Wave-based height field (unchanged)
//     float wave1  = std::sin(uv.x + std::sin(uv.y));
//     float wave2  = std::cos(uv.y + std::cos(uv.x * 1.5f));
//     float ripples = std::sin(10.0f * (uv.x * uv.y));
//     float height = 0.5f * wave1 + 0.5f * wave2 + 0.2f * ripples;
//     height = 0.5f + 0.5f * height;  // map into [0,1]

//     // 3) Pastel pink → white
//     Vector3f pink   = Vector3f(1.0f, 0.80f, 0.90f);  // a soft rose-pink
//     Vector3f white  = Vector3f(1.0f, 1.00f, 1.00f);  // pure white

//     // 4) Blend based on wave height
//     return lerp(pink, white, height);
// };

    // 1) Polka Dots
        mfloor->rho_map_implicit = [](Vector2f uv) -> Vector3f {
        // 1) Frequency: how many dots per axis
        uv = uv * 8.0f;

        // 2) Colours
        Vector3f bg = Vector3f(1.0f, 0.9f, 0.95f);  // pale pink background
        Vector3f fg = Vector3f(1.0f, 0.6f, 0.8f);   // bubblegum-pink dots

        // 3) Find local position inside each cell
        Vector2f f = fract(uv) - Vector2f(0.5f, 0.5f);

        // 4) Radius test
        float radius = 0.3f;
        float dist = std::sqrt(dot(f, f));
        return (dist < radius) ? fg : bg;
    };


    mfloor->kd_map_implicit = [](Vector2f uv) {return Vector3f(0.8f, 0.8f, 0.8f);};
    mfloor->ks_map_implicit = [](Vector2f uv) {return Vector3f(0.2f, 0.2f, 0.2f);};
    mfloor->ior_map_implicit = [](Vector2f uv) {return 2.f;};
    mfloor->opaqueness_map_implicit = [](Vector2f uv) {return 1.f;}; // fully opaque




    // Build the floor manually
    // Define the 4 vertices of a large rectangle floor plane, lower left corner at (0,0,0), 550x560
    Vector3f verts[4] = {{0,3,0}, {552.8,3,0}, {549.6, 3,559.2}, {0,3,559.2}};
    // Define texture coordinates for the 4 vertices
    Vector2f st[4] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
    // Define the 6 indices for the two triangles that make up the rectangle
    // 3 ---- 2
    // |    / |
    // |  /   |
    // 0 ---- 1
    uint32_t vertIndex[6] = {0, 2, 1, 2,0,3};
    // Diffuse Material, black color (will not be used)
    // std::shared_ptr<Material> mfloor = std::make_shared<Material>(DIFFUSE, Vector3f(0));

    // Add the two triangles to the scene
    scene.Add(std::make_unique<MeshTriangle>(verts, vertIndex, 2, st, mfloor));

}