#pragma once

#include <array>
#include <memory>

template <typename TVector>
concept VectorLike = requires(TVector Vector) {
    { Vector.x } -> std::convertible_to<float>;
    { Vector.y } -> std::convertible_to<float>;
    { Vector.z } -> std::convertible_to<float>;
};

template <VectorLike TVector>
struct Boundary {
    const TVector Min = {0, 0, 0};
    const TVector Max = {0, 0, 0};
};

template<typename TVector>
bool IsPointInBoundrary(const TVector& Point, const Boundary<TVector>& Bound) {
    return Point.x >= Bound.Min.x && Point.x <= Bound.Max.x &&
           Point.y >= Bound.Min.y && Point.y <= Bound.Max.y &&
           Point.z >= Bound.Min.z && Point.z <= Bound.Max.z;
}

template<typename TVector>
float Distance(const TVector& Point1, const TVector& Point2) {
    float diffX = Point1.x - Point2.x;
    float diffY = Point1.y - Point2.y;
    float diffZ = Point1.z - Point2.z;
    return std::sqrt(diffX * diffX + diffY * diffY + diffZ * diffZ);
}

template<typename TVector>
TVector Clamp(const TVector& Value, const TVector& Min, const TVector& Max) {
    return std::min(std::max(Value, Min), Max);
}

template<typename TVector>
float DistanceBoxSphere(const Boundary<TVector>& Box, const TVector& SphereCenter, float SphereRadius) {
    const TVector closestPoint = Clamp(SphereCenter, Box.Min, Box.Max);
    if (Distance(closestPoint, SphereCenter) > SphereRadius) {
        return Distance(closestPoint, SphereCenter) - SphereRadius;
    }
    return 0.0f;
}

template <typename TQuery, typename TVector>
concept IsQuery = requires(TQuery Query) {
    { Query.IsInside(TVector()) } -> std::convertible_to<bool>;
    { Query.Covers(Boundary<TVector>()) } -> std::convertible_to<bool>;
};

template <VectorLike TVector>
struct SphereQuery {
    const TVector Midpoint = {0, 0, 0};
    const float Radius = 0.0f;

    bool IsInside(const TVector& Point) const {
        return Distance(Midpoint, Point) <= Radius;
    }

    bool Covers(const Boundary<TVector>& Boundary) const {
        return DistanceBoxSphere(Boundary, Midpoint, Radius) <= 0.0f;
    }
};

template <VectorLike TVector>
struct AllQuery {
    bool IsInside([[maybe_unused]] const TVector& Vector) const {
        return true;
    }
    bool Covers([[maybe_unused]] const Boundary<TVector>& Boundary) const {
        return true;
    }
};

template <VectorLike TVector, std::default_initializable TData>
struct DataWrapper {
    TVector Vector;
    TData Data;
};

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
    explicit OctreeCpp(Boundary<TVector> Boundary) : Boundary(Boundary) {
    }

    void Add(const DataWrapper<TVector, TData>& DataWrapper) {
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

    template <IsQuery<TVector> TQueryObject>
    [[nodiscard]] std::vector<DataWrapper<TVector, TData>> Query(const TQueryObject& QueryObject) const {
        std::vector<DataWrapper<TVector, TData>> result;
        for (const auto& data : Data) {
            if (QueryObject.IsInside(data.Vector)) {
                result.push_back(data);
            }
        }
        for (const auto& child : Children) {
            if (child && QueryObject.Covers(child->Boundary)) {
                auto childResult = child->Query(QueryObject);
                result.insert(result.end(), childResult.begin(), childResult.end());
            }
        }
        return result;
    }

    [[nodiscard]] size_t Size() const {
        return NrObjects;
    }

private:
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

    void AddToChild(Octant octant, const DataWrapper<TVector, TData>& DataWrapper) {
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
    std::vector<DataWrapper<TVector, TData>> Data;
    Boundary<TVector> Boundary;
    size_t NrObjects = 0;
};
