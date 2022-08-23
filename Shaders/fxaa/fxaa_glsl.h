#define STRINGIFY(s) #s

std::string fxaa_header_vs =
    "#version 400\n\
    #define FXAA_PC 1\n\
    #define FXAA_GLSL_130 1\n\
    #define FXAA_QUALITY__PRESET 39\n\
    #define FXAA_DISCARD 0\n\
    #define FXAA_GATHER4_ALPHA 0\n\
    #define FXAA_CALCULATE_LUMINANCE 1\n\
    #include \"Fxaa3_11.h\"\n\
";

std::string fxaa_header_ps =
        "#version 400\n\
    #define FXAA_PC 1\n\
    #define FXAA_GLSL_130 1\n\
    #define FXAA_QUALITY__PRESET 39\n\
    #define FXAA_DISCARD 0\n\
    #define FXAA_GATHER4_ALPHA 0\n\
    #define FXAA_CALCULATE_LUMINANCE 1\n\
    #include \"Fxaa3_11.h\"\n\
";

std::string fxaa_luma_vs = STRINGIFY(
        hash version 400 core\n
        attribute vec2 aPosition; \n
        out vec2 texcoord; \n
        void main() \n
{ \n
        texcoord = vec2((aPosition + 1.0) / 2.0); \n
        gl_Position = vec4(aPosition, 0.0, 1.0); \n
} \n
);

std::string fxaa_luma_ps = STRINGIFY(
        hash version 400 core\n
        in vec2 texcoord; \n
        uniform sampler2D albedo_tex; \n
        float luminance(vec4 color)\n
        {\n
            return (0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b);\n
        }\n
        void main() \n
{ \n
        vec4 color = texture(albedo_tex, texcoord); \n
        float l = luminance(color)*2;\n
        gl_FragColor = vec4(color.rgb,l); \n
} \n
);

std::string fxaa_vs = fxaa_header_vs + STRINGIFY(
        attribute vec2 aPosition; \n
        out vec2 texcoord; \n
        void main() \n
{ \n
        texcoord = vec2((aPosition + 1.0) / 2.0); \n
        gl_Position = vec4(aPosition, 0.0, 1.0); \n
} \n
);

std::string fxaa_ps = fxaa_header_ps + STRINGIFY(
        in vec2 texcoord; \n
        uniform vec2 rcpFrame; \n
        uniform sampler2D albedo_tex; \n
        void main() \n
{ \n
        vec4 pixelColor = FxaaPixelShader(texcoord, vec4(0), albedo_tex, albedo_tex, albedo_tex, rcpFrame, vec4(0), vec4(0), vec4(0), 1.0, 0.063, 0.0625, 8.0, 0.125, 0.04, vec4(0)); \n
        vec4 t = texture(albedo_tex, texcoord);\n
        gl_FragColor = vec4(pixelColor.rgb,1); \n
} \n
);
