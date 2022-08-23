// Opengl Shading Language 3rd edition

//https://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.719.9043&rep=rep1&type=pdf
//https://igm.univ-mlv.fr/~biri/mlaa-gpu/TMLAA.pdf
//https://www.cs.cmu.edu/afs/cs/academic/class/15869-f11/www/readings/reshetov09_mlaa.pdf

#version 400

out vec4 FragColor;
in vec2 TexCoords;
in vec4 offset[2];
uniform vec2 texelSize;
uniform sampler2D screenTexture;
uniform sampler2D depthTexture;
uniform float fDepthThreshold;
uniform float fLuminanceThreshold;

float TextureLuminance(vec2 uv)
{
    vec4 color = texture(screenTexture, uv);
    return (0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b);
}

vec4 ColorEdgeDetectionPS()
{
    float L         = TextureLuminance(TexCoords);
    float Lleft     = TextureLuminance(offset[0].xy);
    float Ltop      = TextureLuminance(offset[0].zw);
    float Lright    = TextureLuminance(offset[1].xy);
    float Lbottom   = TextureLuminance(offset[1].zw);

    vec4 delta = abs(vec4(L) - vec4(Lleft, Ltop, Lright, Lbottom));
    vec4 edges = step(vec4(fLuminanceThreshold), delta);
    if (dot(edges, vec4(1.0)) == 0.0)
        discard;
    return edges;
}

vec4 DepthEdgeDetectionPS()
{
    float D         = texture(depthTexture, TexCoords).r;
    float Dleft     = texture(depthTexture, offset[0].xy).r;
    float Dtop      = texture(depthTexture, offset[0].zw).r;
    float Dright    = texture(depthTexture, offset[1].xy).r;
    float Dbottom   = texture(depthTexture, offset[1].zw).r;

    vec4 delta = abs(vec4(D) - vec4(Dleft, Dtop, Dright, Dbottom));
    vec4 edges = step(vec4(fDepthThreshold) / 10.0, delta); // Dividing by 10 give us results similar to the color-based detection.

    if (dot(edges, vec4(1.0)) == 0.0)
        discard;
    return edges;
}


vec4 EdgeDetector()
{
    float DepthCenter  = texture(depthTexture, TexCoords).r;
    float DepthLeft    = texture(depthTexture, TexCoords + texelSize * vec2(-1.0, 0.0)).r;
    float DepthRight   = texture(depthTexture, TexCoords + texelSize * vec2(1.0, 0.0)).r;
    float DepthTop     = texture(depthTexture, TexCoords + texelSize * vec2(0.0, 1.0)).r;
    float DepthBottom  = texture(depthTexture, TexCoords + texelSize * vec2(0.0, -1.0)).r;
    vec4 DepthDelta    = abs(vec4(DepthCenter) - vec4(DepthLeft, DepthTop, DepthRight, DepthBottom));
    vec4 Edges         = step(vec4(fDepthThreshold), DepthDelta);

    //Convert RGB to Luminance value
    float LuminanceCenter  = TextureLuminance(TexCoords);
    float LuminanceLeft    = TextureLuminance(TexCoords + texelSize * vec2(-1.0, 0.0));
    float LuminanceRight   = TextureLuminance(TexCoords + texelSize * vec2(1.0, 0.0));
    float LuminanceTop     = TextureLuminance(TexCoords + texelSize * vec2(0.0, 1.0));
    float LuminanceBottom  = TextureLuminance(TexCoords + texelSize * vec2(0.0, -1.0));
    vec4 LuminanceDelta    = abs(vec4(LuminanceCenter) - vec4(LuminanceLeft, LuminanceTop, LuminanceRight, LuminanceBottom));
    Edges += step(vec4(fLuminanceThreshold), LuminanceDelta);

    return Edges;
}

void main()
{
    FragColor = ColorEdgeDetectionPS();
}
