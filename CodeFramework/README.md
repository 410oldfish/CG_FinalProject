# Monte Carlo Path Tracing with a Lightweight BSDF


## 1. How to Run?

- Make sure you are in the `build` folder using `cd build`
- `make`
- `./cgfinal [Image Height, default=256] [SPP, default=32]`

## 2. How to Adjust the Camera?

Please go to [`/src/s3_rendering/s3_rendering.hpp`](/src/s3_rendering/Renderer.cpp) and you will see the following constants.
```
const Vector3f eye_pos(-103.16, 314.232, -304.957);
const float x_rotate = 5.5988f; // Pitch
const float y_rotate = 33.5984f; // Yaw
const float z_rotate = 0.f; // Roll
```

## 3. How to Change other Parameters?

In [`/src/s2_modelling/Scene.cpp`](/src/s2_modelling/Scene.cpp)

- Use `RussianRoulette` to change the average max depth. The actual max depth for the function `cast_ray` is twice the number of the average max depth of `RussianRoulette`.

- `PURELY_MONTE_CARLO 1` disable Next Event Estimation (NEE) in Monte Carlo Path Tracing. This will lead to an extremely slow convergence speed.

- `SAMPLE_LIGHTS_BY_WEIGHTS 1` to sample a light by its weight, and then sample a point on the light source uniformly by area when using NEE. Otherwise, the program samples a point on the light source uniformly by area. Currently, there is a "fireflies" issue when it is turned on.

## 4. How to Adjust Objects?

Please go to [`/src/s2_modelling/s2_modelling.hpp`](/src/s2_modelling/s2_modelling.hpp) and you will see the process of creating materials, objects, and light instances.

If you want to load an `.obj` object into the scene, please place the `.obj` and its material file `.mtl` into [`/models/`](/models/) and create an instance of `std::unique_ptr<MeshTriangle>` to load the `.obj` object. You need to specify the file path, a default material (if a `.obj` file has a corresponding `.mtl` file. This will not be used and a new material instance will be created when reading the `.mtl` file), a pointer list tracking all the created material instance in the heap (for garbage collection), and a pointer list tracking all the opened image matrix in the heap.

If you want to load an `.obj` light source, the same approach applies. Additionally, you need to push the light pointer to the list so that the program can easily identify which objects are lights. You also need to assign a weight for the light when it is sampled. This can be done by `scene.light_sources.push_back(lightUp.get());` and `scene.light_source_weights.push_back(1.0f);`.

## 5. How to Read the Code?

- Start with [`/src/main.cpp`](/src/main.cpp)
- Then follow the pipeline stage step by step

The important code you may want to read first.

- [`/src/s1_preprocessing/Material.hpp`](/src/s1_preprocessing/Material.hpp): Defines that a material can have 5 parameters, each of them can be either a function or a matrix.
- [`/src/s1_preprocessing/Triangle.hpp`](/src/s1_preprocessing/Triangle.hpp): Defines how an obj file containing multiple materials are loaded as a single mesh.
- [`/src/s2_modelling/Scene.cpp`](/src/s2_modelling/Scene.cpp): The main implementation of Monte Carlo Path Tracing, Next Event Estimation, and our Customised Lightweight BSDF.
- [`/src/s3_rendering/Renderer.cpp`](/src/s3_rendering/Renderer.cpp): Defines the camera. Transforms from the camera space to the world space. Loops through the pixels.

Other improvements from the original Assignment 4 framework includeschanging all raw pointers into unique pointers (otherwise use a list to store any pointers to heap memory) to avoid memory leak and group the `.cpp` and `.hpp` files into 4 stages.

## 6. Credits

The links for the downloaded models are in the [Model Attributions.txt](/Model%20Attributions.txt) files.