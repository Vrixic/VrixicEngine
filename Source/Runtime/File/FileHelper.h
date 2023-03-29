/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include "FileManager.h"

#include <string>

/**
* Static class which has helper functions for reading files 
*/
class VRIXIC_API FileHelper
{
	// Loads a text file into a string 
	static bool LoadFileToString(std::string& outResult, std::string& inFilePath)
	{
		FileReader Reader = FileManager::GetInstance().CreateFileReader(inFilePath);
		Reader.Seek(0, EFileSeek::End);

		uint64 Size = Reader.Tell();
		if (Size == 0)
		{
			return false;
		}

		Reader.Seek(0, EFileSeek::Begin);

		Reader.Read(&outResult[0], Size);

		Reader.Close();
		return true;
	}
};
