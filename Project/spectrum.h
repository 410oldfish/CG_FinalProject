#pragma once

#include "Vector.hpp"
#include <vector>
#include "global.hpp"
/// For now, lajolla assumes we are operating in the linear and trimulus RGB color space.
/// In the future we might implement a proper spectral renderer.
using Spectrum = Vector3;

inline Spectrum make_zero_spectrum() {
    return Vector3{0, 0, 0};
}

inline Spectrum make_const_spectrum(double v) {
    return Vector3{v, v, v};
}

inline Spectrum fromRGB(const Vector3 &rgb) {
    return rgb;
}

inline Spectrum sqrt(const Spectrum &s) {
    return Vector3{sqrt(max(s[0], double(0))),
                   sqrt(max(s[1], double(0))),
                   sqrt(max(s[2], double(0)))};
}

inline Spectrum exp(const Spectrum &s) {
    return Vector3{exp(s[0]), exp(s[1]), exp(s[2])};
}

inline double luminance(const Spectrum &s) {
    return s.x * double(0.212671) + s.y * double(0.715160) + s.z * double(0.072169);
}

inline double avg(const Spectrum &s) {
    return (s.x + s.y + s.z) / 3;
}

inline Vector3 toRGB(const Spectrum &s) {
    return s;
}

/// To support spectral data, we need to convert spectral measurements (how much energy at each wavelength) to
/// RGB. To do this, we first convert the spectral data to CIE XYZ, by
/// integrating over the XYZ response curve. Here we use an analytical response
/// curve proposed by Wyman et al.: https://jcgt.org/published/0002/02/01/
inline double xFit_1931(double wavelength) {
    double t1 = (wavelength - double(442.0)) * ((wavelength < double(442.0)) ? double(0.0624) : double(0.0374));
    double t2 = (wavelength - double(599.8)) * ((wavelength < double(599.8)) ? double(0.0264) : double(0.0323));
    double t3 = (wavelength - double(501.1)) * ((wavelength < double(501.1)) ? double(0.0490) : double(0.0382));
    return double(0.362) * exp(-double(0.5) * t1 * t1) + 
           double(1.056) * exp(-double(0.5) * t2 * t2) -
           double(0.065) * exp(-double(0.5) * t3 * t3);
}
inline double yFit_1931(double wavelength) {
    double t1 = (wavelength - double(568.8)) * ((wavelength < double(568.8)) ? double(0.0213) : double(0.0247));
    double t2 = (wavelength - double(530.9)) * ((wavelength < double(530.9)) ? double(0.0613) : double(0.0322));
    return double(0.821) * exp(-double(0.5) * t1 * t1) +
           double(0.286) * exp(-double(0.5) * t2 * t2);
}
inline double zFit_1931(double wavelength) {
    double t1 = (wavelength - double(437.0)) * ((wavelength < double(437.0)) ? double(0.0845) : double(0.0278));
    double t2 = (wavelength - double(459.0)) * ((wavelength < double(459.0)) ? double(0.0385) : double(0.0725));
    return double(1.217) * exp(-double(0.5) * t1 * t1) +
           double(0.681) * exp(-double(0.5) * t2 * t2);
}
inline Vector3 XYZintegral_coeff(double wavelength) {
    return Vector3{xFit_1931(wavelength), yFit_1931(wavelength), zFit_1931(wavelength)};
}

inline Vector3 integrate_XYZ(const std::vector<std::pair<double, double>> &data) {
    static const double CIE_Y_integral = 106.856895;
    static const double wavelength_beg = 400;
    static const double wavelength_end = 700;
    if (data.size() == 0) {
        return Vector3{0, 0, 0};
    }
    Vector3 ret = Vector3{0, 0, 0};
    int data_pos = 0;
    // integrate from wavelength 400 nm to 700 nm, increment by 1nm at a time
    // linearly interpolate from the data
    for (double wavelength = wavelength_beg; wavelength <= wavelength_end; wavelength += double(1)) {
        // assume the spectrum data is sorted by wavelength
        // move data_pos such that wavelength is between two data or at one end
        while(data_pos < (int)data.size() - 1 &&
               !((data[data_pos].first <= wavelength &&
                  data[data_pos + 1].first > wavelength) ||
                 data[0].first > wavelength)) {
            data_pos += 1;
        }
        double measurement = 0;
        if (data_pos < (int)data.size() - 1 && data[0].first <= wavelength) {
            double curr_data = data[data_pos].second;
            double next_data = data[std::min(data_pos + 1, (int)data.size() - 1)].second;
            double curr_wave = data[data_pos].first;
            double next_wave = data[std::min(data_pos + 1, (int)data.size() - 1)].first;
            // linearly interpolate
            measurement = curr_data * (next_wave - wavelength) / (next_wave - curr_wave) +
                          next_data * (wavelength - curr_wave) / (next_wave - curr_wave);
        } else {
            // assign the endpoint
            measurement = data[data_pos].second;
        }
        Vector3 coeff = XYZintegral_coeff(wavelength);
        ret += coeff * measurement;
    }
    double wavelength_span = wavelength_end - wavelength_beg;
    ret *= (wavelength_span / (CIE_Y_integral * (wavelength_end - wavelength_beg)));
    return ret;
}

inline Vector3 XYZ_to_RGB(const Vector3 &xyz) {
    return Vector3{
        double( 3.240479) * xyz[0] - double(1.537150) * xyz[1] - double(0.498535) * xyz[2],
        double(-0.969256) * xyz[0] + double(1.875991) * xyz[1] + double(0.041556) * xyz[2],
        double( 0.055648) * xyz[0] - double(0.204043) * xyz[1] + double(1.057311) * xyz[2]};
}

inline Vector3 sRGB_to_RGB(const Vector3 &srgb) {
    // https://en.wikipedia.org/wiki/SRGB#From_sRGB_to_CIE_XYZ
    Vector3 rgb = srgb;
    for (int i = 0; i < 3; i++) {
        rgb[i] = rgb[i] <= double(0.04045) ?
            rgb[i] / double(12.92) :
            pow((rgb[i] + double(0.055)) / double(1.055), double(2.4));
    }
    return rgb;
}
