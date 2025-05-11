#include "../microfacet.h"
#include <vector>

void weightComputation(
    Real specular,
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
}

Spectrum eval_op::operator()(const DisneyBSDF &bsdf) const {
    bool reflect = dot(vertex.geometric_normal, dir_in) *
                   dot(vertex.geometric_normal, dir_out) > 0;
    // Flip the shading frame if it is inconsistent with the geometry normal
    Frame frame = vertex.shading_frame;
    // Homework 1: implement this!

}

Real pdf_sample_bsdf_op::operator()(const DisneyBSDF &bsdf) const {
    bool reflect = dot(vertex.geometric_normal, dir_in) *
                   dot(vertex.geometric_normal, dir_out) > 0;
    // Flip the shading frame if it is inconsistent with the geometry normal
    Frame frame = vertex.shading_frame;
    // Homework 1: implement this!

}

std::optional<BSDFSampleRecord>
        sample_bsdf_op::operator()(const DisneyBSDF &bsdf) const {
    // Flip the shading frame if it is inconsistent with the geometry normal
    Frame frame = vertex.shading_frame;
    // Homework 1: implement this!


}

TextureSpectrum get_texture_op::operator()(const DisneyBSDF &bsdf) const {
    return bsdf.base_color;
}
