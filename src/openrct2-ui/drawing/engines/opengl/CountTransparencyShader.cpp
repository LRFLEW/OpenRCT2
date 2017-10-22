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

#include <algorithm>

#include "CountTransparencyShader.h"
#include <openrct2/util/util.h>

inline sint32 floor_log2(sint32 v)
{
    sint32 result = 0;
    while (v > 0)
    {
        v >>= 1;
        ++result;
    }
    return result;
}

struct VDStruct {
    GLfloat position[2];
    GLfloat texturecoordinate[2];
};

constexpr VDStruct VertexData[4] =
{
    { -1.0f, -1.0f, 0.0f, 0.0f },
    {  1.0f, -1.0f, 1.0f, 0.0f },
    { -1.0f,  1.0f, 0.0f, 1.0f },
    {  1.0f,  1.0f, 1.0f, 1.0f },
};

CountTransparencyShader::CountTransparencyShader() : OpenGLShaderProgram("counttransparency")
{
    GetLocations();
    
    glGenBuffers(1, &_vbo);
    glGenVertexArrays(1, &_vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData), VertexData, GL_STATIC_DRAW);
    
    glBindVertexArray(_vao);
    glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, sizeof(VDStruct), (void*) offsetof(VDStruct, position));
    glVertexAttribPointer(vTextureCoordinate, 2, GL_FLOAT, GL_FALSE, sizeof(VDStruct), (void*) offsetof(VDStruct, texturecoordinate));
    
    glEnableVertexAttribArray(vPosition);
    glEnableVertexAttribArray(vTextureCoordinate);
    
    Use();
    glUniform1i(uTexture, 0);
}

CountTransparencyShader::~CountTransparencyShader()
{
    glDeleteBuffers(1, &_vbo);
    glDeleteVertexArrays(1, &_vao);
}

void CountTransparencyShader::GetLocations()
{
    uScreenSize         = GetUniformLocation("uScreenSize");
    uTexture            = GetUniformLocation("uTexture");
    
    vPosition           = GetAttributeLocation("vPosition");
    vTextureCoordinate  = GetAttributeLocation("vTextureCoordinate");
}

bool CountTransparencyShader::Draw(OpenGLFramebuffer &fbo)
{
    OpenGLFramebuffer *last = &fbo;
    Use();
    glBindVertexArray(_vao);
    
    while (last->GetWidth() > 1 || last->GetHeight() > 1)
    {
        sint32 width = (last->GetWidth() / 2) + (last->GetWidth() % 2);
        sint32 height = (last->GetHeight() / 2) + (last->GetHeight() % 2);
        OpenGLFramebuffer *next = new OpenGLFramebuffer(width, height, false);
        
        next->Bind();
        OpenGLAPI::SetTexture(0, GL_TEXTURE_2D, last->GetTexture());
        glUniform2i(uScreenSize, last->GetWidth(), last->GetHeight());
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        
        if (last != &fbo) delete last;
        last = next;
    }
    
    GLuint out = last->GetTexture();
    glBindTexture(GL_TEXTURE_2D, out);
    uint8 result[1];
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, result);
    
    if (last != &fbo) delete last;
    return result[0] != 0;
}

#endif /* DISABLE_OPENGL */
