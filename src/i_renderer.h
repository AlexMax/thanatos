//
// Copyright(C) 2017 Alex Mayfield
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//     Renderer interface.  Subclass this for your own renderer implementation.
//

#ifndef __I_RENDERER__
#define __I_RENDERER__

#include "doomtype.h"
#include "v_buffer.h"
#include "v_graphics.h"

namespace theta
{

namespace system
{

// Subclass this for your own renderer implementation.
class RendererInterface
{
public:
    virtual void Flip() = 0;
    virtual void Render() = 0;
    virtual void AddGraphic(const video::Graphic& handle) = 0;
    virtual void DrawGraphic(const video::Graphic& handle, int x, int y, double scalex, double scaley) = 0;
    virtual void DrawMapLine(double x1, double y1, double x2, double y2) = 0;
    virtual int GetHeight() const = 0;
    virtual int GetWidth() const = 0;
    virtual void SetPageGraphic(const video::Graphic& handle) = 0;
    virtual void SetResolution(int width, int height) = 0;
    virtual void SetWorldPalette(const byte* palette) = 0;
    virtual void SetWorldPixels(const video::PalletedBuffer& pixels) = 0;
    virtual void SetWorldSize(double x, double y, double width, double height) = 0;
};

}

}

#endif
