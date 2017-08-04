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
const theta::video::Graphic*    sttminus;

void STlib_init(void)
{
    sttminus = &theta::video::GraphicsManager::Instance().LoadPatch(DEH_String("STTMINUS"));
}

namespace theta
{

namespace status
{

int Number::GetX() const
{
    return this->x;
}

int Number::GetY() const
{
    return this->y;
}

void Number::SetData(int data)
{
    this->data = data;
}

void Number::SetNum(int* num)
{
    this->num = num;
}

// 
// A fairly efficient way to draw a number
//  based on differences from the old number.
// Note: worth the trouble?  [AM] No...
//
void Number::Update(boolean refresh)
{
    int numdigits = this->width;
    int num = *this->num;

    int w = this->p[0]->width;
    int h = this->p[0]->height;
    int x = this->x;

    int neg;

    this->oldnum = *this->num;

    neg = num < 0;

    if (neg)
    {
        if (numdigits == 2 && num < -9)
        {
            num = -9;
        }
        else if (numdigits == 3 && num < -99)
        {
            num = -99;
        }

        num = -num;
    }

    // clear the area
    x = this->x - numdigits * w;

    if (this->y - ST_Y < 0)
    {
        I_Error("drawNum: n->y - ST_Y < 0");
    }

    // if non-number, do not draw it
    if (num == 1994)
    {
        return;
    }

    x = this->x;

    // in the special case of 0, you draw 0
    if (!num)
    {
        video::DrawScaledGraphic(x - w, this->y, *this->p[0]);
    }

    // draw the new number
    while (num && numdigits--)
    {
        x -= w;
        video::DrawScaledGraphic(x, this->y, *this->p[num % 10]);
        num /= 10;
    }

    // draw a minus sign if necessary
    if (neg)
    {
        video::DrawScaledGraphic(x - 8, this->y, *sttminus);
    }
}

void Percent::Update(boolean refresh)
{
    video::DrawScaledGraphic(this->n.GetX(), this->n.GetY(), *this->p);

    this->n.Update(refresh);
}

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
    {
        I_Error("updateMultIcon: y - ST_Y < 0");
    }

    video::DrawScaledGraphic(this->x, this->y, *this->p[*this->inum]);
}

void Binicon::Update(boolean refresh)
{
    int x = this->x + this->p.xoff;
    int y = this->y + this->p.yoff;
    int w = this->p.width;
    int h = this->p.height;

    if (y - ST_Y < 0)
    {
        I_Error("updateBinIcon: y - ST_Y < 0");
    }

    video::DrawScaledGraphic(this->x, this->y, this->p);
}

}

}
