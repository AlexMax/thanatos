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

#include "pixel_buffer.h"

namespace theta
{

// Resize the pixel buffer for the presumably updated width and height.
void PixelBuffer::resize()
{
    this->pixels.resize(this->width * this->height);
}

// Get the width of the buffer.
int PixelBuffer::GetWidth() const
{
    return this->width;
}

// Get the height of the buffer.
int PixelBuffer::GetHeight() const
{
    return this->height;
}

// Get raw write access to the pixel buffer.
pixel_t* PixelBuffer::GetRawPixels()
{
    return &this->pixels.front();
}

// Get raw read-only access to the pixel buffer.
const pixel_t* PixelBuffer::GetRawPixels() const
{
    return &this->pixels.front();
}

}