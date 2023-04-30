/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Core.h>
#include <Misc/Defines/GenericDefines.h>

#include <string>
#include <unordered_map>

struct VRIXIC_API TextureHandle
{
public:
    /** The texture memory handle */
    TPointer<uint8> MemoryHandle;

    int32 Width;
    int32 Height;
    int32 BitsPerPixel;

    /* Size of the texture in Bytes */
    uint64 SizeInBytes;

public:
    TextureHandle() : Width(-1), Height(-1), BitsPerPixel(-1), SizeInBytes(0) { }
};

/**
* Base Resource manager class which different types of resource manager can inherit from
*/
class VRIXIC_API ResourceManager
{
public:
    static ResourceManager& Get()
    {
        static ResourceManager Instance;
        return Instance;
    }

    /**
    * Initializes the Resource Manager
    */
    void Init();

    /**
    * Shuts dows the resource manager
    */
    void Shutdown();

    /**
    * Loads a texture using the path passed in 
    * 
    * @param inTexturePath the path of the texture to load
    * @returns TextureHandle the handle to the texture allocated to memory 
    */
    TextureHandle& LoadTexture(std::string& inTexturePath);

    //void FreeTexture(TextureHandle);

private:
    /** A hash_map that contains all textures */
    std::unordered_map<std::string, TextureHandle> TexturesMap;

    /** Memory Heap for textures */
    TMemoryHeap<uint8>* TextureMemoryHeap;
};
