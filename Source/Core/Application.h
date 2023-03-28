#pragma once
#include "Core/Core.h"
#include <Core/Windows/Window.h>
#include "Editor/GameEditor.h"
#include <Misc/Defines/GenericDefines.h>

#include <iostream>

class VRIXIC_API Application
{
	friend class VGameEngine;

private:
	static Application* ApplicationPtr; // The application pointer

	TUniquePtr<Window> WindowPtr; // Pointer to window

	TSharedPtr<VGameEngine> GameEngine;

#ifdef _EDITOR
	TUniquePtr<VGameEditor> GameEditor;
#endif

	bool bIsRunning;
public:
	Application();
	virtual ~Application();

	void OnEvent(WindowEvent& inEvent);

	void Run();
};

