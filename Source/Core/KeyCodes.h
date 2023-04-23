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
		/** Printable Keys->To Screen(Type) */

		Space		= 32,
		Apostrophe	= 39, // '
		Comma		= 44, // ,
		Minus		= 45,
		Period		= 46, // .
		Slash		= 47, // /

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

		Semicolon	= 59, // ;
		Equal		= 61, // =

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

		LeftBracket		= 91, // [
		BackSlash		= 92, /* \ */
		RightBracket	= 93, /* ] */
		Tilde			= 96, // `

		/** Function Keys */

		Escape			= 256,
		Enter			= 257,
		Tab				= 258,
		Backspace		= 259,
		Insert			= 260,
		Delete			= 261,
		ArrowRight		= 262,
		ArrowLeft		= 263,
		ArrowDown		= 264,
		ArrowUp			= 265,
		PageUp			= 266,
		PageDown		= 267,
		Home			= 268,
		End				= 269,
		CapsLock		= 280,
		ScrollLock		= 281,
		NumLock			= 282,
		PrintScreen		= 283,
		Pause			= 284,

		F1				= 290,
		F2				= 291,
		F3				= 292,
		F4				= 293,
		F5				= 294,
		F6				= 295,
		F7				= 296,
		F8				= 297,
		F9				= 298,
		F10				= 299,
		F11				= 300,
		F12				= 301,
		F13				= 302,
		F14				= 303,
		F15				= 304,
		F16				= 305,
		F17				= 306,
		F18				= 307,
		F19				= 308,
		F20				= 309,
		F21				= 310,
		F22				= 311,
		F23				= 312,
		F24				= 313,
		F25				= 314,

		NumPad0 = 320,
		NumPad1 = 321,
		NumPad2 = 322,
		NumPad3 = 323,
		NumPad4 = 324,
		NumPad5 = 325,
		NumPad6 = 326,
		NumPad7 = 327,
		NumPad8 = 328,
		NumPad9 = 329,

		NumPadPeriod		= 330,
		NumPadDivide		= 331,
		NumPadMultiply		= 332,
		NumPadSubtract		= 333,
		NumPadAdd			= 334,
		NumPadEnter			= 335,
		NumPadEqual			= 336,

		LeftShiftKey		= 340,
		LeftControlKey		= 341,
		LeftAlt				= 342,
		RightShiftKey		= 344,
		RightControlKey		= 345,
		RightAlt			= 346,
	};
}
