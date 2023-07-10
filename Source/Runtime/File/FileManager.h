/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include "FileReader.h"

#include <string>

/**
* Singleton class for containing file informations
*/
class VRIXIC_API FileManager
{
public:
	static FileManager GetInstance()
	{
		static FileManager Instance;
		return Instance;
	}

public:

	/**
	* Initializes the file manager with the path to the project 	* 
	* SHOULD ONLY BE CALLED ONCE
	*/
	static void Init(std::string& inProjectPath)
	{
		inProjectPath = ProjectPath;
	}

	/*FileReader CreateFileReader(std::string inFilePath)
	{
		FileReader Reader(inFilePath);
		return Reader;
	}*/

public:
	static const std::string* GetProjectPath() 
	{
		return &ProjectPath;
	}

private:
    static std::string ProjectPath;
};
