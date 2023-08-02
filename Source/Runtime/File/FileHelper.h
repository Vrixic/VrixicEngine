/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include "FileReader.h"

#include <string>

/**
* Static class which has helper functions for reading files
*/
class VRIXIC_API FileHelper
{
public:
    /**
    * Loads a text file into a string
    */
    static bool LoadFileToString(std::string& outResult, std::string& inFilePath)
    {
        FileReader Reader(inFilePath); //= FileManager::GetInstance().CreateFileReader(inFilePath);
        if (!Reader.IsOpen())
        {
            return false;
        }

        uint64 Size = Reader.Size();
        if (Size == 0)
        {
            return false;
        }

        outResult.resize(Size + 1);
        Reader.Read(&outResult[0], Size);

        Reader.Close();
        return true;
    }

    /**
    * Loads a file into a contiguos array of bytes
    * @note: assumes that memory is already allocated
    */
    static bool LoadFileToBytes(char* outResult, std::string& inFilePath)
    {
        FileReader Reader(inFilePath); //= FileManager::GetInstance().CreateFileReader(inFilePath);
        if (!Reader.IsOpen())
        {
            return false;
        }

        uint64 Size = Reader.Size();
        if (Size == 0)
        {
            return false;
        }

        Reader.Read(&outResult[0], Size);
        Reader.Close();
        return true;
    }

    /**
    * Overwrites bytes to the file specified
    */
    static bool WriteBytesToFile(char* inData, uint64 inDataSize, std::string& inFileToWrite)
    {
        FILE* File = fopen(inFileToWrite.c_str(), "wb");

        if (File == nullptr) return false;

        fwrite(inData, inDataSize, 1, File);
        fclose(File);

        return true;
    }

    /**
    * Creates a folder[directory], only Window Supported
    */
    static bool CreateFolder(const std::string& inFilePath)
    {
#if defined(_WIN64)
        int Result = CreateDirectoryA(inFilePath.c_str(), NULL);
        return Result != 0;
#else
        VE_STATIC_ASSERT(false, VE_TEXT("[FileHelper]: Only windows folder supported..."));
#endif // _WIN64
    }

    /**
    * Returns as an out param the current directory path 
    */
    static void GetCurrentFolder(char* outDirectoryPath)
    {
#if defined(_WIN64)
        DWORD written_chars = GetCurrentDirectoryA(512, outDirectoryPath);
        outDirectoryPath[written_chars] = 0;
#else
        VE_STATIC_ASSERT(false, VE_TEXT("[FileHelper]: Only windows folder supported..."));
#endif // _WIN64
    }

    /**
    * Sets the current directory to the one specified if it is valid 
    */
    static void SetCurrentFolder(const std::string inFilePath)
    {
#if defined(_WIN64)
        if (!SetCurrentDirectoryA(inFilePath.c_str())) {
            VE_ASSERT(false, VE_TEXT("[FileHelper]: Cannot change current folder[directory] to {0}"), inFilePath.c_str());
        }
#else
        VE_STATIC_ASSERT(false, VE_TEXT("[FileHelper]: Only windows folder supported..."));
#endif // _WIN64
    }

    /**
    * Checks if a folder exists
    */
    static bool DoesFolderExist(const std::string& inFilePath)
    {
#if defined(_WIN64)
        WIN32_FILE_ATTRIBUTE_DATA Data;
        return GetFileAttributesExA(inFilePath.c_str(), GetFileExInfoStandard, &Data);
#else
        VE_STATIC_ASSERT(false, VE_TEXT("[FileHelper]: Only windows folder supported..."));
#endif // _WIN64
    }

    /**
    * Deletes a folder[directory], only Window Supported
    */
    static bool DeleteFolder(const std::string& inFilePath)
    {
#if defined(_WIN64)
        int Result = RemoveDirectoryA(inFilePath.c_str());
        return Result != 0;
#else
        VE_STATIC_ASSERT(false, VE_TEXT("[FileHelper]: Only windows folder supported..."));
#endif // _WIN64
    }

    /**
    * Checks if a file exists
    */
    static bool DoesFileExist(const std::string& inFilePath)
    {
#if defined(_WIN64) || defined(_WIN32)
        WIN32_FILE_ATTRIBUTE_DATA Data;
        return GetFileAttributesExA(inFilePath.c_str(), GetFileExInfoStandard, &Data);
#else
        FileReader Reader(inFilePath);
        bool IsOpen = Reader.IsOpen();
        Reader.Close();

        return IsOpen;
#endif // _WIN64
    }

    /**
    * Deletes a file, only Window Supported
    */
    static bool DeleteFileFromPath(const std::string& inFilePath)
    {
        int Result = remove(inFilePath.c_str());
        return Result != 0;
    }
};
