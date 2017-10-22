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

#include "CopyFramebufferShader.h"
#include "OpenGLFramebuffer.h"
#include "SwapFramebuffer.h"

constexpr GLfloat depthValue[1] = { 1.0f };
constexpr GLfloat depthValueTransparent[1] = { 0.0f };
constexpr GLuint indexValue[4] = { 0, 0, 0, 0 };

SwapFramebuffer::SwapFramebuffer(sint32 width, sint32 height) :
_width(width), _height(height), _hasMixed(false), _opaqueFramebuffer(width, height),
_transparentFramebuffer(width, height), _mixFramebuffer(width, height, false),
_backDepth(OpenGLFramebuffer::CreateDepthTexture(width, height)),
_countFramebuffer(1, 1, false, false)
{
    _transparentFramebuffer.Bind();
    glClearBufferfv(GL_DEPTH, 0, depthValueTransparent);
}

bool SwapFramebuffer::ApplyTransparency(ApplyTransparencyShader &applyShader, CountTransparencyShader &countShader, GLuint paletteTex)
{
    bool stuff = countShader.Draw(_transparentFramebuffer);
    
    if (stuff)
    {
        _mixFramebuffer.Bind();
        glDisable(GL_DEPTH_TEST);
        applyShader.Use();
        applyShader.SetTextures(
                                _opaqueFramebuffer.GetTexture(),
                                _opaqueFramebuffer.GetDepthTexture(),
                                _transparentFramebuffer.GetTexture(),
                                _transparentFramebuffer.GetDepthTexture(),
                                paletteTex
                                );
        applyShader.Draw();
        
        _backDepth = _transparentFramebuffer.SwapDepthTexture(_backDepth);
        
        // Clear transparency buffers
        _transparentFramebuffer.Bind();
        glClearBufferuiv(GL_COLOR, 0, indexValue);
        glClearBufferfv(GL_DEPTH, 0, depthValueTransparent);
        
        _opaqueFramebuffer.SwapColourBuffer(_mixFramebuffer);
        //Change binding to guaruntee no undefined behavior
        _opaqueFramebuffer.Bind();
    }
    
    return stuff;
}

void SwapFramebuffer::Clear()
{
    _opaqueFramebuffer.Bind();
    glClearBufferfv(GL_DEPTH, 0, depthValue);
}

#endif /* DISABLE_OPENGL */
