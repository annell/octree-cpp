find_package(GTest REQUIRED)

enable_testing()

add_executable(${PROJECT_NAME}_test OctreeCppTests.cpp)
target_link_libraries(${PROJECT_NAME}_test GTest::gtest GTest::gtest_main ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME}_test PUBLIC ".")

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}_test)