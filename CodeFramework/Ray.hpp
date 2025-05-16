//
// Created by LEI XU on 5/16/19.
//

// What does this class do?
// The class ray defines a ray in 3D space.
// It contains the origin and direction of the ray, as well as the time of the ray.
// DONE

#ifndef RAYTRACING_RAY_H
#define RAYTRACING_RAY_H
#include "Vector.hpp"
struct Ray{
    //Destination = origin + t*direction
    Vector3f origin;
    Vector3f direction, direction_inv;
    double t;//transportation time,
    double t_min, t_max; //t_min and t_max are the minimum and maximum distances along the ray direction

    // Constructor
    Ray(const Vector3f& ori, const Vector3f& dir, const double _t = 0.0): origin(ori), direction(dir),t(_t) {
        direction_inv = Vector3f(1./direction.x, 1./direction.y, 1./direction.z);
        t_min = 0.0;
        t_max = std::numeric_limits<double>::max();

    }
    
    // operator() function to get the point at time t along the ray
    // @param t is the time along the ray
    // @return the point at time t along the ray
    Vector3f operator()(double t) const{return origin+direction*t;}

    // friend means that this function can access private members of the Ray class
    // What does this function do?
    // It overloads the << operator to print the Ray object
    // It takes an ostream object and a Ray object as arguments
    // It returns a reference to the ostream object
    // It prints the origin, direction, and time of the ray
    // It is used for debugging and logging purposes
    friend std::ostream &operator<<(std::ostream& os, const Ray& r){
        os<<"[origin:="<<r.origin<<", direction="<<r.direction<<", time="<< r.t<<"]\n";
        return os;
    }
};
#endif //RAYTRACING_RAY_H
