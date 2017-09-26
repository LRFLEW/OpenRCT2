#pragma region Copyright (c) 2014-2017 OpenRCT2 Developers
/*****************************************************************************
 * OpenRCT2, an open source clone of Roller Coaster Tycoon 2.
 *
 * OpenRCT2 is the work of many authors, a full list can be found in contributors.md
 * For more information, visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * A full copy of the GNU General Public License can be found in licence.txt
 *****************************************************************************/
#pragma endregion

#pragma once

#include "GLSLTypes.h"
#include "OpenGLShaderProgram.h"

class OpenGLFramebuffer;

/*
 struct DrawImageInstance {
 vec4i clip;
 sint32 texColourAtlas;
 vec4f texColourBounds;
 sint32 texPaletteAtlas;
 vec4f texPaletteBounds;
 sint32 flags;
 uint8 colour;
 vec4i bounds;
 sint32 mask;
 
 enum
 {
 FLAG_COLOUR              = (1 << 0),
 FLAG_REMAP               = (1 << 1),
 };
 };
 */

class DrawGlassShader final : public OpenGLShaderProgram
{
private:
    GLuint uScreenSize;
    GLuint uTexColourBounds;
    GLuint uClip;
    GLuint uBounds;
    GLuint uTexture;
    GLuint uSourceFramebuffer;
    GLuint uTexColourAtlas;
    GLuint uTexPaletteBounds;
    GLuint uTexPaletteAtlas;

    GLuint vIndex;

    GLuint _vbo;
    GLuint _vao;

    GLuint _texture = 0;
    GLuint _sourceFramebuffer = 0;

public:
    DrawGlassShader();
    ~DrawGlassShader() override;

    void SetScreenSize(sint32 width, sint32 height);
    void SetClip(sint32 left, sint32 top, sint32 right, sint32 bottom);
    void SetBounds(sint32 left, sint32 top, sint32 right, sint32 bottom);
    void SetTexture(GLuint texture);
    void SetSourceFramebuffer(GLuint texture);
    void SetTexColour(vec4f bounds, sint32 atlas);
    void SetTexPalette(vec4f bounds, sint32 atlas);

    void Draw(sint32 left, sint32 top, sint32 right, sint32 bottom);

    GLuint GetSourceFramebuffer() const;

private:
    void GetLocations();
};
