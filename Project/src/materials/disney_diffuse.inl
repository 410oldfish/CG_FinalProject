#include "global.h"

Spectrum eval_op::operator()(const DisneyDiffuse &bsdf) const {
    if (dot(vertex.geometric_normal, dir_in) < 0 ||
            dot(vertex.geometric_normal, dir_out) < 0) {
        // No light below the surface
        return make_zero_spectrum();
    }

    Frame frame = vertex.shading_frame;
    Real VdotN = dot(frame.n, dir_in);
    Real LdotN = dot(frame.n, dir_out);

    if (VdotN <= 0 || LdotN <= 0) {
        return make_zero_spectrum();
    }

    //fd = baseDiffuse + subsurface

    Spectrum base_color = eval(bsdf.base_color, vertex.uv, vertex.uv_screen_size, texture_pool);
    Real roughness = eval(bsdf.roughness, vertex.uv, vertex.uv_screen_size, texture_pool);
    Real ss = eval(bsdf.subsurface, vertex.uv, vertex.uv_screen_size, texture_pool);

    Vector3 H = normalize(dir_in + dir_out);
    //Real HdotN = dot(H, frame.n);
    Real HdotL = dot(H, dir_out);
    Real FV = pow5(1 - VdotN);
    Real FL = pow5(1 - LdotN);
    Real FD = roughness * pow2(abs(HdotL));
    // Real RR = 2 * roughness * HdotN * HdotN;

    Real FD90 = 0.5 + 2 * FD;
    Real FD_in = 1 + (FD90 - 1) * FV;
    Real FD_out = 1 + (FD90 - 1)* FL;

    Real LdotN_abs = abs(LdotN);

    auto baseDiffuse = base_color / c_PI * FD_in * FD_out;

    Real FSS90 = FD;
    Real FSS_in = 1 + (FSS90 -1) * FV;
    Real FSS_out = 1 + (FSS90 -1) * FL;
    auto subsurface = 1.25 * base_color / c_PI * ( FSS_in * FSS_out * ( 1/( abs(VdotN) + abs(LdotN) ) - 0.5 ) + 0.5 );

    return ((1 - ss) * baseDiffuse + ss * subsurface) * LdotN_abs;
}

Real pdf_sample_bsdf_op::operator()(const DisneyDiffuse &bsdf) const {
    if (dot(vertex.geometric_normal, dir_in) < 0 ||
            dot(vertex.geometric_normal, dir_out) < 0) {
        // No light below the surface
        return 0;
    }

    Frame frame = vertex.shading_frame;
    if (dot(frame.n, dir_in) < 0) {
        return 0;
    }
    
    // Homework 1: implement this!
    return fmax(dot(frame.n, dir_out), Real(0)) / c_PI;
}

std::optional<BSDFSampleRecord> sample_bsdf_op::operator()(const DisneyDiffuse &bsdf) const {
    if (dot(vertex.geometric_normal, dir_in) < 0) {
        // No light below the surface
        return {};
    }

    Frame frame = vertex.shading_frame;
    if (dot(frame.n, dir_in) < 0) {
        return {};
    }
    
    // Homework 1: implement this!
    Real roughness = eval(bsdf.roughness, vertex.uv, vertex.uv_screen_size, texture_pool);
    auto sample_dir = sample_cos_hemisphere(rnd_param_uv);
    Vector3 dir_out = to_world(frame, sample_dir);
    return BSDFSampleRecord{
        dir_out,
        Real(0) /* eta */, roughness /* roughness */};
}

TextureSpectrum get_texture_op::operator()(const DisneyDiffuse &bsdf) const {
    return bsdf.base_color;
}
