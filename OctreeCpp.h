#pragma once

#include <array>
#include <memory>

template <typename TVector>
concept VectorLike = requires(TVector v) {
    { v.x } -> std::convertible_to<float>;
    { v.y } -> std::convertible_to<float>;
    { v.z } -> std::convertible_to<float>;
};

template <VectorLike TVector>
struct Boundary {
    TVector Min;
    TVector Max;

    constexpr TVector GetTopLeftFront() const {
        return {Min.x, Max.y, Max.z};
    }

    constexpr TVector GetTopRightFront() const {
        return {Max.x, Max.y, Max.z};
    }

    constexpr TVector GetTopLeftBack() const {
        return {Min.x, Min.y, Max.z};
    }

    constexpr TVector GetTopRightBack() const {
        return {Max.x, Min.y, Max.z};
    }

    constexpr TVector GetBottomLeftFront() const {
        return {Min.x, Max.y, Min.z};
    }

    constexpr TVector GetBottomRightFront() const {
        return {Max.x, Max.y, Min.z};
    }

    constexpr TVector GetBottomLeftBack() const {
        return {Min.x, Min.y, Min.z};
    }

    constexpr TVector GetBottomRightBack() const {
        return {Max.x, Min.y, Min.z};
    }

    bool IsInside(const TVector& Vector) const {
        return Min.x <= Vector.x && Vector.x <= Max.x &&
               Min.y <= Vector.y && Vector.y <= Max.y &&
               Min.z <= Vector.z && Vector.z <= Max.z;
    }
};

template <typename TQuery, typename TVector>
concept IsQuery = requires(TQuery q) {
    { q.IsInside(TVector()) } -> std::convertible_to<bool>;
    { q.Covers(Boundary<TVector>()) } -> std::convertible_to<bool>;
};

template <VectorLike TVector>
struct QueryRadius {
    TVector Midpoint;
    float Radius;

    float Distance(const TVector& Point1, const TVector& Point2) const {
        float dx = Point1.x - Point2.x;
        float dy = Point1.y - Point2.y;
        float dz = Point1.z - Point2.z;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    bool IsInside(const TVector& Point) const {
        return Distance(Midpoint, Point) <= Radius;
    }

    bool Covers(const Boundary<TVector>& Boundary) const {
        return Distance(Midpoint, Boundary.GetTopLeftFront()) <= Radius ||
               Distance(Midpoint, Boundary.GetTopRightFront()) <= Radius ||
               Distance(Midpoint, Boundary.GetTopLeftBack()) <= Radius ||
               Distance(Midpoint, Boundary.GetTopRightBack()) <= Radius ||
               Distance(Midpoint, Boundary.GetBottomLeftFront()) <= Radius ||
               Distance(Midpoint, Boundary.GetBottomRightFront()) <= Radius ||
               Distance(Midpoint, Boundary.GetBottomLeftBack()) <= Radius ||
               Distance(Midpoint, Boundary.GetBottomRightBack()) <= Radius;
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
    static constexpr int MaxData = 8;
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
        if (!Boundary.IsInside(DataWrapper.Vector)) {
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

    size_t Size() const {
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

    bool ValidateInvariant() const {
        if (Data.size() > MaxData) {
            return false;
        }
        for (const auto& data : Data) {
            if (!Boundary.IsInside(data.Vector)) {
                return false;
            }
        }
        return true;
    }

    std::array<std::unique_ptr<OctreeCpp<TVector, TData>>, 8> Children;
    std::vector<DataWrapper<TVector, TData>> Data;
    Boundary<TVector> Boundary;
    size_t NrObjects = 0;
};
