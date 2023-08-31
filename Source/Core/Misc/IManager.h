#pragma once

#include "Interface.h"

/**
* Defines a static manager 
* @note makes it easier to define a singleton manager..
*/
#define VRIXIC_STATIC_MANAGER(ManagerType) inline static ManagerType& Get() { static ManagerType Instance; return Instance; }

class VRIXIC_API IManager : public Interface
{
public:
	/**
	* Initializes the manager 
	*/
	virtual void Init() = 0;

	/**
	* Shutdowns the manager 
	*/
	virtual void Shutdown() = 0;
};
