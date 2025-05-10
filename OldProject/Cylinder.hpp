#ifndef RAYTRACING_CYLINDER_H
#define RAYTRACING_CYLINDER_H

#include "Object.hpp"
#include "Vector.hpp"
#include "Bounds3.hpp"
#include "Material.hpp"

class Cylinder : public Object {
public:
    Vector3f center;  // 圆柱体底面圆心
    float radius;
    float height;
    Material* m;
    float area;

    Cylinder(const Vector3f& c, const float& r, const float& h, Material* mt = new Material())
        : center(c), radius(r), height(h), m(mt)
    {
        area = 2 * M_PI * radius * height + 2 * M_PI * radius * radius; // 侧面积 + 顶底面积
        this->m_name = "Cylinder";
    }

    Intersection getIntersection(Ray ray)
    {
        Intersection result;
        result.happened = false;

        Vector3f dir = ray.direction;
        Vector3f orig = ray.origin;

        // 忽略Y方向，做XZ平面上的圆柱体求交
        Vector3f deltaP = orig - center;

        float a = dir.x * dir.x + dir.z * dir.z;
        float b = 2 * (dir.x * deltaP.x + dir.z * deltaP.z);
        float c = deltaP.x * deltaP.x + deltaP.z * deltaP.z - radius * radius;

        std::vector<float> intersection_points;
        float t0, t1;
        if (solveQuadratic(a, b, c, t0, t1)) {
            //检查侧面交点
            if (t0 > 0)
            {
                float y_hit = orig.y + t0 * dir.y;
                if (y_hit >= center.y && y_hit <= center.y + height) {
                    intersection_points.push_back(t0);
                }
            }
            if (t1 > 0)
            {
                float y_hit = orig.y + t1 * dir.y;
                if (y_hit >= center.y && y_hit <= center.y + height) {
                    intersection_points.push_back(t1);
                }
            }
        }
        //检查顶面和底面交点
        float t_disk_btm = (center.y - orig.y) / dir.y;
        Vector3f p_btm = orig + dir * t_disk_btm;
        Vector3f p_to_center = p_btm - center;
        float distance_squared_btm = p_to_center.x * p_to_center.x + p_to_center.z * p_to_center.z;
        float r_squared = radius * radius;
        // 交顶面
        float t_disk_top = (center.y + height - orig.y) / dir.y;
        Vector3f p_top = orig + dir * t_disk_top;
        Vector3f center_top = center + Vector3f(0.0f, height, 0.0f);
        Vector3f p_to_top_center = p_top - center_top;
        float distance_squared_top = p_to_top_center.x * p_to_top_center.x + p_to_top_center.z * p_to_top_center.z;

        if (distance_squared_btm <= r_squared && t_disk_btm > 0) {
            intersection_points.push_back(t_disk_btm);
        }
        if (distance_squared_top <= r_squared && t_disk_top > 0) {
            intersection_points.push_back(t_disk_top);
        }

        //没有交点
        if (intersection_points.empty()) {
            return result;
        }
        std::sort(intersection_points.begin(), intersection_points.end());
        float nearest_intersection_point = intersection_points[0];
        if (nearest_intersection_point == t0) {
            result.happened = true;
            result.coords = orig + dir * t0;
            Vector3f hitPointLocal = result.coords - center;
            result.normal = normalize(Vector3f(hitPointLocal.x, 0.0f, hitPointLocal.z)); // 侧面法线
            result.material = m;
            result.obj = this;
            result.tnear = t0;
            return result;
        }
        else if (nearest_intersection_point == t1) {
            result.happened = true;
            result.coords = orig + dir * t1;
            Vector3f hitPointLocal = result.coords - center;
            result.normal = normalize(Vector3f(hitPointLocal.x, 0.0f, hitPointLocal.z)); // 侧面法线
            result.material = m;
            result.obj = this;
            result.tnear = t1;
            return result;
        }
        else if (nearest_intersection_point == t_disk_btm) {
            result.happened = true;
            result.coords = p_btm;
            result.normal = Vector3f(0, -1, 0);
            result.material = m;
            result.obj = this;
            result.tnear = t_disk_btm;
            return result;
        }
        else if (nearest_intersection_point == t_disk_top) {
            result.happened = true;
            result.coords = p_top;
            result.normal = Vector3f(0, 1, 0);
            result.material = m;
            result.obj = this;
            result.tnear = t_disk_top;
            return result;
        }

        return result;
    }

    void getSurfaceProperties(const Vector3f& P, const Vector3f& I, const uint32_t& index, const Vector2f& uv, Vector3f& N, Vector2f& st) const
    {
        Vector3f hitPointLocal = P - center;
        float y_rel = P.y - center.y;

        if (std::abs(y_rel) < EPSILON)
        {
            // 底面
            N = Vector3f(0, -1, 0);
        }
        else if (std::abs(y_rel - height) < EPSILON)
        {
            // 顶面
            N = Vector3f(0, 1, 0);
        }
        else
        {
            // 侧面
            N = normalize(Vector3f(hitPointLocal.x, 0.0f, hitPointLocal.z));
        }

        st = Vector2f(0, 0); // 暂时纹理坐标还没展开
    }


    Vector3f evalDiffuseColor(const Vector2f& st) const
    {
        return m->getColor();
    }

    Bounds3 getBounds()
    {
        return Bounds3(
            Vector3f(center.x - radius, center.y, center.z - radius),
            Vector3f(center.x + radius, center.y + height, center.z + radius)
        );
    }

    void Sample(Intersection &pos, float &pdf)
    {
        float sideArea = 2.0f * M_PI * radius * height;
        float capArea = M_PI * radius * radius;
        float totalArea = sideArea + 2.0f * capArea;

        float choose = get_random_float() * totalArea; // [0, totalArea] 均匀采样
        if (choose < sideArea)
        {
            // 采样侧面
            float theta = 2.0f * M_PI * get_random_float(); // 周向随机
            float y = get_random_float() * height;          // 高度随机

            Vector3f dir(std::cos(theta), 0.0f, std::sin(theta)); // 水平方向
            pos.coords = center + Vector3f(radius * dir.x, y, radius * dir.z);
            pos.normal = dir; // 侧面法线指向外
            pos.obj = this;
            pos.material = m;
            pdf = 1.0f / totalArea;
        }
        else if (choose < sideArea + capArea)
        {
            // 采样底面
            float r_disk = radius * std::sqrt(get_random_float());
            float theta = 2.0f * M_PI * get_random_float();
            Vector3f p_disk(r_disk * std::cos(theta), 0.0f, r_disk * std::sin(theta));

            pos.coords = center + p_disk;
            pos.normal = Vector3f(0, -1, 0); // 朝下
            pos.obj = this;
            pos.material = m;
            pdf = 1.0f / totalArea;
        }
        else
        {
            // 采样顶面
            float r_disk = radius * std::sqrt(get_random_float());
            float theta = 2.0f * M_PI * get_random_float();
            Vector3f p_disk(r_disk * std::cos(theta), 0.0f, r_disk * std::sin(theta));

            pos.coords = center + Vector3f(p_disk.x, height, p_disk.z);
            pos.normal = Vector3f(0, 1, 0); // 朝上
            pos.obj = this;
            pos.material = m;
            pdf = 1.0f / totalArea;
        }
    }

    float getArea()
    {
        return area;
    }

    bool hasEmit()
    {
        return m->hasEmission();
    }
};

#endif // RAYTRACING_CYLINDER_H
