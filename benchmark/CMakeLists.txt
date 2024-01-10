find_package(benchmark REQUIRED)

add_executable(${PROJECT_NAME}_benchmark OctreeBenchmark.cpp)
target_link_libraries(${PROJECT_NAME}_benchmark benchmark::benchmark ${PROJECT_NAME})
