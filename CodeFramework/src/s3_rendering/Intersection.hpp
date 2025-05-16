//
// Created by LEI XU on 5/16/19.
//

// What does this class do?
// Intersection class is used to store the intersection information between a ray and an object.

#ifndef RAYTRACING_INTERSECTION_H
#define RAYTRACING_INTERSECTION_H
#include "Vector.hpp"
#include "Material.hpp"
class Object;
class Sphere;

struct Intersection
{
    Intersection(){
        happened=false; // no intersection
        coords=Vector3f(); // intersection coordinates
        normal=Vector3f(); // normal at the intersection point
        tnear= std::numeric_limits<double>::max(); // tnear is the t of the ray
        obj =nullptr; // object that was hit
        material=nullptr; // material of the object
    }
    bool happened;
    Vector3f coords;
    Vector2f tcoords;
    Vector3f normal;
    double tnear;
    Object* obj;
    std::shared_ptr<Material> material;
};
#endif //RAYTRACING_INTERSECTION_H
