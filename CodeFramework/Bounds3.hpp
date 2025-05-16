//
// Created by LEI XU on 5/16/19.
//

// What does this class do?
// This class defines a 3D bounding box (AABB) in a ray tracing application.
// It provides methods for creating, manipulating, and querying the bounding box.
// It also includes methods for checking intersections with rays and other bounding boxes.
// DONE

#ifndef RAYTRACING_BOUNDS3_H
#define RAYTRACING_BOUNDS3_H
#include "Ray.hpp"
#include "Vector.hpp"
#include <limits>
#include <array>
#include<float.h>

class Bounds3
{
  public:
    Vector3f pMin, pMax; // two points to specify the bounding box

    // Default constructor initializes the bounding box to the largest possible values
    Bounds3()
    {
        double minNum = std::numeric_limits<double>::lowest();
        double maxNum = std::numeric_limits<double>::max();
        pMax = Vector3f(minNum, minNum, minNum);
        pMin = Vector3f(maxNum, maxNum, maxNum);
    }

    // A trivial constructor that initializes the bounding box to a single point?
    Bounds3(const Vector3f p) : pMin(p), pMax(p) {}

    // Constructor that takes two points and creates a bounding box that contains both
    Bounds3(const Vector3f p1, const Vector3f p2)
    {
        pMin = Vector3f(fmin(p1.x, p2.x), fmin(p1.y, p2.y), fmin(p1.z, p2.z));
        pMax = Vector3f(fmax(p1.x, p2.x), fmax(p1.y, p2.y), fmax(p1.z, p2.z));
    }

    // Diagonal function returns the vector from pMin to pMax
    Vector3f Diagonal() const { return pMax - pMin; }

    // maxExtent function returns the index of the largest dimension of the bounding box
    // 0: x, 1: y, 2: z
    int maxExtent() const
    {
        Vector3f d = Diagonal();
        if (d.x > d.y && d.x > d.z)
            return 0;
        else if (d.y > d.z)
            return 1;
        else
            return 2;
    }

    // SurfaceArea function returns the surface area of the bounding box
    double SurfaceArea() const
    {
        Vector3f d = Diagonal();
        return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
    }

    // Centroid function returns the center of the bounding box
    Vector3f Centroid() { return 0.5 * pMin + 0.5 * pMax; }

    // Intersect function takes two bounding boxes and 
    // returns a new bounding box that is the intersection of the two
    Bounds3 Intersect(const Bounds3& b)
    {
        return Bounds3(Vector3f(fmax(pMin.x, b.pMin.x), fmax(pMin.y, b.pMin.y),
                                fmax(pMin.z, b.pMin.z)),
                       Vector3f(fmin(pMax.x, b.pMax.x), fmin(pMax.y, b.pMax.y),
                                fmin(pMax.z, b.pMax.z)));
    }

    // What does this function do?
    // It takes a point p and returns a new point that is the offset of p from pMin
    // The offset is normalized by the size of the bounding box
    // If the bounding box is degenerate (i.e., pMax.x <= pMin.x), then the offset is not normalized
    Vector3f Offset(const Vector3f& p) const
    {
        Vector3f o = p - pMin;
        if (pMax.x > pMin.x)
            o.x /= pMax.x - pMin.x;
        if (pMax.y > pMin.y)
            o.y /= pMax.y - pMin.y;
        if (pMax.z > pMin.z)
            o.z /= pMax.z - pMin.z;
        return o;
    }

    // Overlaps function takes two bounding boxes and returns true if they overlap
    bool Overlaps(const Bounds3& b1, const Bounds3& b2)
    {
        bool x = (b1.pMax.x >= b2.pMin.x) && (b1.pMin.x <= b2.pMax.x);
        bool y = (b1.pMax.y >= b2.pMin.y) && (b1.pMin.y <= b2.pMax.y);
        bool z = (b1.pMax.z >= b2.pMin.z) && (b1.pMin.z <= b2.pMax.z);
        return (x && y && z);
    }

    // What does this function do?
    // It takes a point p and a bounding box b and returns true if p is inside b
    bool Inside(const Vector3f& p, const Bounds3& b)
    {
        return (p.x >= b.pMin.x && p.x <= b.pMax.x && p.y >= b.pMin.y &&
                p.y <= b.pMax.y && p.z >= b.pMin.z && p.z <= b.pMax.z);
    }
    inline const Vector3f& operator[](int i) const
    {
        return (i == 0) ? pMin : pMax;
    }

    // InsersectP function takes a ray and returns true if the ray intersects the bounding box
    inline bool IntersectP(const Ray& ray, const Vector3f& invDir,
                           const std::array<int, 3>& dirisNeg) const;
};


// IntersectP function takes a ray and returns true if the ray intersects the bounding box
// How does this function work?
// It uses the slab method to check for intersection
// The slab method is a fast way to check for intersection between a ray and an axis-aligned bounding box (AABB)
// It works by checking if the ray intersects each of the three slabs (x, y, z) of the bounding box
// The function takes the ray, the inverse of the ray direction, and an array that indicates which direction is negative
// The function returns true if the ray intersects the bounding box, false otherwise
inline bool Bounds3::IntersectP(const Ray& ray, const Vector3f& invDir,
                                const std::array<int, 3>& dirIsNeg) const
{
    // invDir: ray direction(x,y,z), invDir=(1.0/x,1.0/y,1.0/z), use this because Multiply is faster that Division
    // dirIsNeg: ray direction(x,y,z), dirIsNeg=[int(x>0),int(y>0),int(z>0)], use this to simplify your logic

    // 1. Initialize tEnter and tExit to the minimum and maximum possible values
    float tEnter=FLT_MIN;
    float tExit=FLT_MAX;

    // 2. For each of the three dimensions (x, y, z), calculate the intersection points
    for (int i=0; i<3; i++)
    {
        // t = (point - ray.origin) / ray.direction
        float t_min=(pMin[i]-ray.origin[i])*invDir[i];
        float t_max=(pMax[i]-ray.origin[i])*invDir[i];
        if (dirIsNeg[i]==0)    std::swap(t_min, t_max); // note: here must be ==0, because dirIsNeg is actually int(x>0)
        // Why using dirIsNeg[i] to swap t_min and t_max?
        // Because if the ray direction is negative, we need to swap the min and max values
        // in order to get the correct tEnter and tExit values
        tEnter=std::max(t_min, tEnter);
        tExit=std::min(t_max, tExit);
    }
    // 3. Check if the ray intersects the bounding box
    // tEnter<=tExit means the ray intersects the bounding box
    // tExit>=0 means the ray is not behind the bounding box
    return tEnter<=tExit && tExit>=0;
}

// Union function takes two bounding boxes and returns a new bounding box that is the union of the two
inline Bounds3 Union(const Bounds3& b1, const Bounds3& b2)
{
    Bounds3 ret;
    ret.pMin = Vector3f::Min(b1.pMin, b2.pMin);
    ret.pMax = Vector3f::Max(b1.pMax, b2.pMax);
    return ret;
}

// Union function takes a bounding box and a point and returns a new bounding box that is the union of the two
inline Bounds3 Union(const Bounds3& b, const Vector3f& p)
{
    Bounds3 ret;
    ret.pMin = Vector3f::Min(b.pMin, p);
    ret.pMax = Vector3f::Max(b.pMax, p);
    return ret;
}

#endif // RAYTRACING_BOUNDS3_H
