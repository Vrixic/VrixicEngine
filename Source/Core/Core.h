#pragma once

#ifdef PLATFORM_WINDOWS
	#ifdef VE_BUILD_DLL
		#define VRIXIC_API __declspec(dllexport)
	#else
		#define VRIXIC_API __declspec(dllimport)
	#endif
#else
	#error Vrixic Engine for now, only supports Windows Platform!
#endif
