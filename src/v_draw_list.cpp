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
//     Draw List.  Stores an arrangement of draw calls that can be
//     iterated over quickly.
//

#include "v_draw_list.h"

namespace video
{

// Add a edict to the draw list.  Each edict is a function, the patch
// drawn by that function, and its position relative to 0, 0.
void DrawList::Add(DrawFunction func, patch_t* patch, int x, int y)
{
    DrawEdict edict{ func, patch, x, y };
    this->edicts.push_back(edict);
}

// Draw the drawlist at a specific x, y offset.
void DrawList::Draw(int x, int y) const
{
    for (const DrawEdict& edict : this->edicts)
    {
        edict.drawer(x + edict.x, y + edict.y, edict.patch);
    }
}

// Get the total width of the drawlist.
int DrawList::GetWidth() const
{
    return this->width;
}

// Set the total width of the drawlist.  Use this so you can reason
// about how much total space your drawlist actually takes up.
void DrawList::SetWidth(int w)
{
    this->width = w;
}

// Get the total height of the drawlist.
int DrawList::GetHeight() const
{
    return this->height;
}

// Set the total height of the drawlist.  Use this so you can reason
// about how much total space your drawlist actually takes up.
void DrawList::SetHeight(int h)
{
    this->height = h;
}

// Empty the drawlist, perhaps for reuse.
void DrawList::Clear()
{
    this->edicts.clear();
    this->width = 0;
    this->height = 0;
}

}