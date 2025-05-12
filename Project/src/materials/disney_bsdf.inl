#include "../microfacet.h"
#include <vector>

void CalculateWeights(
    Real specular_transmission,
    Real metallic,
    Real clearcoat,
    Real sheen,
    std::vector<Real>& weights
){
    // diffuse, clearcoat, metal, glass, sheen
    weights.resize(5);
    weights[0] = (1 - metallic) * (1 - specular_transmission);
    weights[1] = 0.25 * clearcoat;
    weights[2] = (1 - specular_transmission * (1 - metallic));
    weights[3] = (1 - metallic) * specular_transmission;
    weights[4] = (1 - metallic) * sheen;

    Real sum = weights[0] + weights[1] + weights[2] + weights[3] + weights[4];
    for (int i = 0; i < 5; i++) {
        weights[i] /= sum;
    }
}

Spectrum eval_op::operator()(const DisneyBSDF &bsdf) const {
    bool reflect = dot(vertex.geometric_normal, dir_in) *
                   dot(vertex.geometric_normal, dir_out) > 0;
    // Flip the shading frame if it is inconsistent with the geometry normal
    Frame frame = vertex.shading_frame;
    //折射和内反射都只考虑glass
    if (!reflect || dot(frame.n, dir_in) <= 0) {
        //只考虑glass
        DisneyGlass glass_data = {bsdf.base_color, bsdf.roughness, bsdf.anisotropic, bsdf.eta};
        return this->operator()(glass_data);
    }
    else {
        //Data
        Spectrum base_color = eval(bsdf.base_color, vertex.uv, vertex.uv_screen_size, texture_pool);
        Real specular = eval(bsdf.specular, vertex.uv, vertex.uv_screen_size, texture_pool);
        Real specular_tint = eval(bsdf.specular_tint, vertex.uv, vertex.uv_screen_size, texture_pool);
        Real specular_transmission = eval(bsdf.specular_transmission, vertex.uv, vertex.uv_screen_size, texture_pool);
        Real metallic = eval(bsdf.metallic, vertex.uv, vertex.uv_screen_size, texture_pool);
        Real cc = eval(bsdf.clearcoat, vertex.uv, vertex.uv_screen_size, texture_pool);
        Real sh = eval(bsdf.sheen, vertex.uv, vertex.uv_screen_size, texture_pool);

        Real l = luminance(base_color);
        auto Ctint = l > 0 ? l/base_color : 1;
        auto Ks = (1 - specular_tint) + specular_tint * Ctint;
        Real eta = dot(vertex.geometric_normal, dir_in) > 0 ? bsdf.eta : 1 / bsdf.eta;
        Real R0 = pow2( (1 - eta) / (1 + eta ));
        auto C0 = specular * R0 * (1 - metallic) * Ks + metallic * base_color;

        //Calculate all bsdf
        //all bsdf needs data
        DisneyDiffuse diffuse_bsdf = {bsdf.base_color, bsdf.roughness, bsdf.subsurface};
        DisneyClearcoat clearcoat_bsdf = {bsdf.clearcoat_gloss};
        DisneyMetal metal_bsdf = {make_constant_spectrum_texture(C0), bsdf.roughness, bsdf.anisotropic};
        DisneyGlass glass_bsdf = {bsdf.base_color, bsdf.roughness, bsdf.anisotropic, bsdf.eta};
        DisneySheen sheen_bsdf = {bsdf.base_color, bsdf.sheen_tint};

        Spectrum diffuse_value = make_zero_spectrum();
        Spectrum clearcoat_value = make_zero_spectrum();
        Spectrum metal_value = make_zero_spectrum();
        Spectrum glass_value = make_zero_spectrum();
        Spectrum sheen_value = make_zero_spectrum();

        if (dot(frame.n, dir_out) > 0) {
            diffuse_value = this->operator()(diffuse_bsdf);
            clearcoat_value = this->operator()(clearcoat_bsdf);
            metal_value = this->operator()(metal_bsdf);
            sheen_value = this->operator()(sheen_bsdf);
        }
        glass_value = this->operator()(glass_bsdf);

        std::vector<Real> weights;
        CalculateWeights(specular_transmission, metallic, cc, sh, weights);
        Spectrum ret = make_zero_spectrum();

        ret += weights[0] * diffuse_value;
        ret += weights[1] * clearcoat_value;
        ret += weights[2] * metal_value;
        ret += weights[3] * glass_value;
        ret += weights[4] * sheen_value;

        return ret;
    }
}

Real pdf_sample_bsdf_op::operator()(const DisneyBSDF &bsdf) const {
    bool reflect = dot(vertex.geometric_normal, dir_in) *
                   dot(vertex.geometric_normal, dir_out) > 0;
    // Flip the shading frame if it is inconsistent with the geometry normal
    Frame frame = vertex.shading_frame;

    //折射和内反射都只考虑glass
    if (!reflect || dot(frame.n, dir_in) <= 0) {
        //只考虑glass
        DisneyGlass glass_data = {bsdf.base_color, bsdf.roughness, bsdf.anisotropic, bsdf.eta};
        return this->operator()(glass_data);
    }
    else {
        //Data
        Real specular_transmission = eval(bsdf.specular_transmission, vertex.uv, vertex.uv_screen_size, texture_pool);
        Real metallic = eval(bsdf.metallic, vertex.uv, vertex.uv_screen_size, texture_pool);
        Real cc = eval(bsdf.clearcoat, vertex.uv, vertex.uv_screen_size, texture_pool);
        Real sh = eval(bsdf.sheen, vertex.uv, vertex.uv_screen_size, texture_pool);

        //Calculate all bsdf
        //all bsdf needs data
        Real diffuse_value, clearcoat_value, metal_value, glass_value, sheen_value;

        if (dot(frame.n, dir_out) > 0) {
            DisneyDiffuse diffuse_bsdf = {bsdf.base_color, bsdf.roughness, bsdf.subsurface};
            DisneyClearcoat clearcoat_bsdf = {bsdf.clearcoat_gloss};
            DisneyMetal metal_bsdf = {bsdf.base_color, bsdf.roughness, bsdf.anisotropic};
            DisneySheen sheen_bsdf = {bsdf.base_color, bsdf.sheen_tint};

            diffuse_value = this->operator()(diffuse_bsdf);
            clearcoat_value = this->operator()(clearcoat_bsdf);
            metal_value = this->operator()(metal_bsdf);
            sheen_value = this->operator()(sheen_bsdf);
        }
        DisneyGlass glass_bsdf = {bsdf.base_color, bsdf.roughness, bsdf.anisotropic, bsdf.eta};
        glass_value = this->operator()(glass_bsdf);

        std::vector<Real> weights;
        CalculateWeights(specular_transmission, metallic, cc, sh, weights);

        Real ret = 0;

        ret += weights[0] * diffuse_value;
        ret += weights[1] * clearcoat_value;
        ret += weights[2] * metal_value;
        ret += weights[3] * glass_value;
        ret += weights[4] * sheen_value;

        return ret;
    }
}

std::optional<BSDFSampleRecord>
        sample_bsdf_op::operator()(const DisneyBSDF &bsdf) const {
    // Flip the shading frame if it is inconsistent with the geometry normal
    Frame frame = vertex.shading_frame;
    // Homework 1: implement this!
    bool fromInnerToOut = dot(frame.n , dir_in) < 0;
    if (fromInnerToOut) {
        //只考虑Glass
        DisneyGlass glass = {bsdf.base_color, bsdf.roughness, bsdf.anisotropic, bsdf.eta};
        return this->operator()(glass);
    }
    else {
        Real specular_transmission = eval(bsdf.specular_transmission, vertex.uv, vertex.uv_screen_size, texture_pool);
        Real metallic = eval(bsdf.metallic, vertex.uv, vertex.uv_screen_size, texture_pool);
        Real cc = eval(bsdf.clearcoat, vertex.uv, vertex.uv_screen_size, texture_pool);

        //Sheen 比重太小，采样这里忽略
        DisneyDiffuse diffuse = {bsdf.base_color, bsdf.roughness, bsdf.subsurface};
        DisneyClearcoat clearcoat = {bsdf.clearcoat_gloss};
        DisneyMetal metal = {bsdf.base_color, bsdf.roughness, bsdf.anisotropic};
        DisneyGlass glass = {bsdf.base_color, bsdf.roughness, bsdf.anisotropic, bsdf.eta};

        std::vector<Real> weights;
        CalculateWeights(specular_transmission, metallic, cc, 0, weights);
        Real currentRange = 0;
        Real randomNumer = rnd_param_w;
        int targetIndex = 0;
        for ( int i = 0; i< 4; i++) {
            currentRange += weights[i];
            if (randomNumer <= currentRange) {
                targetIndex = i;
            }
        }

        switch(targetIndex){
            case 0:
                return (*this)(diffuse);
            case 1:
                return (*this)(clearcoat);
            case 2:
                return (*this)(metal);
            case 3:
                return (*this)(glass);
        }
    }
}

TextureSpectrum get_texture_op::operator()(const DisneyBSDF &bsdf) const {
    return bsdf.base_color;
}
