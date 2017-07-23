//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
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
//	System specific interface stuff.
//

namespace theta
{

namespace system
{

namespace detail
{

void Error3(const fmt::MemoryWriter& buffer);

// This wrapper function simply combines the arguments into a buffer and
// passes it to the actual error handler.
template<typename... Arguments>
void Error2(const char* file, int line, const char* error, const Arguments&... args)
{
    fmt::MemoryWriter buffer;
    buffer << file << ':' << line << ' ' << fmt::sprintf(error, args...);
    Error3(buffer);
}

}

}

}
