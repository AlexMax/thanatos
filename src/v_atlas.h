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

#ifndef __V_ATLAS__
#define __V_ATLAS__

#include <string>
#include <unordered_map>

namespace theta
{

namespace video
{

// A single atlas entry.
struct AtlasEntry
{
    int x;
    int y;
    int w;
    int h;
    int xoff; // patch_t offset
    int yoff; // patch_t offset
    AtlasEntry(int x, int y, int w, int h, int xoff, int yoff) :
        x(x), y(y), w(w), h(h), xoff(xoff), yoff(yoff) { }
};

// A shelf of the atlas.
struct AtlasShelf
{
    int w;
    int h;
    AtlasShelf(int w, int h) : w(w), h(h) { }
    AtlasShelf(const AtlasShelf&) = delete;
    AtlasShelf(AtlasShelf&&) = default;
};

// A texture atlas implementation.  It has no knowledge of the underlying
// renderer implementation, so you could possibly use this for other renderer
// implementations.
class Atlas
{
    int width;
    int height;
    std::unordered_map<std::string, AtlasEntry> atlas;
    std::vector<AtlasShelf> shelves;
public:
    Atlas(int width, int height) : width(width), height(height) { }
    Atlas(const Atlas&) = delete;
    void Add(const std::string& lump, int w, int h, int xoff, int yoff);
    bool Check(const char* lump);
    bool Find(const char* lump, AtlasEntry& out);
};

}

}


#endif
