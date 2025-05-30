# pragma one
#include "Scene.hpp"
#include "Triangle.hpp"
#include "Sphere.hpp"
#include <opencv2/opencv.hpp>
#include <global.hpp>
#include <Eigen/Dense>
#include <map>
#include <string>


// =============================================================================
//
// You can play around this class to create your own scene.
//
// =============================================================================


inline void s2_modelling(Scene& scene, std::vector<Material*>& loaded_materials, 
    std::map<std::string, void * >& opened_images)
{

    // ================================== MATERIALS ==================================


    Material* white = new Material();
    loaded_materials.push_back(white);
    white->rho_map_implicit = [](Vector2f uv) {return Vector3f(0.48f, 0.45f, 0.4f);}; // always return white
    white->kd_map_implicit = [](Vector2f uv) {return 0.8f * Vector3f(1);};
    white->ks_map_implicit = [](Vector2f uv) {return 0.2f * Vector3f(1);};
    white->ior_map_implicit = [](Vector2f uv) {return 5.f;};
    white->opaqueness_map_implicit = [](Vector2f uv) {return 1.f;};


    // Glass Material
    // std::shared_ptr<Material> glass = std::make_shared<Material>(GLASS, Vector3f(0.9f, 0.9f, 0.9f));
    Material* glass = new Material();
    loaded_materials.push_back(glass);
    glass->rho_map_implicit = [](Vector2f uv) {return Vector3f(0.9f, 0.9f, 0.9f);}; // always return glass
    glass->kd_map_implicit = [](Vector2f uv) {return Vector3f(0.0f, 0.0f, 0.0f);}; // no diffuse reflection
    glass->ks_map_implicit = [](Vector2f uv) {return Vector3f(1.0f, 1.0f, 1.0f);}; // full specular reflection
    glass->ior_map_implicit = [](Vector2f uv) {return 1.5f;}; // refractive index
    glass->opaqueness_map_implicit = [](Vector2f uv) {return 0.0f;}; // fully transparent


    // std::shared_ptr<Material> frosted_glass = std::make_shared<Material>(GLASS, Vector3f(0.5f, 0.7f, 0.13f));
    Material* frosted_glass = new Material();
    frosted_glass->rho_map_implicit = [](Vector2f uv) {return Vector3f(0.5f, 0.7f, 0.13f);}; // always return frosted glass
    loaded_materials.push_back(frosted_glass);
    frosted_glass->kd_map_implicit = [](Vector2f uv) {return 0.2f * Vector3f(1);}; // no diffuse reflection
    frosted_glass->ks_map_implicit = [](Vector2f uv) {return 0.8f * Vector3f(1);}; // full specular reflection
    frosted_glass->ior_map_implicit = [](Vector2f uv) {return 1.5f;}; // refractive index
    frosted_glass->opaqueness_map_implicit = [](Vector2f uv) {return 0.2f;}; // fully transparent


    // ================================== LIGHTS ===============================

    Material* lightUpMaterial = new Material();
    loaded_materials.push_back(lightUpMaterial);
    lightUpMaterial->m_type = EMIT;
    lightUpMaterial->m_emission= 15 * Vector3f(1.f, 1.f, 1.f);
    std::unique_ptr<MeshTriangle> lightUp = std::make_unique<MeshTriangle>("../models/LightUpTest.obj", Vector3f(0, 0, 0), lightUpMaterial, loaded_materials, opened_images);
    scene.light_sources.push_back(lightUp.get());
    scene.light_source_weights.push_back(1.0f); // Add a weight for the light source


    // Material* lightRightMaterial = new Material();
    // loaded_materials.push_back(lightRightMaterial);
    // lightRightMaterial->m_type = EMIT;
    // lightRightMaterial->m_emission= 5 * Vector3f(1.f, 1.f, 1.f);
    // std::unique_ptr<MeshTriangle> lightRight = std::make_unique<MeshTriangle>("../models/LightRight.obj", Vector3f(0), lightRightMaterial, loaded_materials, opened_images);
    

    // Material* lightLeftMaterial = new Material();
    // loaded_materials.push_back(lightLeftMaterial);
    // lightLeftMaterial->m_type = EMIT;
    // lightLeftMaterial->m_emission= 5 * Vector3f(1.f, 1.f, 1.f);
    // std::unique_ptr<MeshTriangle> lightLeft = std::make_unique<MeshTriangle>("../models/LightLeft.obj", Vector3f(0), lightLeftMaterial, loaded_materials, opened_images);


    Material* lightLampMaterial = new Material();
    loaded_materials.push_back(lightLampMaterial);
    lightLampMaterial->m_type = EMIT;
    lightLampMaterial->m_emission= 200 * Vector3f( 1.00f, 0.98f, 0.94f);
    std::unique_ptr<MeshTriangle> lightLamp = std::make_unique<MeshTriangle>("../models/LightLamp.obj", Vector3f(0), lightLampMaterial, loaded_materials, opened_images);
    // std::unique_ptr<Sphere> lightLamp = std::make_unique<Sphere>(Vector3f(464.369f, 244.115f, 206.143f), 5, lightLampMaterial);
    scene.light_sources.push_back(lightLamp.get());
    scene.light_source_weights.push_back(1.0f); // Add a weight for the light source



    // Material* sun1Material = new Material();
    // loaded_materials.push_back(sun1Material);
    // sun1Material->m_type = EMIT;
    // sun1Material->m_emission= 30 * Vector3f(1.f, 1.f, 1.f);
    // std::unique_ptr<MeshTriangle> sun1 = std::make_unique<MeshTriangle>("../models/Sun1.obj", Vector3f(0), sun1Material, loaded_materials, opened_images);

    // Material* sun2Material = new Material();
    // loaded_materials.push_back(sun2Material);
    // sun2Material->m_type = EMIT;
    // sun2Material->m_emission= 30 * Vector3f(1.f, 1.f, 1.f);
    // std::unique_ptr<MeshTriangle> sun2 = std::make_unique<MeshTriangle>("../models/Sun2.obj", Vector3f(0), sun2Material, loaded_materials, opened_images);

    
    Material* sunBigMaterial = new Material();
    loaded_materials.push_back(sunBigMaterial);
    sunBigMaterial->m_type = EMIT;
    sunBigMaterial->m_emission= 100 * Vector3f(1.f, 1.f, 1.f);
    std::unique_ptr<MeshTriangle> sunBig = std::make_unique<MeshTriangle>("../models/SunBig.obj", Vector3f(0), sunBigMaterial, loaded_materials, opened_images);


    // =========================== OBJ OBJECTS ============================
    

    // std::unique_ptr<MeshTriangle> room = std::make_unique<MeshTriangle>("../models/room.obj", Vector3f(0), white, loaded_materials, opened_images);
    // std::unique_ptr<MeshTriangle> room = std::make_unique<MeshTriangle>("../models/room_nobody_here.obj", Vector3f(0), white, loaded_materials, opened_images);


    // scene.Add(std::move(room));

    scene.Add(std::move(lightUp));
    // scene.Add(std::move(sunBig));
    // scene.Add(std::move(lightLamp));
    scene.Add(std::move(lightLamp));
    // scene.Add(std::move(lightLeft));
    // scene.Add(std::move(lightRight));



    // ============================= NON-OBJ SPHERES ============================


    // std::unique_ptr<Sphere> green_sphere = std::make_unique<Sphere>(Vector3f(200, 60, 400), 60, glass);
    // scene.Add(std::move(green_sphere));



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


    // 1) Polka Dots
        mfloor->rho_map_implicit = [](Vector2f uv) -> Vector3f {
        // 1) Frequency: how many dots per axis
        uv = uv * 8.0f;

        // 2) Colours
        Vector3f bg = Vector3f(1.0f, 0.9f, 0.95f);  // RGB (255, 237, 227)
        Vector3f fg = Vector3f(1.0f, 0.6f, 0.8f);   // RGB (255, 163, 196)
        // bg 237, 227, 208
        // fg 163, 196, 188
        // bg = Vector3f(0.847f, 0.768f, 0.631f);
        // fg = Vector3f(0.366f, 0.552f, 0.503f);

        // 3) Find local position inside each cell
        Vector2f f = fract(uv) - Vector2f(0.5f, 0.5f);

        // 4) Radius test
        float radius = 0.3f;
        float dist = std::sqrt(dot(f, f));
        return (dist < radius) ? fg : bg;
    };


    mfloor->kd_map_implicit = [](Vector2f uv) {return Vector3f(0.64f, 0.64f, 0.64f);};
    mfloor->ks_map_implicit = [](Vector2f uv) {return Vector3f(0.2f, 0.2f, 0.2f);};
    mfloor->ior_map_implicit = [](Vector2f uv) {return 2.f;};
    mfloor->opaqueness_map_implicit = [](Vector2f uv) {return 1.f;}; // fully opaque


    // Build the floor manually
    // Define the 4 vertices of a large rectangle floor plane, lower left corner at (0,0,0), 550x560
    Vector3f verts[4] = {{0,3,0}, {564,3,0}, {564, 3,559.2}, {0,3,559.2}};
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