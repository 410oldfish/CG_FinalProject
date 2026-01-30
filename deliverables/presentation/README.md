## Script (Partial)

(1) Hello, everyone. Today’s presentation will introduce our project, rendering indoor photorealistic images by building a customised C++ Monte Carlo path tracing pipeline.

(3) The main motivation for our project comes from this beautiful winning image in a rendering competition. The image is successful regarding the mirror effect on the iMac screen and soft shadow. However, there is also room for improvement, such as the lack of shadow and shading on the piano keyboard, and the lack of a transparent effect on the water glass and window. Our goal is to make it better. 

(5) To achieve our goal, we first built our pipeline based on Monte Carlo Path tracing with a basic Bidirectional Scattering Distribution function (BSDF) and then extended this BSDF to the more advanced Disney BSDF. 

(6) Physically based Ray tracing is a technique for tracing a ray backward from the camera point, image plane, all the hit points, and back to its original light source to determine its final colour on the image plane. It can easily simulate many physical effects that are considered very difficult in rasterisation, such as mirroring and global illumination. However, tracing a way natively by splitting it into two or multiple for further recursion at each hit point is inefficient and even impossible for Lambertian surfaces since an outgoing ray will be split into infinitely many incoming sub-rays. Therefore, we have implemented Monte Carlo Estimation at each splitting point, such as between reflection and refraction on a fully transparent surface or diffuse reflection, specular reflection, and absorption on a fully opaque surface. We roll a dice based on their probability distribution, only select one path for further recursion, but we later divide the result by the probability of choosing this event, which can lead to an unbiased average result.

In our original implementation of our BSDF, we idealised our object into only five parameters. This is enough for simulating some materials, such as plastic and frosted glass, but it struggles to handle many others, such as metal. However, we have laid a solid pipeline foundation when building our basic BSDF; that is, all of these parameters are not just numbers but can be defined either through a matrix or a function. In this way, our pipeline supports different materials on the sample mesh.

(7) Here is the resultant image using our basic BSDF, many new features are found, such as the implicit texture definition for the floor and mirror reflection almost everywhere. 

We are also working on inviting a person into our rendered room. The challenge only comes from the post and bone mapping, but most characters are not manifold meshes. For example, their hair may be made of planes, requiring our ray tracing algorithm to determine whether it is inside or outside a mesh smartly. 

We have found that adjusting light is quite a subtle task on physically based ray tracing because we no longer have a simple ambient light. We have to trap our light in an enclosed space, otherwise it can escape into the dark space. 

We have also experienced a bug but also considered as a physical phenomenon of not seeing anything outside, even though we have placed some light and a beautiful sky image. This is why people from outside cannot see what we are doing on a sunny day.

(8) To support more material, we have extended our original BSDF into Disney BSDF
