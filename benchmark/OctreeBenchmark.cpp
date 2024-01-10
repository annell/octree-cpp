//
// Created by Stefan Annell on 2023-05-20.
//

#include <octree-cpp/OctreeCpp.h>
#include <random>
#include <benchmark/benchmark.h>

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


static void BM_OctreeAdd2d(benchmark::State& state) {
    using Oct = OctreeCpp<vec2d, int>;
    Oct octree({{0.0f, 0.0f}, {1.0f, 1.0f}});
    for (auto _ : state)
        octree.Add({{1.0f, 0.0f}, 5});
}
BENCHMARK(BM_OctreeAdd2d);

static void BM_OctreeAdd3d(benchmark::State& state) {
    using Oct = OctreeCpp<vec, int>;
    Oct octree({{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}});
    for (auto _ : state)
        octree.Add({{1.0f, 0.0f, 0.0f}, 5});
}
BENCHMARK(BM_OctreeAdd3d);

static void BM_OctreeQuerySmall2d(benchmark::State& state) {
    using Oct = OctreeCpp<vec2d, int>;
    Oct octree({{0.0f, 0.0f}, {1.0f, 1.0f}});

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    for (int i = 0; i < state.range(0); i++) {
        octree.Add({{dis(gen), dis(gen)}, i});
    }

    for (auto _ : state) {
        auto result = octree.Query(CircleQuery<Oct::TDataWrapper>{{0.5f, 0.5f}, 0.1f});
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_OctreeQuerySmall2d)->DenseRange(0, 500000, 50000);

static void BM_OctreeQuerySmall3d(benchmark::State& state) {
    using Oct = OctreeCpp<vec, int>;
    Oct octree({{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}});

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    for (int i = 0; i < state.range(0); i++) {
        octree.Add({{dis(gen), dis(gen), dis(gen)}, i});
    }

    for (auto _ : state) {
        auto result = octree.Query(SphereQuery<Oct::TDataWrapper>{{0.5f, 0.5f, 0.5f}, 0.5f});
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BM_OctreeQuerySmall3d)->DenseRange(0, 500000, 50000);

static void BM_OctreeQueryLarge2d(benchmark::State& state) {
    using Oct = OctreeCpp<vec2d, int>;
    Oct octree({{0.0f, 0.0f}, {1.0f, 1.0f}});

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    for (int i = 0; i < state.range(0); i++) {
        octree.Add({{dis(gen), dis(gen)}, i});
    }

    for (auto _ : state) {
        auto result = octree.Query(CircleQuery<Oct::TDataWrapper>{{0.5f, 0.5f}, 1.5f});
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_OctreeQueryLarge2d)->DenseRange(0, 500000, 50000);

static void BM_OctreeQueryLarge3d(benchmark::State& state) {
    using Oct = OctreeCpp<vec, int>;
    Oct octree({{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}});

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    for (int i = 0; i < state.range(0); i++) {
        octree.Add({{dis(gen), dis(gen), dis(gen)}, i});
    }

    for (auto _ : state) {
        auto result = octree.Query(SphereQuery<Oct::TDataWrapper>{{0.5f, 0.5f, 0.5f}, 1.5f});
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_OctreeQueryLarge3d)->DenseRange(0, 500000, 50000);

BENCHMARK_MAIN();