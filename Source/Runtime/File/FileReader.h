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
public:
    FileReader(std::string inPath)
    {
        Handle = { };
        Handle.open(inPath);

        // Get the size of the file 
        SeekEnd(0);
        SizeInBytes = Tell();
        SeekBegin(0);
    }

    ~FileReader()
    {
        Handle.close();
    }

public:
    /**
    * Seek to a section into the file
    *
    * @param inOffset - offset reative to EFileSeek param 'inFileSeek'
    * @param inFileSeek - position where it should seek to
    */
    void Seek(uint64 inOffset, EFileSeek inFileSeek)
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
    * Seek to a the beginning of a file
    *
    * @param inOffset - offset reative to the beginning of the file 
    */
    inline void SeekBegin(uint64 inOffset)
    {
        Handle.seekg(inOffset, std::ios::beg);
    }

    /**
    * Seek to a the end of a file
    *
    * @param inOffset - offset reative to end of the file 
    */
    inline void SeekEnd(uint64 inOffset)
    {
        Handle.seekg(inOffset, std::ios::end);
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

    inline void Close()
    {
        Handle.close();
    }

public:

    /**
    * @return uint64 - return the position of the current character
    */
    inline uint64 Tell()
    {
        return Handle.tellg();
    }

    inline uint64 Size() const
    {
        return SizeInBytes;
    }

    inline bool IsOpen() const 
    {
        return Handle.is_open();
    }

private:
    std::string Path;
    std::ifstream Handle;

    /** Size of the file in bytes */
    uint64 SizeInBytes;
};
