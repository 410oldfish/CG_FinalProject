cmake_minimum_required(VERSION 3.10)
project(RayTracing)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenMP
find_package(OpenMP REQUIRED)

# Define source files
set(SOURCES
        main.cpp
        Object.hpp
        Vector.cpp Vector.hpp
        Sphere.hpp
        global.hpp
        Triangle.hpp
        Scene.cpp Scene.hpp
        PointLight.hpp
        AreaLight.hpp
        BVH.cpp BVH.hpp
        Bounds3.hpp
        Ray.hpp
        Material.hpp
        Intersection.hpp
        Renderer.cpp Renderer.hpp
        frame.h
        microfacet.h
        parallel.cpp
        parallel.h
        pcg.h pcg.cpp
        progress_reporter.h
        spectrum.h
)

# Add executable
add_executable(RayTracing ${SOURCES})

# Link OpenMP if found
if(OpenMP_CXX_FOUND)
    target_link_libraries(RayTracing PUBLIC OpenMP::OpenMP_CXX)
    message(STATUS "OpenMP found. Compiling with OpenMP support.")
else()
    message(FATAL_ERROR "OpenMP not found! Please install OpenMP (libgomp).")
endif()
