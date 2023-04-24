#include <Core/VrixicEngine.h>

#if _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

class Sandbox : public Application
{
public:
	Sandbox()
	{

	}

	~Sandbox()
	{

	}
};

int main()
{
	// Logging and Memory Output
#if _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
#endif // _DEBUG

	Sandbox* SB = new Sandbox();
	SB->Run();
	delete SB;

#if _DEBUG
	_CrtDumpMemoryLeaks();
#endif // _DEBUG
}

