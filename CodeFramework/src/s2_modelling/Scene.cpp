//
// Created by Göksu Güvendiren on 2019-05-14.
//

// 0: Use Monte Carlo Path Tracing Only
// 1: Use Monte Carlo Path Tracing and Next Event Estimation (NEE)
#define PURELY_MONTE_CARLO 1

// 0: Sample a point on the light source uniformly by area
// 1: Sample a light by its weight, and then sample a point on the light source uniformly by area
#define SAMPLE_LIGHTS_BY_WEIGHTS 0

// This control the max depth of the path tracing algorithm.
constexpr float RussianRoulette = 0.999f;
// This calculates the average depth of the path tracing algorithm.
constexpr float AvgDepthF = 1.0f / (1.0f - RussianRoulette);
constexpr int maxDepth = 2 * static_cast<int>(AvgDepthF);


// =============================================================================
//
// The above is everything you need to change the behavior of the path tracer.
//
// =============================================================================

#include "Scene.hpp"
#include <cassert>
// #include <string>

// Build the BVH for the scene, call once at the beginning
void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    // Create a BVH object using the objects in the scene
    // Using NAIVE split method for simplicity
    // Set maxPrimsInNode to 1 for simplicity, although the actual maxPrimsInNode is fixed to 2.
    // this->bvh = new BVHAccel(std::move(this->objects), 1, BVHAccel::SplitMethod::NAIVE);
    this->bvh = std::make_unique<BVHAccel>(std::move(this->objects), 1, BVHAccel::SplitMethod::NAIVE);
}

// Compute ray-scene intersection using the BVH
Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}


#if SAMPLE_LIGHTS_BY_WEIGHTS

void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    if (light_sources.empty()) {
        throw std::runtime_error("No light source found in the scene.");
    }

    // Sanity check
    if (light_sources.size() != light_source_weights.size()) {
        throw std::runtime_error("Mismatch between light sources and weights.");
    }

    // 1. Compute total weight
    float total_weight = 0.0f;
    for (float w : light_source_weights) {
        total_weight += w;
    }

    if (total_weight <= 0.0f) {
        throw std::runtime_error("Total light sampling weight must be > 0.");
    }

    // 2. Sample a light index using inverse transform sampling
    float p = get_random_float() * total_weight;
    float cumulative = 0.0f;
    for (size_t i = 0; i < light_sources.size(); ++i) {
        cumulative += light_source_weights[i];
        if (p <= cumulative) {
            const auto& light = light_sources[i];
            float pdf_local;
            light->Sample(pos, pdf_local); // pdf_local = 1 / light->getArea(), area domain

            // 3. Compute total global PDF
            float p_light = light_source_weights[i] / total_weight;  // weight-based light selection
            pdf = pdf_local * p_light; // Final PDF: light selection * sampling on surface
            return;
        }
    }

    // Should never reach here unless there's floating-point rounding error
    light_sources.back()->Sample(pos, pdf); // fallback
    pdf *= light_source_weights.back() / total_weight;
}

#else // If not sampling by weights, sample uniformly by area


// Randomly samples a point on one of the emissive objects (area lights) in the scene, proportionally to surface area
// @param pos (output): intersection point
// @param pdf (output): probability density function of the sampling
void Scene::sampleLight(Intersection &pos, float &pdf) const
{   
    // If no light source is found, throw an error
    if (light_sources.empty()) {
        throw std::runtime_error("No light source found in the scene.");
    }

    float emit_area_sum = 0;
    for (const auto& light : light_sources) {
        emit_area_sum += light->getArea();
    }
    float emit_area_sum_original = emit_area_sum; // Save the original area sum for normalization
    float p = get_random_float() * emit_area_sum;

    for (const auto& light : light_sources) {
        float area = light->getArea();
        if (p <= emit_area_sum + area) {
            light->Sample(pos, pdf); // This gives: pdf_obj = 1 / area
            pdf = pdf * (area / emit_area_sum_original); // Normalize to global PDF
            return;
        }
        emit_area_sum += area;
    }
}

#endif // SAMPLE_LIGHTS_BY_WEIGHTS



Vector3f Scene::castRay(const Ray &ray, int depth, bool got_NEE_bonus_before) const
{   
    // 1. Depth Check [!! Return !!]
    if (depth > maxDepth)
        return Vector3f(0);

    // ============================== [P1: DEAD OR LIFE ?] ==============================

    // // 2. Russian Roulette Check [!! Return !!]
    if (get_random_float() > RussianRoulette)
        return Vector3f(0);

    // 3. View Point - Scene Intersection Check [!! Return !!]
    Intersection inter = intersect(ray);
    if (!inter.happened)
        return Vector3f(0); // It is more physically correct to return nothing instead of the background color


    // 4. Evaluate Direct Light [!! Return !!]
    if (inter.material->m_type == EMIT) {

        #if PURELY_MONTE_CARLO

        Vector3f radiance_emit = inter.material->getEmission(); // emission color of the object
        return radiance_emit / RussianRoulette; // return the emission color directly

        #else

        if (got_NEE_bonus_before){
            return Vector3f(0);
        } else {
            Vector3f radiance_emit = inter.material->getEmission(); // emission color of the object
            return radiance_emit / RussianRoulette; // return the emission color directly
        }

        #endif
    }

    // Hit point information
    Vector3f point_hit = inter.coords; // Hit point coordinates
    Vector3f normal_hit = inter.normal.normalized(); // Normal at the hit point
    Vector3f dir_hit_to_view = -ray.direction.normalized(); // outgoing light direction from the hit point to the camera
    Vector3f dir_view_to_hit = ray.direction.normalized(); // direction from the camera to the hit point


    Vector2f texture_hit = inter.tcoords; // Texture Barycentric coordinates at the hit point
    Object * object_hit = inter.obj; // Object that was hit
    // Material * material_hit = inter.material; // Material of the object that was hit
    Material* material_hit = inter.material; // Material of the object that was hit

    // Five parameters of the material
    Vector3f rho = material_hit->getRho(texture_hit); // diffuse color of the object
    float kd = material_hit->getKd(texture_hit).x; // diffuse reflection coefficient
    float ks = material_hit->getKs(texture_hit).x; // specular reflection coefficient
    float ior = material_hit->getIor(texture_hit); // index of refraction
    float opaqueness = material_hit->getOpaqueness(texture_hit); // opaqueness

    
    // float kd = material_hit->Kd; // diffuse reflection coefficient
    // float ks = material_hit->Ks; // specular reflection coefficient
    // float opaqueness = material_hit->opaqueness;





    // if (inter.material->m_type == EMIT && !has_evaluated_diffuse_previously){
    //     Vector3f radiance_emit = inter.material->getEmission(); // emission color of the object
    //     return radiance_emit / RussianRoulette; // return the emission color directly
    // }


    // ============================== [P2: TRANSPARENT OR OPAQUE ?] ==============================

    // Opaque or not
    float opaque_weight = opaqueness;
    float transparent_weight = 1 - opaqueness;

    if (get_random_float() < opaque_weight){

        // ============================== [P2.1: DIFFUSE, SPECULAR, OR ABSORPTION ?] ==============================

        // Determine whether we has hit the front or back side of the object
        if (dotProduct(dir_view_to_hit, normal_hit) > 0){
            normal_hit = -normal_hit; // flip the normal
            // std::cout << "Warning: Light hit the back side of " << inter.obj->name << "and it is opque." << std::endl;
        }
        Vector3f point_hit_above = point_hit + normal_hit * EPSILON; // hit point above the surface

 
        // Diffuse or not
        float diffuse_weight = kd;
        float specular_weight = ks;
        float absorption_weight = 1.0f - kd - ks;
        bool is_diffuse = false;
        bool is_specular = false;
        bool is_absorption = false;
        float my_choice = get_random_float();
        if (my_choice < diffuse_weight){
            is_diffuse = true;
        } else if (my_choice < diffuse_weight + specular_weight){
            is_specular = true;
        } else {
            is_absorption = true;
        }

        if (is_diffuse){
            return castRayDiffuse(ray, 
                depth + 1, 
                got_NEE_bonus_before, 
                inter, 
                point_hit, 
                normal_hit, 
                point_hit_above, 
                texture_hit, 
                dir_hit_to_view, 
                dir_view_to_hit,
                rho)
                / RussianRoulette; // * diffuse_weight * opaque_weight / diffuse_weight / opaque_weight (cancelled out!!!)

        
        } else if (is_specular) {

            // ============================== [P2.1.2: REFLECT OR DIFFUSE ?] ==============================


            Vector3f I = dir_view_to_hit;
            float kr = fresnel(I, normal_hit, ior);
            float reflect_weight = kr;
            float refract_weight = 1.0f - kr;
            Vector3f offset = normal_hit * EPSILON;
            if (get_random_float() < reflect_weight){
                Vector3f Rl = reflect(I, normal_hit);
                Ray ray_hit_to_source(point_hit + offset, Rl);
                Vector3f radiance_reflect = castRay(ray_hit_to_source, depth + 1, got_NEE_bonus_before); // * reflect_weight * opaque_weight * reflect_weight
                radiance_reflect = radiance_reflect / RussianRoulette; // -- / reflect_weight / opaque_weight / reflect_weight (cancelled out!!!)
                return radiance_reflect;

            } else {
                return castRayDiffuse(ray, 
                    depth + 1, 
                    got_NEE_bonus_before, 
                    inter, 
                    point_hit, 
                    normal_hit, 
                    point_hit_above, 
                    texture_hit, 
                    dir_hit_to_view, 
                    dir_view_to_hit,
                    rho)
                    / RussianRoulette; // * diffuse_weight * opaque_weight / diffuse_weight / opaque_weight (cancelled out!!!)
            }

            // // Just reflect — like a mirror, but with energy-scaled Fresnel
            // Vector3f R = reflect(dir_view_to_hit, normal_hit);
            // Ray reflected(point_hit_above, R);
            // Vector3f Li = castRay(reflected, depth + 1, has_evaluated_diffuse_previously); // it is possible that it hit the light the next recursion.
            // Vector3f radiance_specular = Li * fresnel(dir_view_to_hit, normal_hit, ior); // * specular_weight * opaque_weight
            // radiance_specular = radiance_specular / RussianRoulette; // / specular_weight * opaque_weight (cancelled out!!!)
            // return radiance_specular;

        } else {
            return Vector3f(0);
        }

    } else {

        // ============================== [P2.2: REFLECT OR REFRACT ?] ==============================

        Vector3f I = dir_view_to_hit;
        float kr = fresnel(I, normal_hit, ior);
        float reflect_weight = kr;
        float refract_weight = 1.0f - kr;
        Vector3f offset;
        if (get_random_float() < reflect_weight){
            Vector3f Rl = reflect(I, normal_hit);
            offset = normal_hit * EPSILON; // slightly above the surface
            Ray ray_hit_to_source(point_hit + offset, Rl);
            Vector3f radiance_reflect = castRay(ray_hit_to_source, depth + 1, got_NEE_bonus_before); // * reflect_weight * transparent_weight
            radiance_reflect = radiance_reflect / RussianRoulette; // -- / reflect_weight / transparent_weight (cancelled out!!!)
            return radiance_reflect ;

        } else {
            Vector3f Rr = refract(I, normal_hit, ior);
            if (dotProduct(I, normal_hit) < 0)
                offset = normal_hit * -EPSILON; // The ray is outside and entering the surface
            else
                offset = normal_hit * EPSILON; // The ray is inside and exiting the surface

            Ray ray_hit_to_source(point_hit + offset, Rr);
            Vector3f radiance_refract = castRay(ray_hit_to_source, depth + 1, got_NEE_bonus_before); // * refract_weight * transparent_weight
            radiance_refract = radiance_refract / RussianRoulette; // -- / refract_weight / transparent_weight (cancelled out!!!)
            return radiance_refract;
        }
    }

    // Can Never Reach Here
    std::cout << "ERROR: Unhandled Case" << std::endl;
    return Vector3f(0);
}


inline Vector3f Scene::castRayDiffuse(Ray ray, int depth, bool got_NEE_bonus_before, 
        Intersection &inter, 
        Vector3f &point_hit,
        Vector3f &normal_hit,
        Vector3f &point_hit_above,
        Vector2f &texture_hit,
        Vector3f &dir_hit_to_view,
        Vector3f &dir_view_to_hit,
        Vector3f &rho
    ) const {

    // ===================================== DIRECT ====================================

    Vector3f radiance_direct = Vector3f(0); // radiance of the light source

    # if !PURELY_MONTE_CARLO

    Intersection lightInter; // contain a randomly sampled point on the area light
    float pdf_light = 0.0f; // pdf of the sampled light in terms of area
    float pdf_dir_hit_to_light = 0.0f; // pdf of the sampled light in terms of solid angle
    sampleLight(lightInter, pdf_light);  // sample a point on the area light
    Vector3f dir_hit_to_light = normalize(lightInter.coords - point_hit_above);

    if (pdf_light <= 0) {
        std::cout << "Mesh Name: " << inter.obj->name << std::endl; 
        std::cout << "Direct Light ERROR: pdf <= 0, Clamping to SMALL_EPSILON" << std::endl;
        pdf_light = SMALL_EPSILON;
    }

    // Shadow Test
    Vector3f shadow_ray_dir = (lightInter.coords - point_hit_above).normalized(); // shadow ray direction
    float shadow_ray_len = (lightInter.coords - point_hit_above).norm(); // distance from hitPoint to light
    Ray shadow_ray(point_hit_above, shadow_ray_dir); // create a shadow ray
    shadow_ray.t_max = shadow_ray_len - EPSILON; // set the maximum distance it can travel
    Intersection blocker = intersect(shadow_ray); // check if the shadow ray hits any object
    if (!blocker.happened) {

        Vector3f point_light = lightInter.coords; // Light source coordinates
        Vector3f normal_light = lightInter.normal; // Normal at the light source
        Vector3f vector_hit_to_light = point_light - point_hit_above; // vector from hitPoint to light
        float len_hit_to_light = vector_hit_to_light.norm(); // distance from hitPoint to light
        float len_hit_to_light_squared = dotProduct(vector_hit_to_light, vector_hit_to_light); // distance squared
        Vector3f dir_hit_to_light = normalize(vector_hit_to_light); // direction from hitPoint to light
        Vector3f dir_light_to_hit = -dir_hit_to_light; // direction from light to hitPoint

        Vector3f radiance_light = lightInter.material->m_emission; // radiance of the light

        Vector3f brdf = rho / M_PI; // Lambertian BRDF = rho / PI

        float cos_i = std::max(0.f, dotProduct(dir_hit_to_light, normal_hit)); // angle between hit point normal and light source direction
        float cos_l = std::max(0.f, dotProduct(dir_light_to_hit, normal_light)); // angle between light normal and light emission direction
        
        radiance_direct =  radiance_light * brdf * cos_i * cos_l / len_hit_to_light_squared / pdf_light;
    }

    bool get_NEE_bonus_now = !blocker.happened;

    #endif // !PURELY_MONTE_CARLO

    // =================================== INDIRECT ====================================

    // 5. Sample a direction from the hit point to a potential source point [!! Return !!]
    Vector3f dir_hit_to_source = inter.material->sample(dir_hit_to_view, normal_hit); // sample a random direction over the hemisphere
    float pdf_dir_hit_to_source = inter.material->pdf(dir_view_to_hit, dir_hit_to_source, normal_hit); // pdf of the sampled direction
    // Define a very small local EPSILON
    if (pdf_dir_hit_to_source <= 0) {
        std::cout << "Mesh Name: " << inter.obj->name << std::endl;
        std::cout << "Indirect Light ERROR: pdf <= 0, Clamping to SMALL_EPSILON" << std::endl;
        pdf_dir_hit_to_source = SMALL_EPSILON;
    }
    
    // 6. Recursively compute the radiance from a source point to the hit point
    Ray ray_hit_to_source(point_hit_above, dir_hit_to_source); // create a ray from the hit point to the light source
    Vector3f radiance_source_to_hit = castRay(ray_hit_to_source, depth + 1, true); // recursively cast a ray to the light source

    // 7. Evaluate BRDF
    Vector3f brdf_value = rho / M_PI; // Lambertian BRDF = rho / PI

    // 8. Cosine Factor
    float cos_theta = std::max(0.f, dotProduct(normal_hit, dir_hit_to_source)); // angle between light normal and light emission direction

    // 9. Compute indirect radiance
    Vector3f radiance_indirect = radiance_source_to_hit * brdf_value * cos_theta / pdf_dir_hit_to_source;


    // ================================== COMBINE ==================================

    Vector3f radiance_diffuse = radiance_direct + radiance_indirect;
    return radiance_diffuse;
}