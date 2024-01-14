//
// Created by Stefan Annell on 2023-12-20.
//
#pragma once

#include <array>
#include <functional>
#include <stdexcept>


template <typename TVector>
concept VectorLike3D = requires(TVector Vector) {
    { Vector.x } -> std::convertible_to<float>;
    { Vector.y } -> std::convertible_to<float>;
    { Vector.z } -> std::convertible_to<float>;
};

template <typename TVector>
concept VectorLike2D = requires(TVector Vector) {
    { Vector.x } -> std::convertible_to<float>;
    { Vector.y } -> std::convertible_to<float>;
};

template <typename TVector>
concept VectorLike2D_t = (not VectorLike3D<TVector> && VectorLike2D<TVector>);

template <typename T>
constexpr bool isVectorLike3D() {
    return requires(T Vector) {
        { Vector.x } -> std::convertible_to<float>;
        { Vector.y } -> std::convertible_to<float>;
        { Vector.z } -> std::convertible_to<float>;
    };
}

template <typename TVector>
concept VectorLike = requires(TVector Vector) {
    VectorLike3D<TVector> || VectorLike2D_t<TVector>;
};
template <typename TVector>
requires VectorLike3D<TVector> || VectorLike2D_t<TVector>
struct Boundary {};

template <VectorLike3D TVector>
struct Boundary<TVector> {
    const TVector Min = {};
    const TVector Max = {};
    static size_t constexpr NrCorners = 8;

    std::array<TVector, NrCorners> Corners() const {
        std::array<TVector, NrCorners> corners;
        corners[0] = {Min.x, Min.y, Min.z};
        corners[1] = {Min.x, Min.y, Max.z};
        corners[2] = {Min.x, Max.y, Min.z};
        corners[3] = {Min.x, Max.y, Max.z};
        corners[4] = {Max.x, Min.y, Min.z};
        corners[5] = {Max.x, Min.y, Max.z};
        corners[6] = {Max.x, Max.y, Min.z};
        corners[7] = {Max.x, Max.y, Max.z};
        return corners;
    }

    TVector GetMidpoint() const {
        return {
                (Min.x + Max.x) / 2,
                (Min.y + Max.y) / 2,
                (Min.z + Max.z) / 2
        };
    }

    TVector GetSize() const {
        return {
                Max.x - Min.x,
                Max.y - Min.y,
                Max.z - Min.z
        };
    }
};

template <VectorLike2D_t TVector>
struct Boundary<TVector> {
    const TVector Min = {};
    const TVector Max = {};
    static size_t constexpr NrCorners = 4;

    std::array<TVector, NrCorners> Corners() const {
        std::array<TVector, NrCorners> corners;
        corners[0] = {Min.x, Min.y};
        corners[1] = {Min.x, Max.y};
        corners[2] = {Max.x, Min.y};
        corners[3] = {Max.x, Max.y};
        return corners;
    }

    TVector GetMidpoint() const {
        return {
                (Min.x + Max.x) / 2,
                (Min.y + Max.y) / 2
        };
    }

    TVector GetSize() const {
        return {
                Max.x - Min.x,
                Max.y - Min.y
        };
    }
};

template <typename TQuery, typename TDataWrapper>
concept IsQuery = requires(TQuery Query) {
    { Query.IsInside(TDataWrapper()) } -> std::convertible_to<bool>;
    { Query.Covers(Boundary<typename TDataWrapper::VectorType>()) } -> std::convertible_to<bool>;
};

template <typename TDataWrapper>
concept IsDataWrapper = requires(TDataWrapper DataWrapper) {
    { DataWrapper.Vector } -> std::convertible_to<typename TDataWrapper::VectorType>;
    { DataWrapper.Data } -> std::convertible_to<typename TDataWrapper::DataT>;
};

template<VectorLike3D TVector>
bool IsPointInBoundrary(const TVector& Point, const Boundary<TVector>& Bound) {
    return Point.x >= Bound.Min.x && Point.x <= Bound.Max.x &&
           Point.y >= Bound.Min.y && Point.y <= Bound.Max.y &&
           Point.z >= Bound.Min.z && Point.z <= Bound.Max.z;
}

template<VectorLike2D_t TVector>
bool IsPointInBoundrary(const TVector& Point, const Boundary<TVector>& Bound) {
    return Point.x >= Bound.Min.x && Point.x <= Bound.Max.x &&
           Point.y >= Bound.Min.y && Point.y <= Bound.Max.y;
}

template<VectorLike3D TVector>
inline float DistanceSquared(const TVector& Point1, const TVector& Point2) {
    float diffX = Point1.x - Point2.x;
    float diffY = Point1.y - Point2.y;
    float diffZ = Point1.z - Point2.z;
    return diffX * diffX + diffY * diffY + diffZ * diffZ;
}

template<VectorLike2D_t TVector>
inline float DistanceSquared(const TVector& Point1, const TVector& Point2) {
    float diffX = Point1.x - Point2.x;
    float diffY = Point1.y - Point2.y;
    return diffX * diffX + diffY * diffY;
}

template <VectorLike3D TVector>
float Dot(const TVector& v1, const TVector& v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

template <VectorLike2D_t TVector>
float Dot(const TVector& v1, const TVector& v2) {
    return v1.x * v2.x + v1.y * v2.y;
}

template <VectorLike3D TVector>
float DistancePointToLine(const TVector& Point, const TVector& LinePoint1, const TVector& LinePoint2) {
    TVector v = LinePoint2;
    v.x -= LinePoint1.x;
    v.y -= LinePoint1.y;
    v.z -= LinePoint1.z;
    TVector w = Point;
    w.x -= LinePoint1.x;
    w.y -= LinePoint1.y;
    w.z -= LinePoint1.z;
    float c1 = Dot(w, v);
    float c2 = Dot(v, v);
    float b = c1 / c2;
    TVector Pb = TVector{LinePoint1};
    Pb.x += v.x * b;
    Pb.y += v.y * b;
    Pb.z += v.z * b;

    return DistanceSquared(Point, Pb);
}

template <VectorLike2D_t TVector>
float DistancePointToLine(const TVector& Point, const TVector& LinePoint1, const TVector& LinePoint2) {
    TVector v = LinePoint2;
    v.x -= LinePoint1.x;
    v.y -= LinePoint1.y;
    TVector w = Point;
    w.x -= LinePoint1.x;
    w.y -= LinePoint1.y;
    float c1 = Dot(w, v);
    float c2 = Dot(v, v);
    float b = c1 / c2;
    TVector Pb = TVector{LinePoint1};
    Pb.x += v.x * b;
    Pb.y += v.y * b;

    return DistanceSquared(Point, Pb);
}

template <VectorLike3D TVector>
bool IsBoxInsideCylinder(const Boundary<TVector>& Boundary, const TVector& CylinderPoint1, const TVector& CylinderPoint2, float CylinderRadius) {
    for (const auto& corner : Boundary.Corners()) {
        if (DistancePointToLine(corner, CylinderPoint1, CylinderPoint2) <= CylinderRadius * CylinderRadius) {
            return true;
        }
    }

    if (IsPointInBoundrary(CylinderPoint1, Boundary) || IsPointInBoundrary(CylinderPoint2, Boundary)) {
        return true;
    }

    return false;
}

template<VectorLike TVector>
inline TVector Clamp(const TVector& Value, const TVector& Min, const TVector& Max) {
    return std::min(std::max(Value, Min), Max);
}

inline bool checkOverlap(float R, float Xc, float Yc, float Zc,
                         float X1, float Y1, float Z1,
                         float X2, float Y2, float Z2)
{
    int Xn = std::max(X1, std::min(Xc, X2));
    int Yn = std::max(Y1, std::min(Yc, Y2));
    int Zn = std::max(Z1, std::min(Zc, Z2));

    int Dx = Xn - Xc;
    int Dy = Yn - Yc;
    int Zy = Zn - Zc;
    return (Dx * Dx + Dy * Dy + Zy * Zy) <= R * R;
}

inline bool checkOverlap2d(float R, float Xc, float Yc,
                         float X1, float Y1,
                         float X2, float Y2)
{
    int Xn = std::max(X1, std::min(Xc, X2));
    int Yn = std::max(Y1, std::min(Yc, Y2));

    int Dx = Xn - Xc;
    int Dy = Yn - Yc;
    return (Dx * Dx + Dy * Dy) <= R * R;
}

template<VectorLike3D TVector>
inline bool CheckOverlapp(const Boundary<TVector>& Boundary, const TVector& Point1, float Radius) {
    return checkOverlap(Radius, Point1.x, Point1.y, Point1.z, Boundary.Min.x, Boundary.Min.y, Boundary.Min.z, Boundary.Max.x, Boundary.Max.y, Boundary.Max.z);
}

template<VectorLike2D_t TVector>
inline bool CheckOverlapp(const Boundary<TVector>& Boundary, const TVector& Point1, float Radius) {
    return checkOverlap2d(Radius, Point1.x, Point1.y, Boundary.Min.x, Boundary.Min.y, Boundary.Max.x, Boundary.Max.y);
}

template <VectorLike TVector, std::default_initializable TData>
struct DataWrapper {
    using VectorType = TVector;
    using DataT = TData;

    VectorType Vector;
    DataT Data;
};

enum class Octant {
    TopLeftFront = 0,
    TopRightFront,
    BottomLeftFront,
    BottomRightFront,
    TopLeftBack,
    TopRightBack,
    BottomLeftBack,
    BottomRightBack,
    Count
};

enum class Quadrant {
    TopLeft = 0,
    TopRight,
    BottomLeft,
    BottomRight,
    Count
};

template <VectorLike3D TVector>
Octant LocateOctant(const TVector& Vector, const TVector& Midpoint) {
    if (Vector.x <= Midpoint.x) {
        if (Vector.y <= Midpoint.y) {
            return Vector.z <= Midpoint.z ? Octant::BottomLeftBack : Octant::TopLeftBack;
        }
        return Vector.z <= Midpoint.z ? Octant::BottomLeftFront : Octant::TopLeftFront;
    }
    if (Vector.y <= Midpoint.y) {
        return Vector.z <= Midpoint.z ? Octant::BottomRightBack : Octant::TopRightBack;
    }
    return Vector.z <= Midpoint.z ? Octant::BottomRightFront : Octant::TopRightFront;
}

template <VectorLike2D_t TVector>
Quadrant LocateOctant(const TVector& Vector, const TVector& Midpoint) {
    if (Vector.x <= Midpoint.x) {
        return Vector.y <= Midpoint.y ? Quadrant::BottomLeft : Quadrant::TopLeft;
    }
    return Vector.y <= Midpoint.y ? Quadrant::BottomRight : Quadrant::TopRight;
}

template <typename TBoundary>
TBoundary GetBoundraryFromSection(Octant octant, const TBoundary& Boundary) {
    auto Midpoint = Boundary.GetMidpoint();
    auto min = Boundary.Min;
    auto max = Boundary.Max;
    switch (octant) {
        case Octant::BottomLeftBack:
            return {min, Midpoint};
        case Octant::BottomLeftFront:
            return {{min.x, Midpoint.y, min.z}, {Midpoint.x, max.y, Midpoint.z}};
        case Octant::BottomRightBack:
            return {{Midpoint.x, min.y, min.z}, {max.x, Midpoint.y, Midpoint.z}};
        case Octant::BottomRightFront:
            return {{Midpoint.x, Midpoint.y, min.z}, {max.x, max.y, Midpoint.z}};
        case Octant::TopLeftBack:
            return {{min.x, min.y, Midpoint.z}, {Midpoint.x, Midpoint.y, max.z}};
        case Octant::TopLeftFront:
            return {{min.x, Midpoint.y, Midpoint.z}, {Midpoint.x, max.y, max.z}};
        case Octant::TopRightBack:
            return {{Midpoint.x, min.y, Midpoint.z}, {max.x, Midpoint.y, max.z}};
        case Octant::TopRightFront:
            return {Midpoint, max};
        default:
            throw std::runtime_error("Invalid octant");
    }
    return {};
}

template <typename TBoundary>
TBoundary GetBoundraryFromSection(Quadrant quadrant, const TBoundary& Boundary) {
    auto Midpoint = Boundary.GetMidpoint();
    auto min = Boundary.Min;
    auto max = Boundary.Max;
    switch (quadrant) {
        case Quadrant::BottomLeft:
            return {min, Midpoint};
        case Quadrant::BottomRight:
            return {{Midpoint.x, min.y}, {max.x, Midpoint.y}};
        case Quadrant::TopLeft:
            return {{min.x, Midpoint.y}, {Midpoint.x, max.y}};
        case Quadrant::TopRight:
            return {Midpoint, max};
        default:
            throw std::runtime_error("Invalid quadrant");
    }
    return {};
}

