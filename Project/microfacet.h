#pragma once

#include "global.hpp"
#include "spectrum.h"
#include "frame.h"

/// A microfacet model assumes that the surface is composed of infinitely many little mirrors/glasses.
/// The orientation of the mirrors determines the amount of lights reflected.
/// The distribution of the orientation is determined empirically.
/// The distribution that fits the best to the data we have so far (which is not a lot of data)
/// is from Trowbridge and Reitz's 1975 paper "Average irregularity representation of a rough ray reflection",
/// wildly known as "GGX" (seems to stand for "Ground Glass X" https://twitter.com/CasualEffects/status/783018211130441728).
/// 
/// We will use a generalized version of GGX called Generalized Trowbridge and Reitz (GTR),
/// proposed by Brent Burley and folks at Disney (https://www.disneyanimation.com/publications/physically-based-shading-at-disney/)
/// as our normal distribution function. GTR2 is equivalent to GGX.

/// Schlick's Fresnel equation approximation
/// from "An Inexpensive BRDF Model for Physically-based Rendering", Schlick
/// https://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.50.2297&rep=rep1&type=pdf
/// See "Memo on Fresnel equations" from Sebastien Lagarde
/// for a doublely nice introduction.
/// https://seblagarde.wordpress.com/2013/04/29/memo-on-fresnel-equations/
template <typename T>
inline T schlick_fresnel(const T &F0, double cos_theta) {
    return F0 + (T(1) - F0) *
        pow(max(1 - cos_theta, double(0)), double(5));
}

// https://renderwonk.com/publications/wp-generalization-adobe/gen-adobe.pdf
inline Spectrum schlick_generalized_fresnel(
    const Spectrum &base_clr,
    double h_dot_out,
    double alpha,
    double tint_strength,
    const Spectrum &tint
) {
    double cos82 = 0.13917310096; // cos(82°), for θ_max
    Spectrum r90 = Spectrum(1.0);

    Spectrum lazanyi_numerator = (r90 - base_clr + (base_clr - tint) * pow(double(1) - cos82, alpha)) * tint_strength;
    double lazanyi_denominator = cos82 * pow(1 - cos82, 6);
    Spectrum lazanyi_a = lazanyi_numerator / lazanyi_denominator;

    Spectrum f = base_clr + (r90 - base_clr) * pow(1 - h_dot_out, alpha) - lazanyi_a * h_dot_out * pow(1 - h_dot_out, 6);
    f.x = clamp(f.x, double(0), double(1));
    f.y = clamp(f.y, double(0), double(1));
    f.z = clamp(f.z, double(0), double(1));
    return f;
}


/// Fresnel equation of a dielectric interface.
/// https://seblagarde.wordpress.com/2013/04/29/memo-on-fresnel-equations/
/// n_dot_i: abs(cos(incident angle))
/// n_dot_t: abs(cos(transmission angle))
/// eta: eta_transmission / eta_incident
inline double fresnel_dielectric(double n_dot_i, double n_dot_t, double eta) {
    assert(n_dot_i >= 0 && n_dot_t >= 0 && eta > 0);
    double rs = (n_dot_i - eta * n_dot_t) / (n_dot_i + eta * n_dot_t);
    double rp = (eta * n_dot_i - n_dot_t) / (eta * n_dot_i + n_dot_t);
    double F = (rs * rs + rp * rp) / 2;
    return F;
}

/// https://seblagarde.wordpress.com/2013/04/29/memo-on-fresnel-equations/
/// This is a specialized version for the code above, only using the incident angle.
/// The transmission angle is derived from 
/// n_dot_i: cos(incident angle) (can be negative)
/// eta: eta_transmission / eta_incident
inline double fresnel_dielectric(double n_dot_i, double eta) {
    assert(eta > 0);
    double n_dot_t_sq = 1 - (1 - n_dot_i * n_dot_i) / (eta * eta);
    if (n_dot_t_sq < 0) {
        // total internal reflection
        return 1;
    }
    double n_dot_t = sqrt(n_dot_t_sq);
    return fresnel_dielectric(fabs(n_dot_i), n_dot_t, eta);
}

inline void AnisoTransform(const double& roughness, const double& aniso, double& alpha_x, double& alpha_y) {
    double r2 = roughness * roughness;
    double aspect = sqrt(1 - aniso * 0.9);
    alpha_x = max(double(0.0001), r2 / aspect);
    alpha_y = max(double(0.0001), r2 * aspect);
}

inline double GTR2(double n_dot_h, double roughness) {
    double alpha = roughness * roughness;
    double a2 = alpha * alpha;
    double t = 1 + (a2 - 1) * n_dot_h * n_dot_h;
    return a2 / (M_PI * t*t);
}

inline double GTR2Aniso(Vector3f h, double alpha_x, double alpha_y){
    double t = (h.x * h.x) / (alpha_x * alpha_x) + (h.y * h.y) / (alpha_y * alpha_y) + h.z * h.z;
    return 1 / (M_PI * alpha_x * alpha_y * t * t);
}

inline double GTR1(double n_dot_h, double alpha_g) {
    double a2 = alpha_g * alpha_g;
    double t = 1 + (a2 - 1) * n_dot_h * n_dot_h;
    return (a2-1) / (M_PI * log(a2) * t);
}

inline double GGX(double n_dot_h, double roughness) {
    return GTR2(n_dot_h, roughness);
}

/// The masking term models the occlusion between the small mirrors of the microfacet models.
/// See Eric Heitz's paper "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"
/// for a great explanation.
/// https://jcgt.org/published/0003/02/03/paper.pdf
/// The derivation is based on Smith's paper "Geometrical shadowing of a random rough surface".
/// Note that different microfacet distributions have different masking terms.
inline double smith_masking_gtr2(const Vector3 &v_local, double roughness) {
    double alpha = roughness * roughness;
    double a2 = alpha * alpha;
    Vector3 v2 = v_local * v_local;
    double Lambda = (-1 + sqrt(1 + (v2.x * a2 + v2.y * a2) / v2.z)) / 2;
    return 1 / (1 + Lambda);
}

inline double smith_masking_gtr2_aniso(const Vector3 &v_local, double alpha_x, double alpha_y) {
    double x_part = v_local.x * alpha_x;
    double y_part = v_local.y * alpha_y;

    double Lambda = (-1 + sqrt(1 + (x_part * x_part + y_part * y_part) / v_local.z / v_local.z) ) / 2;
    return 1 / (1 + Lambda);
}

/// See "Sampling the GGX Distribution of Visible Normals", Heitz, 2018.
/// https://jcgt.org/published/0007/04/01/
inline Vector3 sample_visible_normals(const Vector3 &local_dir_in, double alpha_x, double alpha_y, const Vector2 &rnd_param) {
    // The incoming direction is in the "ellipsodial configuration" in Heitz's paper
    if (local_dir_in.z < 0) {
        // Ensure the input is on top of the surface.
        return -sample_visible_normals(-local_dir_in, alpha_x, alpha_y, rnd_param);
    }

    // Transform the incoming direction to the "hemisphere configuration".
    Vector3 hemi_dir_in = normalize(
        Vector3{alpha_x * local_dir_in.x, alpha_y * local_dir_in.y, local_dir_in.z});

    // Parameterization of the projected area of a hemisphere.
    // First, sample a disk.
    double r = sqrt(rnd_param.x);
    double phi = 2 * M_PI * rnd_param.y;
    double t1 = r * cos(phi);
    double t2 = r * sin(phi);
    // Vertically scale the position of a sample to account for the projection.
    double s = (1 + hemi_dir_in.z) / 2;
    t2 = (1 - s) * sqrt(1 - t1 * t1) + s * t2;
    // Point in the disk space
    Vector3 disk_N{t1, t2, sqrt(max(double(0), 1 - t1*t1 - t2*t2))};

    // Reprojection onto hemisphere -- we get our sampled normal in hemisphere space.
    Frame hemi_frame(hemi_dir_in);
    Vector3 hemi_N = to_world(hemi_frame, disk_N);

    // Transforming the normal back to the ellipsoid configuration
    return normalize(Vector3{alpha_x * hemi_N.x, alpha_y * hemi_N.y, max(double(0), hemi_N.z)});
}
