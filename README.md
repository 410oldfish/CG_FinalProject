# Computer Graphics Final Project

Members:

- Yuan Hong (u8010795@anu.edu.au)
- Haoting Chen (u7227871@anu.edu.au)
- Hongyue Yu (u8068619@anu.edu.au)

Welcome to the artifact repository for the paper "Toward the Photorealistic Rendering of Study Room Scenes through Monte Carlo Path Tracing with a Customised Lightweight BSDF and Disney BSDF"

In this project, we have implemented Monte Carlo Path Tracing with a customised lightweight BSDF and Disney BSDF.

Due to the structural limitation of the code framework of Assignment 4 of the Computer Graphics course of the Australian National University, the two BSDFs are implemented in two different frameworks. In both frameworks, all the core functions for Monte Carlo Path Tracing and BSDF are implemented on our own. Please refer to the README in a particular code framework for detail.


- Monte Carlo Path Tracing with a customised lightweight BSDF ([🔗](/CodeFramework/))
- Monte Carlo Path Tracing with Disney BSDF ([🔗](/Project/))

---

*The code framework for the Path Tracing pipeline equipped with our customised light weight BSDF was borrowed from Assignment 4 [🔗](https://wattlecourses.anu.edu.au/mod/folder/view.php?id=3416289) of the Computer Graphics course of the Australian National University.*

*The code framework for the Path Tracing pipeline equipped with Disney BSDF was borrowed from Assignment 0 [🔗](https://cseweb.ucsd.edu/~tzli/cse272/wi2024/homework0.pdf) and Assignment 1 [🔗](https://cseweb.ucsd.edu/~tzli/cse272/wi2024/homework1.pdf) of the Advanced Image Synthesis course of the University of California San Diego. When we obtained the code, there is NO implementation regarding any of the Assignment tasks.*


## Image Showcase

![/images/1024-256-0.9-181.png](/images/1024-256-0.9-181.png)

- Resolution: 1365 x 1024 (4:3)
- Samples Per Pixel: 256
- Average Max Depth: 10 (Russian Roulette = 0.9)
- Rendering Time (on M1 Max): ~3 Hours
- Rendering Method: Monte Carlo Path Tracing (MCPT) + Next Event Estimation (NEE)
- BSDF: the lightweight BSDF

---

![/images/512-4096-0.999-713.png](/images/512-4096-0.999-713.png)

- Resolution: 682 × 512 (4:3)
- Samples Per Pixel: 4096
- Average Max Depth: 1000 (Russian Roulette = 0.999)
- Rendering Time (on M1 Max): ~11 Hours
- Rendering Method: Purely Monte Carlo Path Tracing
- BSDF: the lightweight BSDF

*The purpose of rendering this image is to use it as a ground truth to show how well can MCPT + NEE be closer the pure MCPT result using a limit number of samples per pixel.*

![/images/disney_bsdf_1344x768.jpg](/images/disney_bsdf_1344x768.jpg)

- Resolution: 1344 × 768
- Samples Per Pixel: 256
- Rendering Time (on M1 Max): ~30min
- Rendering Method: Monte Carlo Path Tracing
- BSDF: Disney BSDF

---
