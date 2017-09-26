#version 150

const int FLAG_COLOUR              = (1 << 0);
const int FLAG_REMAP               = (1 << 1);

uniform usampler2DArray uTexture;

flat in ivec4           fClip;
flat in int             fFlags;
flat in uint            fColour;
flat in int             fTexColourAtlas;
in vec2                 fTexColourCoords;
flat in int             fTexMaskAtlas;
in vec2                 fTexMaskCoords;
flat in int             fTexPaletteAtlas;
flat in vec4            fTexPaletteBounds;
flat in int             fMask;

in vec2 fPosition;
in vec2 fTextureCoordinate;

out uint oColour;

void main()
{
    if (fPosition.x < fClip.x || fPosition.x > fClip.z ||
        fPosition.y < fClip.y || fPosition.y > fClip.w)
    {
        discard;
    }

    uint texel;

    // If remap palette used
    if ((fFlags & FLAG_REMAP) != 0)
    {
        // z is the size of each x pixel in the atlas
        float x = fTexPaletteBounds.x + texture(uTexture, vec3(fTexColourCoords, float(fTexColourAtlas))).r * fTexPaletteBounds.z;
        texel = texture(uTexture, vec3(x, fTexPaletteBounds.y, float(fTexPaletteAtlas))).r;
    }
    else
    {
        texel = texture(uTexture, vec3(fTexColourCoords, float(fTexColourAtlas))).r;
    }

    if (fMask != 0)
    {
        uint mask = texture(uTexture, vec3(fTexMaskCoords, float(fTexMaskAtlas))).r;
        if ( mask == 0u )
        {
            discard;
        }

        oColour = texel;
    }
    else
    {
        if (texel == 0u)
        {
            discard;
        }
        if ((fFlags & FLAG_COLOUR) != 0)
        {
            oColour = fColour;
        }
        else
        {
            oColour = texel;
        }
    }
}
