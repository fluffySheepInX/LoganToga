//
//	Pixel-art post effect (GLSL)
//

# version 410

//
//	Textures
//
uniform sampler2D Texture0;

//
//	PSInput
//
layout(location = 0) in vec4 Color;
layout(location = 1) in vec2 UV;

//
//	PSOutput
//
layout(location = 0) out vec4 FragColor;

//
//	Constant Buffer
//
layout(std140) uniform PSConstants2D
{
    vec4 g_colorAdd;
    vec4 g_sdfParam;
    vec4 g_sdfOutlineColor;
    vec4 g_sdfShadowColor;
    vec4 g_internal;
};

void main()
{
    const float levels = 5.0;

    vec4 texColor = texture(Texture0, UV);

    texColor.rgb = floor(texColor.rgb * levels) / (levels - 1.0);
    texColor.rgb = clamp(texColor.rgb, 0.0, 1.0);

    FragColor = (texColor * Color) + g_colorAdd;
}
