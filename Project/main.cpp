#include "Renderer.hpp"
#include "Scene.hpp"
#include "Triangle.hpp"
#include "Sphere.hpp"
#include "Cylinder.hpp"
#include "Vector.hpp"
#include "global.hpp"
#include <chrono>

int MAX_DEPTH = 10;
int SAMPLE_LIGHT = 12;

// In the main function of the program, we create the scene (create objects and
// lights) as well as set the options for the render (image width and height,
// maximum recursion depth, field-of-view, etc.). We then call the render
// function().
int main(int argc, char** argv)
{
    Scene scene(512, 512);
    scene.spp = 32;

    //pre-defined Materials
    Material* wood = new Material(OPAQUE, Vector3f(0.55f, 0.27f, 0.07f), 0.8f, 0.0f);
    Material* plastic = new Material(OPAQUE, Vector3f(0.2f, 0.6f, 0.9f), 0.2f, 0.0f);
    Material* metal_rusted = new Material(OPAQUE, Vector3f(0.7f, 0.3f, 0.1f), 0.9f, 1.0f);
    Material* metal = new Material(OPAQUE, Vector3f(0.9f, 0.85f, 0.7f), 0.05f, 1.0f);

    Material* red = new Material(OPAQUE, Vector3f(1.0f, 0.0f, 0.0f));
    Material* gray = new Material(OPAQUE, Vector3f(0.2f, 0.2f, 0.2f));
    Material* pink = new Material(OPAQUE, Vector3f(0.72f, 0.48f, 0.56f));
    Material* blue = new Material(OPAQUE, Vector3f(0.2f, 0.6f, 0.86f));
    Material* green = new Material(OPAQUE, Vector3f(0.5f, 0.7f, 0.13f));
    Material* yellow = new Material(OPAQUE, Vector3f(1.0f, 1.0f, 0.0f));
    Material* white = new Material(OPAQUE, Vector3f(0.48f, 0.45f, 0.4f));
    Material* light = new Material(EMIT, Vector3f(1));
    light->m_emission=100;

    MeshTriangle ceil("ceil", "../models/cornellbox/floor.obj", Vector3f(0), white);
    MeshTriangle left("left", "../models/cornellbox/left.obj", Vector3f(0), red);
    MeshTriangle right("right", "../models/cornellbox/right.obj",Vector3f(0),  green);
    MeshTriangle light_("arealight", "../models/cornellbox/light.obj",Vector3f(0,-5,0), light);

    scene.Add(&ceil);
    scene.Add(&left);
    scene.Add(&right);
    scene.Add(&light_);

    scene.Add(new Sphere(Vector3f(440,45,80), 60, metal));
    scene.Add(new Cylinder(Vector3f(330,0,260), 70, 200, metal_rusted));

    MeshTriangle duck("duck", "../models/bob-the-duck/bob.obj",Vector3f(-20,0,-30),  wood);
    scene.Add(&duck);

    MeshTriangle shortbox("box", "../models/cornellbox/shortbox.obj",Vector3f(-20,0,-30),  plastic);
    scene.Add(&shortbox);

    //MeshTriangle tallbox("../models/cornellbox/tallbox.obj",Vector3f(-20,0,0),  gray);
    //scene.Add(&tallbox);

    Vector3f verts[4] = {{0,0,0}, {552.8,0,0}, {549.6, 0,559.2}, {0,0,559.2}};
    Vector2f st[4] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
    uint32_t vertIndex[6] = {0, 2, 1, 2,0,3};
    Material* mfloor=new Material(OPAQUE, Vector3f(1,1,1));
    //mfloor->textured=true;
    scene.Add(new MeshTriangle("floor", verts, vertIndex, 2,st,mfloor));

    //scene.Add(std::make_unique<PointLight>(Vector3f(-2000, 4000, -3000), 0.5));

    scene.buildBVH();

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