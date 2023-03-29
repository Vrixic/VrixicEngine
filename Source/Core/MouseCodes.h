#pragma once
#include <Misc/Defines/GenericDefines.h>
#include "Events/KeyEvent.h"

/**
* All keys
*/
namespace Mouse
{
	using MouseCode = uint16;

	enum : MouseCode
	{
		Button0 = 0,
		Button1 = 1,
		Button2 = 2,
		Button3 = 3,
		Button4 = 4,

		ButtonLeft = Button0,
		ButtonRight = Button1,
		ButtonMiddle = Button2,
	};
}
