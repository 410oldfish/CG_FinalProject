//
// Created by yhy on 5/3/25.
//

#ifndef PREMATERIALS_H
#define PREMATERIALS_H

#include "Material.hpp"
// 玻璃
Material* glass = new Material(
    Vector3f(0.95, 0.95, 0.95), // basecolor
    0.01f,                     // roughness
    0.0f,                      // metallic
    0.0f,                      // subsurface
    0.0f,                      // specular
    0.0f,                      // specularTint
    0.0f,                      // clearcoat
    1.0f,                      // clearcoatGloss
    0.0f,                      // sheen
    0.5f,                      // sheenTint
    1.0f,                      // transmission
    1.5f,                      // ior
    Vector3f(0.0f)             // emission
);

// 蜡
Material* wax = new Material(
    Vector3f(1.0, 0.9, 0.7),
    0.5f,
    0.0f,
    0.8f,   // subsurface
    0.2f,
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    0.5f,
    0.0f,
    1.3f,
    Vector3f(0.0f)
);

// 木头
Material* wood = new Material(
    Vector3f(0.4, 0.25, 0.1),
    0.6f,
    0.0f,
    0.1f,
    0.3f,
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    0.5f,
    0.0f,
    1.3f,
    Vector3f(0.0f)
);

// 玉石
Material* jade = new Material(
    Vector3f(0.6, 0.85, 0.6),
    0.3f,
    0.0f,
    0.3f,
    0.3f,
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    0.5f,
    0.2f,
    1.4f,
    Vector3f(0.0f)
);

// 塑料
Material* plastic = new Material(
    Vector3f(0.8, 0.2, 0.2),
    0.4f,
    0.0f,
    0.0f,
    0.5f,
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    0.5f,
    0.0f,
    1.4f,
    Vector3f(0.0f)
);

// 金属
Material* metal = new Material(
    Vector3f(0.9, 0.85, 0.8),
    0.25f,
    1.0f, // metallic
    0.0f,
    0.0f,
    0.0f,
    0.1f,
    1.0f,
    0.0f,
    0.5f,
    0.0f,
    1.0f,
    Vector3f(0.0f)
);

// 布料
Material* fabric = new Material(
    Vector3f(0.2, 0.3, 0.8),
    0.9f,
    0.0f,
    0.0f,
    0.2f,
    0.0f,
    0.0f,
    1.0f,
    0.4f,  // sheen
    0.5f,
    0.0f,
    1.3f,
    Vector3f(0.0f)
);

// 自发光光源（白色强光）
Material* light_emissive = new Material(
    Vector3f(1.0f),  // base color doesn't matter
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 1.0f,
    Vector3f(10.0f, 10.0f, 10.0f) // emission
);

Material* dirtyMirror = new Material(
    Vector3f(0.85f, 0.85f, 0.85f), // 高亮灰白色镜面底色
    0.05f,                         // 粗糙度较低（接近镜面，但非完美）
    0.9f,                          // 高金属度（接近金属镜）
    0.0f,                          // 无 subsurface
    0.0f,                          // 默认 specular
    0.0f,                          // 不加 tint
    0.1f,                          // 少量 clearcoat 增加表面效果
    1.0f,                          // clearcoat glossiness 高
    0.0f,                          // 无 sheen
    0.5f,                          // 默认 sheen tint
    0.0f,                          // 不透光
    1.0f,                          // 默认 IOR
    Vector3f(0.0f)                 // 非自发光
);


// #region legacy materials
//pre-defined Materials
// Material* wood = new Material(OPAQUE, Vector3f(0.55f, 0.27f, 0.07f), 0.8f, 0.0f);
// Material* plastic = new Material(OPAQUE, Vector3f(0.2f, 0.6f, 0.9f), 0.2f, 0.0f);
// Material* metal_rusted = new Material(OPAQUE, Vector3f(0.7f, 0.3f, 0.1f), 0.9f, 1.0f);
// Material* metal = new Material(OPAQUE, Vector3f(0.9f, 0.85f, 0.7f), 0.05f, 1.0f);
// Material* glass = new Material(TRANSPARENT, Vector3f(1.0f), 0.05f, 0.0f);
// glass->m_ior = 1.5f;
// glass->m_transmission = 1.0f;
//
// Material* jade = new Material(TRANSPARENT , Vector3f(0.3f, 0.8f,0.4f), 0.4f, 0.0f, Vector3f(0.5f, 1.0f, 0.7f), 0.7f);
// jade->m_ior = 1.45f;
//
// Material* wax = new Material(TRANSPARENT , Vector3f(1.0f, 0.9f,0.6f), 0.6f, 0.0f, Vector3f(1.0f, 0.8f, 0.3f), 0.9f);
// wax->m_ior = 1.4f;
//
// Material* red = new Material(OPAQUE, Vector3f(1.0f, 0.0f, 0.0f));
// Material* gray = new Material(OPAQUE, Vector3f(0.2f, 0.2f, 0.2f));
// Material* pink = new Material(OPAQUE, Vector3f(0.72f, 0.48f, 0.56f));
// Material* blue = new Material(OPAQUE, Vector3f(0.2f, 0.6f, 0.86f));
// Material* green = new Material(OPAQUE, Vector3f(0.5f, 0.7f, 0.13f));
// Material* yellow = new Material(OPAQUE, Vector3f(1.0f, 1.0f, 0.0f));
// Material* white = new Material(OPAQUE, Vector3f(0.48f, 0.45f, 0.4f));
// Material* light = new Material(EMIT, Vector3f(1));
// light->m_emission=100;
// #regionend


#endif //PREMATERIALS_H
