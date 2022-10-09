# CS-GY 6533 A / CS-UY 4533 - Interactive Computer Graphics - Fall 2022

# Assignment 2: Ray Tracer by Hao Wu(hw2280@nyu.edu)

### (1) Basic Setup: Camera and View Rays
![1_basic_setup](./images_for_README/1_basic_setup.jpg)

### (2) Spheres
![2_spheres](./images_for_README/2_spheres.jpg)

### (3) Shading

![3_shading](./images_for_README/3_shading.jpg)

### (4) Shadows

![4_shadows](./images_for_README/4_shadows.jpg)

### (5) Planes

![5_planes](./images_for_README/5_planes.jpg)

### (6) Reflections

![6_reflections](./images_for_README/6_reflections.jpg)

## Optional Tasks

This tasks are optional and worth 1.5% of the final grade.

### Refraction and Parallelization

Make it possible for sphere to be transparent and add refraction to your ray tracing pipeline. The spheres need to have a different refraction coefficient than the surrounding medium (which typically has a refraction coefficient of 1, meaning that we consider it to be a vacuum).

As discussed in the lecture, ray tracing can be accelerated straightforwardly by parallelizing the computation of the primary rays. You can use OpenMP to parallelize ``for`` loops by adding a simple ``#pragma`` to your code. Find out how that works (e.g., by looking at the tutorial slides "An Overview of OpenMP" by Ruud van der Pas on [www.openmp.org](https://www.openmp.org/resources/tutorials-articles/)) and parallelize your ray tracer. *Be careful: parallelization can induce side effects if the same variables are used in the for loop!* Look at the results: What performance gain would you expect? How much speedup do you actually achieve?

*Hint:* The provided CMakeLists.txt already adds OpenMP support to your project. If you use Windows/VisualStudio, it might be a good idea to switch to "Release" mode.