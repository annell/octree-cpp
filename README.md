[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

# octree-cpp
A Octree implementation in C++.

## What is Octree?
> An octree is a tree data structure in which each internal node has exactly eight children. Octrees are most often used to partition a three-dimensional space by recursively subdividing it into eight octants. Octrees are the three-dimensional analog of quadtrees. The word is derived from oct (Greek root meaning "eight") + tree. Octrees are often used in 3D graphics and 3D game engines.

[- Wikipedia](https://en.wikipedia.org/wiki/Octree)
## How to use
```C++
// Create the octree with its boundry
// First parameter is vector type, second parameter is payload
OctreeCpp<vec, float> octree({{0, 0, 0}, {1, 1, 1}});
auto hits = octree.Query(AllQuery<vec>());
EXPECT_EQ(hits.size(), 0);

// Add data to the octree
octree.Add({.Vector{0.5f, 0.5f, 0.5f}, .Data{1.0f}});
hits = octree.Query(AllQuery<vec>());
EXPECT_EQ(hits.size(), 1);

// Query the octree using the SphereQuery
hits = octree.Query(SphereQuery<vec>{{0.5f, 0.5f, 0.5f}, 0.5f});
EXPECT_EQ(hits.size(), 1)
```
# To install
## Header only method
1. Copy `OctreeCpp.h` to your project and include header.

## Subrepo method
1. Clone repo with `git clone --recurse-submodules`
2. Include `octree-cpp` folder in the CMakeList file with: `add_subdirectory("path/to/octree-cpp")`

# Dependencies
- C++20

# To run test suite
1. Clone repo to your project.
2. Install dependencies.
3. Include the CMakeList in your cmake structure.
4. Build & run octree-cpp_test.

# To run example
1. Clone repo to your project with submodules recursively `git clone --recurse-submodules`
2. Install dependencies.
3. Include the CMakeList in your cmake structure.
4. Build & run octree-cpp_demo.

And see the following results:
![sphere-query-example.png](example%2Fsphere-query-example.png)

![sphere-query-example.png](example%2Fquery-sphere-or-not-example.png)

### Dependencies
- C++20
- GTest

### To install all dependencies using Conan [optional]
This library uses the PackageManager [Conan](https://conan.io) for its dependencies, and all dependencies can be found in `conantfile.txt`.
1. Install conan `pip3 install conan`
2. Go to the build folder that cmake generates.
3. Run `conan install ..` see [installing dependencies](https://docs.conan.io/en/1.7/using_packages/conanfile_txt.html)
