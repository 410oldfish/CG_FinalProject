//
// Created by Göksu Güvendiren on 2019-05-14.
//

// What does this class do?
// Defines a PointLight class that represents a point light source in 3D space.
// READ

#pragma once

#include "Vector.hpp"

class PointLight
{
public:
    PointLight(const Vector3f &p, const Vector3f &i) : position(p), intensity(i) {}
    virtual ~PointLight() = default;
    Vector3f position;
    Vector3f intensity;
};
