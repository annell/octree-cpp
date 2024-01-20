[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](https://opensource.org/licenses/MIT)
![GitHub Actions Workflow Status](https://img.shields.io/github/actions/workflow/status/annell/octree-cpp/main.yml)

# octree-cpp
A Octree implementation in C++ built to work with your own vector class and any generic data blob stored along side its position in the world.
The queries are very easily extendable via templates if there is some specific usecase that you want to query on.

The usecase for this octree is to be able to, quickly do complex queries in 2D or 3D space.
(Technically a octree is a 3D data structure, but it can be used in 2D space as well and it will at compile time convert to a quadtree.)

## What is Octree?
> An octree is a tree data structure in which each internal node has exactly eight children. Octrees are most often used to partition a three-dimensional space by recursively subdividing it into eight octants. Octrees are the three-dimensional analog of quadtrees. The word is derived from oct (Greek root meaning "eight") + tree. Octrees are often used in 3D graphics and 3D game engines.

[- Wikipedia](https://en.wikipedia.org/wiki/Octree)

## Features
- Bring your own vector class, no need to adapt to a specific vector class defined in this library.
- Header only implementation.
- Simple cmake library integration.
- Possible to use both at 2D or 3D space, and its automatically set at compile time, transforms between a Octree in 3d space and a Quadtree in 2d space depending on the provided vector.
- Possible to use any generic data blob as payload.
- Possible to extend the queries with your own custom queries, only need to satisfy the IsQuery concept.
- Queries can be combined with AND, OR, NOT and Predicate to build up more complex shapes.
- Very quickly builds up a new tree when the world changes.
- Extensive unit testing of library.

## How to use
The octree is very light weight to use, the basic usecase is:
1. Setup the octree with your vector class and the payload that you want to use.
```c++
using Octree = OctreeCpp<vec, float>;
Octree octree({{0, 0, 0}, {1, 1, 1}});
```
Here the vector class is called vec and the payload is a float, and it can be whetever you want.
The min / max values for this octree is set to be [0, 0, 0] -> [1, 1, 1]

2. Add data to the octree.
```c++
octree.Add({.Vector{0.5f, 0.5f, 0.5f}, .Data{1.0f}});
```
The payload is copied to the octree by default.

3. Query the octree for _hits_.
```c++
auto hits = octree.Query(Octree::Sphere{{0.5f, 0.5f, 0.5f}, 0.5f});
```
Query returns a list of all objects that was within the given query.

### Basic query
A simple usage using a query to find all data points within the given area.

```C++
// Create the octree with its boundry
// First parameter is vector type, second parameter is payload
using Octree = OctreeCpp<vec, float>;
Octree octree({{0, 0, 0}, {1, 1, 1}});
auto hits = octree.Query(Octree::All());
EXPECT_EQ(hits.size(), 0);

// Add data to the octree
octree.Add({.Vector{0.5f, 0.5f, 0.5f}, .Data{1.0f}});
hits = octree.Query(Octree::All());
EXPECT_EQ(hits.size(), 1);

// Query the octree using the SphereQuery
hits = octree.Query(Octree::Sphere{{0.5f, 0.5f, 0.5f}, 0.5f});
EXPECT_EQ(hits.size(), 1)
```
### Compund query
You can use basic shapes and then also build up more complex queries by compound them with AND, OR, NOT and Predicate to do much more interesting queries while still only doing one pass.
````c++
// Basic Sphere query at 0, 0, 0 with 50 radius
auto midQuery = Octree::Sphere{{0, 0, 0}, 50.0f};

// A sphere query but Not'ed
auto notQuery = Octree::Not<Octree::Sphere>{{25, 25, 25}, 10};

// Two queries, one Not'ed, combined with AND 
auto result = octree.Query(Octree::And<Octree::Sphere, Octree::Not<Octree::Sphere>>{midQuery, notQuery});
````

# To install
## CMake method
1. Clone octree-cpp to your project `git clone --recurse-submodules`.
2. Add `add_subdirectory(path/octree-cpp)` to your CMakeLists.txt.
3. Link your project to `octree-cpp`.
4. Include `#include <octree-cpp/OctreeCpp.h>` in your project.

### Dependencies
- C++20

### Example on integration
Here you can find a example on how to integrate this library into your project using CMake: [ecs-cpp-example](https://github.com/annell/physim-cpp).
In this example, the library is placed under thirdparty/octree-cpp.

## Dependencies
- C++20
- GTest

### To install all dependencies using Conan [optional]
This library uses the PackageManager [Conan](https://conan.io) for its dependencies, and all dependencies can be found in `conantfile.txt`.
1. Install conan `pip3 install conan`
2. Go to the build folder that cmake generates.
3. Run `conan install ..` see [installing dependencies](https://docs.conan.io/en/1.7/using_packages/conanfile_txt.html)

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

### Basic sphere query.
![sphere-query-example.png](example%2Fsphere-query-example.png)

### A compound query with more complex geometry.
![sphere-query-example.png](example%2Fquery-sphere-or-not-example.png)

