#include "../microfacet.h"

inline Real square(Real x) { return x * x; }

Spectrum eval_op::operator()(const DisneyGlass &bsdf) const {
    bool reflect = dot(vertex.geometric_normal, dir_in) *
                   dot(vertex.geometric_normal, dir_out) > 0;
    // Flip the shading frame if it is inconsistent with the geometry normal
    Frame frame = vertex.shading_frame;
    if (dot(frame.n, dir_in) * dot(vertex.geometric_normal, dir_in) < 0) {
        frame = -frame;
    }
    Spectrum base_color = eval(bsdf.base_color, vertex.uv, vertex.uv_screen_size, texture_pool);
    Real roughness = eval(bsdf.roughness, vertex.uv, vertex.uv_screen_size, texture_pool);
    Real aniso = eval(bsdf.anisotropic, vertex.uv, vertex.uv_screen_size, texture_pool);
    Real alpha_x, alpha_y;
    AnisoTransform(roughness, aniso, alpha_x, alpha_y);

    bool inner = dot(vertex.geometric_normal, dir_in) < 0;
    Real eta = inner ? 1 / bsdf.eta : bsdf.eta;

    Real NdotIn = dot(dir_in, frame.n);
    Real NdotOut = dot(dir_out, frame.n);

    Vector3 H = reflect ? normalize(dir_in + dir_out) : normalize(-dir_in - eta * dir_out);
    //Why?
    if(length_squared(H) < 1e-3) {
        H = normalize(cross(cross(dir_out, frame.n), dir_out));
    }

    if (dot(H, frame.n) < 0) {
        H = -H;
    }

    Real HdotIn = dot(dir_in, H);
    Real HdotOut = dot(dir_out, H);

    //这是使用了真实的菲涅尔公式，因为折射率在接近1的时候schlick那个方法误差大
    Real F = fresnel_dielectric(HdotIn, eta);
    Real G = smith_masking_gtr2_aniso(to_local(frame, dir_in), alpha_x, alpha_y) * smith_masking_gtr2_aniso(to_local(frame, dir_out), alpha_x, alpha_y);
    Real D = GTR2Aniso(to_local(frame, H), alpha_x, alpha_y);

    Spectrum ret = make_zero_spectrum();
    if (reflect) {
        ret = base_color * (F * G * D ) / (4 * abs(NdotIn));
    }
    else {
        ret = sqrt(base_color) * (1 - F) * D * G * abs(HdotOut * HdotIn) / ( abs(NdotIn) * pow2(HdotIn + eta * HdotOut) );
    }
    return ret;
}

Real pdf_sample_bsdf_op::operator()(const DisneyGlass &bsdf) const {
    bool reflect = dot(vertex.geometric_normal, dir_in) *
                   dot(vertex.geometric_normal, dir_out) > 0;
    // Flip the shading frame if it is inconsistent with the geometry normal
    Frame frame = vertex.shading_frame;
    if (dot(frame.n, dir_in) * dot(vertex.geometric_normal, dir_in) < 0) {
        frame = -frame;
    }
    // Homework 1: implement this!
    Spectrum base_color = eval(bsdf.base_color, vertex.uv, vertex.uv_screen_size, texture_pool);
    Real roughness = eval(bsdf.roughness, vertex.uv, vertex.uv_screen_size, texture_pool);
    Real aniso = eval(bsdf.anisotropic, vertex.uv, vertex.uv_screen_size, texture_pool);
    Real alpha_x, alpha_y;
    AnisoTransform(roughness, aniso, alpha_x, alpha_y);

    bool inner = dot(vertex.geometric_normal, dir_in) < 0;
    Real eta = inner ? 1 / bsdf.eta : bsdf.eta;

    Real NdotIn = dot(dir_in, frame.n);
    Real NdotOut = dot(dir_out, frame.n);

    Vector3 H = reflect ? normalize(dir_in + dir_out) : normalize(-dir_in - eta * dir_out);
    //Why?
    if(length_squared(H) < 1e-3) {
        H = normalize(cross(cross(dir_out, frame.n), dir_out));
    }

    if (dot(H, frame.n) < 0) {
        H = -H;
    }

    Real HdotIn = dot(dir_in, H);
    Real HdotOut = dot(dir_out, H);

    //这是使用了真实的菲涅尔公式，因为折射率在接近1的时候schlick那个方法误差大
    Real F = fresnel_dielectric(HdotIn, eta);
    Real D = GTR2Aniso(to_local(frame, H), alpha_x, alpha_y);

    Real ret;
    if (reflect) {
        ret = ( F * D ) / (4 * abs(NdotIn));
    }
    else {
        ret = (1 - F) * D * abs(HdotOut * HdotIn) / ( abs(NdotIn) * pow2(HdotIn + eta * HdotOut) );
    }
    return ret;
}

std::optional<BSDFSampleRecord>
        sample_bsdf_op::operator()(const DisneyGlass &bsdf) const {
    // Flip the shading frame if it is inconsistent with the geometry normal
    Frame frame = vertex.shading_frame;
    if (dot(frame.n, dir_in) * dot(vertex.geometric_normal, dir_in) < 0) {
        frame = -frame;
    }
    //glass的采样跟金属一样，在specular上根据微表面模型采样出一个随机法线，然后根据随机数在菲尼尔系数的区间，决定入射光与这条法线的交互是反射还是折射
    Real roughness = eval(bsdf.roughness, vertex.uv, vertex.uv_screen_size, texture_pool);
    Real aniso = eval(bsdf.anisotropic, vertex.uv, vertex.uv_screen_size, texture_pool);
    Real alpha_x, alpha_y;
    AnisoTransform(roughness, aniso, alpha_x, alpha_y);

    bool inner = dot(vertex.geometric_normal, dir_in) < 0;
    Real eta = inner ? 1 / bsdf.eta : bsdf.eta;
    Vector3 local_dir_in = to_local(frame, dir_in);
    Vector3 local_H = sample_visible_normals(local_dir_in, alpha_x, alpha_y, rnd_param_uv);
    Vector3 H = to_world(frame, local_H);
    if (dot(H, frame.n) < 0) {
        H = -H;
    }

    Real HdotIn = dot(H, dir_in);

    //F
    Real F = fresnel_dielectric(HdotIn, eta);
    if (F > rnd_param_uv[0]) {
        Vector3 reflect_dir = reflect_vector(dir_in, H);
        return BSDFSampleRecord{ reflect_dir, Real(0), roughness };
    }
    else {
        Vector3 refract_dir = refract_vector(dir_in, H, eta);
        return BSDFSampleRecord{ refract_dir, eta, roughness };
    }
}

TextureSpectrum get_texture_op::operator()(const DisneyGlass &bsdf) const {
    return bsdf.base_color;
}
