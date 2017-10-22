#version 150

uniform ivec2 uScreenSize;

in vec4 vPosition;
in vec2 vTextureCoordinate;

out vec2      fTextureCoordinate;
flat out vec2 fTextureDelta;

void main()
{
    fTextureCoordinate = vTextureCoordinate;
    fTextureDelta = vec2(0.5, 0.5) / uScreenSize;
    gl_Position = vPosition;
}

