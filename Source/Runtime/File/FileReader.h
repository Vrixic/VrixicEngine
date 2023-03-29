/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Core.h>
#include <Misc/Defines/GenericDefines.h>

#include <string>
#include <fstream>

enum class EFileOpenFlags
{
	Read, // Read the file
	Write, // Write to file, overwrites its previous contents 
	Append // Add to the pre-existing contents of the file
};

enum class EFileSeek
{
	Begin,
	Current,
	End
};

enum class EFileReadResult
{
	Success,
	Error,
	OutOfBytesToRead
};

/**
* Represents a file, which can be opened, read, writted to, and  closed...
*/
class VRIXIC_API FileReader
{
private:
	std::string Path;
	std::ifstream Handle;

public:
	FileReader(std::string inPath)
	{
		Handle.open(inPath);
	}

	~FileReader()
	{
		Handle.close();
	}

public:
	/**
	* To to a section into the file 
	* 
	* @param inOffset - offset reative to EFileSeek param 'inFileSeek'
	* @param inFileSeek - position where it should seek to 
	*/
	void Seek(uint32 inOffset, EFileSeek inFileSeek)
	{
		switch (inFileSeek)
		{
		case EFileSeek::Begin:
			Handle.seekg(inOffset, std::ios::beg);
			break;
		case EFileSeek::End:
			Handle.seekg(inOffset, std::ios::end);
			break;
		}		
	}

	/**
	* Reads bytes from file
	*
	* @param outResult - bytes read will be stored in this variable
	* @param inSize - count of bytes to read 
	*/
	void Read(char* outResult, uint64 inSize)
	{
		Handle.read(outResult, inSize);
	}

	/**
	* Read a byte from file
	*
	* @param outResult - byte read will be stored in this variable
	*/
	EFileReadResult Read8(uint8* outResult)
	{
		if (Handle.eof())
		{
			return EFileReadResult::OutOfBytesToRead;
		}

		Handle.read((char*)outResult, sizeof(uint8));

		return EFileReadResult::Success;
	}

	/**
	* Reads four bytes from file
	*
	* @param outResult - bytes read will be stored in this variable
	*/
	EFileReadResult Read32(uint32* outResult)
	{
		if (Handle.eof())
		{
			return EFileReadResult::OutOfBytesToRead;
		}

		Handle.read((char*)outResult, sizeof(uint32));

		return EFileReadResult::Success;
	}

	/**
	* @return uint64 - return the position of the current character
	*/
	uint64 Tell()
	{
		return Handle.tellg();
	}

	void Close()
	{
		Handle.close();
	}

public:

	bool IsOpen()
	{
		return Handle.is_open();
	}

};
