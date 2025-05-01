//
// Created by LEI XU on 5/16/19.
//

#ifndef RAYTRACING_MATERIAL_H
#define RAYTRACING_MATERIAL_H

#include "Vector.hpp"

enum MaterialType { OPAQUE, TRANSPARENT, EMIT};

class Material{
private:



// Compute reflection direction
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

    // Compute Fresnel equation
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

    Vector3f toWorld(const Vector3f &a, const Vector3f &N) {
        Vector3f B, C;
        if (std::fabs(N.x) > std::fabs(N.y))
            B = Vector3f(N.z, 0, -N.x).normalized();
        else
            B = Vector3f(0, -N.z, N.y).normalized();
        C = crossProduct(B, N);
        return a.x * B + a.y * C + a.z * N;
    }


public:
    MaterialType m_type;
    Vector3f m_color;
    Vector3f m_emission;
    float ior;  // index of refraction
    bool textured;
    float m_roughness;
    float m_metallic;

    inline Material();
    inline Material(MaterialType t, Vector3f e);
    inline Material(MaterialType t, Vector3f e, float r, float m);
    inline MaterialType getType();
    inline Vector3f getColor();
    inline Vector3f getColorAt(double u, double v);
    inline Vector3f getEmission();
    inline bool hasEmission();

    // sample a ray by Material properties
    inline Vector3f sample(const Vector3f &wi, const Vector3f &N);
    inline Vector3f sample_opaque(const Vector3f &wi, const Vector3f &N);
    inline Vector3f sample_glass(const Vector3f &wi, const Vector3f &N);
    // given a ray, calculate the PdF of this ray
    inline float pdf(const Vector3f &wi, const Vector3f &wo, const Vector3f &N);
    // given a ray direction and normal, calculate the contribution of this ray
    inline Vector3f eval(const Vector3f &wi, const Vector3f &wo, const Vector3f &N);

};
Material::Material() {
    m_type = OPAQUE;
    m_color = Vector3f(1,1,1);
    m_emission=0;
    m_roughness = 1;
    m_metallic = 0;
    textured=false;
    ior=2;
}
Material::Material(MaterialType t, Vector3f color){
    m_type = t;
    m_color = color;
    m_emission=0;
    m_roughness = 1;
    m_metallic = 0;
    textured=false;
    ior=2;
}

Material::Material(MaterialType t, Vector3f color, float roughness, float metallic){
    m_type = t;
    m_color = color;
    m_emission=0;
    m_roughness = roughness;
    m_metallic = metallic;
    textured=false;
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


Vector3f Material::sample(const Vector3f &wo, const Vector3f &N) {
    switch (m_type) {
        case OPAQUE: {
            return sample_opaque(wo, N);
        }

        case TRANSPARENT: {
            return sample_glass(wo, N);
        }

        case EMIT: {
            return Vector3f(0,0,0);
        }
    }
    return Vector3f(0,0,0);
}

Vector3f Material::sample_opaque(const Vector3f &wo, const Vector3f &N) {
    float r = get_random_float();

    if (r < m_metallic) {
        // GGX sampling
        // importance sample half-vector h from GGX distribution
        float r1 = get_random_float();
        float r2 = get_random_float();

        float alpha = m_roughness * m_roughness;
        float phi = 2.0f * M_PI * r1;
        float cosTheta = std::sqrt((1.0f - r2) / (1.0f + (alpha * alpha - 1.0f) * r2));
        float sinTheta = std::sqrt(1.0f - cosTheta * cosTheta);

        Vector3f h_local = Vector3f(
            sinTheta * std::cos(phi),
            sinTheta * std::sin(phi),
            cosTheta
        );
        Vector3f h = toWorld(h_local, N);
        return reflect(-wo, h).normalized();  // 反射方向
    } else {
        // cosine-weighted hemisphere sampling (Lambertian)
        float r1 = get_random_float();
        float r2 = get_random_float();
        float theta = std::acos(std::sqrt(1.0f - r1));
        float phi = 2.0f * M_PI * r2;
        float x = std::sin(theta) * std::cos(phi);
        float y = std::sin(theta) * std::sin(phi);
        float z = std::cos(theta);
        Vector3f local(x, y, z);
        return toWorld(local, N).normalized();
    }
}

Vector3f Material::sample_glass(const Vector3f &wo, const Vector3f &N) {
    float ior = this->ior;  // 折射率，默认 1.5
    Vector3f normal = N;
    bool into = dotProduct(wo, N) < 0;

    if (!into) {
        normal = -N;
        ior = 1.0f / ior;  // 从物体内部射出
    }

    float kr = 0;
    fresnel(wo, normal, ior, kr);
    if (get_random_float() < kr) {
        // 反射路径
        return reflect(wo, normal).normalized();
    } else {
        // 折射路径
        return refract(wo, normal, ior).normalized();
    }
}

float Material::pdf(const Vector3f &wi, const Vector3f &wo, const Vector3f &N) {
    float cos_theta = std::max(0.0f, dotProduct(N, wi));

    // GGX PDF (from Walter et al., "Microfacet Models for Refraction and Reflection")
    Vector3f h = (wo + wi).normalized();
    float NdotH = std::max(dotProduct(N, h), 0.0f);
    float VdotH = std::max(dotProduct(wo, h), 0.0f);

    float alpha = m_roughness * m_roughness;
    float alpha2 = alpha * alpha;
    float denom = NdotH * NdotH * (alpha2 - 1.0f) + 1.0f;
    float D = alpha2 / (M_PI * denom * denom);

    float pdf_spec = (D * NdotH) / (4.0f * VdotH + EPSILON);
    float pdf_diffuse = cos_theta / M_PI;

    return m_metallic * pdf_spec + (1.0f - m_metallic) * pdf_diffuse;
}

Vector3f Material::eval(const Vector3f &wi, const Vector3f &wo, const Vector3f &N) {
    if (m_type == TRANSPARENT) {
        return Vector3f(0);
    }
    if (m_type == EMIT) {
        return Vector3f(0);
    }

    Vector3f h = (wi + wo).normalized();
    float NdotL = std::max(dotProduct(N, wi), 0.0f);
    float NdotV = std::max(dotProduct(N, wo), 0.0f);
    float NdotH = std::max(dotProduct(N, h), 0.0f);
    float VdotH = std::max(dotProduct(wo, h), 0.0f);

    if (NdotL <= 0.0f || NdotV <= 0.0f)
        return Vector3f(0.0f);

    // Diffuse (Lambert)
    Vector3f kd = (1.0f - m_metallic) * m_color;
    Vector3f diffuse = kd / M_PI;

    // Fresnel (Schlick)
    Vector3f F0 = Vector3f(0.04f); // 电介质默认反射率
    F0 = lerp(F0, m_color, m_metallic);  // 金属使用 baseColor 作为 F0
    Vector3f F = F0 + (Vector3f(1.0f) - F0) * std::pow(1.0f - VdotH, 5.0f);

    // Normal Distribution Function (GGX)
    float alpha = m_roughness * m_roughness;
    float alpha2 = alpha * alpha;
    float denom = NdotH * NdotH * (alpha2 - 1.0f) + 1.0f;
    float D = alpha2 / (M_PI * denom * denom);

    // Geometry term (Smith's method)
    float k = alpha / 2.0f;
    float G_Schlick_NdotL = NdotL / (NdotL * (1.0f - k) + k);
    float G_Schlick_NdotV = NdotV / (NdotV * (1.0f - k) + k);
    float G = G_Schlick_NdotL * G_Schlick_NdotV;

    // Specular
    Vector3f spec = (F * D * G) / (4.0f * NdotL * NdotV + EPSILON);

    return diffuse + spec;
}


#endif //RAYTRACING_MATERIAL_H
