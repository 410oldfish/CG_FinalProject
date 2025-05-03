#pragma once
#include <iostream>
#include <cmath>
#include <random>
#include <iostream>
#include <cstdlib> 
#include <ctime>

#undef M_PI
#define M_PI 3.141592653589793f

extern const float  EPSILON;
extern int MAX_DEPTH;
extern int SAMPLE_LIGHT;
const float kInfinity = std::numeric_limits<float>::max();

inline float clamp(const float &lo, const float &hi, const float &v)
{ return std::max(lo, std::min(hi, v)); }

inline  bool solveQuadratic(const float &a, const float &b, const float &c, float &x0, float &x1)
{
    float discr = b * b - 4 * a * c;
    if (discr < 0) return false;
    else if (discr == 0) x0 = x1 = - 0.5 * b / a;
    else {
        float q = (b > 0) ?
                  -0.5 * (b + sqrt(discr)) :
                  -0.5 * (b - sqrt(discr));
        x0 = q / a;
        x1 = c / q;
    }
    if (x0 > x1) std::swap(x0, x1);
    return true;
}

inline float get_random_float() {
    static thread_local std::mt19937 gen(std::random_device{}());
    static thread_local std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(gen);
}

inline Vector3f exp_vec3(const Vector3f& v) {
    return Vector3f(std::exp(v.x), std::exp(v.y), std::exp(v.z));
}

inline float sqr(float x) { return x*x; }

inline Vector3f mix(const Vector3f& a, const Vector3f& b, float t) {
    return a * (1.0f - t) + b * t;
}

inline float mix(float a, float b, float t) {
    return a * (1.0f - t) + b * t;
}

inline void UpdateProgress(float progress)
{
    int barWidth = 70;

    std::cout << "[";
    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << " %\r";
    std::cout.flush();
};

//Material funcs
inline float SchlickFresnel(float u)
{
    float m = clamp(1-u, 0, 1);
    float m2 = m*m;
    return m2*m2*m; // pow(m,5)
}

inline float GTR1(float NdotH, float a)
{
    if (a >= 1) return 1/M_PI;
    float a2 = a*a;
    float t = 1 + (a2-1)*NdotH*NdotH;
    return (a2-1) / (M_PI*log(a2)*t);
}

inline float GTR2(float NdotH, float a)
{
    float a2 = a*a;
    float t = 1 + (a2-1)*NdotH*NdotH;
    return a2 / (M_PI * t*t);
}

inline float GTR2_aniso(float NdotH, float HdotX, float HdotY, float ax, float ay)
{
    return 1 / (M_PI * ax*ay * sqr( sqr(HdotX/ax) + sqr(HdotY/ay) + NdotH*NdotH ));
}

inline float smithG_GGX(float NdotV, float alphaG)
{
    float a = alphaG*alphaG;
    float b = NdotV*NdotV;
    return 1 / (NdotV + sqrt(a + b - a*b));
}

inline float smithG_GGX_aniso(float NdotV, float VdotX, float VdotY, float ax, float ay)
{
    return 1 / (NdotV + sqrt( sqr(VdotX*ax) + sqr(VdotY*ay) + sqr(NdotV) ));
}

inline Vector3f mon2lin(Vector3f x)
{
    return Vector3f(pow(x[0], 2.2), pow(x[1], 2.2), pow(x[2], 2.2));
}