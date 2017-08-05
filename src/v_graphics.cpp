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

#include "deh_str.h"
#include "i_video.h"
#include "v_graphics.h"
#include "w_wad.h"
#include "z_zone.h"

namespace theta
{

namespace video
{

// Get the graphics manager singleton instance.
GraphicsManager& GraphicsManager::Instance()
{
    static GraphicsManager singleton;
    return singleton;
}

// Add a patch with given patch data to the graphics manager.
//
// Returns a unqiue handle to the graphic that you can use to reference
// the graphic therafter.  Don't lose it, there's no duplicate detection.
const Graphic& GraphicsManager::AddPatch(patch_t* patch)
{
    // Create a Graphic handle with our patch data.
    auto doompal = static_cast<byte*>(W_CacheLumpName(DEH_String("PLAYPAL"), PU_CACHE));
    this->handles.emplace_back(std::make_unique<Graphic>(RGBABuffer(patch, doompal), patch->width, patch->height, -patch->leftoffset, -patch->topoffset));
    const Graphic& handle = *((this->handles.cend() - 1)->get());

    // Put the graphic associated with the handle into the texture atlas
    // if our renderer is active.
    if (system::renderer != nullptr) {
        system::renderer->AddGraphic(handle);
    }

    return handle;
}

// Load patch data with a given name into the graphics manager, or return
// an already-existant handle.
//
// Returns a unique handle to the graphic that you can use to reference
// the graphic therafter.  This method has dupe detection based on the
// graphic name, so you can lose the patch all you like.
const Graphic& GraphicsManager::LoadPatch(const std::string& name)
{
    auto found = this->names.find(name);
    if (found == this->names.end())
    {
        // Not found.  Load the patch and return a handle to it.
        auto patch = static_cast<patch_t*>(W_CacheLumpName(name.c_str(), PU_CACHE));
        const Graphic& handle = this->AddPatch(patch);
        this->names.emplace(name, &handle);
        return handle;
    }
    else
    {
        // Found.  Return the handle.
        return *(found->second);
    }
}

// Get the name of a handle.
//
// This involves a complete scan of the internal hashtable.  You don't
// actually need to know the name of the graphic unless you're debugging
// it.  And if you actually do...well, I guess I'll need to implement this
// feature properly at that point...
const std::string GraphicsManager::DebugNameOf(const Graphic& handle)
{
    for (const auto& name : this->names)
    {
        if (name.second == &handle)
        {
            return name.first;
        }
    }

    return "(unknown)";
}

}

}

