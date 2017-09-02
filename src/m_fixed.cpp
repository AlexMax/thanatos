//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
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
//	Fixed point implementation.
//



#include "stdlib.h"

#include "doomtype.h"
#include "i_system.h"

#include "m_fixed.h"


namespace theta
{

// Fixme. __USE_C_FIXED__ or something.

fixed_t
FixedMul
( fixed_t	a,
  fixed_t	b )
{
    return ((int64_t) a * (int64_t) b) >> FRACBITS;
}



//
// FixedDiv, C version.
//

fixed_t FixedDiv(fixed_t a, fixed_t b)
{
    if ((abs(a) >> 14) >= abs(b))
    {
	return (a^b) < 0 ? INT_MIN : INT_MAX;
    }
    else
    {
	int64_t result;

	result = ((int64_t) a << FRACBITS) / b;

	return (fixed_t) result;
    }
}

// Turn a fixed-point number into a floating point number.
// WARNING: DO NOT USE THIS IN GAMEPLAY CODE
double FixedToFloat(fixed_t f)
{
    double whole = f >> FRACBITS;
    double frac = (f & (FRACUNIT - 1)) / static_cast<double>(FRACUNIT);
    return whole + frac;
}

// Turn a floating-point number into a fixed-point number.
// WARNING: DO NOT USE THIS IN GAMEPLAY CODE
fixed_t FloatToFixed(double d)
{
    int whole = static_cast<int>(d);
    assert(whole < FRACUNIT && whole > -FRACUNIT);
    int frac = static_cast<int>((d - whole) * FRACUNIT);
    return (whole << FRACBITS) + frac;
}

}
