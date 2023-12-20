#pragma once

#include "OctreeUtil.h"
#include <list>

/**
 * A octree implementation with Bring your own vector class depending on what you use
 * in your project. Capabale of storing whatever type of data positioned in 3d space
 * and support complex queries to find whatever data you are looking for quickly.
 *
 * @tparam TVector "Bring your own", Vector class that you want to use. Needs to fufil VectorLike concept.
 * @tparam TData Data blob that should be paired up with the added object.
 */
template <VectorLike TVector, typename TData>
class OctreeCpp {
private:
    static constexpr size_t MaxData = 8;
    static constexpr size_t NrChildren = 8;
    enum class Octant {
        TopLeftFront = 0,
        TopRightFront,
        BottomLeftFront,
        BottomRightFront,
        TopLeftBack,
        TopRightBack,
        BottomLeftBack,
        BottomRightBack
    };
public:
    using TDataWrapper = DataWrapper<TVector, TData>;

    /**
     * Not query, inverts whatever query it has as input.
     */
    template <IsQuery<TDataWrapper> Query>
    using Not = NotQuery<TDataWrapper, Query>;

    /**
     * Sphere query, for finding objects within given sphere.
     */
    using Sphere = SphereQuery<TDataWrapper>;

    /**
     * Cylinder query, for finding objects within given cylinder.
     */
    using Cylinder = CylinderQuery<TDataWrapper>;

    /**
     * Predicate query to find based on something specific in
     * either position or the data.
     */
    using Pred = PredQuery<TDataWrapper>;

    /**
     * Returns all points.
     */
    using All = AllQuery<TDataWrapper>;

    /**
     * Composite queries AND / OR that binds together several queries to
     * get more interesting results.
     */
    template <IsQuery<TDataWrapper> QueryLHS, IsQuery<TDataWrapper> QueryRHS>
    using And = AndQuery<TDataWrapper, QueryLHS, QueryRHS>;
    template <IsQuery<TDataWrapper> QueryLHS, IsQuery<TDataWrapper> QueryRHS>
    using Or = OrQuery<TDataWrapper, QueryLHS, QueryRHS>;

    /**
     * Constructor to setup the Octree.
     *
     * @param Boundary min and max X, Y, Z values of the octree.
     */
    explicit OctreeCpp(Boundary<TVector> Boundary) : Boundary(Boundary) {
    }

    /**
     * Stores the given data in the octree container.
     * @param DataWrapper
     */
    void Add(const TDataWrapper& DataWrapper) {
        if (!IsPointInBoundrary(DataWrapper.Vector, Boundary)) {
            throw std::runtime_error("Vector is outside of boundary");
        }

        if (Data.size() < MaxData) {
            Data.push_back(DataWrapper);
            if (!ValidateInvariant()) {
                throw std::runtime_error("Invariant is broken");
            }
            NrObjects++;
            return;
        }
        auto octant = LocateOctant(DataWrapper.Vector);
        if (!HasChild(octant)) {
            CreateChild(octant);
        }
        AddToChild(octant, DataWrapper);
        NrObjects++;
    }

    /**
     * Queries the octree and returns all results that returns a hit.
     * Predefined queries can be user or you can define your own that fufil the concept IsQuery.
     *
     * @tparam TQueryObject The query type passed in.
     * @param QueryObject The object of TQueryObject with the query
     * @return A vector of results.
     */
    template <IsQuery<TDataWrapper> TQueryObject>
    [[nodiscard]] std::vector<TDataWrapper> Query(const TQueryObject& QueryObject) const {
        std::vector<TDataWrapper> result;
        QueryInternal(QueryObject, result);
        return result;
    }

    /**
     * @return Number of object in container.
     */
    [[nodiscard]] size_t Size() const {
        return NrObjects;
    }

private:
    template <IsQuery<TDataWrapper> TQueryObject>
    void QueryInternal(const TQueryObject& QueryObject, std::vector<TDataWrapper>& result) const {
        for (const auto& data : Data) {
            if (QueryObject.IsInside(data)) {
                result.push_back(data);
            }
        }
        if (Data.size() < MaxData) {
            return;
        }
        for (const auto& child : Children) {
            if (child && QueryObject.Covers(child->Boundary)) {
                child->QueryInternal(QueryObject, result);
            }
        }
    }
    TVector GetMidpoint() const {
        return {
            (Boundary.Min.x + Boundary.Max.x) / 2,
            (Boundary.Min.y + Boundary.Max.y) / 2,
            (Boundary.Min.z + Boundary.Max.z) / 2
        };
    }

    Octant LocateOctant(const TVector& Vector) const {
        auto midpoint = GetMidpoint();
        if (Vector.x <= midpoint.x) {
            if (Vector.y <= midpoint.y) {
                return Vector.z <= midpoint.z ? Octant::BottomLeftBack : Octant::TopLeftBack;
            }
            return Vector.z <= midpoint.z ? Octant::BottomLeftFront : Octant::TopLeftFront;
        }
        if (Vector.y <= midpoint.y) {
            return Vector.z <= midpoint.z ? Octant::BottomRightBack : Octant::TopRightBack;
        }
        return Vector.z <= midpoint.z ? Octant::BottomRightFront : Octant::TopRightFront;
    }

    Boundary<TVector> GetBoundraryFromOctant(Octant octant) const {
        auto midpoint = GetMidpoint();
        auto min = Boundary.Min;
        auto max = Boundary.Max;
        switch (octant) {
            case Octant::BottomLeftBack:
                return {min, midpoint};
            case Octant::BottomLeftFront:
                return {{min.x, midpoint.y, min.z}, {midpoint.x, max.y, midpoint.z}};
            case Octant::BottomRightBack:
                return {{midpoint.x, min.y, min.z}, {max.x, midpoint.y, midpoint.z}};
            case Octant::BottomRightFront:
                return {{midpoint.x, midpoint.y, min.z}, {max.x, max.y, midpoint.z}};
            case Octant::TopLeftBack:
                return {{min.x, min.y, midpoint.z}, {midpoint.x, midpoint.y, max.z}};
            case Octant::TopLeftFront:
                return {{min.x, midpoint.y, midpoint.z}, {midpoint.x, max.y, max.z}};
            case Octant::TopRightBack:
                return {{midpoint.x, min.y, midpoint.z}, {max.x, midpoint.y, max.z}};
            case Octant::TopRightFront:
                return {midpoint, max};
            default:
                throw std::runtime_error("Invalid octant");
        }
        return {};
    }

    void CreateChild(Octant octant) {
        if (HasChild(octant)) {
            throw std::runtime_error("Child already exists");
        }
        Children.at(static_cast<int>(octant)) = std::move(std::make_unique<OctreeCpp<TVector, TData>>(
                GetBoundraryFromOctant(octant)));
    }

    void AddToChild(Octant octant, const TDataWrapper& DataWrapper) {
        int index = static_cast<int>(octant);
        if (!Children.at(index)) {
            throw std::runtime_error("Child does not exist");
        }
        Children.at(index)->Add(DataWrapper);
    }

    bool HasChild(Octant octant) const {
        size_t index = static_cast<int>(octant);
        if (index > Children.size()) {
            throw std::runtime_error("Invalid octant");
        }
        return Children.at(index) != nullptr;
    }

    [[nodiscard]] bool ValidateInvariant() const {
        if (Data.size() > MaxData) {
            return false;
        }
        for (const auto& data : Data) {
            if (!IsPointInBoundrary(data.Vector, Boundary)) {
                return false;
            }
        }
        return true;
    }

    std::array<std::unique_ptr<OctreeCpp<TVector, TData>>, NrChildren> Children;
    std::list<TDataWrapper> Data;
    Boundary<TVector> Boundary;
    size_t NrObjects = 0;
};
