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
//     Graphics Manager.
//

#ifndef __V_GRAPHICS__
#define __V_GRAPHICS__

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "v_buffer.h"
#include "v_patch.h"

namespace theta
{

namespace video
{

struct Graphic
{
    RGBABuffer data;
    int width;
    int height;
    int xoff;
    int yoff;
    Graphic(RGBABuffer data, int width, int height, int xoff, int yoff) :
        data(data), width(width), height(height), xoff(xoff), yoff(yoff) { }
    Graphic(const Graphic&) = delete;
};

// Manage graphic resources.
class GraphicsManager
{
    std::vector<std::unique_ptr<Graphic>> handles;
    std::unordered_map<std::string, const Graphic*> names;
public:
    static GraphicsManager& Instance();
    const Graphic& AddPatch(patch_t* patch);
    const Graphic& LoadPatch(const std::string& name);
};

}

}

#endif