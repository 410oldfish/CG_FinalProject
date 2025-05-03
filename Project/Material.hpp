//
// Created by LEI XU on 5/16/19.
//

#ifndef RAYTRACING_MATERIAL_H
#define RAYTRACING_MATERIAL_H

#include "global.hpp"
#include "Vector.hpp"
#include "frame.h"
#include "microfacet.h"
#include "pcg.h"
class Material{
private:
    Vector3 reflect(const Vector3 &i, const Vector3 &h) {
        return normalize(-i + 2 * dot(i, h) * h);
    }

    Vector3 refract(const Vector3 &i, const Vector3 &h, double eta) {
        double cos_theta_i = dot(i, h);
        double sin2_theta_i = max(double(0), 1 - cos_theta_i * cos_theta_i);
        double sin2_theta_t = sin2_theta_i / (eta * eta);

        // Total Internal Reflection
        if (sin2_theta_t >= 1) return Vector3(0);
        auto half = h;
        if(cos_theta_i < 0) half = -h;

        double cos_theta_t = sqrt(1 - sin2_theta_t);
        if (fabs(cos_theta_t) < 1e-6) cos_theta_t = 1e-6;
        return normalize(-i / eta + (fabs(cos_theta_i) / eta - cos_theta_t) * half + 1e-6 * half);
    }
public:
    //Disney BSDF
    Vector3 m_baseColor;
    double m_roughness;
    double m_metallic;
    double m_subsurface = 0.0f;
    double m_specular = 0.5f;
    double m_specularTint = 0.0f;
    double m_clearcoat = 0.0f;
    double m_clearcoatGloss = 1.0f;
    double m_sheen = 0.0f;
    double m_sheenTint = 0.5f;
    double m_transmission;
    double m_anisotropoc;
    double m_ior;
    Vector3f m_emission;
    bool textured;

    inline Material();
    inline Material(Vector3f basecolor, double roughness, double metallic, double subsurface, double specular, double specularTint, double clearcoat, double clearcoatGloss, double sheen, double sheenTint, double transmission,double transmission_roughness, double ior, Vector3f emission);
    inline Vector3f getColor();
    inline Vector3f getColorAt(double u, double v);
    inline Vector3f getEmission();
    inline bool hasEmission();
    inline std::vector<double> calculateWeight()const;
    // sample a ray by Material properties
    inline Vector3f sample(const Vector3 &wi, const Vector3 &N, Vector2 &rnd_param_uv, double &rnd_param_w, pcg32_state &rng);
    // given a ray, calculate the PdF of this ray
    inline double pdf(const Vector3 &wi, const Vector3 &wo, const Vector3 &N);
    // given a ray direction and normal, calculate the contribution of this ray
    inline Vector3f eval(const Vector3 &wi, const Vector3 &wo, const Vector3 &N);

};

inline Material::Material() {
}

//Disney BSDF
inline Material::Material(Vector3f basecolor, double roughness, double metallic, double subsurface, double specular,
    double specularTint, double clearcoat, double clearcoatGloss, double sheen, double sheenTint, double transmission,
    double anisotropic,double ior, Vector3f emission) {
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
    m_anisotropoc = anisotropic;
    m_ior = ior;
    m_emission = emission;
}

Vector3f Material::getColor(){return m_baseColor;}
Vector3f Material::getEmission() {return m_emission;}
bool Material::hasEmission() {
    if (length(m_emission) > EPSILON) return true;
    else return false;
}

Vector3f Material::getColorAt(double u, double v) {
    return Vector3f();
}

inline Vector3 sample_cos_hemisphere(const Vector2 &rnd_param) {
    double phi = c_TWOPI * rnd_param[0];
    double tmp = sqrt(std::clamp(1 - rnd_param[1], double(0), double(1)));
    return Vector3{
        cos(phi) * tmp, sin(phi) * tmp,
        sqrt(std::clamp(rnd_param[1], double(0), double(1)))
    };
}

Vector3f Material::sample(const Vector3 &wo, const Vector3 &N, Vector2 &rnd_param_uv, double &rnd_param_w,pcg32_state &rng) {
    // diffuse, clearcoat, metal, glass, sheen
    auto weightVector = calculateWeight();
    double diffuseWeight = weightVector[0];
    double clearcoatWeight = weightVector[1];
    double metalWeight = weightVector[2];
    double glassWeight = weightVector[3];
    double sheenWeight = weightVector[4];

    double r = next_pcg32_real<double>(rng);
    Frame frame = Frame(N);
    if (r < diffuseWeight) {
        if (dot(wo, N) < 0) {
            return Vector3f(0);
        }
        if (dot(N, wo) < 0) {
            return {};
        }
        return to_world(frame, sample_cos_hemisphere(rnd_param_uv));
    } else if (r < diffuseWeight + clearcoatWeight) {
        if (dot(N, wo) <= 0) {
            return {};
        }
        double alpha = (1 - m_clearcoatGloss) * 0.1 + m_clearcoatGloss * 0.001;
        double a2 = alpha * alpha;

        double h_elevation = acos(sqrt((1-pow(a2, 1-rnd_param_uv[0])) / (1-a2)));
        double h_azimuth = 2 * M_PI * rnd_param_uv[1];

        Vector3f h_local;
        h_local.x = sin(h_elevation) * cos(h_azimuth);
        h_local.y = sin(h_elevation) * sin(h_azimuth);
        h_local.z = cos(h_elevation);

        Vector3 half_vector = to_world(frame, h_local);
        Vector3 reflected = normalize(-wo + 2 * dot(wo, half_vector) * half_vector);
        return reflected;
    } else if (r < diffuseWeight + clearcoatWeight + metalWeight) {
        if (dot(N, wo) <= 0) {
            return {};
        }

        double alpha_x, alpha_y;
        AnisoTransform(m_roughness, m_anisotropoc, alpha_x, alpha_y);

        Vector3 local_micro_normal = sample_visible_normals(to_local(frame, wo), alpha_x, alpha_y, rnd_param_uv);

        Vector3 half_vector = to_world(frame, local_micro_normal);
        Vector3 reflected = normalize(-wo + 2 * dot(wo, half_vector) * half_vector);
        return reflected;
    } else if (r < diffuseWeight + clearcoatWeight + metalWeight + glassWeight){
        if (dot(N, wo) < 0) {
            frame = -frame;
        }
        double alpha_x, alpha_y;
        AnisoTransform(m_roughness, m_anisotropoc, alpha_x, alpha_y);

        // Sample a micro normal and transform it to world space -- this is our half-vector.
        bool inner = dot(N, wo) < 0;
        double eta = inner ? 1 / m_ior : m_ior;
        Vector3 local_in = to_local(frame, wo);
        Vector3 half_vector = to_world(frame, sample_visible_normals(local_in, alpha_x, alpha_y, rnd_param_uv));
        if (dot(half_vector, frame.n) < 0) half_vector = -half_vector;
        double F_g = fresnel_dielectric(dot(half_vector, wo), eta);
        if (F_g > rnd_param_w)
        {
            Vector3 reflected = reflect(wo, half_vector);
            return reflected;
        }
        else
        {
            Vector3 refracted = refract(wo, half_vector, eta);
            if(refracted == Vector3(0)) return {};
            return refracted;
        }
    }
    else { //sheenWeight
            if (dot(N, wo)<=0) {
                return {};
            }
        return to_world(frame, sample_cos_hemisphere(rnd_param_uv));
    }
}

std::vector<double> Material::calculateWeight() const {
    // 权重归一化，与 sample() 一致
    // diffuse, clearcoat, metal, glass, sheen
    double diffuseWeight = (1.0f - m_metallic) * (1.0f - m_transmission);
    double clearcoatWeight = 0.25f * m_clearcoat;
    double metalWeight = (1 - m_transmission * (1 - m_metallic));
    double glassWeight = (1.0f - m_metallic) * m_transmission;
    double sheenWeight = (1.0f - m_metallic) * m_sheen;


    double total = diffuseWeight + clearcoatWeight + metalWeight + glassWeight + sheenWeight;
    diffuseWeight   /= total;
    clearcoatWeight  /= total;
    metalWeight     /= total;
    glassWeight /= total;
    sheenWeight /= total;

    return std::vector<double>{diffuseWeight, clearcoatWeight, metalWeight, glassWeight, sheenWeight};
}

double Material::pdf(const Vector3 &wi, const Vector3 &wo, const Vector3 &N) {
    auto weights = calculateWeight();
    double diffuseWeight = weights[0];
    double clearcoatWeight  = weights[1];
    double metalWeight  = weights[2];
    double glassWeight = weights[3];
    double sheenWeight = weights[4];

    bool reflect = dot(N, wi) *
                   dot(N, wo) > 0;
    // Flip the shading frame if it is inconsistent with the geometry normal
    Frame frame = Frame(N);
    // Homework 1: implement this!
    if(!reflect || dot(N, wi) <= 0){
        // only glass
        double alpha_x, alpha_y;
        AnisoTransform(m_roughness, m_anisotropoc, alpha_x, alpha_y);

        bool inner = dot(N, wi) < 0;
        double eta = inner ? 1 / m_ior : m_ior;

        Vector3 half_vector;
        if(reflect)half_vector = normalize(wi + wo); 
        else half_vector = normalize(wi + eta * wo);
        if(length_squared(-wi - eta * wo) < 1e-3) {
            half_vector = normalize(cross(cross(wo, frame.n), wo));
        }
        if (dot(half_vector, frame.n) < 0) half_vector = -half_vector;

        double h_dot_in = dot(half_vector, wi);
        double h_dot_out = dot(half_vector, wo);


        double F_g = fresnel_dielectric(h_dot_in, eta);
        double D_g = GTR2Aniso(to_local(frame, half_vector), alpha_x, alpha_y);
        if (reflect) return (F_g * D_g) / (4 * fabs(h_dot_in));
        else
        {
            double numerator = h_dot_in + h_dot_out * eta;
            if (fabs(numerator) < 1e-4) numerator = 1e-4;
            double jacobian = eta * eta / (numerator * numerator);
            return (1 - F_g) * D_g * fabs(h_dot_out * h_dot_in / dot(frame.n, wi)) * jacobian;
        }
    }
    else{
        //---------Diffuse-----------------
        double diffuse_bsdf_pdf;
        if (dot(N, wi) < 0 ||
            dot(N, wo) < 0) {
            // No light below the surface
            diffuse_bsdf_pdf = 0;
            }
        else if (dot(frame.n, wi) < 0) {
            diffuse_bsdf_pdf = 0;
        }
        else {
            diffuse_bsdf_pdf = fmax(dot(frame.n, wo), double(0)) / M_PI;
        }
        //--------ClearCoat----------------------
        double clearcoat_bsdf_pdf;
        if (dot(N, wi) < 0 ||
            dot(N, wo) < 0) {
            // No light below the surface
            clearcoat_bsdf_pdf = 0;
            }
        // Flip the shading frame if it is inconsistent with the geometry normal
        else if (dot(frame.n, wi) <= 0) {
            clearcoat_bsdf_pdf = 0;
        }
        else {
            // Homework 1: implement this!
            Vector3 half_vector = normalize(wi + wo);
            double n_dot_out = dot(frame.n, wo);
            double n_dot_h = dot(frame.n, half_vector);
            if (n_dot_out <= 0 || n_dot_h <= 0) {
                clearcoat_bsdf_pdf = 0;
            }
            else {
                double alpha_g = (1 - m_clearcoatGloss) * 0.1 + m_clearcoatGloss * 0.001;

                double D_c = GTR1(n_dot_h, alpha_g);

                clearcoat_bsdf_pdf = D_c * n_dot_h / (4 * n_dot_out);
            }
        }

        //--------Metal---------------------------
        double metal_bsdf_pdf;
        if (dot(N, wi) < 0 ||
            dot(N, wo) < 0) {
            // No light below the surface
            metal_bsdf_pdf = 0;
            }
        else if (dot(frame.n, wi) <= 0) {
            metal_bsdf_pdf = 0;
        }
        else {
            // Homework 1: implement this!
            Vector3 half_vector = normalize(wi + wo);
            double n_dot_out = dot(frame.n, wo);
            double n_dot_h = dot(frame.n, half_vector);
            double h_dot_in = dot(half_vector, wi);
            if (n_dot_out <= 0 || n_dot_h <= 0) {
                metal_bsdf_pdf = 0;
            }
            else {
                double alpha_x, alpha_y;
                AnisoTransform(m_roughness, m_anisotropoc, alpha_x, alpha_y);
                double D = GTR2Aniso(to_local(frame, half_vector), alpha_x, alpha_y);
                metal_bsdf_pdf = D / (4 * h_dot_in);
            }
        }

        //------Glass---------------------
        double glass_bsdf_pdf;

        // Flip the shading frame if it is inconsistent with the geometry normal
        // if (dot(frame.n, wi) * dot(vertex.geometric_normal, wi) < 0) {
        //     frame = -frame;
        // }
        double alpha_x, alpha_y;
        AnisoTransform(m_roughness, m_anisotropoc, alpha_x, alpha_y);

        bool inner = dot(N, wi) < 0;
        double eta = inner ? 1 / m_ior : m_ior;

        Vector3 half_vector;
        if(reflect)half_vector = normalize(wi + wo);
        else half_vector = normalize(wi + eta * wo);
        if(length_squared(-wi - eta * wo) < 1e-3) {
            half_vector = normalize(cross(cross(wo, frame.n), wo));
        }
        if (dot(half_vector, frame.n) < 0) half_vector = -half_vector;

        double h_dot_in = dot(half_vector, wi);
        double h_dot_out = dot(half_vector, wo);


        double F_g = fresnel_dielectric(h_dot_in, eta);
        double D_g = GTR2Aniso(to_local(frame, half_vector), alpha_x, alpha_y);
        if (reflect) glass_bsdf_pdf = (F_g * D_g) / (4 * fabs(h_dot_in));
        else
        {
            double numerator = h_dot_in + h_dot_out * eta;
            if (fabs(numerator) < 1e-4) numerator = 1e-4;
            double jacobian = eta * eta / (numerator * numerator);
            glass_bsdf_pdf = (1 - F_g) * D_g * fabs(h_dot_out * h_dot_in / dot(frame.n, wi)) * jacobian;
        }
        //---------Sheen------------------------
        double sheen_bsdf_pdf;
        if (dot(N, wi) < 0 ||
                   dot(N, wo) < 0) {
            // No light below the surface
            sheen_bsdf_pdf = 0;
                   }
        else if (dot(frame.n, wi) <= 0) {
            sheen_bsdf_pdf = 0;
        }
        else {
            sheen_bsdf_pdf = fmax(dot(frame.n, wo), double(0)) / M_PI;
        }
        //-----------------------------------------
        double result = double(0);

        result += diffuseWeight * diffuse_bsdf_pdf;
        result += clearcoatWeight * clearcoat_bsdf_pdf;
        result += metalWeight * metal_bsdf_pdf;
        result += glassWeight * glass_bsdf_pdf;
        result += sheenWeight * sheen_bsdf_pdf;

        return result;
    }
}

Vector3f Material::eval(const Vector3& wi,const Vector3& wo,const Vector3& N)//X Y
{
    auto weightVector = calculateWeight();
    double diffuseWeight = weightVector[0];
    double clearcoatWeight = weightVector[1];
    double metalWeight = weightVector[2];
    double glassWeight = weightVector[3];
    double sheenWeight = weightVector[4];

    bool reflect = dot(N, wi) *
                   dot(N, wo) > 0;
    // Flip the shading frame if it is inconsistent with the geometry normal
    Frame frame = Frame(N);
    // Homework 1: implement this!
    if(!reflect || dot(N, wi) <= 0){
        // only glass
        // Flip the shading frame if it is inconsistent with the geometry normal
        // if (dot(N, wi) < 0) {
        //     frame = -frame;
        // }
        // Homework 1: implement this!
        double alpha_x, alpha_y;
        AnisoTransform(m_roughness, m_anisotropoc, alpha_x, alpha_y);

        bool inner = dot(N, wi) < 0;
        double eta = inner ? 1 / m_ior : m_ior;

        double n_dot_in = fabs(dot(N, wi));
        double n_dot_out = fabs(dot(N, wo));

        Vector3 half_vector;
        if(reflect)half_vector = normalize(wi + wo);
        else half_vector = normalize(-wi - eta * wo);
        if(length_squared(-wi - eta * wo) < 1e-3) {
            half_vector = normalize(cross(cross(wo, N), wo));
        }
        if (dot(half_vector, frame.n) < 0) half_vector = -half_vector;

        double h_dot_in = dot(half_vector, wi); // cos(theta_i)
        double h_dot_out = dot(half_vector, wo); // cos(theta_t)


        double F_g =  fresnel_dielectric(h_dot_in, eta);
        double G_g = smith_masking_gtr2_aniso(to_local(frame, wi), alpha_x, alpha_y) *
                   smith_masking_gtr2_aniso(to_local(frame, wo), alpha_x, alpha_y);
        double D_g = GTR2Aniso(to_local(frame, half_vector), alpha_x, alpha_y);
        if (reflect)
        {
            return m_baseColor * F_g * G_g * D_g / (4 * n_dot_in);
        }
        else
        {
            double denominator = h_dot_in + h_dot_out * eta;
            return m_baseColor * ((1 - F_g) * G_g * D_g * abs(h_dot_in * h_dot_out)) * eta * eta / (abs(n_dot_in) * denominator * denominator);
            // return sqrt(base_clr) * ((1 - F_g) * G_g * D_g * abs(h_dot_in * h_dot_out)) / (abs(n_dot_in) * denominator * denominator);
        }
    }
    else{
        Spectrum C0;
        {
            double l = luminance(m_baseColor);
            Spectrum Ctint = Spectrum(1);
            if (l > 0) Ctint = m_baseColor / l;
            double eta = dot(N, wi) > 0 ? m_ior : 1 / m_ior;
            Spectrum Ks = (1 - (double)m_specularTint) + (double)m_specularTint * Ctint;
            double r0 = (1.0 - eta) / (1.0 + eta);
            r0 = r0 * r0;
            C0 = m_specular * r0 * (1 - m_metallic) * Ks + m_metallic * m_baseColor;
        }

        Spectrum f_Diffuse = make_zero_spectrum();
        Spectrum f_Clearcoat = make_zero_spectrum();
        Spectrum f_Sheen = make_zero_spectrum();
        Spectrum f_Glass = make_zero_spectrum();
        Spectrum f_Metal = make_zero_spectrum();

        //--repeat code for glass
        // Homework 1: implement this!
        double alpha_x, alpha_y;
        AnisoTransform(m_roughness, m_anisotropoc, alpha_x, alpha_y);

        bool inner = dot(N, wi) < 0;
        double eta = inner ? 1 / m_ior : m_ior;

        double n_dot_in = fabs(dot(N, wi));
        double n_dot_out = fabs(dot(N, wo));

        Vector3 half_vector;
        if(reflect)half_vector = normalize(wi + wo);
        else half_vector = normalize(-wi - eta * wo);
        if(length_squared(-wi - eta * wo) < 1e-3) {
            half_vector = normalize(cross(cross(wo, N), wo));
        }
        if (dot(half_vector, frame.n) < 0) half_vector = -half_vector;

        double h_dot_in = dot(half_vector, wi); // cos(theta_i)
        double h_dot_out = dot(half_vector, wo); // cos(theta_t)


        double F_g =  fresnel_dielectric(h_dot_in, eta);
        double G_g = smith_masking_gtr2_aniso(to_local(frame, wi), alpha_x, alpha_y) *
                   smith_masking_gtr2_aniso(to_local(frame, wo), alpha_x, alpha_y);
        double D_g = GTR2Aniso(to_local(frame, half_vector), alpha_x, alpha_y);
        if (reflect)
        {
            f_Glass = m_baseColor * F_g * G_g * D_g / (4 * n_dot_in);
        }
        else
        {
            double denominator = h_dot_in + h_dot_out * eta;
            f_Glass = m_baseColor * ((1 - F_g) * G_g * D_g * abs(h_dot_in * h_dot_out)) * eta * eta / (abs(n_dot_in) * denominator * denominator);
        }

        if (dot(N, wo) > 0)
        {
            //------Metal--------------
            if (dot(N, wi) < 0 ||
            dot(N, wo) < 0) {
                // No light below the surface
                f_Metal = make_zero_spectrum();
            }
            else {

                if (dot(frame.n, wi) <= 0) {
                    f_Metal = make_zero_spectrum();
                }

                // Homework 1: implement this!
                Vector3 half_vector = normalize(wi + wo);
                if(length_squared(half_vector) == 0) half_vector = frame.n;

                double n_dot_h = dot(frame.n, half_vector);
                double n_dot_in = dot(frame.n, wi);
                double n_dot_out = dot(frame.n, wo);
                double h_dot_out = dot(half_vector, wo);
                if (n_dot_out <= 0 || n_dot_h <= 0) {
                    f_Metal = make_zero_spectrum();
                }

                double alpha_x, alpha_y;
                AnisoTransform(m_roughness, m_anisotropoc, alpha_x, alpha_y);

                Spectrum tint = Spectrum(0.0, 1.0, 0.0);
                double tint_strength = 1.0;

                Spectrum F_m = schlick_fresnel(m_baseColor, h_dot_out);
                // Spectrum F_m = schlick_generalized_fresnel(base_clr, h_dot_out, 5.0, tint_strength, tint);

                double D_m = GTR2Aniso(to_local(frame, half_vector), alpha_x, alpha_y);
                double G_m = smith_masking_gtr2_aniso(to_local(frame, wi), alpha_x, alpha_y) *
                           smith_masking_gtr2_aniso(to_local(frame, wo), alpha_x, alpha_y);

                f_Metal = F_m * D_m * G_m / (4 * n_dot_in);
            }

            //----------------------Diffuse----------------------------------------------------------------
            if (dot(N, wi) < 0 ||
                        dot(N, wo) < 0) {
                // No light below the surface
                f_Diffuse = make_zero_spectrum();
                        }
            else {
                double ndin = dot(frame.n, wi);
                double ndout = dot(frame.n, wo);
                if (ndin <= 0 || ndout <= 0) {
                    f_Diffuse = make_zero_spectrum();
                }

                Vector3 h = wi + wo;
                if(length_squared(h) == 0) h = frame.n;
                h = normalize(h);
                double hdout = max(0.0, dot(h, wo));


                auto FresnelSchlick = [](double F0, double cosTheta) {
                    return 1.0 + (F0 - 1.0) * pow(1.0 - cosTheta, 5);
                };

                double FSS90 = m_roughness * hdout * hdout;

                double denominator = ndin + ndout;
                if(denominator < 0.06) denominator = 0.06;
                double ssCoeff = 1.25 * (FresnelSchlick(FSS90, ndin) * FresnelSchlick(FSS90, ndout) * (1.0 / denominator - 0.5) + 0.5);

                /* 2012 Disney */
                // double FD90 = 0.5 + 2 * FSS90;
                // double baseCoeff = FresnelSchlick(FD90, ndin) * FresnelSchlick(FD90, ndout);
                // return base_color / c_PI * ( (1.0 - ss) * baseCoeff + ss * ssCoeff ) * ndout;

                /* 2015 Disney */
                double FL = pow((1 - ndin), double(5));
                double FV = pow((1 - ndout), double(5));
                double Rr = 2 * FSS90;
                double retroCoeff = Rr * (FL + FV + FL * FV * (Rr - 1));
                double lambertCoeff = (1.0 - m_subsurface) + m_subsurface * ssCoeff;
                f_Diffuse = m_baseColor / M_PI * (lambertCoeff * (1 - 0.5 * FL) * (1 - 0.5 * FV) + retroCoeff) * ndout;
            }


            //-------------ClearCoat-----------------------------
            if (dot(N, wi) < 0 ||
            dot(N, wo) < 0) {
                // No light below the surface
                f_Clearcoat = make_zero_spectrum();
            }
            else {
                if (dot(frame.n, wi) <= 0) {
                    f_Clearcoat = make_zero_spectrum();
                }
                // Homework 1: implement this!
                Vector3 half_vector = normalize(wi + wo);
                double n_dot_h = dot(frame.n, half_vector);
                double n_dot_in = dot(frame.n, wi);
                double n_dot_out = dot(frame.n, wo);
                double h_dot_out = dot(half_vector, wo);
                if (n_dot_out <= 0 || n_dot_h <= 0) {
                    f_Clearcoat = make_zero_spectrum();
                }

                double alpha_g = (1 - m_clearcoatGloss) * 0.1 + m_clearcoatGloss * 0.001;

                double R_0 = 0.04; //(1.5 - 1)^2 / (1.5 + 1)^2

                double F_c = schlick_fresnel(R_0, h_dot_out);
                double D_c = GTR1(n_dot_h, alpha_g);
                double G_c = smith_masking_gtr2_aniso(to_local(frame, wi), 0.25, 0.25) *
                           smith_masking_gtr2_aniso(to_local(frame, wo), 0.25, 0.25);

                f_Clearcoat = F_c * D_c * G_c / (4 * n_dot_in);
            }


            //--------Sheen---------------------------------
            if (dot(N, wi) < 0 ||
            dot(N, wo) < 0) {
                // No light below the surface
                f_Sheen = make_zero_spectrum();
            }

            if (dot(frame.n, wi) <= 0) {
                f_Sheen = make_zero_spectrum();
            }

            double l = luminance(m_baseColor);
            Spectrum tint = Spectrum(1);
            if (l > 0) tint = m_baseColor / l;

            Spectrum sheen = (1 - m_sheenTint) + m_sheenTint * tint;
            Vector3 half_vector = normalize(wi + wo);

            f_Sheen = sheen * pow(1 - abs(dot(half_vector, wo)), 5) * max(double(0), dot(frame.n, wo));
        }

        Spectrum result = diffuseWeight * f_Diffuse
                        + clearcoatWeight * f_Clearcoat
                        + metalWeight * f_Metal
                        + glassWeight * f_Glass
                        + sheenWeight * f_Sheen;

        return result;
    }
}

#endif //RAYTRACING_MATERIAL_H
