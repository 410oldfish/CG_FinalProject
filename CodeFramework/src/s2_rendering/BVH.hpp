//
// Created by LEI XU on 5/16/19.
//

#ifndef RAYTRACING_BVH_H
#define RAYTRACING_BVH_H

#include <atomic> // not used
#include <vector>
#include <memory> // not used
#include <ctime>
#include "Object.hpp"
#include "Ray.hpp"
#include "Bounds3.hpp"
#include "Intersection.hpp"
#include "Vector.hpp"

struct BVHBuildNode; // Node in the BVH tree
// BVHAccel Forward Declarations
struct BVHPrimitiveInfo; // Primitive information for BVH, possibly used for SAH(Surface Area Heuristic) splitting

// Global Counters
// totalPrimitives: total number of primitives in the BVH
// interiorNodes: total number of interior nodes during recursive build
// leafNodes: number of leaf nodes currently in the BVH
// totalLeafNodes: total number of leaf noes created during construction (same as leafNodes?)
inline int leafNodes, totalLeafNodes, totalPrimitives, interiorNodes;

// This class build and traverse a BVH (Bounding Volume Hierarchy) for ray tracing
class BVHAccel {

public:
    // BVHAccel Public Types
    // NAIVE: sort their centroids along the longest axis and split them in the middle
    // SAH: use Surface Area Heuristic to split the primitives
    enum class SplitMethod { NAIVE, SAH };

    // Constructor
    // p: vector of pointers to objects
    // maxPrimsInNode: maximum number of primitives in a node
    // splitMethod: method to split the primitives
    // @output: build the BVH tree and stores the root in root
    BVHAccel(std::vector<Object*> p, int maxPrimsInNode = 1, SplitMethod splitMethod = SplitMethod::NAIVE);

    // Probability defined elsewhere, it should return the bounding box of the whole scene
    Bounds3 WorldBound() const;

    // Destructor
    ~BVHAccel();

    // @param ray: ray to be tested
    // @return: the closest intersection where the ray hits an object or empty.
    Intersection Intersect(const Ray &ray) const;

    // Recursive helper: walks the tree adn checks bounding boxes before testing objects. Skips subtree that ray cannot reach.
    Intersection getIntersection(BVHBuildNode* node, const Ray& ray)const;

    // Not implemented — Does the ray hit anything at all?
    bool IntersectP(const Ray &ray) const;

    // Pointer to the root of the BVH tree
    BVHBuildNode* root;

    // @param: objects: vector of pointers to objects
    // @param: dim: dimension to split along, rotating between x/y/z
    // @return: pointer to the root of the BVH tree
    BVHBuildNode* recursiveBuild(std::vector<Object*>objects, int dim);

    // BVHAccel Private Data
    const int maxPrimsInNode; // max objects allowed in a leaf node
    const SplitMethod splitMethod; // how to divide objects during building
    std::vector<Object*> primitives; // stores all input scene objects

    // What does these functions do?


    void getSample(BVHBuildNode* node, float p, Intersection &pos, float &pdf);

    void Sample(Intersection &pos, float &pdf);
};


struct BVHBuildNode {
    Bounds3 bounds;
    BVHBuildNode *left;
    BVHBuildNode *right;
    Object* object; // the object if this is a leaf node
    float area; // area of all objects in this node
    std::vector<Object*> registered_objects; // only used in task 1, to store the objects in this node

public:
    // Unused placeholders (likely for SAH or more advanced build strategies)
    int splitAxis=0, firstPrimOffset=0, nPrimitives=0;

    // BVHBuildNode Public Methods
    // Default constructor for a BVHBuildNode
    BVHBuildNode(){
        bounds = Bounds3();
        left = nullptr;right = nullptr;
        object = nullptr;
        registered_objects = std::vector<Object*>();
    }
    // Only used in Task 1 when BVH is off. Stores all objects for brute-force intersection.
    void registerObjects(const std::vector<Object*>& objects){
        registered_objects = objects;
    }
    // Getter for those objects.
    std::vector<Object*> getRegisteredObjects() const {
        return registered_objects;
    }
};


#endif //RAYTRACING_BVH_H
