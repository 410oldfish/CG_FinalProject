//
// Created by LEI XU on 5/16/19.
//

#ifndef RAYTRACING_MATERIAL_H
#define RAYTRACING_MATERIAL_H

#include "global.hpp"
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
    //Disney BSDF
    Vector3f m_baseColor;
    float m_roughness;
    float m_metallic;
    float m_subsurface = 0.0f;
    float m_specular = 0.5f;
    float m_specularTint = 0.0f;
    float m_clearcoat = 0.0f;
    float m_clearcoatGloss = 1.0f;
    float m_sheen = 0.0f;
    float m_sheenTint = 0.5f;
    float m_transmission;
    float m_ior;
    MaterialType m_type;
    Vector3f m_emission;
    bool textured;

    inline Material();
    inline Material(MaterialType t, Vector3f e);
    inline Material(MaterialType t, Vector3f e, float r, float m);
    inline Material(MaterialType t, Vector3f e, float r, float m, Vector3f subsurface_color, float transmission);
    inline Material(Vector3f basecolor, float roughness, float metallic, float subsurface, float specular, float specularTint, float clearcoat, float clearcoatGloss, float sheen, float sheenTint, float transmission, float ior, Vector3f emission);
    inline MaterialType getType();
    inline Vector3f getColor();
    inline Vector3f getColorAt(double u, double v);
    inline Vector3f getEmission();
    inline bool hasEmission();

    // sample a ray by Material properties
    inline Vector3f sample(const Vector3f &wi, const Vector3f &N);
    inline Vector3f sample_opaque(const Vector3f &wi, const Vector3f &N);
    inline Vector3f sample_glass(const Vector3f &wi, const Vector3f &N);

    inline Vector3f sample_hemisphere_cosine();
    inline Vector3f sampleGGX(const Vector3f &N, float roughness);

    //各部分贡献权重
    

    // given a ray, calculate the PdF of this ray
    inline float pdf(const Vector3f &wi, const Vector3f &wo, const Vector3f &N);
    // given a ray direction and normal, calculate the contribution of this ray
    inline Vector3f eval(const Vector3f &wi, const Vector3f &wo, const Vector3f &N);

};
Material::Material() {
    m_type = OPAQUE;
    m_baseColor = Vector3f(1,1,1);
    m_emission=0;
    m_roughness = 1;
    m_metallic = 0;
    textured=false;
    m_ior=2;
    m_transmission = 0.0f;
}
Material::Material(MaterialType t, Vector3f color){
    m_type = t;
    m_baseColor = color;
    m_emission=0;
    m_roughness = 1;
    m_metallic = 0;
    textured=false;
    m_ior=2;
    m_transmission = 0.0f;
}

Material::Material(MaterialType t, Vector3f color, float roughness, float metallic){
    m_type = t;
    m_baseColor = color;
    m_emission=0;
    m_roughness = roughness;
    m_metallic = metallic;
    textured=false;
    m_ior=2;
    m_transmission = 0.0f;
}

Material::Material(MaterialType t, Vector3f color, float roughness, float metallic, Vector3f subsurface_color, float transmission) {
    m_type = t;
    m_baseColor = color;
    m_emission=0;
    m_roughness = roughness;
    m_metallic = metallic;
    textured=false;
    m_ior=2;
    m_transmission = transmission;
}

//Disney BSDF
inline Material::Material(Vector3f basecolor, float roughness, float metallic, float subsurface, float specular,
    float specularTint, float clearcoat, float clearcoatGloss, float sheen, float sheenTint, float transmission,
    float ior, Vector3f emission) {
    m_baseColor = basecolor;
    m_roughness = roughness;
    m_metallic = metallic;
    m_subsurface = subsurface;
    m_specular = specular;
    m_specularTint = specularTint;
    m_clearcoat = clearcoat;
    m_clearcoatGloss = clearcoatGloss;
    m_sheen = sheen;
    m_sheenTint = sheenTint;
    m_transmission = transmission;
    m_ior = ior;
    m_emission = emission;
}

MaterialType Material::getType(){return m_type;}
Vector3f Material::getColor(){return m_baseColor;}
Vector3f Material::getEmission() {return m_emission;}
bool Material::hasEmission() {
    if (m_emission.norm() > EPSILON) return true;
    else return false;
}

Vector3f Material::getColorAt(double u, double v) {
    return Vector3f();
}


Vector3f Material::sample(const Vector3f &wo, const Vector3f &N) {
    float diffuseWeight = (1.0f - m_metallic) * (1.0f - m_transmission);
    float specularWeight = (1.0f - m_transmission);
    float glassWeight = (1.0f - m_metallic) * m_transmission;
    float clearcoatWeight = 0.25f * m_clearcoat;

    float total = diffuseWeight + specularWeight + glassWeight + clearcoatWeight;
    diffuseWeight   /= total;
    specularWeight  /= total;
    glassWeight     /= total;
    clearcoatWeight /= total;

    float r = get_random_float();
    if (r < diffuseWeight) {
        return toWorld(sample_hemisphere_cosine(), N);
    } else if (r < diffuseWeight + specularWeight) {
        Vector3f h = sampleGGX(N, m_roughness);
        return reflect(-wo, h).normalized();
    } else if (r < diffuseWeight + specularWeight + glassWeight) {
        Vector3f h = sampleGGX(N, m_roughness);
        bool into = dotProduct(wo, N) > 0;
        float eta_i = into ? 1.0f : m_ior;
        float eta_t = into ? m_ior : 1.0f;
        float eta = eta_i / eta_t;
        return refract(-wo, h, eta).normalized();
    } else {
        Vector3f h = sampleGGX(N, 0.25f); // fixed roughness for clearcoat
        return reflect(-wo, h).normalized();
    }
}

Vector3f Material::sample_hemisphere_cosine() {
    float r1 = get_random_float();
    float r2 = get_random_float();

    float phi = 2.0f * M_PI * r1;
    float r = std::sqrt(r2);
    float x = r * std::cos(phi);
    float y = r * std::sin(phi);
    float z = std::sqrt(1.0f - r2);  // z = cos(θ)

    return Vector3f(x, y, z);  // in local tangent space
}

Vector3f Material::sampleGGX(const Vector3f &N, float roughness) {
    float a = roughness * roughness;

    float r1 = get_random_float();
    float r2 = get_random_float();

    float phi = 2.0f * M_PI * r1;
    float cosTheta = std::sqrt((1.0f - r2) / (1.0f + (a * a - 1.0f) * r2));
    float sinTheta = std::sqrt(1.0f - cosTheta * cosTheta);

    float x = sinTheta * std::cos(phi);
    float y = sinTheta * std::sin(phi);
    float z = cosTheta;

    Vector3f localH(x, y, z);  // in tangent space
    return toWorld(localH, N).normalized();  // return world-space half-vector
}


float Material::pdf(const Vector3f &wi, const Vector3f &wo, const Vector3f &N) {
    float cos_theta = std::max(0.0f, dotProduct(N, wi));
    if (cos_theta < EPSILON) return 0.0f;

    // 权重归一化，与 sample() 一致
    float diffuseWeight   = (1.0f - m_metallic) * (1.0f - m_transmission);
    float specularWeight  = (1.0f - m_transmission);
    float glassWeight     = (1.0f - m_metallic) * m_transmission;
    float clearcoatWeight = 0.25f * m_clearcoat;

    float totalWeight = diffuseWeight + specularWeight + glassWeight + clearcoatWeight;
    diffuseWeight   /= totalWeight;
    specularWeight  /= totalWeight;
    glassWeight     /= totalWeight;
    clearcoatWeight /= totalWeight;

    float pdf_total = 0.0f;

    // ===== 1. Diffuse lobe =====
    float pdf_diffuse = cos_theta / M_PI;
    pdf_total += diffuseWeight * pdf_diffuse;

    // ===== 2. Specular reflection (GGX) =====
    Vector3f h = (wi + wo).normalized();
    float NdotH = std::max(dotProduct(N, h), 0.0f);
    float VdotH = std::max(dotProduct(wo, h), EPSILON);
    float alpha = m_roughness * m_roughness;
    float alpha2 = alpha * alpha;
    float D = alpha2 / (M_PI * std::pow((NdotH * NdotH * (alpha2 - 1.0f) + 1.0f), 2.0f));
    float pdf_spec = (D * NdotH) / (4.0f * VdotH);
    pdf_total += specularWeight * pdf_spec;

    // ===== 3. Glass refraction (GGX) =====
    // 若没有折射，跳过此项（透明材质时 wo·wi < 0）
    if (m_transmission > 0.0f && dotProduct(wo, wi) < 0.0f) {
        // 使用 BTDF pdf，参考 Walter et al.
        float eta = dotProduct(wo, N) > 0 ? 1.0f / m_ior : m_ior;
        float HdotWo = std::max(dotProduct(h, wo), EPSILON);
        float HdotWi = std::max(dotProduct(h, wi), EPSILON);
        float sqrt_denom = eta * HdotWo + HdotWi;
        float factor = std::abs(HdotWi * HdotWo) / (sqrt_denom * sqrt_denom);

        float pdf_btdf = D * NdotH * factor;
        pdf_total += glassWeight * pdf_btdf;
    }

    // ===== 4. Clearcoat (GGX with fixed roughness) =====
    float alpha_clear = 0.25f * 0.25f;
    float Dc = alpha_clear / (M_PI * std::pow((NdotH * NdotH * (alpha_clear - 1.0f) + 1.0f), 2.0f));
    float pdf_clear = (Dc * NdotH) / (4.0f * VdotH);
    pdf_total += clearcoatWeight * pdf_clear;

    return pdf_total;
}

// Vector3f Material::eval(const Vector3f &wi, const Vector3f &wo, const Vector3f &N) {
//     if (m_type == EMIT) return Vector3f(0.0f);
//     if (dotProduct(N, wi) < EPSILON || dotProduct(N, wo) < EPSILON)
//         return Vector3f(0.0f);
//
//     Vector3f h = (wi + wo).normalized();
//     float NdotL = std::max(dotProduct(N, wi), 0.0f);
//     float NdotV = std::max(dotProduct(N, wo), 0.0f);
//     float NdotH = std::max(dotProduct(N, h), 0.0f);
//     float VdotH = std::max(dotProduct(wo, h), EPSILON);  // 避免除0
//
//     // Fresnel term (Schlick)
//     Vector3f F0 = Vector3f(0.04f);
//     F0 = lerp(F0, m_basecolor, m_metallic);  // 金属使用 baseColor 作为 F0
//     Vector3f F = F0 + (Vector3f(1.0f) - F0) * std::pow(1.0f - VdotH, 5.0f);
//
//     // ==============================
//     // 1. Disney Diffuse with FD term
//     // ==============================
//     Vector3f kd = (1.0f - m_metallic) * m_basecolor;
//
//     float HoWi = std::max(dotProduct(h, wi), 0.0f);
//     float HoWo = std::max(dotProduct(h, wo), 0.0f);
//
//     float FD90i = 0.5f + 2.0f * m_roughness * HoWi * HoWi;
//     float FD90o = 0.5f + 2.0f * m_roughness * HoWo * HoWo;
//
//     float Fi = 1.0f + (FD90i - 1.0f) * std::pow(1.0f - NdotL, 5.0f);
//     float Fo = 1.0f + (FD90o - 1.0f) * std::pow(1.0f - NdotV, 5.0f);
//
//     Vector3f disneyDiffuse = kd * Fi * Fo / M_PI;
//
//     // Optional: Add retro-reflective subsurface lobe
//     Vector3f subsurface = m_subsurface * kd * (Vector3f(1.0f) - F) / M_PI;
//
//     // ====================================
//     // 2. Specular (GGX Microfacet Model)
//     // ====================================
//     float alpha = m_roughness * m_roughness;
//     float alpha2 = alpha * alpha;
//     float denom = NdotH * NdotH * (alpha2 - 1.0f) + 1.0f;
//     float D = alpha2 / (M_PI * denom * denom);
//
//     float k = alpha / 2.0f;
//     float G_V = NdotV / (NdotV * (1.0f - k) + k);
//     float G_L = NdotL / (NdotL * (1.0f - k) + k);
//     float G = G_V * G_L;
//
//     Vector3f specular = F * D * G / (4.0f * NdotV * NdotL + EPSILON);
//
//     // ========================
//     // 3. Clearcoat GGX Lobe
//     // ========================
//     float alpha_c = 0.25f * 0.25f;
//     float Dc = alpha_c / (M_PI * std::pow(NdotH * NdotH * (alpha_c - 1.0f) + 1.0f, 2.0f));
//     float Fc = std::pow(1.0f - VdotH, 5.0f);  // Clearcoat uses fixed F0 = 0.04
//     float Gc = G_V * G_L;
//     Vector3f clear = 0.25f * m_clearcoat * Fc * Dc * Gc / (4.0f * NdotL * NdotV + EPSILON);
//
//     // ========================
//     // 4. Transmission (Thin Glass / SSS)
//     // ========================
//     Vector3f glass = Vector3f(0);
//     if (m_transmission > 0.0f && dotProduct(wo, wi) < 0.0f) {
//         glass = m_basecolor * (Vector3f(1.0f) - F) * m_transmission;
//     }
//
//     // Final BSDF composition
//     return (1.0f - m_transmission) * (disneyDiffuse + subsurface + specular + clear) + glass;
// }

Vector3f Material::eval(const Vector3f& L,const Vector3f& V,const Vector3f& N)//X Y
{
    float NdotL = dotProduct(N,L);
    float NdotV = dotProduct(N,V);
    if (NdotL < 0 || NdotV < 0) return Vector3f(0);

    Vector3f H = normalize(L+V);
    float NdotH = dotProduct(N,H);
    float LdotH = dotProduct(L,H);

    Vector3f Cdlin = mon2lin(m_baseColor);
    float Cdlum = .3*Cdlin[0] + .6*Cdlin[1]  + .1*Cdlin[2]; // luminance approx.

    Vector3f Ctint = Cdlum > 0 ? Cdlin/Cdlum : Vector3f(1); // normalize lum. to isolate hue+sat
    Vector3f Cspec0 = mix(m_specular*.08*mix(Vector3f(1), Ctint, m_specularTint), Cdlin, m_metallic);
    Vector3f Csheen = mix(Vector3f(1), Ctint, m_sheenTint);

    // Diffuse fresnel - go from 1 at normal incidence to .5 at grazing
    // and mix in diffuse retro-reflection based on roughness
    float FL = SchlickFresnel(NdotL), FV = SchlickFresnel(NdotV);
    float Fd90 = 0.5 + 2 * LdotH*LdotH * m_roughness;
    float Fd = mix(1.0, Fd90, FL) * mix(1.0, Fd90, FV);

    // Based on Hanrahan-Krueger brdf approximation of isotropic bssrdf
    // 1.25 scale is used to (roughly) preserve albedo
    // Fss90 used to "flatten" retroreflection based on roughness
    float Fss90 = LdotH*LdotH * m_roughness;
    float Fss = mix(1.0, Fss90, FL) * mix(1.0, Fss90, FV);
    float ss = 1.25 * (Fss * (1 / (NdotL + NdotV) - .5) + .5);

    // specular 只考虑各向同性
    float alpha = std::max(0.001f, m_roughness * m_roughness);
    float Ds = GTR2(NdotH, alpha);
    float FH = SchlickFresnel(LdotH);
    Vector3f Fs = mix(Cspec0, Vector3f(1), FH);
    float Gs = smithG_GGX(NdotL, alpha) * smithG_GGX(NdotV, alpha);

    // sheen
    Vector3f Fsheen = FH * m_sheen * Csheen;

    // clearcoat (ior = 1.5 -> F0 = 0.04)
    float Dr = GTR1(NdotH, mix(.1,.001,m_clearcoatGloss));
    float Fr = mix(.04, 1.0, FH);
    float Gr = smithG_GGX(NdotL, .25) * smithG_GGX(NdotV, .25);

    return ((1/M_PI) * mix(Fd, ss, m_subsurface)*Cdlin + Fsheen)
        * (1-m_metallic)
        + Gs*Fs*Ds + .25*m_clearcoat*Gr*Fr*Dr;
}

#endif //RAYTRACING_MATERIAL_H
