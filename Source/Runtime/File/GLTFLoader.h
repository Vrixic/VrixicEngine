/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Core.h>
#include <Misc/Logging/Log.h>
#include <Misc/Defines/StringDefines.h>
#include <Runtime/Core/Math/Vector3D.h>
#include <Runtime/Core/Math/Vector4D.h>
#include <Runtime/Core/Math/Matrix4D.h>
#include "FileHelper.h"
#include <Misc/Assert.h>

#include <External/json/Includes/nlohmann_json/json.hpp>
#include <fileapi.h>
#include <fstream>

namespace GLTF
{
    using json = nlohmann::json;

    struct VRIXIC_API FAccessor
    {
    public:
        enum class EComponentType
        {
            Byte = 5120,
            UnsignedByte = 5121,
            Short = 5122,
            UnsignedShort = 5123,
            UnsignedInt = 5125,
            Float = 5126
        };

        enum class EType
        {
            Scalar,
            Vec2,
            Vec3,
            Vec4,
            Mat2,
            Mat3,
            Mat4,
        };

        /** Index of buffer view */
        uint32 BufferView;

        /** Byte offset into the buffer view*/
        uint32 ByteOffset;

        /** Coomponent type of this accessor */
        EComponentType ComponentType;

        /** Count of data */
        uint32 Count;

        /** MIN/MAX omitted for now */

        /** The type of the data */
        EType Type;
    };

    struct VRIXIC_API FBufferView
    {
    public:
        enum class ETarget
        {
            VertexData = 34962, /*ArrayBuffer*/
            IndexData = 34963  /*ElementArrayBuffer*/
        };

        std::string Name;

        /** The index of the buffer */
        uint32 BufferIndex;

        /** The byte length of the buffer view */
        uint32 ByteLength;

        /** The byte offset into the buffer data */
        uint32 ByteOffset;

        uint32 ByteStride;

        /** Target of the buffer view */
        ETarget Target;
    };

    struct VRIXIC_API FBuffer
    {
    public:
        /** Byte length of the buffer */
        uint32 ByteLength;

        /** The bin file for the buffer */
        std::string Uri;
    };

    struct VRIXIC_API FImage
    {
    public:
        /** Buffer view for the image */
        uint32 BufferView;

        /** The Uri for the image */
        std::string Uri;
    };

    struct VRIXIC_API FTextureInfo
    {
    public:
        /** Index into the textures */
        uint32 Index;
        uint32 TexCoord;
    };

    struct VRIXIC_API FNormalTextureInfo
    {
        /** Index into the textures */
        uint32 Index;
        uint32 TexCoord;

        /**
        * Linearlys scales X and Y values of the normal vector (Should normalize the vector after scaling)...
        */
        float Scale;
    };

    struct VRIXIC_API FOcclusionTextureInfo
    {
        /** Index into the textures */
        uint32 Index;
        uint32 TexCoord;

        /**
        * Used to reduce the occulusion effect. Occulisions values 1.0 + Strength * (occulusionTexture - 1.0f);
        */
        float Strength;
    };

    struct VRIXIC_API FPBRMetallicRoughnessInfo
    {
        /** Base color of the material */
        Vector4D BaseColorFactor;
        FTextureInfo BaseColorTexture;

        float MetallicFactor;
        FTextureInfo MetallicRoughnessTexture;

        float RoughnessFactor;
    };

    struct VRIXIC_API FMaterial
    {
    public:
        enum class EAlphaMode
        {
            Invalid,
            Opaque,
            Mask,
            Blend
        };

        /** The name of the material */
        std::string Name;

        float AlphaCutoff;
        EAlphaMode AlphaMode;

        Vector3D EmissiveFactor;
        FTextureInfo EmissiveTexture;

        FNormalTextureInfo NormalTexture;
        FOcclusionTextureInfo OcclusionTexture;
        FPBRMetallicRoughnessInfo PBRMetallicRoughnessInfo;
    };

    struct VRIXIC_API FMeshPrimitiveAttribute
    {
    public:
        /** Key of the attribute: JOINTS_0...NORMAL...POSITION....TEXTCOORD_0..., etc*/
        std::string Key;

        /** Index into the accessors */
        uint32 AccessorIndex;
    };

    struct VRIXIC_API FMeshPrimitive
    {
    public:
        enum class EMode
        {
            Points = 0,
            Lines,
            LineLoop,
            LineStrip,
            Triangles,
            TriangleStrip,
            TriangleFan
        };

        /** All attributes for this mesh primitive */
        std::vector<FMeshPrimitiveAttribute> Attributes;

        /** The index into the indicies */
        uint32 IndiciesIndex;

        /** The index into the material data */
        uint32 MaterialIndex;

        EMode Mode;
    };

    struct VRIXIC_API FMesh
    {
    public:
        /** Name of the mesh*/
        std::string Name;

        /** All of the mesh primitives */
        std::vector<FMeshPrimitive> Primitives;
    };

    struct VRIXIC_API FCamera
    {
    public:
        std::string Name;
        bool bisOrthographic;
    };

    struct VRIXIC_API FNode
    {
    public:
        std::string Name;

        /** Index to the camera object referenced by this node */
        uint32 CameraIndex;

        /** All of the nodes children, their indicies*/
        std::vector<uint32> Children;

        Vector4D Rotation;
        Vector3D Scale;
        Vector3D Translation;
        Matrix4D Matrix;

        /** Index into the meshes data */
        uint32 MeshIndex;
    };

    struct VRIXIC_API FSampler
    {
    public:
        enum class EFilter
        {
            Nearest = 9728,

            Linear = 9729,

            NearestMipmapNearest = 9984,

            LinearMipmapNearest = 9985,

            NearestMipmapLinear = 9986,

            LinearMipmapLinear = 9987
        };

        enum class EWrap
        {
            ClampToEdge = 33071,

            MirroredRepeat = 33648,

            Repeat = 10497
        };

        EFilter MagFilter;
        EFilter MinFilter;
        EWrap WrapS;
        EWrap WrapT;
    };

    struct VRIXIC_API FScene
    {
    public:
        /** All the nodes (their indexes) that make up the scene */
        std::vector<uint32> Nodes;
    };

    struct VRIXIC_API FTexture
    {
    public:
        std::string Name;

        /** Index into the samplers */
        uint32 SamplerIndex;

        /** Aka. Source, index of the image this texture refers to */
        uint32 ImageIndex;
    };

    struct VRIXIC_API FWorld
    {
        std::vector<FAccessor> Accessors;

        std::vector<FBufferView> BufferViews;

        std::vector<FBuffer> Buffers;

        std::vector<FImage> Images;

        std::vector<FMaterial> Materials;

        std::vector<FMesh> Meshes;

        //std::vector<FCamera> Cameras;

        std::vector<FNode> Nodes;

        std::vector<FSampler> Samplers;

        std::vector<FScene> Scenes;

        std::vector<FTexture> Textures;
    };

    struct VRIXIC_API FJsonHelpers
    {
        /**
        * Trys to load a uint32 from json data, loads it if it exists, otherwise will not load
        */
        static void TryLoadUint32(json& inJsonData, const char* inDataKey, uint32& outValue)
        {
            // Check if it exists, if not exit 
            auto It = inJsonData.find(inDataKey);
            if (It == inJsonData.end())
            {
                outValue = UINT32_MAX;
                return;
            }

            outValue = inJsonData.value(inDataKey, 0);
        }

        /**
        * Trys to load a float from json data, loads it if it exists, otherwise will not load
        */
        static void TryLoadFloat(json& inJsonData, const char* inDataKey, float& outValue)
        {
            // Check if it exists, if not exit 
            auto It = inJsonData.find(inDataKey);
            if (It == inJsonData.end())
            {
                return;
            }

            outValue = inJsonData.value(inDataKey, 0.0f);
        }

        /**
        * Trys to load a string from json data, loads it if it exists, otherwise will not load
        */
        static void TryLoadString(json& inJsonData, const char* inDataKey, std::string& outValue)
        {
            // Check if it exists, if not exit 
            auto It = inJsonData.find(inDataKey);
            if (It == inJsonData.end())
            {
                return;
            }

            outValue = inJsonData.value(inDataKey, "");
        }

        /**
        * Trys to load int vector from json data, loads it if it exists, otherwise will not load
        */
        static void TryLoadIntVector(json& inJsonData, const char* inDataKey, std::vector<uint32>& outVector)
        {
            // Check if it exists, if not exit 
            auto It = inJsonData.find(inDataKey);
            if (It == inJsonData.end())
            {
                return;
            }

            json IntArray = inJsonData.at(inDataKey);

            outVector.resize(IntArray.size());
            for (uint32 i = 0; i < IntArray.size(); i++)
            {
                outVector[i] = IntArray.at(i);
            }
        }

        /**
        * Trys to load texture info from json data, loads it if it exists, otherwise will not load
        */
        static void TryLoadTextureInfo(json& inJsonData, const char* inDataKey, FTextureInfo& outTextureInfo)
        {
            // Check if it exists, if not exit 
            auto It = inJsonData.find(inDataKey);
            if (It == inJsonData.end())
            {
                outTextureInfo.Index = UINT32_MAX;
                return;
            }

            TryLoadUint32(inJsonData, "index", outTextureInfo.Index);
            TryLoadUint32(inJsonData, "texCoord", outTextureInfo.TexCoord);
        }

        /**
        * Trys to load normal texture info from json data, loads it if it exists, otherwise will not load
        */
        static void TryLoadNormalTextureInfo(json& inJsonData, const char* inDataKey, FNormalTextureInfo& outTextureInfo)
        {
            // Check if it exists, if not exit 
            auto It = inJsonData.find(inDataKey);
            if (It == inJsonData.end())
            {
                outTextureInfo.Index = UINT32_MAX;
                return;
            }

            TryLoadUint32(inJsonData, "index", outTextureInfo.Index);
            TryLoadUint32(inJsonData, "texCoord", outTextureInfo.TexCoord);
            TryLoadFloat(inJsonData, "scale", outTextureInfo.Scale);
        }

        /**
        * Trys to load normal texture info from json data, loads it if it exists, otherwise will not load
        */
        static void TryLoadOcclusionTextureInfo(json& inJsonData, const char* inDataKey, FOcclusionTextureInfo& outTextureInfo)
        {
            // Check if it exists, if not exit 
            auto It = inJsonData.find(inDataKey);
            if (It == inJsonData.end())
            {
                outTextureInfo.Index = UINT32_MAX;
                return;
            }

            TryLoadUint32(inJsonData, "index", outTextureInfo.Index);
            TryLoadUint32(inJsonData, "texCoord", outTextureInfo.TexCoord);
            TryLoadFloat(inJsonData, "strength", outTextureInfo.Strength);
        }

        /**
        * Trys to load normal texture info from json data, loads it if it exists, otherwise will not load
        */
        static void TryLoadPBRMetallicRoughnessInfo(json& inJsonData, const char* inDataKey, FPBRMetallicRoughnessInfo& outInfo)
        {
            // Check if it exists, if not exit 
            auto It = inJsonData.find(inDataKey);
            if (It == inJsonData.end())
            {
                return;
            }

            TryLoadVector4D(inJsonData, "index", outInfo.BaseColorFactor);
            TryLoadTextureInfo(inJsonData, "baseColorTexture", outInfo.BaseColorTexture);

            TryLoadFloat(inJsonData, "metallicFactor", outInfo.MetallicFactor);
            TryLoadTextureInfo(inJsonData, "metallicRoughnessTexture", outInfo.MetallicRoughnessTexture);

            TryLoadFloat(inJsonData, "roughnessFactor", outInfo.RoughnessFactor);
        }

        /**
        * Trys to load a vector3d from json data, loads it if it exists, otherwise will not load
        */
        static void TryLoadVector3D(json& inJsonData, const char* inDataKey, Vector3D& outVector)
        {
            // Check if it exists, if not exit 
            auto It = inJsonData.find(inDataKey);
            if (It == inJsonData.end())
            {
                outVector.X = 0.0f;
                outVector.Y = 0.0f;
                outVector.Z = 0.0f;
                return;
            }

            json FloatArray = inJsonData.at(inDataKey);

            if (FloatArray.size() > 3)
            {
                VE_ASSERT(false, VE_TEXT("[FJsonHelpers]: trying to input more data then is expected...."));
            }

            outVector.X = FloatArray[0];
            outVector.Y = FloatArray[1];
            outVector.Z = FloatArray[2];
        }

        /**
        * Trys to load a vector4d from json data, loads it if it exists, otherwise will not load
        */
        static void TryLoadVector4D(json& inJsonData, const char* inDataKey, Vector4D& outVector)
        {
            // Check if it exists, if not exit 
            auto It = inJsonData.find(inDataKey);
            if (It == inJsonData.end())
            {
                outVector.X = 0.0f;
                outVector.Y = 0.0f;
                outVector.Z = 0.0f;
                outVector.W = 0.0f;
                return;
            }

            json FloatArray = inJsonData.at(inDataKey);

            if (FloatArray.size() > 4)
            {
                VE_ASSERT(false, VE_TEXT("[FJsonHelpers]: trying to input more data then is expected...."));
            }

            outVector.X = FloatArray[0];
            outVector.Y = FloatArray[1];
            outVector.Z = FloatArray[2];
            outVector.W = FloatArray[3];
        }

        /**
       * Trys to load a matrix4d from json data, loads it if it exists, otherwise will not load
       */
        static void TryLoadMatrix4D(json& inJsonData, const char* inDataKey, Matrix4D& outMatrix)
        {
            // Check if it exists, if not exit 
            auto It = inJsonData.find(inDataKey);
            if (It == inJsonData.end())
            {
                return;
            }

            json FloatArray = inJsonData.at(inDataKey);

            if (FloatArray.size() > 16)
            {
                VE_ASSERT(false, VE_TEXT("[FJsonHelpers]: trying to input more data then is expected...."));
            }

            for (uint32 i = 0; i < 4; i++)
            {
                for (uint32 j = 0; j < 4; j++)
                {
                    outMatrix(i, j) = FloatArray[(j * 4) + i];
                }
            }
        }

        /**
        * Trys to load type from json data
        */
        static void TryLoadType(json& inJsonData, const char* inDataKey, FAccessor::EType& outType)
        {
            std::string StringType = inJsonData.value(inDataKey, "");

            if (StringType._Equal("SCALAR"))
            {
                outType = FAccessor::EType::Scalar;
            }
            else if (StringType._Equal("VEC2"))
            {
                outType = FAccessor::EType::Vec2;
            }
            else if (StringType._Equal("VEC3"))
            {
                outType = FAccessor::EType::Vec3;
            }
            else if (StringType._Equal("VEC4"))
            {
                outType = FAccessor::EType::Vec4;
            }
            else if (StringType._Equal("MAT2"))
            {
                outType = FAccessor::EType::Mat2;
            }
            else if (StringType._Equal("MAT3"))
            {
                outType = FAccessor::EType::Mat3;
            }
            else if (StringType._Equal("MAT4"))
            {
                outType = FAccessor::EType::Mat4;
            }
            else
            {
                // If it makes it this far then an error has occured as it has to be one of those options
                VE_ASSERT(false, VE_TEXT("[FJsonHelpers]: Cannot load type as its invalid...."));
            }
        }

        /**
        * Trys to load type from json data
        */
        static void TryLoadAlphaMode(json& inJsonData, const char* inDataKey, FMaterial::EAlphaMode& outAlphaMode)
        {
            std::string StringType = inJsonData.value(inDataKey, "");

            if (StringType._Equal("OPAQUE"))
            {
                outAlphaMode = FMaterial::EAlphaMode::Opaque;
            }
            else if (StringType._Equal("MASK"))
            {
                outAlphaMode = FMaterial::EAlphaMode::Mask;
            }
            else if (StringType._Equal("BLEND"))
            {
                outAlphaMode = FMaterial::EAlphaMode::Blend;
            }
            //else
            //{
            //    // If it makes it this far then an error has occured as it has to be one of those options
            //    VE_ASSERT(false, VE_TEXT("[FJsonHelpers]: Cannot load alpha mode as its invalid...."));
            //}
        }
    };

    struct VRIXIC_API FGLTFLoader
    {
    public:
        /**
        * Helper function which just loads all of the accessors from the parsed json data
        */
        static void LoadAccessors(json& inJsonData, std::vector<FAccessor>& outAccessors)
        {
            json Data = inJsonData["accessors"];

            uint32 DataSize = Data.size();
            outAccessors.resize(DataSize);
            memset(outAccessors.data(), 0, sizeof(FAccessor) * DataSize);

            for (uint32 i = 0; i < DataSize; i++)
            {
                FJsonHelpers::TryLoadUint32(Data[i], "bufferView", outAccessors[i].BufferView);
                FJsonHelpers::TryLoadUint32(Data[i], "byteOffset", outAccessors[i].ByteOffset);
                FJsonHelpers::TryLoadUint32(Data[i], "componentType", (uint32&)outAccessors[i].ComponentType);
                FJsonHelpers::TryLoadUint32(Data[i], "count", outAccessors[i].Count);
                FJsonHelpers::TryLoadType(Data[i], "type", outAccessors[i].Type);
            }
        }

        /**
        * Helper function which just loads all of the bufferviews from the parsed json data
        */
        static void LoadBufferViews(json& inJsonData, std::vector<FBufferView>& outBufferView)
        {
            json Data = inJsonData["bufferViews"];

            uint32 DataSize = Data.size();
            outBufferView.resize(DataSize);
            memset(outBufferView.data(), 0, sizeof(FBufferView) * DataSize);

            for (uint32 i = 0; i < DataSize; i++)
            {
                FJsonHelpers::TryLoadUint32(Data[i], "buffer", outBufferView[i].BufferIndex);
                FJsonHelpers::TryLoadUint32(Data[i], "byteLength", outBufferView[i].ByteLength);
                FJsonHelpers::TryLoadUint32(Data[i], "byteOffset", outBufferView[i].ByteOffset);
                FJsonHelpers::TryLoadUint32(Data[i], "byteStride", outBufferView[i].ByteStride);
                FJsonHelpers::TryLoadUint32(Data[i], "target", (uint32&)outBufferView[i].Target);
                FJsonHelpers::TryLoadString(Data[i], "name", outBufferView[i].Name);
            }
        }

        /**
        * Helper function which just loads all of the images from the parsed json data
        */
        static void LoadImages(json& inJsonData, std::vector<FImage>& outImages)
        {
            json Data = inJsonData["images"];

            uint32 DataSize = Data.size();
            outImages.resize(DataSize);
            memset(outImages.data(), 0, sizeof(FImage) * DataSize);

            for (uint32 i = 0; i < DataSize; i++)
            {
                FJsonHelpers::TryLoadUint32(Data[i], "bufferView", outImages[i].BufferView);
                FJsonHelpers::TryLoadString(Data[i], "uri", outImages[i].Uri);
            }
        }

        /**
        * Helper function which just loads all of the materials from the parsed json data
        */
        static void LoadMaterials(json& inJsonData, std::vector<FMaterial>& outMaterials)
        {
            json Data = inJsonData["materials"];

            uint32 DataSize = Data.size();
            outMaterials.resize(DataSize);
            memset(outMaterials.data(), 0, sizeof(FMaterial) * DataSize);

            for (uint32 i = 0; i < DataSize; i++)
            {
                FJsonHelpers::TryLoadString(Data[i], "name", outMaterials[i].Name);

                FJsonHelpers::TryLoadFloat(Data[i], "alphaCutoff", outMaterials[i].AlphaCutoff);
                FJsonHelpers::TryLoadAlphaMode(Data[i], "alphaMode", outMaterials[i].AlphaMode);

                FJsonHelpers::TryLoadVector3D(Data[i], "emissiveFactor", outMaterials[i].EmissiveFactor);
                FJsonHelpers::TryLoadTextureInfo(Data[i], "emissiveTexture", outMaterials[i].EmissiveTexture);

                FJsonHelpers::TryLoadNormalTextureInfo(Data[i], "normalTexture", outMaterials[i].NormalTexture);
                FJsonHelpers::TryLoadOcclusionTextureInfo(Data[i], "occlusionTexture", outMaterials[i].OcclusionTexture);
                FJsonHelpers::TryLoadPBRMetallicRoughnessInfo(Data[i], "pbrMetallicRoughness", outMaterials[i].PBRMetallicRoughnessInfo);
            }
        }

        /**
        * Helper function which just loads all of the mesh primitives from the parsed json data
        */
        static void LoadMeshPrimitives(json& inJsonData, std::vector<FMeshPrimitive> outMeshPrimitives)
        {
            json Data = inJsonData["primitives"];

            uint32 DataSize = Data.size();
            outMeshPrimitives.resize(DataSize);
            memset(outMeshPrimitives.data(), 0, sizeof(FMeshPrimitive) * DataSize);

            for (uint32 i = 0; i < DataSize; i++)
            {
                FJsonHelpers::TryLoadUint32(Data[i], "indices", outMeshPrimitives[i].IndiciesIndex);
                FJsonHelpers::TryLoadUint32(Data[i], "material", outMeshPrimitives[i].MaterialIndex);
                FJsonHelpers::TryLoadUint32(Data[i], "mode", (uint32&)outMeshPrimitives[i].Mode);

                json Attributes = inJsonData["attributes"];
                outMeshPrimitives[i].Attributes.resize(Attributes.size());
                memset(outMeshPrimitives[i].Attributes.data(), 0, sizeof(FMeshPrimitiveAttribute) * Attributes.size());

                uint32 Index = 0;
                for (auto JsonAttribute : Attributes.items())
                {
                    std::string Key = JsonAttribute.key();
                    outMeshPrimitives[i].Attributes[Index].Key = Key;
                    outMeshPrimitives[i].Attributes[Index++].AccessorIndex = JsonAttribute.value();
                }
            }
        }

        /**
        * Helper function which just loads all of the meshes from the parsed json data
        */
        static void LoadMeshes(json& inJsonData, std::vector<FMesh>& outMeshes)
        {
            json Data = inJsonData["meshes"];

            uint32 DataSize = Data.size();
            outMeshes.resize(DataSize);
            memset(outMeshes.data(), 0, sizeof(FMesh) * DataSize);

            for (uint32 i = 0; i < DataSize; i++)
            {
                FJsonHelpers::TryLoadString(Data[i], "name", outMeshes[i].Name);
                LoadMeshPrimitives(Data[i], outMeshes[i].Primitives);
            }
        }

        /**
        * Helper function which just loads all of the nodes from the parsed json data
        */
        static void LoadNodes(json& inJsonData, std::vector<FNode>& outNodes)
        {
            json Data = inJsonData["nodes"];

            uint32 DataSize = Data.size();
            outNodes.resize(DataSize);
            memset(outNodes.data(), 0, sizeof(FNode) * DataSize);

            for (uint32 i = 0; i < DataSize; i++)
            {
                FJsonHelpers::TryLoadString(Data[i], "name", outNodes[i].Name);

                FJsonHelpers::TryLoadUint32(Data[i], "camera", outNodes[i].CameraIndex);
                FJsonHelpers::TryLoadUint32(Data[i], "mesh", outNodes[i].MeshIndex);

                FJsonHelpers::TryLoadIntVector(Data[i], "children", outNodes[i].Children);

                FJsonHelpers::TryLoadVector3D(Data[i], "scale", outNodes[i].Scale);
                FJsonHelpers::TryLoadVector3D(Data[i], "translation", outNodes[i].Translation);
                FJsonHelpers::TryLoadVector4D(Data[i], "rotation", outNodes[i].Rotation);
                FJsonHelpers::TryLoadMatrix4D(Data[i], "matrix", outNodes[i].Matrix);
            }
        }

        /**
        * Helper function which just loads all of the samplers from the parsed json data
        */
        static void LoadSamplers(json& inJsonData, std::vector<FSampler>& outSamplers)
        {
            json Data = inJsonData["samplers"];

            uint32 DataSize = Data.size();
            outSamplers.resize(DataSize);
            memset(outSamplers.data(), 0, sizeof(FSampler) * DataSize);

            for (uint32 i = 0; i < DataSize; i++)
            {
                FJsonHelpers::TryLoadUint32(Data[i], "magFilter", (uint32&)outSamplers[i].MagFilter);
                FJsonHelpers::TryLoadUint32(Data[i], "minFilter", (uint32&)outSamplers[i].MinFilter);
                FJsonHelpers::TryLoadUint32(Data[i], "wrapS", (uint32&)outSamplers[i].WrapS);
                FJsonHelpers::TryLoadUint32(Data[i], "wrapT", (uint32&)outSamplers[i].WrapT);
            }
        }

        /**
        * Helper function which just loads all of the scenes from the parsed json data
        */
        static void LoadScenes(json& inJsonData, std::vector<FScene>& outScenes)
        {
            json Data = inJsonData["scenes"];

            uint32 DataSize = Data.size();
            outScenes.resize(DataSize);
            memset(outScenes.data(), 0, sizeof(FScene) * DataSize);

            for (uint32 i = 0; i < DataSize; i++)
            {
                FJsonHelpers::TryLoadIntVector(Data[i], "nodes", outScenes[i].Nodes);
            }
        }

        /**
        * Helper function which just loads all of the scenes from the parsed json data
        */
        static void LoadTextures(json& inJsonData, std::vector<FTexture>& outTextures)
        {
            json Data = inJsonData["textures"];

            uint32 DataSize = Data.size();
            outTextures.resize(DataSize);
            memset(outTextures.data(), 0, sizeof(FTexture) * DataSize);

            for (uint32 i = 0; i < DataSize; i++)
            {
                FJsonHelpers::TryLoadString(Data[i], "name", outTextures[i].Name);
                FJsonHelpers::TryLoadUint32(Data[i], "sampler", outTextures[i].SamplerIndex);
                FJsonHelpers::TryLoadUint32(Data[i], "source", outTextures[i].ImageIndex);
            }
        }

        /**
        * Helper function which just loads all of the buffers from the parsed json data
        */
        static void LoadBuffers(json& inJsonData, std::vector<FBuffer>& outBuffers)
        {
            json Data = inJsonData["buffers"];

            uint32 DataSize = Data.size();
            outBuffers.resize(DataSize);
            memset(outBuffers.data(), 0, sizeof(FBuffer) * DataSize);

            for (uint32 i = 0; i < DataSize; i++)
            {
                FJsonHelpers::TryLoadString(Data[i], "uri", outBuffers[i].Uri);
                FJsonHelpers::TryLoadUint32(Data[i], "byteLength", outBuffers[i].ByteLength);
            }
        }

        static FWorld LoadFromFile(const char* inFilePath)
        {
            FWorld World = { };

            /** Check if the file is open if not, return empty world */
            std::string FileStringData;
            std::string FilePath(inFilePath);
            if (!FileHelper::LoadFileToString(FileStringData, FilePath))
            {
                VE_CORE_LOG_FATAL(VE_TEXT("{0} file path does not exists..."), inFilePath);
                return World;
            }

            json ParsedGltfData = json::parse(FileStringData.data());

            if (ParsedGltfData.is_discarded())
            {
                VE_CORE_LOG_FATAL(VE_TEXT("{0} file could not be parsed by json parser..."), inFilePath);
                return World;
            }

            // Go through all the properties that were parsed and now fill up the world with the data 
            for (auto Properties : ParsedGltfData.items())
            {
                if (Properties.key() == "accessors")
                {
                    LoadAccessors(ParsedGltfData, World.Accessors);
                }
                else if (Properties.key()._Equal("bufferViews"))
                {
                    LoadBufferViews(ParsedGltfData, World.BufferViews);
                }
                else if (Properties.key()._Equal("buffers"))
                {
                    LoadBuffers(ParsedGltfData, World.Buffers);
                }
                else if (Properties.key()._Equal("images"))
                {
                    LoadImages(ParsedGltfData, World.Images);
                }
                else if (Properties.key()._Equal("materials"))
                {
                    LoadMaterials(ParsedGltfData, World.Materials);
                }
                else if (Properties.key()._Equal("meshes"))
                {
                    LoadMeshes(ParsedGltfData, World.Meshes);
                }
                else if (Properties.key()._Equal("nodes"))
                {
                    LoadNodes(ParsedGltfData, World.Nodes);
                }
                else if (Properties.key()._Equal("samplers"))
                {
                    LoadSamplers(ParsedGltfData, World.Samplers);
                }
                else if (Properties.key()._Equal("scenes"))
                {
                    LoadScenes(ParsedGltfData, World.Scenes);
                }
                else if (Properties.key()._Equal("textures"))
                {
                    LoadTextures(ParsedGltfData, World.Textures);
                }
            }

            return World;
        }
    };

} // namespace GLTF
