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
// 	The status bar widget code.
//

#ifndef __STLIB__
#define __STLIB__

// We are referring to patches.
#include "r_defs.h"

namespace theta
{

namespace status
{

//
// Typedefs of widgets
//

// Number widget

class Number
{
    friend class Percent;

    // upper right-hand corner
    //  of the number (right-justified)
    int		x;
    int		y;

    // max # of digits in number
    int width;    

    // last number value
    int		oldnum;
    
    // pointer to current value
    int*	num;

    // pointer to boolean stating
    //  whether to update number
    boolean*	on;

    // list of patches for 0-9
    const video::Graphic**  p;

    // user data
    int data;
public:
    Number(int x, int y, const video::Graphic** pl, int* num, boolean* on, int width) :
        x(x), y(y), oldnum(0), width(width), num(num), on(on), p(pl) { }
    void SetData(int data);
    void SetNum(int* num);
    void Update(boolean refresh);
};



// Percent widget ("child" of number widget,
//  or, more precisely, contains a number widget.)
class Percent
{
    // number information
    Number                  n;

    // percent sign graphic
    const video::Graphic*   p;
public:
    Percent(int x, int y, const video::Graphic** pl, int* num,
        boolean* on, const video::Graphic* percent) :
        n(Number(x, y, pl, num, on, 3)), p(percent) { }
    void Update(boolean refresh);
};

// Multiple Icon widget
class Multiicon
{
     // center-justified location of icons
    int			x;
    int			y;

    // last icon number
    int			oldinum;

    // pointer to current icon
    int*		inum;

    // pointer to boolean stating
    //  whether to update icon
    boolean*		on;

    // list of icons
    const video::Graphic** p;
    
    // user data
    int			data;
public:
    Multiicon(int x, int y, const video::Graphic** il, int* inum, boolean* on) :
        x(x), y(y), oldinum(-1), inum(inum), on(on), p(il) { }
    void Update(boolean refresh);
};

// Binary Icon widget

class Binicon
{
    // center-justified location of icon
    int			x;
    int			y;

    // last icon value
    boolean		oldval;

    // pointer to current icon status
    boolean*		val;

    // pointer to boolean
    //  stating whether to update icon
    boolean*		on;  


    const video::Graphic&   p;  // icon
    int			data;   // user data
public:
    Binicon(int x, int y, const video::Graphic& i, boolean* val, boolean* on) :
        x(x), y(y), oldval(false), val(val), on(on), p(i) { }
    void Update(boolean refresh);
};

}


//
// Widget creation, access, and update routines
//

// Initializes widget library.
// More precisely, initialize STMINUS,
//  everything else is done somewhere else.
//
void STlib_init(void);

}

#endif
