/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include "Core/Core.h"
#include <Core/Windows/Window.h>
#include "Editor/GameEditor.h"
#include "Events/KeyEvent.h"

#include <iostream>

class VRIXIC_API Application
{
private:
	static Application* ApplicationPtr; // The application pointer

	TUniquePtr<IWindow> WindowPtr; // Pointer to window

	TSharedPtr<VGameEngine> GameEngine;

//#ifdef _EDITOR
//	TUniquePtr<VGameEditor> GameEditor;
//#endif

	bool bIsRunning;
public:
	Application();
	virtual ~Application();

	void OnEvent(WindowEvent& inEvent);

	bool OnKeyDownEvent(KeyPressedEvent& inKeyEvent);

	void Run();

	static Application* Get()
	{
		return ApplicationPtr;
	}

	IWindow& GetWindow() const
	{
		return *WindowPtr;
	}
};

