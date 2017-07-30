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

// Insert a graphic of width w and height h into the atlas.
void Atlas::Add(const std::string& lump, int w, int h, int xoff, int yoff)
{
    auto entry = this->atlas.find(lump);
    if (entry != this->atlas.end())
    {
        // Atlas entry already exists, silently don't do anything.
        return;
    }

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
                auto res = this->atlas.emplace(lump, AtlasEntry(it->w, y, w, h, xoff, yoff));
                if (res.second == false)
                {
                    I_Error("Couldn't emplace into to the atlas");
                }
                it->w += w;
                return;
            }
        }

        y += it->h;
    }

    // There's no space on an existing shelf.  Do we have space for a new one?
    if (h <= this->height - y)
    {
        // We do!  Create the new shelf and put the atlas entry there.
        this->shelves.emplace_back(AtlasShelf(w, h));
        auto ares = this->atlas.emplace(lump, AtlasEntry(0, y, w, h, xoff, yoff));
        if (ares.second == false)
        {
            I_Error("Couldn't emplace into to the atlas");
        }
        return;
    }

    // No space.
    I_Error("No space left in texture atlas");
}

// Check to see if an atlas entry - any atlas entry - exists for the given
// lump.
bool Atlas::Check(const char* lump)
{
    auto res = this->atlas.find(lump);
    if (res == this->atlas.end())
    {
        return false;
    }
    return true;
}

// Find the atlas entry that belongs to the given lump.
bool Atlas::Find(const char* lump, AtlasEntry& out)
{
    auto res = this->atlas.find(lump);
    if (res == this->atlas.end())
    {
        return false;
    }
    out = res->second;
    return true;
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

