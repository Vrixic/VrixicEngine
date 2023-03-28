#include "GameEditor.h"
#include <External/imgui/Includes/imgui.h>

VGameEditor::VGameEditor()
{
	GameEngine = nullptr;
}

VGameEditor::~VGameEditor()
{
	Shutdown();
}

void VGameEditor::Init(TSharedPtr<VGameEngine> inGameEngine)
{
	GameEngine = inGameEngine;

	// attach imgui to game editor
}

void VGameEditor::Tick()
{
	GameEngine->Tick();
}

void VGameEditor::Shutdown()
{

}

void VGameEditor::OnEvent(WindowEvent& inWindowEvent)
{
	WindowEventDispatcher EventDispatcher(inWindowEvent);
	EventDispatcher.Dispatch<WindowResizeEvent>(VE_BIND_EVENT_FUNC(OnWindowResized));
}

bool VGameEditor::OnWindowResized(WindowResizeEvent& inWindowResizeEvent)
{
	RenderViewportSize NewWindowSize = { inWindowResizeEvent.GetWidth(), inWindowResizeEvent.GetHeight() };
	GameEngine->GetRenderer().Get()->OnRenderViewportResized(NewWindowSize);

	return true;
}
