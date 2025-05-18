#pragma once
#include <iostream>
#include <cmath>
#include <random>
#include <iostream>
#include <cstdlib> 
#include <ctime>

#include <opencv2/opencv.hpp>
#include <Eigen/Dense>
#include "Vector.hpp"  // Assuming Vector3f is defined here as Eigen::Vector3f


// What does this class do?
// This class defines a set of global constants and utility functions for a ray tracing application.
// READ

#undef M_PI
#define M_PI 3.141592653589793f

extern const float  EPSILON; // extern means this variable is defined in another file and will be linked at compile time
extern const float  SMALL_EPSILON;
extern int TASK_N;
const float kInfinity = std::numeric_limits<float>::max();

// clamp function to restrict a value v between lo and hi
inline float clamp(const float &lo, const float &hi, const float &v)
{ return std::max(lo, std::min(hi, v)); }


// solveQuadratic function to find the roots of a quadratic equation
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

// get_random_float function to generate a random float between 0 and 1
inline float get_random_float()
{
    float rand = 1.0f * (std::rand() % 10000) / 10000.f;
    return rand;
}

// UpdateProgress function to display a progress bar in the console
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


inline Eigen::Matrix<Vector3f, Eigen::Dynamic, Eigen::Dynamic> loadImageAsMatrix(const std::string& path)
{
    // Load image in BGR format
    cv::Mat image = cv::imread(path, cv::IMREAD_COLOR);
    std::cout << "Loading image: " << path << std::endl;
    if (image.empty()) {
        throw std::runtime_error("Failed to load image at path: " + path);
    }

    if (image.channels() != 3 || image.type() != CV_8UC3) {
        throw std::runtime_error("Image must be 3-channel 8-bit color (CV_8UC3): " + path);
    }

    int height = image.rows;
    int width = image.cols;
    Eigen::Matrix<Vector3f, Eigen::Dynamic, Eigen::Dynamic> mat(height, width);

    // Convert BGR [0,255] → RGB [0.0, 1.0]
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x) {
            const cv::Vec3b& bgr = image.at<cv::Vec3b>(y, x);
            mat(y, x) = Vector3f(
                bgr[2] / 255.0f,  // R
                bgr[1] / 255.0f,  // G
                bgr[0] / 255.0f   // B
            );
        }

    return mat;
}


inline Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> loadImageAsMatrixBW(const std::string& path)
{
    // Load image in grayscale format
    cv::Mat image = cv::imread(path, cv::IMREAD_GRAYSCALE);
    if (image.empty()) {
        throw std::runtime_error("Failed to load image at path: " + path);
    }

    if (image.type() != CV_8UC1) {
        throw std::runtime_error("Image must be single-channel 8-bit grayscale (CV_8UC1): " + path);
    }

    int height = image.rows;
    int width = image.cols;
    Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> mat(height, width);

    // Convert [0,255] → [0.0, 1.0]
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x) {
            const uchar& gray = image.at<uchar>(y, x);
            mat(y, x) = gray / 255.0f;
        }

    return mat;
}
