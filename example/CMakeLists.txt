add_subdirectory("polyscope")

add_executable(${PROJECT_NAME}_demo demo.cpp)
target_link_libraries(${PROJECT_NAME}_demo octree-cpp polyscope)