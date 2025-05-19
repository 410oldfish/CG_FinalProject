//
// Created by LEI XU on 5/16/19.
//  


#ifndef RAYTRACING_MATERIAL_H
#define RAYTRACING_MATERIAL_H

#include "Vector.hpp"
#include "global.hpp"
#include <Eigen/Eigen>
#include <opencv2/opencv.hpp>

#include <functional>



enum MaterialType {DIFFUSE, GLASS, EMIT, MIRROR};

class Material{
private:


// Compute reflection direction

    // Compute the mirror reflection direction
    Vector3f reflect(const Vector3f &I, const Vector3f &N) const
    {   
        // R = I - 2 (I dot N) N
        return I - 2 * dotProduct(I, N) * N;
    }


    
    // where n_i and n_t are the refractive indices of the two materials, 
    // theta_i is the angle of incidence, and theta_t is the angle of refraction.
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



    // !! I can't understand how it works but I don't need to
    // 
    // Computes the refraction direction of an incoming ray I as it enters or exits a material with refractive index ior
    //
    // Snell's law "n_i * sin(theta_i) = n_t * sin(theta_t)"
    // 
    // @ param I is the incident ray direction
    // @ param N is the normal at the intersection point
    // @ param ior is the material refractive index
    // @ return the refracted ray direction
    Vector3f refract(const Vector3f &I, const Vector3f &N, const float &ior) const
    {   
        // Compute cos(theta_i): the angle between the incident ray and the normal
        float cosi = clamp(-1, 1, dotProduct(I, N));

        // Index of refraction inside and outside the material
        float etai = 1, etat = ior;

        // Make a copy of the normal for possible inversion
        Vector3f n = N;

        // Entering or exiting the material
        if (cosi < 0) { 
            cosi = -cosi; 
        } 
        else { 
            std::swap(etai, etat); n= -N; 
        }

        // n = n_i / n_t: how much the light will bend
        float eta = etai / etat;

        // k check whether the square root is negative
        float k = 1 - eta * eta * (1 - cosi * cosi);

        // T = eta I 
        return k < 0 ? 0 : eta * I + (eta * cosi - sqrtf(k)) * n;
    }

    // !! I can't understand how it works but I don't need to
    // 
    // Compute Fresnel equation
    //
    // It calculates how much light is reflected vs how much is refracted when a ray hits a transparent material like glass or water.
    //
    // \param I is the incident view direction
    // 
    // \param N is the normal at the intersection point
    //
    // \param ior is the material refractive index
    //
    // \param[out] kr is the amount of light reflected
    void fresnel(const Vector3f &I, const Vector3f &N, const float &ior, float &kr) const
    {
        float cosi = clamp(-1, 1, dotProduct(I, N));
        float etai = 1, etat = ior;
        if (cosi > 0) {  std::swap(etai, etat); }
        // Compute sini using Snell's law
        float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));
        // Total internal reflection
        if (sint >= 1) {
            kr = 1;
        }
        else {
            float cost = sqrtf(std::max(0.f, 1 - sint * sint));
            cosi = fabsf(cosi);
            float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
            float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
            kr = (Rs * Rs + Rp * Rp) / 2;
        }
        // As a consequence of the conservation of energy, transmittance is given by:
        // kt = 1 - kr;
    }

    // !! I can't understand how it works but I don't need to
    // Converts a vector a defined in local coordinates (relative to the surface normal) into world coordinates.
    Vector3f toWorld(const Vector3f &a, const Vector3f &N){

        // Buildin orthonormal basis N, B, C

        Vector3f B, C;
        if (std::fabs(N.x) > std::fabs(N.y)){
            float invLen = 1.0f / std::sqrt(N.x * N.x + N.z * N.z);
            C = Vector3f(N.z * invLen, 0.0f, -N.x *invLen);
        }
        else {
            float invLen = 1.0f / std::sqrt(N.y * N.y + N.z * N.z);
            C = Vector3f(0.0f, N.z * invLen, -N.y *invLen);
        }
        B = crossProduct(C, N);
        return a.x * B + a.y * C + a.z * N;
    }





public:

    // ========== Only 5 Types of Properties are supported ==========

    // Rho
    std::function<Vector3f(Vector2f)> rho_map_implicit; // base colour
    Eigen::Matrix<Vector3f, Eigen::Dynamic, Eigen::Dynamic> * rho_map; // base colour

    // kd
    std::function<Vector3f(Vector2f)> kd_map_implicit; // diffuse reflection coefficient
    Eigen::Matrix<Vector3f, Eigen::Dynamic, Eigen::Dynamic> * kd_map; // diffuse reflection coefficient

    // ks
    std::function<Vector3f(Vector2f)> ks_map_implicit; // specular reflection coefficient
    Eigen::Matrix<Vector3f, Eigen::Dynamic, Eigen::Dynamic> * ks_map; // specular reflection coefficient

    // ior
    std::function<float(Vector2f)> ior_map_implicit; // refractive index
    Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> * ior_map; // refractive index

    // opaqueness
    std::function<float(Vector2f)> opaqueness_map_implicit; // opaqueness
    Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> * opaqueness_map; // opaqueness
    







    MaterialType m_type; // either DIFFUSE, GLASS or EMIT
    Vector3f m_color; // base color
    Vector3f m_emission; // non-zero for light source
    // m_mission is the color of the light emitted by the material
    float ior;  // index of refraction (used for glass)
    float Kd, Ks;  // diffuse and specular reflection coefficients
    float specularExponent; // exponent for specular highlight sharpness
    bool textured; // whether the material is textured or not
    float opaqueness; // whether the material is opaque or not

    // Constructor, default to DIFFUSE, color to black
    inline Material(MaterialType t=DIFFUSE, Vector3f e=Vector3f(0,0,0));

    // Type of the material
    inline MaterialType getType();

    // Color of the material
    inline Vector3f getColor();

    // Get the color of the material at (u, v)
    // NOT IMPLEMENTED YET
    inline Vector3f getColorAt(double u, double v);
    inline Vector3f getEmission();
    inline bool hasEmission();

    // sample a ray by Material properties
    inline Vector3f sample(const Vector3f &wi, const Vector3f &N);

    // given a ray, calculate the PdF of this ray
    inline float pdf(const Vector3f &wi, const Vector3f &wo, const Vector3f &N);

    // given a ray direction and normal, calculate the contribution of this ray
    inline Vector3f eval(const Vector3f &dir, const Vector3f &N);



    inline Vector3f getRho(Vector2f rho_p_coords) const
    {
        // If explicit function is defined, use it
        if (rho_map && rho_map->size() !=0)
        {
            int width = rho_map->cols();
            int height = rho_map->rows();

            int x = std::clamp(static_cast<int>(rho_p_coords.x * width), 0, width - 1);
            int y = std::clamp(static_cast<int>((1.0f - rho_p_coords.y) * height), 0, height - 1);

            return (* rho_map)(y, x);  // row = y, col = x

        } else if (rho_map_implicit){

            return rho_map_implicit(rho_p_coords);

        } else
        {
            // If no texture map or implicit function is defined, return black
            std::cerr << "Error: No Explicit or Implicit Rho Map. Returning black." << std::endl;
            return Vector3f(0, 0, 0);
        }
    }

    inline Vector3f getKd(Vector2f rho_p_coords) const
    {
        // If explicit function is defined, use it
        if (kd_map && kd_map->size() !=0)
        {
            int width = kd_map->cols();
            int height = kd_map->rows();

            int x = std::clamp(static_cast<int>(rho_p_coords.x * width), 0, width - 1);
            int y = std::clamp(static_cast<int>((1.0f - rho_p_coords.y) * height), 0, height - 1);

            return (* kd_map)(y, x);  // row = y, col = x

        } else if (kd_map_implicit){

            return kd_map_implicit(rho_p_coords);

        } else
        {
            // If no texture map or implicit function is defined, return black
            std::cerr << "Error: No Explicit or Implicit Kd Map. Returning 0.8." << std::endl;
            return Vector3f(0.8, 0.8, 0.8);
        }
    }

    inline Vector3f getKs(Vector2f rho_p_coords) const
    {
        // If explicit function is defined, use it
        if (ks_map && ks_map->size() !=0)
        {
            int width = ks_map->cols();
            int height = ks_map->rows();

            int x = std::clamp(static_cast<int>(rho_p_coords.x * width), 0, width - 1);
            int y = std::clamp(static_cast<int>((1.0f - rho_p_coords.y) * height), 0, height - 1);

            return (* ks_map)(y, x);  // row = y, col = x

        } else if (ks_map_implicit){

            return ks_map_implicit(rho_p_coords);

        } else
        {
            // If no texture map or implicit function is defined, return black
            std::cerr << "Error: No Explicit or Implicit Ks Map. Returning 0.2." << std::endl;
            return Vector3f(0.2, 0.2, 0.2);
        }
    }

    inline float getIor(Vector2f rho_p_coords) const
    {
        // If explicit function is defined, use it
        if (ior_map && ior_map->size() !=0)
        {
            int width = ior_map->cols();
            int height = ior_map->rows();

            int x = std::clamp(static_cast<int>(rho_p_coords.x * width), 0, width - 1);
            int y = std::clamp(static_cast<int>((1.0f - rho_p_coords.y) * height), 0, height - 1);

            return (* ior_map)(y, x);  // row = y, col = x

        } else if (ior_map_implicit){

            return ior_map_implicit(rho_p_coords);

        } else
        {
            // If no texture map or implicit function is defined, return black
            std::cerr << "Error: No Explicit or Implicit Ior Map. Returning 1.5." << std::endl;
            return 1.5;
        }
    }

    inline float getOpaqueness(Vector2f rho_p_coords) const
    {
        // If explicit function is defined, use it
        if (opaqueness_map && opaqueness_map->size() !=0)
        {
            int width = opaqueness_map->cols();
            int height = opaqueness_map->rows();

            int x = std::clamp(static_cast<int>(rho_p_coords.x * width), 0, width - 1);
            int y = std::clamp(static_cast<int>((1.0f - rho_p_coords.y) * height), 0, height - 1);

            return (* opaqueness_map)(y, x);  // row = y, col = x

        } else if (opaqueness_map_implicit){

            return opaqueness_map_implicit(rho_p_coords);

        } else
        {
            // If no texture map or implicit function is defined, return black
            std::cerr << "Error: No Explicit or Implicit Opaqueness Map. Returning 1." << std::endl;
            return 1;
        }
    }
};






Material::Material(MaterialType t, Vector3f color){
    m_type = t;
    m_color = color;
    Kd=0.8;
    Ks=0.2;
    specularExponent=25;
    m_emission=0;
    textured=false;
    opaqueness = 1;
    ior=2;
}

MaterialType Material::getType(){return m_type;}
Vector3f Material::getColor(){return m_color;}
Vector3f Material::getEmission() {return m_emission;}
bool Material::hasEmission() {
    if (m_emission.norm() > EPSILON) return true;
    else return false;
}

Vector3f Material::getColorAt(double u, double v) {
    return Vector3f();
}





// Vector3f Material::sample(const Vector3f &dir_view_to_hit, const Vector3f &normal)
// {
//     switch (m_type)
//     {
//     case DIFFUSE:
//     {
//         // Uniform hemisphere sampling (for debugging)
//         float xi1 = get_random_float(); // z = cos(theta)
//         float xi2 = get_random_float(); // azimuthal angle phi

//         float z = xi1;
//         float r = std::sqrt(std::max(0.0f, 1.0f - z * z));
//         float phi = 2.0f * M_PI * xi2;

//         float local_x = r * std::cos(phi);
//         float local_y = r * std::sin(phi);
//         float local_z = z;

//         Vector3f local_dir(local_x, local_y, local_z);

//         // Convert to world space aligned with surface normal
//         return toWorld(local_dir, normal);
//     }

//     default:
//         return Vector3f(0);
//     }
// }


// float Material::pdf(const Vector3f &dir_view_to_hit, const Vector3f &dir_hit_to_source, const Vector3f &normal)
// {
//     switch (m_type)
//     {
//     case DIFFUSE:
//     {
//         float cos_theta = dotProduct(dir_hit_to_source, normal);
//         return (cos_theta > 0.0f) ? 1.0f / (2.0f * M_PI) : 0.0f;
//     }

//     default:
//         return 0.0f;
//     }
// }









// // Cosine-weighted hemisphere sampling
// // Cosine-weighted hemisphere sampling for Lambertian BRDF
Vector3f Material::sample(const Vector3f &dir_view_to_hit, const Vector3f &normal)
{

    float xi1 = get_random_float();
    float xi2 = get_random_float();

    float r = std::sqrt(xi1);
    float phi = 2.0f * M_PI * xi2;

    float local_x = r * std::cos(phi);
    float local_y = r * std::sin(phi);
    float local_z = std::sqrt(1.0f - xi1); // = cos(theta)

    Vector3f local_dir(local_x, local_y, local_z);

    // Convert to world direction starting from hit point going outward
    Vector3f dir_hit_to_next = toWorld(local_dir, normal);
    return dir_hit_to_next;

    // switch (m_type)
    // {
    // case DIFFUSE:
    // {
    //     float xi1 = get_random_float();
    //     float xi2 = get_random_float();

    //     float r = std::sqrt(xi1);
    //     float phi = 2.0f * M_PI * xi2;

    //     float local_x = r * std::cos(phi);
    //     float local_y = r * std::sin(phi);
    //     float local_z = std::sqrt(1.0f - xi1); // = cos(theta)

    //     Vector3f local_dir(local_x, local_y, local_z);

    //     // Convert to world direction starting from hit point going outward
    //     Vector3f dir_hit_to_next = toWorld(local_dir, normal);
    //     return dir_hit_to_next;
    // }

    // default:
    //     return Vector3f(0);
    // }
}


float Material::pdf(const Vector3f &dir_view_to_hit, const Vector3f &dir_hit_to_source, const Vector3f &normal)
{
    float cos_theta = dotProduct(dir_hit_to_source, normal);
    return (cos_theta > 0.0f) ? cos_theta / M_PI : 0.0f;
    // switch (m_type)
    // {
    // case DIFFUSE:
    // {
    //     float cos_theta = dotProduct(dir_hit_to_source, normal);
    //     return (cos_theta > 0.0f) ? cos_theta / M_PI : 0.0f;
    // }

    // default:
    //     return 0.0f;
    // }
}





// // Goal: generate a random direction w_o over the hemisphere 
// // given the normal N and the incoming direction w_i
// Vector3f Material::sample(const Vector3f &wi, const Vector3f &N){
//     // uniform sample on the hemisphere
//     float x_1 = get_random_float(), x_2 = get_random_float(); // [0, 1)

//     float z = std::fabs(1.0f - 2.0f * x_1); // height of the point

//     float r = std::sqrt(1.0f - z * z), phi = 2 * M_PI * x_2; // radius from center to the point

//     Vector3f localRay(r*std::cos(phi), r*std::sin(phi), z); // x, y, z coordinates

//     return toWorld(localRay, N); // convert to world coordinates
//     // switch(m_type){
//     //     case DIFFUSE:
//     //     {
//     //         // uniform sample on the hemisphere
//     //         float x_1 = get_random_float(), x_2 = get_random_float(); // [0, 1)

//     //         float z = std::fabs(1.0f - 2.0f * x_1); // height of the point

//     //         float r = std::sqrt(1.0f - z * z), phi = 2 * M_PI * x_2; // radius from center to the point

//     //         Vector3f localRay(r*std::cos(phi), r*std::sin(phi), z); // x, y, z coordinates

//     //         return toWorld(localRay, N); // convert to world coordinates
            
//     //         break;
//     //     }
//     // }
// }


// float Material::pdf(const Vector3f &dir_view_to_hit, const Vector3f &dir_hit_to_source, const Vector3f &normal)
// {
//     float cos_theta = dotProduct(dir_hit_to_source, normal);
//     return (cos_theta > 0.0f) ? 0.5f / M_PI : 0.0f;
//     // switch (m_type)
//     // {
//     // case DIFFUSE:
//     // {
//     //     float cos_theta = dotProduct(dir_hit_to_source, normal);
//     //     return (cos_theta > 0.0f) ? cos_theta / M_PI : 0.0f;
//     // }

//     // default:
//     //     return 0.0f;
//     // }
// }






// given a ray direction and normal, calculate the contribution of this ray
// Input: dir is the direction of the ray, N is the normal at the intersection point
Vector3f Material::eval(const Vector3f &dir, const Vector3f &N){
    switch(m_type){
        default:
        {
            // calculate the contribution of diffuse   model
            float cosalpha = dotProduct(N, dir); // cosine of the angle between the normal and the ray direction
            if (cosalpha > 0.0f) { // if the ray is in the same hemisphere as the normal
                Vector3f diffuse = Kd / M_PI; // for lambertian surface, the diffuse reflection coefficient is Kd / PI
                return diffuse;
            }
            else
                return Vector3f(0.0f);
            break;
        }
    }
}

#endif //RAYTRACING_MATERIAL_H
