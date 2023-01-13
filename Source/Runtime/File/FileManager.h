#pragma once
#include "FileReader.h"
#include <string>

/**
* Singleton class for containing file informations
*/
class FileManager
{
private:
	static std::string ProjectPath;
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

	FileReader CreateFileReader(std::string inFilePath)
	{
		FileReader Reader(inFilePath);
		return Reader;
	}

public:
	static const std::string* GetProjectPath() 
	{
		return &ProjectPath;
	}
};