//
// Created by LEI XU on 5/13/19.
//

// What does this class do?
// This class defines an abstract base class for 3D objects in a ray tracing application.
// DONE

#pragma once // This is a modern alternative to #ifndef/#define guards
#ifndef RAYTRACING_OBJECT_H
#define RAYTRACING_OBJECT_H

#include "Vector.hpp" // Include the Vector3f and Vector2f classes
#include "global.hpp" // Include global constants and utility functions
#include "Bounds3.hpp" // Include the Bounds3 class for bounding box operations
#include "Ray.hpp" // Include the Ray class for ray operations
#include "Intersection.hpp" // Include the Intersection class for intersection operations

class Object
{
public:
    // Constructor and destructor
    Object() {}
    virtual ~Object() {}

    // Get the Intersection of a ray with the object
    virtual Intersection getIntersection(Ray _ray) = 0;

    // ?
    virtual void getSurfaceProperties(const Vector3f &, const Vector3f &, const uint32_t &, const Vector2f &, Vector3f &, Vector2f &) const = 0;

    // evaluate the diffuse color of the object at a given texture coordinate
    virtual Vector3f evalDiffuseColor(const Vector2f &) const =0;

    // Get the bounding box of the object?
    virtual Bounds3 getBounds()=0;

    // Get the area of the object
    virtual float getArea()=0;

    // Given an intersection point and a probability density function (pdf), sample a point on the object?
    virtual void Sample(Intersection &pos, float &pdf)=0;

    // Whether the object has an emission property
    virtual bool hasEmit()=0;
};


#endif //RAYTRACING_OBJECT_H
