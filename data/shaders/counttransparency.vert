#version 150

uniform ivec2 uScreenSize;

out vec2 fTextureCoordinate;

void main()
{
    fTextureCoordinate = vec2(mod(gl_InstanceID, uScreenSize.x),
                              floor(gl_InstanceID / uScreenSize.x)) / uScreenSize;
    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
}
