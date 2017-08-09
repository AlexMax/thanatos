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

#include "i_swap.h"
#include "v_buffer.h"

namespace theta
{

namespace video
{

// Initialize the buffer with the contents of a patch.
RGBABuffer::RGBABuffer(const patch_t* patch, const byte* palette) :
    width(patch->width), height(patch->height), pixels(patch->width * patch->height * 4)
{
    pixel_t* desttop = this->pixels.data();
    int w = SHORT(patch->width);

    for (int col = 0;col < w;col++, desttop += 4)
    {
        column_t* column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            byte* source = (byte *)column + 3;
            pixel_t* dest = desttop + column->topdelta * this->width * 4;
            int count = column->length;

            while (count--)
            {
                *dest = palette[(*source) * 3];           // Red
                *(dest + 1) = palette[(*source) * 3 + 1]; // Green
                *(dest + 2) = palette[(*source) * 3 + 2]; // Blue
                *(dest + 3) = 0xFF;                       // Alpha

                source++;
                dest += this->width * 4;
            }
            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

}

}