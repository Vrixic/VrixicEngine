#pragma once
#include "Core/Core.h"
#include <Core/Windows/Window.h>
#include <Misc/Defines/GenericDefines.h>

#include <iostream>

class VRIXIC_API Application
{
private:
	static Application* ApplicationPtr; // The application pointer

	TUniquePtr<Window> WindowPtr;

	bool bIsRunning;
public:
	Application();
	virtual ~Application();

	void OnEvent(WindowEvent& inEvent);

	void Run();
};

