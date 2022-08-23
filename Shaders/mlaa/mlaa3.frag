#version 400

#define mad(m,a,b) m*a+b
#define lerp(a,b,c) mix(a,b,c)

out vec4 FragColor;
in vec2 TexCoords;
in vec4 offset[2];

uniform vec2 texelSize;
uniform sampler2D colorTexture;
uniform sampler2D blendTexture;
uniform sampler2D blendTextureBA;

vec4 NeighborhoodBlendingPS()
{
// Fetch the blending weights for current pixel:
    vec4 topLeft = texture(blendTexture, TexCoords);
    float bottom = texture(blendTexture, offset[1].zw).g;
    float right = texture(blendTexture, offset[1].xy).a;
    vec4 a = vec4(topLeft.r, bottom, topLeft.b, right);

// Up to 4 lines can be crossing a pixel (one in each edge). So, we perform
// a weighted average, where the weight of each line is 'a' cubed, which
// favors blending and works well in practice.
    vec4 w = a * a * a;

// There is some blending weight with a value greater than 0.0?
    float sum = dot(w, vec4(1.0));
    if (sum < 1e-5)
        return texture(colorTexture, TexCoords);

    vec4 color = vec4(0.0);

// Add the contributions of the possible 4 lines that can cross this pixel:
    #ifdef BILINEAR_FILTER_TRICK
    vec4 coords = mad(vec4( 0.0, -a.r, 0.0,  a.g), texelSize.yyyy, TexCoords.xyxy);
    color = mad(texture(colorTexture, coords.xy), w.r, color);
    color = mad(texture(colorTexture, coords.zw), w.g, color);

    coords = mad(vec4(-a.b,  0.0, a.a,  0.0), texelSize.xxxx, TexCoords.xyxy);
    color = mad(texture(colorTexture, coords.xy), w.b, color);
    color = mad(texture(colorTexture, coords.zw), w.a, color);
    #else
    vec4 C = texture(colorTexture, TexCoords);
    vec4 Cleft = texture(colorTexture, offset[0].xy);
    vec4 Ctop = texture(colorTexture, offset[0].zw);
    vec4 Cright = texture(colorTexture, offset[1].xy);
    vec4 Cbottom = texture(colorTexture, offset[1].zw);
    color = mad(lerp(C, Ctop, a.r), w.r, color);
    color = mad(lerp(C, Cbottom, a.g), w.g, color);
    color = mad(lerp(C, Cleft, a.b), w.b, color);
    color = mad(lerp(C, Cright, a.a), w.a, color);
    #endif

// Normalize the resulting color and we are finished!
    return (color / sum);
}

vec4 BlendNeighbours()
{
// Fetch the blending weights for current pixel:
    vec4 topLeft    = vec4(texture(blendTexture, TexCoords).rg,texture(blendTextureBA, TexCoords).rg);
    float bottom    = texture(blendTexture, TexCoords + texelSize * vec2(0, -1)).g;
    float right     = texture(blendTextureBA, TexCoords + texelSize * vec2(1, 0)).g;
    vec4 a = vec4(topLeft.r, bottom, topLeft.b, right);

// Up to 4 lines can be crossing a pixel (one in each edge). So, we perform
// a weighted average, where the weight of each line is 'a' cubed, which
// favors blending and works well in practice.
    vec4 w = a * a * a;
    vec4 color = vec4(0.0);

// There is some blending weight with a value greater than 0.0?
    float sum = dot(w, vec4(1.0));

    if (sum < 1e-5)
        return texture(colorTexture, TexCoords);

// Add the contributions of the possible 4 lines that can cross this pixel:
    vec4 CCenter    = texture(colorTexture, TexCoords);
    vec4 Cleft      = texture(colorTexture, TexCoords + texelSize * vec2(-1, 0));
    vec4 Ctop       = texture(colorTexture, TexCoords + texelSize * vec2(0, 1));
    vec4 Cright     = texture(colorTexture, TexCoords + texelSize * vec2(1, 0));
    vec4 Cbottom    = texture(colorTexture, TexCoords + texelSize * vec2(0, -1));
    color = mix(CCenter, Ctop, a.r)       * w.r + color;
    color = mix(CCenter, Cbottom, a.g)    * w.g + color;
    color = mix(CCenter, Cleft, a.b)      * w.b + color;
    color = mix(CCenter, Cright, a.a)     * w.a + color;

// Normalize the resulting color and we are finished!
    return (color / sum);
}

void main()
{
    FragColor = NeighborhoodBlendingPS();
}