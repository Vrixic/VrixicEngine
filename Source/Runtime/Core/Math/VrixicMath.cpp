#include "VrixicMath.h"

/*
* Many math data structures implemenation are located here for ease of access
*/

namespace Vrixic
{
	namespace Math
	{
		inline float ManhattanDistance(const Vector3D& v1, const Vector3D& v2)
		{
			float DeltaX = std::abs(v2.X - v1.X);
			float DeltaY = std::abs(v2.Y - v1.Y);
			float DeltaZ = std::abs(v2.Z - v1.Z);

			return DeltaX + DeltaY + DeltaZ;
		}
	}
}

