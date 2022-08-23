#version 400 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture_diffuse1;

void main()
{
    vec4 oColor = texture(texture_diffuse1, TexCoord);
    FragColor = oColor;
}