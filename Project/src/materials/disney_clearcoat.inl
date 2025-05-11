#include "../microfacet.h"

Spectrum eval_op::operator()(const DisneyClearcoat &bsdf) const {
    if (dot(vertex.geometric_normal, dir_in) < 0 ||
            dot(vertex.geometric_normal, dir_out) < 0) {
        // No light below the surface
        return make_zero_spectrum();
    }
    Frame frame = vertex.shading_frame;
    if (dot(frame.n, dir_in) <= 0) {
        return make_zero_spectrum();
    }

    Real clearcoatGloss = eval(bsdf.clearcoat_gloss, vertex.uv, vertex.uv_screen_size, texture_pool);
    Real R0 = 0.04;
    Vector3 H = normalize(dir_in + dir_out);
    Real HdotOut = dot(H, dir_out);
    Real NdotH = dot(frame.n, H);
    Real NdotIn = dot(frame.n, dir_in);
    if (NdotIn <= 0) {
        return make_zero_spectrum();
    }

    Real alpha_g = (1 - clearcoatGloss) * 0.1 + clearcoatGloss * 0.001;

    Real Fc = schlick_fresnel(R0, HdotOut);
    Real Dc = GTR1(NdotH, alpha_g);
    Real Gc = smith_masking_gtr2_aniso(to_local(frame, dir_in), 0.25, 0.25) *
               smith_masking_gtr2_aniso(to_local(frame, dir_out), 0.25, 0.25);

    return Fc * Dc * Gc / (4 * NdotIn);
}

Real pdf_sample_bsdf_op::operator()(const DisneyClearcoat &bsdf) const {
    if (dot(vertex.geometric_normal, dir_in) < 0 ||
            dot(vertex.geometric_normal, dir_out) < 0) {
        // No light below the surface
        return 0;
    }
    // Flip the shading frame if it is inconsistent with the geometry normal
    Frame frame = vertex.shading_frame;
    if (dot(frame.n, dir_in) <= 0) {
        return 0;
    }
    // Homework 1: implement this!
    Vector3 H = normalize(dir_in + dir_out);
    Real NdotOut = dot(frame.n, dir_out);
    Real NdotH = dot(frame.n, H);
    if (NdotOut <= 0 || NdotH <= 0) {
        return 0;
    }

    Real clearcoatGloss = eval(bsdf.clearcoat_gloss, vertex.uv, vertex.uv_screen_size, texture_pool);
    Real alpha_g = (1 - clearcoatGloss) * 0.1 + clearcoatGloss * 0.001;

    Real Dc = GTR1(NdotH, alpha_g);

    return Dc * NdotH / (4 * NdotOut);
}

std::optional<BSDFSampleRecord>
        sample_bsdf_op::operator()(const DisneyClearcoat &bsdf) const {
    if (dot(vertex.geometric_normal, dir_in) < 0) {
        // No light below the surface
        return {};
    }
    // Flip the shading frame if it is inconsistent with the geometry normal
    Frame frame = vertex.shading_frame;
    if (dot(frame.n, dir_in) <= 0) {
        return {};
    }

    Real clearcoatGloss = eval(bsdf.clearcoat_gloss, vertex.uv, vertex.uv_screen_size, texture_pool);
    Real alpha_g = (1 - clearcoatGloss) * 0.1 + clearcoatGloss * 0.001;
    Real a2 = pow2(alpha_g);

    Real H_elevation = acos(sqrt(( 1 - pow(a2, 1 - rnd_param_uv[0]) ) / (1 - a2)));
    Real H_azimuth = 2 * c_PI * rnd_param_uv[1];

    Vector3 H;
    H.x = sin(H_elevation) * cos(H_azimuth);
    H.y = sin(H_elevation) * sin(H_azimuth);
    H.z = cos(H_elevation);

    Vector3 H_world = to_world(frame, H);
    Vector3 reflect_dir = reflect_vector(dir_in, H_world);
    return BSDFSampleRecord{
        reflect_dir,
        Real(0) /* eta */, clearcoatGloss /* roughness */
    };
}

TextureSpectrum get_texture_op::operator()(const DisneyClearcoat &bsdf) const {
    return make_constant_spectrum_texture(make_zero_spectrum());
}
