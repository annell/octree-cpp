[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

# octree-cpp
A Octree implementation in C++.

## What is Octree?
From [Wikipedia](https://en.wikipedia.org/wiki/Octree):

# To install
## Header only method
1. Copy `OctreeCpp.h` to your project and include header.

### Dependencies
- C++20

## To run test suite
1. Clone repo to your project.
2. Install dependencies.
3. Include the CMakeList in your cmake structure.
4. Build & run octree-cpp_test.

## To run example
1. Clone repo to your project with submodules recursively `git clone --recurse-submodules`
2. Install dependencies.
3. Include the CMakeList in your cmake structure.
4. Build & run octree-cpp_demo.

And see the following results:
![sphere-query-example.png](example%2Fsphere-query-example.png)

### Dependencies
- C++20
- GTest

### To install all dependencies using Conan [optional]
This library uses the PackageManager [Conan](https://conan.io) for its dependencies, and all dependencies can be found in `conantfile.txt`.
1. Install conan `pip3 install conan`
2. Go to the build folder that cmake generates.
3. Run `conan install ..` see [installing dependencies](https://docs.conan.io/en/1.7/using_packages/conanfile_txt.html)
