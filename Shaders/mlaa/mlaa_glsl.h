#define STRINGIFY(s) #s

std::string mlaa_header_vs =
        "#version 400 core\n\
 #define MLAA_ONLY_COMPILE_VS 1\n\
 #include \"MLAA.h\"\n\
";

std::string mlaa_header_ps =
        "#version 400 core\n\
 #define MLAA_ONLY_COMPILE_PS 1\n\
 #include \"MLAA.h\"\n\
";

std::string mlaa_test_vs = STRINGIFY(
        hash version 330 core \n
        attribute vec2 aPosition; \n
        out vec2 texcoord; \n
        void main() \n
{ \n
        texcoord = vec2((aPosition + 1.0) / 2.0); \n
        gl_Position = vec4(aPosition, 0.0, 1.0); \n
} \n
);

std::string mlaa_test_ps = STRINGIFY(
        hash version 330 core \n
        in vec2 texcoord; \n
        uniform sampler2D albedo_tex; \n
        void main() \n
{ \n
        gl_FragColor = texture(albedo_tex, texcoord); \n
} \n
);

std::string mlaa_test_vs2 = mlaa_header_vs + STRINGIFY(
        attribute vec2 aPosition; \n
        out vec2 texcoord; \n
        void main() \n
{ \n
        texcoord = vec2((aPosition + 1.0) / 2.0); \n
        gl_Position = vec4(aPosition, 0.0, 1.0); \n
} \n
);

std::string mlaa_test_ps2 = mlaa_header_ps + STRINGIFY(
        in vec2 texcoord; \n
        uniform sampler2D albedo_tex; \n
        void main() \n
{ \n
        gl_FragColor = ColorEdgeDetectionPS(texcoord, offset, albedo_tex, 0.1); \n
        gl_FragColor = texture(albedo_tex, texcoord); \n
} \n
);

std::string mlaa_edge_vs =
        mlaa_header_vs +
        STRINGIFY(
                attribute vec2 aPosition; \n
                out vec2 texcoord; \n
                out vec4 offset[2]; \n
                void main() \n
        { \n
                texcoord = vec2((aPosition + 1.0) / 2.0); \n
                offset[0] = texcoord.xyxy + PIXEL_SIZE.xyxy * float4(-1.0, 0.0, 0.0, -1.0); \n
                offset[1] = texcoord.xyxy + PIXEL_SIZE.xyxy * float4( 1.0, 0.0, 0.0,  1.0); \n
                gl_Position = vec4(aPosition, 0.0, 1.0); \n
        } \n
        );

std::string mlaa_edge_ps =
        mlaa_header_ps +
        STRINGIFY(
                uniform sampler2D albedo_tex; \n
                in vec2 texcoord; \n
                in vec4 offset[2]; \n
                void main() \n
        { \n
                gl_FragColor = ColorEdgeDetectionPS(texcoord, offset, albedo_tex); \n
                gl_FragColor = vec4(texcoord,0.0,1.0); \n
        } \n
        );

std::string mlaa_blend_vs =
        mlaa_header_vs +
        STRINGIFY(
                attribute vec2 aPosition; \n
                out vec2 texcoord; \n
                out vec2 pixcoord; \n
                out vec4 offset[3]; \n
                out vec4 dummy2; \n
                void main() \n
        { \n
                texcoord = vec2((aPosition + 1.0) / 2.0); \n
                vec4 dummy1 = vec4(0); \n
                SMAABlendingWeightCalculationVS(dummy1, dummy2, texcoord, pixcoord, offset); \n
                gl_Position = vec4(aPosition, 0.0, 1.0); \n
        } \n
        );

std::string mlaa_blend_ps =
        mlaa_header_ps +
        STRINGIFY(
                uniform sampler2D edge_tex; \n
                uniform sampler2D area_tex; \n
                uniform sampler2D search_tex; \n
                in vec2 texcoord; \n
                in vec2 pixcoord; \n
                in vec4 offset[3]; \n
                in vec4 dummy2; \n
                void main() \n
        { \n
                gl_FragColor = SMAABlendingWeightCalculationPS(texcoord, pixcoord, offset, edge_tex, area_tex, search_tex, ivec4(0)); \n
        } \n
        );

std::string mlaa_neighborhood_vs =
        mlaa_header_vs +
        STRINGIFY(
                attribute vec2 aPosition; \n
                out vec2 texcoord; \n
                out vec4 offset[2]; \n
                out vec4 dummy2; \n
                void main() \n
        { \n
                texcoord = vec2((aPosition + 1.0) / 2.0); \n
                vec4 dummy1 = vec4(0); \n
                SMAANeighborhoodBlendingVS(dummy1, dummy2, texcoord, offset); \n
                gl_Position = vec4(aPosition, 0.0, 1.0); \n
        } \n
        );

std::string mlaa_neighborhood_ps =
        mlaa_header_ps +
        STRINGIFY(
                uniform sampler2D albedo_tex; \n
                uniform sampler2D blend_tex; \n
                in vec2 texcoord; \n
                in vec4 offset[2]; \n
                in vec4 dummy2; \n
                void main() \n
        { \n
                gl_FragColor = SMAANeighborhoodBlendingPS(texcoord, offset, albedo_tex, blend_tex); \n
        } \n
        );