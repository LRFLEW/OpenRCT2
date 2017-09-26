#version 150

uniform usampler2DArray uTexture;
uniform usampler2D      uSourceFramebuffer;

uniform ivec4           uClip;
uniform int             uTexColourAtlas;
uniform vec4            uTexPaletteBounds;
uniform int             uTexPaletteAtlas;

in vec2 fPosition;
in vec2 fFrame;
in vec2 fTexColourCoords;

out uint oColour;

void main()
{
    if (fPosition.x < uClip.x || fPosition.x > uClip.z ||
        fPosition.y < uClip.y || fPosition.y > uClip.w)
    {
        discard;
    }

    uint line = texture(uTexture, vec3(fTexColourCoords, float(uTexColourAtlas))).r;
    if (line == 0u)
    {
        discard;
    }
    uint texel = texture(uSourceFramebuffer, fFrame).r;

    // z is the size of each x pixel in the atlas
    float x = uTexPaletteBounds.x + uTexPaletteBounds.z * float(texel);
    oColour = texture(uTexture, vec3(x, uTexPaletteBounds.y, float(uTexPaletteAtlas))).r;
    
    return;
}
