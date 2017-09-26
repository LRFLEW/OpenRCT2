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

#ifndef DISABLE_OPENGL

#include "DrawGlassShader.h"
#include "OpenGLFramebuffer.h"

DrawGlassShader::DrawGlassShader() : OpenGLShaderProgram("drawglass")
{
    GetLocations();

    glGenBuffers(1, &_vbo);
    glGenVertexArrays(1, &_vao);

    GLuint vertices[] = { 0, 1, 2, 2, 1, 3 };
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(_vao);
    glEnableVertexAttribArray(vIndex);
    glVertexAttribIPointer(vIndex, 1, GL_INT, 0, 0);

    Use();
    glUniform1i(uTexture, 0);
    glUniform1i(uSourceFramebuffer, 1);
}

DrawGlassShader::~DrawGlassShader()
{
    glDeleteBuffers(1, &_vbo);
    glDeleteVertexArrays(1, &_vao);
}

void DrawGlassShader::GetLocations()
{
    uScreenSize         = GetUniformLocation("uScreenSize");
    uTexColourBounds    = GetUniformLocation("uTexColourBounds");
    uClip               = GetUniformLocation("uClip");
    uBounds             = GetUniformLocation("uBounds");
    uTexture            = GetUniformLocation("uTexture");
    uSourceFramebuffer  = GetUniformLocation("uSourceFramebuffer");
    uTexColourAtlas     = GetUniformLocation("uTexColourAtlas");
    uTexPaletteBounds   = GetUniformLocation("uTexPaletteBounds");
    uTexPaletteAtlas    = GetUniformLocation("uTexPaletteAtlas");

    vIndex              = GetAttributeLocation("vIndex");
}

void DrawGlassShader::SetScreenSize(sint32 width, sint32 height)
{
    glUniform2i(uScreenSize, width, height);
}

void DrawGlassShader::SetClip(sint32 left, sint32 top, sint32 right, sint32 bottom)
{
    glUniform4i(uClip, left, top, right, bottom);
}

void DrawGlassShader::SetBounds(sint32 left, sint32 top, sint32 right, sint32 bottom)
{
    glUniform4i(uBounds, left, top, right, bottom);
}

void DrawGlassShader::SetTexture(GLuint texture) {
    _texture = texture;
    OpenGLAPI::SetTexture(0, GL_TEXTURE_2D, texture);
}

void DrawGlassShader::SetSourceFramebuffer(GLuint texture)
{
    _sourceFramebuffer = texture;
    OpenGLAPI::SetTexture(1, GL_TEXTURE_2D, texture);
}

void DrawGlassShader::SetTexColour(vec4f bounds, sint32 atlas) {
    glUniform4fv(uTexColourBounds, 1, reinterpret_cast<float *>(&bounds));
    glUniform1i(uTexColourAtlas, atlas);
}

void DrawGlassShader::SetTexPalette(vec4f bounds, sint32 atlas) {
    glUniform4fv(uTexPaletteBounds, 1, reinterpret_cast<float *>(&bounds));
    glUniform1i(uTexPaletteAtlas, atlas);
}

void DrawGlassShader::Draw(sint32 left, sint32 top, sint32 right, sint32 bottom)
{
    SetBounds(left, top, right, bottom);

    glBindVertexArray(_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

GLuint DrawGlassShader::GetSourceFramebuffer() const
{
    return _sourceFramebuffer;
}

#endif /* DISABLE_OPENGL */
