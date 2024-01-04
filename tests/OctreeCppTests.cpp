//
// Created by Stefan Annell on 2023-05-20.
//

#include <octree-cpp/OctreeCpp.h>
#include <gtest/gtest.h>
#include <random>

struct vec {
    float x, y, z;
    auto operator<=>(const vec&) const = default;
};

struct vec2d {
    float x, y;
    auto operator<=>(const vec2d&) const = default;
};

using BasicOctree = OctreeCpp<vec, float>;
using BasicOctree2d = OctreeCpp<vec2d, float>;

TEST(OctreeCppTest, VectorLikeConcept) {
    static_assert(VectorLike<vec>);
    static_assert(VectorLike3D<vec>);
    static_assert(not VectorLike3D<vec2d>);
    static_assert(VectorLike2D_t<vec2d>);
    static_assert(not VectorLike2D_t<vec>);
    static_assert(not VectorLike2D_t<float>);
    static_assert(not VectorLike3D<float>);
}

TEST(OctreeCppTest, BoundaryConcept) {
    static_assert(std::is_constructible_v<Boundary<vec>>);
    static_assert(std::is_constructible_v<Boundary<vec2d>>);
    [[maybe_unused]] Boundary<vec> boundary({{0, 0, 0}, {1, 1, 1}});
    [[maybe_unused]] Boundary<vec2d> boundary2d({{0, 0}, {1, 1}});
}

TEST(OctreeCppTest, DataWrapperConcept) {
    static_assert(std::is_constructible_v<DataWrapper<vec, float>>);
    static_assert(std::is_constructible_v<DataWrapper<vec2d, float>>);
    [[maybe_unused]] DataWrapper<vec, float> data;
    [[maybe_unused]] DataWrapper<vec2d, float> data2d;
}

TEST(OctreeCppTest, IsQueryConcept) {
    static_assert(IsQuery<SphereQuery<BasicOctree::TDataWrapper>, BasicOctree::TDataWrapper>);
    static_assert(not IsQuery<float, vec>);
    static_assert(not IsQuery<SphereQuery<DataWrapper<vec, float>>, float>);
    [[maybe_unused]] SphereQuery<BasicOctree::TDataWrapper> query;
}

TEST(OctreeCppTest, OctreeConstructible) {
    static_assert(std::is_constructible_v<OctreeCpp<vec, float>, Boundary<vec>>);
    static_assert(std::is_constructible_v<OctreeCpp<vec2d, float>, Boundary<vec2d>>);
    static_assert(not std::is_constructible_v<OctreeCpp<vec2d, float>, Boundary<vec>>);
    static_assert(not std::is_constructible_v<OctreeCpp<vec, float>, Boundary<vec2d>>);
    [[maybe_unused]] BasicOctree octree({});
    [[maybe_unused]] BasicOctree2d octree2d({});
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

TEST(OctreeCppTest, OctreePredQuery) {
    using IntOctree = OctreeCpp<vec, int>;
    IntOctree octree({{0, 0, 0}, {1, 1, 1}});
    octree.Add(IntOctree::TDataWrapper{{0.5f, 0.5f, 0.5f}, -1});

    for (int i = 0; i < 1000; i++) {
        octree.Add(IntOctree::TDataWrapper{{0.0f, 0.0f, 0.0f}, i});
        octree.Add(IntOctree::TDataWrapper{{1.0f, 0.0f, 0.0f}, i});
        octree.Add(IntOctree::TDataWrapper{{0.0f, 1.0f, 0.0f}, i});
        octree.Add(IntOctree::TDataWrapper{{0.0f, 0.0f, 1.0f}, i});
        octree.Add(IntOctree::TDataWrapper{{1.0f, 1.0f, 0.0f}, i});
        octree.Add(IntOctree::TDataWrapper{{0.0f, 1.0f, 1.0f}, i});
        octree.Add(IntOctree::TDataWrapper{{1.0f, 1.0f, 1.0f}, i});
        octree.Add(IntOctree::TDataWrapper{{1.0f, 0.0f, 1.0f}, i});
    }

    EXPECT_EQ(octree.Query(SphereQuery<IntOctree::TDataWrapper>{{0.5f, 0.5f, 0.5f}, 0.1f}).size(), 1);
    EXPECT_EQ(octree.Size(), 8001);
    EXPECT_EQ(octree.Query(SphereQuery<IntOctree::TDataWrapper>{{0.0f, 0.0f, 0.0f}, 0.1f}).size(), 1000);
    EXPECT_EQ(octree.Query(PredQuery<IntOctree::TDataWrapper>{[](const auto& Data){return Data.Data < 0;}}).size(), 1);
    EXPECT_EQ(octree.Query(PredQuery<IntOctree::TDataWrapper>{[](const auto& Data){return Data.Data >= 0;}}).size(), 8000);
    EXPECT_EQ(octree.Query(PredQuery<IntOctree::TDataWrapper>{[](const auto& Data){return Data.Data == 0;}}).size(), 8);
}

TEST(OctreeCppTest, OctreeAndQuery) {
    using Oct = OctreeCpp<vec, int>;
    Oct octree({{0, 0, 0}, {1, 1, 1}});
    octree.Add({{0.5f, 0.5f, 0.5f}, -1});

    for (int i = 0; i < 1000; i++) {
        octree.Add({{0.0f, 0.0f, 0.0f}, i});
        octree.Add({{1.0f, 0.0f, 0.0f}, i});
        octree.Add({{0.0f, 1.0f, 0.0f}, i});
        octree.Add({{0.0f, 0.0f, 1.0f}, i});
        octree.Add({{1.0f, 1.0f, 0.0f}, i});
        octree.Add({{0.0f, 1.0f, 1.0f}, i});
        octree.Add({{1.0f, 1.0f, 1.0f}, i});
        octree.Add({{1.0f, 0.0f, 1.0f}, i});
    }

    auto Pred1 = Oct::Pred{[](const auto& Data){return Data.Data > 0;}};
    auto Pred2 = Oct::Pred{[](const auto& Data){return Data.Data < 2;}};
    auto Sphere = Oct::Sphere{{0.5f, 0.5f, 0.5f}, 0.1f};
    auto AQuery = Oct::And < Oct::Pred, Oct::Pred>{Pred1, Pred2};
    auto BQuery = Oct::And < Oct::Sphere, Oct::Pred>{Sphere, Pred2};
    auto CQuery = Oct::And < Oct::Not < Oct::Sphere >, Oct::Pred>{Sphere, Pred2};
    auto DQuery = Oct::Or < Oct::Sphere, Oct::Not<Oct::Sphere>>{Sphere, Sphere};
    EXPECT_EQ(octree.Query(AQuery).size(), 8);
    EXPECT_EQ(octree.Query(BQuery).size(), 1);
    EXPECT_EQ(octree.Query(CQuery).size(), 16);
    EXPECT_EQ(octree.Query(DQuery).size(), 8001);
}

TEST(OctreeCppTest, OctreeNotQuery) {
    using Oct = OctreeCpp<vec, int>;
    Oct octree({{0, 0, 0}, {1, 1, 1}});
    octree.Add({{0.5f, 0.5f, 0.5f}, 0});
    for (int i = 0; i < 20; i++) {
        octree.Add({{0.0f, 0.0f, 0.0f}, 1});
    }

    auto Sphere = Oct::Sphere{{0.5f, 0.5f, 0.5f}, 0.1f};
    auto result = octree.Query(Oct::Not<Oct::Sphere>{Sphere});
    EXPECT_EQ(result.size(), 20);
}

TEST(OctreeCppTest, OctreeQueryCylinderHit) {
    BasicOctree octree({{0, 0, 0}, {1, 1, 1}});
    auto query = BasicOctree::Cylinder{{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, 0.5f};
    EXPECT_EQ(octree.Query(query).size(), 0);

    octree.Add(BasicOctree::TDataWrapper{{0.5f, 0.5f, 0.5f}, 1.0f});
    EXPECT_EQ(octree.Query(query).size(), 1);
    octree.Add(BasicOctree::TDataWrapper{{1.0f, 1.0f, 0.0f}, 1.0f});
    EXPECT_EQ(octree.Query(query).size(), 1);
}

TEST(OctreeCppTest, OctreeQueryCylinderMiss) {
    BasicOctree octree({{0, 0, 0}, {1, 1, 1}});
    auto query = BasicOctree::Cylinder{{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, 0.5f};
    EXPECT_EQ(octree.Query(query).size(), 0);

    octree.Add(BasicOctree::TDataWrapper{{1.0f, 1.0f, 0.0f}, 1.0f});
    EXPECT_EQ(octree.Query(query).size(), 0);

    octree.Add(BasicOctree::TDataWrapper{{0.0f, 1.0f, 1.0f}, 1.0f});
    EXPECT_EQ(octree.Query(query).size(), 0);

    octree.Add(BasicOctree::TDataWrapper{{1.0f, 0.0f, 1.0f}, 1.0f});
    EXPECT_EQ(octree.Query(query).size(), 0);
}

TEST(OctreeCppTest, OctreeQueryCircleHit2d) {
    BasicOctree2d octree({{0, 0}, {1, 1}});
    EXPECT_EQ(octree.Query(CircleQuery<BasicOctree2d::TDataWrapper>{{0.5f, 0.5f}, 0.5f}).size(), 0);

    octree.Add({{0.5f, 0.5f}, 1.0f});
    EXPECT_EQ(octree.Query(SphereQuery<BasicOctree2d::TDataWrapper>{{0.5f, 0.5f}, 0.5f}).size(), 1);
}


TEST(OctreeCppTest, OctreeQueryCircle2dHitNegative) {
    BasicOctree2d octree({{-100, -100}, {100, 100}});
    EXPECT_EQ(octree.Query(SphereQuery<BasicOctree2d::TDataWrapper>{{0.5f, 0.5f}, 0.5f}).size(), 0);

    octree.Add({{-20.0f, -20.5f}, 1.0f});
    octree.Add({{-5.0f, -20.5f}, 1.0f});
    octree.Add({{-5.0f, -5.5f}, 1.0f});
    EXPECT_EQ(octree.Query(SphereQuery<BasicOctree2d::TDataWrapper>{{-20.0f, -70.0f}, 50.0f}).size(), 1);
}
