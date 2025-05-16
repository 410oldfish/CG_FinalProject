cmake_minimum_required(VERSION 3.10)
project(cgfinal)

# Use C++17
set(CMAKE_CXX_STANDARD 17)

# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# add_definitions(-DSOURCE_DIR=\"${CMAKE_SOURCE_DIR}\")

# Enable OpenMP (optional)
find_package(OpenMP)

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
    src/s2_rendering/Bounds3.hpp
    src/s2_rendering/BVH.cpp
    src/s2_rendering/BVH.hpp
    src/s2_rendering/Intersection.hpp
    src/s2_rendering/Ray.hpp
    src/s2_rendering/Renderer.cpp
    src/s2_rendering/Renderer.hpp

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
    ${PROJECT_SOURCE_DIR}/src/s2_rendering
    ${PROJECT_SOURCE_DIR}/src/s3_postprocessing
    ${PROJECT_SOURCE_DIR}/src/utilities
)

# === Link OpenMP if available ===
if(OpenMP_CXX_FOUND)
    target_link_libraries(cgfinal PUBLIC OpenMP::OpenMP_CXX)
    message(STATUS "OpenMP found. Compiling with OpenMP support.")
else()
    message(STATUS "OpenMP not found. Compiling without OpenMP support.")
endif()

