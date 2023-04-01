/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include "Matrix4D.h"

/* Quaternion representations -> [w, vector] -> [w, x, y, z]*/
/* For a given vector V which is the axis of rotation and angleInDegrees w, Quat = [V, w] */
struct VRIXIC_API Quat
{
public:
    float X;
    float Y;
    float Z;
    float W;

public:
    inline Quat();

    inline Quat(float x, float y, float z, float w);

    inline Quat(const Vector3D& v, float w);

public:
    /* Unary operator overloads */

    inline Quat operator+(const Quat& q) const;

    inline Quat operator-(const Quat& q) const;

    /* Returns the negated copy of vector */
    inline Quat operator-() const;

    /* Product of 2 Quaternionss: Q1 * Q2 = [w1 * w2 - Dot(v1, v2), w1*v2 + w2*v1 + Cross(v1, v2)] */
    inline Quat operator*(const Quat& q) const;

    inline Quat operator+=(const Quat& q);

    inline Quat operator-=(const Quat& q);

    inline Quat operator*=(const Quat& q);

public:
    inline static Quat Identity();

    inline static float DotProduct(const Quat& a, const Quat& b);

    /* Factory function for making a rotation quat with an axis and an angleInDegrees */
    inline static Quat MakeRotationQuat(float axisX, float axisY, float axisZ, float angleInDegrees);

    /* Factory function for making a rotation quat with an axis and an angleInDegrees */
    inline static Quat MakeRotationQuat(const Vector3D& axis, float angleInDegrees);

    /**
    * Creates a Quaternion from Euler Angles
    *
    * @param inPitch - X-Angle in degrees
    * @param inYaw - Y-Angle in degrees
    * @param inRoll - Z-Angle in degrees
    */
    inline static Quat MakeFromEuler(float inPitch, float inYaw, float inRoll);

    /**
    * Creates a quaternion from a rotational Matrix4D
    * Algorithm from: "https://www.gamedeveloper.com/programming/rotating-objects-using-quaternions"
    */
    inline static Quat MakeFromMatrix4D(const Matrix4D& inMat);

    /**
    * Spherical Interpolation
    * Algorithm from: "https://www.gamedeveloper.com/programming/rotating-objects-using-quaternions"
    * @param inFrom - start quaternion
    * @param inTarget - target quaternion
    * @param inTime - slerp factor
    */
    inline static Quat Slerp(const Quat& inFrom, const Quat& inTarget, float inTime);

    /**
    * Lerps the quaternion 'from' to 'target'
    */
    inline static Quat Lerp(const Quat& inFrom, const Quat& inTarget, float inTime);

    inline void SetIdentity();

    inline float Length() const;

    inline float LengthSquared() const;

    inline const Quat& Normalize();

    /*
    * Returns the conjugate of this quaternion
    *
    * if |Quat| = 1 (meaning quaternion is normalized), the Inverse() = Conjugate()
    */
    inline Quat Conjugate() const;

    /*
    * return Inverse of quaternion
    *
    * if |Quat| = 1 (meaning quaternion is normalized), the Inverse() = Conjugate()
    */
    inline Quat Inverse() const;

    /*
    * Faster as Inverse() = Conjugate(), it is implied that Quat is normalized
    *
    * Passive Rotation -> Coordinate system is rotated with respect to the point
    * Rotate a vector by this quaternion -> for a point q and quat q -> qpq^-1
    */
    inline Vector3D RotateVector(const Vector3D& v) const;

    /*
    * Slower as it calculates the Inverse()
    *
    * Passive Rotation -> Coordinate system is rotated with respect to the point
    * Rotate a vector by this quaternion -> for a point q and quat q -> qpq^-1
    */
    inline Vector3D RotateVectorSlow(const Vector3D& v) const;

    /**
    * Creates a Matrix4D from quaternion
    *
    * Algorithm from Algorithm from: "https://www.gamedeveloper.com/programming/rotating-objects-using-quaternions"
    */
    inline Matrix4D ToMatrix4D() const;
};

inline Quat::Quat()
    : X(0.0f), Y(0.0f), Z(0.0f), W(0.0f) { }

inline Quat::Quat(const Vector3D& v, float w)
    : X(v.X), Y(v.Y), Z(v.Z), W(w) { }

inline Quat::Quat(float x, float y, float z, float w)
    : X(x), Y(y), Z(z), W(w) { }

inline Quat Quat::operator+(const Quat& q) const
{
    return Quat(X + q.X, Y + q.Y, Z + q.Z, W + q.W);
}

inline Quat Quat::operator-(const Quat& q) const
{
    return Quat(X - q.X, Y - q.Y, Z - q.Z, W - q.W);
}

inline Quat Quat::operator-() const
{
    return Quat(-X, -Y, -Z, -W);
}

inline Quat Quat::operator*(const Quat& q) const
{
    /* Q1 * Q2 = [w1 * w2 - Dot(v1, v2), w1*v2 + w2*v1 + Cross(v1, v2)] */

    // InEfficient
    //float ResultX = W * q.X + q.W * X + Y * q.Z - Z * q.Y;
    //float ResultY = W * q.Y + q.W * Y + Z * q.X - X * q.Z;
    //float ResultZ = W * q.Z + q.W * Z + X * q.Y - Y * q.X;
    //float ResultW = W * q.W - (X * q.X + Y * q.Y + Z * q.Z);

    // Efficient 
    float A, B, C, D, E, F, G, H;
    A = (W + X) * (q.W + q.X);
    B = (Z - Y) * (q.Y - q.Z);
    C = (W - X) * (q.Y + q.Z);
    D = (Y + Z) * (q.W - q.X);
    E = (X + Z) * (q.X + q.Y);
    F = (X - Z) * (q.X - q.Y);
    G = (W + Y) * (q.W - q.Z);
    H = (W - Y) * (q.W + q.Z);

    float ResultX = A - (E + F + G + H) * 0.5f;
    float ResultY = C + (E - F + G - H) * 0.5f;
    float ResultZ = D + (E - F - G + H) * 0.5f;
    float ResultW = B + (-E - F + G + H) * 0.5f;

    return Quat(ResultX, ResultY, ResultZ, ResultW);
}

inline Quat Quat::operator+=(const Quat& q)
{
    X += q.X;
    Y += q.Y;
    Z += q.Z;
    W += q.W;

    return *this;
}

inline Quat Quat::operator-=(const Quat& q)
{
    X -= q.X;
    Y -= q.Y;
    Z -= q.Z;
    W -= q.W;

    return *this;
}

inline Quat Quat::operator*=(const Quat& q)
{
    float ResultX = W * q.X + q.W * X + Y * q.Z - Z * q.Y;
    float ResultY = W * q.Y + q.W * Y + Z * q.X - X * q.Z;
    float ResultZ = W * q.Z + q.W * Z + X * q.Y - Y * q.X;
    float ResultW = W * q.W - (X * q.X + Y * q.Y + Z * q.Z);

    X = ResultX;
    Y = ResultY;
    Z = ResultZ;
    W = ResultW;

    return *this;
}

inline Quat Quat::Identity()
{
    return Quat(0.0f, 0.0f, 0.0f, 1.0f);
}

inline float Quat::DotProduct(const Quat& a, const Quat& b)
{
    return a.X * b.X + a.Y * b.Y + a.Z * b.Z + a.W * b.W;
}

inline Quat Quat::MakeRotationQuat(float axisX, float axisY, float axisZ, float angleInDegrees)
{
    float HalfAngleInRads = MathUtils::DegreesToRadians(angleInDegrees * 0.5f);

    float C = cosf(HalfAngleInRads);
    float S = sinf(HalfAngleInRads);

    return Quat(axisX * S, axisY * S, axisZ * S, C);
}

inline Quat Quat::MakeRotationQuat(const Vector3D& axis, float angleInDegrees)
{
    float HalfAngleInRads = MathUtils::DegreesToRadians(angleInDegrees * 0.5f);

    float C = cosf(HalfAngleInRads);
    float S = sinf(HalfAngleInRads);

    return Quat(axis.X * S, axis.Y * S, axis.Z * S, C);
}

inline Quat Quat::MakeFromEuler(float inPitch, float inYaw, float inRoll)
{
    float PitchRads = MathUtils::DegreesToRadians(inPitch * 0.5f);
    float YawRads = MathUtils::DegreesToRadians(inYaw * 0.5f);
    float RollRads = MathUtils::DegreesToRadians(inRoll * 0.5f);

    float SinPitch = sinf(PitchRads);
    float CosPitch = cosf(PitchRads);

    float SinYaw = sinf(YawRads);
    float CosYaw = cosf(YawRads);

    float SinRoll = sinf(RollRads);
    float CosRoll = cosf(RollRads);

    float CosPitchCosYaw = CosPitch * CosYaw;
    float SinPitchSinYaw = SinPitch * SinYaw;

    return Quat(SinRoll * CosPitchCosYaw - CosRoll * SinPitchSinYaw,
        CosRoll * SinPitch * CosYaw + SinRoll * CosPitch * SinYaw,
        CosRoll * CosPitch * SinYaw - SinRoll * SinPitch * CosYaw,
        CosRoll * CosPitchCosYaw + SinRoll * SinPitchSinYaw);
}

inline Quat Quat::MakeFromMatrix4D(const Matrix4D& inMat)
{
    Quat Result = Quat();

    float Trace = inMat(0, 0) + inMat(1, 1) + inMat(2, 2);

    // Check the diagonal
    if (Trace > 0.0f)
    {
        float S = sqrtf(Trace + 1.0f);
        float T = 0.5f / S;

        Result.X = (inMat(2, 1) - inMat(1, 2)) * T;
        Result.Y = (inMat(0, 2) - inMat(2, 0)) * T;
        Result.Z = (inMat(1, 0) - inMat(0, 1)) * T;
        Result.W = S * 0.5f;
    }
    else
    {
        // Diagonal is negative
        int I = 0;
        if (inMat(1, 1) > inMat(0, 0)) I = 1;
        if (inMat(2, 2) > inMat(I, I)) I = 2;

        static const int NEXT[3] = { 1, 2, 0 };
        int J = NEXT[I];
        int K = NEXT[J];

        float* QuatPtr[4] = { &Result.X, &Result.Y, &Result.Z, &Result.W };

        float S = sqrt((inMat(I, J) - (inMat(J, J) + inMat(K, K)))
            + 1.0f);
        *QuatPtr[I] = S * 0.5f;
        float T;
        if (S != 0.0)
        {
            T = 0.5f / S;
        }
        else
        {
            T = S;
        }

        *QuatPtr[3] = (inMat(K, J) - inMat(J, K)) * T;
        *QuatPtr[J] = (inMat(J, I) + inMat(I, J)) * T;
        *QuatPtr[K] = (inMat(K, I) + inMat(I, K)) * T;
    }

    return Result;
}

inline Quat Quat::Slerp(const Quat& inFrom, const Quat& inTarget, float inTime)
{
    static float DELTA = 0.001f;

    Quat Target;

    // calculate cosine = Dot product
    float Dot = DotProduct(inFrom, inTarget);
    float Scale0;
    float Scale1;

    // Adjust signs if it is necessary
    if (Dot < 0.0f)
    {
        Dot = -Dot;
        Target = -inTarget;
    }
    else
    {
        Target = inTarget;
    }

    if ((1.0f - Dot) > DELTA)
    {
        // Slerp - calculate coefficients 
        float Theta = acosf(Dot);
        float SinTheta = sinf(Theta);
        Scale0 = sinf((1.0f - inTime) * Theta) / SinTheta;
        Scale1 = sinf(inTime * Theta) / SinTheta;
    }
    else
    {
        // "From" to "Target" quaternions are very close, so we can do linear interpolation
        Scale0 = 1.0f - inTime;
        Scale1 = inTime;
    }

    return Quat(Scale0 * inFrom.X + Scale1 * Target.X,
        Scale0 * inFrom.Y + Scale1 * Target.Y,
        Scale0 * inFrom.Z + Scale1 * Target.Z,
        Scale0 * inFrom.W + Scale1 * Target.W);
}

inline Quat Quat::Lerp(const Quat& inFrom, const Quat& inTarget, float inTime)
{
    Quat Result = { };
    float Time = 1.0f - inTime;
    Result.X = (Time * inFrom.X) + (inTime * inTarget.X);
    Result.Y = (Time * inFrom.Y) + (inTime * inTarget.Y);
    Result.Z = (Time * inFrom.Z) + (inTime * inTarget.Z);
    Result.W = (Time * inFrom.W) + (inTime * inTarget.W);

    Result.Normalize();

    return Result;
}

inline void Quat::SetIdentity()
{
    X = 0.0f;
    Y = 0.0f;
    Z = 0.0f;
    W = 1.0f;
}

inline float Quat::Length() const
{
    return sqrtf(X * X + Y * Y + Z * Z + W * W);
}

inline float Quat::LengthSquared() const
{
    return (X * X + Y * Y + Z * Z + W * W);
}

inline const Quat& Quat::Normalize()
{
    const float Magnitude = (1.0f / (Length() + EPSILON));

    X *= Magnitude;
    Y *= Magnitude;
    Z *= Magnitude;
    W *= Magnitude;

    return *this;
}

inline Quat Quat::Conjugate() const
{
    return Quat(-X, -Y, -Z, W);
}

inline Quat Quat::Inverse() const
{
    Quat Con = Conjugate();
    float LenRecip = 1.0f / LengthSquared();

    return Quat(Con.X * LenRecip, Con.Y * LenRecip, Con.Z * LenRecip, Con.W * LenRecip);
}

inline Vector3D Quat::RotateVector(const Vector3D& v) const
{
    Quat ToRotate = Quat(v, 0.0f);
    Quat Inv = Conjugate();

    Quat Result = *this * ToRotate * Inv;

    return Vector3D(Result.X, Result.Y, Result.Z);
}

inline Vector3D Quat::RotateVectorSlow(const Vector3D& v) const
{
    Quat ToRotate = Quat(v, 0.0f);
    Quat Inv = Inverse();

    Quat Result = *this * ToRotate * Inv;

    return Vector3D(Result.X, Result.Y, Result.Z);
}

inline Matrix4D Quat::ToMatrix4D() const
{
    // Converts this quaternion to a rotation matrix.
    //
    // | 1 - 2(y^2 + z^2)	2(xy + zw)			2(xz - yw)			0 |
    // | 2(xy - zw)			1 - 2(x^2 + z^2)	2(yz + xw)			0 |
    // | 2(xz + yw)			2(yz - xw)			1 - 2(x^2 + y^2)	0 |
    // | 0					0					0					1 |

    float X2 = X + X;
    float Y2 = Y + Y;
    float Z2 = Z + Z;

    float XX = X * X2;
    float XY = X * Y2;
    float XZ = X * Z2;

    float YY = Y * Y2;
    float YZ = Y * Z2;

    float ZZ = Z * Z2;

    float WX = W * X2;
    float WY = W * Y2;
    float WZ = W * Z2;

    return Matrix4D(1 - (YY + ZZ), XY - WZ, XZ + WY, 0,
        XY + WZ, 1 - (XX + ZZ), YZ - WX, 0,
        XZ - WY, YZ + WX, 1 - (XX + YY), 0,
        0, 0, 0, 1);
}
