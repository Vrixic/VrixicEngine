#pragma once

#include "VrixicMathHelper.h"

#include "Vector3D.h"
#include "Vector4D.h"
#include "Matrix4D.h"
#include "Plane.h"
#include "Quat.h"

#define VM Vrixic::Math

namespace Vrixic
{
	namespace Math
	{
		/* Manhattan distance -> non-accurate, but fast distance calculation*/
		static float ManhattanDistance(const Vector3D& v1, const Vector3D& v2);
	}
}
