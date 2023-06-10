//
// Created by Stefan Annell on 2023-05-20.
//

#include "src/OctreeCpp.h"
#include <gtest/gtest.h>
#include <random>

struct vec {
    float x, y, z;
    auto operator<=>(const vec&) const = default;
};

using BasicOctree = OctreeCpp<vec, float>;

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
    static_assert(IsQuery<SphereQuery<BasicOctree::TDataWrapper>, BasicOctree::TDataWrapper>);
    static_assert(not IsQuery<float, vec>);
    static_assert(not IsQuery<SphereQuery<DataWrapper<vec, float>>, float>);
    [[maybe_unused]] SphereQuery<BasicOctree::TDataWrapper> query;
}

TEST(OctreeCppTest, OctreeConstructible) {
    static_assert(std::is_constructible_v<OctreeCpp<vec, float>, Boundary<vec>>);
    [[maybe_unused]] BasicOctree octree({});
}

TEST(OctreeCppTest, OctreeAdd) {
    BasicOctree octree({{0, 0, 0}, {1, 1, 1}});

    BasicOctree::TDataWrapper data = {{0.5f, 0.5f, 0.5f}, 1.0f};
    octree.Add(data);
    EXPECT_EQ(octree.Size(), 1);
}

TEST(OctreeCppTest, OctreeAddOutsideX) {
    BasicOctree octree({{0, 0, 0}, {1, 1, 1}});

    BasicOctree::TDataWrapper data = {{1.5f, 0.5f, 0.5f}, 1.0f};
    EXPECT_THROW(octree.Add(data), std::runtime_error);
    EXPECT_EQ(octree.Size(), 0);
}

TEST(OctreeCppTest, OctreeAddOutsideY) {
    BasicOctree octree({{0, 0, 0}, {1, 1, 1}});

    BasicOctree::TDataWrapper data = {{0.5f, 1.5f, 0.5f}, 1.0f};
    EXPECT_THROW(octree.Add(data), std::runtime_error);
    EXPECT_EQ(octree.Size(), 0);
}

TEST(OctreeCppTest, OctreeAddOutsideZ) {
    BasicOctree octree({{0, 0, 0}, {1, 1, 1}});

    BasicOctree::TDataWrapper data = {{0.5f, 0.5f, 1.5f}, 1.0f};
    EXPECT_THROW(octree.Add(data), std::runtime_error);
    EXPECT_EQ(octree.Size(), 0);
}

TEST(OctreeCppTest, OctreeAddMany) {
    BasicOctree octree({{0, 0, 0}, {1, 1, 1}});

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    for (int i = 0; i < 100; i++) {
        BasicOctree::TDataWrapper data = {{dis(gen), dis(gen), dis(gen)}, 1.0f};
        octree.Add(data);
    }
    BasicOctree::TDataWrapper data = {{0.5f, 0.5f, 1.5f}, 1.0f};
    EXPECT_THROW(octree.Add(data), std::runtime_error);
    EXPECT_EQ(octree.Size(), 100);
}

TEST(OctreeCppTest, OctreeQueryCircleEmpty) {
    BasicOctree octree({{0, 0, 0}, {1, 1, 1}});
    SphereQuery<BasicOctree::TDataWrapper> query = {{0.5f, 0.5f, 0.5f}, 0.5f};
    auto result = octree.Query(query);
    EXPECT_EQ(result.size(), 0);
}

TEST(OctreeCppTest, OctreeQueryCircleHit) {
    BasicOctree octree({{0, 0, 0}, {1, 1, 1}});
    EXPECT_EQ(octree.Query(SphereQuery<BasicOctree::TDataWrapper>{{0.5f, 0.5f, 0.5f}, 0.5f}).size(), 0);

    octree.Add(BasicOctree::TDataWrapper{{0.5f, 0.5f, 0.5f}, 1.0f});
    EXPECT_EQ(octree.Query(SphereQuery<BasicOctree::TDataWrapper>{{0.5f, 0.5f, 0.5f}, 0.5f}).size(), 1);
}

TEST(OctreeCppTest, OctreeQueryCircleHitNegative) {
    BasicOctree octree({{-100, -100, -100}, {100, 100, 100}});
    EXPECT_EQ(octree.Query(SphereQuery<BasicOctree::TDataWrapper>{{0.5f, 0.5f, 0.5f}, 0.5f}).size(), 0);

    octree.Add(BasicOctree::TDataWrapper{{-20.0f, -20.5f, -10.5f}, 1.0f});
    octree.Add(BasicOctree::TDataWrapper{{-5.0f, -20.5f, -10.5f}, 1.0f});
    octree.Add(BasicOctree::TDataWrapper{{-5.0f, -5.5f, -5.5f}, 1.0f});
    EXPECT_EQ(octree.Query(SphereQuery<BasicOctree::TDataWrapper>{{-20.0f, -70.0f, -10.5f}, 50.0f}).size(), 1);
}

TEST(OctreeCppTest, OctreeQueryCircleHitNegativeFullOctree) {
    OctreeCpp<vec, int> octree({{-100, -100, -100}, {100, 100, 100}});

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-90.0f, 100.0f);
    for (int i = 0; i < 10000; i++) {
        DataWrapper<vec, int> data = {{dis(gen), dis(gen), dis(gen)}, i};
        octree.Add(data);
    }

    EXPECT_EQ(octree.Query(AllQuery<DataWrapper<vec, int>>{}).size(), 10000);

    octree.Add(DataWrapper<vec, int>{{-99.0f, -100.0f, -100.0f}, -1});
    octree.Add(DataWrapper<vec, int>{{-95.0f, -100.0f, -100.0f}, -2});
    octree.Add(DataWrapper<vec, int>{{-96.0f, -100.0f, -100.0f}, -3});
    octree.Add(DataWrapper<vec, int>{{-97.0f, -100.0f, -100.0f}, -4});
    EXPECT_EQ(octree.Query(SphereQuery<DataWrapper<vec, int>>{{-145.0f, -100.0f, -100.0f}, 50.0f}).size(), 4);
}

TEST(OctreeCppTest, OctreeQueryCircleMiss) {
    BasicOctree octree({{0, 0, 0}, {1, 1, 1}});
    EXPECT_EQ(octree.Query(SphereQuery<BasicOctree::TDataWrapper>{{0.5f, 0.5f, 0.5f}, 0.5f}).size(), 0);

    octree.Add(BasicOctree::TDataWrapper{{0.0f, 0.0f, 0.0f}, 1.0f});
    octree.Add(BasicOctree::TDataWrapper{{1.0f, 0.0f, 0.0f}, 1.0f});
    octree.Add(BasicOctree::TDataWrapper{{0.0f, 1.0f, 0.0f}, 1.0f});
    octree.Add(BasicOctree::TDataWrapper{{0.0f, 0.0f, 1.0f}, 1.0f});
    octree.Add(BasicOctree::TDataWrapper{{1.0f, 1.0f, 0.0f}, 1.0f});
    octree.Add(BasicOctree::TDataWrapper{{0.0f, 1.0f, 1.0f}, 1.0f});
    octree.Add(BasicOctree::TDataWrapper{{1.0f, 1.0f, 1.0f}, 1.0f});
    octree.Add(BasicOctree::TDataWrapper{{1.0f, 0.0f, 1.0f}, 1.0f});
    EXPECT_EQ(octree.Query(SphereQuery<BasicOctree::TDataWrapper>{{0.5f, 0.5f, 0.5f}, 0.5f}).size(), 0);

    EXPECT_EQ(octree.Query(SphereQuery<BasicOctree::TDataWrapper>{{0.5f, 0.5f, 0.5f}, 1.5f}).size(), 8);
}

TEST(OctreeCppTest, OctreeQueryOutsideBoundary) {
    BasicOctree octree({{0, 0, 0}, {1, 1, 1}});
    EXPECT_EQ(octree.Query(SphereQuery<BasicOctree::TDataWrapper>{{0.5f, 0.5f, 0.5f}, 0.5f}).size(), 0);

    octree.Add(BasicOctree::TDataWrapper{{0.0f, 0.0f, 0.0f}, 1.0f});
    octree.Add(BasicOctree::TDataWrapper{{1.0f, 0.0f, 0.0f}, 1.0f});
    octree.Add(BasicOctree::TDataWrapper{{0.0f, 1.0f, 0.0f}, 1.0f});
    octree.Add(BasicOctree::TDataWrapper{{0.0f, 0.0f, 1.0f}, 1.0f});
    octree.Add(BasicOctree::TDataWrapper{{1.0f, 1.0f, 0.0f}, 1.0f});
    octree.Add(BasicOctree::TDataWrapper{{0.0f, 1.0f, 1.0f}, 1.0f});
    octree.Add(BasicOctree::TDataWrapper{{1.0f, 1.0f, 1.0f}, 1.0f});
    octree.Add(BasicOctree::TDataWrapper{{1.0f, 0.0f, 1.0f}, 1.0f});
    EXPECT_EQ(octree.Query(SphereQuery<BasicOctree::TDataWrapper>{{0.0f, 0.0f, -0.1f}, 0.5f}).size(), 1);
}

TEST(OctreeCppTest, OctreeQueryRand) {
    BasicOctree octree({{0, 0, 0}, {1, 1, 1}});
    EXPECT_EQ(octree.Query(SphereQuery<BasicOctree::TDataWrapper>{{0.5f, 0.5f, 0.5f}, 0.5f}).size(), 0);

    octree.Add(BasicOctree::TDataWrapper{{0.0f, 0.0f, 0.0f}, 1.0f});
    octree.Add(BasicOctree::TDataWrapper{{1.0f, 0.0f, 0.0f}, 1.0f});
    octree.Add(BasicOctree::TDataWrapper{{0.0f, 1.0f, 0.0f}, 1.0f});
    octree.Add(BasicOctree::TDataWrapper{{0.0f, 0.0f, 1.0f}, 1.0f});
    octree.Add(BasicOctree::TDataWrapper{{1.0f, 1.0f, 0.0f}, 1.0f});
    octree.Add(BasicOctree::TDataWrapper{{0.0f, 1.0f, 1.0f}, 1.0f});
    octree.Add(BasicOctree::TDataWrapper{{1.0f, 1.0f, 1.0f}, 1.0f});
    octree.Add(BasicOctree::TDataWrapper{{1.0f, 0.0f, 1.0f}, 1.0f});
    EXPECT_EQ(octree.Query(SphereQuery<BasicOctree::TDataWrapper>{{0.5f, 0.5f, 0.5f}, 0.9f}).size(), 8);
}

TEST(OctreeCppTest, OctreeQueryMany) {
    BasicOctree octree({{0, 0, 0}, {1, 1, 1}});
    octree.Add(DataWrapper<vec, float>{{0.5f, 0.5f, 0.5f}, 1.0f});

    for (int i = 0; i < 1000; i++) {
        octree.Add(BasicOctree::TDataWrapper{{0.0f, 0.0f, 0.0f}, 1.0f});
        octree.Add(BasicOctree::TDataWrapper{{1.0f, 0.0f, 0.0f}, 1.0f});
        octree.Add(BasicOctree::TDataWrapper{{0.0f, 1.0f, 0.0f}, 1.0f});
        octree.Add(BasicOctree::TDataWrapper{{0.0f, 0.0f, 1.0f}, 1.0f});
        octree.Add(BasicOctree::TDataWrapper{{1.0f, 1.0f, 0.0f}, 1.0f});
        octree.Add(BasicOctree::TDataWrapper{{0.0f, 1.0f, 1.0f}, 1.0f});
        octree.Add(BasicOctree::TDataWrapper{{1.0f, 1.0f, 1.0f}, 1.0f});
        octree.Add(BasicOctree::TDataWrapper{{1.0f, 0.0f, 1.0f}, 1.0f});
    }

    EXPECT_EQ(octree.Query(SphereQuery<BasicOctree::TDataWrapper>{{0.5f, 0.5f, 0.5f}, 0.1f}).size(), 1);
    EXPECT_EQ(octree.Size(), 8001);
    EXPECT_EQ(octree.Query(SphereQuery<BasicOctree::TDataWrapper>{{0.0f, 0.0f, 0.0f}, 0.1f}).size(), 1000);
}
