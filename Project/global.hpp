#pragma once
#include <cassert>
#include <iostream>
#include <cmath>
#include <random>
#include <iostream>
#include <cstdlib> 
#include <ctime>

#undef M_PI
const static double M_PI = 3.141592653589793f;

const double c_INVPI = double(1.0) / M_PI;
const double c_TWOPI = double(2.0) * M_PI;
const double c_INVTWOPI = double(1.0) / c_TWOPI;
const double c_FOURPI = double(4.0) * M_PI;
const double c_INVFOURPI = double(1.0) / c_FOURPI;
const double c_PIOVERTWO = double(0.5) * M_PI;
const double c_PIOVERFOUR = double(0.25) * M_PI;

static float MaxFloat = std::numeric_limits<float>::max();
static float Infinity = std::numeric_limits<float>::infinity();

extern int IMAGE_W;
extern int IMAGE_H;
extern int SPP;
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

// BSDF Inline Functions
inline float CosTheta(const Vector3f &w) { return w.z; }
inline float Cos2Theta(const Vector3f &w) { return w.z * w.z; }
inline float AbsCosTheta(const Vector3f &w) { return std::abs(w.z); }
inline float Sin2Theta(const Vector3f &w) {
    return std::max((float)0, (float)1 - Cos2Theta(w));
}

inline float SinTheta(const Vector3f &w) { return std::sqrt(Sin2Theta(w)); }

inline float TanTheta(const Vector3f &w) { return SinTheta(w) / CosTheta(w); }

inline float Tan2Theta(const Vector3f &w) {
    return Sin2Theta(w) / Cos2Theta(w);
}

inline float CosPhi(const Vector3f &w) {
    float sinTheta = SinTheta(w);
    return (sinTheta == 0) ? 1 : clamp(w.x / sinTheta, -1, 1);
}

inline float SinPhi(const Vector3f &w) {
    float sinTheta = SinTheta(w);
    return (sinTheta == 0) ? 0 : clamp(w.y / sinTheta, -1, 1);
}

inline float Cos2Phi(const Vector3f &w) { return CosPhi(w) * CosPhi(w); }

inline float Sin2Phi(const Vector3f &w) { return SinPhi(w) * SinPhi(w); }

inline float CosDPhi(const Vector3f &wa, const Vector3f &wb) {
    float waxy = wa.x * wa.x + wa.y * wa.y;
    float wbxy = wb.x * wb.x + wb.y * wb.y;
    if (waxy == 0 || wbxy == 0)
        return 1;
    return clamp((wa.x * wb.x + wa.y * wb.y) / std::sqrt(waxy * wbxy), -1, 1);
}

inline Vector3f Reflect(const Vector3f &wo, const Vector3f &n) {
    return -wo + 2 * dot(wo, n) * n;
}

inline bool Refract(const Vector3f &wi, const Vector3f &n, float eta,
                    Vector3f *wt) {
    // Compute $\cos \theta_\roman{t}$ using Snell's law
    float cosThetaI = dot(n, wi);
    float sin2ThetaI = std::max(float(0), float(1 - cosThetaI * cosThetaI));
    float sin2ThetaT = eta * eta * sin2ThetaI;

    // Handle total internal reflection for transmission
    if (sin2ThetaT >= 1) return false;
    float cosThetaT = std::sqrt(1 - sin2ThetaT);
    *wt = eta * -wi + (eta * cosThetaI - cosThetaT) * Vector3f(n);
    return true;
}

inline bool SameHemisphere(const Vector3f &w, const Vector3f &wp) {
    return w.z * wp.z > 0;
}

inline float Lerp(float t, float v1, float v2) { return (1 - t) * v1 + t * v2; }

inline bool Quadratic(float a, float b, float c, float *t0, float *t1) {
    // Find quadratic discriminant
    double discrim = (double)b * (double)b - 4 * (double)a * (double)c;
    if (discrim < 0) return false;
    double rootDiscrim = std::sqrt(discrim);

    // Compute quadratic _t_ values
    double q;
    if (b < 0)
        q = -.5 * (b - rootDiscrim);
    else
        q = -.5 * (b + rootDiscrim);
    *t0 = q / a;
    *t1 = c / q;
    if (*t0 > *t1) std::swap(*t0, *t1);
    return true;
}

inline float ErfInv(float x) {
    float w, p;
    x = clamp(x, -.99999f, .99999f);
    w = -std::log((1 - x) * (1 + x));
    if (w < 5) {
        w = w - 2.5f;
        p = 2.81022636e-08f;
        p = 3.43273939e-07f + p * w;
        p = -3.5233877e-06f + p * w;
        p = -4.39150654e-06f + p * w;
        p = 0.00021858087f + p * w;
        p = -0.00125372503f + p * w;
        p = -0.00417768164f + p * w;
        p = 0.246640727f + p * w;
        p = 1.50140941f + p * w;
    } else {
        w = std::sqrt(w) - 3;
        p = -0.000200214257f;
        p = 0.000100950558f + p * w;
        p = 0.00134934322f + p * w;
        p = -0.00367342844f + p * w;
        p = 0.00573950773f + p * w;
        p = -0.0076224613f + p * w;
        p = 0.00943887047f + p * w;
        p = 1.00167406f + p * w;
        p = 2.83297682f + p * w;
    }
    return p * x;
}

inline float Erf(float x) {
    // constants
    float a1 = 0.254829592f;
    float a2 = -0.284496736f;
    float a3 = 1.421413741f;
    float a4 = -1.453152027f;
    float a5 = 1.061405429f;
    float p = 0.3275911f;

    // Save the sign of x
    int sign = 1;
    if (x < 0) sign = -1;
    x = std::abs(x);

    // A&S formula 7.1.26
    float t = 1 / (1 + p * x);
    float y =
        1 -
        (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t * std::exp(-x * x);

    return sign * y;
}

// https://seblagarde.wordpress.com/2013/04/29/memo-on-fresnel-equations/
//
// The Schlick Fresnel approximation is:
//
// R = R(0) + (1 - R(0)) (1 - cos theta)^5,
//
// where R(0) is the reflectance at normal indicence.
inline float SchlickWeight(float cosTheta) {
    float m = clamp(1 - cosTheta, 0, 1);
    return (m * m) * (m * m) * m;
}

inline float FrSchlick(float R0, float cosTheta) {
    return Lerp(SchlickWeight(cosTheta), R0, 1);
}

// inline Spectrum FrSchlick(const Spectrum &R0, float cosTheta) {
//     return Lerp(SchlickWeight(cosTheta), R0, Spectrum(1.));
// }

// For a dielectric, R(0) = (eta - 1)^2 / (eta + 1)^2, assuming we're
// coming from air..
inline float SchlickR0FromEta(float eta) { return sqr(eta - 1) / sqr(eta + 1); }

inline Vector3f SphericalDirection(float sinTheta, float cosTheta, float phi) {
    return Vector3f(sinTheta * std::cos(phi), sinTheta * std::sin(phi),
                    cosTheta);
}

inline int modulo(int a, int b) {
    auto r = a % b;
    return (r < 0) ? r+b : r;
}

inline float modulo(float a, float b) {
    float r = ::fmodf(a, b);
    return (r < 0.0f) ? r+b : r;
}

inline double modulo(double a, double b) {
    double r = ::fmod(a, b);
    return (r < 0.0) ? r+b : r;
}

template <typename T>
inline T max(const T &a, const T &b) {
    return a > b ? a : b;
}

template <typename T>
inline T min(const T &a, const T &b) {
    return a < b ? a : b;
}

template <typename T>
inline T clamp(const T &x, const T &min, const T &max) {
    return x < min ? min : (x > max ? max : x);
}

inline double radians(const double deg) {
    return (M_PI / double(180)) * deg;
}

inline double degrees(const double rad) {
    return (double(180) / M_PI) * rad;
}

inline float get_random_float() {
    static std::mt19937 rng(std::random_device{}());  // 线程安全的全局 RNG
    static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(rng);
}

inline Vector3f Vector_Min(const Vector3f &p1, const Vector3f &p2) {
    return Vector3f(std::min(p1.x, p2.x), std::min(p1.y, p2.y),
                   std::min(p1.z, p2.z));
}

inline Vector3f Vector_Max(const Vector3f &p1, const Vector3f &p2) {
    return Vector3f(std::max(p1.x, p2.x), std::max(p1.y, p2.y),
                   std::max(p1.z, p2.z));
}