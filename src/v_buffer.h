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

#ifndef __V_BUFFER__
#define __V_BUFFER__

#include <vector>

#include "doomtype.h"
#include "v_patch.h"

namespace theta
{

namespace video
{

// A buffer of 8-bit pixels that knows its own resolution.
class PalletedBuffer
{
    int width;
    int height;
    std::vector<pixel_t> pixels;
public:
    PalletedBuffer(int width, int height) :
        width(width), height(height), pixels(width * height) { }
    inline int GetWidth() const { return this->width; };
    inline int GetHeight() const { return this->height; };
    inline int GetSize() const { return this->width * this->height; };
    pixel_t* GetRawPixels() { return &this->pixels.front(); }
    const pixel_t* GetRawPixels() const { return &this->pixels.front(); };
};

// A buffer of truecolor + transparency pixels that knows its own resolution.
class RGBABuffer
{
    int width;
    int height;
    std::vector<pixel_t> pixels;
public:
    RGBABuffer(int width, int height) :
        width(width), height(height), pixels(width * height * 4) { }
    RGBABuffer(const patch_t* patch, const byte* palette);
    int GetWidth() const { return this->width; };
    int GetHeight() const { return this->height; };
    int GetSize() const { return this->width * this->height; };
    pixel_t* GetRawPixels() { return &this->pixels.front(); }
    const pixel_t* GetRawPixels() const { return &this->pixels.front(); };
};

}

}

#endif