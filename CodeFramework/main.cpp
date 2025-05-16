#include "Renderer.hpp"
#include "Scene.hpp"
#include "Triangle.hpp"
#include "Sphere.hpp"
#include "Cylinder.hpp"
#include "Vector.hpp"
#include "global.hpp"
#include <chrono>

// Acknowledge: GitHub copilot has been used to help write comments for some files to help 
// improve code readability before implementing the any tasks.
// Comments for the function recursiveBuild() and getIntersection() are written on my own.

int TASK_N=5;  // 1, 2, 3, 4, 5

// In the main function of the program, we create the scene (create objects and
// lights) as well as set the options for the render (image width and height,
// maximum recursion depth, field-of-view, etc.). We then call the render
// function().
int main(int argc, char** argv)
{
    if (argc>=2)
        TASK_N=(int)atoi(argv[1]);
    // change the resolution for quick debugging if rendering is slow
    // Scene scene(64, 64);
    // Scene scene(256, 256); // use this resolution for final rendering
    // Scene scene(512, 512);
    Scene scene(1024, 1024);

    if(TASK_N>=4)
        scene.spp = 128; // number of samples per pixel
    else
        scene.spp = 1;

    
    // Diffuse Materials
    Material* pink = new Material(DIFFUSE, Vector3f(0.72f, 0.48f, 0.56f));
    pink->Kd = 0.8f; // diffuse reflection coefficient
    pink->Ks = 0.2f; // specular reflection coefficient
    pink->ior = 1.46f; // refractive index

    Material* blue = new Material(DIFFUSE, Vector3f(0.2f, 0.6f, 0.86f));
    blue->Kd = 0.8f; // diffuse reflection coefficient
    blue->Ks = 0.2f; // specular reflection coefficient
    blue->ior = 1.46f; // refractive index

    Material* green = new Material(DIFFUSE, Vector3f(0.5f, 0.7f, 0.13f));
    green->Kd = 0.8f; // diffuse reflection coefficient
    green->Ks = 0.2f; // specular reflection coefficient
    green->ior = 1.46f; // refractive index

    Material* white = new Material(DIFFUSE, Vector3f(0.48f, 0.45f, 0.4f));
    white->Kd = 0.8f; // diffuse reflection coefficient
    white->Ks = 0.2f; // specular reflection coefficient
    white->ior = 100.f; // refractive index

    // Material* light_yellowish_green = new Material(DIFFUSE, Vector3f(0.6, 0.6, 0.3));
    Material* red = new Material(DIFFUSE, Vector3f(0.8f, 0.2f, 0.0f));
    red->Kd = 0.8f; // diffuse reflection coefficient
    red->Ks = 0.2f; // specular reflection coefficient
    red->ior = 1.46f; // refractive index

    // Glass Material
    Material* glass = new Material(GLASS, Vector3f(0.9f, 0.9f, 0.9f));
    glass->Kd = 0.f; // diffuse reflection coefficient
    glass->Ks = 1.f; // specular reflection coefficient
    glass->ior = 1.5f; // refractive index
    glass->opaqueness=0;

    Material* mirror = new Material(GLASS, Vector3f(0.9f, 0.9f, 0.9f));
    mirror->Kd = 0.f; // diffuse reflection coefficient
    mirror->Ks = 1.f; // specular reflection coefficient
    mirror->ior = 100.f; // refractive index

    // Emissive Material
    Material* light = new Material(EMIT, Vector3f(1));
    light->Kd = 0.8f; // diffuse reflection coefficient
    light->Ks = 0.2f; // specular reflection coefficient
    light->ior = 1.5f; // refractive index
    light->m_emission=100; // set light intensity

    // =======================


    Material* frosted_glass = new Material(GLASS, Vector3f(0.5f, 0.7f, 0.13f));
    frosted_glass->Kd = 0.2;
    frosted_glass->Ks = 0.8;
    frosted_glass->ior = 1.5;
    frosted_glass->opaqueness = 0.2;


    Material* gold = new Material(GLASS, Vector3f(255.f/255.f, 215.f/255.f, 0.0f));
    gold->Kd = 0.5f; // diffuse reflection coefficient
    gold->Ks = 0.5f; // specular reflection coefficient
    gold->ior = 100.f; // refractive index
    gold->opaqueness = 1;

    Material* stone = new Material(DIFFUSE, Vector3f(0.2f, 0.6f, 0.86f));
    stone->Kd = 0.8f;
    stone->Ks = 0.f;
    stone->ior = 1.5f;
    stone->opaqueness = 1;

    // Load Cornel box components from .obj files
    MeshTriangle floor("../models/cornellbox/floor.obj", Vector3f(0), white); // Vector3f(0) means no translation
    MeshTriangle left("../models/cornellbox/left.obj", Vector3f(0), mirror);
    MeshTriangle right("../models/cornellbox/right.obj",Vector3f(0), stone);
    MeshTriangle light_("../models/cornellbox/light.obj",Vector3f(0,-5,0), light); // light is translated downwards to be inside the box

    // Add mesh objects to the scene
    scene.Add(&floor); // stack object, scene will not delete it, but main will
    scene.Add(&left);
    scene.Add(&right);
    scene.Add(&light_);

    // ======= Additional objects =======

    MeshTriangle shortBox("../models/cornellbox/shortbox.obj", Vector3f(0), frosted_glass);
    scene.Add(&shortBox);

    MeshTriangle bob("../models/bob-the-duck/bob.obj", Vector3f(0), gold);
    scene.Add(&bob);


    // scene.Add(new Cylinder(Vector3f(360, 0, 280), 60, 150, blue)); // Add a cylinder to the scene


    // Add a sphere to the scene, placed at (370, 30, 150) with a radius of 30, colored grey
    // !!! Heap allocation, and you need to explicitly delete the object
    scene.Add(new Sphere(Vector3f(440,60,100), 60, glass));

    scene.Add(new Sphere(Vector3f(380,60,400), 60, green));

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
    Material* mfloor=new Material(DIFFUSE, Vector3f(0));
    // use a checkerboard texture pattern
    mfloor->textured=true;
    // Add the two triangles to the scene
    scene.Add(new MeshTriangle(verts, vertIndex, 2,st,mfloor));

    // Add a point light to the scene,located far above the scene with a relatively low intensity
    // scene now owns the light object and will delete it when the scene is deleted
    scene.Add(std::make_unique<PointLight>(Vector3f(-2000, 4000, -3000), 0.5));

    // Add an area light at the same location as the light object
    // scene.Add(std::make_unique<AreaLight>(Vector3f(0, 0, 0), Vector3f(0, 1, 0), Vector3f(1, 1, 1), 100));


    // Build the bounding volume hierarchy (BVH) for the scene
    scene.buildBVH();
    
    // Initialiser the renderer
    Renderer r;

    auto start = std::chrono::system_clock::now();
    r.Render(scene);
    auto stop = std::chrono::system_clock::now();

    std::cout << "Render complete: \n";
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << " hours\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() << " minutes\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() << " seconds\n";

    return 0;
}