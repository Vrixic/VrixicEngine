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
		Button16 = 16,

		ButtonLeft = Button1,
		ButtonRight = Button2,
		ButtonMiddle = Button16
	};
}
