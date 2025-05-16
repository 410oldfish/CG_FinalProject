#pragma once

#include "BVH.hpp"
#include "Intersection.hpp"
#include "Material.hpp"
#include "OBJ_Loader.hpp"
#include "Object.hpp"
#include "Triangle.hpp"
#include <cassert>
#include <array>
#include <cstring>


// ========================= TRIANGLE CLASS ========================= //

class Triangle : public Object
{
public:
    Vector3f v0, v1, v2; // vertices A, B ,C , counter-clockwise order
    Vector3f e1, e2;     // 2 edges v1-v0, v2-v0;
    Vector2f t0, t1, t2; // texture coords
    Vector3f normal;
    float area;
    std::shared_ptr<Material> m;

    // Constructor. Precompute the normal and area of the triangle
    // @ param _v0, _v1, _v2 are the vertices of the triangle
    // @ param _m is the material of the triangle
    Triangle(Vector3f _v0, Vector3f _v1, Vector3f _v2, std::shared_ptr<Material> _m = std::make_shared<Material>(Material()))
        : v0(_v0), v1(_v1), v2(_v2), m(_m)
    {
        e1 = v1 - v0;
        e2 = v2 - v0;
        normal = normalize(crossProduct(e1, e2));
        area = crossProduct(e1, e2).norm()*0.5f;
    }

    // Möller–Trumbore intersection algorithm
    // @ param ray is the ray to be tested
    // @ return an Intersection object that contains the intersection information
    Intersection getIntersection(Ray ray) override;

    // Obtain the surface normal and texture coordinates
    // @ param P is the point of intersection on the triangle surface
    // @ param I is the incoming ray direction
    // @ param index is the index of the triangle within a mesh
    // @ param uv is the texture coordinates of the triangle ?
    // @ param (output) N is the normal at the intersection point
    // @ param (output) st is the texture coordinates of the triangle
    void getSurfaceProperties(const Vector3f& P, const Vector3f& I,
                              const uint32_t& index, const Vector2f& uv,
                              Vector3f& N, Vector2f& st) const override
    {
        N = normal;
        //        throw std::runtime_error("triangle::getSurfaceProperties not
        //        implemented.");
    }
    // Evaluate the diffuse color of the triangle at a given texture coordinate
    // @ param uv is the texture coordinates of the triangle
    // @ return the diffuse color of the triangle at the given texture coordinate
    Vector3f evalDiffuseColor(const Vector2f&) const override;

    // Compute the bounding box of the triangle using min/max of the vertices
    // @ return the bounding box of the triangle
    Bounds3 getBounds() override;

    // Uniformly sample a point on the triangle surface using barycentric coordinates
    // @ param pos (output) is the intersection point on the triangle surface
    // @ param pdf (output) is the probability density function of the sample
    // mark override
    void Sample(Intersection &pos, float &pdf) override{
        float x = std::sqrt(get_random_float()), y = get_random_float();
        pos.coords = v0 * (1.0f - x) + v1 * (x * (1.0f - y)) + v2 * (x * y);
        pos.normal = this->normal;
        pdf = 1.0f / area;
    }

    // Return the precomputed area of the triangle

    float getArea() override {
        return area;
    }

    // Check if the triangle has an emission property
    bool hasEmit() override {
        return m->hasEmission();
    }
};

// ========================= TRIANGLE MESH CLASS ========================= //

class MeshTriangle : public Object
{
public:
    // Constructor from raw vertex data
    // @ param verts is the array of vertex positions
    // @ param vertsIndex is the array of vertex indices (3 indices per triangle)
    // @ param numTris is the number of triangles
    // @ param st is the array of texture coordinates (1 for each vertex)
    // @ param mt is the material of the mesh
    MeshTriangle(const Vector3f* verts, const uint32_t* vertsIndex, const uint32_t& numTris, 
        const Vector2f* st, std::shared_ptr<Material> mt = std::make_shared<Material>(Material()))
    {   
        // Loop through vertsIndex to find out the number of vertices - 1, i.e., the max vertex index
        uint32_t maxIndex = 0;
        for (uint32_t i = 0; i < numTris * 3; ++i)
            if (vertsIndex[i] > maxIndex)
                maxIndex = vertsIndex[i];
        
        // Copy (maxIndex + 1) texture coordinates from st to stCoordinates
        stCoordinates = std::unique_ptr<Vector2f[]>(new Vector2f[maxIndex]);
        memcpy(stCoordinates.get(), st, sizeof(Vector2f) * maxIndex);

        // Assign the material to the mesh, m is one of the fields of the Object class
        m=mt;

        // Intialise a infinitely large bounding box
        Vector3f min_vert = Vector3f{std::numeric_limits<float>::infinity(),
                                     std::numeric_limits<float>::infinity(),
                                     std::numeric_limits<float>::infinity()};
        Vector3f max_vert = Vector3f{-std::numeric_limits<float>::infinity(),
                                     -std::numeric_limits<float>::infinity(),
                                     -std::numeric_limits<float>::infinity()};
        
        // Iterate over each triangle faces              
        for (int i = 0; i < numTris; i++) {
            std::array<Vector3f, 3> face_vertices;

            for (int j = 0; j < 3; j++) {
                // Fetch the three vertex positions of the triangle
                auto vert = Vector3f(verts[vertsIndex[i*3+j]].x,
                                     verts[vertsIndex[i*3+j]].y,
                                     verts[vertsIndex[i*3+j]].z);
                face_vertices[j] = vert;
                
                // Update the bounding box of the mesh
                min_vert = Vector3f(std::min(min_vert.x, vert.x),
                                    std::min(min_vert.y, vert.y),
                                    std::min(min_vert.z, vert.z));
                max_vert = Vector3f(std::max(max_vert.x, vert.x),
                                    std::max(max_vert.y, vert.y),
                                    std::max(max_vert.z, vert.z));
            }
            // Create a new triangle object and add it to the triangles vector
            // Triangle* tri = new Triangle(face_vertices[0], face_vertices[1],
            //                             face_vertices[2], mt);
            // Triangle* tri = std::make_unique<Triangle>(face_vertices[0], face_vertices[1],
            //                        face_vertices[2], mt).release();
            std::unique_ptr<Triangle> tri = std::make_unique<Triangle>(face_vertices[0], face_vertices[1],
                                   face_vertices[2], mt);

            // Set the precomputed texture coordinates for each vertex of the triangle
            tri->t0=st[vertsIndex[i*3]];
            tri->t1=st[vertsIndex[i*3+1]];
            tri->t2=st[vertsIndex[i*3+2]];
            
            // Store this triangle into the triangle list of the current mesh
            // triangles.push_back
            triangles.push_back(std::move(tri));

//            triangles.emplace_back(face_vertices[0], face_vertices[1],
//                                   face_vertices[2], mt);
        }

        // Finalise a mesh bounding box
        bounding_box = Bounds3(min_vert, max_vert);

        // Put all triangle pointers into a vector
        std::vector<std::unique_ptr<Object>> ptrs;
        for (auto& tri : triangles){
            area += tri->area;
            ptrs.push_back(std::move(tri));
        }
        // Create a BVH tree from the triangle pointers
        bvh = std::make_unique<BVHAccel>(std::move(ptrs));
    }


    // Constructor from OBJ file
    // @ param filename is the name of the OBJ file
    // @ param offset is the offset to be added to each vertex position
    // @ param mt is the material of the mesh
    MeshTriangle(const std::string& filename, Vector3f offset, 
                 std::shared_ptr<Material> mt = std::make_shared<Material>(Material()))
    {   
        // Load the OBJ file using the objl::Loader class
        objl::Loader loader;
        loader.LoadFile(filename);

        // Initialse area to 0
        area = 0;

        // Assigns the provided material
        m = mt;

        // Only one mesh is supported
        assert(loader.LoadedMeshes.size() == 1);
        // A Mesh has the following fields:
        //   std::string MeshName;
        //   std::vector<Vertex> Vertices;
        //   std::vector<unsigned int> Indices;
        //   std::optional<Material> MeshMaterial;
        auto mesh = loader.LoadedMeshes[0];
        
        // Prepare a infinitely large bounding box
        Vector3f min_vert = Vector3f{std::numeric_limits<float>::infinity(),
                                     std::numeric_limits<float>::infinity(),
                                     std::numeric_limits<float>::infinity()};
        Vector3f max_vert = Vector3f{-std::numeric_limits<float>::infinity(),
                                     -std::numeric_limits<float>::infinity(),
                                     -std::numeric_limits<float>::infinity()};
        
        // Loop over the vertices (3 vertices per triangle)
        for (int i = 0; i < mesh.Vertices.size(); i += 3) {

            // Extract the coordinates of the three vertices of the triangle
            std::array<Vector3f, 3> face_vertices;
            for (int j = 0; j < 3; j++) {
                auto vert = Vector3f(mesh.Vertices[i + j].Position.X+offset.x,
                                     mesh.Vertices[i + j].Position.Y+offset.y,
                                     mesh.Vertices[i + j].Position.Z+offset.z);
                face_vertices[j] = vert;
                
                // Update the bounding box of the mesh
                min_vert = Vector3f(std::min(min_vert.x, vert.x),
                                    std::min(min_vert.y, vert.y),
                                    std::min(min_vert.z, vert.z));
                max_vert = Vector3f(std::max(max_vert.x, vert.x),
                                    std::max(max_vert.y, vert.y),
                                    std::max(max_vert.z, vert.z));
            }

            // Create a new triangle object and add it to the triangles vector
            // Use of emplace_back to avoid unnecessary copies
            // triangles.emplace_back(face_vertices[0], face_vertices[1],
            //                        face_vertices[2], mt);
            std::unique_ptr<Triangle> tri = std::make_unique<Triangle>(face_vertices[0], face_vertices[1],
                                   face_vertices[2], mt);

            triangles.push_back(std::move(tri));
        }

        // Finalise the mesh bounding box
        bounding_box = Bounds3(min_vert, max_vert);

        // Put all triangle pointers into a vector
        std::vector<std::unique_ptr<Object>> ptrs;
        for (auto& tri : triangles){
            area += tri->area;
            ptrs.push_back(std::move(tri));
        }

        // Create a BVH tree from the triangle pointers
        bvh = std::make_unique<BVHAccel>(std::move(ptrs));
    }

    // Return the mesh's overall bounding box
    // @ return the bounding box of the mesh
    Bounds3 getBounds() { return bounding_box; }


    // Compute the surface normal and texture coordinates at a point of intersection on a triangle
    // @ param P is the point of intersection on the triangle surface (not used)
    // @ param I is the incoming ray direction (not used)
    // @ param index is the index of the triangle within a mesh
    // @ param uv is the Barycentric coordinates (u, v) of intersection point
    // @ param (output) N is the normal at the intersection point
    // @ param (output) st is the interpolated texture coordinates of the triangle
    void getSurfaceProperties(const Vector3f& P, const Vector3f& I,
                              const uint32_t& index, const Vector2f& uv,
                              Vector3f& N, Vector2f& st) const
    {
        // Retrieve the 3 vertices of the triangle
        // vertexIndex[index * 3] is the index of the first vertex
        // vertices[vertexIndex[index * 3]] is the position of the first vertex
        const Vector3f& v0 = vertices[vertexIndex[index * 3]];
        const Vector3f& v1 = vertices[vertexIndex[index * 3 + 1]];
        const Vector3f& v2 = vertices[vertexIndex[index * 3 + 2]];

        // Normalise the edges of the triangle and compute the normal
        Vector3f e0 = normalize(v1 - v0);
        Vector3f e1 = normalize(v2 - v1);
        N = normalize(crossProduct(e0, e1));

        // Retrieve the 3 texture coordinates of the triangle
        // vertexIndex[index * 3] is the index of the first vertex
        // stCoordinates[vertexIndex[index * 3]] is the texture coordinate of the first vertex
        const Vector2f& st0 = stCoordinates[vertexIndex[index * 3]];
        const Vector2f& st1 = stCoordinates[vertexIndex[index * 3 + 1]];
        const Vector2f& st2 = stCoordinates[vertexIndex[index * 3 + 2]];

        // Compute the interpolated texture coordinates using Barycentric coordinates
        // Why no perspective correction needed in ray tracing?
        // We are doing interpolation in the real world space, not in the screen space
        st = st0 * (1 - uv.x - uv.y) + st1 * uv.x + st2 * uv.y;
    }

    // Compute the diffuse based colour given a texture coordinate
    // The checkerboard texture is used
    // @ param st is the texture coordinates of the triangle
    // @ return the diffuse color of the triangle at the given texture coordinate
    Vector3f evalDiffuseColor(const Vector2f& st) const
    {
        if(!m->textured)return m->getColor(); // if not textured, return color

        float scale = 5;
        float pattern =
            (fmodf(st.x * scale, 1) > 0.5) ^ (fmodf(st.y * scale, 1) > 0.5);
        return lerp(Vector3f(0.815, 0.235, 0.031),
                    Vector3f(0.937, 0.937, 0.231), pattern);




    // // t0, t1, t2 are the texture coordinates of the triangle
    // // st is the barycentric coordinates of the point
    // auto uv = t0 * (1 - st.x - st.y) + t1 * st.x + t2 * st.y;
    // float scale = 5;

    // // Decide whether that UV point lands on a red or yellow square
    // // The pattern is a checkerboard pattern


    // // What does this line do?
    // // It takes the x and y coordinates of the UV point, multiplies them by the scale,
    // // and takes the modulus with 1.0 to get a value between 0 and 1.
    // // Then it checks if the value is greater than 0.5 and returns true or false.
    // // The ^ operator is a bitwise XOR operator, which means that it will return true
    // // if one of the values is true and the other is false.
    // float pattern = (fmodf(uv.x * scale, 1) > 0.5) ^ (fmodf(uv.y * scale, 1) > 0.5);

    // // What does this line do?
    // // It takes the pattern value and uses it to interpolate between two colors.
    // return lerp(Vector3f(0.815, 0.235, 0.031), Vector3f(0.937, 0.937, 0.231), pattern);

    }

    // Find the intersection of a ray with the mesh
    // @ param ray is the ray to be tested
    // @ return an Intersection object that contains the intersection information
    // Intersection:
    //   - happened: true if the ray hits the mesh
    //   - coords: the coordinates of the intersection point
    //   - normal: the normal at the intersection point
    //   - tnear: the distance from the ray origin to the intersection point
    //   - material: the material of the object
    // Idea: use the BVH tree to find the closest intersection
    Intersection getIntersection(Ray ray)
    {
        Intersection intersec;

        if (bvh) {
            intersec = bvh->Intersect(ray);
        }

        return intersec;
    }
    
    // Sample a point on the mesh surface using the BVH tree
    void Sample(Intersection &pos, float &pdf){
        bvh->Sample(pos, pdf);
        pos.obj=this;
        pos.material=this->m;
    }
    // Return the precomputed area of the mesh
    float getArea(){
        return area;
    }
    // Check if the mesh has an emission property
    bool hasEmit(){
        return m->hasEmission();
    }

    // Fields

    Bounds3 bounding_box; // axis-aligned bounding box of the entire mesh
    std::unique_ptr<Vector3f[]> vertices; // dynamic array of all vertices
    uint32_t numTriangles; // number of triangles in the mesh
    std::unique_ptr<uint32_t[]> vertexIndex; // dynamic array of vertex indices (3 indices per triangle)
    std::unique_ptr<Vector2f[]> stCoordinates; // dynamic array of texture coordinates (1 per vertex)
    std::vector<std::unique_ptr<Triangle>> triangles; // vector of triangle objects

    // BVHAccel* bvh; // BVH tree for fast intersection testing
    std::unique_ptr<BVHAccel> bvh; // BVH tree for fast intersection testing
    float area; // surface area of the mesh, used for sampling

    std::shared_ptr<Material> m; // material of the mesh
};


// ========================= FUNCTION IMPLEMENTATION FOR TRIANGLE CLASS  ========================= //


inline Bounds3 Triangle::getBounds() { return Union(Bounds3(v0, v1), v2); }


// Möller–Trumbore intersection algorithm
// @ param ray is the ray to be tested
// @ return an Intersection object that contains the intersection information
inline Intersection Triangle::getIntersection(Ray ray)
{
    Intersection inter;
    // TODO: task 1.1 Ray-Triangle Intersection

    // constexpr float LOCAL_EPSILON = std::numeric_limits<float>::epsilon();
    constexpr float LOCAL_EPSILON = 1e-6f;
    Vector3f & D = ray.direction; // The direction of the ray
    Vector3f & O = ray.origin; // The origin of the ray
    Vector3f & P0 = v0; // The first vertex of the triangle
    Vector3f & P1 = v1; // The second vertex of the triangle
    Vector3f & P2 = v2; // The third vertex of the triangle

    // ==========

    // E_1 = P_1 - P_0 (an edge of the triangle)
    Vector3f E1 = P1 - P0;

    // E_2 = P_2 - P_0 (an edge of the triangle)
    Vector3f E2 = P2 - P0;

    // S = O - P_0 (the vector from the triangle vertex to the ray origin)
    Vector3f S;

    // S_1 = D x E_2 (the cross product of the ray direction and the second edge of the triangle)
    Vector3f S1 = crossProduct(D, E2);

    // S_2 = S x E_1 (the cross product of the vector from the triangle vertex to the ray origin and the first edge of the triangle)
    Vector3f S2;

    // ==========

    // Compute the "determinant" = S1 dot E_1
    float det = dotProduct(S1, E1);
    
    // If the ray is parallel to the triangle (E1 dot S1 == 0), return no intersection
    if (det < LOCAL_EPSILON && det > -LOCAL_EPSILON) {
        return inter;
    }

    // Compute the inverse of the determinant
    float inv_det = 1.0f / det;

    // ==========

    // Compute b1 = (S1 dot S) * inv_det
    S = O - P0;
    float b1 = dotProduct(S1, S) * inv_det;

    // Check b1 with epsilon
    if ((b1 < -LOCAL_EPSILON || b1 > 1.0f + LOCAL_EPSILON)) {
        return inter;
    }

    // Compute b2 = (D dot S2) * inv_det
    S2 = crossProduct(S, E1);
    float b2 = dotProduct(D, S2) * inv_det;

    // Check b2 with epsilon
    if ((b2 < -LOCAL_EPSILON || b1 + b2 > 1.0f + LOCAL_EPSILON)) {
        return inter;
    }

    // Compute t = (E2 dot S2) * inv_det
    float t = dotProduct(E2, S2) * inv_det;

    // Check t with epsilon
    if (t < ray.t_min || t > ray.t_max || t < LOCAL_EPSILON) {
        return inter; // behind the ray or out of range
    }

    // ==========

    // Build the intersection object
    inter.happened = true; // intersection happened
    inter.tnear = t; // time from the ray origin to the intersection point
    inter.coords = ray(t); // intersection point
    inter.normal = this->normal; // normal at the intersection point
    inter.material = this->m; // material of the triangle
    inter.obj = this; // object that was hit
    inter.tcoords.x = b1; // barycentric coordinate u
    inter.tcoords.y = b2; // barycentric coordinate v
    return inter;
}


inline Vector3f Triangle::evalDiffuseColor(const Vector2f& st) const
{
    if(!m->textured)return m->getColor(); // if not textured, return color

    // t0, t1, t2 are the texture coordinates of the triangle
    // st is the barycentric coordinates of the point
    auto uv = t0 * (1 - st.x - st.y) + t1 * st.x + t2 * st.y;
    float scale = 5;

    // Decide whether that UV point lands on a red or yellow square
    // The pattern is a checkerboard pattern


    // What does this line do?
    // It takes the x and y coordinates of the UV point, multiplies them by the scale,
    // and takes the modulus with 1.0 to get a value between 0 and 1.
    // Then it checks if the value is greater than 0.5 and returns true or false.
    // The ^ operator is a bitwise XOR operator, which means that it will return true
    // if one of the values is true and the other is false.
    float pattern = (fmodf(uv.x * scale, 1) > 0.5) ^ (fmodf(uv.y * scale, 1) > 0.5);

    // What does this line do?
    // It takes the pattern value and uses it to interpolate between two colors.
    return lerp(Vector3f(0.815, 0.235, 0.031), Vector3f(0.937, 0.937, 0.231), pattern);

    // Example:
    // uv.x = 0.15, uv.y = 0.6
    // scale = 5
    // 0.15 * 5 = 0.75 -> fmod(0.75, 1) = 0.75
    // 0.6 * 5 = 3.0 -> fmod(3.0, 1) = 0.0
    // 0.75 > 0.5 = true
    // 0.0 > 0.5 = false
    // pattern = true ^ false = true
    // So the color will be lerp(0.815, 0.235, 1), the colour is yellow
    // If the pattern is false, the color will be lerp(0.937, 0.937, 0), the colour is red
}
