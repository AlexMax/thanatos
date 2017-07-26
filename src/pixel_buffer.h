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
//     A buffer of pixels that knows its own resolution.
//

#ifndef __PIXEL_BUFFER__
#define __PIXEL_BUFFER__

#include <vector>

#include "doomtype.h"

namespace theta
{

// A buffer of pixels that knows its own resolution.
class PixelBuffer
{
    int width;
    int height;
    std::vector<pixel_t> pixels;
    void resize();
public:
    PixelBuffer(int width, int height) :
        width(width), height(height), pixels(width * height) { }
    //PixelBuffer(const PixelBuffer&) = delete;
    int GetWidth() const;
    int GetHeight() const;
    int GetSize() const;
    pixel_t* GetRawPixels();
    const pixel_t* GetRawPixels() const;
};

}

#endif