/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include "Vector3D.h"

#include <iostream>

/*
* XYZ for the normal of the plane
* Distance is the distance of the plane -> often called distance from the origin
*/
namespace Vrixic
{
	namespace Math
	{
		struct VRIXIC_API Plane
		{
			float X;
			float Y;
			float Z;
			float Distance;

			inline Plane();

			inline Plane(float x, float y, float z, float distance);

			inline Plane(Vector3D& normal, float distance);
		public:

			/*
			* The dot product between a point and a plane is equal to the signed
			*	perpendicular distance between them
			*
			* When the normal is unit length, The dot product between a point and a plane
			*	is equal to the length of the projection of p onto n
			*
			* The sign of the distance given by The dot product between a point and a plane
			*	depends on the which side of the plane the point lies.
			*/
			inline static float Dot(const Plane& p, const Vector3D& v);

			inline static PlaneIntersectionResult IntersectSphereOnPlane(const Vector3D& center, const float radius, const Plane& plane);

			inline static PlaneIntersectionResult IntersectAABBOnPlane(const Vector3D& aabbMin, const Vector3D& aabbMax, Plane& plane);
			
			inline void Normalize();

			inline Vector3D GetNormal() const;

			inline Vector3D AbsNormal();

		};

		inline Plane::Plane() : X(0.0f), Y(0.0f), Z(0.0f), Distance(0.0f) { }

		inline Plane::Plane(float x, float y, float z, float distance)
			: X(x), Y(y), Z(z), Distance(distance) { }

		inline Plane::Plane(Vector3D& normal, float distance)
			: X(normal.X), Y(normal.Y), Z(normal.Z), Distance(distance) { }

		inline float Plane::Dot(const Plane& p, const Vector3D& v)
		{
			return p.X * v.X + p.Y * v.Y + p.Z * v.Z;
		}

		inline PlaneIntersectionResult Plane::IntersectSphereOnPlane(const Vector3D& center, const float radius, const Plane& plane)
		{
			float SphereCenterOffset = Plane::Dot(plane, center);
			float SphereSignedDistance = SphereCenterOffset - plane.Distance;

			if (SphereSignedDistance < -radius)
			{
				return PlaneIntersectionResult::Back;
			}
			else if (SphereSignedDistance > radius)
			{
				return PlaneIntersectionResult::Front;
			}

			return PlaneIntersectionResult::Intersection;
		}
		
		//inline PlaneIntersectionResult VrixicMath::IntersectAABBOnPlane(const Vector3D& aabbMin, const Vector3D& aabbMax, Plane& plane)
		//{
		//	Vector3D SphereCenter = (aabbMin + aabbMax) * 0.5f;
		//	Vector3D BoxExtents = aabbMax - SphereCenter;
		//
		//	float SphereProjectedRadius = Vector3D::DotProduct(plane.AbsNormal(), BoxExtents);
		//
		//	return IntersectSphereOnPlane(SphereCenter, SphereProjectedRadius, plane);
		//}

		inline PlaneIntersectionResult Plane::IntersectAABBOnPlane(const Vector3D& inCenter, const Vector3D& inExtents, Plane& inPlane)
		{
			float SphereProjectedRadius = Vector3D::DotProduct(inPlane.AbsNormal(), inExtents);
			return IntersectSphereOnPlane(inCenter, SphereProjectedRadius, inPlane);
		}

		inline void Plane::Normalize()
		{
			float Magnitude = (X * X + Y * Y + Z * Z);
			float R = 1 / (Magnitude + EPSILON);

			X *= R;
			Y *= R;
			Z *= R;
			Distance *= R;
		}

		inline Vector3D Plane::GetNormal() const
		{
			return Vector3D(X, Y, Z);
		}

		inline Vector3D Plane::AbsNormal()
		{
			return Vector3D(std::abs(X), std::abs(Y), std::abs(Z));
		}
	}
}
