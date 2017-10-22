#version 150

uniform usampler2D uTexture;

in vec2       fTextureCoordinate;
flat in vec2 fTextureDelta;

out float oColour;

void main()
{
    uint a = texture(uTexture, fTextureCoordinate + fTextureDelta * vec2(-1.0, -1.0)).r;
    uint b = texture(uTexture, fTextureCoordinate + fTextureDelta * vec2( 1.0, -1.0)).r;
    uint c = texture(uTexture, fTextureCoordinate + fTextureDelta * vec2(-1.0,  1.0)).r;
    uint d = texture(uTexture, fTextureCoordinate + fTextureDelta * vec2( 1.0,  1.0)).r;
    oColour = max(max(a, b), max(c, d));
}
