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
//     Scripting interface.
//

#include "duktape.h"

#include "i_system.h"

namespace script
{

// RAII wrapping class for Duktape heap.
class DukContext
{
    duk_context* ctx;
public:
    DukContext();
    ~DukContext();
    DukContext(const DukContext&) = delete;
    static DukContext& Instance();
};

DukContext::DukContext() : ctx(duk_create_heap_default())
{
    if (this->ctx == NULL)
    {
        I_Error("Could not create scripting heap");
    }
}

DukContext::~DukContext()
{
    duk_destroy_heap(this->ctx);
}

// Get the singleton instance of the class.

DukContext& DukContext::Instance()
{
    static DukContext singleton;
    return singleton;
}

void Init()
{
    DukContext::Instance();

    printf("Initialized Duktape %s instance.\n", DUK_GIT_DESCRIBE);
    return;
}

}
