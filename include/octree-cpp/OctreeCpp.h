#pragma once

#include "OctreeUtil.h"
#include "OctreeQuery.h"
#include <memory>

/**
 * A octree implementation with Bring your own vector class depending on what you use
 * in your project. Capabale of storing whatever type of data positioned in 3d space
 * and support complex queries to find whatever data you are looking for quickly.
 *
 * @tparam TVector "Bring your own", Vector class that you want to use. Needs to fufil VectorLike concept.
 * @tparam TData Data blob that should be paired up with the added object.
 */
template <typename TVector, typename TData>
requires VectorLike<TVector>
class OctreeCpp {
private:
    static constexpr size_t MaxData = 8;
    using Section = std::conditional_t<isVectorLike3D<TVector>(), Octant, Quadrant>;

public:
    using TDataWrapper = DataWrapper<TVector, TData>;
    using TBoundary = Boundary<TVector>;

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
     * Circle query, for finding objects within given circle.
     */
    using Circle = CircleQuery<TDataWrapper>;

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
    explicit OctreeCpp(TBoundary Boundary) : BoundaryData(Boundary) {
        Data.reserve(MaxData);
    }

    /**
     * Stores the given data in the octree container.
     * @param DataWrapper
     */
    void Add(const TDataWrapper& DataWrapper) {
        if (!IsPointInBoundrary(DataWrapper.Vector, BoundaryData)) {
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
        Section section = LocateOctant(DataWrapper.Vector, BoundaryData.GetMidpoint());
        if (!HasChild(section)) {
            CreateChild(section);
        }
        AddToChild(section, DataWrapper);
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

    /**
     * @return The boundraries of the octree.
     */
     [[nodiscard]] std::vector<TBoundary> GetBoundaries() const {
        std::vector<TBoundary> result;
        result.push_back(BoundaryData);
        for (const auto& child : Children) {
            if (child) {
                auto childBoundaries = child->GetBoundaries();
                for (auto & childchild : childBoundaries) {
                    result.push_back(childchild);
                }
            }
        }
        return result;
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
            if (child && QueryObject.Covers(child->BoundaryData)) {
                child->QueryInternal(QueryObject, result);
            }
        }
    }

    void CreateChild(Section section) {
        if (HasChild(section)) {
            throw std::runtime_error("Child already exists");
        }
        Children.at(static_cast<int>(section)) = std::move(std::make_unique<OctreeCpp<TVector, TData>>(
                GetBoundraryFromSection(section, BoundaryData)));
    }

    void AddToChild(Section octant, const TDataWrapper& DataWrapper) {
        int index = static_cast<int>(octant);
        if (!Children.at(index)) {
            throw std::runtime_error("Child does not exist");
        }
        Children.at(index)->Add(DataWrapper);
    }

    bool HasChild(Section octant) const {
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
            if (!IsPointInBoundrary(data.Vector, BoundaryData)) {
                return false;
            }
        }
        return true;
    }

    std::array<std::unique_ptr<OctreeCpp<TVector, TData>>, static_cast<int>(Section::Count)> Children;
    std::vector<TDataWrapper> Data;
    TBoundary BoundaryData;
    size_t NrObjects = 0;
};
