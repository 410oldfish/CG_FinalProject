//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"


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

// #region Disney BSDF
Vector3f Scene::castRay(const Ray &ray, int depth) const {
    if (depth > MAX_DEPTH) return Vector3f(0);

    Intersection inter = intersect(ray);
    if (!inter.happened) return backgroundColor;

    auto &material = inter.material;
    Vector3f hitPoint = inter.coords;
    Vector3f N = inter.normal.normalized();
    Vector3f wo = -ray.direction.normalized();

    // 发光物体直接返回自发光
    if (material->hasEmission()) {
        return material->m_emission;
    }

    // Russian Roulette
    float rr_prob = 0.8f;
    if (get_random_float() > rr_prob) return Vector3f(0);

    Vector3f L_direct(0), L_indirect(0);

    // ================= Direct Light via MIS =================
    for (int i = 0; i < SAMPLE_LIGHT; ++i) {
        Intersection lightInter;
        float pdf_light = 0.0f;
        sampleLight(lightInter, pdf_light);  // 按面积采样光源

        Vector3f x = hitPoint;
        Vector3f x_l = lightInter.coords;
        Vector3f n_l = lightInter.normal;
        Vector3f wi = (x_l - x).normalized();
        float dist = (x_l - x).norm();
        float dist2 = dist * dist;

        // shadow ray
        Ray shadowRay(hitPoint + N * EPSILON, wi);
        Intersection shadowHit = intersect(shadowRay);

        if (shadowHit.happened && shadowHit.material->hasEmission()) {
            Vector3f f = material->eval(wi, wo, N);
            Vector3f Le = lightInter.material->m_emission;
            float cos_theta = std::max(0.0f, dotProduct(N, wi));
            float cos_theta_light = std::max(0.0f, dotProduct(n_l, -wi));

            float pdf_bsdf = material->pdf(wo, wi, N);
            float pdf_light_dist2 = pdf_light * dist2;
            float mis_weight = pdf_light_dist2 * pdf_light_dist2 /
                               (pdf_light_dist2 * pdf_light_dist2 + pdf_bsdf * pdf_bsdf + EPSILON);
            L_direct += f * Le * cos_theta * cos_theta_light /
                        (pdf_light_dist2 + EPSILON) * mis_weight;
        }
    }
    L_direct = L_direct/(float)SAMPLE_LIGHT;

    // ================= Indirect Light =================
    Vector3f wi = material->sample(wo, N);  // 采样方向
    float pdf_bsdf = material->pdf(wo, wi, N);
    if (pdf_bsdf > EPSILON) {
        Vector3f f = material->eval(wi, wo, N);
        float cos_theta = std::max(0.0f, dotProduct(N, wi));

        // ===== 判断是否为折射路径 =====
        Vector3f shifted_origin;
        if (dotProduct(N, wi) < 0.0f) {
            // 从内部射出，翻转法线方向
            Vector3f newN = -N;
            shifted_origin = hitPoint - newN * EPSILON;
        } else {
            shifted_origin = hitPoint + N * EPSILON;
        }

        Ray newRay(shifted_origin, wi);
        Intersection newInter = intersect(newRay);

        if (newInter.happened) {
            if (newInter.material->hasEmission()) {
                // Light hit via BSDF path
                float pdf_light = get_light_pdf(newInter, hitPoint);
                float mis_weight = pdf_bsdf * pdf_bsdf /
                                   (pdf_bsdf * pdf_bsdf + pdf_light * pdf_light + EPSILON);
                L_indirect += f * newInter.material->m_emission *
                              cos_theta / (pdf_bsdf * rr_prob + EPSILON) * mis_weight;
            } else {
                // Regular recursive bounce
                Vector3f Li = castRay(newRay, depth + 1);
                L_indirect += f * Li * cos_theta / (pdf_bsdf * rr_prob + EPSILON);
            }
        }
    }

    return L_direct + L_indirect;
}

float Scene::get_light_pdf(const Intersection& lightInter, const Vector3f& shadingPoint) const {
    Vector3f x_l = lightInter.coords;             // 光源采样点
    Vector3f n_l = lightInter.normal.normalized(); // 光源法线
    Vector3f dir = (x_l - shadingPoint);
    float dist2 = dir.x * dir.x + dir.y * dir.y + dir.z * dir.z;
    Vector3f wi = dir.normalized();               // 方向
    float cos_theta_light = std::max(0.0f, dotProduct(n_l, -wi));
    float area = lightInter.obj->getArea();

    // 转换面积 PDF 为方向 PDF
    return dist2 / (area * cos_theta_light + EPSILON);
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
