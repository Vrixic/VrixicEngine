/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Core.h>
#include "RenderPass.h"

/**
* Primitve tology - defines how vertices are read in
*/
enum class EPrimitiveTopology
{
    // Each vertex represents a single point primitive 
    PointList,

    // Each pair of two vertices represents a single line primitive.
    LineList,

    // Each vertex generates a new line primitive while the previous vertex is used as line start
    LineStrip,

    // Similar to LineList but each end point has a corresponding adjacent vertex that is accessible in a geometry shader
    LineListAdjacency,

    // Similar to LineStrip but each end point has a corresponding adjacent vertex that is accessible in a geometry shader
    LineStripAdjacency,

    // Each set of three vertices represent a single triangle primitive
    TriangleList,

    // Each vertex generates a new triangle primitive with an alternative triangle winding
    TriangleStrip,

    // Similar to TriangleList but each triangle edge has a corresponding adjacent vertex that is accessible in a geometry shader
    TriangleListAdjacency,

    // Similar to TriangleStrip but each triangle edge has a corresponding adjacent vertex that is accessible in a geometry shader
    TriangleStripAdjacency,
};

/**
* Comparison operator that compares a reference and a test value
*/
enum class ECompareOp
{
    Never = 0, // always evaluates to false
    Less = 1, // <
    Equal = 2, // ==
    LessOrEqual = 3, // <=
    Greater = 4, // >
    NotEqual = 5, // !=
    GreaterOrEqual = 6, // >=
    Always = 7, // always evaluates to true 
};

/**
* Used to specify what happens to stored stencil values
*/
enum class EStencilOp
{
    Keep = 0, // keeps the current stencil value
    Zero = 1, // sets the stencil value to 0
    Replace = 2, // sets the stencil value to the one provided
    IncrementAndClamp = 3, // increments the current stencil value and clamps
    DecrementAndClamp = 4, // decrements the current stencil value and clamps
    Invert = 5, // bitwise-inverts the current stencil value
    IncrementAndWrap = 6, // increments the current stencil value and wraps back if exceeded
    DecrementAndWrap = 7, // decrements the current stencil value and wraps to max value if it goes below 0
};

/**
* Blend arithemetic operations from source to destination blend factors
*/
enum class EBlendOp
{
    Add = 0, // adds b to a 
    Subtract = 1, // subs a from b
    ReverseSubtract = 2, // subs b from a
    Min = 3, // min(a, b)
    Max = 4, // max(a, b)
};

/**
* Source and destination color and alpha blending factors
*/
enum class EBlendFactor
{
    Zero                        = 0,
    One                         = 1,
    SrcColor                    = 2,
    OneMinusSrcColor            = 3,
    DstColor                    = 4,
    OneMinusDstColor            = 5,
    SrcAlpha                    = 6,
    OneMinusSrcAlpha            = 7,
    DstAlpha                    = 8,
    OneMinusDstAlpha            = 9,
    ConstantColor               = 10,
    OneMinusConstantColor       = 11,
    ConstantAlpha               = 12,
    OneMinusConstantAlpha       = 13,
    SrcAlphaSaturate            = 14,
    Src1Color                   = 15,
    OneMinusSrc1Color           = 16,
    Src1Alpha                   = 17,
    OneMinusSrc1Alpha           = 18,
};

/**
* polygon mode specifying the method of rasterization of polygons
*/
enum class EPolygonMode
{
    Fill    = 0,
    Line    = 1,
    Point   = 2,
};

enum class ECullMode 
{
    None    = 0, // no triangles are discarded
    Front   = 1, // front-facing triangles are discarded
    Back    = 2, // back-facing triangles are discareded 
};

enum class EFrontFace
{
    CounterClockwise        = 0,
    Clockwise               = 1
};

/**
* 
*/
enum class ELogicOp
{
    Disabled        = -1,
    Clear           = 0,
    And             = 1,
    AndReverse      = 2,
    Copy            = 3,
    AndInverted     = 4,
    NoOp            = 5,
    XOR             = 6,
    Or              = 7,
    Nor             = 8,
    Equivalent      = 9,
    Invert          = 10,
    OrReverse       = 11,
    CopyInverted    = 12,
    OrInverted      = 13,
    NAND            = 14,
    Set             = 15,
};

struct VRIXIC_API ColorComponentFlags
{
#define BIT(x) (1 << x)
    enum
    {
        R       = BIT(0),
        G       = BIT(1),
        B       = BIT(2),
        A       = BIT(3),

        RGB     = R | G | B,
        All     = RGB | A
    };
};


/**
* Defines a render viewport which is used by command buffers for rendering
* Specifies how the normalized device coords (NDC) are transformed into the pixel coords of the framebuffer
*/
struct VRIXIC_API RenderViewport
{
public:
    // The X coordinate from the upper left corner of the screen
    float X;

    // The Y coordinate from the upper left corner of the screen
    float Y;

    // Width of the viewport
    float Width;

    // Height of the viewport
    float Height;

    // Min depth range of the viewport 
    float MinDepth;

    // Max depth range of the viewport 
    float MaxDepth;
};

/**
* Defines a render scissor which is used by command buffers for rendering
* Used to render parts of the screen inside of a RenderViewport
*/
struct VRIXIC_API RenderScissor
{
public:
    // Width of the scissor rect (How much to width render)
    float Width;

    // Height of the scissor rect (How much to width render)
    float Height;

    // The x offset of the upper left corner of the screen
    float OffsetX;

    // The y offset of the upper left corner of the screen
    float OffsetY;
};

/**
* Defines depth state
*/
struct VRIXIC_API DepthConfig
{
public:
    // is depth testing enabled or disabled 
    bool bIsTestingEnabled;

    // is writing to depth buffer enabled or disabled
    bool bIsWritingEnabled;

    // the comparison operator used for depth testing 
    ECompareOp CompareOp;
};

/**
* Defines stencil operation states 
*/
struct VRIXIC_API StencilOpConfig
{
public:
    // specifies the action performed on samples that fail the stencil test
    EStencilOp StencilFailOp;

    // specifies the action performed on samples that pass the stencil test
    EStencilOp StencilPassOp;

    // specifys the action performed on samples thats pass the stencil test and fail the depth test
    EStencilOp DepthFailOp;

    // specifies the comparidon operator used in the stencil test
    ECompareOp CompareOp;

    // selects the bits of the uint stencil value that participate in stencil test
    uint32 CompareMask;

    // selects the bits of the uint stencil value that are updated by the stencil test in the stencil framebuffer attachment
    uint32 WriteMask;

    // integer stencil reference value that is used in the unsigned stencil comparison operations
    uint32 ReferenceValue;
};

/**
* Defines stencil states
*/
struct VRIXIC_API StencilStateConfig
{
public:
    // perform stencil test?
    bool bIsTestingEnabled;

    // specifies front face descriptions of stencil testing
    StencilOpConfig Front;

    // specifies back face descriptions of stencil testing
    StencilOpConfig Back;
};

/**
* A descriptor for depth bias that allows control of a fragments depth value
*/
struct VRIXIC_API DepthBiasConfig
{
public:
    // Scalar factor that controls the constant depth value added to each fragment
    float ConstantFactor;

    // can be the min or max of the depth bias of a fragment 
    float Clamp;

    // Scalar factor that is applied to a fragments slope in depth bias calculations
    float SlopeFactor;
};

/**
* Defines the rasterization phase 
*/
struct RasterizerConfig
{
public:
    // Polygon mode 
    EPolygonMode PolygonMode;

    // The culling mode
    ECullMode CullMode;

    // depth bias descriptor for fragment depth values
    DepthBiasConfig DepthBias;

    // Polygon winding
    EFrontFace FrontFace;

    // Should clamp depth values? if so near and far planes are not effective
    bool bDepthClampEnabled;

    // controls whether primitives are discarded immediately before the rasterization stage.
    bool bRasterizerDiscardEnabled;

    // controls whether to bias fragment depth values.
    bool bDepthBiasEnabled;

    // width of rasterized line segments.
    float LineWidth;
};

/**
* Specifys a pipelines blend attachment state 
*/
struct VRIXIC_API BlendOpConfig
{
public:
    // controls whether blending is enabled for the corresponding color attachment. If blending is not enabled, 
    // the source fragment’s color for that attachment is passed through unmodified
    bool bIsBlendEnabled;

    // used to select which blend factor is used to determine the source factors
    EBlendFactor SrcColorBlendFactor;

    // used to select which blend factor is used to determine the destination factors
    EBlendFactor DstColorBlendFactor;

    // used to select which blend operation is used to calculate the RGB values to write to the color attachment
    EBlendOp ColorBlendOp;

    // used to select which blend factor is used to determine the source factor
    EBlendFactor SrcAlphaBlendFactor;

    // used to select which blend factor is used to determine the destination factor
    EBlendFactor DstAlphaBlendFactor;

    // used to select which blend operation is used to calculate the alpha values to write to the color attachment
    EBlendOp AlphaBlendOp;

    // specifys R G B A components that are enabled for writing 
    uint8 ColorWriteMask;
};

/**
* Specifys params of a pipelines blend state
*/
struct VRIXIC_API BlendStateConfig
{
public:
    // Specifies logic fragment operation
    ELogicOp LogicOp;

    // All of the blend op color attachments 
    std::vector<BlendOpConfig> BlendOpDescriptors;

    // if the blend factor will be dynamically set 
    bool bIsBlendFactorDynamic;

    // Specifies the blending color factor 
    // ignored if bIsBlendFactorDynamic is set to true
    float BlendConstants[4];

public:
    uint32 GetNumBlendOpDescriptors() const
    {
        return BlendOpDescriptors.size();
    }
};

/**
* Defines a graphics pipeline description
*/
struct VRIXIC_API GraphicsPipelineConfig
{
public:
    // Layout of the pipeline
    const PipelineLayout* PipelineLayoutPtr;

    // Renderpass associated with this pipeline
    const IRenderPass* RenderPassPtr;

    // The primitive topology
    EPrimitiveTopology PrimitiveTopology;

    // Viewports
    std::vector<RenderViewport> Viewports;

    // Scissors
    std::vector<RenderScissor> Scissors;

    // The depth description 
    DepthConfig     DepthDesc;

    // The stencil description
    StencilStateConfig StencilDesc;

    // The rasterization stage description
    RasterizerConfig RasterizerDesc;

    // the blend state description
    BlendStateConfig  BlendDesc;
};

