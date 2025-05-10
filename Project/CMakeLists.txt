cmake_minimum_required(VERSION 3.10)

project(DisneyBSDFRenderer)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)

#if(MSVC)
#  add_compile_options(/Wall)
#else()
#  add_compile_options(-Wall)
#endif()

find_package(Embree REQUIRED)
include_directories(${EMBREE_INCLUDE_PATH})
link_libraries(${EMBREE_LIBRARY})

include_directories(${CMAKE_SOURCE_DIR}/src)

set(SRCS src/main.cpp
         src/3rdparty/miniz.h
         src/3rdparty/miniz.c
         src/3rdparty/pugiconfig.hpp
         src/3rdparty/pugixml.hpp
         src/3rdparty/pugixml.cpp
         src/3rdparty/stb_image.h
         src/3rdparty/tinyexr.h
         src/3rdparty/tinyply.h
         src/filters/box.inl
         src/filters/gaussian.inl
         src/filters/tent.inl
         src/lights/diffuse_area_light.inl
         src/lights/envmap.inl
         src/materials/lambertian.inl
         src/materials/roughdielectric.inl
         src/materials/roughplastic.inl
         src/materials/disney_diffuse.inl
         src/materials/disney_metal.inl
         src/materials/disney_glass.inl
         src/materials/disney_clearcoat.inl
         src/materials/disney_sheen.inl
         src/materials/disney_bsdf.inl
         src/media/homogeneous.inl
         src/media/heterogeneous.inl
         src/parsers/load_serialized.h
         src/parsers/parse_obj.h
         src/parsers/parse_ply.h
         src/parsers/parse_scene.h
         src/phase_functions/isotropic.inl
         src/phase_functions/henyeygreenstein.inl
         src/shapes/sphere.inl
         src/shapes/triangle_mesh.inl
         src/camera.h
         src/filter.h
         src/flexception.h
         src/frame.h
         src/image.h
         src/intersection.h
        src/global.h
         src/light.h
         src/material.h
         src/matrix.h
         src/medium.h
         src/microfacet.h
         src/mipmap.h
         src/parallel.h
         src/path_tracing.h
         src/phase_function.h
         src/vol_path_tracing.h
         src/volume.h
         src/point_and_normal.h
         src/ray.h
         src/render.h
         src/scene.h
         src/shape.h
         src/spectrum.h
         src/table_dist.h
         src/texture.h
         src/transform.h
         src/vector.h
         src/parsers/load_serialized.cpp
         src/parsers/parse_obj.cpp
         src/parsers/parse_ply.cpp
         src/parsers/parse_scene.cpp
         src/camera.cpp
         src/filter.cpp
         src/image.cpp
         src/intersection.cpp
         src/light.cpp
         src/material.cpp
         src/medium.cpp
         src/parallel.cpp
         src/phase_function.cpp
         src/render.cpp
         src/scene.cpp
         src/shape.cpp
         src/table_dist.cpp
         src/transform.cpp
         src/volume.cpp)

add_executable(DisneyBSDFRenderer ${SRCS})

add_custom_command(TARGET DisneyBSDFRenderer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/embree/bin"
        $<TARGET_FILE_DIR:DisneyBSDFRenderer>)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(DisneyBSDFRenderer Threads::Threads)
