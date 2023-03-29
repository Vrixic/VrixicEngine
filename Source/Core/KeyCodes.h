/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Misc/Defines/GenericDefines.h>
#include "Events/KeyEvent.h"

/**
* All keys 
*/
namespace Key
{
	using KeyCode  = uint16;

	enum : KeyCode
	{ 
		Backspace	= 8,
		Tab			= 9,
		Enter		= 10,
		CapsLock	= 20,
		Escape		= 27,
		Space		= 32,
		LeftArrow	= 37,
		UpArrow		= 38,
		RightArrow	= 39,
		DownArrow	= 40,

		Delete		= 46,

		Zero		= 48, // 0
		One			= 49, // 1
		Two			= 50, // 2
		Three		= 51, // 3
		Four		= 52, // 4
		Five		= 53, // 5
		Six			= 54, // 6
		Seven		= 55, // 7
		Eight		= 56, // 8
		Nine		= 57, // 9

		A = 65, 
		B = 66,
		C = 67,
		D = 68,
		E = 69,
		F = 70,
		G = 71,
		H = 72,
		I = 73,
		J = 74,
		K = 75,
		L = 76,
		M = 77,
		N = 78,
		O = 79,
		P = 80,
		Q = 81,
		R = 82,
		S = 83,
		T = 84,
		U = 85,
		V = 86,
		W = 87,
		X = 88,
		Y = 89,
		Z = 90,

		NumPad0 =  96,
		NumPad1 =  97,
		NumPad2 =  98,
		NumPad3 =  99,
		NumPad4 = 100,
		NumPad5 = 101,
		NumPad6 = 102,
		NumPad7 = 103,
		NumPad8 = 104,
		NumPad9 = 105,

		Multiply	= 106,
		Add			= 107,
		Subtract	= 109,
		Divide		= 111,


		Separator	= 108,
		Decimal		= 110,

		F1		= 112,
		F2		= 113,
		F3		= 114,
		F4		= 115,
		F5		= 116,
		F6		= 117,
		F7		= 118,
		F8		= 119,
		F9		= 120,
		F10		= 121,
		F11		= 122,
		F12		= 123,
		F13		= 124,
		F14		= 125,
		F15		= 126,
		F16		= 127,
		F17		= 128,
		F18		= 129,
		F19		= 130,
		F20		= 131,
		F21		= 132,
		F22		= 133,
		F23		= 134,
		F24		= 135,
		
		NumLock = 144,
		
		LeftShiftKey		= 160,
		RightShiftKey		= 161,

		LeftControlKey		= 162,
		RightControlKey		= 163,

		LeftAlt				= 164,
		RightAlt			= 165,
	};
}
