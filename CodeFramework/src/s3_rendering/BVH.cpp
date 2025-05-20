#include <algorithm>
#include <cassert>
#include "BVH.hpp"
#include<float.h>

// BVHAccel means Bounding Volume Hierarchy Accelerator
// Constructor for BVHAccel
BVHAccel::BVHAccel(std::vector<std::unique_ptr<Object>> p,
                        int maxPrimsInNode,
                   SplitMethod splitMethod)
    : maxPrimsInNode(std::min(255, maxPrimsInNode)), // input is clamped to 255
        splitMethod(splitMethod),
      primitives(std::move(p)) // move the input vector to the member variable.
      // What does "move" do?
      // It transfers the ownership of the vector to the member variable, avoiding a deep copy.
      // Result: the input vector is now empty and cannot be used anymore.
{
    time_t start, stop;
    time(&start);
    if (primitives.empty())
        throw std::runtime_error("No primitives to build BVH from.");
        return;

    // Call the helper and get the root node assigned
    root = recursiveBuild(std::move(primitives), 0);

    // Record the time
    time(&stop);
    double diff = difftime(stop, start);
    int hrs = (int)diff / 3600;
    int mins = ((int)diff / 60) - (hrs * 60);
    int secs = (int)diff - (hrs * 3600) - (mins * 60);

    printf(
        "\rBVH Generation complete: \nTime Taken: %i hrs, %i mins, %i secs\n\n",
        hrs, mins, secs);
}



// @param: objects: vector of pointers to objects
// @param: dim: dimension to split along, rotating between x/y/z
// @return: pointer to the root of the BVH tree
// BVHBuildNode* BVHAccel::recursiveBuild(std::vector<Object*> objects, int dim)
// BVHBuildNode* BVHAccel::recursiveBuild(std::vector<std::unique_ptr<Object>> objects, int dim)
std::unique_ptr<BVHBuildNode> BVHAccel::recursiveBuild(std::vector<std::unique_ptr<Object>> objects, int dim)

    // Create a new BVHBuildNode
{

    std::unique_ptr<BVHBuildNode> node = std::make_unique<BVHBuildNode>();

    // if(TASK_N<=1) { // we are not using BVH in task 1
    //     // not building an actual BVH tree, just register the objects
    //     node->registerObjects(objects);
    //     return node; // our tree will just have one node
    // }


    // TODO: task 2 BVH algorithm starts here
    Bounds3 bounds;

    // Get the largest bounding box containing all the current objects
    for (int i = 0; i < objects.size(); ++i)
        bounds = Union(bounds, objects[i]->getBounds());

    // Construct a left node if only one object is present
    if (objects.size() == 1) {
        node->bounds = objects[0]->getBounds();
        node->area = objects[0]->getArea();
        node->object = std::move(objects[0]); // move the object to the node
        node->left = nullptr;
        node->right = nullptr;
        return node;
    }

    // Construct a left and right node if two objects are present
    // Set area and bounds
    // This is a non-leaf node
    else if (objects.size() == 2) {
        std::vector<std::unique_ptr<Object>> leftshapes;
        std::vector<std::unique_ptr<Object>> rightshapes;
        leftshapes.push_back(std::move(objects[0]));
        rightshapes.push_back(std::move(objects[1]));

        node->left = recursiveBuild(std::move(leftshapes), dim);
        node->right = recursiveBuild(std::move(rightshapes), dim);

        node->bounds = Union(node->left->bounds, node->right->bounds);
        node->area = node->left->area + node->right->area;
        return node;
    }
    // Sort all object in the curent working dimension
    else {
        Bounds3 centroidBounds;
        switch (dim%3) {
            case 0:
                std::sort(objects.begin(), objects.end(), [](const std::unique_ptr<Object>& f1, const std::unique_ptr<Object>& f2) {
                    return f1->getBounds().Centroid().x <
                           f2->getBounds().Centroid().x;
                });
                break;
            case 1:
                std::sort(objects.begin(), objects.end(), [](const std::unique_ptr<Object>& f1, const std::unique_ptr<Object>& f2) {
                    return f1->getBounds().Centroid().y <
                           f2->getBounds().Centroid().y;
                });
                break;
            case 2:
                std::sort(objects.begin(), objects.end(), [](const std::unique_ptr<Object>& f1, const std::unique_ptr<Object>& f2) {
                    return f1->getBounds().Centroid().z <
                           f2->getBounds().Centroid().z;
                });
                break;
        }

        // Get the 
        auto beginning = objects.begin();
        auto middling = objects.begin() + (objects.size() / 2); // move the iterator forward by half
        auto ending = objects.end();

        // auto leftshapes = std::vector<Object*>(beginning, middling);
        // auto rightshapes = std::vector<Object*>(middling, ending);

        std::vector<std::unique_ptr<Object>> leftshapes;
        std::vector<std::unique_ptr<Object>> rightshapes;

        leftshapes.reserve(objects.size() / 2);  // avoid reallocations
        rightshapes.reserve(objects.size() - objects.size() / 2);

        // Move left half
        for (auto it = objects.begin(); it < middling; ++it)
            leftshapes.push_back(std::move(*it));

        // Move right half
        for (auto it = middling; it < objects.end(); ++it)
            rightshapes.push_back(std::move(*it));


        assert(objects.size() == (leftshapes.size() + rightshapes.size()));

        node->left = recursiveBuild(std::move(leftshapes), dim+1);
        node->right = recursiveBuild(std::move(rightshapes), dim+1);

        node->bounds = Union(node->left->bounds, node->right->bounds);
        node->area = node->left->area + node->right->area;
    }
    return node;
}

// @param ray: ray to be tested
// @return: the closest intersection where the ray hits an object or empty.
Intersection BVHAccel::Intersect(const Ray& ray) const
{
    Intersection isect;
    if (!root) // You must have a root node first
        return isect;
    isect = BVHAccel::getIntersection(root.get(), ray); // recursively traverse the function from the root node.
    return isect;
}

// Complexity: O(log(n)) for balanced trees, O(n) for unbalanced trees
Intersection BVHAccel::getIntersection(BVHBuildNode* node, const Ray& ray) const
{
    // if(TASK_N<=1) {  // we are not using BVH in task 1
    //     // loop through all objects saved earlier
    //     std::vector<Object *> objects = node->getRegisteredObjects();
    //     Intersection first_isect;
    //     for (int i = 0; i < objects.size(); i++) {
    //         // Try to get an intersection point between the ray and the object
    //         Intersection isect = objects[i]->getIntersection(ray);
    //         // If the intersection happened, check if it is the first one or if it is the closest one
    //         if (isect.happened && (!first_isect.happened || (first_isect.tnear > isect.tnear))) {
    //             first_isect = isect; // update the first intersection
    //         }
    //     }
    //     return first_isect;
    // }

    // TODO: task 2 BVH algorithm starts here
    Intersection inter;
    // Precompute the inverse of the ray direction and avoid division by zero
    Vector3f indiv(1.0f/(ray.direction[0]==0?0.00001:ray.direction[0]),
                   1.0f/(ray.direction[1]==0?0.00001:ray.direction[1]),
                   1.0f/(ray.direction[2]==0?0.00001:ray.direction[2]));
    
    // Store whether each component of the ray is negative
    std::array<int, 3> dirIsNeg;
    dirIsNeg[0]=int(ray.direction.x>0);
    dirIsNeg[1]=int(ray.direction.y>0);
    dirIsNeg[2]=int(ray.direction.z>0);

    // If the ray does not intersect with the current node's bounding box, return empty intersection
    // We will not even look at its children
    if (!node->bounds.IntersectP(ray, indiv, dirIsNeg))    return inter;

    // If the node is a leaf, get the intersection with the object and return it
    if (node->left==nullptr && node->right==nullptr)
    {
        inter=node->object->getIntersection(ray);
        return inter;
    }

    // Compute the intersection points of the left and right children
    Intersection left=getIntersection(node->left.get(), ray);
    Intersection right=getIntersection(node->right.get(), ray);

    // Return the closest intersection point
    return left.tnear < right.tnear ? left : right;
}




// Randomly sample a point on an emissive object in the scene, proportional to surface area.
// Importance: used for path tracing, where we want to generate light samples more likely to affect the scene.


// NOTE: this BVH built over only the emissive objects in the scene.

// @param node: node containing a emissive object
// @param p: a random number between 0 and surface area of the object
// @param pos (output): the sampled position on a light's surface
// @param pdf (output): unnormalised PDF
// void BVHAccel::getSample(BVHBuildNode* node, float p, Intersection &pos, float &pdf){
void BVHAccel::getSample(std::unique_ptr<BVHBuildNode>& node, float p, Intersection &pos, float &pdf){

    // If the node is a leaf, sample directly on the object and return
    if(node->left == nullptr || node->right == nullptr){
        node->object->Sample(pos, pdf); // pdf here is probability of sampling a point on the current object, e.g., 1/area
        pdf *= node->area; 
        return;
    }
    // Make sure a larger area get sampled more often
    if(p < node->left->area) 
        getSample(node->left, p, pos, pdf);
    else 
        getSample(node->right, p - node->left->area, pos, pdf);
}

// Start the area weighted sampling process
// @param pos: the sampled position on a light's surface
// @param pdf: PDF of sampling this point.
void BVHAccel::Sample(Intersection &pos, float &pdf){
    float p = std::sqrt(get_random_float()) * root->area;
    getSample(root, p, pos, pdf);
    pdf /= root->area;
}
