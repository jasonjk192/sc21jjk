#version 400

#define mad(m,a,b) m*a+b

out vec4 FragColor;

in vec2 TexCoords;
in vec4 offset[2];

uniform vec2 texelSize;
uniform int maxDistance;
uniform int maxSearchSteps;

uniform sampler2D edgeTexture;
uniform sampler2D luminanceTexture;
uniform sampler2D areaTexture;

float TextureLuminance(vec2 uv, vec2 offset)
{
    vec4 color = texture(luminanceTexture,  uv + texelSize * offset);
    return (0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b);
}

float SearchXLeft(vec2 texcoord){
    float i;
    float e = 0.0;
    for (i = -1.5; i > -2.0 * maxSearchSteps; i -= 2.0){
        e = TextureLuminance(texcoord, vec2(i, 0.0));
        if (e < 0.9) break;
    }
    return max(i + 1.5 - 2.0 * e, -2.0 * maxSearchSteps);
}

float SearchXRight(vec2 texcoord) {
    float i;
    float e = 0.0;
    for (i = 1.5; i < 2.0 * maxSearchSteps; i += 2.0) {
        e = TextureLuminance(texcoord, vec2(i, 0.0));
        if (e < 0.9) break;
    }
    return min(i - 1.5 + 2.0 * e, 2.0 * maxSearchSteps);
}

float SearchYDown(vec2 texcoord) {
    float i;
    float e = 0.0;
    for (i = -1.5; i > -2.0 * maxSearchSteps; i -= 2.0) {
        e = TextureLuminance(texcoord, vec2(i, 0.0).yx);
        if (e < 0.9) break;
    }
    return max(i + 1.5 - 2.0 * e, -2.0 * maxSearchSteps);
}

float SearchYUp(vec2 texcoord) {
    float i;
    float e = 0.0;
    for (i = 1.5; i < 2.0 * maxSearchSteps; i += 2.0) {
        e = TextureLuminance(texcoord, vec2(i, 0.0).yx);
        if (e < 0.9) break;
    }
    return min(i - 1.5 + 2.0 * e, 2.0 * maxSearchSteps);
}

vec2 Area(vec2 distance, float e1, float e2)
{
    float areaSize = maxDistance * 5.0;
    vec2 pixcoord = maxDistance * round(4.0 * vec2(e1, e2)) + distance;
    vec2 texcoord = pixcoord / (areaSize - 1.0);
    return texture(areaTexture, texcoord).rg;
}

vec4 BlendWeights(vec2 uv)
{
    vec4 weights = vec4(0);
    vec2 e = texture(edgeTexture, uv).rg;
    if(e.g != 0.0)
    {
        vec2 d = vec2(SearchXLeft(uv), SearchXRight(uv));
        vec4 coords = vec4(d.x, 0.25, d.y + 1.0, 0.25) * texelSize.xyxy + uv.xyxy;
        float e1 = texture(edgeTexture, coords.xy).r;
        float e2 = texture(edgeTexture, coords.zw).r;
        weights.rg = Area(abs(d), e1, e2);
    }
    if(e.r != 0.0)
    {
        vec2 d = vec2(SearchYUp(uv), SearchYDown(uv));
        vec4 coords = vec4(-0.25, d.x, -0.25, d.y - 1.0) * texelSize.xyxy + uv.xyxy;
        float e1 = texture(edgeTexture, coords.xy).g;
        float e2 = texture(edgeTexture, coords.zw).g;
        weights.ba = Area(abs(d), e1, e2);
    }
    return weights;
}

vec4 BlendWeightCalculationPS()
{
    vec4 areas = vec4(0.0);

    vec4 e = texture(edgeTexture, TexCoords);

    if (e.g > 0.0) { // Edge at north

    // Search distances to the left and to the right:
        vec2 d = vec2(SearchXLeft(TexCoords), SearchXRight(TexCoords));

    // Now fetch the crossing edges. Instead of sampling between edgels, we
    // sample at -0.25, to be able to discern what value has each edgel:
        vec4 coords = mad(vec4(d.x, -0.25, d.y + 1.0, -0.25),texelSize.xyxy, TexCoords.xyxy);
        float e1 = texture(edgeTexture, coords.xy).r;
        float e2 = texture(edgeTexture, coords.zw).r;

    // Ok, we know how this pattern looks like, now it is time for getting
    // the actual area:
        areas.rg = Area(abs(d), e1, e2);
    }

    if (e.r > 0.0){ // Edge at west

    // Search distances to the top and to the bottom:
        vec2 d = vec2(SearchYUp(TexCoords), SearchYDown(TexCoords));

    // Now fetch the crossing edges (yet again):
        vec4 coords = mad(vec4(-0.25, d.x, -0.25, d.y + 1.0), texelSize.xyxy, TexCoords.xyxy);
        float e1 = texture(edgeTexture, coords.xy).g;
        float e2 = texture(edgeTexture, coords.zw).g;

    // Get the area for this direction:
        areas.ba = Area(abs(d), e1, e2);
    }
    return areas;
}

void main()
{
    FragColor = BlendWeightCalculationPS();
}