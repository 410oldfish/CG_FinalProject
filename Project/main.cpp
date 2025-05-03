#include "Renderer.hpp"
#include "Scene.hpp"
#include "Triangle.hpp"
#include "Sphere.hpp"
#include "Cylinder.hpp"
#include "Vector.hpp"
#include "global.hpp"
#include <chrono>
#include "PreMaterials.h"

int IMAGE_W = 256;
int IMAGE_H = 256;
int MAX_DEPTH = 7;
int SAMPLE_LIGHT = 4;
int SPP = 16;
pcg32_state GLOBAL_RNG = init_pcg32();
// In the main function of the program, we create the scene (create objects and
// lights) as well as set the options for the render (image width and height,
// maximum recursion depth, field-of-view, etc.). We then call the render
// function().
int main(int argc, char** argv)
{
    Scene scene(IMAGE_W, IMAGE_H);
    scene.spp = SPP;


    Material* greenPlastic = new Material(*plastic); // 拷贝所有参数
    greenPlastic->m_baseColor = Vector3f(0.2f, 0.8f, 0.2f); // 绿色
    Material* blackPlastic = new Material(*plastic);
    blackPlastic->m_baseColor = Vector3f(0.2f, 0.2f, 0.2f);
    Material* whitePlastic = new Material(*plastic);
    whitePlastic->m_baseColor = Vector3f(1.0f, 1.0f, 1.0f);
    Material* redPlastic = new Material(*plastic);
    redPlastic->m_baseColor = Vector3f(0.8f, 0.2f, 0.2f);

    MeshTriangle ceil("ceil", "../models/cornellbox/floor.obj", Vector3f(0), blackPlastic);
    MeshTriangle left("left", "../models/cornellbox/left.obj", Vector3f(0), redPlastic);
    MeshTriangle right("right", "../models/cornellbox/right.obj",Vector3f(0),  greenPlastic);
    MeshTriangle light_("arealight", "../models/cornellbox/light.obj",Vector3f(0,-5,0), light_emissive);

    scene.Add(&ceil);
    scene.Add(&left);
    scene.Add(&right);
    scene.Add(&light_);

    scene.Add(new Sphere(Vector3f(200,200,150), 60, glass));
    //scene.Add(new Cylinder(Vector3f(330,0,260), 70, 200, wax));

    MeshTriangle duck("duck", "../models/bob-the-duck/bob.obj",Vector3f(-20,0,-30),  jade);
    //scene.Add(&duck);

    MeshTriangle shortbox("box", "../models/cornellbox/shortbox.obj",Vector3f(-20,0,-30),  metal);
    //scene.Add(&shortbox);

    //MeshTriangle tallbox("../models/cornellbox/tallbox.obj",Vector3f(-20,0,0),  gray);
    //scene.Add(&tallbox);

    Vector3f verts[4] = {{0,0,0}, {552.8f,0.0f,0.0f}, {549.6f, 0.0f,559.2f}, {0.0f,0.0f,559.2f}};
    Vector2f st[4] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
    uint32_t vertIndex[6] = {0, 2, 1, 2,0,3};
    //mfloor->textured=true;
    scene.Add(new MeshTriangle("floor", verts, vertIndex, 2,st,whitePlastic));

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