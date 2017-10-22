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

#include "CountTransparencyShader.h"

CountTransparencyShader::CountTransparencyShader() : OpenGLShaderProgram("counttransparency")
{
    GetLocations();
    
    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    Use();
    glUniform1i(uTexture, 0);
    
    instanceCount = 0;
}

CountTransparencyShader::~CountTransparencyShader()
{
}

void CountTransparencyShader::GetLocations()
{
    uScreenSize         = GetUniformLocation("uScreenSize");
    uTexture            = GetUniformLocation("uTexture");
}

void CountTransparencyShader::SetScreenSize(sint32 width, sint32 height)
{
    glUniform2i(uScreenSize, width, height);
    instanceCount = width * height;
}

void CountTransparencyShader::Draw(GLuint texture)
{
    Use();
    glBindVertexArray(_vao);
    OpenGLAPI::SetTexture(0, GL_TEXTURE_2D, texture);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDrawArraysInstanced(GL_POINTS, 0, 1, instanceCount);
    glDisable(GL_BLEND);
}

#endif /* DISABLE_OPENGL */
