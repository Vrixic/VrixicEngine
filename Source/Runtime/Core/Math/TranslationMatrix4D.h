#pragma once
#include "Matrix4D.h"

namespace Vrixic
{
	namespace Math
	{
		/* Represents a translation matrix */
		struct VRIXIC_API TranslationMatrix4D : public Matrix4D
		{
			inline TranslationMatrix4D(const Vector3D& translationVector);

			/* A Factory function to easily make a translation matrix */
			inline static TranslationMatrix4D Make(const Vector3D& translationVector);
		};

		inline TranslationMatrix4D::TranslationMatrix4D(const Vector3D& translationVector)
			: Matrix4D(1.0f, 0.0f, 0.0f, 0.0f,
					   0.0f, 1.0f, 0.0f, 0.0f, 
					   0.0f, 0.0f, 1.0f, 0.0f,
					   translationVector.X, translationVector.Y, translationVector.Z, 1.0f) { }

		inline TranslationMatrix4D TranslationMatrix4D::Make(const Vector3D& translationVector)
		{
			return TranslationMatrix4D(translationVector);
		}
	}
}
