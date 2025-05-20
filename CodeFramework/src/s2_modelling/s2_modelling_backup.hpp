# pragma one
#include "Scene.hpp"
#include "Triangle.hpp"
#include "Sphere.hpp"
#include <opencv2/opencv.hpp>
#include <global.hpp>


// inline Vector3f hsv2rgb(float h, float s, float v) {
//     float c = v * s;
//     float x = c * (1 - std::fabs(fmod(h * 6.0f, 2) - 1));
//     float m = v - c;
//     float r, g, b;
//     if      (h < 1.0/6) { r = c; g = x; b = 0; }
//     else if (h < 2.0/6) { r = x; g = c; b = 0; }
//     else if (h < 3.0/6) { r = 0; g = c; b = x; }
//     else if (h < 4.0/6) { r = 0; g = x; b = c; }
//     else if (h < 5.0/6) { r = x; g = 0; b = c; }
//     else                { r = c; g = 0; b = x; }
//     return Vector3f(r + m, g + m, b + m);
// }

inline void s2_modelling(Scene& scene, std::vector<Material*>& loaded_materials)
{

    // ======= Materials =======

    // Diffuse Materials
    Material* pink = new Material();
    loaded_materials.push_back(pink);
    pink->rho_map_implicit = [](Vector2f uv) {return Vector3f(0.72f, 0.48f, 0.56f);};
    pink->kd_map_implicit = [](Vector2f uv) {return Vector3f(0.8f, 0.8f, 0.8f);};
    pink->ks_map_implicit = [](Vector2f uv) {return Vector3f(0.2f, 0.2f, 0.2f);};
    pink->ior_map_implicit = [](Vector2f uv) {return 1.46f;};
    pink->opaqueness_map_implicit = [](Vector2f uv) {return 1.f;};


    Material* blue = new Material();
    loaded_materials.push_back(blue);
    blue->rho_map_implicit = [](Vector2f uv) {return Vector3f(0.2f, 0.6f, 0.86f);};
    blue->kd_map_implicit = [](Vector2f uv) {return Vector3f(1.0f, 1.0f, 1.0f);};
    blue->ks_map_implicit = [](Vector2f uv) {return Vector3f(0.0f, 0.0f, 0.0f);};
    blue->ior_map_implicit = [](Vector2f uv) {return 1.46f;};
    blue->opaqueness_map_implicit = [](Vector2f uv) {return 1.f;};


    Material* green = new Material();
    loaded_materials.push_back(green);
    green->rho_map_implicit = [](Vector2f uv) {return Vector3f(0.5f, 0.7f, 0.13f);};
    green->kd_map_implicit = [](Vector2f uv) {return Vector3f(0.8f, 0.8f, 0.8f);};
    green->ks_map_implicit = [](Vector2f uv) {return Vector3f(0.2f, 0.2f, 0.2f);};
    green->ior_map_implicit = [](Vector2f uv) {return 1.46f;};
    green->opaqueness_map_implicit = [](Vector2f uv) {return 1.f;};


    Material* white = new Material();
    loaded_materials.push_back(white);
    white->rho_map_implicit = [](Vector2f uv) {return Vector3f(0.48f, 0.45f, 0.4f);}; // always return white
    white->kd_map_implicit = [](Vector2f uv) {return 0.8f * Vector3f(1);};
    white->ks_map_implicit = [](Vector2f uv) {return 0.2f * Vector3f(1);};
    white->ior_map_implicit = [](Vector2f uv) {return 5.f;};
    white->opaqueness_map_implicit = [](Vector2f uv) {return 1.f;};


    Material* red = new Material();
    loaded_materials.push_back(red);
    red->rho_map_implicit = [](Vector2f uv) {return Vector3f(0.8f, 0.2f, 0.0f);}; // always return red
    red->kd_map_implicit = [](Vector2f uv) {return Vector3f(0.8f, 0.8f, 0.8f);};
    red->ks_map_implicit = [](Vector2f uv) {return Vector3f(0.2f, 0.2f, 0.2f);};
    red->ior_map_implicit = [](Vector2f uv) {return 1.46f;};
    red->opaqueness_map_implicit = [](Vector2f uv) {return 1.f;};
    

    // Glass Material
    // std::shared_ptr<Material> glass = std::make_shared<Material>(GLASS, Vector3f(0.9f, 0.9f, 0.9f));
    Material* glass = new Material();
    loaded_materials.push_back(glass);
    glass->rho_map_implicit = [](Vector2f uv) {return Vector3f(0.9f, 0.9f, 0.9f);}; // always return glass
    glass->kd_map_implicit = [](Vector2f uv) {return Vector3f(0.0f, 0.0f, 0.0f);}; // no diffuse reflection
    glass->ks_map_implicit = [](Vector2f uv) {return Vector3f(1.0f, 1.0f, 1.0f);}; // full specular reflection
    glass->ior_map_implicit = [](Vector2f uv) {return 1.5f;}; // refractive index
    glass->opaqueness_map_implicit = [](Vector2f uv) {return 0.0f;}; // fully transparent

    

    // std::shared_ptr<Material> mirror = std::make_shared<Material>(GLASS, Vector3f(0.9f, 0.9f, 0.9f));
    Material* mirror = new Material();
    loaded_materials.push_back(mirror);
    mirror->rho_map_implicit = [](Vector2f uv) {return Vector3f(0.9f, 0.9f, 0.9f);}; // always return mirror
    mirror->kd_map_implicit = [](Vector2f uv) {return Vector3f(0.0f, 0.0f, 0.0f);}; // no diffuse reflection
    mirror->ks_map_implicit = [](Vector2f uv) {return Vector3f(1.0f, 1.0f, 1.0f);}; // full specular reflection
    mirror->ior_map_implicit = [](Vector2f uv) {return 100.f;}; // refractive index
    mirror->opaqueness_map_implicit = [](Vector2f uv) {return 1.0f;}; // fully opaque
    

    // Emissive Material
    // std::shared_ptr<Material> light = std::make_shared<Material>(EMIT, Vector3f(1));
    Material* light = new Material();
    loaded_materials.push_back(light);
    // light->rho_map_implicit = [](Vector2f uv) {return Vector3f(0.8,0.8,0.8);}; // always return light
    light->m_type = EMIT;
    // Yellow light
    light->m_emission= 60 * Vector3f(1.f, 1.f, 1.f); // emission color of the light
    // light->kd_map_implicit = [](Vector2f uv) {return Vector3f(0.8f, 0.8f, 0.8f);};
    // light->ks_map_implicit = [](Vector2f uv) {return Vector3f(0.2f, 0.2f, 0.2f);};
    // light->ior_map_implicit = [](Vector2f uv) {return 1.5f;};
    // light->opaqueness_map_implicit = [](Vector2f uv) {return 1.f;}; // fully opaque
    


    // std::shared_ptr<Material> frosted_glass = std::make_shared<Material>(GLASS, Vector3f(0.5f, 0.7f, 0.13f));
    Material* frosted_glass = new Material();
    frosted_glass->rho_map_implicit = [](Vector2f uv) {return Vector3f(0.5f, 0.7f, 0.13f);}; // always return frosted glass
    loaded_materials.push_back(frosted_glass);
    frosted_glass->kd_map_implicit = [](Vector2f uv) {return 0.2f * Vector3f(1);}; // no diffuse reflection
    frosted_glass->ks_map_implicit = [](Vector2f uv) {return 0.8f * Vector3f(1);}; // full specular reflection
    frosted_glass->ior_map_implicit = [](Vector2f uv) {return 1.5f;}; // refractive index
    frosted_glass->opaqueness_map_implicit = [](Vector2f uv) {return 0.2f;}; // fully transparent


    // std::shared_ptr<Material> gold = std::make_shared<Material>(GLASS, Vector3f(255.f/255.f, 215.f/255.f, 0.0f));
    Material* gold = new Material();
    loaded_materials.push_back(gold);
    gold->rho_map_implicit = [](Vector2f uv) {return Vector3f(1.f, 1.f, 0.0f);}; // always return gold
    gold->kd_map_implicit = [](Vector2f uv) {return Vector3f(0.5f, 0.5f, 0.5f);}; // no diffuse reflection
    gold->ks_map_implicit = [](Vector2f uv) {return Vector3f(0.5f, 0.5f, 0.5f);}; // full specular reflection
    gold->ior_map_implicit = [](Vector2f uv) {return 100.f;}; // refractive index
    gold->opaqueness_map_implicit = [](Vector2f uv) {return 1.f;}; // fully opaque
    

    // std::shared_ptr<Material> stone = std::make_shared<Material>(DIFFUSE, Vector3f(0.2f, 0.6f, 0.86f));
    Material* stone = new Material();
    loaded_materials.push_back(stone);
    stone->rho_map_implicit = [](Vector2f uv) {return Vector3f(0.2f, 0.6f, 0.86f);}; // always return stone
    stone->kd_map_implicit = [](Vector2f uv) {return Vector3f(1.0f, 1.0f, 1.0f);};
    stone->ks_map_implicit = [](Vector2f uv) {return Vector3f(0.0f, 0.0f, 0.0f);};
    stone->ior_map_implicit = [](Vector2f uv) {return 1.5f;};
    stone->opaqueness_map_implicit = [](Vector2f uv) {return 1.f;}; // fully opaque
    


    Material* yellow = new Material(DIFFUSE, Vector3f(1, 1, 1));
    loaded_materials.push_back(yellow);
    yellow->rho_map = loadImageAsMatrix("../models/bob-the-duck/bob_diffuse.png");
    yellow->kd_map_implicit = [](Vector2f uv) {return Vector3f(0.8f, 0.8f, 0.8f);};
    yellow->ks_map_implicit = [](Vector2f uv) {return Vector3f(0.2f, 0.2f, 0.2f);};
    yellow->ior_map_implicit = [](Vector2f uv) {return 1.5f;};
    yellow->opaqueness_map_implicit = [](Vector2f uv) {return 1.f;}; // fully opaque



    Material* mfloor = new Material(DIFFUSE, Vector3f(0));
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


    mfloor->rho_map_implicit = [](Vector2f uv) -> Vector3f {
    uv = uv * 10.0f; // scale UVs for tiling

    // Simple wave-based displacement
    float wave1 = std::sin(uv.x + std::sin(uv.y));
    float wave2 = std::cos(uv.y + std::cos(uv.x * 1.5f));
    float ripples = std::sin(10.0f * (uv.x * uv.y));

    // Combine and normalize
    float height = 0.5f * wave1 + 0.5f * wave2 + 0.2f * ripples;
    height = 0.5f + 0.5f * height; // map to [0, 1]

    // Optional color modulation (blue water + specular highlight)
    Vector3f waterColor = Vector3f(0.0f, 0.4f, 0.7f);       // deep water
    Vector3f highlight  = Vector3f(0.8f, 0.9f, 1.0f);       // caustic shimmer
    return lerp(waterColor, highlight, height);
    };


    mfloor->kd_map_implicit = [](Vector2f uv) {return Vector3f(0.8f, 0.8f, 0.8f);};
    mfloor->ks_map_implicit = [](Vector2f uv) {return Vector3f(0.2f, 0.2f, 0.2f);};
    mfloor->ior_map_implicit = [](Vector2f uv) {return 2.f;};
    mfloor->opaqueness_map_implicit = [](Vector2f uv) {return 1.f;}; // fully opaque



    // ======= Objects =======

    std::unique_ptr<MeshTriangle> floor = std::make_unique<MeshTriangle>("../models/cornellbox/floor.obj", Vector3f(0), white, loaded_materials);
    std::unique_ptr<MeshTriangle> left = std::make_unique<MeshTriangle>("../models/cornellbox/left.obj", Vector3f(0), mirror, loaded_materials);
    std::unique_ptr<MeshTriangle> right = std::make_unique<MeshTriangle>("../models/cornellbox/right.obj", Vector3f(0), blue, loaded_materials);
    std::unique_ptr<MeshTriangle> light_ = std::make_unique<MeshTriangle>("../models/cornellbox/light.obj", Vector3f(0, -5, 0), light, loaded_materials);
    // std::unique_ptr<MeshTriangle> light_ = std::make_unique<MeshTriangle>("../models/cornellbox/light.obj", Vector3f(100, -5, -100), light, loaded_materials);

    std::unique_ptr<MeshTriangle> rin = std::make_unique<MeshTriangle>("../models/rin/rin.obj", Vector3f(400, 0, 400), yellow, loaded_materials);

    // std::unique_ptr<MeshTriangle> window_wall = std::make_unique<MeshTriangle>("../models/window/wall.obj", Vector3f(275, 0, 500), white, loaded_materials);
    // std::unique_ptr<MeshTriangle> window_body = std::make_unique<MeshTriangle>("../models/window/window2.obj", Vector3f(275, 0, 500), white, loaded_materials);
    // std::unique_ptr<MeshTriangle> window = std::make_unique<MeshTriangle>("../models/window/window.obj", Vector3f(0, 0, 0), nullptr, loaded_materials);
    // std::unique_ptr<MeshTriangle> table = std::make_unique<MeshTriangle>("../models/table/table.obj", Vector3f(0, 0, 0), nullptr, loaded_materials);
    // std::unique_ptr<MeshTriangle> rabbit = std::make_unique<MeshTriangle>("../models/rabbit/rabbit.obj", Vector3f(0, 0, 0), nullptr, loaded_materials);
    // std::unique_ptr<MeshTriangle> lamp = std::make_unique<MeshTriangle>("../models/lamp/lamp.obj", Vector3f(0, 0, 0), nullptr, loaded_materials);
    // std::unique_ptr<MeshTriangle> plant = std::make_unique<MeshTriangle>("../models/plant/plant.obj", Vector3f(0, 0, 0), nullptr, loaded_materials);
    // std::unique_ptr<MeshTriangle> cannon = std::make_unique<MeshTriangle>("../models/cannon/cannon.obj", Vector3f(0, 0, 0), nullptr, loaded_materials);
    // std::unique_ptr<MeshTriangle> glass_cup = std::make_unique<MeshTriangle>("../models/glass/glass.obj", Vector3f(0, 0, 0), nullptr, loaded_materials);



    // std::unique_ptr<MeshTriangle> shortBox = std::make_unique<MeshTriangle>("../models/cornellbox/shortbox.obj", Vector3f(0), frosted_glass, loaded_materials);
    // std::unique_ptr<MeshTriangle> bob = std::make_unique<MeshTriangle>("../models/bob-the-duck/bob.obj", Vector3f(0), yellow, loaded_materials);


    std::unique_ptr<Sphere> glass_sphere = std::make_unique<Sphere>(Vector3f(440, 60, 100), 60, glass); glass_sphere->name = "glass_sphere";
    std::unique_ptr<Sphere> green_sphere = std::make_unique<Sphere>(Vector3f(380, 60, 400), 60, green); green_sphere->name = "green_sphere";


    // Material* ball_light = new Material();
    // loaded_materials.push_back(light);
    // // light->rho_map_implicit = [](Vector2f uv) {return Vector3f(0.8,0.8,0.8);}; // always return light
    // ball_light->m_type = EMIT;
    // // Yellow light
    // ball_light->m_emission= 5 * Vector3f(1.f, 1.f, 1.f); // emission color of the light
    // std::unique_ptr<Sphere> light_sphere = std::make_unique<Sphere>(Vector3f(400,600, 60), 30, light);




    // scene.Add(std::move(window));
    // scene.Add(std::move(table));
    // scene.Add(std::move(rabbit));
    // scene.Add(std::move(lamp));
    // scene.Add(std::move(plant));
    // scene.Add(std::move(cannon));
    // scene.Add(std::move(glass_cup));    

    // scene.Add(std::move(window_body));
    scene.Add(std::move(floor));
    scene.Add(std::move(left));
    scene.Add(std::move(right));
    scene.Add(std::move(light_));
    scene.Add(std::move(rin));
    // scene.Add(std::move(shortBox));
    // scene.Add(std::move(bob));
    // scene.Add(std::move(glass_sphere));
    // scene.Add(std::move(green_sphere));
    // scene.Add(std::move(light_sphere));


    std::cout << scene.light_sources.size() << std::endl;


    // ======= Additional objects =======


    
    


    


    // Build the floor manually
    // Define the 4 vertices of a large rectangle floor plane, lower left corner at (0,0,0), 550x560
    Vector3f verts[4] = {{0,0,0}, {552.8,0,0}, {549.6, 0,559.2}, {0,0,559.2}};
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
