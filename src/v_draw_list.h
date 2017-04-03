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

#ifndef __DRAW_LIST__
#define __DRAW_LIST__

#include <functional>
#include <vector>

#include "doomtype.h"
#include "v_patch.h"

namespace video
{

typedef std::function<void(int, int, patch_t*)> DrawFunction;

// A type that stores a single draw call, along with the patch and
// x, y coordinates.
struct DrawEdict
{
    DrawFunction drawer;
    patch_t* patch;
    int x;
    int y;
};

// A type that stores a list of draw functions, with their associated
// patches and x, y coordinates.
class DrawList
{
private:
    std::vector<DrawEdict> edicts;
    int width;
    int height;
public:
    void Add(DrawFunction func, patch_t* patch, int x, int y);
    void Draw(int x, int y) const;
    int GetWidth() const;
    void SetWidth(int w);
    int GetHeight() const;
    void SetHeight(int h);
    void Clear();
};

}

#endif