//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"
#include <cassert>

// Build the BVH for the scene, call once at the beginning
void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    // Create a BVH object using the objects in the scene
    // Using NAIVE split method for simplicity
    // Set maxPrimsInNode to 1 for simplicity, although the actual maxPrimsInNode is fixed to 2.
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

// Compute ray-scene intersection using the BVH
Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}


// Randomly samples a point on one of the emissive objects (area lights) in the scene, proportionally to surface area
// @param pos (output): intersection point
// @param pdf (output): probability density function of the sampling
// void Scene::sampleLight(Intersection &pos, float &pdf) const
// {   
//     // Compute the total surface area of all emissive objects
//     float emit_area_sum = 0;
//     for (uint32_t k = 0; k < objects.size(); ++k) {
//         if (objects[k]->hasEmit()){
//             pos.happened=true;  // area light that has emission exists
//             emit_area_sum += objects[k]->getArea();
//         }
//     }

//     // Get a random number between 0 and the total surface area
//     float p = get_random_float() * emit_area_sum;

//     // Fine the object that corresponds to the random number
//     emit_area_sum = 0;
//     for (uint32_t k = 0; k < objects.size(); ++k) {
//         if (objects[k]->hasEmit()){
//             emit_area_sum += objects[k]->getArea();
//             if (p <= emit_area_sum){
//                 objects[k]->Sample(pos, pdf);
//                 pdf = pdf * (objects[k]->getArea() / emit_area_sum); // pdf is the pdf of a single light source
//                 break;
//             }
//         }
//     }
// }



// // Scene.cpp
// #include "Scene.hpp"
// #include <algorithm>
// #include <cmath>

static constexpr int   MAX_DEPTH        = 8;
static constexpr float RR_PROB          = 0.9f;
const float EPSILON_OFFSET   = EPSILON;// 1e-4f;

void Scene::sampleLight(Intersection &pos, float &pdf) const {
    // 1. Compute total emissive area
    float totalArea = 0.0f;
    for (auto obj : objects)
        if (obj->hasEmit())
            totalArea += obj->getArea();

    // 2. Pick a random area‐weighted point
    float p = get_random_float() * totalArea;
    float accum = 0.0f;
    for (auto obj : objects) {
        if (!obj->hasEmit()) continue;
        float A = obj->getArea();
        accum += A;
        if (p <= accum) {
            // Sample that object: pdf_obj = 1/A
            float pdf_obj;
            obj->Sample(pos, pdf_obj);
            // Convert to global area PDF:
            pdf = pdf_obj * (A / totalArea);
            return;
        }
    }  
    // Should never get here if there's at least one light
    pdf = 0.0f;
}

// Vector3f Scene::castRay(const Ray &ray, int depth) const {
//     // 1) Depth & Russian roulette
//     if (depth > MAX_DEPTH || get_random_float() > RR_PROB)
//         return Vector3f(0.0f);

//     // 2) Trace ray
//     Intersection isect = intersect(ray);
//     if (!isect.happened)
//         return Vector3f(0.0f);

//     // 3) If hit an emitter, return its emission
//     if (isect.material->m_type == EMIT)
//         return isect.material->getEmission();

//     Vector3f L_direct(0.0f), L_indirect(0.0f);

//     // ——— NEXT-EVENT ESTIMATION (Direct) ———
//     float pdf_dir_hit_to_light = 0.0f;
//     {
//         Intersection lightI;
//         float pdf_light;
//         sampleLight(lightI, pdf_light);
//         if (pdf_light > 0.0f) {
//             // std::cout << "hey" << std::endl;
//             Vector3f wi = (lightI.coords - isect.coords).normalized();
//             float dist = (lightI.coords - isect.coords).norm();
//             float dist2 = dist * dist;

//             // Shadow ray
//             Ray shadow(
//                 isect.coords + isect.normal * EPSILON_OFFSET,
//                 wi
//             );
//             shadow.t_max = dist - EPSILON_OFFSET;
//             Intersection occ = intersect(shadow);
//             if (!occ.happened) {

//                 // std::cout << "hey" << std::endl;

//                 // Geometry term
//                 float cosθ = std::max(0.0f, dotProduct(isect.normal, wi));
//                 float cosϕ = std::max(0.0f, dotProduct(lightI.normal, -wi));

//                 // BRDF × albedo
//                 Vector3f f = isect.material->eval(wi, isect.normal)
//                            * isect.obj->evalDiffuseColor(isect.tcoords);

//                 // accumulate direct contribution
//                 L_direct = lightI.material->getEmission()
//                          * f * cosθ * cosϕ
//                          / (dist2 * pdf_light);

//                 if (cosϕ != 0){
//                     pdf_dir_hit_to_light = pdf_light * dist2 / cosϕ;
//                 } else {
//                     cosϕ = EPSILON_OFFSET;
//                     pdf_dir_hit_to_light = pdf_light * dist2 / cosϕ;

//                     // std::cout << pdf_dir_hit_to_light << std::endl;
//                 }
//             }
//         }
//     }

//     // ——— INDIRECT (Pure MCPT) ———
//     float pdf_dir_hit_to_source;
//     {
//         Vector3f wo = -ray.direction.normalized();
//         Vector3f wi = isect.material->sample(wo, isect.normal);
//         float pdf_brdf = isect.material->pdf(wo, wi, isect.normal);

//         pdf_dir_hit_to_source = pdf_brdf;
//         if (pdf_dir_hit_to_source == 0){
//             std::cout << "out" << std::endl;
//         }

//         if (pdf_brdf > 1e-6f) {
//             Ray next(
//                 isect.coords + isect.normal * EPSILON_OFFSET,
//                 wi
//             );
//             Vector3f Li = castRay(next, depth + 1);

//             Vector3f f     = isect.material->eval(wi, isect.normal)
//                             * isect.obj->evalDiffuseColor(isect.tcoords);
//             float   cosθ  = std::max(0.0f, dotProduct(isect.normal, wi));

//             L_indirect = Li * f * cosθ / (pdf_brdf * RR_PROB);
//         }
//     }



//         float weight_direct = 0.0f;
//         float weight_indirect = 0.0f;
//         if (pdf_dir_hit_to_light !=0 || pdf_dir_hit_to_source !=0) {
//             weight_direct = pdf_dir_hit_to_source / (pdf_dir_hit_to_light + pdf_dir_hit_to_source);
//             weight_indirect = pdf_dir_hit_to_light / (pdf_dir_hit_to_light + pdf_dir_hit_to_source);
//         }

//     // return weight_direct * L_direct + weight_indirect * L_indirect;
//     return L_direct + L_indirect;
// }








// !!!! Main Path Tracing Function !!!!
// Cast a ray into the scene and return the color at the intersection point
// @param ray: ray to be cast (from camera or a bounce ray)
// @param depth: current recursion depth (for limiting recursion)
// @return: color at the intersection point
// Ideas:
// - Check Intersection
// - Handle emission, diffuse, and glass materials
// - Handle area light sampling
// !!!! Main Path Tracing Function !!!!
// Cast a ray into the scene and return the color at the intersection point
// @param ray: ray to be cast (from camera or a bounce ray)
// @param depth: current recursion depth (for limiting recursion)
// @return: color at the intersection point
// Ideas:
// - Check Intersection
// - Handle emission, diffuse, and glass materials
// - Handle area light sampling




// Vector3f Scene::castRay(const Ray &ray, int depth) const
// {   
//     // 1. Depth Check [!! Return !!]
//     const int maxDepth = 8;
//     if (depth > maxDepth)
//         return Vector3f(0);

//     // // 2. Russian Roulette Check [!! Return !!]
//     constexpr float RussianRoulette = 0.9f;
//     if (get_random_float() > RussianRoulette)
//         return Vector3f(0);

//     // 3. View Point - Scene Intersection Check [!! Return !!]
//     Intersection inter = intersect(ray);
//     if (!inter.happened)
//         return Vector3f(0); // It is more physically correct to return nothing instead of the background color

//     // Hit point information
//     Vector3f point_hit = inter.coords; // Hit point coordinates
//     Vector3f normal_hit = inter.normal.normalized(); // Normal at the hit point
//     Vector3f point_hit_above = point_hit + normal_hit * EPSILON; // hit point above the surface
//     Vector2f texture_hit = inter.tcoords; // Texture Barycentric coordinates at the hit point
//     Object * object_hit = inter.obj; // Object that was hit
//     Material * material_hit = inter.material; // Material of the object that was hit
//     Vector3f dir_hit_to_view = -ray.direction.normalized(); // outgoing light direction from the hit point to the camera
//     Vector3f dir_view_to_hit = ray.direction.normalized(); // direction from the camera to the hit point
//     float kd = material_hit->Kd; // diffuse reflection coefficient
//     float ks = material_hit->Ks; // specular reflection coefficient
//     float opaqueness = material_hit->opaqueness;

//     // ==================================== EMISSION ====================================

//     // 4. Evaluate Direct Light [!! Return !!]
//     Vector3f radiance_emit = Vector3f(0); // radiance of the light source
//     if (inter.material->m_type == EMIT && depth == 0) {
//         radiance_emit = inter.material->getEmission(); // emission color of the object
//         return radiance_emit; // return the emission color directly
//     }

//     // ==================================== MATERIAL RENDERING ====================================
//     Vector3f radiance_received;

//     Vector3f dir_hit_to_source;
//     float pdf_dir_hit_to_source;

//     // 5. Sample a direction from the hit point to a potential source point [!! Return !!]
//     dir_hit_to_source = inter.material->sample(dir_hit_to_view, normal_hit); // sample a random direction over the hemisphere
//     pdf_dir_hit_to_source = inter.material->pdf(dir_view_to_hit, dir_hit_to_source, normal_hit); // pdf of the sampled direction
//     // Define a very small local EPSILON
//     if (pdf_dir_hit_to_source <= 0) {
//         std::cout << "ERROR: pdf <= 0" << "= " << pdf_dir_hit_to_source << std::endl;
//         return Vector3f(0); // Avoid division by zero
//     }

//     // 6. Recursively compute the radiance from a source point to the hit point
//     point_hit_above = point_hit + normal_hit * EPSILON;
//     Ray ray_hit_to_source(point_hit_above, dir_hit_to_source); // create a ray from the hit point to the light source
//     Vector3f radiance_source_to_hit = castRay(ray_hit_to_source, depth + 1); // recursively cast a ray to the light source

//     // 7. Evaluate BRDF
//     Vector3f base_colour_rho = inter.obj->evalDiffuseColor(texture_hit); // diffuse color of the object at the intersection point
//     Vector3f brdf_value = inter.material->eval(dir_hit_to_source, normal_hit) * base_colour_rho; // Lambertian BRDF = rho * Kd / PI

//     // 8. Cosine Factor
//     float cos_theta = std::max(0.f, dotProduct(normal_hit, dir_hit_to_source)); // angle between light normal and light emission direction

//     // 9. Compute indirect radiance
//     Vector3f radiance_indirect = radiance_source_to_hit * brdf_value * cos_theta / pdf_dir_hit_to_source / RussianRoulette;

//     // ===================================== DIRECT ====================================

//     Vector3f radiance_direct = Vector3f(0); // radiance of the light source
//     Intersection lightInter; // contain a randomly sampled point on the area light
//     float pdf_light = 0.0f; // pdf of the sampled light in terms of area
//     float pdf_dir_hit_to_light = 0.0f; // pdf of the sampled light in terms of solid angle
//     sampleLight(lightInter, pdf_light);  // sample a point on the area light
//     Vector3f dir_hit_to_light = normalize(lightInter.coords - point_hit_above);
//     if (pdf_light > 0){

//         // Shadow Test
//         point_hit_above = point_hit + normal_hit * EPSILON;
//         Vector3f shadow_ray_dir = (lightInter.coords - point_hit_above).normalized(); // shadow ray direction
//         float shadow_ray_len = (lightInter.coords - point_hit_above).norm(); // distance from hitPoint to light
//         Ray shadow_ray(point_hit_above, shadow_ray_dir); // create a shadow ray
//         shadow_ray.t_max = shadow_ray_len - EPSILON; // set the maximum distance it can travel
//         Intersection blocker = intersect(shadow_ray); // check if the shadow ray hits any object
//         if (!blocker.happened) {

//             Vector3f point_light = lightInter.coords; // Light source coordinates
//             Vector3f normal_light = lightInter.normal; // Normal at the light source
//             Vector3f vector_hit_to_light = point_light - point_hit_above; // vector from hitPoint to light
//             float len_hit_to_light = vector_hit_to_light.norm(); // distance from hitPoint to light
//             float len_hit_to_light_squared = dotProduct(vector_hit_to_light, vector_hit_to_light); // distance squared
//             Vector3f dir_hit_to_light = normalize(vector_hit_to_light); // direction from hitPoint to light
//             Vector3f dir_light_to_hit = -dir_hit_to_light; // direction from light to hitPoint

//             Vector3f radiance_light = lightInter.material->m_emission; // radiance of the light

//             Vector3f base_colour_rho = inter.obj->evalDiffuseColor(texture_hit); 
//             Vector3f brdf = inter.material->eval(dir_hit_to_light, normal_hit) * base_colour_rho; // Lambertian BRDF = rho * Kd / PI

//             float cos_i = std::max(0.f, dotProduct(dir_hit_to_light, normal_hit)); // angle between hit point normal and light source direction
//             float cos_l = std::max(0.f, dotProduct(dir_light_to_hit, normal_light)); // angle between light normal and light emission direction
//             if (cos_l > 0){

//                 pdf_dir_hit_to_light = pdf_light * len_hit_to_light_squared / cos_l; // pdf of the sampled light in terms of solid angle
//                 // Indirect light contribution
//                 radiance_direct = brdf * radiance_light * cos_i / pdf_dir_hit_to_light;
//             }

//         }
//     }

//     // ================================== COMBINE ==================================

//     // float weight_direct = 0.0f;
//     // float weight_indirect = 0.0f;
//     // if (pdf_dir_hit_to_light !=0 || pdf_dir_hit_to_source !=0) {
//     //     weight_direct = pdf_dir_hit_to_light / (pdf_dir_hit_to_light + pdf_dir_hit_to_source);
//     //     weight_indirect = pdf_dir_hit_to_source / (pdf_dir_hit_to_light + pdf_dir_hit_to_source);
//     // }

//     radiance_received = radiance_direct; // total radiance received at the hit point

//     // radiance_received = radiance_direct ;

//     // radiance_received = radiance_indirect ;
    



//     // 现在的部分都是只能被间接光照到的地方

//     // if (radiance_emit.x !=0 ){
//     //     //std::cout << radiance_emit << std::endl;
//     //     return Vector3f(0);
//     // }

//     // if (radiance_emit.x ==0 ){
//     //     //std::cout << radiance_emit << std::endl;
//     //     return Vector3f(0);
//     // }
//     // std::cout << "can you reach here?" << std::endl;



//     // // 能被间接光照到的全部返回0
//     // if (weight_indirect != 0){
//     //     return Vector3f(0);
//     // }

//     // 萤火虫、灯上的点、不能被直接光照到，但是能被间接光照到





//     Vector3f radiance_hit_to_view = radiance_received / RussianRoulette; // radiance from the hit point to the camera
//     // if (radiance_hit_to_view.x > 111){
//     //     std::cout << radiance_hit_to_view.x << std::endl;
//     //     // return Vector3f(0);
//     // }

//     // 对于收不到直接光和间接光的地方，我们直接返回0，如果返回0，萤火虫就没了，所以萤火虫在这里
//     // 比如说灯的表面上，但是为什么这个时候radiance emit应该等于100，而我们直接全部=0，但是为什么我们依旧能看见灯




//     // 如果收不到间接光，那它一定不能发光
//     // if (radiance_received.norm() == 0){
//     //     // std::cout << radiance_hit_to_view << std::endl;
//     //     return Vector3f(0);
//     // }


    
//     return radiance_hit_to_view;
// }






















// ==============


// void Scene::sampleLight(Intersection &pos, float &pdf) const
// {
//     // 1. Compute total area of all emitters
//     float totalArea = 0;
//     for (auto *obj : objects)
//         if (obj->hasEmit())
//             totalArea += obj->getArea();

//     // 2. Pick a random “area” in [0, totalArea)
//     float p = get_random_float() * totalArea;

//     // 3. Find which light that falls into
//     float accum = 0;
//     for (auto *obj : objects) {
//         if (!obj->hasEmit()) continue;
//         accum += obj->getArea();
//         if (p <= accum) {
//             // 4. Sample a point on that light: gives you a conditional PDF over that light’s area
//             float pdf_obj;  
//             obj->Sample(pos, pdf_obj);  // pdf_obj = 1 / A_k typically

//             // 5. Combine into the global PDF over all lights (area‐domain):
//             pdf = pdf_obj * (obj->getArea() / totalArea);
//             return;
//         }
//     }
//     pdf = 0;
//     std::cerr << "Error: No emitter found in the scene!" << std::endl;
// }






Vector3f Scene::castRay(const Ray &ray, int depth) const {
    // 1) Max depth
    const int maxDepth = 8;
    if (depth > maxDepth)
        return Vector3f(0);

    // 2) Russian-Roulette
    constexpr float RR = 0.9f;
    if (get_random_float() > RR)
        return Vector3f(0);

    // 3) Intersection
    Intersection inter = intersect(ray);
    if (!inter.happened)
        return Vector3f(0);

    // 4) If we hit an emissive surface, return its emission directly
    if (inter.material->hasEmission() && depth == 0)
        return inter.material->getEmission();

    // 5) Setup locals
    Vector3f hitP       = inter.coords;
    Vector3f N          = inter.normal.normalized();
    Vector3f wo         = -ray.direction.normalized();    // outgoing/view dir
    Vector3f hitP_eps   = hitP + N * EPSILON;             // avoid self‐intersection
    Vector3f rho        = inter.obj->evalDiffuseColor(inter.tcoords);  // albedo

    // ---------------------------------------------
    // 6) Next‐Event Estimation (Direct Lighting)
    // ---------------------------------------------
    Vector3f Ld(0);
    {
        Intersection lightI;
        float pdfArea = 0.f;
        sampleLight(lightI, pdfArea);                // sample a point on an emitter
        if (pdfArea > 0.f) {
            Vector3f xL = lightI.coords;
            Vector3f NL = lightI.normal.normalized();
            Vector3f emit = lightI.material->getEmission();

            Vector3f wi = (xL - hitP_eps).normalized();
            float dist = (xL - hitP_eps).norm();
            float dist2        = dist * dist;
            float cosSurf      = std::max(0.f, dotProduct(wi, N));
            float cosLight     = std::max(0.f, dotProduct(-wi, NL));

            // shadow test
            Ray shadowRay(hitP_eps, wi);
            shadowRay.t_max = std::sqrt(dist2) - EPSILON;
            if (!intersect(shadowRay).happened && cosSurf > 0.f && cosLight > 0.f) {
                // convert area‐pdf to solid‐angle pdf
                float pdfSolid = pdfArea * dist2 / cosLight;
                // Lambertian BRDF = rho / PI
                Vector3f brdf = rho * inter.material->eval(wi, N);
                Ld = emit * brdf * cosSurf / pdfSolid;
            }
        }
    }

    // ---------------------------------------------
    // 7) Monte-Carlo Path Tracing (Indirect Lighting)
    // ---------------------------------------------
    Vector3f Li(0);
    float pdfMC = 0.f;
    {
        // cosine‐weighted hemisphere sampling
        Vector3f wi = inter.material->sample(wo, N);
        pdfMC = inter.material->pdf(wo, wi, N);
        if (pdfMC > 0) {
            Ray nextRay(hitP_eps, wi);
            Vector3f rec = castRay(nextRay, depth + 1);
            float cosTheta = std::max(0.f, dotProduct(wi, N));
            Vector3f brdf  = rho * inter.material->eval(wi, N);
            Li = rec * brdf * cosTheta / pdfMC;
        } else {
            std::cout << "ERROR: pdf <= 0" << "= " << pdfMC << std::endl;
        }
    }

    // ---------------------------------------------
    // 8) MIS Weights
    // ---------------------------------------------
    float pdfLightSolid = 0.f;
    // // Recompute pdfLightSolid for MIS denominator (same as above)
    // {
    //     Intersection lightI;
    //     float pdfArea = 0.f;
    //     sampleLight(lightI, pdfArea);
    //     if (pdfArea > 0.f) {
    //         Vector3f xL = lightI.coords;
    //         Vector3f NL = lightI.normal.normalized();
    //         Vector3f wi = (xL - hitP).normalized();
    //         float dist = (xL - hitP).norm();
    //         float dist2    = dist  * dist ;
    //         float cosLight = std::max(0.f, dotProduct(-wi, NL));
    //         if (cosLight > 0.f)
    //             pdfLightSolid = pdfArea * dist2 / cosLight;
    //     }
    // }

    // float wD = 0.f, wI = 0.f;
    // float denom = pdfLightSolid + pdfMC;
    // if (denom > 0.f) {
    //     wD = pdfLightSolid / denom;
    //     wI = pdfMC          / denom;
    // }

    // ---------------------------------------------
    // 9) Final combination and RR correction
    // ---------------------------------------------
    Vector3f L = Ld + Li;
    return L / RR;
}













    //         // Shadow Test
    //         Vector3f shadow_ray_origin = hitPoint + N * EPSILON; // shadow ray origin (a bit away from hitPoint along N)
    //         Vector3f shadow_ray_dir = (lightInter.coords - shadow_ray_origin).normalized(); // shadow ray direction
    //         float shadow_ray_len = (lightInter.coords - shadow_ray_origin).norm(); // distance from hitPoint to light
    //         Ray shadow_ray(shadow_ray_origin, shadow_ray_dir); // create a shadow ray
    //         shadow_ray.t_max = shadow_ray_len - EPSILON; // set the maximum distance it can travel
    //         Intersection blocker = intersect(shadow_ray);
    //         if (blocker.happened) {
    //             continue; // in shadow, skip this light
    //         }


    //         Vector3f ws = lightInter.coords - hitPoint;
    //         float ws_len = ws.norm(); // shadow ray length
    //         float ws_len2 = dotProduct(ws, ws); // shadow ray length squared
    //         ws = normalize(ws); // shadow ray direction
            
    //         Vector3f Le = lightInter.material->m_emission; // radiance of the light

    //         Vector3f base_colour_rho = inter.obj->evalDiffuseColor(st);
    //         Vector3f brdf = inter.material->eval(ws, N) * base_colour_rho; // Lambertian BRDF = rho * Kd / PI
            
    //         float cos_i = std::max(0.f, dotProduct(ws, N)); // angle between hit point normal and shadow ray direction
    //         float cos_l = std::max(0.f, dotProduct(-ws, lightInter.normal)); // angle between light normal and light emission direction

    //         float inverse_ws_len2 = 1.0f / ws_len2; // we need this because we are integrating over the area light instead of solid angle

    //         // Compute the contribution of the light to the shading at the hit point
    //         hitColor += Le * brdf * cos_i * cos_l * inverse_ws_len2 / pdf_light;
            
    //     }







    // // Sample a direction from the current surface's BRDF
    // Vector3f dir_hit_to_next = inter.material->sample(dir_hit_to_view, normal_hit);
    // float pdf = inter.material->pdf(dir_view_to_hit, dir_hit_to_next, normal_hit);
    // if (pdf < EPSILON) return Vector3f(0);

    // // Recursively trace the new ray
    // Ray next_ray(point_hit + normal_hit * EPSILON, dir_hit_to_next);
    // Vector3f incoming_radiance = castRay(next_ray, depth + 1);

    // // BRDF and cosine
    // Vector3f base_colour_rho = inter.obj->evalDiffuseColor(texture_hit);
    // Vector3f brdf = inter.material->eval(dir_hit_to_next, normal_hit) * base_colour_rho;
    // float cos_theta = std::max(0.f, dotProduct(normal_hit, dir_hit_to_next));

    // // Final radiance estimate
    // Vector3f radiance = incoming_radiance * brdf * cos_theta / pdf / RussianRoulette;
    // return radiance;







    // // Sample a incoming light direction
    // Vector3f dir_hit_to_source = inter.material->sample(dir_hit_to_view, normal_hit); // sample a random direction over the hemisphere
    // Vector3f dir_source_to_hit = -dir_hit_to_source; // direction from the light source to the hit point

    
    // // Hit Point - Light Source Intersection [Check] !!!!
    // Vector3f x_hit_above = point_hit + normal_hit * EPSILON; // hit point above the surface
    // Ray ray_hit_to_source(x_hit_above, dir_hit_to_source); // create a ray from the hit point to the light source
    // Intersection source_inter = intersect(ray_hit_to_source); // check if the ray hits any object
    // if (!source_inter.happened) {
    //     return Vector3f(0); // No light source found
    // }

    // // Light source information
    // Vector3f point_source = source_inter.coords; // Light source coordinates
    // Vector3f normal_source = source_inter.normal; // Normal at the light source
    // Material * material_source = source_inter.material; // Material of the light source
    

    // Vector3f radiance_source_to_hit; // Incoming light radiance (colour) from the light source to the hit point
    // Vector3f brdf_value; // Bidirectional Reflectance Distribution Function (BRDF) value
    // Vector3f radiance_hit_to_view; // Outgoing light radiance (colour) from the hit point to the camera




    // // If we hit an emissive material, return the emission color directly
    // if (material_source->m_type == EMIT) {

    //     // std::cout << "Hit an emissive material" << std::endl;

    //     // L_i
    //     float cos_l = std::max(0.f, dotProduct(dir_source_to_hit, normal_source)); // angle between light normal and light emission direction
    //     radiance_source_to_hit = material_source->m_emission * cos_l; // radiance of the light source

    //     // f_r
    //     Vector3f base_colour_rho = inter.obj->evalDiffuseColor(texture_hit); // diffuse color of the object at the intersection point
    //     brdf_value = inter.material->eval(dir_hit_to_source, normal_hit) * base_colour_rho; // Lambertian BRDF = rho * Kd / PI


    //     // cosine_i
    //     float cos_i = std::max(0.f, dotProduct(dir_hit_to_source, normal_hit)); // angle between hit point normal and light source direction

    //     // pdf(dir_hit_to_source)
    //     float pdf_dir_hit_to_source = inter.material->pdf(dir_view_to_hit, dir_hit_to_source, normal_hit); // pdf of the sampled direction
    //     if (pdf_dir_hit_to_source < EPSILON) {
    //         return Vector3f(0); // Avoid division by zero
    //     }

    //     // p(russian_roulette)
    //     float p_russian_roulette = RussianRoulette;

    //     // L_o: outgoing light radiance (colour) from the hit point to the camera
    //     radiance_hit_to_view = radiance_source_to_hit * brdf_value * cos_i / pdf_dir_hit_to_source / p_russian_roulette;

    //     return radiance_hit_to_view;
    // }


    // // If we hit a diffuse material
    // if (material_source->m_type == DIFFUSE) {

    //     // std::cout << "Hit a diffuse material" << std::endl;

    //     // L_i
    //     radiance_source_to_hit = castRay(ray_hit_to_source, depth + 1); // recursively cast a ray to the light source

    //     // f_r
    //     Vector3f base_colour_rho = inter.obj->evalDiffuseColor(texture_hit); // diffuse color of the object at the intersection point
    //     brdf_value = inter.material->eval(dir_source_to_hit, normal_hit) * base_colour_rho; // Lambertian BRDF = rho * Kd / PI

    //     // cosine_i
    //     float cos_i = std::max(0.f, dotProduct(dir_hit_to_source, normal_hit)); // angle between hit point normal and light source direction

    //     // pdf(dir_hit_to_source)
    //     float pdf_dir_hit_to_source = inter.material->pdf(dir_view_to_hit, dir_hit_to_source, normal_hit); // pdf of the sampled direction

    //     // p(russian_roulette)
    //     float p_russian_roulette = RussianRoulette;

    //     // L_o: outgoing light radiance (colour) from the hit point to the camera
    //     radiance_hit_to_view = radiance_source_to_hit * brdf_value * cos_i / pdf_dir_hit_to_source / p_russian_roulette;

    //     return radiance_hit_to_view;
    // }


    // return Vector3f(0); // No light source found

    





    // Ray shadow_ray(x_hit_offset, w_shadow); // create a shadow ray
    // Inter light_source_inter = intersect(shadow_ray); // check if the shadow ray hits any object

    // // If the intersection is with an emissive material, return the emission color directly
    // if (inter.material->m_type == EMIT) {
    //     return inter.material->m_emission;
    
    // // If the intersection is with a diffuse material (Lambertian or Phong)
    // // For task < 3, we treat all materials as diffuse
    // } else if (inter.material->m_type == DIFFUSE || TASK_N<3) {

    //     if (get_random_float() < RussianRoulette && depth < MAX_DEPTH) {
    //         // Sample a random direction over the hemisphere
    //         Vector3f wo = inter.material->sample(-dir, N);

    //     }

    //     // sample area light
    //     int light_sample=4; // Sample 4 points on the area light
    //     for (int i = 0; i < light_sample && TASK_N >= 5; ++i) {


    //         Intersection lightInter; // contain a randomly sampled point on the area light
    //         float pdf_light = 0.0f; // give the PDF used to compute brightness scaling
    //         sampleLight(lightInter, pdf_light);  // sample a point on the area light
    //         // TODO: task 5 soft shadow


    //         // Shadow Test
    //         Vector3f shadow_ray_origin = hitPoint + N * EPSILON; // shadow ray origin (a bit away from hitPoint along N)
    //         Vector3f shadow_ray_dir = (lightInter.coords - shadow_ray_origin).normalized(); // shadow ray direction
    //         float shadow_ray_len = (lightInter.coords - shadow_ray_origin).norm(); // distance from hitPoint to light
    //         Ray shadow_ray(shadow_ray_origin, shadow_ray_dir); // create a shadow ray
    //         shadow_ray.t_max = shadow_ray_len - EPSILON; // set the maximum distance it can travel
    //         Intersection blocker = intersect(shadow_ray);
    //         if (blocker.happened) {
    //             continue; // in shadow, skip this light
    //         }


    //         Vector3f ws = lightInter.coords - hitPoint;
    //         float ws_len = ws.norm(); // shadow ray length
    //         float ws_len2 = dotProduct(ws, ws); // shadow ray length squared
    //         ws = normalize(ws); // shadow ray direction
            
    //         Vector3f Le = lightInter.material->m_emission; // radiance of the light

    //         Vector3f base_colour_rho = inter.obj->evalDiffuseColor(st);
    //         Vector3f brdf = inter.material->eval(ws, N) * base_colour_rho; // Lambertian BRDF = rho * Kd / PI
            
    //         float cos_i = std::max(0.f, dotProduct(ws, N)); // angle between hit point normal and shadow ray direction
    //         float cos_l = std::max(0.f, dotProduct(-ws, lightInter.normal)); // angle between light normal and light emission direction

    //         float inverse_ws_len2 = 1.0f / ws_len2; // we need this because we are integrating over the area light instead of solid angle

    //         // Compute the contribution of the light to the shading at the hit point
    //         hitColor += Le * brdf * cos_i * cos_l * inverse_ws_len2 / pdf_light;
            
    //     }
    //     hitColor = hitColor / light_sample; // average the color over the number of samples





        // // TODO: task 1.3 Basic shading

        // // Obtain some material properties
        // Vector3f V = -dir.normalized(); // from hitPoint to viewpoint
        // float Kd = inter.material->Kd; // diffuse reflection coefficient
        // float Ks = inter.material->Ks; // specular reflection coefficient
        // float shininess = inter.material->specularExponent;
        // Vector3f base_colour = inter.obj->evalDiffuseColor(st); // diffuse color of the object at the intersection point
        



        // Ambient Light (Unused)
        // hitColor += 0.1f * base_colour; // ambient light contribution


        // for (const auto & light_unique_pointer : lights){

        //     const PointLight & light = * light_unique_pointer; // obtain the light object
        //     Vector3f L = (light.position - hitPoint).normalized(); // from hitPoint to light

        //     // Shadow Test
        //     Vector3f shadow_ray_origin = hitPoint + N * EPSILON; // shadow ray origin (a bit away from hitPoint along N)
        //     Vector3f shadow_ray_dir = (light.position - shadow_ray_origin).normalized(); // shadow ray direction
        //     float shadow_ray_len = (light.position - shadow_ray_origin).norm(); // distance from hitPoint to light
        //     Ray shadow_ray(shadow_ray_origin, shadow_ray_dir); // create a shadow ray
        //     shadow_ray.t_max = shadow_ray_len - EPSILON; // set the maximum distance it can travel
        //     Intersection blocker = intersect(shadow_ray);
        //     if (blocker.happened) {
        //         continue; // in shadow, skip this light
        //     }

        //     // Diffuse
        //     float N_dot_L = std::max(0.f, dotProduct(N, L)); // angle between normal and light direction
        //     hitColor += Kd * base_colour * N_dot_L * light.intensity;

        //     // Specular
        //     Vector3f R = normalize(reflect(-L, N)); // reflected light direction
        //     float R_dot_V = std::max(0.f, dotProduct(R, V)); // angle between reflected light and view direction
        //     hitColor += Ks * powf(R_dot_V, shininess) * light.intensity;
        // }

        








    // } else if (inter.material->m_type == GLASS && TASK_N>=3) {
    //     // TODO: task 3 glass material

    //     Vector3f I = dir.normalized(); // incident ray direction (from camera to hitPoint)

    //     // Proportion of reflected light vs refracted light
    //     float kr = fresnel(I, N, inter.material->ior);

    //     // Reflection direction
    //     Vector3f reflected_ray_dir = reflect(I, N); // reflected ray direction
    //     Ray reflected_ray(hitPoint + N * EPSILON, reflected_ray_dir);
    //     Vector3f reflected_color = castRay(reflected_ray, depth + 1);

    //     // Refraction direction
    //     Vector3f refracted_ray_dir = refract(I, N, inter.material->ior); // refracted ray direction
    //     float local_epsilon = EPSILON;
    //     if (dotProduct(refracted_ray_dir, N) < 0) {
    //         local_epsilon = -EPSILON; // if the ray is inside the object, negate the epsilon
    //     }
    //     Ray refracted_ray(hitPoint + N * local_epsilon, refracted_ray_dir);
    //     Vector3f refracted_color = castRay(refracted_ray, depth + 1);

    //     // Combine the reflected and refracted colors
    //     hitColor = (1 - kr) * refracted_color + kr * reflected_color;

    // }

