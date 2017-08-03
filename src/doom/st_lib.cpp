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
//	The status bar widget code.
//


#include <stdio.h>
#include <ctype.h>

#include "deh_main.h"
#include "doomdef.h"

#include "z_zone.h"
#include "v_video.h"

#include "i_swap.h"
#include "i_system.h"

#include "w_wad.h"

#include "st_stuff.h"
#include "st_lib.h"
#include "r_local.h"


// in AM_map.c
extern boolean		automapactive; 




//
// Hack display negative frags.
//  Loads and store the stminus lump.
//
patch_t*		sttminus;

void STlib_init(void)
{
    sttminus = (patch_t *) W_CacheLumpName(DEH_String("STTMINUS"), PU_STATIC);
}


// ?
void
STlib_initNum
( st_number_t*		n,
  int			x,
  int			y,
  patch_t**		pl,
  int*			num,
  boolean*		on,
  int			width )
{
    n->x	= x;
    n->y	= y;
    n->oldnum	= 0;
    n->width	= width;
    n->num	= num;
    n->on	= on;
    n->p	= pl;
}


// 
// A fairly efficient way to draw a number
//  based on differences from the old number.
// Note: worth the trouble?
//
void
STlib_drawNum
( st_number_t*	n,
  boolean	refresh )
{

    int		numdigits = n->width;
    int		num = *n->num;
    
    int		w = SHORT(n->p[0]->width);
    int		h = SHORT(n->p[0]->height);
    int		x = n->x;
    
    int		neg;

    n->oldnum = *n->num;

    neg = num < 0;

    if (neg)
    {
	if (numdigits == 2 && num < -9)
	    num = -9;
	else if (numdigits == 3 && num < -99)
	    num = -99;
	
	num = -num;
    }

    // clear the area
    x = n->x - numdigits*w;

    if (n->y - ST_Y < 0)
	I_Error("drawNum: n->y - ST_Y < 0");

    V_CopyRect(x, n->y - ST_Y, st_backing_screen, w*numdigits, h, x, n->y);

    // if non-number, do not draw it
    if (num == 1994)
	return;

    x = n->x;

    // in the special case of 0, you draw 0
    if (!num)
	V_DrawPatch(x - w, n->y, n->p[ 0 ]);

    // draw the new number
    while (num && numdigits--)
    {
	x -= w;
	V_DrawPatch(x, n->y, n->p[ num % 10 ]);
	num /= 10;
    }

    // draw a minus sign if necessary
    if (neg)
	V_DrawPatch(x - 8, n->y, sttminus);
}


//
void
STlib_updateNum
( st_number_t*		n,
  boolean		refresh )
{
    if (*n->on) STlib_drawNum(n, refresh);
}


//
void
STlib_initPercent
( st_percent_t*		p,
  int			x,
  int			y,
  patch_t**		pl,
  int*			num,
  boolean*		on,
  patch_t*		percent )
{
    STlib_initNum(&p->n, x, y, pl, num, on, 3);
    p->p = percent;
}




void
STlib_updatePercent
( st_percent_t*		per,
  int			refresh )
{
    if (refresh && *per->n.on)
	V_DrawPatch(per->n.x, per->n.y, per->p);
    
    STlib_updateNum(&per->n, refresh);
}

namespace theta
{

namespace status
{

void Multiicon::Update(boolean refresh)
{
    if (*this->inum == -1)
    {
        return;
    }

    int x = this->x + this->p[*this->inum]->xoff;
    int y = this->y + this->p[*this->inum]->yoff;
    int w = this->p[*this->inum]->width;
    int h = this->p[*this->inum]->height;

    if (y - ST_Y < 0)
        I_Error("updateMultIcon: y - ST_Y < 0");

    video::DrawScaledGraphic(this->x, this->y, *this->p[*this->inum]);
}

void Binicon::Update(boolean refresh)
{
    int x = this->x + this->p.xoff;
    int y = this->y + this->p.yoff;
    int w = this->p.width;
    int h = this->p.height;

    if (y - ST_Y < 0)
        I_Error("updateBinIcon: y - ST_Y < 0");

    video::DrawScaledGraphic(this->x, this->y, this->p);
}

}

}
