# pragma once
#include "Renderer.hpp"
#include "Scene.hpp"

inline void s3_rendering(Scene &scene)
{
    // Build the bounding volume hierarchy (BVH) for the scene
    scene.buildBVH();
    
    // Initialiser the renderer
    Renderer r;
    r.Render(scene);
}