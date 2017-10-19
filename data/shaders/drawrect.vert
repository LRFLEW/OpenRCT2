#version 150

// Allows for about 8 million draws per frame
const float DEPTH_INCREMENT = 1.0 / float(1u << 22u);

uniform ivec2 uScreenSize;

in ivec4 vClip;
in int   vTexColourAtlas;
in vec4  vTexColourBounds;
in int   vTexMaskAtlas;
in vec4  vTexMaskBounds;
in ivec3 vPalettes;
in int   vFlags;
in uint  vColour;
in ivec4 vBounds;
in int   vDepth;

in mat4x2 vVertMat;
in vec2   vVertVec;

out vec2       fPosition;
flat out int   fFlags;
flat out uint  fColour;
out vec3       fTexColour;
out vec3       fTexMask;
flat out vec3  fPalettes;

void main()
{
    // Clamp position by vClip, correcting interpolated values for the clipping
    vec2 m = clamp(((vVertMat * vClip) - (vVertMat * vBounds))/(vBounds.zw - vBounds.xy) + vVertVec, 0.0, 1.0);
    vec2 pos = mix(vBounds.xy, vBounds.zw, m);
    fTexColour = vec3(mix(vTexColourBounds.xy, vTexColourBounds.zw, m), vTexColourAtlas);
    fTexMask = vec3(mix(vTexMaskBounds.xy, vTexMaskBounds.zw, m), vTexMaskAtlas);

    fPosition = pos;

    // Transform screen coordinates to viewport
    pos.x = (pos.x * (2.0 / uScreenSize.x)) - 1.0;
    pos.y = (pos.y * (2.0 / uScreenSize.y)) - 1.0;
    pos.y *= -1;
    float depth = 1.0 - vDepth * DEPTH_INCREMENT;

    fFlags = vFlags;
    fColour = vColour;
    fPalettes = vec3(vPalettes);

    gl_Position = vec4(pos, depth, 1.0);
}
