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
//     A dynamically-sized view of an array.  Does not concern itself with
//     the underlying lifetime of the array.

#ifndef __ARRAY_VIEW__
#define __ARRAY_VIEW__

#include <array>
#include <cstddef>

#include "doomtype.h"

namespace theta
{

class ArrayView
{
private:
    const byte* data = nullptr;
    std::size_t size;
public:
    // Initialize array view with array of arbitrary size.
    template <std::size_t SIZE>
    ArrayView(std::array<byte, SIZE>& a)
        : data(a.data()), size(a.size()) { }

    const byte* GetData();
    std::size_t GetSize();
};

}

#endif
