#pragma once
#include "Vector3D.h"

namespace Vrixic
{
	namespace Math
	{
		struct VRIXIC_API Vector4D
		{
		public:
			float X;
			float Y;
			float Z;
			float W;

		public:
			inline Vector4D();

			inline Vector4D(float val);

			inline Vector4D(float x, float y, float z, float w = 1);

			inline Vector4D(const Vector3D& v, float w = 1);

		public:
			/* Unary operator overloads */

			inline Vector4D operator+(const Vector4D& v) const;

			inline Vector4D operator-(const Vector4D& v) const;

			/* Returns the negated copy of vector */
			inline Vector4D operator-() const;

			inline Vector4D operator*(float scalar) const;

			inline Vector4D operator/(float scalar) const;

			inline Vector4D operator*(const Vector4D& v) const;

			inline Vector4D operator/(const Vector4D& v) const;

			inline Vector4D operator+=(const Vector4D& v);

			inline Vector4D operator-=(const Vector4D& v);

			inline Vector4D operator*=(float scalar);

			inline Vector4D operator/=(float scalar);

			inline Vector4D operator*=(const Vector4D& v);

			inline Vector4D operator/=(const Vector4D& v);

		public:
			inline static Vector4D ZeroVector();

			inline static float DotProduct(const Vector4D& a, const Vector4D& b);

			inline static Vector4D Lerp(const Vector4D& start, const Vector4D& end, float ratio);

			inline float Length() const;

			inline float LengthSquared() const;

			inline const Vector4D& Normalize();

			inline Vector3D ToVector3D() const;
		};

		inline Vector4D::Vector4D()
			: X(0.0f), Y(0.0f), Z(0.0f), W(0.0f) {}

		inline Vector4D::Vector4D(float val)
			: X(val), Y(val), Z(val), W(val) {}

		inline Vector4D::Vector4D(float x, float y, float z, float w)
			: X(x), Y(y), Z(z), W(w) {}

		inline Vector4D::Vector4D(const Vector3D& v, float w)
			: X(v.X), Y(v.Y), Z(v.Z), W(w) {}

		inline Vector4D Vector4D::operator+(const Vector4D& v) const
		{
			return Vector4D(X + v.X, Y + v.Y, Z + v.Z, W + v.W);
		}

		inline Vector4D Vector4D::operator-(const Vector4D& v) const
		{
			return Vector4D(X - v.X, Y - v.Y, Z - v.Z, W - v.W);
		}

		inline Vector4D Vector4D::operator-() const
		{
			return Vector4D(-X, -Y, -Z, -W);
		}

		inline Vector4D Vector4D::operator*(float scalar) const
		{
			return Vector4D(X * scalar, Y * scalar, Z * scalar, W * scalar);
		}

		inline Vector4D Vector4D::operator/(float scalar) const
		{
			float r = 1.0f / scalar;
			return Vector4D(X * r, Y * r, Z * r, W * r);
		}

		inline Vector4D Vector4D::operator*(const Vector4D& v) const
		{
			return Vector4D(X * v.X, Y * v.Y, Z * v.Z, W * v.W);
		}

		inline Vector4D Vector4D::operator/(const Vector4D& v) const
		{
			return Vector4D(X / v.X, Y / v.Y, Z / v.Z, W / v.W);
		}

		inline Vector4D Vector4D::operator+=(const Vector4D& v)
		{
			X += v.X;
			Y += v.Y;
			Z += v.Z;
			W += v.W;

			return *this;
		}

		inline Vector4D Vector4D::operator-=(const Vector4D& v)
		{
			X -= v.X;
			Y -= v.Y;
			Z -= v.Z;
			W -= v.W;

			return *this;
		}

		inline Vector4D Vector4D::operator*=(float scalar)
		{
			X *= scalar;
			Y *= scalar;
			Z *= scalar;
			W *= scalar;

			return *this;
		}

		inline Vector4D Vector4D::operator/=(float scalar)
		{
			float r = 1.0f / scalar;
			X *= r;
			Y *= r;
			Z *= r;
			W *= r;

			return *this;
		}

		inline Vector4D Vector4D::operator*=(const Vector4D& v)
		{
			X *= v.X;
			Y *= v.Y;
			Z *= v.Z;
			W *= v.W;

			return *this;
		}

		inline Vector4D Vector4D::operator/=(const Vector4D& v)
		{
			X /= v.X;
			Y /= v.Y;
			Z /= v.Z;
			W /= v.W;

			return *this;
		}

		inline Vector4D Vector4D::ZeroVector()
		{
			return Vector4D(0, 0, 0, 0);
		}

		inline float Vector4D::DotProduct(const Vector4D& a, const Vector4D& b)
		{
			return (a.X * b.X + a.Y * b.Y + a.Z * b.Z + a.W * b.W);
		}

		inline Vector4D Vector4D::Lerp(const Vector4D& start, const Vector4D& end, float ratio)
		{
			//return Vector4D(MathUtils::Lerp(start.X, end.X, ratio), MathUtils::Lerp(start.Y, end.Y, ratio), MathUtils::Lerp(start.Z, end.Z, ratio), MathUtils::Lerp(start.W, end.W, ratio));
			return start + (end - start) * ratio;
		}

		inline float Vector4D::Length() const
		{
			return sqrtf(X * X + Y * Y + Z * Z + W * W);
		}

		inline float Vector4D::LengthSquared() const
		{
			return (X * X + Y * Y + Z * Z + W * W);
		}

		inline const Vector4D& Vector4D::Normalize()
		{
			const float Magnitude = (1.0f / (Length() + EPSILON));
			X *= Magnitude;
			Y *= Magnitude;
			Z *= Magnitude;
			W *= Magnitude;

			return *this;
		}

		inline Vector3D Vector4D::ToVector3D() const
		{
			return Vector3D(X, Y, Z);
		}
	}
}
