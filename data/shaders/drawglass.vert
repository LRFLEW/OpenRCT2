#version 150

uniform ivec2 uScreenSize;
uniform vec4  uTexColourBounds;
uniform ivec4 uBounds;

in uint vIndex;

out vec2       fPosition;
out vec2       fFrame;
out vec2       fTexColourCoords;

void main()
{
    vec2 pos;
    switch (vIndex) {
    case 0u:
        pos = uBounds.xy;
        fTexColourCoords = uTexColourBounds.xy;
        break;
    case 1u:
        pos = uBounds.zy;
        fTexColourCoords = uTexColourBounds.zy;
        break;
    case 2u:
        pos = uBounds.xw;
        fTexColourCoords = uTexColourBounds.xw;
        break;
    case 3u:
        pos = uBounds.zw;
        fTexColourCoords = uTexColourBounds.zw;
        break;
    }

    fPosition = pos;

    // Transform screen coordinates to viewport
    pos.x = (pos.x * (2.0 / uScreenSize.x)) - 1.0;
    pos.y = (pos.y * (2.0 / uScreenSize.y)) - 1.0;
    pos.y *= -1;

    gl_Position = vec4(pos, 0.0, 1.0);
    
    fFrame = (pos + 1.0) * 0.5;
}
