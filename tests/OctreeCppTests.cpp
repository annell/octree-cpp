//
// Created by Stefan Annell on 2023-05-20.
//

#include "OctreeCpp.h"
#include <gtest/gtest.h>
#include <random>

struct vec {
    float x, y, z;
};

TEST(OctreeCppTest, VectorLikeConcept) {
    static_assert(VectorLike<vec>);
    static_assert(not VectorLike<float>);
}

TEST(OctreeCppTest, BoundaryConcept) {
    static_assert(std::is_constructible_v<Boundary<vec>>);
    [[maybe_unused]] Boundary<vec> boundary({{0, 0, 0}, {1, 1, 1}});
}

TEST(OctreeCppTest, DataWrapperConcept) {
    static_assert(std::is_constructible_v<DataWrapper<vec, float>>);
    [[maybe_unused]] DataWrapper<vec, float> data;
}

TEST(OctreeCppTest, IsQueryConcept) {
    static_assert(IsQuery<QueryRadius<vec>, vec>);
    static_assert(not IsQuery<float, vec>);
    static_assert(not IsQuery<QueryRadius<vec>, float>);
    [[maybe_unused]] QueryRadius<vec> query;
}

TEST(OctreeCppTest, OctreeConstructible) {
    static_assert(std::is_constructible_v<OctreeCpp<vec, float>, Boundary<vec>>);
    [[maybe_unused]] OctreeCpp<vec, float> octree({});
}

TEST(OctreeCppTest, OctreeAdd) {
    OctreeCpp<vec, float> octree({{0, 0, 0}, {1, 1, 1}});

    DataWrapper<vec, float> data = {{0.5f, 0.5f, 0.5f}, 1.0f};
    octree.Add(data);
    EXPECT_EQ(octree.Size(), 1);
}

TEST(OctreeCppTest, OctreeAddOutsideX) {
    OctreeCpp<vec, float> octree({{0, 0, 0}, {1, 1, 1}});

    DataWrapper<vec, float> data = {{1.5f, 0.5f, 0.5f}, 1.0f};
    EXPECT_THROW(octree.Add(data), std::runtime_error);
    EXPECT_EQ(octree.Size(), 0);
}

TEST(OctreeCppTest, OctreeAddOutsideY) {
    OctreeCpp<vec, float> octree({{0, 0, 0}, {1, 1, 1}});

    DataWrapper<vec, float> data = {{0.5f, 1.5f, 0.5f}, 1.0f};
    EXPECT_THROW(octree.Add(data), std::runtime_error);
    EXPECT_EQ(octree.Size(), 0);
}

TEST(OctreeCppTest, OctreeAddOutsideZ) {
    OctreeCpp<vec, float> octree({{0, 0, 0}, {1, 1, 1}});

    DataWrapper<vec, float> data = {{0.5f, 0.5f, 1.5f}, 1.0f};
    EXPECT_THROW(octree.Add(data), std::runtime_error);
    EXPECT_EQ(octree.Size(), 0);
}

TEST(OctreeCppTest, OctreeAddMany) {
    OctreeCpp<vec, float> octree({{0, 0, 0}, {1, 1, 1}});

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    for (int i = 0; i < 100; i++) {
        DataWrapper<vec, float> data = {{dis(gen), dis(gen), dis(gen)}, 1.0f};
        octree.Add(data);
    }
    DataWrapper<vec, float> data = {{0.5f, 0.5f, 1.5f}, 1.0f};
    EXPECT_THROW(octree.Add(data), std::runtime_error);
    EXPECT_EQ(octree.Size(), 100);
}

TEST(OctreeCppTest, OctreeQueryCircleEmpty) {
    OctreeCpp<vec, float> octree({{0, 0, 0}, {1, 1, 1}});
    QueryRadius<vec> query = {{0.5f, 0.5f, 0.5f}, 0.5f};
    auto result = octree.Query(query);
    EXPECT_EQ(result.size(), 0);
}

TEST(OctreeCppTest, OctreeQueryCircleHit) {
    OctreeCpp<vec, float> octree({{0, 0, 0}, {1, 1, 1}});
    EXPECT_EQ(octree.Query(QueryRadius<vec>{{0.5f, 0.5f, 0.5f}, 0.5f}).size(), 0);

    octree.Add(DataWrapper<vec, float>{{0.5f, 0.5f, 0.5f}, 1.0f});
    EXPECT_EQ(octree.Query(QueryRadius<vec>{{0.5f, 0.5f, 0.5f}, 0.5f}).size(), 1);
}

TEST(OctreeCppTest, OctreeQueryCircleMiss) {
    OctreeCpp<vec, float> octree({{0, 0, 0}, {1, 1, 1}});
    EXPECT_EQ(octree.Query(QueryRadius<vec>{{0.5f, 0.5f, 0.5f}, 0.5f}).size(), 0);

    octree.Add(DataWrapper<vec, float>{{0.0f, 0.0f, 0.0f}, 1.0f});
    octree.Add(DataWrapper<vec, float>{{1.0f, 0.0f, 0.0f}, 1.0f});
    octree.Add(DataWrapper<vec, float>{{0.0f, 1.0f, 0.0f}, 1.0f});
    octree.Add(DataWrapper<vec, float>{{0.0f, 0.0f, 1.0f}, 1.0f});
    octree.Add(DataWrapper<vec, float>{{1.0f, 1.0f, 0.0f}, 1.0f});
    octree.Add(DataWrapper<vec, float>{{0.0f, 1.0f, 1.0f}, 1.0f});
    octree.Add(DataWrapper<vec, float>{{1.0f, 1.0f, 1.0f}, 1.0f});
    octree.Add(DataWrapper<vec, float>{{1.0f, 0.0f, 1.0f}, 1.0f});
    EXPECT_EQ(octree.Query(QueryRadius<vec>{{0.5f, 0.5f, 0.5f}, 0.5f}).size(), 0);

    EXPECT_EQ(octree.Query(QueryRadius<vec>{{0.5f, 0.5f, 0.5f}, 1.5f}).size(), 8);
}

TEST(OctreeCppTest, OctreeQueryOutsideBoundary) {
    OctreeCpp<vec, float> octree({{0, 0, 0}, {1, 1, 1}});
    EXPECT_EQ(octree.Query(QueryRadius<vec>{{0.5f, 0.5f, 0.5f}, 0.5f}).size(), 0);

    octree.Add(DataWrapper<vec, float>{{0.0f, 0.0f, 0.0f}, 1.0f});
    octree.Add(DataWrapper<vec, float>{{1.0f, 0.0f, 0.0f}, 1.0f});
    octree.Add(DataWrapper<vec, float>{{0.0f, 1.0f, 0.0f}, 1.0f});
    octree.Add(DataWrapper<vec, float>{{0.0f, 0.0f, 1.0f}, 1.0f});
    octree.Add(DataWrapper<vec, float>{{1.0f, 1.0f, 0.0f}, 1.0f});
    octree.Add(DataWrapper<vec, float>{{0.0f, 1.0f, 1.0f}, 1.0f});
    octree.Add(DataWrapper<vec, float>{{1.0f, 1.0f, 1.0f}, 1.0f});
    octree.Add(DataWrapper<vec, float>{{1.0f, 0.0f, 1.0f}, 1.0f});
    EXPECT_EQ(octree.Query(QueryRadius<vec>{{0.0f, 0.0f, -0.1f}, 0.5f}).size(), 1);
}

TEST(OctreeCppTest, OctreeQueryRand) {
    OctreeCpp<vec, float> octree({{0, 0, 0}, {1, 1, 1}});
    EXPECT_EQ(octree.Query(QueryRadius<vec>{{0.5f, 0.5f, 0.5f}, 0.5f}).size(), 0);

    octree.Add(DataWrapper<vec, float>{{0.0f, 0.0f, 0.0f}, 1.0f});
    octree.Add(DataWrapper<vec, float>{{1.0f, 0.0f, 0.0f}, 1.0f});
    octree.Add(DataWrapper<vec, float>{{0.0f, 1.0f, 0.0f}, 1.0f});
    octree.Add(DataWrapper<vec, float>{{0.0f, 0.0f, 1.0f}, 1.0f});
    octree.Add(DataWrapper<vec, float>{{1.0f, 1.0f, 0.0f}, 1.0f});
    octree.Add(DataWrapper<vec, float>{{0.0f, 1.0f, 1.0f}, 1.0f});
    octree.Add(DataWrapper<vec, float>{{1.0f, 1.0f, 1.0f}, 1.0f});
    octree.Add(DataWrapper<vec, float>{{1.0f, 0.0f, 1.0f}, 1.0f});
    EXPECT_EQ(octree.Query(QueryRadius<vec>{{0.5f, 0.5f, 0.5f}, 0.9f}).size(), 8);
}

TEST(OctreeCppTest, OctreeQueryMany) {
    OctreeCpp<vec, float> octree({{0, 0, 0}, {1, 1, 1}});
    octree.Add(DataWrapper<vec, float>{{0.5f, 0.5f, 0.5f}, 1.0f});

    for (int i = 0; i < 1000; i++) {
        octree.Add(DataWrapper<vec, float>{{0.0f, 0.0f, 0.0f}, 1.0f});
        octree.Add(DataWrapper<vec, float>{{1.0f, 0.0f, 0.0f}, 1.0f});
        octree.Add(DataWrapper<vec, float>{{0.0f, 1.0f, 0.0f}, 1.0f});
        octree.Add(DataWrapper<vec, float>{{0.0f, 0.0f, 1.0f}, 1.0f});
        octree.Add(DataWrapper<vec, float>{{1.0f, 1.0f, 0.0f}, 1.0f});
        octree.Add(DataWrapper<vec, float>{{0.0f, 1.0f, 1.0f}, 1.0f});
        octree.Add(DataWrapper<vec, float>{{1.0f, 1.0f, 1.0f}, 1.0f});
        octree.Add(DataWrapper<vec, float>{{1.0f, 0.0f, 1.0f}, 1.0f});
    }

    EXPECT_EQ(octree.Query(QueryRadius<vec>{{0.5f, 0.5f, 0.5f}, 0.1f}).size(), 1);
    EXPECT_EQ(octree.Size(), 8001);
    EXPECT_EQ(octree.Query(QueryRadius<vec>{{0.0f, 0.0f, 0.0f}, 0.1f}).size(), 1000);
}
