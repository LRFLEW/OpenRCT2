#version 150

uniform usampler2D uTexture;

in vec2 fTextureCoordinate;

out float oColour;

void main()
{
    oColour = sign(texture(uTexture, fTextureCoordinate).r);
}
