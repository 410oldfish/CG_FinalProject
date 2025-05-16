#ifndef RAYTRACING_CYLINDER_H
#define RAYTRACING_CYLINDER_H

#include "Object.hpp"
#include "Vector.hpp"
#include "Bounds3.hpp"
#include "Material.hpp"

// TODO


class Cylinder : public Object {
    // TODO: Define a cylinder

public:
    Vector3f centre;
    float radius, radius_squared;
    float height;
    Material *m;
    float area;

private:
    Intersection getIntersectionSide(Ray ray){
        Intersection result;
        result.happened = false;

        // Cylinder equation: (x - Cx)^2 + (z - Cz)^2 = R^2
        // Ray equation: r(t) = o + t * d
        // Substituting the ray equation into the cylinder equation gives:
        // (o.x + t * d.x - Cx)^2 + (o.z + t * d.z - Cz)^2 = R^2
        // Rearranging gives a quadratic equation:
        float a = ray.direction.x * ray.direction.x + ray.direction.z * ray.direction.z;
        float b = 2 * (ray.direction.x * (ray.origin.x - centre.x) + ray.direction.z * (ray.origin.z - centre.z));
        float c = (ray.origin.x - centre.x) * (ray.origin.x - centre.x) 
                + (ray.origin.z - centre.z) * (ray.origin.z - centre.z) - radius_squared;
        float t0, t1;
        if (!solveQuadratic(a, b, c, t0, t1)) return result;
        if (t0 < 0) t0 = t1; // We only want the positive root
        if (t0 < 0) return result; // Both roots are negative
        if (t0 > ray.t_max) return result; // Intersection is beyond the ray max distance

        float y_intersection = ray.origin.y + t0 * ray.direction.y;

        // Check if the intersection is within the height of the cylinder
        if (y_intersection < centre.y - EPSILON || y_intersection > centre.y + height + EPSILON) {
            return result; // No intersection
        }
        result.happened=true;
        result.coords = Vector3f(ray.origin + ray.direction * t0);
        result.normal = normalize(Vector3f(result.coords.x - centre.x, 0, result.coords.z - centre.z));
        result.material = this->m;
        result.obj = this;
        result.tnear = t0;
        return result;
    }

    Intersection getIntersectionTop(Ray ray){
        Intersection result;
        result.happened = false;
        
        // Cylinder top equation: y = h
        // Ray equation: r(t) = o + t * d
        // Substituting the ray equation into the cylinder top equation gives:
        // o.y + t * d.y = h
        // Rearranging gives a linear equation:
        if (ray.direction.y == 0) return result; // Ray is parallel to the top
        float t = (centre.y + height - ray.origin.y) / ray.direction.y;

        if (t < 0) return result; // Intersection is behind the ray origin
        if (t > ray.t_max) return result; // Intersection is beyond the ray max distance

        // Check if the intersection is within the radius of the cylinder
        float x_intersection = ray.origin.x + t * ray.direction.x;
        float z_intersection = ray.origin.z + t * ray.direction.z;
        if ((x_intersection - centre.x) * (x_intersection - centre.x) + 
            (z_intersection - centre.z) * (z_intersection - centre.z) > radius_squared + EPSILON) {
            return result; // No intersection
            }
        
        result.happened=true;
        result.coords = Vector3f(ray.origin + ray.direction * t);
        result.normal = Vector3f(0, 1, 0); // Normal is pointing up
        result.material = this->m;
        result.obj = this;
        result.tnear = t;
        return result;
    }

    Intersection getIntersectionBottom(Ray ray){
        Intersection result;
        result.happened = false;
        
        // Cylinder bottom equation: y = 0
        // Ray equation: r(t) = o + t * d
        // Substituting the ray equation into the cylinder bottom equation gives:
        // o.y + t * d.y = 0
        // Rearranging gives a linear equation:
        if (ray.direction.y == 0) return result; // Ray is parallel to the bottom
        float t = -ray.origin.y / ray.direction.y;

        if (t < 0) return result; // Intersection is behind the ray origin
        if (t > ray.t_max) return result; // Intersection is beyond the ray max distance

        // Check if the intersection is within the radius of the cylinder
        float x_intersection = ray.origin.x + t * ray.direction.x;
        float z_intersection = ray.origin.z + t * ray.direction.z;
        if ((x_intersection - centre.x) * (x_intersection - centre.x) + 
            (z_intersection - centre.z) * (z_intersection - centre.z) > radius_squared + EPSILON) {
            return result; // No intersection
            }
        
        result.happened=true;
        result.coords = Vector3f(ray.origin + ray.direction * t);
        result.normal = Vector3f(0, -1, 0); // Normal is pointing down
        result.material = this->m;
        result.obj = this;
        result.tnear = t;
        return result;
    }


public:
    // Constructor
    Cylinder(const Vector3f &c, const float &r, const float &h, Material* mt = new Material()) 
        : centre(c), radius(r), radius_squared(r * r), height(h), m(mt), area(2 * M_PI * r * (r + h)) {}

    Intersection getIntersection(Ray ray){
        // Find the closest intersection point
        Intersection closest_intersection;

        Intersection side_intersection = getIntersectionSide(ray);
        if (side_intersection.happened) {
            closest_intersection = side_intersection;
        }
        Intersection top_intersection = getIntersectionTop(ray);
        if (top_intersection.happened) {
            if (top_intersection.tnear < closest_intersection.tnear) {
                closest_intersection = top_intersection;
            }
        }
        Intersection bottom_intersection = getIntersectionBottom(ray);
        if (bottom_intersection.happened) {
            if (bottom_intersection.tnear < closest_intersection.tnear) {
                closest_intersection = bottom_intersection;
            }
        }
        return closest_intersection;
    }

    void getSurfaceProperties(const Vector3f &P, const Vector3f &I, const uint32_t &index, const Vector2f &uv, Vector3f &N, Vector2f &st) const
    {
        // Not needed for this assignment
    }

    Vector3f evalDiffuseColor(const Vector2f &st)const {
        return m->getColor();
    }

    Bounds3 getBounds(){
        return Bounds3(Vector3f(centre.x - radius, centre.y, centre.z - radius),
                       Vector3f(centre.x + radius, centre.y+height, centre.z + radius));
    }

    void Sample(Intersection &pos, float &pdf){
        // Not needed for this assignment
    }

    float getArea() {
        return area;
    }

    bool hasEmit() {
        return m->hasEmission();
    }



};
    
#endif // RAYTRACING_CYLINDER_H