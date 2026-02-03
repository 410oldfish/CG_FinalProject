## Abstract


**Title**: Toward the Photorealistic Rendering of Study Room Scenes through Monte Carlo Path Tracing with a Customised Lightweight BSDF and Disney BSDF

**Keywords**: Photorealistic Rendering, Ray Tracing, Monte Carlo Path Tracing

**Type**: Team Project

**Collaborator**: [Yuan Hong](https://github.com/HongYuan6139), [Hongyue Yu](https://github.com/410oldfish)

**Duration**: April 2025 – May 2025

**Course Name**: [COMP8610](https://programsandcourses.anu.edu.au/2025/course/COMP8610) – Computer Graphics

**Course Outline**: 

- Students in this course were required to either conduct computer graphics research projects or participate in a photorealistic rendering competition in a group. For the competition, the group were required to creatively design a 3D scene, autonomously choose a combination of 3D rendering techniques, and implement them from scratch.


**Course Final Mark**: 81 / 100

**Project Weight**: [30%](https://1drv.ms/f/c/4f49bb445ba8ff14/IgAYYjrNIBt0SpZ_SL9pQ9pOATAFfnwSbclO-syJrqqytos) of COMP8610

**Project Mark**: N/A

**Deliverables**: [paper](https://github.com/glowing-sea/disni-ray-tracing-pipeline/blob/main/deliverables/paper.pdf), [code](https://github.com/glowing-sea/disni-ray-tracing-pipeline), [presentation](https://github.com/glowing-sea/disni-ray-tracing-pipeline/tree/main/deliverables/presentation)

**Description**:

- This project has implemented a Disney Monte Carlo Path Tracing-based 3D rendering pipeline from scratch using only the standard C++ libraries.
- The outcome is valuable for educational demonstrations that explicitly reveal the underlying physical and algorithmic principles of 3D ray tracing rendering, since many open-source 3D rendering projects are built on high-level frameworks, such as OpenGL.
- The pipeline is evaluated in two indoor study-room scene rendering settings, with objects featuring materials such as plastic, wood, cement, mirror, glass, and skin.
- Results show that although the pipeline can generate an overall photorealistic image from an input 3D scene model, challenges remain, such as fine-tuning material parameters and addressing violations of energy conservation in some edge cases.
- In addition to refining the current rendering logic, one major future work is to support GPU rendering to significantly improve computational efficiency.

**About Collaboration**:

- Haoting Chen contributed about 33% to the project. Haoting Chen’s main roles include writing the majority of the paper's sections and implementing the code for the customised Monte Carlo ray tracing pipeline. Yuan Hong extended the code to support Disney BSDF rendering. Hongyu Yu implemented an input interface for OBJ files to generate C++ triangular instances.