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
//
// DESCRIPTION:  the automap code
//


#include <stdio.h>
#include <array>
#include <gsl/span>

#include "deh_main.h"

#include "z_zone.h"
#include "doomkeys.h"
#include "doomdef.h"
#include "st_stuff.h"
#include "p_local.h"
#include "w_wad.h"

#include "m_cheat.h"
#include "m_controls.h"
#include "m_misc.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"

// Needs access to LFB.
#include "v_video.h"

// State.
#include "doomstat.h"
#include "r_state.h"

// Data.
#include "dstrings.h"

#include "am_map.h"


namespace theta
{

// For use if I do walls with outsides/insides
#define REDS		(256-5*16)
#define REDRANGE	16
#define BLUES		(256-4*16+8)
#define BLUERANGE	8
#define GREENS		(7*16)
#define GREENRANGE	16
#define GRAYS		(6*16)
#define GRAYSRANGE	16
#define BROWNS		(4*16)
#define BROWNRANGE	16
#define YELLOWS		(256-32+7)
#define YELLOWRANGE	1
#define BLACK		0
#define WHITE		(256-47)

// Automap colors
#define BACKGROUND	BLACK
#define YOURCOLORS	WHITE
#define YOURRANGE	0
#define WALLCOLORS	REDS
#define WALLRANGE	REDRANGE
#define TSWALLCOLORS	GRAYS
#define TSWALLRANGE	GRAYSRANGE
#define FDWALLCOLORS	BROWNS
#define FDWALLRANGE	BROWNRANGE
#define CDWALLCOLORS	YELLOWS
#define CDWALLRANGE	YELLOWRANGE
#define THINGCOLORS	GREENS
#define THINGRANGE	GREENRANGE
#define SECRETWALLCOLORS WALLCOLORS
#define SECRETWALLRANGE WALLRANGE
#define GRIDCOLORS	(GRAYS + GRAYSRANGE/2)
#define GRIDRANGE	0
#define XHAIRCOLORS	GRAYS

static std::array<byte, 3 * 11> map_palette{
    0, 0, 0,       // Background
    255, 255, 255, // You
    255, 0, 0,     // Walls
    155, 0, 0,     // Teleporters
    131, 131, 131, // Cheated walls
    179, 115, 71,  // FD walls
    255, 255, 0,   // CD walls
    111, 111, 111, // Computer map walls
    119, 255, 111, // Things
    131, 131, 131, // Grid lines
    131, 131, 131, // Crosshair
};

enum PaletteIndex
{
    background,
    you,
    walls,
    teles,
    tswalls,
    fdwalls,
    cdwalls,
    pwwalls,
    things,
    gridlines,
    xhair,
};

// drawing stuff

#define AM_NUMMARKPOINTS 10

// scale on entry
static const double INITSCALEMTOF = 0.2;
// how much the automap moves window per tic in frame-buffer coordinates
// moves 140 pixels in 1 second
// [AM] Scaled to floating-point framebuffer
static const double F_PANINC = (140.0 / VIRTUALHEIGHT) / TICRATE;
// how much zoom-in per tic
// goes to 2x in 1 second
static const double M_ZOOMIN = 1.02;
// how much zoom-out per tic
// pulls out to 0.5x in 1 second
static const double M_ZOOMOUT = 0.98;

// the following is crap
#define LINE_NEVERSEE ML_DONTDRAW

typedef struct
{
    double x, y;
} fpoint_t;

typedef struct
{
    fpoint_t a, b;
} fline_t;

typedef struct
{
    fixed_t		x,y;
} mpoint_t;

typedef struct
{
    mpoint_t a, b;
} mline_t;

typedef struct
{
    fixed_t slp, islp;
} islope_t;



//
// The vector graphics for the automap.
//  A line drawing of the player pointing right,
//   starting from the middle.
//
#define R ((8*PLAYERRADIUS)/7)
mline_t player_arrow[] = {
    { { -R+R/8, 0 }, { R, 0 } }, // -----
    { { R, 0 }, { R-R/2, R/4 } },  // ----->
    { { R, 0 }, { R-R/2, -R/4 } },
    { { -R+R/8, 0 }, { -R-R/8, R/4 } }, // >---->
    { { -R+R/8, 0 }, { -R-R/8, -R/4 } },
    { { -R+3*R/8, 0 }, { -R+R/8, R/4 } }, // >>--->
    { { -R+3*R/8, 0 }, { -R+R/8, -R/4 } }
};
#undef R

#define R ((8*PLAYERRADIUS)/7)
mline_t cheat_player_arrow[] = {
    { { -R+R/8, 0 }, { R, 0 } }, // -----
    { { R, 0 }, { R-R/2, R/6 } },  // ----->
    { { R, 0 }, { R-R/2, -R/6 } },
    { { -R+R/8, 0 }, { -R-R/8, R/6 } }, // >----->
    { { -R+R/8, 0 }, { -R-R/8, -R/6 } },
    { { -R+3*R/8, 0 }, { -R+R/8, R/6 } }, // >>----->
    { { -R+3*R/8, 0 }, { -R+R/8, -R/6 } },
    { { -R/2, 0 }, { -R/2, -R/6 } }, // >>-d--->
    { { -R/2, -R/6 }, { -R/2+R/6, -R/6 } },
    { { -R/2+R/6, -R/6 }, { -R/2+R/6, R/4 } },
    { { -R/6, 0 }, { -R/6, -R/6 } }, // >>-dd-->
    { { -R/6, -R/6 }, { 0, -R/6 } },
    { { 0, -R/6 }, { 0, R/4 } },
    { { R/6, R/4 }, { R/6, -R/7 } }, // >>-ddt->
    { { R/6, -R/7 }, { R/6+R/32, -R/7-R/32 } },
    { { R/6+R/32, -R/7-R/32 }, { R/6+R/10, -R/7 } }
};
#undef R

#define R (FRACUNIT)
mline_t triangle_guy[] = {
    { { (fixed_t)(-.867*R), (fixed_t)(-.5*R) }, { (fixed_t)(.867*R ), (fixed_t)(-.5*R) } },
    { { (fixed_t)(.867*R ), (fixed_t)(-.5*R) }, { (fixed_t)(0      ), (fixed_t)(R    ) } },
    { { (fixed_t)(0      ), (fixed_t)(R    ) }, { (fixed_t)(-.867*R), (fixed_t)(-.5*R) } }
};
#undef R

#define R (FRACUNIT)
mline_t thintriangle_guy[] = {
    { { (fixed_t)(-.5*R), (fixed_t)(-.7*R) }, { (fixed_t)(R    ), (fixed_t)(0    ) } },
    { { (fixed_t)(R    ), (fixed_t)(0    ) }, { (fixed_t)(-.5*R), (fixed_t)(.7*R ) } },
    { { (fixed_t)(-.5*R), (fixed_t)(.7*R ) }, { (fixed_t)(-.5*R), (fixed_t)(-.7*R) } }
};
#undef R




static int 	cheating = 0;
static int 	grid = 0;

static int 	leveljuststarted = 1; 	// kluge until AM_LevelInit() is called

boolean    	automapactive = false;
static double   finit_width = VIRTUALWIDTH;
static double   finit_height = VIRTUALHEIGHT - ST_HEIGHT;

// location of window on screen
static double   f_x;
static double   f_y;

// size of window on screen
static double   f_w;
static double   f_h;

static int 	lightlev; 		// used for funky strobing effect
static int 	amclock;

static mpoint_t m_paninc; // how far the window pans each tic (map coords)
static double   mtof_zoommul; // how far the window zooms in each tic (map coords)
static double   ftom_zoommul; // how far the window zooms in each tic (fb coords)

static fixed_t 	m_x, m_y;   // LL x,y where the window is on the map (map coords)
static fixed_t 	m_x2, m_y2; // UR x,y where the window is on the map (map coords)

//
// width/height of window on map (map coords)
//
static fixed_t 	m_w;
static fixed_t	m_h;

// based on level size
static fixed_t 	min_x;
static fixed_t	min_y; 
static fixed_t 	max_x;
static fixed_t  max_y;

static fixed_t 	max_w; // max_x-min_x,
static fixed_t  max_h; // max_y-min_y

// based on player size
static fixed_t 	min_w;
static fixed_t  min_h;


static double min_scale_mtof; // used to tell when to stop zooming out
static double max_scale_mtof; // used to tell when to stop zooming in

// old stuff for recovery later
static fixed_t old_m_w, old_m_h;
static fixed_t old_m_x, old_m_y;

// old location used by the Follower routine
static mpoint_t f_oldloc;

// used by MTOF to scale from map-to-frame-buffer coords
static double scale_mtof = INITSCALEMTOF;
// used by FTOM to scale from frame-buffer-to-map coords (=1/scale_mtof)
static double scale_ftom;

static player_t *plr; // the player represented by an arrow

static patch_t *marknums[10]; // numbers used for marking by the automap
static mpoint_t markpoints[AM_NUMMARKPOINTS]; // where the points are
static int markpointnum = 0; // next point to be assigned

static int followplayer = 1; // specifies whether to follow the player around

cheatseq_t cheat_amap = CHEAT("iddt", 0);

static boolean stopped = true;

// translates between frame-buffer and map distances
fixed_t FTOM(double x) {
    return FloatToFixed(x * scale_ftom);
}

double MTOF(fixed_t x) {
    return FixedToFloat(x) * scale_mtof;
}

// translates between frame-buffer and map coordinates
double CXMTOF(fixed_t x) {
    return f_x + MTOF(x - m_x);
}

double CYMTOF(fixed_t y) {
    return f_y + (f_h - MTOF(y - m_y));
}

// Calculates the slope and slope according to the x-axis of a line
// segment in map coordinates (with the upright y-axis n' all) so
// that it can be used with the brain-dead drawing stuff.

void
AM_getIslope
( mline_t*	ml,
  islope_t*	is )
{
    int dx, dy;

    dy = ml->a.y - ml->b.y;
    dx = ml->b.x - ml->a.x;
    if (!dy) is->islp = (dx<0?-INT_MAX:INT_MAX);
    else is->islp = FixedDiv(dx, dy);
    if (!dx) is->slp = (dy<0?-INT_MAX:INT_MAX);
    else is->slp = FixedDiv(dy, dx);

}

//
//
//
void AM_activateNewScale(void)
{
    m_x += m_w/2;
    m_y += m_h/2;
    m_w = FTOM(f_w);
    m_h = FTOM(f_h);
    m_x -= m_w/2;
    m_y -= m_h/2;
    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;
}

//
//
//
void AM_saveScaleAndLoc(void)
{
    old_m_x = m_x;
    old_m_y = m_y;
    old_m_w = m_w;
    old_m_h = m_h;
}

//
//
//
void AM_restoreScaleAndLoc(void)
{

    m_w = old_m_w;
    m_h = old_m_h;
    if (!followplayer)
    {
	m_x = old_m_x;
	m_y = old_m_y;
    } else {
	m_x = plr->mo->x - m_w/2;
	m_y = plr->mo->y - m_h/2;
    }
    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;

    // Change the scaling multipliers
    scale_mtof = f_w / FixedToFloat(m_w);
    scale_ftom = 1 / scale_mtof;
}

//
// adds a marker at the current location
//
void AM_addMark(void)
{
    markpoints[markpointnum].x = m_x + m_w/2;
    markpoints[markpointnum].y = m_y + m_h/2;
    markpointnum = (markpointnum + 1) % AM_NUMMARKPOINTS;

}

//
// Determines bounding box of all vertices,
// sets global variables controlling zoom range.
//
void AM_findMinMaxBoundaries(void)
{
    min_x = min_y =  INT_MAX;
    max_x = max_y = -INT_MAX;

    for (int i = 0;i < numvertexes;i++)
    {
        if (vertexes[i].x < min_x)
        {
            min_x = vertexes[i].x;
        }
        else if (vertexes[i].x > max_x)
        {
            max_x = vertexes[i].x;
        }
        if (vertexes[i].y < min_y)
        {
            min_y = vertexes[i].y;
        }
        else if (vertexes[i].y > max_y)
        {
            max_y = vertexes[i].y;
        }
    }

    max_w = max_x - min_x;
    max_h = max_y - min_y;

    min_w = 2*PLAYERRADIUS; // const? never changed?
    min_h = 2*PLAYERRADIUS;

    double a = f_w / FixedToFloat(max_w);
    double b = f_h / FixedToFloat(max_h);

    min_scale_mtof = a < b ? a : b;
    max_scale_mtof = f_h / (2.0 * FixedToFloat(PLAYERRADIUS));
}


//
//
//
void AM_changeWindowLoc(void)
{
    if (m_paninc.x || m_paninc.y)
    {
	followplayer = 0;
	f_oldloc.x = INT_MAX;
    }

    m_x += m_paninc.x;
    m_y += m_paninc.y;

    if (m_x + m_w/2 > max_x)
	m_x = max_x - m_w/2;
    else if (m_x + m_w/2 < min_x)
	m_x = min_x - m_w/2;
  
    if (m_y + m_h/2 > max_y)
	m_y = max_y - m_h/2;
    else if (m_y + m_h/2 < min_y)
	m_y = min_y - m_h/2;

    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;
}


//
//
//
void AM_initVariables(void)
{
    int pnum;
    static event_t st_notify = { ev_keyup, AM_MSGENTERED, 0, 0 };

    automapactive = true;

    f_oldloc.x = INT_MAX;
    amclock = 0;
    lightlev = 0;

    m_paninc.x = m_paninc.y = 0;
    ftom_zoommul = 1.0;
    mtof_zoommul = 1.0;

    m_w = FTOM(f_w);
    m_h = FTOM(f_h);

    // find player to center on initially
    if (playeringame[consoleplayer])
    {
        plr = &players[consoleplayer];
    }
    else
    {
        plr = &players[0];

	for (pnum=0;pnum<MAXPLAYERS;pnum++)
        {
	    if (playeringame[pnum])
            {
                plr = &players[pnum];
		break;
            }
        }
    }

    m_x = plr->mo->x - m_w/2;
    m_y = plr->mo->y - m_h/2;
    AM_changeWindowLoc();

    // for saving & restoring
    old_m_x = m_x;
    old_m_y = m_y;
    old_m_w = m_w;
    old_m_h = m_h;

    // inform the status bar of the change
    ST_Responder(&st_notify);

}

//
// 
//
void AM_loadPics(void)
{
    int i;
    char namebuf[9];
  
    for (i=0;i<10;i++)
    {
	DEH_snprintf(namebuf, 9, "AMMNUM%d", i);
	marknums[i] = static_cast<patch_t*>(W_CacheLumpName(namebuf, PU_STATIC));
    }

}

void AM_unloadPics(void)
{
    int i;
    char namebuf[9];
  
    for (i=0;i<10;i++)
    {
	DEH_snprintf(namebuf, 9, "AMMNUM%d", i);
	W_ReleaseLumpName(namebuf);
    }
}

void AM_clearMarks(void)
{
    int i;

    for (i=0;i<AM_NUMMARKPOINTS;i++)
	markpoints[i].x = -1; // means empty
    markpointnum = 0;
}

// Set the x scaling of the automap to whatever the dimensions of the
// renderer happens to be, so we get a square grid.
static void SetAspectRatio()
{
    f_w = system::renderer->GetWidth() / static_cast<double>(system::renderer->GetHeight());
}

//
// should be called at the start of every level
// right now, i figure it out myself
//
void AM_LevelInit(void)
{
    leveljuststarted = 0;

    f_x = f_y = 0;
    f_w = 1.0;
    f_h = 1.0;

    SetAspectRatio();

    AM_clearMarks();

    AM_findMinMaxBoundaries();
    scale_mtof = min_scale_mtof / 0.7;
    if (scale_mtof > max_scale_mtof)
	scale_mtof = min_scale_mtof;
    scale_ftom = 1 / scale_mtof;
}




//
//
//
void AM_Stop (void)
{
    static event_t st_notify = { ev_keydown /* 0 */, ev_keyup, AM_MSGEXITED, 0 };

    AM_unloadPics();
    automapactive = false;
    ST_Responder(&st_notify);
    stopped = true;
}

//
//
//
void AM_Start (void)
{
    static int lastlevel = -1, lastepisode = -1;

    // [AM] Ensure that the map is drawn in the right spot on the screen.
    system::renderer->SetMapGeometry(0.0, 0.0, 1.0, ST_Y / (double)VIRTUALHEIGHT);
    system::renderer->SetMapPalette(gsl::make_span(map_palette));

    if (!stopped) AM_Stop();
    stopped = false;
    if (lastlevel != gamemap || lastepisode != gameepisode)
    {
	AM_LevelInit();
	lastlevel = gamemap;
	lastepisode = gameepisode;
    }
    AM_initVariables();
    AM_loadPics();
}

//
// set the window scale to the maximum size
//
void AM_minOutWindowScale(void)
{
    scale_mtof = min_scale_mtof;
    scale_ftom = 1 / scale_mtof;
    AM_activateNewScale();
}

//
// set the window scale to the minimum size
//
void AM_maxOutWindowScale(void)
{
    scale_mtof = max_scale_mtof;
    scale_ftom = 1 / scale_mtof;
    AM_activateNewScale();
}

namespace automap
{

void OnResolutionChange()
{
    SetAspectRatio();
    AM_findMinMaxBoundaries();
    if (scale_mtof > max_scale_mtof)
    {
        scale_mtof = min_scale_mtof;
    }
}

}

//
// Handle events (user inputs) in automap mode
//
boolean
AM_Responder
( event_t*	ev )
{

    int rc;
    static int bigstate=0;
    static char buffer[20];
    int key;

    rc = false;

    if (ev->type == ev_joystick && joybautomap >= 0
        && (ev->data1 & (1 << joybautomap)) != 0)
    {
        joywait = I_GetTime() + 5;

        if (!automapactive)
        {
            AM_Start ();
            viewactive = false;
        }
        else
        {
            bigstate = 0;
            viewactive = true;
            AM_Stop ();
        }

        return true;
    }

    if (!automapactive)
    {
	if (ev->type == ev_keydown && ev->data1 == key_map_toggle)
	{
	    AM_Start ();
	    viewactive = false;
	    rc = true;
	}
    }
    else if (ev->type == ev_keydown)
    {
	rc = true;
        key = ev->data1;

        if (key == key_map_east)          // pan right
        {
            if (!followplayer) m_paninc.x = FTOM(F_PANINC);
            else rc = false;
        }
        else if (key == key_map_west)     // pan left
        {
            if (!followplayer) m_paninc.x = -FTOM(F_PANINC);
            else rc = false;
        }
        else if (key == key_map_north)    // pan up
        {
            if (!followplayer) m_paninc.y = FTOM(F_PANINC);
            else rc = false;
        }
        else if (key == key_map_south)    // pan down
        {
            if (!followplayer) m_paninc.y = -FTOM(F_PANINC);
            else rc = false;
        }
        else if (key == key_map_zoomout)  // zoom out
        {
            mtof_zoommul = M_ZOOMOUT;
            ftom_zoommul = M_ZOOMIN;
        }
        else if (key == key_map_zoomin)   // zoom in
        {
            mtof_zoommul = M_ZOOMIN;
            ftom_zoommul = M_ZOOMOUT;
        }
        else if (key == key_map_toggle)
        {
            bigstate = 0;
            viewactive = true;
            AM_Stop ();
        }
        else if (key == key_map_maxzoom)
        {
            bigstate = !bigstate;
            if (bigstate)
            {
                AM_saveScaleAndLoc();
                AM_minOutWindowScale();
            }
            else AM_restoreScaleAndLoc();
        }
        else if (key == key_map_follow)
        {
            followplayer = !followplayer;
            f_oldloc.x = INT_MAX;
            if (followplayer)
                plr->message = DEH_String(AMSTR_FOLLOWON);
            else
                plr->message = DEH_String(AMSTR_FOLLOWOFF);
        }
        else if (key == key_map_grid)
        {
            grid = !grid;
            if (grid)
                plr->message = DEH_String(AMSTR_GRIDON);
            else
                plr->message = DEH_String(AMSTR_GRIDOFF);
        }
        else if (key == key_map_mark)
        {
            M_snprintf(buffer, sizeof(buffer), "%s %d",
                       DEH_String(AMSTR_MARKEDSPOT), markpointnum);
            plr->message = buffer;
            AM_addMark();
        }
        else if (key == key_map_clearmark)
        {
            AM_clearMarks();
            plr->message = DEH_String(AMSTR_MARKSCLEARED);
        }
        else
        {
            rc = false;
        }

        if ((!deathmatch || gameversion <= exe_doom_1_8)
         && cht_CheckCheat(&cheat_amap, ev->data2))
        {
            rc = false;
            cheating = (cheating + 1) % 3;
        }
    }
    else if (ev->type == ev_keyup)
    {
        rc = false;
        key = ev->data1;

        if (key == key_map_east)
        {
            if (!followplayer) m_paninc.x = 0;
        }
        else if (key == key_map_west)
        {
            if (!followplayer) m_paninc.x = 0;
        }
        else if (key == key_map_north)
        {
            if (!followplayer) m_paninc.y = 0;
        }
        else if (key == key_map_south)
        {
            if (!followplayer) m_paninc.y = 0;
        }
        else if (key == key_map_zoomout || key == key_map_zoomin)
        {
            mtof_zoommul = 1.0;
            ftom_zoommul = 1.0;
        }
    }

    return rc;

}


//
// Zooming
//
void AM_changeWindowScale(void)
{

    // Change the scaling multipliers
    scale_mtof = scale_mtof * mtof_zoommul;
    scale_ftom = 1 / scale_mtof;

    if (scale_mtof < min_scale_mtof)
	AM_minOutWindowScale();
    else if (scale_mtof > max_scale_mtof)
	AM_maxOutWindowScale();
    else
	AM_activateNewScale();
}


//
//
//
void AM_doFollowPlayer(void)
{

    if (f_oldloc.x != plr->mo->x || f_oldloc.y != plr->mo->y)
    {
	m_x = FTOM(MTOF(plr->mo->x)) - m_w/2;
	m_y = FTOM(MTOF(plr->mo->y)) - m_h/2;
	m_x2 = m_x + m_w;
	m_y2 = m_y + m_h;
	f_oldloc.x = plr->mo->x;
	f_oldloc.y = plr->mo->y;

	//  m_x = FTOM(MTOF(plr->mo->x - m_w/2));
	//  m_y = FTOM(MTOF(plr->mo->y - m_h/2));
	//  m_x = plr->mo->x - m_w/2;
	//  m_y = plr->mo->y - m_h/2;

    }

}

//
//
//
void AM_updateLightLev(void)
{
    static int nexttic = 0;
    //static int litelevels[] = { 0, 3, 5, 6, 6, 7, 7, 7 };
    static int litelevels[] = { 0, 4, 7, 10, 12, 14, 15, 15 };
    static int litelevelscnt = 0;
   
    // Change light level
    if (amclock>nexttic)
    {
	lightlev = litelevels[litelevelscnt++];
	if (litelevelscnt == arrlen(litelevels)) litelevelscnt = 0;
	nexttic = amclock + 6 - (amclock % 6);
    }

}


//
// Updates on Game Tick
//
void AM_Ticker (void)
{

    if (!automapactive)
	return;

    amclock++;

    if (followplayer)
	AM_doFollowPlayer();

    // Change the zoom if necessary
    if (ftom_zoommul != FRACUNIT)
	AM_changeWindowScale();

    // Change x,y location
    if (m_paninc.x || m_paninc.y)
	AM_changeWindowLoc();

    // Update light level
    // AM_updateLightLev();

}

// Outcode type
typedef uint8_t outcodes_t;

// Valid Outcodes
static const outcodes_t OUT_LEFT = 1;
static const outcodes_t OUT_RIGHT = 1 << 1;
static const outcodes_t OUT_BOTTOM = 1 << 2;
static const outcodes_t OUT_TOP = 1 << 3;

// Calculate Outcodes
outcodes_t DoOutcode(double mx, double my)
{
    outcodes_t oc = 0;

    if (my < 0)
    {
        oc |= OUT_TOP;
    }
    else if (my > f_h)
    {
        oc |= OUT_BOTTOM;
    }
    if (mx < 0)
    {
        oc |= OUT_LEFT;
    }
    else if (mx > f_w)
    {
        oc |= OUT_RIGHT;
    }

    return oc;
}

//
// Automap clipping of lines.
//
// Based on Cohen-Sutherland clipping algorithm but with a slightly
// faster reject and precalculated slopes.  If the speed is needed,
// use a hash algorithm to handle  the common cases.
//
boolean AM_clipMline (mline_t* ml, fline_t* fl)
{
    outcodes_t outcode1 = 0;
    outcodes_t outcode2 = 0;
    outcodes_t outside;
    
    fpoint_t	tmp;
    double      dx;
    double      dy;

    // do trivial rejects and outcodes
    if (ml->a.y > m_y2)
	outcode1 = OUT_TOP;
    else if (ml->a.y < m_y)
	outcode1 = OUT_BOTTOM;

    if (ml->b.y > m_y2)
	outcode2 = OUT_TOP;
    else if (ml->b.y < m_y)
	outcode2 = OUT_BOTTOM;
    
    if (outcode1 & outcode2)
	return false; // trivially outside

    if (ml->a.x < m_x)
	outcode1 |= OUT_LEFT;
    else if (ml->a.x > m_x2)
	outcode1 |= OUT_RIGHT;
    
    if (ml->b.x < m_x)
	outcode2 |= OUT_LEFT;
    else if (ml->b.x > m_x2)
	outcode2 |= OUT_RIGHT;
    
    if (outcode1 & outcode2)
	return false; // trivially outside

    // transform to frame-buffer coordinates.
    fl->a.x = CXMTOF(ml->a.x);
    fl->a.y = CYMTOF(ml->a.y);
    fl->b.x = CXMTOF(ml->b.x);
    fl->b.y = CYMTOF(ml->b.y);

    outcode1 = DoOutcode(fl->a.x, fl->a.y);
    outcode2 = DoOutcode(fl->b.x, fl->b.y);

    if (outcode1 & outcode2)
	return false;

    while (outcode1 | outcode2)
    {
	// may be partially inside box
	// find an outside point
	if (outcode1)
	    outside = outcode1;
	else
	    outside = outcode2;
	
	// clip to each side
	if (outside & OUT_TOP)
	{
	    dy = fl->a.y - fl->b.y;
	    dx = fl->b.x - fl->a.x;
	    tmp.x = fl->a.x + (dx * (fl->a.y)) / dy;
	    tmp.y = 0;
	}
	else if (outside & OUT_BOTTOM)
	{
	    dy = fl->a.y - fl->b.y;
	    dx = fl->b.x - fl->a.x;
	    tmp.x = fl->a.x + (dx * (fl->a.y - f_h)) / dy;
	    tmp.y = f_h;
	}
	else if (outside & OUT_RIGHT)
	{
	    dy = fl->b.y - fl->a.y;
	    dx = fl->b.x - fl->a.x;
	    tmp.y = fl->a.y + (dy * (f_w - fl->a.x)) / dx;
	    tmp.x = f_w;
	}
	else if (outside & OUT_LEFT)
	{
	    dy = fl->b.y - fl->a.y;
	    dx = fl->b.x - fl->a.x;
	    tmp.y = fl->a.y + (dy * (-fl->a.x)) / dx;
	    tmp.x = 0;
	}
        else
        {
            tmp.x = 0;
            tmp.y = 0;
        }

	if (outside == outcode1)
	{
	    fl->a = tmp;
            outcode1 = DoOutcode(fl->a.x, fl->a.y);
	}
	else
	{
	    fl->b = tmp;
            outcode2 = DoOutcode(fl->b.x, fl->b.y);
	}
	
	if (outcode1 & outcode2)
	    return false; // trivially outside
    }

    return true;
}


//
// Actually do the drawing of the line.
//
// This function used to actually draw a line on a screen, pixel by pixel.
// Now we let the renderer take care of things.
//
void AM_drawFline(fline_t* fl, int color)
{
    // Scale x coordinates into 0.0-1.0 range expected by the renderer.
    double x1 = fl->a.x / f_w;
    double x2 = fl->b.x / f_w;

    system::renderer->DrawMapLine(x1, fl->a.y, x2, fl->b.y, color);
}


//
// Clip lines, draw visible part sof lines.
//
void
AM_drawMline
( mline_t*	ml,
  int		color )
{
    static fline_t fl;

    if (AM_clipMline(ml, &fl))
	AM_drawFline(&fl, color); // draws it on frame buffer using fb coords
}



//
// Draws flat (floor/ceiling tile) aligned grid lines.
//
void AM_drawGrid(int color)
{
    fixed_t x, y;
    fixed_t start, end;
    mline_t ml;

    // Figure out start of vertical gridlines
    start = m_x;
    if ((start - bmaporgx) % (MAPBLOCKUNITS << FRACBITS))
    {
        start -= ((start - bmaporgx) % (MAPBLOCKUNITS << FRACBITS));
    }
    end = m_x + m_w;

    // draw vertical gridlines
    ml.a.y = m_y;
    ml.b.y = m_y + m_h;
    for (x = start;x < end;x += (MAPBLOCKUNITS << FRACBITS))
    {
        ml.a.x = x;
        ml.b.x = x;
        AM_drawMline(&ml, color);
    }

    // Figure out start of horizontal gridlines
    start = m_y;
    if ((start - bmaporgy) % (MAPBLOCKUNITS << FRACBITS))
    {
        start -= ((start - bmaporgy) % (MAPBLOCKUNITS << FRACBITS));
    }
    end = m_y + m_h;

    // draw horizontal gridlines
    ml.a.x = m_x;
    ml.b.x = m_x + m_w;
    for (y = start;y < end;y += (MAPBLOCKUNITS << FRACBITS))
    {
        ml.a.y = y;
        ml.b.y = y;
        AM_drawMline(&ml, color);
    }
}

//
// Determines visible lines, draws them.
// This is LineDef based, not LineSeg based.
//
void AM_drawWalls(void)
{
    int i;
    static mline_t l;

    for (i=0;i<numlines;i++)
    {
	l.a.x = lines[i].v1->x;
	l.a.y = lines[i].v1->y;
	l.b.x = lines[i].v2->x;
	l.b.y = lines[i].v2->y;
	if (cheating || (lines[i].flags & ML_MAPPED))
	{
	    if ((lines[i].flags & LINE_NEVERSEE) && !cheating)
		continue;
	    if (!lines[i].backsector)
	    {
		AM_drawMline(&l, PaletteIndex::walls);
	    }
	    else
	    {
                // [AM] Vanilla only showed W1 teles, we're showing both kinds
		if (lines[i].special == 39 || lines[i].special == 97)
		{ // teleporters
		    AM_drawMline(&l, PaletteIndex::teles);
		}
		else if (lines[i].flags & ML_SECRET) // secret door
		{
                    if (cheating)
                    {
                        // [AM] Maybe one day?  Kinda useless though...
                        AM_drawMline(&l, PaletteIndex::walls);
                    }
                    else
                    {
                        AM_drawMline(&l, PaletteIndex::walls);
                    }
		}
		else if (lines[i].backsector->floorheight
			   != lines[i].frontsector->floorheight) {
		    AM_drawMline(&l, PaletteIndex::fdwalls); // floor level change
		}
		else if (lines[i].backsector->ceilingheight
			   != lines[i].frontsector->ceilingheight) {
		    AM_drawMline(&l, PaletteIndex::cdwalls); // ceiling level change
		}
		else if (cheating) {
		    AM_drawMline(&l, PaletteIndex::tswalls);
		}
	    }
	}
	else if (plr->powers[pw_allmap])
	{
            if (!(lines[i].flags & LINE_NEVERSEE))
            {
                AM_drawMline(&l, PaletteIndex::pwwalls);
            }
	}
    }
}


//
// Rotation in 2D.
// Used to rotate player arrow line character.
//
void
AM_rotate
( fixed_t*	x,
  fixed_t*	y,
  angle_t	a )
{
    fixed_t tmpx;

    tmpx =
	FixedMul(*x,finecosine[a>>ANGLETOFINESHIFT])
	- FixedMul(*y,finesine[a>>ANGLETOFINESHIFT]);
    
    *y   =
	FixedMul(*x,finesine[a>>ANGLETOFINESHIFT])
	+ FixedMul(*y,finecosine[a>>ANGLETOFINESHIFT]);

    *x = tmpx;
}

void
AM_drawLineCharacter
( mline_t*	lineguy,
  int		lineguylines,
  fixed_t	scale,
  angle_t	angle,
  int		color,
  fixed_t	x,
  fixed_t	y )
{
    int		i;
    mline_t	l;

    for (i=0;i<lineguylines;i++)
    {
	l.a.x = lineguy[i].a.x;
	l.a.y = lineguy[i].a.y;

	if (scale)
	{
	    l.a.x = FixedMul(scale, l.a.x);
	    l.a.y = FixedMul(scale, l.a.y);
	}

	if (angle)
	    AM_rotate(&l.a.x, &l.a.y, angle);

	l.a.x += x;
	l.a.y += y;

	l.b.x = lineguy[i].b.x;
	l.b.y = lineguy[i].b.y;

	if (scale)
	{
	    l.b.x = FixedMul(scale, l.b.x);
	    l.b.y = FixedMul(scale, l.b.y);
	}

	if (angle)
	    AM_rotate(&l.b.x, &l.b.y, angle);
	
	l.b.x += x;
	l.b.y += y;

	AM_drawMline(&l, color);
    }
}

void AM_drawPlayers(void)
{
    int		i;
    player_t*	p;
    static int 	their_colors[] = { GREENS, GRAYS, BROWNS, REDS };
    int		their_color = -1;
    int		color;

    if (!netgame)
    {
        if (cheating)
        {
            AM_drawLineCharacter(cheat_player_arrow, arrlen(cheat_player_arrow), 0,
                plr->mo->angle, PaletteIndex::you, plr->mo->x, plr->mo->y);
        }
        else
        {
            AM_drawLineCharacter(player_arrow, arrlen(player_arrow), 0,
                plr->mo->angle, PaletteIndex::you, plr->mo->x, plr->mo->y);
        }
        return;
    }

    // [AM] FIXME: Player colors.  Until I get working netcode, I don't
    //      really care about any of this.

    for (i=0;i<MAXPLAYERS;i++)
    {
	their_color++;
	p = &players[i];

	if ( (deathmatch && !singledemo) && p != plr)
	    continue;

	if (!playeringame[i])
	    continue;

	if (p->powers[pw_invisibility])
	    color = 246; // *close* to black
	else
	    color = their_colors[their_color];
	
	AM_drawLineCharacter
	    (player_arrow, arrlen(player_arrow), 0, p->mo->angle,
	     color, p->mo->x, p->mo->y);
    }

}

void AM_drawThings(int colors)
{
    int		i;
    mobj_t*	t;

    for (i=0;i<numsectors;i++)
    {
	t = sectors[i].thinglist;
	while (t)
	{
	    AM_drawLineCharacter
		(thintriangle_guy, arrlen(thintriangle_guy),
		 16<<FRACBITS, t->angle, colors+lightlev, t->x, t->y);
	    t = t->snext;
	}
    }
}

void AM_drawMarks(void)
{
    int i, fx, fy, w, h;

    for (i=0;i<AM_NUMMARKPOINTS;i++)
    {
	if (markpoints[i].x != -1)
	{
	    //      w = SHORT(marknums[i]->width);
	    //      h = SHORT(marknums[i]->height);
	    w = 5; // because something's wrong with the wad, i guess
	    h = 6; // because something's wrong with the wad, i guess
	    fx = CXMTOF(markpoints[i].x);
	    fy = CYMTOF(markpoints[i].y);
	    if (fx >= f_x && fx <= f_w - w && fy >= f_y && fy <= f_h - h)
		V_DrawPatch(fx, fy, marknums[i]);
	}
    }

}

void AM_drawCrosshair(int color)
{
    system::renderer->DrawMapLine(0.49, 0.5, 0.51, 0.5, PaletteIndex::xhair);
    system::renderer->DrawMapLine(0.5, 0.49, 0.5, 0.51, PaletteIndex::xhair);
}

void AM_Drawer (void)
{
    if (!automapactive) return;

    if (grid)
	AM_drawGrid(PaletteIndex::gridlines);
    AM_drawWalls();
    AM_drawPlayers();
    if (cheating==2)
	AM_drawThings(PaletteIndex::things);
    AM_drawCrosshair(PaletteIndex::xhair);

    AM_drawMarks();

    V_MarkRect(f_x, f_y, f_w, f_h);

}

}
