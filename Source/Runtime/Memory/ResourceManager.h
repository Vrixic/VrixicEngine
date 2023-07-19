/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel) 
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Core.h>
#include <Misc/Assert.h>
#include <Misc/Defines/StringDefines.h>
#include <Runtime/Graphics/Vertex.h>

#include <string>
#include <unordered_map>

struct VRIXIC_API TextureResourceHandle
{
    friend class ResourceManager;
public:
    int32 Width;
    int32 Height;
    int32 BitsPerPixel;

    /* Size of the texture in Bytes */
    uint64 SizeInBytes;

public:
    TextureResourceHandle() : Width(-1), Height(-1), BitsPerPixel(-1), SizeInBytes(0), MemoryIndex(-1) { }

    uint8* GetMemoryHandle() const
    {
        return MemoryViewHandle.Get() + MemoryIndex;
    }

private:
    /** The texture memory handle */
    TPointer<uint8> MemoryViewHandle;
    uint64 MemoryIndex;
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
    TextureResourceHandle& LoadTexture(const std::string& inTexturePath);

    //void FreeTexture(TextureHandle);

private:
    /** A hash_map that contains all textures */
    std::unordered_map<std::string, TextureResourceHandle> TexturesMap;

    /**
    * A view into aligned memory.. For specific uses..
    */
    template<typename T>
    struct VRIXIC_API HMemoryView
    {
    public:
        TPointer<T> MemoryHandle;
        uint64 MemoryUsed;
        uint64 MemorySize;

    public:
        HMemoryView() : MemoryHandle(nullptr), MemoryUsed(0), MemorySize(0) { }
        HMemoryView(T** inMemoryHandle, uint64 inMemorySize) : MemoryHandle(inMemoryHandle), MemoryUsed(0), MemorySize(inMemorySize) { }

        uint64 Malloc(uint64 inSizeInBytes)
        {
            // Check if we can allocate enough memory
            VE_ASSERT((MemoryUsed + inSizeInBytes) < MemorySize, VE_TEXT("[HMemoryView]: Out of memory; Memory OverFlow!"));

            uint64 MemoryIndex = MemoryUsed;
            MemoryUsed += inSizeInBytes;

            return MemoryIndex;
        }
    };
    /** Memory View for Textures */
    HMemoryView<uint8> TextureMemoryView;

    /** Memory View for vertices (Vertex Buffer) */
    HMemoryView<FPBRVertex> VertexMemoryView;

    /** Memory View for index data (Index Buffer) */
    HMemoryView<uint32> IndexMemoryView;
};
