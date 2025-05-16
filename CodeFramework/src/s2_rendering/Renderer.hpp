//
// Created by goksu on 2/25/20.
//
#include "Scene.hpp"

#pragma once

// Detail hit information (Not used in this project)
struct hit_payload
{
    float tNear; // The closest intersection distance (t-value) along the ray
    uint32_t index; // Optional: the index of the primitive hit (used in triangle meshes or BVHs)
    Vector2f uv; // Texture coordinates at the intersection (u,v)
    Object* hit_obj; // // Pointer to the object that was hit by the ray
};

class Renderer
{
public:
    // Shoot rays from the eye into the scene, shade hits, and save the output.
    // @param scene: the scene to be rendered
    void Render(const Scene& scene);

private:
};
