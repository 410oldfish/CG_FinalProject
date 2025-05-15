#include <complex>

#include "../microfacet.h"

Spectrum eval_op::operator()(const DisneyMetal &bsdf) const {
    if (dot(vertex.geometric_normal, dir_in) < 0 ||
            dot(vertex.geometric_normal, dir_out) < 0) {
        // No light below the surface
        return make_zero_spectrum();
    }
    Frame frame = vertex.shading_frame;
    Real NdotIn = dot(frame.n, dir_in);
    if ( NdotIn <= 0) {
        return make_zero_spectrum();
    }

    Spectrum base_color = eval(bsdf.base_color, vertex.uv, vertex.uv_screen_size, texture_pool);
    Real roughness = eval(bsdf.roughness, vertex.uv, vertex.uv_screen_size, texture_pool);
    roughness = std::clamp(roughness, Real(0.01), Real(1));
    Real aniso = eval(bsdf.anisotropic, vertex.uv, vertex.uv_screen_size, texture_pool);
    Real alpha_x, alpha_y;
    AnisoTransform(roughness, aniso, alpha_x, alpha_y);

    Vector3 H = normalize(dir_in + dir_out);
    Real HdotOut = dot(H, dir_out);

    Spectrum F = schlick_fresnel(base_color, HdotOut);
    Spectrum D = GTR2Aniso(to_local(frame, H), alpha_x, alpha_y);
    Spectrum G = smith_masking_gtr2_aniso(to_local(frame, dir_in), alpha_x, alpha_y) * smith_masking_gtr2_aniso(to_local(frame, dir_out), alpha_x, alpha_y);;

    return F * D * G / (4 * NdotIn);
}

Real pdf_sample_bsdf_op::operator()(const DisneyMetal &bsdf) const {
    if (dot(vertex.geometric_normal, dir_in) < 0 ||
            dot(vertex.geometric_normal, dir_out) < 0) {
        // No light below the surface
        return 0;
    }
    Frame frame = vertex.shading_frame;
    if (dot(frame.n, dir_in) <= 0) {
        return 0;
    }

    Spectrum base_color = eval(bsdf.base_color, vertex.uv, vertex.uv_screen_size, texture_pool);
    Real roughness = eval(bsdf.roughness, vertex.uv, vertex.uv_screen_size, texture_pool);
    roughness = std::clamp(roughness, Real(0.01), Real(1));
    Real aniso = eval(bsdf.anisotropic, vertex.uv, vertex.uv_screen_size, texture_pool);
    Real alpha_x, alpha_y;
    AnisoTransform(roughness, aniso, alpha_x, alpha_y);

    Vector3 H = normalize(dir_in + dir_out);
    Real NdotIn = dot(frame.n, dir_in);
    Real HdotIn = dot(H, dir_in);
    if (NdotIn <= 0) {
        return Real(0);
    }
    Real D = GTR2Aniso(to_local(frame, H), alpha_x, alpha_y);
    Real G_in = smith_masking_gtr2_aniso(to_local(frame, dir_in), alpha_x, alpha_y);
    //这里用了VNDF 考虑了微表面的可见性,G_in是视线可见部分的遮蔽函数，只对可见的微表面采样，方向性更强，噪声更小,效率高
    return D * G_in / (4 * HdotIn);
}

std::optional<BSDFSampleRecord>
        sample_bsdf_op::operator()(const DisneyMetal &bsdf) const {
    if (dot(vertex.geometric_normal, dir_in) < 0) {
        // No light below the surface
        return {};
    }
    Frame frame = vertex.shading_frame;
    if (dot(frame.n, dir_in) <= 0) {
        return {};
    }
    
    // Homework 1: implement this!
    Real roughness = eval(bsdf.roughness, vertex.uv, vertex.uv_screen_size, texture_pool);
    roughness = clamp(roughness, Real(0.01), Real(1));
    Real aniso = eval(bsdf.anisotropic, vertex.uv, vertex.uv_screen_size, texture_pool);
    Real alpha_x, alpha_y;
    AnisoTransform(roughness, aniso, alpha_x, alpha_y);
    //在微表面采样一个可见的随机法线，计算入射光对于这个法线的反射方向
    Vector3 local_micro_normal = sample_visible_normals(to_local(frame, dir_in), alpha_x, alpha_y, rnd_param_uv);

    Vector3 half_vector = to_world(frame, local_micro_normal);
    Vector3 reflected = reflect_vector(dir_in, half_vector);
    return BSDFSampleRecord{
        reflected,
        Real(0) /* eta */, roughness /* roughness */
    };
}

TextureSpectrum get_texture_op::operator()(const DisneyMetal &bsdf) const {
    return bsdf.base_color;
}

Real get_ao_value_op::operator()(const DisneyMetal &bsdf) const{
    return Real(1);
}

Spectrum get_normal_value_op::operator()(const DisneyMetal &bsdf) const{
    return Spectrum(0);
}