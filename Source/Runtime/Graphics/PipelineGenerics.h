/**
* This file is part of the "Vrixic Engine" project (Copyright (c) 2022-2023 by Vrij Patel)
* See "LICENSE.txt" for license information.
*/

#pragma once
#include <Core/Core.h>
#include "PipelineLayout.h"
#include "RenderPass.h"
#include "Shader.h"

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
* Logic Operations 
*/
enum class ELogicOp
{
    Disabled        = -1, // none, do not want to use any logic operations 
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

/**
* The bind point for a pipeline 
*/
enum class EPipelineBindPoint
{
    Undefined       = -1,
    Graphics        = 0,
    Compute         = 1,
    //RayTracingKhr = 1000165000,
    //SubpassShadingHuawei = 1000369003,
    //RAY_TRACING_NV = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
};

/**
* Color component flags structure 
*/
struct VRIXIC_API FColorComponentFlags
{
public:
#define BIT(x) (1 << x)
    enum
    {
        R       = BIT(0), // Red
        G       = BIT(1), // Green
        B       = BIT(2), // Blue
        A       = BIT(3), // Alpha

        RGB     = R | G | B, // Red Green Blue
        RGBA    = RGB | A, // Red Green Blue Alpha
        All     = RGBA
    };
};


/**
* Defines a render viewport which is used by command buffers for rendering
* Specifies how the normalized device coords (NDC) are transformed into the pixel coords of the framebuffer
*/
struct VRIXIC_API FRenderViewport
{
public:
    /** The X coordinate from the upper left corner of the screen */
    float X;

    /** The Y coordinate from the upper left corner of the screen */
    float Y;

    /** Width of the viewport */
    float Width;

    /** Height of the viewport */
    float Height;

    /** Min depth range of the viewport */
    float MinDepth;

    /** Max depth range of the viewport */
    float MaxDepth;

public:
    FRenderViewport()
        : X(0.0f), Y(0.0f), Width(0.0f), Height(0.0f), MinDepth(0.0f), MaxDepth(0.0f) { }
};

/**
* Defines a render scissor which is used by command buffers for rendering
* Used to render parts of the screen inside of a RenderViewport
*/
struct VRIXIC_API FRenderScissor
{
public:
    /** The x offset of the upper left corner of the screen */
    int32 OffsetX;

    /** The y offset of the upper left corner of the screen */
    int32 OffsetY;

    /** Width of the scissor rect (How much to width render) */
    uint32 Width;

    /** Height of the scissor rect (How much to width render) */
    uint32 Height;

public:
    FRenderScissor()
        : OffsetX(0), OffsetY(0), Width(0), Height(0) { } 
};

/**
* Defines depth state configuration settings 
*/
struct VRIXIC_API FDepthConfig
{
public:
    /** is depth testing enabled or disabled */
    bool bIsTestingEnabled;

    /** is writing to depth buffer enabled or disabled */
    bool bIsWritingEnabled;

    /** the comparison operator used for depth testing */
    ECompareOp CompareOp;

public:
    FDepthConfig()
        : bIsTestingEnabled(false), bIsWritingEnabled(false), CompareOp(ECompareOp::Less) { }
};

/**
* Defines stencil operation states 
*/
struct VRIXIC_API FStencilOpConfig
{
public:
    /** specifies the action performed on samples that fail the stencil test */
    EStencilOp StencilFailOp;

    /** specifies the action performed on samples that pass the stencil test */
    EStencilOp StencilPassOp;

    /** specifys the action performed on samples thats pass the stencil test and fail the depth test */
    EStencilOp DepthFailOp;

    /** specifies the comparidon operator used in the stencil test */
    ECompareOp CompareOp;

    /** selects the bits of the uint stencil value that participate in stencil test */
    uint32 CompareMask;

    /** selects the bits of the uint stencil value that are updated by the stencil test in the stencil framebuffer attachment */
    uint32 WriteMask;

    /** integer stencil reference value that is used in the unsigned stencil comparison operations */
    uint32 ReferenceValue;

public:
    FStencilOpConfig()
        : StencilFailOp(EStencilOp::Keep), StencilPassOp(EStencilOp::Keep), DepthFailOp(EStencilOp::Keep),
          CompareOp(ECompareOp::Less), CompareMask(0u), WriteMask(0u), ReferenceValue(0u) { }
};

/**
* Defines stencil states
*/
struct VRIXIC_API FStencilStateConfig
{
public:
    /** perform stencil test? */
    bool bIsTestingEnabled;

    /** specifies if the reference value will be dynamically set */
    bool bIsReferenceValueDynamic;

    /** specifies front face descriptions of stencil testing */
    FStencilOpConfig Front;

    /** specifies back face descriptions of stencil testing */
    FStencilOpConfig Back;

public:
    FStencilStateConfig()
        : bIsTestingEnabled(false), bIsReferenceValueDynamic(false) { }
};

/**
* A descriptor for depth bias that allows control of a fragments depth value
*/
struct VRIXIC_API FDepthBiasConfig
{
public:
    /** Scalar factor that controls the constant depth value added to each fragment */
    float ConstantFactor;

    /** can be the min or max of the depth bias of a fragment */
    float Clamp;

    /** Scalar factor that is applied to a fragments slope in depth bias calculations */
    float SlopeFactor;

public:
    FDepthBiasConfig()
        : ConstantFactor(0.0f), Clamp(0.0f), SlopeFactor(0.0f) { }
};

/**
* Defines the rasterization phase 
*/
struct VRIXIC_API FRasterizerConfig
{
public:
    /** Polygon mode */
    EPolygonMode PolygonMode;

    /** The culling mode */
    ECullMode CullMode;

    /** depth bias descriptor for fragment depth values */
    FDepthBiasConfig DepthBias;

    /** Polygon winding */
    EFrontFace FrontFace;

    /** Should clamp depth values? if so near and far planes are not effective */
    bool bDepthClampEnabled;

    /** controls whether primitives are discarded immediately before the rasterization stage. */
    bool bRasterizerDiscardEnabled;

    /** controls whether to bias fragment depth values. */
    bool bDepthBiasEnabled;

    /** width of rasterized line segments. */
    float LineWidth;

public:
    FRasterizerConfig()
        : PolygonMode(EPolygonMode::Fill), CullMode(ECullMode::None), FrontFace(EFrontFace::Clockwise),
        bDepthClampEnabled(false), bRasterizerDiscardEnabled(false), bDepthBiasEnabled(false),
        LineWidth(1.0f) { }
};

/**
* Specifys a pipelines blend attachment state 
*/
struct VRIXIC_API FBlendOpConfig
{
public:
    /** 
    * controls whether blending is enabled for the corresponding color attachment. If blending is not enabled, 
    * the source fragment’s color for that attachment is passed through unmodified
    */
    bool bIsBlendEnabled;

    /** *used to select which blend factor is used to determine the source factors */
    EBlendFactor SrcColorBlendFactor;

    /** used to select which blend factor is used to determine the destination factors */
    EBlendFactor DstColorBlendFactor;

    /** used to select which blend operation is used to calculate the RGB values to write to the color attachment */
    EBlendOp ColorBlendOp;

    /** used to select which blend factor is used to determine the source factor */
    EBlendFactor SrcAlphaBlendFactor;

    /** used to select which blend factor is used to determine the destination factor */
    EBlendFactor DstAlphaBlendFactor;

    /** used to select which blend operation is used to calculate the alpha values to write to the color attachment */
    EBlendOp AlphaBlendOp;

    /** specifys R G B A components that are enabled for writing */
    uint8 ColorWriteMask;

public:
    FBlendOpConfig()
        : bIsBlendEnabled(false), SrcColorBlendFactor(EBlendFactor::SrcAlpha), DstColorBlendFactor(EBlendFactor::OneMinusSrcAlpha),
        ColorBlendOp(EBlendOp::Add), AlphaBlendOp(EBlendOp::Add), SrcAlphaBlendFactor(EBlendFactor::SrcAlpha), DstAlphaBlendFactor(EBlendFactor::OneMinusSrcAlpha),
        ColorWriteMask(FColorComponentFlags::All) { }
};

/**
* Specifys params of a pipelines blend state
*/
struct VRIXIC_API FBlendStateConfig
{
public:
    /** Specifies if alphaToCoverageEnabled is enabled to be used as a multi-sampling technique */
    bool bAlphaToCoverageEnabled;

    /** 
    * Specifies to enable independentBlendEnabled when blending simultanieous color attachments
    * for use in multi-sample states 
    */
    bool bIndependentBlendEnabled;

    /** the bitmask used if bAlphaToCoverageEnabled is enabled */
    uint32 SampleMask; 

    /** Specifies logic fragment operation */
    ELogicOp LogicOp;

    /** All of the blend op color attachments */
    std::vector<FBlendOpConfig> BlendOpConfigs;

    /** if the blend factor will be dynamically set */
    bool bIsBlendFactorDynamic;

    /**
    * Specifies the blending color factor 
    * ignored if bIsBlendFactorDynamic is set to true
    */
    float BlendConstants[4];

public:
    FBlendStateConfig()
        : bAlphaToCoverageEnabled(false), bIndependentBlendEnabled(false), SampleMask(0u), LogicOp(ELogicOp::Disabled),
        bIsBlendFactorDynamic(false), BlendConstants{ 0.0f, 0.0f, 0.0f, 0.0f } { }

public:
    uint32 GetNumBlendOpConfigs() const
    {
        return static_cast<uint32>(BlendOpConfigs.size());
    }
};

/**
* Defines a graphics pipeline description
* 
* @note only supports two types of shaders for now Fragment & Vertex 
*/
struct VRIXIC_API FGraphicsPipelineConfig
{
public:
    /** Layout of the pipeline */
    const PipelineLayout* PipelineLayoutPtr;

    /** Renderpass associated with this pipeline */
    const IRenderPass* RenderPassPtr;

    /** Vertex Shader, manipulates vertices */
    Shader* VertexShader;

    /** Pixel Shader, manipulates pixels */
    Shader* FragmentShader;

    /** The primitive topology */
    EPrimitiveTopology PrimitiveTopology;

    /** Viewports */
    std::vector<FRenderViewport> Viewports;

    /** Scissors */
    std::vector<FRenderScissor> Scissors;

    /** The depth description */
    FDepthConfig     DepthState;

    /** The stencil description */
    FStencilStateConfig StencilState;

    /** The rasterization stage description */
    FRasterizerConfig RasterizerState;

    /** the blend state description */
    FBlendStateConfig  BlendState;

public:
    FGraphicsPipelineConfig()
        : PipelineLayoutPtr(nullptr), RenderPassPtr(nullptr), VertexShader(nullptr), FragmentShader(nullptr),
        PrimitiveTopology(EPrimitiveTopology::TriangleList) { }
};

