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
            Invalid = -1,
            Byte = 5120,
            UnsignedByte = 5121,
            Short = 5122,
            UnsignedShort = 5123,
            UnsignedInt = 5125,
            Float = 5126
        };

        enum class EType
        {
            Invalid = -1,
            Scalar,
            Vec2,
            Vec3,
            Vec4,
            Mat2,
            Mat3,
            Mat4,
        };

        /** Index of buffer view, default = -1 */
        int64 BufferView;

        /** Byte offset into the buffer view, default = 0 */
        int64 ByteOffset;

        /** Coomponent type of this accessor */
        EComponentType ComponentType;

        /** Count of data, default = 0 */
        int64 Count;

        /** Min/Max used for bounding boxes */
        Vector3D Min; // default = EPSILON 
        Vector3D Max; // default = EPSILON

        /** The type of the data */
        EType Type;

        FAccessor()
            : BufferView(-1),
            ByteOffset(0),
            ComponentType(EComponentType::Invalid),
            Count(0),
            Type(EType::Invalid),
            Min(EPSILON), Max(EPSILON)
        { }
    };

    struct VRIXIC_API FBufferView
    {
    public:
        enum class ETarget
        {
            Invalid = -1,
            VertexData = 34962, /*ArrayBuffer*/
            IndexData = 34963  /*ElementArrayBuffer*/
        };

        std::string Name;

        /** The index of the buffer */
        int64 BufferIndex;

        /** The byte length of the buffer view */
        int64 ByteLength;

        /** The byte offset into the buffer data */
        int64 ByteOffset;

        /** Stride of the buffer, should be tighly packed */
        int64 ByteStride;

        /** Target of the buffer view */
        ETarget Target;

        FBufferView()
            : BufferIndex(-1),
            ByteLength(0),
            ByteOffset(0),
            ByteStride(0),
            Target(ETarget::Invalid)
        {}
    };

    struct VRIXIC_API FBuffer
    {
    public:
        /** Byte length of the buffer */
        int64 ByteLength;

        /** The bin file for the buffer */
        std::string Uri;

        /** Indicates that the URI is the buffer and not a path to the bin*/
        bool bIsUriBuffer;

        FBuffer()
            : ByteLength(0),
            bIsUriBuffer(false)
        { }
    };

    struct VRIXIC_API FImage
    {
    public:
        /** Buffer view for the image */
        int32 BufferView;

        /** The Uri for the image */
        std::string Uri;

        /** Indicates that the URI is the buffer and not a path to the bin*/
        bool bIsUriBuffer;

        /** {"image/jpeg", "image/png", "image/bmp", "image/gif"} */
        std::string MimeType;

        FImage()
            : BufferView(-1),
            bIsUriBuffer(false)
        { }
    };

    struct VRIXIC_API FTextureInfo
    {
    public:
        /** Index into the textures */
        int32 Index;

        /** The set index for the textures TEXCOORD attrib/used for texture coordinate mapping */
        int32 TexCoord;

        FTextureInfo()
            : Index(-1),
            TexCoord(0)
        { }
    };

    struct VRIXIC_API FNormalTextureInfo
    {
        /** Index into the textures */
        int32 Index;

        /** The set index for the textures TEXCOORD attrib/used for texture coordinate mapping */
        int32 TexCoord;

        /**
        * Linearlys scales X and Y values of the normal vector (Should normalize the vector after scaling)...
        * EX: normalize((SurfaceNormal * 2.0 - 1.0) * vec3(NormalScale, NormalScale, 1.0))
        */
        float Scale;

        FNormalTextureInfo()
            : Index(-1),
            TexCoord(0),
            Scale(1.0) { }
    };

    struct VRIXIC_API FOcclusionTextureInfo
    {
        /** Index into the textures */
        int32 Index;

        /** The set index for the textures TEXCOORD attrib/used for texture coordinate mapping */
        int32 TexCoord;

        /**
        * Used to reduce the occulusion effect. Occulisions values 1.0 + Strength * (occulusionTexture - 1.0f);
        * Ex in glsl: mix(Color, Color * SampledOcclusionTextureValue, OcclusionStrength)
        */
        float Strength;

        FOcclusionTextureInfo()
            : Index(-1),
            TexCoord(0),
            Strength(1.0) { }
    };

    struct VRIXIC_API FPBRMetallicRoughnessInfo
    {
        /** Base color of the material */
        Vector4D BaseColorFactor;
        FTextureInfo BaseColorTexture;

        float RoughnessFactor;
        float MetallicFactor;
        FTextureInfo MetallicRoughnessTexture;

        FPBRMetallicRoughnessInfo()
            : BaseColorFactor(1),
            RoughnessFactor(1.0),
            MetallicFactor(1.0) { }
    };

    struct VRIXIC_API FMaterial
    {
    public:
        enum class EAlphaMode
        {
            Invalid = -1,
            Opaque,
            Mask,
            Blend
        };

        /** The name of the material */
        std::string Name;

        float AlphaCutoff;
        EAlphaMode AlphaMode;
        bool bIsDoubleSided;

        Vector3D EmissiveFactor;
        FTextureInfo EmissiveTexture;

        FNormalTextureInfo NormalTexture;
        FOcclusionTextureInfo OcclusionTexture;
        FPBRMetallicRoughnessInfo PBRMetallicRoughnessInfo;

        FMaterial()
            : AlphaCutoff(0.5f),
            AlphaMode(EAlphaMode::Opaque),
            bIsDoubleSided(false),
            EmissiveFactor(0.0f) { }

    };

    struct VRIXIC_API FMeshPrimitiveAttribute
    {
    public:
        /** Key of the attribute: JOINTS_0...NORMAL...POSITION....TEXTCOORD_0..., etc*/
        std::string Key;

        /** Index into the accessors */
        int32 AccessorIndex;

        FMeshPrimitiveAttribute()
            : AccessorIndex(-1) { }
    };

    struct VRIXIC_API FMeshPrimitive
    {
    public:
        enum class EMode
        {
            Invalid = -1,
            Points,
            Lines,
            LineLoop,
            LineStrip,
            Triangles,
            TriangleStrip,
            TriangleFan
        };

        /** All attributes for this mesh primitive */
        std::vector<FMeshPrimitiveAttribute> Attributes;

        /** The index into the accessors that contains the indicies */
        int32 IndiciesIndex;

        /** The index into the material data | this material will be applied to this primitive */
        int32 MaterialIndex;

        /** Basically PrimitiveTopology */
        EMode Mode;

        FMeshPrimitive()
            : IndiciesIndex(-1),
            MaterialIndex(-1),
            Mode(EMode::Invalid) { }
    };

    struct VRIXIC_API FMesh
    {
    public:
        /** Name of the mesh*/
        std::string Name;

        /** All of the mesh primitives */
        std::vector<FMeshPrimitive> Primitives;
    };

    struct VRIXIC_API FPerspectiveCamera
    {
    public:
        float AspectRatio;
        float YFov; // In Radians 
        float ZFar; // ZFar == 0, its is a infinite projection matrix 
        float ZNear;

        FPerspectiveCamera()
            : AspectRatio(0.0f),
            YFov(0.0f),
            ZFar(0.0f),
            ZNear(0.0f) { }
    };

    struct VRIXIC_API FOrthographicCamera
    {
    public:
        float XMag;
        float YMag;
        float ZFar;
        float ZNear;

        FOrthographicCamera()
            : XMag(0.0f),
            YMag(0.0f),
            ZFar(0.0f),
            ZNear(0.0f) { }
    };

    struct VRIXIC_API FCamera
    {
    public:
        std::string Name;
        bool bIsOrthographic;

        FPerspectiveCamera PerspectiveCamera;
        FOrthographicCamera OrthographicCamera;

        FCamera()
            : bIsOrthographic(false) { }
    };

    struct VRIXIC_API FNode
    {
    public:
        std::string Name;

        /** Index to the camera object referenced by this node */
        int32 CameraIndex;

        /** All of the nodes children, their indicies*/
        std::vector<uint32> Children;

        Vector4D Rotation;
        Vector3D Scale;
        Vector3D Translation;
        Matrix4D Matrix;

        /** Index into the meshes data */
        int32 MeshIndex;

        FNode()
            : CameraIndex(-1),
            Rotation(0.0f),
            Scale(1.0f),
            Translation(0.0f),
            MeshIndex(-1)
        {
            Matrix.SetIdentity();
        }
    };

    struct VRIXIC_API FSampler
    {
    public:
        enum class EFilter
        {
            Invalid = -1,

            Nearest = 9728,

            Linear = 9729,

            NearestMipmapNearest = 9984,

            LinearMipmapNearest = 9985,

            NearestMipmapLinear = 9986,

            LinearMipmapLinear = 9987
        };

        enum class EWrap
        {
            Invalid = -1,

            ClampToEdge = 33071,

            MirroredRepeat = 33648,

            Repeat = 10497
        };

        EFilter MagFilter;
        EFilter MinFilter;
        EWrap WrapS;
        EWrap WrapT;

        FSampler()
            : MagFilter(EFilter::Invalid),
            MinFilter(EFilter::Invalid),
            WrapS(EWrap::Invalid),
            WrapT(EWrap::Invalid) { }
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
        int32 SamplerIndex;

        /** Aka. Source - index of the image this texture refers to */
        int32 ImageIndex;

        FTexture()
            : SamplerIndex(-1),
            ImageIndex(-1) { }
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
        static void TryLoadUint32(json& inJsonData, const char* inDataKey, uint32 inFailValue, uint32& outValue)
        {
            // Check if it exists, if not exit 
            auto It = inJsonData.find(inDataKey);
            if (It == inJsonData.end())
            {
                outValue = inFailValue;
                return;
            }

            outValue = inJsonData.value(inDataKey, 0);
        }

        static void TryLoadInt32(json& inJsonData, const char* inDataKey, int32 inFailValue, int32& outValue)
        {
            // Check if it exists, if not exit 
            auto It = inJsonData.find(inDataKey);
            if (It == inJsonData.end())
            {
                outValue = inFailValue;
                return;
            }

            outValue = inJsonData.value(inDataKey, 0);
        }

        static void TryLoadInt64(json& inJsonData, const char* inDataKey, int64 inFailValue, int64& outValue)
        {
            // Check if it exists, if not exit 
            auto It = inJsonData.find(inDataKey);
            if (It == inJsonData.end())
            {
                outValue = inFailValue;
                return;
            }

            outValue = inJsonData.value(inDataKey, 0);
        }

        /**
        * Trys to load a float from json data, loads it if it exists, otherwise will not load
        */
        static void TryLoadFloat(json& inJsonData, const char* inDataKey, float inFailValue, float& outValue)
        {
            // Check if it exists, if not exit 
            auto It = inJsonData.find(inDataKey);
            if (It == inJsonData.end())
            {
                outValue = inFailValue;
                return;
            }

            outValue = inJsonData.value(inDataKey, 0.0f);
        }

        static void TryLoadBoolean(json& inJsonData, const char* inDataKey, bool inFailValue, bool& outValue)
        {
            // Check if it exists, if not exit 
            auto It = inJsonData.find(inDataKey);
            if (It == inJsonData.end())
            {
                outValue = inFailValue;
                return;
            }

            outValue = inJsonData.value(inDataKey, false);
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
        static void TryLoadUint32Vector(json& inJsonData, const char* inDataKey, std::vector<uint32>& outVector)
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
                return;
            }

            TryLoadInt32(*It, "index", -1, outTextureInfo.Index);
            TryLoadInt32(*It, "texCoord", 0, outTextureInfo.TexCoord);
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
                return;
            }

            TryLoadInt32(*It, "index", -1, outTextureInfo.Index);
            TryLoadInt32(*It, "texCoord", 0, outTextureInfo.TexCoord);
            TryLoadFloat(*It, "scale", 1.0f, outTextureInfo.Scale);
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
                return;
            }

            TryLoadInt32(*It, "index", -1, outTextureInfo.Index);
            TryLoadInt32(*It, "texCoord", 0, outTextureInfo.TexCoord);
            TryLoadFloat(*It, "strength", 1.0f, outTextureInfo.Strength);
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

            TryLoadVector4D(*It, "baseColorFactor", 1.0f, outInfo.BaseColorFactor);
            TryLoadTextureInfo(*It, "baseColorTexture", outInfo.BaseColorTexture);

            TryLoadFloat(*It, "metallicFactor", 1.0f, outInfo.MetallicFactor);
            TryLoadTextureInfo(*It, "metallicRoughnessTexture", outInfo.MetallicRoughnessTexture);

            TryLoadFloat(*It, "roughnessFactor", 1.0f, outInfo.RoughnessFactor);
        }

        /**
        * Trys to load a vector3d from json data, loads it if it exists, otherwise will not load
        */
        static void TryLoadVector3D(json& inJsonData, const char* inDataKey, float inFailValue, Vector3D& outVector)
        {
            // Check if it exists, if not exit 
            auto It = inJsonData.find(inDataKey);
            if (It == inJsonData.end())
            {
                outVector.X = inFailValue;
                outVector.Y = inFailValue;
                outVector.Z = inFailValue;
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
        static void TryLoadVector4D(json& inJsonData, const char* inDataKey, float inFailValue, Vector4D& outVector)
        {
            // Check if it exists, if not exit 
            auto It = inJsonData.find(inDataKey);
            if (It == inJsonData.end())
            {
                outVector.X = inFailValue;
                outVector.Y = inFailValue;
                outVector.Z = inFailValue;
                outVector.W = inFailValue;
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
                outMatrix(0, 0) = EPSILON;
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
            else
            {
                // If it makes it this far then an error has occured as it has to be one of those options
                VE_CORE_LOG_ERROR(VE_TEXT("[FJsonHelpers]: Cannot load alpha mode as its invalid...."));
            }
        }

        static inline bool IsUCharBase64(unsigned char c) {
            return (isalnum(c) || (c == '+') || (c == '/'));
        }

        static std::string DecodeBase64(std::string const& inEncodedString)
        {
            int in_len = static_cast<int>(inEncodedString.size());
            int i = 0;
            int j = 0;
            int in_ = 0;
            unsigned char char_array_4[4], char_array_3[3];
            std::string ret;

            const std::string base64_chars =
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz"
                "0123456789+/";

            while (in_len-- && (inEncodedString[in_] != '=') &&
                IsUCharBase64(inEncodedString[in_])) {
                char_array_4[i++] = inEncodedString[in_];
                in_++;
                if (i == 4) {
                    for (i = 0; i < 4; i++)
                        char_array_4[i] =
                        static_cast<unsigned char>(base64_chars.find(char_array_4[i]));

                    char_array_3[0] =
                        (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                    char_array_3[1] =
                        ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                    for (i = 0; (i < 3); i++) ret += char_array_3[i];
                    i = 0;
                }
            }

            if (i) {
                for (j = i; j < 4; j++) char_array_4[j] = 0;

                for (j = 0; j < 4; j++)
                    char_array_4[j] =
                    static_cast<unsigned char>(base64_chars.find(char_array_4[j]));

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] =
                    ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
            }

            return ret;
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
                FJsonHelpers::TryLoadInt64(Data[i], "bufferView", -1, outAccessors[i].BufferView);
                FJsonHelpers::TryLoadInt64(Data[i], "byteOffset", 0, outAccessors[i].ByteOffset);
                FJsonHelpers::TryLoadInt32(Data[i], "componentType", -1, (int32&)outAccessors[i].ComponentType);
                FJsonHelpers::TryLoadInt64(Data[i], "count", 0, outAccessors[i].Count);
                FJsonHelpers::TryLoadType(Data[i], "type", outAccessors[i].Type);

                if (outAccessors[i].Type == FAccessor::EType::Vec3)
                {
                    FJsonHelpers::TryLoadVector3D(Data[i], "min", EPSILON, outAccessors[i].Min);
                    FJsonHelpers::TryLoadVector3D(Data[i], "max", EPSILON, outAccessors[i].Max);
                }
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
                FJsonHelpers::TryLoadInt64(Data[i], "buffer", -1, outBufferView[i].BufferIndex);
                FJsonHelpers::TryLoadInt64(Data[i], "byteLength", 0, outBufferView[i].ByteLength);
                FJsonHelpers::TryLoadInt64(Data[i], "byteOffset", 0, outBufferView[i].ByteOffset);
                FJsonHelpers::TryLoadInt64(Data[i], "byteStride", 0, outBufferView[i].ByteStride);
                FJsonHelpers::TryLoadInt32(Data[i], "target", -1, (int32&)outBufferView[i].Target);
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
                FJsonHelpers::TryLoadInt32(Data[i], "bufferView", -1, outImages[i].BufferView);
                FJsonHelpers::TryLoadString(Data[i], "uri", outImages[i].Uri);

                int32 HeaderLength = -1;
                int32 HeaderIndex = CheckURI(outImages[i].Uri, HeaderLength);

                outImages[i].bIsUriBuffer = false;
                if (HeaderIndex != -1)
                {
                    outImages[i].bIsUriBuffer = true;

                    outImages[i].MimeType = outImages[i].Uri.substr(5, outImages[i].Uri.find(';'));

                    std::string DecodedString = FJsonHelpers::DecodeBase64(outImages[i].Uri.substr(HeaderLength));
                    outImages[i].Uri = DecodedString;
                }
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

                FJsonHelpers::TryLoadFloat(Data[i], "alphaCutoff", 0.5f, outMaterials[i].AlphaCutoff);
                FJsonHelpers::TryLoadAlphaMode(Data[i], "alphaMode", outMaterials[i].AlphaMode);
                FJsonHelpers::TryLoadBoolean(Data[i], "doubleSided", false, outMaterials[i].bIsDoubleSided);

                FJsonHelpers::TryLoadVector3D(Data[i], "emissiveFactor", 0.0f, outMaterials[i].EmissiveFactor);
                FJsonHelpers::TryLoadTextureInfo(Data[i], "emissiveTexture", outMaterials[i].EmissiveTexture);

                FJsonHelpers::TryLoadNormalTextureInfo(Data[i], "normalTexture", outMaterials[i].NormalTexture);
                FJsonHelpers::TryLoadOcclusionTextureInfo(Data[i], "occlusionTexture", outMaterials[i].OcclusionTexture);
                FJsonHelpers::TryLoadPBRMetallicRoughnessInfo(Data[i], "pbrMetallicRoughness", outMaterials[i].PBRMetallicRoughnessInfo);
            }
        }

        /**
        * Helper function which just loads all of the mesh primitives from the parsed json data
        */
        static void LoadMeshPrimitives(json& inJsonData, std::vector<FMeshPrimitive>& outMeshPrimitives)
        {
            json Data = inJsonData["primitives"];

            uint32 DataSize = Data.size();
            outMeshPrimitives.resize(DataSize);
            memset(outMeshPrimitives.data(), 0, sizeof(FMeshPrimitive) * DataSize);

            for (uint32 i = 0; i < DataSize; i++)
            {
                FJsonHelpers::TryLoadInt32(Data[i], "indices", -1, outMeshPrimitives[i].IndiciesIndex);
                FJsonHelpers::TryLoadInt32(Data[i], "material", -1, outMeshPrimitives[i].MaterialIndex);
                FJsonHelpers::TryLoadInt32(Data[i], "mode", -1, (int32&)outMeshPrimitives[i].Mode);

                json Attributes = Data[i]["attributes"];
                outMeshPrimitives[i].Attributes.resize(Attributes.size());
                memset(outMeshPrimitives[i].Attributes.data(), 0, sizeof(FMeshPrimitiveAttribute) * Attributes.size());

                uint32 Index = 0;
                for (auto JsonAttribute : Attributes.items())
                {
                    outMeshPrimitives[i].Attributes[Index].Key.resize(JsonAttribute.key().size());
                    outMeshPrimitives[i].Attributes[Index].Key = JsonAttribute.key();
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

                FJsonHelpers::TryLoadInt32(Data[i], "camera", -1, outNodes[i].CameraIndex);
                FJsonHelpers::TryLoadInt32(Data[i], "mesh", -1, outNodes[i].MeshIndex);

                FJsonHelpers::TryLoadUint32Vector(Data[i], "children", outNodes[i].Children);

                FJsonHelpers::TryLoadVector3D(Data[i], "scale", 1.0f, outNodes[i].Scale);
                FJsonHelpers::TryLoadVector3D(Data[i], "translation", 0.0f, outNodes[i].Translation);
                FJsonHelpers::TryLoadVector4D(Data[i], "rotation", 0.0f, outNodes[i].Rotation);
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
                FJsonHelpers::TryLoadInt32(Data[i], "magFilter", -1, (int32&)outSamplers[i].MagFilter);
                FJsonHelpers::TryLoadInt32(Data[i], "minFilter", -1, (int32&)outSamplers[i].MinFilter);
                FJsonHelpers::TryLoadInt32(Data[i], "wrapS", -1, (int32&)outSamplers[i].WrapS);
                FJsonHelpers::TryLoadInt32(Data[i], "wrapT", -1, (int32&)outSamplers[i].WrapT);
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
                FJsonHelpers::TryLoadUint32Vector(Data[i], "nodes", outScenes[i].Nodes);
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
                FJsonHelpers::TryLoadInt32(Data[i], "sampler", -1, outTextures[i].SamplerIndex);
                FJsonHelpers::TryLoadInt32(Data[i], "source", -1, outTextures[i].ImageIndex);
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
                FJsonHelpers::TryLoadInt64(Data[i], "byteLength", 0, outBuffers[i].ByteLength);

                int32 HeaderLength = -1;
                int32 HeaderIndex = CheckURI(outBuffers[i].Uri, HeaderLength);

                outBuffers[i].bIsUriBuffer = false;
                if (HeaderIndex != -1)
                {
                    outBuffers[i].bIsUriBuffer = true;

                    std::string DecodedString = FJsonHelpers::DecodeBase64(outBuffers[i].Uri.substr(HeaderLength));
                    outBuffers[i].Uri = DecodedString;
                }
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

    private:

        static int CheckURI(const std::string& inURI, int32& outUriLength)
        {
            // Firstly we want to check for encodings 
            static std::string Headers[7] = {
                "data:application/octet-stream;base64,",
                "data:image/jpeg;base64,",
                "data:image/png;base64,",
                "data:image/bmp;base64,",
                "data:image/gif;base64,",
                "data:text/plain;base64,",
                "data:application/gltf-buffer;base64,"
            };

            // if data is the uri then we will know
            for (uint32 j = 0; j < 7; ++j)
            {
                if (inURI.find(Headers[j]) == 0)
                {
                    outUriLength = Headers[j].length();
                    return j;
                    break;
                }
            }

            outUriLength = -1;
            return -1;
        }
    };

} // namespace GLTF
