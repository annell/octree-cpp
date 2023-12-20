//
// Created by Stefan Annell on 2023-12-20.
//
#pragma once

#include <array>
#include <functional>


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

    std::array<TVector, 8> Corners() const {
        std::array<TVector, 8> corners;
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

template<VectorLike TVector>
bool IsPointInBoundrary(const TVector& Point, const Boundary<TVector>& Bound) {
    return Point.x >= Bound.Min.x && Point.x <= Bound.Max.x &&
           Point.y >= Bound.Min.y && Point.y <= Bound.Max.y &&
           Point.z >= Bound.Min.z && Point.z <= Bound.Max.z;
}

template<VectorLike TVector>
inline float DistanceSquared(const TVector& Point1, const TVector& Point2) {
    float diffX = Point1.x - Point2.x;
    float diffY = Point1.y - Point2.y;
    float diffZ = Point1.z - Point2.z;
    return diffX * diffX + diffY * diffY + diffZ * diffZ;
}

template <VectorLike TVector>
float Dot(const TVector& v1, const TVector& v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

template <VectorLike TVector>
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

template <VectorLike TVector>
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

template<VectorLike TVector>
inline bool CheckOverlapp(const Boundary<TVector>& Boundary, const TVector& Point1, float Radius) {
    return checkOverlap(Radius, Point1.x, Point1.y, Point1.z, Boundary.Min.x, Boundary.Min.y, Boundary.Min.z, Boundary.Max.x, Boundary.Max.y, Boundary.Max.z);
}

template <VectorLike TVector, std::default_initializable TData>
struct DataWrapper {
    using VectorType = TVector;
    using DataT = TData;

    VectorType Vector;
    DataT Data;
};

template <IsDataWrapper TDataWrapper>
struct AllQuery {
    bool IsInside([[maybe_unused]] const TDataWrapper& Vector) const {
        return true;
    }
    bool Covers([[maybe_unused]] const Boundary<typename TDataWrapper::VectorType>& Boundary) const {
        return true;
    }
};

template <IsDataWrapper TDataWrapper>
struct SphereQuery {
    const typename TDataWrapper::VectorType Midpoint = {0, 0, 0};
    const float Radius = 0.0f;

    bool IsInside(const TDataWrapper& Data) const {
        return DistanceSquared(Midpoint, Data.Vector) <= Radius * Radius;
    }

    bool Covers(const Boundary<typename TDataWrapper::VectorType>& Boundary) const {
        return CheckOverlapp(Boundary, Midpoint, Radius);
    }
};

template <IsDataWrapper TDataWrapper>
struct CylinderQuery {
    const typename TDataWrapper::VectorType Point1 = {0, 0, 0};
    const typename TDataWrapper::VectorType Point2 = {0, 0, 0};
    const float Radius = 0.0f;

    bool IsInside(const TDataWrapper& Data) const {
        return DistancePointToLine(Data.Vector, Point1, Point2) <= Radius;
    }

    bool Covers(const Boundary<typename TDataWrapper::VectorType>& Boundary) const {
        return IsBoxInsideCylinder(Boundary, Point1, Point2, Radius);
    }
};

template <IsDataWrapper TDataWrapper>
struct PredQuery {
    std::function<bool(const TDataWrapper&)> Pred;

    bool IsInside(const TDataWrapper& Data) const {
        return Pred(Data);
    }

    bool Covers([[maybe_unused]] const Boundary<typename TDataWrapper::VectorType>& Boundary) const {
        return true;
    }
};

template <IsDataWrapper TDataWrapper, IsQuery<TDataWrapper> QueryLHS, IsQuery<TDataWrapper> QueryRHS>
struct AndQuery {
    QueryLHS Query1;
    QueryRHS Query2;

    bool IsInside(const TDataWrapper& Data) const {
        return Query1.IsInside(Data) && Query2.IsInside(Data);
    }

    bool Covers(const Boundary<typename TDataWrapper::VectorType>& Boundary) const {
        return Query1.Covers(Boundary) && Query2.Covers(Boundary);
    }
};

template <IsDataWrapper TDataWrapper, IsQuery<TDataWrapper> QueryLHS, IsQuery<TDataWrapper> QueryRHS>
struct OrQuery {
    QueryLHS Query1;
    QueryRHS Query2;

    bool IsInside(const TDataWrapper& Data) const {
        return Query1.IsInside(Data) || Query2.IsInside(Data);
    }

    bool Covers(const Boundary<typename TDataWrapper::VectorType>& Boundary) const {
        return Query1.Covers(Boundary) || Query2.Covers(Boundary);
    }
};

template <IsDataWrapper TDataWrapper, IsQuery<TDataWrapper> TQuery>
struct NotQuery {
    TQuery Query;

    bool IsInside(const TDataWrapper& Data) const {
        return !Query.IsInside(Data);
    }

    bool Covers([[maybe_unused]] const Boundary<typename TDataWrapper::VectorType>& Boundary) const {
        return true;
    }
};

