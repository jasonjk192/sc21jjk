#version 400
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;
out vec4 offset[2];

uniform vec2 texelSize;

void main()
{
    TexCoords = vec2((aPos + 1.0) / 2.0);
    gl_Position = vec4(aPos, 0.0, 1.0);
    offset[0] = TexCoords.xyxy + texelSize.xyxy * vec4(-1.0, 0.0, 0.0, -1.0);
    offset[1] = TexCoords.xyxy + texelSize.xyxy * vec4( 1.0, 0.0, 0.0,  1.0);
}