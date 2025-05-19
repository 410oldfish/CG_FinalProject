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

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;


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


inline Eigen::Matrix<Vector3f, Eigen::Dynamic, Eigen::Dynamic>* loadImageAsMatrix(
    std::map<std::string, void *>& opened_images,
    // Eigen::Matrix<Vector3f, Eigen::Dynamic, Eigen::Dynamic>
    const fs::path& texture_path,
    const fs::path& model_path = "")
{
    // Resolve full path
    fs::path full_path = model_path.empty() ? texture_path : (model_path / texture_path);
    std::string full_path_str = full_path.string();

    // Check if the image is already loaded
    auto it = opened_images.find(full_path_str);
    if (it != opened_images.end()) {
        auto* mat_ptr = static_cast<Eigen::Matrix<Vector3f, Eigen::Dynamic, Eigen::Dynamic>*>(it->second);
        return mat_ptr; // Reuse existing pointer
    }

    // Load image using OpenCV
    cv::Mat image = cv::imread(full_path, cv::IMREAD_COLOR);
    std::cout << "Loading image: " << full_path_str << std::endl;

    if (image.empty()) {
        throw std::runtime_error("Failed to load image at path: " + full_path_str);
    }

    if (image.channels() != 3 || image.type() != CV_8UC3) {
        throw std::runtime_error("Image must be 3-channel 8-bit color (CV_8UC3): " + full_path_str);
    }

    int height = image.rows;
    int width = image.cols;

    // Allocate matrix on heap
    auto* mat_ptr = new Eigen::Matrix<Vector3f, Eigen::Dynamic, Eigen::Dynamic>(height, width);

    // Fill matrix: Convert BGR [0,255] → RGB [0.0, 1.0]
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x) {
            const cv::Vec3b& bgr = image.at<cv::Vec3b>(y, x);
            (*mat_ptr)(y, x) = Vector3f(
                bgr[2] / 255.0f,  // R
                bgr[1] / 255.0f,  // G
                bgr[0] / 255.0f   // B
            );
        }

    // Cache it
    opened_images[full_path_str] = mat_ptr;

    return mat_ptr;
}



inline Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>* loadImageAsMatrixBW(
    std::map<std::string, void *>& opened_images,
    const fs::path& texture_path,
    const fs::path& model_path = "")
{
    // Construct full path
    fs::path full_path = model_path.empty() ? texture_path : (model_path / texture_path);
    std::string full_path_str = full_path.string();

    // Check if already loaded
    auto it = opened_images.find(full_path_str);
    if (it != opened_images.end()) {
        auto* mat_ptr = static_cast<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>*>(it->second);
        return mat_ptr; // Reuse existing pointer
    }

    // Load grayscale image
    cv::Mat image = cv::imread(full_path, cv::IMREAD_GRAYSCALE);
    std::cout << "Loading grayscale image: " << full_path_str << std::endl;

    if (image.empty()) {
        throw std::runtime_error("Failed to load image at path: " + full_path_str);
    }

    if (image.type() != CV_8UC1) {
        throw std::runtime_error("Image must be single-channel 8-bit grayscale (CV_8UC1): " + full_path_str);
    }

    int height = image.rows;
    int width = image.cols;

    // Allocate matrix on heap
    auto* mat_ptr = new Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>(height, width);

    // Convert to float in range [0.0, 1.0]
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x) {
            uchar gray = image.at<uchar>(y, x);
            (*mat_ptr)(y, x) = gray / 255.0f;
        }

    // Cache the loaded matrix
    opened_images[full_path_str] = mat_ptr;

    return mat_ptr;
}
