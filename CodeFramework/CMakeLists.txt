cmake_minimum_required(VERSION 3.10)
project(cgfinal)

# Use C++17
set(CMAKE_CXX_STANDARD 17)

# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# add_definitions(-DSOURCE_DIR=\"${CMAKE_SOURCE_DIR}\")

# Enable OpenMP (optional)
find_package(OpenMP)
find_package(OpenCV REQUIRED)
find_package(Eigen3 REQUIRED)

include_directories(${EIGEN3_INCLUDE_DIR} ${OpenCV_INCLUDE_DIRS})


# === Source Files ===
set(SRC_FILES
    src/main.cpp

    # s1_preprocessing
    src/s1_preprocessing/Material.hpp
    src/s1_preprocessing/OBJ_Loader.hpp
    src/s1_preprocessing/Object.hpp
    src/s1_preprocessing/Sphere.hpp
    src/s1_preprocessing/Triangle.hpp

    # s2_modelling
    src/s2_modelling/PointLight.hpp
    src/s2_modelling/Scene.cpp
    src/s2_modelling/Scene.hpp
    
    # s2_rendering
    src/s3_rendering/Bounds3.hpp
    src/s3_rendering/BVH.cpp
    src/s3_rendering/BVH.hpp
    src/s3_rendering/Intersection.hpp
    src/s3_rendering/Ray.hpp
    src/s3_rendering/Renderer.cpp
    src/s3_rendering/Renderer.hpp

    # utilities
    src/utilities/global.hpp
    src/utilities/Vector.hpp
    src/utilities/Vector.cpp
)

# === Create Executable ===
add_executable(cgfinal ${SRC_FILES})

# === Include Paths ===
target_include_directories(cgfinal PUBLIC
    ${PROJECT_SOURCE_DIR}/src/s1_preprocessing
    ${PROJECT_SOURCE_DIR}/src/s2_modelling
    ${PROJECT_SOURCE_DIR}/src/s3_rendering
    ${PROJECT_SOURCE_DIR}/src/s4_postprocessing
    ${PROJECT_SOURCE_DIR}/src/utilities
)

target_link_libraries(cgfinal PUBLIC ${OpenCV_LIBS})


# === Link OpenMP if available ===
if(OpenMP_CXX_FOUND)
    target_link_libraries(cgfinal PUBLIC OpenMP::OpenMP_CXX)
    message(STATUS "OpenMP found. Compiling with OpenMP support.")
else()
    message(STATUS "OpenMP not found. Compiling without OpenMP support.")
endif()

