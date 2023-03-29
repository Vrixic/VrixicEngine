/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include "Matrix4D.h"

namespace Vrixic
{
	namespace Math
	{
		/* Represents a perspective projection matrix */
		struct VRIXIC_API ProjectionMatrix4D : public Matrix4D
		{
		public:
			inline ProjectionMatrix4D() : Matrix4D() { };

			/* By default its is a Left Handed Matrix, for DirectX */
			inline ProjectionMatrix4D(float aspectRatio, float verticalFOVInDegs, float nearZ, float farZ);

			/* Factory function for making a left handed perspective projection matrix for DirectX */
			inline static ProjectionMatrix4D MakeProjectionDirectXLH(float aspectRatio, float verticalFOVInDegs, float nearZ, float farZ);
			
			/* Factory function for making a right handed perspective projection matrix for DirectX */
			inline static ProjectionMatrix4D MakeProjectionDirectXRH(float aspectRatio, float verticalFOVInDegs, float nearZ, float farZ);

			/* Factory function for making a left handed perspective projection matrix for Vulkan */
			inline static ProjectionMatrix4D MakeProjectionVulkanLH(float aspectRatio, float verticalFOVInDegs, float nearZ, float farZ);
		};

		inline ProjectionMatrix4D::ProjectionMatrix4D(float aspectRatio, float verticalFOVInDegs, float nearZ, float farZ)
		{
			float Rads = MathUtils::DegreesToRadians(verticalFOVInDegs * 0.5f);
			float Height = 1.0f / std::tanf(Rads);
			
			float FarRange = farZ / (farZ - nearZ);
			
			M[0][0] = Height / aspectRatio;
			M[0][1] = 0.0f;
			M[0][2] = 0.0f;
			M[0][3] = 0.0f;

			M[1][0] = 0.0f;
			M[1][1] = Height;
			M[1][2] = 0.0f;
			M[1][3] = 0.0f;

			M[2][0] = 0.0f;
			M[2][1] = 0.0f;
			M[2][2] = FarRange;
			M[2][3] = 1.0f;

			M[3][0] = 0.0f;
			M[3][1] = 0.0f;
			M[3][2] = -nearZ * FarRange;
			M[3][3] = 0.0f;
		}

		inline ProjectionMatrix4D ProjectionMatrix4D::MakeProjectionDirectXLH(float aspectRatio, float verticalFOVInDegs, float nearZ, float farZ)
		{
			return ProjectionMatrix4D(aspectRatio, verticalFOVInDegs, nearZ, farZ);
		}

		inline ProjectionMatrix4D ProjectionMatrix4D::MakeProjectionDirectXRH(float aspectRatio, float verticalFOVInDegs, float nearZ, float farZ)
		{
			ProjectionMatrix4D Result = { };
			float Rads = MathUtils::DegreesToRadians(verticalFOVInDegs * 0.5f);
			float Height = 1.0f / std::tanf(Rads);

			float FarRange = farZ / (nearZ - farZ);

			Result.M[0][0] = Height / aspectRatio;
			Result.M[0][1] = 0.0f;
			Result.M[0][2] = 0.0f;
			Result.M[0][3] = 0.0f;

			Result.M[1][0] = 0.0f;
			Result.M[1][1] = Height;
			Result.M[1][2] = 0.0f;
			Result.M[1][3] = 0.0f;

			Result.M[2][0] = 0.0f;
			Result.M[2][1] = 0.0f;
			Result.M[2][2] = FarRange;
			Result.M[2][3] = -1.0f;

			Result.M[3][0] = 0.0f;
			Result.M[3][1] = 0.0f;
			Result.M[3][2] = nearZ * FarRange;
			Result.M[3][3] = 0.0f;

			return Result;
		}
		
		inline ProjectionMatrix4D ProjectionMatrix4D::MakeProjectionVulkanLH(float aspectRatio, float verticalFOVInDegs, float nearZ, float farZ)
		{
			ProjectionMatrix4D Result = { };
			float Rads = MathUtils::DegreesToRadians(verticalFOVInDegs * 0.5f);
			float Height = 1.0f / std::tanf(Rads);

			float FarRange = farZ / (farZ - nearZ);

			Result.M[0][0] = Height / aspectRatio;
			Result.M[0][1] = 0.0f;
			Result.M[0][2] = 0.0f;
			Result.M[0][3] = 0.0f;

			Result.M[1][0] = 0.0f;
			Result.M[1][1] = -Height; // Vulkan clip space for Y is [1, -1] -> its has to be flipped
			Result.M[1][2] = 0.0f;
			Result.M[1][3] = 0.0f;

			Result.M[2][0] = 0.0f;
			Result.M[2][1] = 0.0f;
			Result.M[2][2] = FarRange;
			Result.M[2][3] = 1.0f;

			Result.M[3][0] = 0.0f;
			Result.M[3][1] = 0.0f;
			Result.M[3][2] = -nearZ * FarRange;
			Result.M[3][3] = 0.0f;

			return Result;
		}
	}
}
