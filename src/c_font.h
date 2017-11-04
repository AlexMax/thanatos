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
//     Console font.
//

#ifndef __C_FONT__
#define __C_FONT__

#include <unordered_map>

#include <gsl/span>

#include "doomtype.h"

namespace theta
{

namespace console
{

typedef std::unordered_map<byte, gsl::span<byte>> Font;
extern Font ConsoleFont;

}

}

#endif