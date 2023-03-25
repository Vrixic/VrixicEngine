#include <Core/VrixicEngine.h>

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
	Sandbox* SB = new Sandbox();
	SB->Run();
	delete SB;
}

