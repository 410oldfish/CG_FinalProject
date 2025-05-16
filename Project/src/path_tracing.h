#pragma once

#include "scene.h"
#include "pcg.h"

/// Unidirectional path tracing
Spectrum path_tracing(const Scene &scene, pcg32_state &rng, Ray& ray, RayDifferential& ray_diff, Real eta_scale = Real(1), Spectrum current_path_throughput = fromRGB(Vector3f(1,1,1)), int depth = 0) {
    int max_depth = scene.options.max_depth;
    if (max_depth != -1 && depth > max_depth) {
        return make_zero_spectrum();
    }

    //Intersect Point
    std::optional<IntersectPoint> intersect_point_ptr = intersect(scene, ray, ray_diff);
    if (!intersect_point_ptr) {
        //Hit envMap or nothing
        if (has_envmap(scene)) {
            const Light &envmap = get_envmap(scene);
            return emission(envmap,
                            -ray.dir, // pointing outwards from light
                            ray_diff.spread,
                            PointAndNormal{}, // dummy parameter for envmap
                            scene);
        }
        return make_zero_spectrum();
    }
    Vector3 dir_view = -ray.dir;
    IntersectPoint intersect_point = *intersect_point_ptr;
    if (dot(intersect_point.geometric_normal, dir_view) * dot(intersect_point.shading_frame.n, dir_view) < 0) {
        intersect_point.shading_frame = Frame(intersect_point.geometric_normal);
    }

    Spectrum radiance = make_zero_spectrum();

    //if hit light, just return emit
    if (depth == 0 && is_light(scene.shapes[intersect_point.shape_id])) {
        radiance += current_path_throughput *
            emission(intersect_point, -ray.dir, scene);
        return radiance;
    }

    const Material &mat = scene.materials[intersect_point.material_id];

    Vector2 light_uv{next_pcg32_real<Real>(rng), next_pcg32_real<Real>(rng)};
    Real light_w = next_pcg32_real<Real>(rng);
    Real shape_w = next_pcg32_real<Real>(rng);
    //Random a light source
    int light_id = sample_light(scene, light_w);
    const Light &light = scene.lights[light_id];
    //Get a random point on light
    PointAndNormal point_on_light =
        sample_point_on_light(light, intersect_point.position, light_uv, shape_w, scene);

    Spectrum radiance_direct = make_zero_spectrum();
    //MIS to weight light contribution in directly and indirectly
    Real MIS_weight_direct = 0;

    //Direct Light radiance
    {
        Real G = 0;
        Vector3 dir_light;

        if (!is_envmap(light)) {
            dir_light = normalize(point_on_light.position - intersect_point.position);
            Ray shadow_ray{intersect_point.position, dir_light,
                           get_shadow_epsilon(scene),
                           (1 - get_shadow_epsilon(scene)) *
                               distance(point_on_light.position, intersect_point.position)};
            //If the shadow ray no hit occlusion
            if (!occluded(scene, shadow_ray)) {
                G = max(dot(-dir_light, point_on_light.normal), Real(0)) /
                    distance_squared(point_on_light.position, intersect_point.position);
            }
        } else {
            dir_light = -point_on_light.normal;
            Ray shadow_ray{intersect_point.position, dir_light,
                           get_shadow_epsilon(scene),
                           infinity<Real>() /* envmaps are infinitely far away */};
            if (!occluded(scene, shadow_ray)) {
                G = 1;
            }
        }

        //pdf_direct_light = sample a light  *  sample a point in the light area
        Real pdf_direct_light = light_pmf(scene, light_id) *
            pdf_point_on_light(light, point_on_light, intersect_point.position, scene);

        if (G > 0 && pdf_direct_light > 0) {

            assert(intersect_point.material_id >= 0);

            Spectrum brdf_direct_light_mat = eval(mat, dir_view, dir_light, intersect_point, scene.texture_pool);

            Spectrum light_emittion = emission(light, -dir_light, Real(0), point_on_light, scene);

            radiance_direct = G * brdf_direct_light_mat * light_emittion;

            Real pdf_mat = pdf_sample_bsdf(
                mat, dir_view, dir_light, intersect_point, scene.texture_pool);

            pdf_mat *= G;

            MIS_weight_direct = (pdf_direct_light*pdf_direct_light) / (pdf_direct_light*pdf_direct_light + pdf_mat*pdf_mat);
            radiance_direct /= pdf_direct_light;
        }
    }
    radiance += current_path_throughput * radiance_direct * MIS_weight_direct;

    //Indirect Light radiance
    {
        Vector2 bsdf_rnd_param_uv{next_pcg32_real<Real>(rng), next_pcg32_real<Real>(rng)};
        Real bsdf_rnd_param_w = next_pcg32_real<Real>(rng);
        std::optional<BSDFSampleRecord> bsdf_sample_ =
            sample_bsdf(mat,
                        dir_view,
                        intersect_point,
                        scene.texture_pool,
                        bsdf_rnd_param_uv,
                        bsdf_rnd_param_w);
        if (!bsdf_sample_) {
            return radiance;
        }

        const BSDFSampleRecord &bsdf_sample = *bsdf_sample_;
        Vector3 dir_bsdf = bsdf_sample.dir_out;
        // Update ray differentials & eta_scale
        if (bsdf_sample.eta == 0) {
            ray_diff.spread = reflect(ray_diff, intersect_point.mean_curvature, bsdf_sample.roughness);
        } else {
            ray_diff.spread = refract(ray_diff, intersect_point.mean_curvature, bsdf_sample.eta, bsdf_sample.roughness);
            eta_scale /= (bsdf_sample.eta * bsdf_sample.eta);
        }

        Ray bsdf_ray{intersect_point.position, dir_bsdf, get_intersection_epsilon(scene), infinity<Real>()};
        std::optional<IntersectPoint> bsdf_intersect_point = intersect(scene, bsdf_ray);

        Real G;
        if (bsdf_intersect_point) {
            G = fabs(dot(dir_bsdf, bsdf_intersect_point->geometric_normal)) /
                distance_squared(bsdf_intersect_point->position, intersect_point.position);
        } else {
            // We hit nothing, set G to 1 to account for the environment map contribution.
            G = 1;
        }

        Spectrum brdf_indirect = eval(mat, dir_view, dir_bsdf, intersect_point, scene.texture_pool);
        Real pdf_indirect = pdf_sample_bsdf(mat, dir_view, dir_bsdf, intersect_point, scene.texture_pool);
        if (pdf_indirect <= 0) {
            // Numerical issue -- we generated some invalid rays.
            return radiance;
        }

        pdf_indirect *= G;
        auto ao = get_ao_value(mat, intersect_point, scene.texture_pool);
        bool touch_light = false;
        //If sample ray intersect with sth
        if (bsdf_intersect_point) {
            //if sample ray intersect with light
            if (is_light(scene.shapes[bsdf_intersect_point->shape_id])) {
                touch_light = true;
                // G & f are already computed.
                Spectrum emittion_indirect_light = emission(*bsdf_intersect_point, -dir_bsdf, scene);
                Spectrum radiance_indirect = G * brdf_indirect * emittion_indirect_light;
                // Next let's compute p1(v2): the probability of the light source sampling
                // directly drawing the point corresponds to bsdf_dir.
                int light_id = get_area_light_id(scene.shapes[bsdf_intersect_point->shape_id]);
                assert(light_id >= 0);
                const Light &light = scene.lights[light_id];
                PointAndNormal light_point{bsdf_intersect_point->position, bsdf_intersect_point->geometric_normal};
                Real pdf_indirect_light = light_pmf(scene, light_id) *
                    pdf_point_on_light(light, light_point, intersect_point.position, scene);
                Real MIS_weight_indirect = (pdf_indirect*pdf_indirect) / (pdf_indirect_light*pdf_indirect_light + pdf_indirect*pdf_indirect);

                radiance_indirect /= pdf_indirect;
                radiance += current_path_throughput * radiance_indirect * MIS_weight_indirect;
            }
        } else if (!bsdf_intersect_point && has_envmap(scene)) {
            touch_light = true;
            // G & f are already computed.
            const Light &light = get_envmap(scene);
            Spectrum L = emission(light,
                                  -dir_bsdf, // pointing outwards from light
                                  ray_diff.spread,
                                  PointAndNormal{}, // dummy parameter for envmap
                                  scene);
            Spectrum C2 = G * brdf_indirect * L;
            // Next let's compute p1(v2): the probability of the light source sampling
            // directly drawing the direction bsdf_dir.
            PointAndNormal light_point{Vector3{0, 0, 0}, -dir_bsdf}; // pointing outwards from light
            Real p1 = light_pmf(scene, scene.envmap_light_id) *
                      pdf_point_on_light(light, light_point, intersect_point.position, scene);
            Real w2 = (pdf_indirect*pdf_indirect) / (p1*p1 + pdf_indirect*pdf_indirect);

            C2 /= pdf_indirect;
            //AO
            //Real ao = eval(mat.clearcoat_gloss, vertex.uv, vertex.uv_screen_size, texture_pool);

            radiance += current_path_throughput * C2 * w2 * ao;
        }

        if (!bsdf_intersect_point) {
            // Hit nothing -- can't continue tracing.
            return radiance;
        }

        Real rr_prob = 1;
        if (depth >= scene.options.rr_depth) {
            rr_prob = min(max((1 / eta_scale) * current_path_throughput), Real(0.95));
            if (next_pcg32_real<Real>(rng) > rr_prob) {
                // Terminate the path
                return  radiance;
            }
        }

        if (!touch_light) {
            current_path_throughput = current_path_throughput * (G * brdf_indirect) * ao / (pdf_indirect * rr_prob);
            radiance += path_tracing(scene, rng, bsdf_ray, ray_diff, eta_scale, current_path_throughput, depth + 1);
        }
    }

    return radiance;
}