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
//     Texture atlas.
//

#include "v_atlas.h"

#include "i_system.h"

namespace theta
{

namespace video
{

// Insert a graphic into the atlas.  Returns true if the atlas entry is
// brand new, false if the texture already existed in the atlas.
bool Atlas::Add(const Graphic& handle, AtlasEntry& out)
{
    auto entry = this->atlas.find(&handle);
    if (entry != this->atlas.end())
    {
        // Atlas entry already exists, just return it.
        out = entry->second;
        return false;
    }

    // FIXME: Once I figure out if the Graphic or RGABBuffer will hold
    //        the width and height, come back and fix this.
    auto w = handle.data.GetWidth();
    auto h = handle.data.GetHeight();

    if (w > this->width || h > this->height)
    {
        I_Error("Texture is too big for the atlas");
    }

    int y = 0;
    for (auto it = this->shelves.begin();it != this->shelves.end();++it)
    {
        // Can the shelf hold it?
        if (h <= it->h)
        {
            // Is there space on the shelf?
            if (w <= this->width - it->w)
            {
                // There is!  Put the altas entry there, then adjust the shelf.
                auto res = this->atlas.emplace(&handle, AtlasEntry(it->w, y, w, h));
                if (res.second == false)
                {
                    I_Error("Couldn't emplace into to the atlas");
                }
                it->w += w;
                out = res.first->second;
                return true;
            }
        }

        y += it->h;
    }

    // There's no space on an existing shelf.  Do we have space for a new one?
    if (h <= this->height - y)
    {
        // We do!  Create the new shelf and put the atlas entry there.
        this->shelves.emplace_back(AtlasShelf(w, h));
        auto ares = this->atlas.emplace(&handle, AtlasEntry(0, y, w, h));
        if (ares.second == false)
        {
            I_Error("Couldn't emplace into to the atlas");
        }
        out = ares.first->second;
        return true;
    }

    // No space.
    I_Error("No space left in texture atlas");

    return false; // Unreachable
}

// Get height of atlas.
int Atlas::GetHeight() const
{
    return this->width;
}

// Get width of atlas.
int Atlas::GetWidth() const
{
    return this->height;
}

}

}

