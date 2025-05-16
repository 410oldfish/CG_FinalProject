//
// Created by LEI XU on 5/13/19.
//

// What does this class do?
// The Sphere class represents a sphere in 3D space.

#ifndef RAYTRACING_SPHERE_H
#define RAYTRACING_SPHERE_H

#include "Object.hpp"
#include "Vector.hpp"
#include "Bounds3.hpp"
#include "Material.hpp"

class Sphere : public Object{
public:
    Vector3f center;
    float radius, radius2; // radius and radius squared
    std::shared_ptr<Material> m;
    float area;

    // Given a centre, radius, and material, create a sphere
    Sphere(const Vector3f &c, const float &r, std::shared_ptr<Material> mt = std::make_shared<Material>(Material())) : 
        center(c), radius(r), radius2(r * r), m(mt), area(4 * M_PI *r *r) {}
    
    // Return the intersection information of a ray with the sphere
    Intersection getIntersection(Ray ray){
        Intersection result;
        result.happened = false;
        Vector3f L = ray.origin - center; 

        // Solving t^2 (d dot d) + 2t (d dot L) + (L dot L - r^2) = 0
        float a = dotProduct(ray.direction, ray.direction);
        float b = 2 * dotProduct(ray.direction, L);
        float c = dotProduct(L, L) - radius2;
        float t0, t1;
        if (!solveQuadratic(a, b, c, t0, t1)) return result;
        if (t0 < 0) t0 = t1;
        if (t0 < 0) return result;
        result.happened=true;

        result.coords = Vector3f(ray.origin + ray.direction * t0);
        result.normal = normalize(Vector3f(result.coords - center));
        result.material = this->m;
        result.obj = this;
        result.tnear = t0; // tnear is the distance from the ray origin to the intersection point
        return result;
    }

    // Input:
    // P: point in 3D space
    // I: incident ray direction
    // index: index of the object
    // uv: texture coordinates
    // Output:
    // N: normal at the intersection point
    void getSurfaceProperties(const Vector3f &P, const Vector3f &I, const uint32_t &index, const Vector2f &uv, Vector3f &N, Vector2f &st) const
    { N = normalize(P - center); }

    // Evaluate the diffuse color of the sphere at a given texture coordinate
    Vector3f evalDiffuseColor(const Vector2f &st)const {
        return m->getColor();
    }

    // getBounds function returns the bounding box of the sphere
    Bounds3 getBounds(){
        return Bounds3(Vector3f(center.x-radius, center.y-radius, center.z-radius),
                       Vector3f(center.x+radius, center.y+radius, center.z+radius));
    }

    // Sample a point on the sphere
    void Sample(Intersection &pos, float &pdf){
        // Question: Is this sampling uniform?

        // theta = [0, 2pi) azimuthal angle
        // phi = [0, pi) polar angle
        float theta = 2.0 * M_PI * get_random_float(), phi = M_PI * get_random_float();

        // From spherical coordinates to Cartesian coordinates
        Vector3f dir(std::cos(phi), std::sin(phi)*std::cos(theta), std::sin(phi)*std::sin(theta));

        // Put values into the Intersection object
        pos.coords = center + radius * dir; // absolute position
        pos.normal = dir; // normal at the intersection point
        pos.obj = this; // object that was hit
        pos.material=m; // material of the object
        pdf = 1.0f / area; // probability density function
    }


    float getArea(){
        return area;
    }

    // Whether the sphere has an emission property (i.e., it emits light)
    bool hasEmit(){
        return m->hasEmission();
    }
};


#endif //RAYTRACING_SPHERE_H
