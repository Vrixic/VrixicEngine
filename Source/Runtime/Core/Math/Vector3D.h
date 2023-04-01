/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include "VrixicMathHelper.h"

#include <corecrt_math.h>

/* Row vector */
struct VRIXIC_API Vector3D
{
public:
    float X;
    float Y;
    float Z;

public:
    inline Vector3D();

    inline Vector3D(float val);

    inline Vector3D(float x, float y, float z);

public:
    /* Unary operator overloads */

    inline Vector3D operator=(const Vector3D& other);

    inline Vector3D operator+(const Vector3D& v) const;

    inline Vector3D operator-(const Vector3D& v) const;

    /* Returns the negated copy of vector */
    inline Vector3D operator-() const;

    inline Vector3D operator*(float scalar) const;

    inline Vector3D operator/(float scalar) const;

    inline Vector3D operator*(const Vector3D& v) const;

    inline Vector3D operator/(const Vector3D& v) const;

    inline Vector3D operator+=(const Vector3D& v);

    inline Vector3D operator-=(const Vector3D& v);

    inline Vector3D operator*=(float scalar);

    inline Vector3D operator/=(float scalar);

    inline Vector3D operator*=(const Vector3D& v);

    inline Vector3D operator/=(const Vector3D& v);

public:
    inline static Vector3D ZeroVector();

    inline static float DotProduct(const Vector3D& a, const Vector3D& b);

    inline static Vector3D CrossProduct(const Vector3D& a, const Vector3D& b);

    inline static Vector3D Lerp(const Vector3D& start, const Vector3D& end, float ratio);

    inline float Length() const;

    inline float LengthSquared() const;

    /**
    * Normalize this vector
    *
    * @return const Vector3D& normalized vector
    */
    inline const Vector3D& Normalize();
};

inline Vector3D::Vector3D()
    : X(0.0f), Y(0.0f), Z(0.0f) {}

inline Vector3D::Vector3D(float val)
    : X(val), Y(val), Z(val) {}

inline Vector3D::Vector3D(float x, float y, float z)
    : X(x), Y(y), Z(z) {}

inline Vector3D Vector3D::operator=(const Vector3D& other)
{
    X = other.X;
    Y = other.Y;
    Z = other.Z;

    return *this;
}

inline Vector3D Vector3D::operator+(const Vector3D& v) const
{
    return Vector3D(X + v.X, Y + v.Y, Z + v.Z);
}

inline Vector3D Vector3D::operator-(const Vector3D& v) const
{
    return Vector3D(X - v.X, Y - v.Y, Z - v.Z);
}

inline Vector3D Vector3D::operator-() const
{
    return Vector3D(-X, -Y, -Z);
}

inline Vector3D Vector3D::operator*(float scalar) const
{
    return Vector3D(X * scalar, Y * scalar, Z * scalar);
}

inline Vector3D Vector3D::operator/(float scalar) const
{
    float r = 1.0f / scalar;
    return Vector3D(X * r, Y * r, Z * r);
}

inline Vector3D Vector3D::operator*(const Vector3D& v) const
{
    return Vector3D(X * v.X, Y * v.Y, Z * v.Z);
}

inline Vector3D Vector3D::operator/(const Vector3D& v) const
{
    return Vector3D(X / v.X, Y / v.Y, Z / v.Z);
}

inline Vector3D Vector3D::operator+=(const Vector3D& v)
{
    X += v.X;
    Y += v.Y;
    Z += v.Z;

    return *this;
}

inline Vector3D Vector3D::operator-=(const Vector3D& v)
{
    X -= v.X;
    Y -= v.Y;
    Z -= v.Z;

    return *this;
}

inline Vector3D Vector3D::operator*=(float scalar)
{
    X *= scalar;
    Y *= scalar;
    Z *= scalar;

    return *this;
}

inline Vector3D Vector3D::operator/=(float scalar)
{
    float r = 1.0f / scalar;
    X *= r;
    Y *= r;
    Z *= r;

    return *this;
}

inline Vector3D Vector3D::operator*=(const Vector3D& v)
{
    X *= v.X;
    Y *= v.Y;
    Z *= v.Z;

    return *this;
}

inline Vector3D Vector3D::operator/=(const Vector3D& v)
{
    X /= v.X;
    Y /= v.Y;
    Z /= v.Z;

    return *this;
}

inline Vector3D Vector3D::ZeroVector()
{
    return Vector3D(0.0f, 0.0f, 0.0f);
}

inline float Vector3D::DotProduct(const Vector3D& a, const Vector3D& b)
{
    return (a.X * b.X + a.Y * b.Y + a.Z * b.Z);
}

inline Vector3D Vector3D::CrossProduct(const Vector3D& a, const Vector3D& b)
{
    return Vector3D(
        a.Y * b.Z - a.Z * b.Y,
        a.Z * b.X - a.X * b.Z,
        a.X * b.Y - a.Y * b.X
    );
}

inline Vector3D Vector3D::Lerp(const Vector3D& start, const Vector3D& end, float ratio)
{
    //return Vector3D(MathUtils::Lerp(start.X, end.X, ratio), MathUtils::Lerp(start.Y, end.Y, ratio), MathUtils::Lerp(start.Z, end.Z, ratio));
    return start + (end - start) * ratio;
}

inline float Vector3D::Length() const
{
    return sqrtf(X * X + Y * Y + Z * Z);
}

inline float Vector3D::LengthSquared() const
{
    return (X * X + Y * Y + Z * Z);
}

inline const Vector3D& Vector3D::Normalize()
{
    const float Magnitude = (1.0f / (Length() + EPSILON));
    X *= Magnitude;
    Y *= Magnitude;
    Z *= Magnitude;

    return *this;
}