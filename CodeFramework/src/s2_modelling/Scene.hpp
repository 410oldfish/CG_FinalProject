//
// Created by Göksu Güvendiren on 2019-05-14.
//

#pragma once

#include <vector>
#include "Vector.hpp"
#include "Object.hpp"
#include "PointLight.hpp"
// #include "AreaLight.hpp"
#include "BVH.hpp"
#include "Ray.hpp"


class Scene
{
public:
    // Fields Definition (Part 1)
    int width = 1280; // image width, default 1280
    int height = 960; // image height, default 960
    double fov = 40; // camera field of view, default 40
    Vector3f backgroundColor = Vector3f(0.235294, 0.67451, 0.843137); // background color, default light blue
    int spp = 64; // samples per pixel, default 64


    // Constructor for the scene that takes width and height as parameters
    // @parm w: width of the scene
    // @parm h: height of the scene
    Scene(int w, int h) : width(w), height(h)
    {}

    // Add an object to the scene
    // @param object: pointer to the object to be added
    // void Add(Object *object) { objects.push_back(object); }
    void Add(std::unique_ptr<Object> object) {
    if (object->hasEmit()) {
        light_sources.push_back(object.get());
    }
    objects.push_back(std::move(object));
}


    // Add a light to the scene
    // @param light: pointer to the light to be added
    void Add(std::unique_ptr<PointLight> light) { lights.push_back(std::move(light)); }

    // Get a list of all object pointers in the scene
    // @return: vector of pointers to objects in the scene
    // const std::vector<Object*>& get_objects() const { return objects; }
    const std::vector<std::unique_ptr<Object> >& get_objects() const { return objects; }

    // Get a list of all light pointers in the scene
    // @return: vector of pointers to lights in the scene
    const std::vector<std::unique_ptr<PointLight> >&  get_lights() const { return lights; }

    // Compute ray-scene intersection using the BVH
    // @param ray: ray to be tested
    // @return: the closest intersection where the ray hits an object or empty.
    Intersection intersect(const Ray& ray) const;

    // Fields Definition (Part 2)
    // BVHAccel *bvh; // pointer to the BVH object of the scene
    std::unique_ptr<BVHAccel> bvh; // unique pointer to the BVH object of the scene
    
    // Build the BVH for the scene, call once at the beginning
    void buildBVH();

    // !!!! Main Path Tracing Function !!!!
    // Cast a ray into the scene and return the color at the intersection point
    // @param ray: ray to be cast (from camera or a bounce ray)
    // @param depth: current recursion depth (for limiting recursion)
    // @return: color at the intersection point
    // Ideas:
    // - Check Intersection
    // - Handle emission, diffuse, and glass materials
    // - Handle area light sampling
    Vector3f castRay(const Ray &ray, int depth, bool has_evaluated_diffuse_previously) const;



    // Randomly samples a point on one of the emissive objects (area lights) in the scene, proportionally to surface area
    // @param pos (output): intersection point
    // @param pdf (output): probability density function of the sampling
    // !!!!!!!
    // This code may be wrong PDF is the PDF of a single light source, not pdf /= emit_area_sum; 
    // !!!!!!!
    void sampleLight(Intersection &pos, float &pdf) const;

    // Evaluate the contribution of an area light to the shading at a specific point
    // - Sample a point on the area light
    // - Cast a shadow ray from the hit point to the light
    // - Computing diffuse and specular reflection
    // @param light: the area light being sampled
    // @param hitPoint: the point on the object being shaded
    // @param N: the normal at the hit point
    // @param shadowPointOrig: Offset ray origin to avoid self-intersection (a bit away from hitPoint along N)
    // @param objects: The scene geometry list (for testing shadow ray occlusion)
    // @param index: Likely a placeholder, unused here unless used internally for object indexing
    // @param dir: Direction from hit point to camera (view vector), used for specular reflection calculation
    // @param specularExponent: Sharpness of specular reflection (Phong model, higher = shinier)
    
    // std::tuple<Vector3f, Vector3f> HandleAreaLight(const AreaLight &light, const Vector3f &hitPoint, const Vector3f &N,
    //                                                const Vector3f &shadowPointOrig,
    //                                                const std::vector<Object *> &objects, uint32_t &index,
    //                                                const Vector3f &dir, float specularExponent);


    // Fields Definition (Part 3)
    // unique pointer
    std::vector<std::unique_ptr<Object>> objects; // vector of pointers to objects in the scene

    std::vector<Object*> light_sources; // they are the objects copied from the objects vector

    std::vector<std::unique_ptr<PointLight> > lights; // vector of pointers to lights in the scene



    // ======== NOTE: The following functions are also defined in the Material class !!!! ========


    // Compute reflection direction
    // @param I: incident ray direction
    // @param N: normal at the intersection point
    // @return: reflection direction
    Vector3f reflect(const Vector3f &I, const Vector3f &N) const
    {
        return I - 2 * dotProduct(I, N) * N;
    }


    // Compute refraction direction using Snell's law
    //
    // We need to handle with care the two possible situations:
    //
    //    - When the ray is inside the object
    //
    //    - When the ray is outside.
    //
    // If the ray is outside, you need to make cosi positive cosi = -N.I
    //
    // If the ray is inside, you need to invert the refractive indices and negate the normal N
    //
    // Computes refraction direction using Snell's Law.
    // Handles rays entering or exiting a dielectric (like glass).
    // ior = index of refraction (e.g., 1.5 for glass).
    // Returns zero vector if total internal reflection occurs.
    //
    // @param I: incident ray direction
    // @param N: normal at the intersection point
    // @param ior: index of refraction (e.g., 1.5 for glass)
    // @return: refraction direction
    Vector3f refract(const Vector3f &I, const Vector3f &N, const float &ior) const
    {
        float cosi = clamp(-1, 1, dotProduct(I, N));
        float etai = 1, etat = ior;
        Vector3f n = N;
        if (cosi < 0) { cosi = -cosi; } else { std::swap(etai, etat); n= -N; }
        float eta = etai / etat;
        float k = 1 - eta * eta * (1 - cosi * cosi);
        return k < 0 ? 0 : eta * I + (eta * cosi - sqrtf(k)) * n;
    }


    // !! I can't understand how it works but I don't need to
    // 
    // Compute Fresnel equation
    //
    // It calculates how much light is reflected vs how much is refracted when a ray hits a transparent material like glass or water.
    //
    // @param I: Incident direction (view or light ray direction), pointing into the surface
    // @param N: Surface normal at intersection point (assumed to point out of surface)
    // @param ior: Index of refraction of the object (e.g., 1.5 for glass, 1.0 for vacuum)
    // @return: Fresnel reflectance — the fraction of light reflected at the surface. 1 - kr is transmitted (refracted).
    float fresnel(const Vector3f &I, const Vector3f &N, const float &ior) const
    {
        float cosi = clamp(-1, 1, dotProduct(I, N));
        float etai = 1, etat = ior;
        if (cosi > 0) {  std::swap(etai, etat); }
        // Compute sini using Snell's law
        float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));
        // Total internal reflection
        if (sint >= 1) {
            return 1;
        }
        else {
            float cost = sqrtf(std::max(0.f, 1 - sint * sint));
            cosi = fabsf(cosi);
            float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
            float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
            return (Rs * Rs + Rp * Rp) / 2;
        }
        // As a consequence of the conservation of energy, transmittance is given by:
        // kt = 1 - kr;
    }

private:

    inline Vector3f castRayDiffuse(Ray ray, int depth, bool has_evaluated_diffuse_previously, 
        Intersection &inter, 
        Vector3f &point_hit,
        Vector3f &normal_hit,
        Vector3f &point_hit_above,
        Vector2f &texture_hit,
        Vector3f &dir_hit_to_view,
        Vector3f &dir_view_to_hit,
        Vector3f &rho
    ) const;

};