[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

# octree-cpp
A Octree implementation in C++ built to work with your own vector class and any generic data blob stored along side its position in the world.
The queries are very easily extendable via templates if there is some specific usecase that you want to query on.

The usecase for this octree is to be able to, quickly do complex queries in 3D space.

## What is Octree?
> An octree is a tree data structure in which each internal node has exactly eight children. Octrees are most often used to partition a three-dimensional space by recursively subdividing it into eight octants. Octrees are the three-dimensional analog of quadtrees. The word is derived from oct (Greek root meaning "eight") + tree. Octrees are often used in 3D graphics and 3D game engines.

[- Wikipedia](https://en.wikipedia.org/wiki/Octree)

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
## Header only method
1. Copy `OctreeCpp.h` to your project and include header.

## Subrepo method
1. Clone repo with `git clone --recurse-submodules`
2. Include `octree-cpp` folder in the CMakeList file with: `add_subdirectory("path/to/octree-cpp")`

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

