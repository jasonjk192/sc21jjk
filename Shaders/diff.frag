#version 400
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D defaultTexture;
uniform sampler2D shadedTexture;

void main()
{
    vec4 defColor = texture(defaultTexture, TexCoords);
    vec4 shadedColor = texture(shadedTexture, TexCoords);
    vec4 diff = abs(defColor - shadedColor);
    if(diff.r == 0 && diff.g == 0 && diff.b == 0)
        discard;
    FragColor = vec4(diff.rgb, 1.0);
}