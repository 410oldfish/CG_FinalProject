# pragma one
#include "Scene.hpp"
#include "Triangle.hpp"
#include "Sphere.hpp"
#include <opencv2/opencv.hpp>

inline void s2_modelling(Scene& scene, std::vector<Material*>& materials)
{

    // Diffuse Materials
    // std::shared_ptr<Material> pink = std::make_shared<Material>(DIFFUSE, Vector3f(0.72f, 0.48f, 0.56f));
    Material* pink = new Material(DIFFUSE, Vector3f(0.72f, 0.48f, 0.56f));
    pink->Kd = 0.8f; // diffuse reflection coefficient
    pink->Ks = 0.2f; // specular reflection coefficient
    pink->ior = 1.46f; // refractive index
    materials.push_back(pink);


    // std::shared_ptr<Material> blue = std::make_shared<Material>(DIFFUSE, Vector3f(0.2f, 0.6f, 0.86f));
    Material* blue = new Material(DIFFUSE, Vector3f(0.2f, 0.6f, 0.86f));
    blue->Kd = 0.8f; // diffuse reflection coefficient
    blue->Ks = 0.2f; // specular reflection coefficient
    blue->ior = 1.46f; // refractive index
    materials.push_back(blue);


    // std::shared_ptr<Material> green = std::make_shared<Material>(DIFFUSE, Vector3f(0.5f, 0.7f, 0.13f));
    Material* green = new Material(DIFFUSE, Vector3f(0.5f, 0.7f, 0.13f));
    green->Kd = 0.8f; // diffuse reflection coefficient
    green->Ks = 0.2f; // specular reflection coefficient
    green->ior = 1.46f; // refractive index
    materials.push_back(green);


    // std::shared_ptr<Material> white = std::make_shared<Material>(DIFFUSE, Vector3f(0.48f, 0.45f, 0.4f));
    Material* white = new Material(DIFFUSE, Vector3f(0.48f, 0.45f, 0.4f));
    white->Kd = 0.8f; // diffuse reflection coefficient
    white->Ks = 0.2f; // specular reflection coefficient
    white->ior = 5.f; // refractive index
    materials.push_back(white);

    // std::shared_ptr<Material> red = std::make_shared<Material>(DIFFUSE, Vector3f(0.8f, 0.2f, 0.0f));
    Material* red = new Material(DIFFUSE, Vector3f(0.8f, 0.2f, 0.0f));
    red->Kd = 0.8f; // diffuse reflection coefficient
    red->Ks = 0.2f; // specular reflection coefficient
    red->ior = 1.46f; // refractive index
    materials.push_back(red);

    // Glass Material
    // std::shared_ptr<Material> glass = std::make_shared<Material>(GLASS, Vector3f(0.9f, 0.9f, 0.9f));
    Material* glass = new Material(GLASS, Vector3f(0.9f, 0.9f, 0.9f));
    glass->Kd = 0.f; // diffuse reflection coefficient
    glass->Ks = 1.f; // specular reflection coefficient
    glass->ior = 1.5f; // refractive index
    glass->opaqueness=0;
    materials.push_back(glass);

    // std::shared_ptr<Material> mirror = std::make_shared<Material>(GLASS, Vector3f(0.9f, 0.9f, 0.9f));
    Material* mirror = new Material(GLASS, Vector3f(0.9f, 0.9f, 0.9f));
    mirror->Kd = 0.f; // diffuse reflection coefficient
    mirror->Ks = 1.f; // specular reflection coefficient
    mirror->ior = 100.f; // refractive index
    materials.push_back(mirror);

    // Emissive Material
    // std::shared_ptr<Material> light = std::make_shared<Material>(EMIT, Vector3f(1));
    Material* light = new Material(EMIT, Vector3f(1));
    light->Kd = 0.8f; // diffuse reflection coefficient
    light->Ks = 0.2f; // specular reflection coefficient
    light->ior = 1.5f; // refractive index
    light->m_emission=100; // set light intensity
    materials.push_back(light);


    // std::shared_ptr<Material> frosted_glass = std::make_shared<Material>(GLASS, Vector3f(0.5f, 0.7f, 0.13f));
    Material* frosted_glass = new Material(GLASS, Vector3f(0.5f, 0.7f, 0.13f));
    frosted_glass->Kd = 0.2;
    frosted_glass->Ks = 0.8;
    frosted_glass->ior = 1.5;
    frosted_glass->opaqueness = 0.2;
    materials.push_back(frosted_glass);


    // std::shared_ptr<Material> gold = std::make_shared<Material>(GLASS, Vector3f(255.f/255.f, 215.f/255.f, 0.0f));
    Material* gold = new Material(GLASS, Vector3f(255.f/255.f, 215.f/255.f, 0.0f));
    gold->Kd = 0.5f; // diffuse reflection coefficient
    gold->Ks = 0.5f; // specular reflection coefficient
    gold->ior = 100.f; // refractive index
    gold->opaqueness = 1;
    materials.push_back(gold);


    // std::shared_ptr<Material> stone = std::make_shared<Material>(DIFFUSE, Vector3f(0.2f, 0.6f, 0.86f));
    Material* stone = new Material(DIFFUSE, Vector3f(0.2f, 0.6f, 0.86f));
    stone->Kd = 0.8f;
    stone->Ks = 0.f;
    stone->ior = 1.5f;
    stone->opaqueness = 1;
    materials.push_back(stone);


    Material* yellow = new Material(DIFFUSE, Vector3f(1, 1, 0));
    yellow->Kd = 0.8f; // diffuse reflection coefficient
    yellow->Ks = 0.2f; // specular reflection coefficient
    yellow->ior = 1.5f; // refractive index
    yellow->opaqueness = 1;
    materials.push_back(yellow);
    cv::Mat image_data_bob = cv::imread("../models/bob-the-duck/bob_diffuse.png");
    cv::cvtColor(image_data_bob, image_data_bob, cv::COLOR_RGB2BGR);
    yellow->rho_texture_map = image_data_bob;




    std::unique_ptr<MeshTriangle> floor = std::make_unique<MeshTriangle>("../models/cornellbox/floor.obj", Vector3f(0), white);
    std::unique_ptr<MeshTriangle> left = std::make_unique<MeshTriangle>("../models/cornellbox/left.obj", Vector3f(0), mirror);
    std::unique_ptr<MeshTriangle> right = std::make_unique<MeshTriangle>("../models/cornellbox/right.obj", Vector3f(0), blue);
    std::unique_ptr<MeshTriangle> light_ = std::make_unique<MeshTriangle>("../models/cornellbox/light.obj", Vector3f(0, -5, 0), light); // light is translated downwards to be inside the box



    // Add mesh objects to the scene
    // scene.Add(floor); // stack object, scene will not delete it, but main will
    // scene.Add(left);
    // scene.Add(right);
    // scene.Add(light_);
    scene.Add(std::move(floor));
    scene.Add(std::move(left));
    scene.Add(std::move(right));
    scene.Add(std::move(light_));


    // ======= Additional objects =======


    std::unique_ptr<MeshTriangle> shortBox = std::make_unique<MeshTriangle>("../models/cornellbox/shortbox.obj", Vector3f(0), frosted_glass);
    scene.Add(std::move(shortBox));


    std::unique_ptr<MeshTriangle> bob = std::make_unique<MeshTriangle>("../models/bob-the-duck/bob.obj", Vector3f(0), yellow);
    scene.Add(std::move(bob));


    scene.Add(std::make_unique<Sphere>(Vector3f(440, 60, 100), 60, glass));
    scene.Add(std::make_unique<Sphere>(Vector3f(380, 60, 400), 60, green));

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
    Material* mfloor = new Material(DIFFUSE, Vector3f(0));
    // use a checkerboard texture pattern


    cv::Mat image_data = cv::imread("../models/bob-the-duck/bob_diffuse.png");
    cv::cvtColor(image_data, image_data, cv::COLOR_RGB2BGR);
    mfloor->rho_texture_map = image_data;


    mfloor->rho_texture_map_implicit = [](Vector2f uv) -> Vector3f {
    float scale = 5;
    float u = fmodf(uv.x * scale, 1.0f);
    float v = fmodf(uv.y * scale, 1.0f);
    bool pattern = (u > 0.5f) ^ (v > 0.5f); // XOR checker
    // Return two alternating colors
    return pattern
        ? Vector3f(0.815f, 0.235f, 0.031f)
        : Vector3f(0.937f, 0.937f, 0.231f);
    };







    // Add the two triangles to the scene
    scene.Add(std::make_unique<MeshTriangle>(verts, vertIndex, 2, st, mfloor));

    // Add a point light to the scene,located far above the scene with a relatively low intensity
    // scene now owns the light object and will delete it when the scene is deleted
    scene.Add(std::make_unique<PointLight>(Vector3f(-2000, 4000, -3000), 0.5));

    // Add an area light at the same location as the light object
    // scene.Add(std::make_unique<AreaLight>(Vector3f(0, 0, 0), Vector3f(0, 1, 0), Vector3f(1, 1, 1), 100));
}