/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#include "ResourceManager.h"
#include <Runtime/Memory/Core/MemoryManager.h>

#include <External/stb/Includes/stb_image.h>

void ResourceManager::Init()
{
    TextureMemoryView = { };
    TextureMemoryView.MemorySize = MEBIBYTES_TO_BYTES(250);
    TextureMemoryView.MemoryHandle = TPointer<uint8>(MemoryManager::Get().MallocAligned<uint8>(TextureMemoryView.MemorySize, 1));

    VertexMemoryView = { };
    VertexMemoryView.MemorySize = MEBIBYTES_TO_BYTES(250);
    VertexMemoryView.MemoryHandle = TPointer<PBRVertex>(MemoryManager::Get().MallocAligned<PBRVertex>(VertexMemoryView.MemorySize, sizeof(PBRVertex)));

    IndexMemoryView = { };
    IndexMemoryView.MemorySize = MEBIBYTES_TO_BYTES(100);
    IndexMemoryView.MemoryHandle = TPointer<uint32>(MemoryManager::Get().MallocAligned<uint32>(IndexMemoryView.MemorySize, sizeof(uint32)));
}

void ResourceManager::Shutdown()
{
}

TextureHandle& ResourceManager::LoadTexture(std::string& inTexturePath)
{
    if (TexturesMap.find(inTexturePath) != TexturesMap.end())
    {
        return TexturesMap[inTexturePath];
    }

    VE_CORE_LOG_INFO(VE_TEXT("[ResourceManager]: Loading Texture {0} "), inTexturePath);

    TextureHandle Handle = { };

    // Load the texture 
    uint8* TextureMemory = stbi_load(inTexturePath.c_str(), &Handle.Width, &Handle.Height, &Handle.BitsPerPixel, 4);

    VE_ASSERT(TextureMemory != nullptr, VE_TEXT("[ResourceManager]: Failed to load texture: {0}"), inTexturePath)

    Handle.SizeInBytes = Handle.Width * Handle.Height * 4;

    Handle.MemoryViewHandle = TextureMemoryView.MemoryHandle;
    Handle.MemoryIndex = TextureMemoryView.Malloc(Handle.SizeInBytes);

    VE_CORE_LOG_INFO(VE_TEXT("[TextureMemoryView]: Loaded Texture with num bytes: {0} "), Handle.SizeInBytes);
    //Handle.MemoryHandle = TPointer<uint8>(MemoryManager::Get().MallocAligned<uint8>(Handle.SizeInBytes, 4));

    // Expensive but for now we will have to do it as I still need to implement my very own image decoders 
    // that will internally use the memory manager 
    memcpy(Handle.GetMemoryHandle(), TextureMemory, Handle.SizeInBytes);

    // free stb image memory 
    stbi_image_free(TextureMemory);

    // Then insert it into the TexturesMap
    TexturesMap.insert(std::make_pair(inTexturePath, Handle));

    return TexturesMap[inTexturePath];
}

