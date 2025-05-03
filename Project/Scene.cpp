//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"
#include "pcg.h"
#include <complex>


void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}


void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            pos.happened=true;  // area light that has emission exists
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum){
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}


// Implementation of Path Tracing
// Vector3f Scene::castRay(const Ray &ray, int depth) const
// {
//     Vector3f hitColor = Vector3f(0);
//     auto inter = intersect(ray);
//     if (!inter.happened)return backgroundColor;
//
//     Vector3f hitPoint = inter.coords;
//     Vector3f N = inter.normal; // normal
//     Vector2f st = inter.tcoords; // texture coordinates
//     Vector3f dir = ray.direction;
//
//     auto& material = inter.material;
//
//     auto& light_point = this->lights[0];
//     auto light_dir = (light_point.get()->position - hitPoint).normalized();
//
//     if (inter.material->m_type == EMIT) {
// //自发光
//         return inter.material->m_emission;
//     } else if (inter.material->m_type == DIFFUSE || TASK_N<3) {
//         if (TASK_N >= 5) {
//             //if (depth == 0) {
//                 // sample area light
//                 int light_sample=4;
//                 for (int i = 0; i < light_sample; ++i) {
//                     Intersection lightInter;
//                     float pdf_light = 0.0f;
//                     sampleLight(lightInter, pdf_light);  // sample a point on the area light
//                     // TODO: task 5 soft shadow
//                     Vector3f shadow_ray_dir = lightInter.coords - hitPoint;
//                     float shadow_ray_norm = shadow_ray_dir.norm();
//                     shadow_ray_dir = shadow_ray_dir.normalized();
//
//                     //计算来自光源的直接光照贡献
//                     if (shadow_ray_test_area(hitPoint, shadow_ray_dir, N)) {
//                         auto& light_material = lightInter.material;
//                         hitColor+= (inter.obj->evalDiffuseColor(st) * light_material->getEmission() * light_material->eval(shadow_ray_dir, N)
//                         * std::max(0.0f,dotProduct(shadow_ray_dir, N)))/std::pow(shadow_ray_norm, 2) / pdf_light;
//                     }
//                 }
//                 //直接光照贡献
//                 hitColor = hitColor/(float)light_sample;
//             //}
//
//             // //继续trace,计算来自间接光照的部分
//             // if (depth < DIFF_TRACE_MAX) {
//             //     auto sample_dir = material->sample(dir, N);
//             //     Ray sampleRay(hitPoint, sample_dir);
//             //     Vector3f indirect_color = castRay(sampleRay, depth + 1);
//             //     hitColor += material->eval(sample_dir, N) * indirect_color;
//             // }
//         }
//         // TODO: task 1.3 Basic shading
//         //test is shadow
//         if (!shadow_ray_test_point(hitPoint, light_dir, N))return hitColor; //shadow
//
//         auto color_from_material = inter.obj->evalDiffuseColor(st);
//         auto diff_color = material->Kd * color_from_material * std::max(0.0f,dotProduct(N, light_dir));
//         auto view_dir = -dir;
//         auto h = (view_dir + light_dir).normalized();
//         auto spec_color = material->Ks * Vector3f(1.0f) * std::pow(std::max(0.0f, dotProduct(N, h)), material->specularExponent);
//         hitColor += diff_color + spec_color;
//
//     } else if (inter.material->m_type == GLASS && TASK_N>=3) {
//         // TODO: task 3 glass material
//         if (depth <= 3) {
//
//             Vector3f N_correct = N;
//             float ior = material->ior;
//             bool is_out = dotProduct(dir, N) < 0;
//
//             if (!is_out) {
//                 // 光线从物体内射出到物体外
//                 N_correct = -N;  // 翻转法线
//                 ior = 1.0f / ior;  //折射率取倒数
//             }
//
//             float kr = fresnel(dir, N_correct, ior);
//             float bias = 0.01f;
//             Vector3f reflect_ray_dir = reflect(dir, N_correct).normalized();
//             Ray reflect_ray(hitPoint + bias * reflect_ray_dir, reflect_ray_dir);
//
//             if (kr < 1.0f) {
//                 Vector3f refract_ray_dir = refract(dir, N_correct, ior).normalized();
//                 Ray refract_ray(hitPoint + bias * refract_ray_dir, refract_ray_dir);
//
//                 Vector3f reflect_color = castRay(reflect_ray, depth+1);
//                 Vector3f refract_color = castRay(refract_ray, depth+1);
//
//                 hitColor = kr * reflect_color + (1 - kr) * refract_color;
//             }
//             else {
//                 hitColor = castRay(reflect_ray, depth+1);
//             }
//
//         }
//         else {
//             hitColor = Vector3f(0);
//         }
//     }
//
//     return hitColor;
// }

// #region Monte Caro
// Vector3f Scene::castRay(const Ray &ray, int depth) const
// {
//     if (depth > MAX_DEPTH)
//         return Vector3f(0);
//
//     Intersection inter = intersect(ray);
//     if (!inter.happened)
//         return backgroundColor;
//
//     Vector3f hitColor(0);
//     Vector3f hitPoint = inter.coords;
//     Vector3f N = inter.normal.normalized();
//     Vector3f wo = -ray.direction.normalized();  // 观察方向
//     auto &material = inter.material;
//
//     // 如果是自发光材质，直接返回
//     if (material->hasEmission())
//         return material->getEmission();
//
//     // Russian Roulette
//     float rr_prob = 0.8f;
//     if (get_random_float() > rr_prob)
//         return Vector3f(0);
//
//     // =============== 1. 光源采样（Direct Light with MIS） ===============
//     {
//         for (int i =0; i< SAMPLE_LIGHT;i++) {
//             Intersection lightInter;
//             float pdf_light = 0.0f;
//             sampleLight(lightInter, pdf_light);  // 按面积采样一个光源点
//
//             Vector3f lightDir = (lightInter.coords - hitPoint).normalized();
//             float dist = (lightInter.coords - hitPoint).norm();
//             float dist2 = dist * dist;
//             Vector3f shadowOrigin = hitPoint + N * EPSILON;
//             Ray shadowRay(shadowOrigin, lightDir);
//             Intersection shadowHit = intersect(shadowRay);
//
//             // 检查遮挡（shadow ray 命中目标光源）
//             if (shadowHit.happened && shadowHit.material->hasEmission()) {
//                 Vector3f f = material->eval( lightDir, wo, N);
//                 float cos_theta = std::max(0.f, dotProduct(N, lightDir));
//                 float cos_theta_light = std::max(0.f, dotProduct(lightInter.normal, -lightDir));
//                 Vector3f Le = lightInter.material->getEmission();
//
//                 //pdf_light = get_light_pdf(lightInter, hitPoint);
//                 float pdf_light_dist2 = pdf_light * dist2;
//                 float pdf_bsdf = material->pdf(wo, lightDir, N);
//                 float mis_weight = pdf_light_dist2 / (pdf_light_dist2 + pdf_bsdf + EPSILON);
//                 hitColor += f * Le * cos_theta * cos_theta_light /  (pdf_light_dist2 + EPSILON) * mis_weight;
//             }
//         }
//         hitColor = hitColor/(float)SAMPLE_LIGHT;
//         //std::cout << hitColor << std::endl;
//     }
//
//     // =============== 2. BSDF采样（Indirect Light + MIS） ===============
//    {
//         Vector3f wi = material->sample(wo, N);  // 采样方向
//         float pdf_bsdf = material->pdf(wo, wi, N);
//
//         if (pdf_bsdf > EPSILON) {
//             Vector3f f = material->eval(wi, wo, N);  // BRDF
//             //Vector3f albedo = material->m_color;
//             float cos_theta = std::max(0.f, dotProduct(wi, N));
//             Vector3f origin = hitPoint + N * EPSILON;
//             Ray newRay(origin, wi);
//             Intersection newInter = intersect(newRay);
//
//             if (newInter.happened) {
//                 //采样光线射中了光源
//                 if (newInter.material->hasEmission()) {
//                     // 光源路径 - 使用 MIS 权重
//                     Vector3f Le = newInter.material->getEmission();
//                     float pdf_light = get_light_pdf(newInter, hitPoint);
//                     float mis_weight = pdf_bsdf / (pdf_bsdf + pdf_light + EPSILON);
//                     hitColor += f * Le * cos_theta / (pdf_bsdf * rr_prob + EPSILON) * mis_weight;
//                 } else {
//                     // 间接递归路径
//                     Vector3f Li = castRay(newRay, depth + 1);
//                     hitColor += f * Li * cos_theta / (pdf_bsdf * rr_prob + EPSILON);
//                 }
//             }
//         }
//    }
//
//     return hitColor;
// }
// #endregion

//#region Disney BSDF
// Vector3f Scene::castRay(const Ray &ray, int depth, pcg32_state &rng) const {
//     if (depth > MAX_DEPTH) return Vector3f(0);
//
//     Intersection inter = intersect(ray);
//     if (!inter.happened) return backgroundColor;
//
//     auto &material = inter.material;
//     Vector3f hitPoint = inter.coords;
//     Vector3f N = normalize(inter.normal);
//     Vector3f dir_view = -normalize(ray.direction);
//     // if (dot(N, wo) < 0.0f) {
//     //     N = -N;
//     // }
//     // 发光物体直接返回自发光
//     if (material->hasEmission()) {
//         return material->m_emission;
//     }
//
//     // Russian Roulette
//     float rr_prob = 0.6f;
//     if (get_random_float() > rr_prob) return Vector3f(0);
//
//     Vector3f L_direct(0), L_indirect(0);
//
//     // ================= Direct Light via MIS =================
//     for (int i = 0; i < SAMPLE_LIGHT; ++i) {
//         Intersection lightInter;
//         float pdf_light = 0.0f;
//         sampleLight(lightInter, pdf_light);  // 按面积采样光源
//
//         Vector3f x = hitPoint;
//         Vector3f x_l = lightInter.coords;
//         Vector3f n_l = lightInter.normal;
//         Vector3f dir_light = normalize(x_l - x);
//         float dist = distance(x_l , x);
//         float dist2 = dist * dist;
//
//         // shadow ray
//         Ray shadowRay(hitPoint + N * EPSILON, dir_light);
//         Intersection shadowHit = intersect(shadowRay);
//
//         if (shadowHit.happened && shadowHit.material->hasEmission()) {
//             Vector3f f = material->eval(dir_view, dir_light, N);
//             Vector3f Le = lightInter.material->m_emission;
//             float cos_theta = std::max(0.0f, dot(N, dir_light));
//             float cos_theta_light = std::max(0.0f, dot(n_l, -dir_light));
//
//             float pdf_bsdf = material->pdf(dir_view, dir_light, N);
//             float pdf_light_dist2 = pdf_light * dist2;
//             float pdf_light_dist2_pow2 = pdf_light_dist2 * pdf_light_dist2;
//             float pdf_bsdf_pow2 = pdf_bsdf * pdf_bsdf;
//             float mis_weight = pdf_light_dist2_pow2 / (pdf_light_dist2_pow2 + pdf_bsdf_pow2 + EPSILON);
//
//             L_direct += f * Le * cos_theta * cos_theta_light / (pdf_light_dist2 + EPSILON) * mis_weight;
//
//         }
//     }
//     L_direct = L_direct/(SAMPLE_LIGHT * rr_prob);
//
//     // ================= Indirect Light =================
//     Vector2 bsdf_rnd_param_uv{next_pcg32_real<double>(rng), next_pcg32_real<double>(rng)};
//     double bsdf_rnd_param_w = next_pcg32_real<double>(rng);
//     Vector3f dir_sample = material->sample(dir_view, N, bsdf_rnd_param_uv, bsdf_rnd_param_w, rng);  // 采样方向
//
//     float pdf_bsdf = material->pdf(dir_view, dir_sample, N);
//     float pdf_bsdf_pow2 = pdf_bsdf * pdf_bsdf;
//     if (pdf_bsdf > EPSILON) {
//         Vector3f f = material->eval(dir_view, dir_sample, N);
//         float cos_theta = std::max(0.0f, dot(N, dir_sample));
//         bool inner = dot(N, dir_sample) < 0;
//         auto cur_N = inner ? -N :N;
//         Ray newRay(hitPoint + cur_N * EPSILON, dir_sample);
//         Intersection newInter = intersect(newRay);
//
//         if (newInter.happened) {
//             if (newInter.material->hasEmission()) {
//                 // Light hit via BSDF path
//                 float pdf_light = get_light_pdf(newInter, hitPoint);
//                 float pdf_light_pow2 = pdf_light * pdf_light;
//                 float mis_weight = pdf_bsdf_pow2 / (pdf_bsdf_pow2 + pdf_light_pow2 + EPSILON);
//
//                 L_indirect += f * newInter.material->m_emission *
//                               cos_theta / (pdf_bsdf * rr_prob + EPSILON) * mis_weight;
//             } else {
//                 // Regular recursive bounce
//                 Vector3f Li = castRay(newRay, depth + 1, rng);
//                 L_indirect += f * Li * cos_theta / (pdf_bsdf * rr_prob + EPSILON);
//             }
//         }
//     }
//
//     return L_direct + L_indirect;
// }

// #region Disney BSDF github
// Vector3f Scene::castRay(const Ray &ray, int depth, pcg32_state &rng) const {
//     if (depth > MAX_DEPTH) return Vector3f(0);
//
//     Intersection inter = intersect(ray);
//     if (!inter.happened) return backgroundColor;
//
//     auto &material = inter.material;
//     Vector3f hitPoint = inter.coords;
//     Vector3f N = normalize(inter.normal);
//     Vector3f wo = -normalize(ray.direction);
//     bool inner = dot(wo, N) < 0;
//
//     Spectrum radiance = make_zero_spectrum();
//     // A path's contribution is
//     // C(v) = W(v0, v1) * G(v0, v1) * f(v0, v1, v2) *
//     //                    G(v1, v2) * f(v1, v2, v3) *
//     //                  ........
//     //                  * G(v_{n-1}, v_n) * L(v_{n-1}, v_n)
//     // where v is the path vertices, W is the sensor response
//     // G is the geometry term, f is the BSDF, L is the emission
//     //
//     // "sample_primary" importance samples both W and G,
//     // and we assume it always has weight 1.
//
//     // current_path_throughput stores the ratio between
//     // 1) the path contribution from v0 up to v_{i} (the BSDF f(v_{i-1}, v_i, v_{i+1}) is not included),
//     // where i is where the PathVertex "vertex" lies on, and
//     // 2) the probability density for computing the path v from v0 up to v_i,
//     // so that we can compute the Monte Carlo estimates C/p.
//     Spectrum current_path_throughput = fromRGB(Vector3{1, 1, 1});
//     // eta_scale stores the scale introduced by Snell-Descartes law to the BSDF (eta^2).
//     // We use the same Russian roulette strategy as Mitsuba/pbrt-v3
//     // and tracking eta_scale and removing it from the
//     // path contribution is crucial for many bounces of refraction.
//     double eta_scale = double(1);
//
//     // We hit a light immediately.
//     // This path has only two vertices and has contribution
//     // C = W(v0, v1) * G(v0, v1) * L(v0, v1)
//     if (inter.material->hasEmission()) {
//         radiance += current_path_throughput * fromRGB(inter.material->m_emission);
//     }
//
//         // First, we sample a point on the light source.
//         // We do this by first picking a light source, then pick a point on it.
//         Vector2 light_uv{next_pcg32_real<double>(rng), next_pcg32_real<double>(rng)};
//         double light_w = next_pcg32_real<double>(rng);
//         double shape_w = next_pcg32_real<double>(rng);
//         Intersection lightInter;
//         float pdf_light = 0.0f;
//         sampleLight(lightInter, pdf_light);  // 按面积采样光源
//
//         // Next, we compute w1*C1/p1. We store C1/p1 in C1.
//         Spectrum C1 = make_zero_spectrum();
//         double w1 = 0;
//         // Remember "current_path_throughput" already stores all the path contribution on and before v_i.
//         // So we only need to compute G(v_{i}, v_{i+1}) * f(v_{i-1}, v_{i}, v_{i+1}) * L(v_{i}, v_{i+1})
//         {
//             // Let's first deal with C1 = G * f * L.
//             // Let's first compute G.
//             double G = 0;
//             Vector3 dir_light;
//             dir_light = normalize(lightInter.coords - hitPoint);
//             // If the point on light is occluded, G is 0. So we need to test for occlusion.
//             // To avoid self intersection, we need to set the tnear of the ray
//             // to a small "epsilon". We set the epsilon to be a small constant times the
//             // scale of the scene, which we can obtain through the get_shadow_epsilon() function.
//             Ray shadow_ray(hitPoint + N * EPSILON, dir_light);
//
//             Intersection shadowHit = intersect(shadow_ray);
//
//             if (shadowHit.happened && shadowHit.material->hasEmission()) {
//                 // geometry term is cosine at v_{i+1} divided by distance squared
//                 // this can be derived by the infinitesimal area of a surface projected on
//                 // a unit sphere -- it's the Jacobian between the area measure and the solid angle
//                 // measure.
//                 G = max(-dot(dir_light, (Vector3)shadowHit.normal), double(0)) /
//                     distance_squared(shadowHit.coords, hitPoint);
//             }
//
//             // Before we proceed, we first compute the probability density p1(v1)
//             // The probability density for light sampling to sample our point is
//             // just the probability of sampling a light times the probability of sampling a point
//             //light_pmf(scene, light_id)
//             double p1 = get_light_pdf(lightInter, hitPoint);
//
//             // We don't need to continue the computation if G is 0.
//             // Also sometimes there can be some numerical issue such that we generate
//             // a light path with probability zero
//             if (G > 0 && p1 > 0) {
//                 // Let's compute f (BSDF) next.
//                 Vector3 dir_view = -ray.direction;
//                 //Spectrum f = eval(mat, dir_view, dir_light, vertex, scene.texture_pool);
//                 Spectrum f = material->eval(dir_view, dir_light, N);
//                 // Evaluate the emission
//                 // We set the footprint to zero since it is not fully clear how
//                 // to set it in this case.
//                 // One way is to use a roughness based heuristics, but we have multi-layered BRDFs.
//                 // See "double-time Shading with Filtered Importance Sampling" from Colbert et al.
//                 // for the roughness based heuristics.
//                 //Spectrum L = emission(light, -dir_light, double(0), point_on_light, scene);
//                 Spectrum L = lightInter.material->getEmission();
//                 // C1 is just a product of all of them!
//                 C1 = G * f * L;
//
//                 // Next let's compute w1
//
//                 // Remember that we want to set
//                 // w1 = p_1(v^1)^2 / (p_1(v^1)^2 + p_2(v^1)^2)
//                 // Notice that all of the probability density share the same path prefix and those cancel out.
//                 // Therefore we only need to account for the generation of the vertex v_{i+1}.
//
//                 // The probability density for our hemispherical sampling to sample
//                 double p2 = material->pdf(dir_view,dir_light, N);
//                 // !!!! IMPORTANT !!!!
//                 // In general, p1 and p2 now live in different spaces!!
//                 // our BSDF API outputs a probability density in the solid angle measure
//                 // while our light probability density is in the area measure.
//                 // We need to make sure that they are in the same space.
//                 // This can be done by accounting for the Jacobian of the transformation
//                 // between the two measures.
//                 // In general, I recommend to transform everything to area measure
//                 // (except for directional lights) since it fits to the path-space math better.
//                 // Converting a solid angle measure to an area measure is just a
//                 // multiplication of the geometry term G (let solid angle be dS, area be dA,
//                 // we have dA/dS = G).
//                 p2 *= G;
//
//                 w1 = (p1*p1) / (p1*p1 + p2*p2);
//                 C1 /= p1;
//             }
//         }
//         radiance += current_path_throughput * C1 * w1;
//
//         // Let's do the hemispherical sampling next.
//         Vector3 dir_view = -ray.direction;
//         Vector2 bsdf_rnd_param_uv{next_pcg32_real<double>(rng), next_pcg32_real<double>(rng)};
//         double bsdf_rnd_param_w = next_pcg32_real<double>(rng);
//
//         Vector3 dir_bsdf = material->sample(dir_view, N, bsdf_rnd_param_uv, bsdf_rnd_param_w, rng);
//         // Update ray differentials & eta_scale
//         if (bsdf_sample.eta == 0) {
//             ray_diff.spread = reflect(ray_diff, vertex.mean_curvature, bsdf_sample.roughness);
//         } else {
//             ray_diff.spread = refract(ray_diff, vertex.mean_curvature, bsdf_sample.eta, bsdf_sample.roughness);
//             eta_scale /= (bsdf_sample.eta * bsdf_sample.eta);
//         }
//
//         // Trace a ray towards bsdf_dir. Note that again we have
//         // to have an "epsilon" tnear to prevent self intersection.
//         Ray bsdf_ray{vertex.position, dir_bsdf, get_intersection_epsilon(scene), infinity<double>()};
//         std::optional<PathVertex> bsdf_vertex = intersect(scene, bsdf_ray);
//
//         // To update current_path_throughput
//         // we need to multiply G(v_{i}, v_{i+1}) * f(v_{i-1}, v_{i}, v_{i+1}) to it
//         // and divide it with the pdf for getting v_{i+1} using hemisphere sampling.
//         double G;
//         if (bsdf_vertex) {
//             G = fabs(dot(dir_bsdf, bsdf_vertex->geometric_normal)) /
//                 distance_squared(bsdf_vertex->position, vertex.position);
//         } else {
//             // We hit nothing, set G to 1 to account for the environment map contribution.
//             G = 1;
//         }
//
//         Spectrum f = eval(mat, dir_view, dir_bsdf, vertex, scene.texture_pool);
//         double p2 = pdf_sample_bsdf(mat, dir_view, dir_bsdf, vertex, scene.texture_pool);
//         if (p2 <= 0) {
//             // Numerical issue -- we generated some invalid rays.
//             break;
//         }
//
//         // Remember to convert p2 to area measure!
//         p2 *= G;
//         // note that G cancels out in the division f/p, but we still need
//         // G later for the calculation of w2.
//
//         // Now we want to check whether dir_bsdf hit a light source, and
//         // account for the light contribution (C2 & w2 & p2).
//         // There are two possibilities: either we hit an emissive surface,
//         // or we hit an environment map.
//         // We will handle them separately.
//         if (bsdf_vertex && is_light(scene.shapes[bsdf_vertex->shape_id])) {
//             // G & f are already computed.
//             Spectrum L = emission(*bsdf_vertex, -dir_bsdf, scene);
//             Spectrum C2 = G * f * L;
//             // Next let's compute p1(v2): the probability of the light source sampling
//             // directly drawing the point corresponds to bsdf_dir.
//             int light_id = get_area_light_id(scene.shapes[bsdf_vertex->shape_id]);
//             assert(light_id >= 0);
//             const Light &light = scene.lights[light_id];
//             PointAndNormal light_point{bsdf_vertex->position, bsdf_vertex->geometric_normal};
//             double p1 = light_pmf(scene, light_id) *
//                 pdf_point_on_light(light, light_point, vertex.position, scene);
//             double w2 = (p2*p2) / (p1*p1 + p2*p2);
//
//             C2 /= p2;
//             radiance += current_path_throughput * C2 * w2;
//         } else if (!bsdf_vertex && has_envmap(scene)) {
//             // G & f are already computed.
//             const Light &light = get_envmap(scene);
//             Spectrum L = emission(light,
//                                   -dir_bsdf, // pointing outwards from light
//                                   ray_diff.spread,
//                                   PointAndNormal{}, // dummy parameter for envmap
//                                   scene);
//             Spectrum C2 = G * f * L;
//             // Next let's compute p1(v2): the probability of the light source sampling
//             // directly drawing the direction bsdf_dir.
//             PointAndNormal light_point{Vector3{0, 0, 0}, -dir_bsdf}; // pointing outwards from light
//             double p1 = light_pmf(scene, scene.envmap_light_id) *
//                       pdf_point_on_light(light, light_point, vertex.position, scene);
//             double w2 = (p2*p2) / (p1*p1 + p2*p2);
//
//             C2 /= p2;
//             radiance += current_path_throughput * C2 * w2;
//         }
//
//         if (!bsdf_vertex) {
//             // Hit nothing -- can't continue tracing.
//             break;
//         }
//
//         // Update rays/intersection/current_path_throughput/current_pdf
//         // Russian roulette heuristics
//         double rr_prob = 1;
//         if (num_vertices - 1 >= scene.options.rr_depth) {
//             rr_prob = min(max((1 / eta_scale) * current_path_throughput), double(0.95));
//             if (next_pcg32_double<double>(rng) > rr_prob) {
//                 // Terminate the path
//                 break;
//             }
//         }
//
//         ray = bsdf_ray;
//         vertex = *bsdf_vertex;
//         current_path_throughput = current_path_throughput * (G * f) / (p2 * rr_prob);
// }

Spectrum path_tracing(const Ray &ray, int depth, pcg32_state &rng) {
    std::optional<PathVertex> vertex_ = intersect(scene, ray, ray_diff);
    if (!vertex_) {
        // Hit background. Account for the environment map if needed.
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
    PathVertex vertex = *vertex_;

    Spectrum radiance = make_zero_spectrum();
    // A path's contribution is
    // C(v) = W(v0, v1) * G(v0, v1) * f(v0, v1, v2) *
    //                    G(v1, v2) * f(v1, v2, v3) *
    //                  ........
    //                  * G(v_{n-1}, v_n) * L(v_{n-1}, v_n)
    // where v is the path vertices, W is the sensor response
    // G is the geometry term, f is the BSDF, L is the emission
    //
    // "sample_primary" importance samples both W and G,
    // and we assume it always has weight 1.

    // current_path_throughput stores the ratio between
    // 1) the path contribution from v0 up to v_{i} (the BSDF f(v_{i-1}, v_i, v_{i+1}) is not included),
    // where i is where the PathVertex "vertex" lies on, and
    // 2) the probability density for computing the path v from v0 up to v_i,
    // so that we can compute the Monte Carlo estimates C/p.
    Spectrum current_path_throughput = fromRGB(Vector3{1, 1, 1});
    // eta_scale stores the scale introduced by Snell-Descartes law to the BSDF (eta^2).
    // We use the same Russian roulette strategy as Mitsuba/pbrt-v3
    // and tracking eta_scale and removing it from the
    // path contribution is crucial for many bounces of refraction.
    Real eta_scale = Real(1);

    // We hit a light immediately.
    // This path has only two vertices and has contribution
    // C = W(v0, v1) * G(v0, v1) * L(v0, v1)
    if (is_light(scene.shapes[vertex.shape_id])) {
        radiance += current_path_throughput *
            emission(vertex, -ray.dir, scene);
    }

    // We iteratively sum up path contributions from paths with different number of vertices
    // If max_depth == -1, we rely on Russian roulette for path termination.
    int max_depth = scene.options.max_depth;
    for (int num_vertices = 3; max_depth == -1 || num_vertices <= max_depth + 1; num_vertices++) {
        // We are at v_i, and all the path contribution on and before has been accounted for.
        // Now we need to somehow generate v_{i+1} to account for paths with more vertices.
        // In path tracing, we generate two vertices:
        // 1) we sample a point on the light source (often called "Next Event Estimation")
        // 2) we randomly trace a ray from the surface point at v_i and hope we hit something.
        //
        // The first importance samples L(v_i, v_{i+1}), and the second
        // importance samples f(v_{i-1}, v_i, v_{i+1}) * G(v_i, v_{i+1})
        //
        // We then combine the two sampling strategies to estimate the contribution using weighted average.
        // Say the contribution of the first sampling is C1 (with probability density p1),
        // and the contribution of the second sampling is C2 (with probability density p2,
        // then we compute the estimate as w1*C1/p1 + w2*C2/p2.
        //
        // Assuming the vertices for C1 is v^1, and v^2 for C2,
        // Eric Veach showed that it is a good idea setting
        // w1 = p_1(v^1)^k / (p_1(v^1)^k + p_2(v^1)^k)
        // w2 = p_2(v^2)^k / (p_1(v^2)^k + p_2(v^2)^k),
        // where k is some scalar real number, and p_a(v^b) is the probability density of generating
        // vertices v^b using sampling method "a".
        // We will set k=2 as suggested by Eric Veach.

        // Finally, we set our "next vertex" in the loop to the v_{i+1} generated
        // by the second sampling, and update current_path_throughput using
        // our hemisphere sampling.

        // Let's implement this!
        const Material &mat = scene.materials[vertex.material_id];

        // First, we sample a point on the light source.
        // We do this by first picking a light source, then pick a point on it.
        Vector2 light_uv{next_pcg32_real<Real>(rng), next_pcg32_real<Real>(rng)};
        Real light_w = next_pcg32_real<Real>(rng);
        Real shape_w = next_pcg32_real<Real>(rng);
        int light_id = sample_light(scene, light_w);
        const Light &light = scene.lights[light_id];
        PointAndNormal point_on_light =
            sample_point_on_light(light, vertex.position, light_uv, shape_w, scene);

        // Next, we compute w1*C1/p1. We store C1/p1 in C1.
        Spectrum C1 = make_zero_spectrum();
        Real w1 = 0;
        // Remember "current_path_throughput" already stores all the path contribution on and before v_i.
        // So we only need to compute G(v_{i}, v_{i+1}) * f(v_{i-1}, v_{i}, v_{i+1}) * L(v_{i}, v_{i+1})
        {
            // Let's first deal with C1 = G * f * L.
            // Let's first compute G.
            Real G = 0;
            Vector3 dir_light;
            // The geometry term is different between directional light sources and
            // others. Currently we only have environment maps as directional light sources.
            if (!is_envmap(light)) {
                dir_light = normalize(point_on_light.position - vertex.position);
                // If the point on light is occluded, G is 0. So we need to test for occlusion.
                // To avoid self intersection, we need to set the tnear of the ray
                // to a small "epsilon". We set the epsilon to be a small constant times the
                // scale of the scene, which we can obtain through the get_shadow_epsilon() function.
                Ray shadow_ray{vertex.position, dir_light,
                               get_shadow_epsilon(scene),
                               (1 - get_shadow_epsilon(scene)) *
                                   distance(point_on_light.position, vertex.position)};
                if (!occluded(scene, shadow_ray)) {
                    // geometry term is cosine at v_{i+1} divided by distance squared
                    // this can be derived by the infinitesimal area of a surface projected on
                    // a unit sphere -- it's the Jacobian between the area measure and the solid angle
                    // measure.
                    G = max(-dot(dir_light, point_on_light.normal), Real(0)) /
                        distance_squared(point_on_light.position, vertex.position);
                }
            } else {
                // The direction from envmap towards the point is stored in
                // point_on_light.normal.
                dir_light = -point_on_light.normal;
                // If the point on light is occluded, G is 0. So we need to test for occlusion.
                // To avoid self intersection, we need to set the tnear of the ray
                // to a small "epsilon" which we define as c_shadow_epsilon as a global constant.
                Ray shadow_ray{vertex.position, dir_light,
                               get_shadow_epsilon(scene),
                               infinity<Real>() /* envmaps are infinitely far away */};
                if (!occluded(scene, shadow_ray)) {
                    // We integrate envmaps using the solid angle measure,
                    // so the geometry term is 1.
                    G = 1;
                }
            }

            // Before we proceed, we first compute the probability density p1(v1)
            // The probability density for light sampling to sample our point is
            // just the probability of sampling a light times the probability of sampling a point
            Real p1 = light_pmf(scene, light_id) *
                pdf_point_on_light(light, point_on_light, vertex.position, scene);

            // We don't need to continue the computation if G is 0.
            // Also sometimes there can be some numerical issue such that we generate
            // a light path with probability zero
            if (G > 0 && p1 > 0) {
                // Let's compute f (BSDF) next.
                Vector3 dir_view = -ray.dir;
                assert(vertex.material_id >= 0);
                Spectrum f = eval(mat, dir_view, dir_light, vertex, scene.texture_pool);

                // Evaluate the emission
                // We set the footprint to zero since it is not fully clear how
                // to set it in this case.
                // One way is to use a roughness based heuristics, but we have multi-layered BRDFs.
                // See "Real-time Shading with Filtered Importance Sampling" from Colbert et al.
                // for the roughness based heuristics.
                Spectrum L = emission(light, -dir_light, Real(0), point_on_light, scene);

                // C1 is just a product of all of them!
                C1 = G * f * L;

                // Next let's compute w1

                // Remember that we want to set
                // w1 = p_1(v^1)^2 / (p_1(v^1)^2 + p_2(v^1)^2)
                // Notice that all of the probability density share the same path prefix and those cancel out.
                // Therefore we only need to account for the generation of the vertex v_{i+1}.

                // The probability density for our hemispherical sampling to sample
                Real p2 = pdf_sample_bsdf(
                    mat, dir_view, dir_light, vertex, scene.texture_pool);
                // !!!! IMPORTANT !!!!
                // In general, p1 and p2 now live in different spaces!!
                // our BSDF API outputs a probability density in the solid angle measure
                // while our light probability density is in the area measure.
                // We need to make sure that they are in the same space.
                // This can be done by accounting for the Jacobian of the transformation
                // between the two measures.
                // In general, I recommend to transform everything to area measure
                // (except for directional lights) since it fits to the path-space math better.
                // Converting a solid angle measure to an area measure is just a
                // multiplication of the geometry term G (let solid angle be dS, area be dA,
                // we have dA/dS = G).
                p2 *= G;

                w1 = (p1*p1) / (p1*p1 + p2*p2);
                C1 /= p1;
            }
        }
        radiance += current_path_throughput * C1 * w1;

        // Let's do the hemispherical sampling next.
        Vector3 dir_view = -ray.dir;
        Vector2 bsdf_rnd_param_uv{next_pcg32_real<Real>(rng), next_pcg32_real<Real>(rng)};
        Real bsdf_rnd_param_w = next_pcg32_real<Real>(rng);
        std::optional<BSDFSampleRecord> bsdf_sample_ =
            sample_bsdf(mat,
                        dir_view,
                        vertex,
                        scene.texture_pool,
                        bsdf_rnd_param_uv,
                        bsdf_rnd_param_w);
        if (!bsdf_sample_) {
            // BSDF sampling failed. Abort the loop.
            break;
        }
        const BSDFSampleRecord &bsdf_sample = *bsdf_sample_;
        Vector3 dir_bsdf = bsdf_sample.dir_out;
        // Update ray differentials & eta_scale
        if (bsdf_sample.eta == 0) {
            ray_diff.spread = reflect(ray_diff, vertex.mean_curvature, bsdf_sample.roughness);
        } else {
            ray_diff.spread = refract(ray_diff, vertex.mean_curvature, bsdf_sample.eta, bsdf_sample.roughness);
            eta_scale /= (bsdf_sample.eta * bsdf_sample.eta);
        }

        // Trace a ray towards bsdf_dir. Note that again we have
        // to have an "epsilon" tnear to prevent self intersection.
        Ray bsdf_ray{vertex.position, dir_bsdf, get_intersection_epsilon(scene), infinity<Real>()};
        std::optional<PathVertex> bsdf_vertex = intersect(scene, bsdf_ray);

        // To update current_path_throughput
        // we need to multiply G(v_{i}, v_{i+1}) * f(v_{i-1}, v_{i}, v_{i+1}) to it
        // and divide it with the pdf for getting v_{i+1} using hemisphere sampling.
        Real G;
        if (bsdf_vertex) {
            G = fabs(dot(dir_bsdf, bsdf_vertex->geometric_normal)) /
                distance_squared(bsdf_vertex->position, vertex.position);
        } else {
            // We hit nothing, set G to 1 to account for the environment map contribution.
            G = 1;
        }

        Spectrum f = eval(mat, dir_view, dir_bsdf, vertex, scene.texture_pool);
        Real p2 = pdf_sample_bsdf(mat, dir_view, dir_bsdf, vertex, scene.texture_pool);
        if (p2 <= 0) {
            // Numerical issue -- we generated some invalid rays.
            break;
        }

        // Remember to convert p2 to area measure!
        p2 *= G;
        // note that G cancels out in the division f/p, but we still need
        // G later for the calculation of w2.

        // Now we want to check whether dir_bsdf hit a light source, and
        // account for the light contribution (C2 & w2 & p2).
        // There are two possibilities: either we hit an emissive surface,
        // or we hit an environment map.
        // We will handle them separately.
        if (bsdf_vertex && is_light(scene.shapes[bsdf_vertex->shape_id])) {
            // G & f are already computed.
            Spectrum L = emission(*bsdf_vertex, -dir_bsdf, scene);
            Spectrum C2 = G * f * L;
            // Next let's compute p1(v2): the probability of the light source sampling
            // directly drawing the point corresponds to bsdf_dir.
            int light_id = get_area_light_id(scene.shapes[bsdf_vertex->shape_id]);
            assert(light_id >= 0);
            const Light &light = scene.lights[light_id];
            PointAndNormal light_point{bsdf_vertex->position, bsdf_vertex->geometric_normal};
            Real p1 = light_pmf(scene, light_id) *
                pdf_point_on_light(light, light_point, vertex.position, scene);
            Real w2 = (p2*p2) / (p1*p1 + p2*p2);

            C2 /= p2;
            radiance += current_path_throughput * C2 * w2;
        } else if (!bsdf_vertex && has_envmap(scene)) {
            // G & f are already computed.
            const Light &light = get_envmap(scene);
            Spectrum L = emission(light,
                                  -dir_bsdf, // pointing outwards from light
                                  ray_diff.spread,
                                  PointAndNormal{}, // dummy parameter for envmap
                                  scene);
            Spectrum C2 = G * f * L;
            // Next let's compute p1(v2): the probability of the light source sampling
            // directly drawing the direction bsdf_dir.
            PointAndNormal light_point{Vector3{0, 0, 0}, -dir_bsdf}; // pointing outwards from light
            Real p1 = light_pmf(scene, scene.envmap_light_id) *
                      pdf_point_on_light(light, light_point, vertex.position, scene);
            Real w2 = (p2*p2) / (p1*p1 + p2*p2);

            C2 /= p2;
            radiance += current_path_throughput * C2 * w2;
        }

        if (!bsdf_vertex) {
            // Hit nothing -- can't continue tracing.
            break;
        }

        // Update rays/intersection/current_path_throughput/current_pdf
        // Russian roulette heuristics
        Real rr_prob = 1;
        if (num_vertices - 1 >= scene.options.rr_depth) {
            rr_prob = min(max((1 / eta_scale) * current_path_throughput), Real(0.95));
            if (next_pcg32_real<Real>(rng) > rr_prob) {
                // Terminate the path
                break;
            }
        }

        ray = bsdf_ray;
        vertex = *bsdf_vertex;
        current_path_throughput = current_path_throughput * (G * f) / (p2 * rr_prob);
    }
    return radiance;
}

float Scene::get_light_pdf(const Intersection& lightInter, const Vector3f& shadingPoint) const {
    Vector3f x_l = lightInter.coords;             // 光源采样点
    Vector3f n_l = normalize(lightInter.normal); // 光源法线
    Vector3f dir = (x_l - shadingPoint);
    float dist2 = dir.x * dir.x + dir.y * dir.y + dir.z * dir.z;
    Vector3f wi = normalize(dir);               // 方向
    float cos_theta_light = std::max(0.0f, dot(n_l, -wi));
    float area = lightInter.obj->getArea();

    // 转换面积 PDF 为方向 PDF
    return dist2 / (area * cos_theta_light + EPSILON);
    //return 1 / (area + EPSILON);
}

bool Scene::shadow_ray_test_point(Vector3f hit_point, Vector3f light_dir, Vector3f normal) const {
    float bias = 0.01;
    Ray shadow_ray(hit_point + bias * normal, light_dir);
    auto shadow_inter = intersect(shadow_ray);

    return !shadow_inter.happened;
}

bool Scene::shadow_ray_test_area(Vector3f hit_point, Vector3f light_dir, Vector3f normal) const {
    float bias = 0.01;
    Ray shadow_ray(hit_point + bias * normal, light_dir);
    auto shadow_inter = intersect(shadow_ray);

    return shadow_inter.happened && shadow_inter.material->hasEmission();
}
