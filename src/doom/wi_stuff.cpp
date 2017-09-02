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
//	Intermission screens.
//


#include <array>
#include <functional> // reference_wrapper
#include <vector>

#include <stdio.h>

#include "array_view.h"
#include "z_zone.h"

#include "m_misc.h"
#include "m_random.h"

#include "deh_main.h"
#include "i_swap.h"
#include "i_system.h"

#include "w_wad.h"

#include "g_game.h"

#include "r_local.h"
#include "s_sound.h"

#include "doomstat.h"

// Data.
#include "sounds.h"

// Needs access to LFB.
#include "v_video.h"

#include "wi_stuff.h"

namespace theta
{

//
// Data needed to add patches to full screen intermission pics.
// Patches are statistics messages, and animations.
// Loads of by-pixel layout and placement, offsets etc.
//


//
// Different vetween registered DOOM (1994) and
//  Ultimate DOOM - Final edition (retail, 1995?).
// This is supposedly ignored for commercial
//  release (aka DOOM II), which had 34 maps
//  in one episode. So there.
#define NUMEPISODES	4
#define NUMMAPS		9


// in tics
//U #define PAUSELEN		(TICRATE*2) 
//U #define SCORESTEP		100
//U #define ANIMPERIOD		32
// pixel distance from "(YOU)" to "PLAYER N"
//U #define STARDIST		10 
//U #define WK 1


// GLOBAL LOCATIONS
#define WI_TITLEY		2
#define WI_SPACINGY    		33

// SINGPLE-PLAYER STUFF
#define SP_STATSX		50
#define SP_STATSY		50

#define SP_TIMEX		16
#define SP_TIMEY		(VIRTUALHEIGHT - 32)


// NET GAME STUFF
#define NG_STATSY		50
#define NG_STATSX		(32 + SHORT(star->width)/2 + 32*!dofrags)

#define NG_SPACINGX    		64


// DEATHMATCH STUFF
#define DM_MATRIXX		42
#define DM_MATRIXY		68

#define DM_SPACINGX		40

#define DM_TOTALSX		269

#define DM_KILLERSX		10
#define DM_KILLERSY		100
#define DM_VICTIMSX    		5
#define DM_VICTIMSY		50




struct Point
{
    int		x;
    int		y;
    Point(int x, int y) : x(x), y(y) { }
};

//
// Animation.
//
struct Animation
{
    enum class Type { always, random, level };
    Type type;

    // period in tics between animations
    int		period;

    // number of animation frames
    int		nanims;

    // location of animation
    Point	loc;

    // ALWAYS: n/a,
    // RANDOM: period deviation (<256),
    // LEVEL: level
    int		data1;

    // ALWAYS: n/a,
    // RANDOM: random base period,
    // LEVEL: n/a
    int		data2; 

    // actual graphics for frames of animations
    std::vector<std::reference_wrapper<const video::Graphic>> p;

    // following must be initialized to zero before use!

    // next value of bcnt (used in conjunction with period)
    int		nexttic;

    // last drawn animation frame
    int		lastdrawn;

    // next frame number to animate
    int		ctr;
    
    // used by RANDOM and LEVEL when animating
    int		state;  

    Animation(Type type, int period, int nanims, int x, int y, int nexttic) :
        type(type), period(period), nanims(nanims), loc(Point(x, y)),
        nexttic(nexttic) { }
};


static std::array<std::array<Point, NUMMAPS>, NUMEPISODES - 1> lnodes
{
    // Episode 0 World Map
    std::array<Point, NUMMAPS>
    {
        Point(185, 164),    // location of level 0 (CJ)
        Point(148, 143),    // location of level 1 (CJ)
        Point(69, 122),     // location of level 2 (CJ)
        Point(209, 102),    // location of level 3 (CJ)
        Point(116, 89),     // location of level 4 (CJ)
        Point(166, 55),     // location of level 5 (CJ)
        Point(71, 56),      // location of level 6 (CJ)
        Point(135, 29),     // location of level 7 (CJ)
        Point(71, 24),      // location of level 8 (CJ)
    },

    // Episode 1 World Map should go here
    std::array<Point, NUMMAPS>
    {
        Point(254, 25),     // location of level 0 (CJ)
        Point(97, 50),      // location of level 1 (CJ)
        Point(188, 64),     // location of level 2 (CJ)
        Point(128, 78),     // location of level 3 (CJ)
        Point(214, 92),     // location of level 4 (CJ)
        Point(133, 130),    // location of level 5 (CJ)
        Point(208, 136),    // location of level 6 (CJ)
        Point(148, 140),    // location of level 7 (CJ)
        Point(235, 158),    // location of level 8 (CJ)
    },

    // Episode 2 World Map should go here
    std::array<Point, NUMMAPS>
    {
        Point(156, 168),    // location of level 0 (CJ)
        Point(48, 154),     // location of level 1 (CJ)
        Point(174, 95),     // location of level 2 (CJ)
        Point(265, 75),     // location of level 3 (CJ)
        Point(130, 48),     // location of level 4 (CJ)
        Point(279, 23),     // location of level 5 (CJ)
        Point(198, 48),     // location of level 6 (CJ)
        Point(140, 25),     // location of level 7 (CJ)
        Point(281, 136),    // location of level 8 (CJ)
    }
};


//
// Animation locations for episode 0 (1).
// Using patches saves a lot of space,
//  as they replace 320x200 full screen frames.
//

static std::array<Animation, 10> epsd0animinfo
{
    Animation(Animation::Type::always, TICRATE / 3, 3, 224, 104, 0),
    Animation(Animation::Type::always, TICRATE / 3, 3, 184, 160, 0),
    Animation(Animation::Type::always, TICRATE / 3, 3, 112, 136, 0),
    Animation(Animation::Type::always, TICRATE / 3, 3, 72, 112, 0),
    Animation(Animation::Type::always, TICRATE / 3, 3, 88, 96, 0),
    Animation(Animation::Type::always, TICRATE / 3, 3, 64, 48, 0),
    Animation(Animation::Type::always, TICRATE / 3, 3, 192, 40, 0),
    Animation(Animation::Type::always, TICRATE / 3, 3, 136, 16, 0),
    Animation(Animation::Type::always, TICRATE / 3, 3, 80, 16, 0),
    Animation(Animation::Type::always, TICRATE / 3, 3, 64, 24, 0)
};

static std::array<Animation, 9> epsd1animinfo
{
    Animation(Animation::Type::level, TICRATE / 3, 1, 128, 136, 1),
    Animation(Animation::Type::level, TICRATE / 3, 1, 128, 136, 2),
    Animation(Animation::Type::level, TICRATE / 3, 1, 128, 136, 3),
    Animation(Animation::Type::level, TICRATE / 3, 1, 128, 136, 4),
    Animation(Animation::Type::level, TICRATE / 3, 1, 128, 136, 5),
    Animation(Animation::Type::level, TICRATE / 3, 1, 128, 136, 6),
    Animation(Animation::Type::level, TICRATE / 3, 1, 128, 136, 7),
    Animation(Animation::Type::level, TICRATE / 3, 3, 192, 144, 8),
    Animation(Animation::Type::level, TICRATE / 3, 1, 128, 136, 8),
};

static std::array<Animation, 6> epsd2animinfo
{
    Animation(Animation::Type::level, TICRATE / 3, 3, 104, 168, 0),
    Animation(Animation::Type::level, TICRATE / 3, 3, 40, 136, 0),
    Animation(Animation::Type::level, TICRATE / 3, 3, 160, 96, 0),
    Animation(Animation::Type::level, TICRATE / 3, 3, 104, 80, 0),
    Animation(Animation::Type::level, TICRATE / 3, 3, 120, 32, 0),
    Animation(Animation::Type::level, TICRATE / 4, 3, 40, 0, 0),
};

static std::array<std::size_t, NUMEPISODES> NUMANIMS
{
    epsd0animinfo.size(),
    epsd1animinfo.size(),
    epsd2animinfo.size(),
};

static std::array<ArrayView<Animation>, NUMEPISODES - 1> anims
{
    ArrayView<Animation>(epsd0animinfo),
    ArrayView<Animation>(epsd1animinfo),
    ArrayView<Animation>(epsd2animinfo),
};


//
// GENERAL DATA
//

//
// Locally used stuff.
//

// States for single-player
#define SP_KILLS		0
#define SP_ITEMS		2
#define SP_SECRET		4
#define SP_FRAGS		6 
#define SP_TIME			8 
#define SP_PAR			ST_TIME

#define SP_PAUSE		1

// in seconds
#define SHOWNEXTLOCDELAY	4
//#define SHOWLASTLOCDELAY	SHOWNEXTLOCDELAY


// used to accelerate or skip a stage
static int		acceleratestage;

// wbs->pnum
static int		me;

 // specifies current state
static stateenum_t	state;

// contains information passed into intermission
static wbstartstruct_t*	wbs;

static wbplayerstruct_t* plrs;  // wbs->plyr[]

// used for general timing
static int 		cnt;  

// used for timing of background animation
static int 		bcnt;

// signals to refresh everything for one frame
static int 		firstrefresh; 

static int		cnt_kills[MAXPLAYERS];
static int		cnt_items[MAXPLAYERS];
static int		cnt_secret[MAXPLAYERS];
static int		cnt_time;
static int		cnt_par;
static int		cnt_pause;

// # of commercial levels
static int		NUMCMAPS; 


//
//	GRAPHICS
//

// You Are Here graphic
static patch_t*		yah[3] = { NULL, NULL, NULL }; 

// splat
static patch_t*		splat[2] = { NULL, NULL };

// %, : graphics
static const video::Graphic* percent;
static const video::Graphic* colon;

// 0-9 graphic
static std::vector<std::reference_wrapper<const video::Graphic>> num;

// minus sign
static const video::Graphic* wiminus;

// "Finished!" graphics
static const video::Graphic* finished;

// "Entering" graphic
static const video::Graphic* entering;

// "secret"
static const video::Graphic* sp_secret;

 // "Kills", "Scrt", "Items", "Frags"
static const video::Graphic* kills;
static const video::Graphic* secret;
static const video::Graphic* items;
static const video::Graphic* frags;

// Time sucks.
static const video::Graphic* timepatch;
static const video::Graphic* par;
static const video::Graphic* sucks;

// "killers", "victims"
static const video::Graphic* killers;
static const video::Graphic* victims;

// "Total", your face, your dead face
static const video::Graphic* total;
static const video::Graphic* star;
static const video::Graphic* bstar;

// "red P[1..MAXPLAYERS]"
static std::vector<std::reference_wrapper<const video::Graphic>> p;

// "gray P[1..MAXPLAYERS]"
static std::vector<std::reference_wrapper<const video::Graphic>> bp;

 // Name graphics of each level (centered)
static std::vector<std::reference_wrapper<const video::Graphic>> lnames;

// Buffer storing the backdrop
static const video::Graphic* background;

//
// CODE
//

// slam background
void WI_slamBackground(void)
{
    video::DrawPageGraphic(*background);
}

// The ticker is used to detect keys
//  because of timing issues in netgames.
boolean WI_Responder(event_t* ev)
{
    return false;
}


// Draws "<Levelname> Finished!"
void WI_drawLF(void)
{
    int y = WI_TITLEY;

    if (gamemode != commercial || wbs->last < NUMCMAPS)
    {
        // draw <LevelName> 
        video::DrawScaledGraphic(
            (VIRTUALWIDTH - lnames[wbs->last].get().width) / 2, y,
            lnames[wbs->last].get());

        // draw "Finished!"
        y += (5 * lnames[wbs->last].get().height) / 4;

        video::DrawScaledGraphic(
            (VIRTUALWIDTH - finished->width) / 2, y, *finished);
    }
    else if (wbs->last >= NUMCMAPS)
    {
        I_Error("Invalid map graphic");
    }
}



// Draws "Entering <LevelName>"
void WI_drawEL(void)
{
    int y = WI_TITLEY;

    // draw "Entering"
    video::DrawScaledGraphic((VIRTUALWIDTH - entering->width) / 2, y, *entering);

    // draw level
    y += (5 * lnames[wbs->next].get().height) / 4;

    video::DrawScaledGraphic(
        (VIRTUALWIDTH - lnames[wbs->next].get().width) / 2, y, lnames[wbs->next]);
}

void
WI_drawOnLnode
( int		n,
  patch_t*	c[] )
{

    int		i;
    int		left;
    int		top;
    int		right;
    int		bottom;
    boolean	fits = false;

    i = 0;
    do
    {
	left = lnodes[wbs->epsd][n].x - SHORT(c[i]->leftoffset);
	top = lnodes[wbs->epsd][n].y - SHORT(c[i]->topoffset);
	right = left + SHORT(c[i]->width);
	bottom = top + SHORT(c[i]->height);

	if (left >= 0
	    && right < SCREENWIDTH
	    && top >= 0
	    && bottom < SCREENHEIGHT)
	{
	    fits = true;
	}
	else
	{
	    i++;
	}
    } while (!fits && i!=2 && c[i] != NULL);

    if (fits && i<2)
    {
	V_DrawPatch(lnodes[wbs->epsd][n].x,
                    lnodes[wbs->epsd][n].y,
		    c[i]);
    }
    else
    {
	// DEBUG
	printf("Could not place patch on level %d", n+1); 
    }
}



void WI_initAnimatedBack(void)
{
    int		i;
    Animation*	a;

    if (gamemode == commercial)
	return;

    if (wbs->epsd > 2)
	return;

    for (i=0;i<NUMANIMS[wbs->epsd];i++)
    {
	a = &anims[wbs->epsd][i];

	// init variables
	a->ctr = -1;

	// specify the next time to draw it
	if (a->type == Animation::Type::always)
	    a->nexttic = bcnt + 1 + (M_Random()%a->period);
	else if (a->type == Animation::Type::random)
	    a->nexttic = bcnt + 1 + a->data2+(M_Random()%a->data1);
	else if (a->type == Animation::Type::level)
	    a->nexttic = bcnt + 1;
    }

}

void WI_updateAnimatedBack(void)
{
    int		i;
    Animation*	a;

    if (gamemode == commercial)
	return;

    if (wbs->epsd > 2)
	return;

    for (i=0;i<NUMANIMS[wbs->epsd];i++)
    {
	a = &anims[wbs->epsd][i];

	if (bcnt == a->nexttic)
	{
	    switch (a->type)
	    {
            case Animation::Type::always:
		if (++a->ctr >= a->nanims) a->ctr = 0;
		a->nexttic = bcnt + a->period;
		break;

	    case Animation::Type::random:
		a->ctr++;
		if (a->ctr == a->nanims)
		{
		    a->ctr = -1;
		    a->nexttic = bcnt+a->data2+(M_Random()%a->data1);
		}
		else a->nexttic = bcnt + a->period;
		break;
		
            case Animation::Type::level:
                // gawd-awful hack for level anims
		if (!(state == StatCount && i == 7)
		    && wbs->next == a->data1)
		{
		    a->ctr++;
		    if (a->ctr == a->nanims) a->ctr--;
		    a->nexttic = bcnt + a->period;
		}
		break;
	    }
	}

    }

}

void WI_drawAnimatedBack(void)
{
    if (gamemode == commercial)
    {
        return;
    }

    if (wbs->epsd > 2)
    {
        return;
    }

    for (int i = 0;i < NUMANIMS[wbs->epsd];i++)
    {
        Animation* a = &anims[wbs->epsd][i];

        if (a->ctr >= 0)
        {
            video::DrawScaledGraphic(a->loc.x, a->loc.y, a->p[a->ctr]);
        }
    }
}

//
// Draws a number.
// If digits > 0, then use that many digits minimum,
//  otherwise only use as many as necessary.
// Returns new x position.
//

int WI_drawNum(int x, int y, int n, int digits)
{
    int fontwidth = num[0].get().width;

    if (digits < 0)
    {
        if (!n)
        {
            // make variable-length zeros 1 digit long
            digits = 1;
        }
        else
        {
            // figure out # of digits in #
            digits = 0;
            int temp = n;

            while (temp)
            {
                temp /= 10;
                digits++;
            }
        }
    }

    int neg = n < 0;
    if (neg)
    {
        n = -n;
    }

    // if non-number, do not draw it
    if (n == 1994)
    {
        return 0;
    }

    // draw the new number
    while (digits--)
    {
        x -= fontwidth;
        video::DrawScaledGraphic(x, y, num[ n % 10 ]);
        n /= 10;
    }

    // draw a minus sign if necessary
    if (neg)
    {
        video::DrawScaledGraphic(x -= 8, y, *wiminus);
    }

    return x;
}

void WI_drawPercent(int x, int y, int p)
{
    if (p < 0)
    {
        return;
    }

    video::DrawScaledGraphic(x, y, *percent);
    WI_drawNum(x, y, p, -1);
}



//
// Display level completion time and par,
//  or "sucks" message if overflow.
//
void WI_drawTime(int x, int y, int t)
{
    if (t < 0)
    {
        return;
    }

    if (t <= 61 * 59)
    {
        int div = 1;

        do
        {
            int n = (t / div) % 60;
            x = WI_drawNum(x, y, n, 2) - colon->width;
            div *= 60;

            // draw
            if (div == 60 || t / div)
            {
                video::DrawScaledGraphic(x, y, *colon);
            }
        } while (t / div);
    }
    else
    {
        // "sucks"
        video::DrawScaledGraphic(x - sucks->width, y, *sucks);
    }
}

void WI_unloadData(void);

void WI_End(void)
{
    WI_unloadData();
}

void WI_initNoState(void)
{
    state = NoState;
    acceleratestage = 0;
    cnt = 10;
}

void WI_updateNoState(void) {

    WI_updateAnimatedBack();

    if (!--cnt)
    {
        // Don't call WI_End yet.  G_WorldDone doesnt immediately 
        // change gamestate, so WI_Drawer is still going to get
        // run until that happens.  If we do that after WI_End
        // (which unloads all the graphics), we're in trouble.
	//WI_End();
	G_WorldDone();
    }

}

static boolean		snl_pointeron = false;


void WI_initShowNextLoc(void)
{
    state = ShowNextLoc;
    acceleratestage = 0;
    cnt = SHOWNEXTLOCDELAY * TICRATE;

    WI_initAnimatedBack();
}

void WI_updateShowNextLoc(void)
{
    WI_updateAnimatedBack();

    if (!--cnt || acceleratestage)
	WI_initNoState();
    else
	snl_pointeron = (cnt & 31) < 20;
}

void WI_drawShowNextLoc(void)
{

    int		i;
    int		last;

    WI_slamBackground();

    // draw animated background
    WI_drawAnimatedBack(); 

    if ( gamemode != commercial)
    {
  	if (wbs->epsd > 2)
	{
	    WI_drawEL();
	    return;
	}
	
	last = (wbs->last == 8) ? wbs->next - 1 : wbs->last;

	// draw a splat on taken cities.
	for (i=0 ; i<=last ; i++)
	    WI_drawOnLnode(i, splat);

	// splat the secret level?
	if (wbs->didsecret)
	    WI_drawOnLnode(8, splat);

	// draw flashing ptr
	if (snl_pointeron)
	    WI_drawOnLnode(wbs->next, yah); 
    }

    // draws which level you are entering..
    if ( (gamemode != commercial)
	 || wbs->next != 30)
	WI_drawEL();  

}

void WI_drawNoState(void)
{
    snl_pointeron = true;
    WI_drawShowNextLoc();
}

int WI_fragSum(int playernum)
{
    int		i;
    int		frags = 0;
    
    for (i=0 ; i<MAXPLAYERS ; i++)
    {
	if (playeringame[i]
	    && i!=playernum)
	{
	    frags += plrs[playernum].frags[i];
	}
    }

	
    // JDC hack - negative frags.
    frags -= plrs[playernum].frags[playernum];
    // UNUSED if (frags < 0)
    // 	frags = 0;

    return frags;
}



static int		dm_state;
static int		dm_frags[MAXPLAYERS][MAXPLAYERS];
static int		dm_totals[MAXPLAYERS];



void WI_initDeathmatchStats(void)
{

    int		i;
    int		j;

    state = StatCount;
    acceleratestage = 0;
    dm_state = 1;

    cnt_pause = TICRATE;

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
	if (playeringame[i])
	{
	    for (j=0 ; j<MAXPLAYERS ; j++)
		if (playeringame[j])
		    dm_frags[i][j] = 0;

	    dm_totals[i] = 0;
	}
    }
    
    WI_initAnimatedBack();
}



void WI_updateDeathmatchStats(void)
{

    int		i;
    int		j;
    
    boolean	stillticking;

    WI_updateAnimatedBack();

    if (acceleratestage && dm_state != 4)
    {
	acceleratestage = 0;

	for (i=0 ; i<MAXPLAYERS ; i++)
	{
	    if (playeringame[i])
	    {
		for (j=0 ; j<MAXPLAYERS ; j++)
		    if (playeringame[j])
			dm_frags[i][j] = plrs[i].frags[j];

		dm_totals[i] = WI_fragSum(i);
	    }
	}
	

	S_StartSound(0, sfx_barexp);
	dm_state = 4;
    }

    
    if (dm_state == 2)
    {
	if (!(bcnt&3))
	    S_StartSound(0, sfx_pistol);
	
	stillticking = false;

	for (i=0 ; i<MAXPLAYERS ; i++)
	{
	    if (playeringame[i])
	    {
		for (j=0 ; j<MAXPLAYERS ; j++)
		{
		    if (playeringame[j]
			&& dm_frags[i][j] != plrs[i].frags[j])
		    {
			if (plrs[i].frags[j] < 0)
			    dm_frags[i][j]--;
			else
			    dm_frags[i][j]++;

			if (dm_frags[i][j] > 99)
			    dm_frags[i][j] = 99;

			if (dm_frags[i][j] < -99)
			    dm_frags[i][j] = -99;
			
			stillticking = true;
		    }
		}
		dm_totals[i] = WI_fragSum(i);

		if (dm_totals[i] > 99)
		    dm_totals[i] = 99;
		
		if (dm_totals[i] < -99)
		    dm_totals[i] = -99;
	    }
	    
	}
	if (!stillticking)
	{
	    S_StartSound(0, sfx_barexp);
	    dm_state++;
	}

    }
    else if (dm_state == 4)
    {
	if (acceleratestage)
	{
	    S_StartSound(0, sfx_slop);

	    if ( gamemode == commercial)
		WI_initNoState();
	    else
		WI_initShowNextLoc();
	}
    }
    else if (dm_state & 1)
    {
	if (!--cnt_pause)
	{
	    dm_state++;
	    cnt_pause = TICRATE;
	}
    }
}



void WI_drawDeathmatchStats(void)
{
    int		j;
    int		w;

    WI_slamBackground();

    // draw animated background
    WI_drawAnimatedBack(); 
    WI_drawLF();

    // draw stat titles (top line)
    video::DrawScaledGraphic(DM_TOTALSX - total->width / 2,
        DM_MATRIXY - WI_SPACINGY + 10, *total);

    video::DrawScaledGraphic(DM_KILLERSX, DM_KILLERSY, *killers);
    video::DrawScaledGraphic(DM_VICTIMSX, DM_VICTIMSY, *victims);

    // draw P?
    int x = DM_MATRIXX + DM_SPACINGX;
    int y = DM_MATRIXY;

    for (int i = 0;i < MAXPLAYERS;i++)
    {
        if (playeringame[i])
        {
            video::DrawScaledGraphic(x - p[i].get().width / 2,
                DM_MATRIXY - WI_SPACINGY, p[i]);

            video::DrawScaledGraphic(DM_MATRIXX - p[i].get().width / 2,
                y, p[i]);

            if (i == me)
            {
                video::DrawScaledGraphic(x - p[i].get().width / 2,
                    DM_MATRIXY - WI_SPACINGY, *bstar);

                video::DrawScaledGraphic(DM_MATRIXX - p[i].get().width / 2,
                    y, *star);
            }
        }
        else
        {
            // V_DrawPatch(x-SHORT(bp[i]->width)/2,
            //   DM_MATRIXY - WI_SPACINGY, bp[i]);
            // V_DrawPatch(DM_MATRIXX-SHORT(bp[i]->width)/2,
            //   y, bp[i]);
        }
        x += DM_SPACINGX;
        y += WI_SPACINGY;
    }

    // draw stats
    y = DM_MATRIXY+10;
    w = num[0].get().width;

    for (int i = 0;i < MAXPLAYERS;i++)
    {
        x = DM_MATRIXX + DM_SPACINGX;

        if (playeringame[i])
        {
            for (j = 0;j < MAXPLAYERS;j++)
            {
                if (playeringame[j])
                {
                    WI_drawNum(x + w, y, dm_frags[i][j], 2);
                }

                x += DM_SPACINGX;
            }
            WI_drawNum(DM_TOTALSX + w, y, dm_totals[i], 2);
        }
        y += WI_SPACINGY;
    }
}

static int	cnt_frags[MAXPLAYERS];
static int	dofrags;
static int	ng_state;

void WI_initNetgameStats(void)
{

    int i;

    state = StatCount;
    acceleratestage = 0;
    ng_state = 1;

    cnt_pause = TICRATE;

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
	if (!playeringame[i])
	    continue;

	cnt_kills[i] = cnt_items[i] = cnt_secret[i] = cnt_frags[i] = 0;

	dofrags += WI_fragSum(i);
    }

    dofrags = !!dofrags;

    WI_initAnimatedBack();
}



void WI_updateNetgameStats(void)
{

    int		i;
    int		fsum;
    
    boolean	stillticking;

    WI_updateAnimatedBack();

    if (acceleratestage && ng_state != 10)
    {
	acceleratestage = 0;

	for (i=0 ; i<MAXPLAYERS ; i++)
	{
	    if (!playeringame[i])
		continue;

	    cnt_kills[i] = (plrs[i].skills * 100) / wbs->maxkills;
	    cnt_items[i] = (plrs[i].sitems * 100) / wbs->maxitems;
	    cnt_secret[i] = (plrs[i].ssecret * 100) / wbs->maxsecret;

	    if (dofrags)
		cnt_frags[i] = WI_fragSum(i);
	}
	S_StartSound(0, sfx_barexp);
	ng_state = 10;
    }

    if (ng_state == 2)
    {
	if (!(bcnt&3))
	    S_StartSound(0, sfx_pistol);

	stillticking = false;

	for (i=0 ; i<MAXPLAYERS ; i++)
	{
	    if (!playeringame[i])
		continue;

	    cnt_kills[i] += 2;

	    if (cnt_kills[i] >= (plrs[i].skills * 100) / wbs->maxkills)
		cnt_kills[i] = (plrs[i].skills * 100) / wbs->maxkills;
	    else
		stillticking = true;
	}
	
	if (!stillticking)
	{
	    S_StartSound(0, sfx_barexp);
	    ng_state++;
	}
    }
    else if (ng_state == 4)
    {
	if (!(bcnt&3))
	    S_StartSound(0, sfx_pistol);

	stillticking = false;

	for (i=0 ; i<MAXPLAYERS ; i++)
	{
	    if (!playeringame[i])
		continue;

	    cnt_items[i] += 2;
	    if (cnt_items[i] >= (plrs[i].sitems * 100) / wbs->maxitems)
		cnt_items[i] = (plrs[i].sitems * 100) / wbs->maxitems;
	    else
		stillticking = true;
	}
	if (!stillticking)
	{
	    S_StartSound(0, sfx_barexp);
	    ng_state++;
	}
    }
    else if (ng_state == 6)
    {
	if (!(bcnt&3))
	    S_StartSound(0, sfx_pistol);

	stillticking = false;

	for (i=0 ; i<MAXPLAYERS ; i++)
	{
	    if (!playeringame[i])
		continue;

	    cnt_secret[i] += 2;

	    if (cnt_secret[i] >= (plrs[i].ssecret * 100) / wbs->maxsecret)
		cnt_secret[i] = (plrs[i].ssecret * 100) / wbs->maxsecret;
	    else
		stillticking = true;
	}
	
	if (!stillticking)
	{
	    S_StartSound(0, sfx_barexp);
	    ng_state += 1 + 2*!dofrags;
	}
    }
    else if (ng_state == 8)
    {
	if (!(bcnt&3))
	    S_StartSound(0, sfx_pistol);

	stillticking = false;

	for (i=0 ; i<MAXPLAYERS ; i++)
	{
	    if (!playeringame[i])
		continue;

	    cnt_frags[i] += 1;

	    if (cnt_frags[i] >= (fsum = WI_fragSum(i)))
		cnt_frags[i] = fsum;
	    else
		stillticking = true;
	}
	
	if (!stillticking)
	{
	    S_StartSound(0, sfx_pldeth);
	    ng_state++;
	}
    }
    else if (ng_state == 10)
    {
	if (acceleratestage)
	{
	    S_StartSound(0, sfx_sgcock);
	    if ( gamemode == commercial )
		WI_initNoState();
	    else
		WI_initShowNextLoc();
	}
    }
    else if (ng_state & 1)
    {
	if (!--cnt_pause)
	{
	    ng_state++;
	    cnt_pause = TICRATE;
	}
    }
}



void WI_drawNetgameStats(void)
{
    int i;
    int x;
    int y;
    int pwidth = percent->width;

    WI_slamBackground();

    // draw animated background
    WI_drawAnimatedBack(); 

    WI_drawLF();

    // draw stat titles (top line)
    video::DrawScaledGraphic(NG_STATSX + NG_SPACINGX - kills->width,
        NG_STATSY, *kills);

    video::DrawScaledGraphic(NG_STATSX + 2 * NG_SPACINGX - items->width,
        NG_STATSY, *items);

    video::DrawScaledGraphic(NG_STATSX + 3 * NG_SPACINGX - secret->width,
        NG_STATSY, *secret);

    if (dofrags)
    {
        video::DrawScaledGraphic(NG_STATSX + 4 * NG_SPACINGX - frags->width,
            NG_STATSY, *frags);
    }

    // draw stats
    y = NG_STATSY + kills->height;

    for (i = 0;i < MAXPLAYERS;i++)
    {
        if (!playeringame[i])
        {
            continue;
        }

        x = NG_STATSX;
        video::DrawScaledGraphic(x - p[i].get().width, y, p[i]);

        if (i == me)
        {
            video::DrawScaledGraphic(x - p[i].get().width, y, *star);
        }

        x += NG_SPACINGX;
        WI_drawPercent(x - pwidth, y + 10, cnt_kills[i]);
        x += NG_SPACINGX;
        WI_drawPercent(x - pwidth, y + 10, cnt_items[i]);
        x += NG_SPACINGX;
        WI_drawPercent(x - pwidth, y + 10, cnt_secret[i]);
        x += NG_SPACINGX;

        if (dofrags)
        {
            WI_drawNum(x, y + 10, cnt_frags[i], -1);
        }

        y += WI_SPACINGY;
    }
}

static int	sp_state;

void WI_initStats(void)
{
    state = StatCount;
    acceleratestage = 0;
    sp_state = 1;
    cnt_kills[0] = cnt_items[0] = cnt_secret[0] = -1;
    cnt_time = cnt_par = -1;
    cnt_pause = TICRATE;

    WI_initAnimatedBack();
}

void WI_updateStats(void)
{

    WI_updateAnimatedBack();

    if (acceleratestage && sp_state != 10)
    {
	acceleratestage = 0;
	cnt_kills[0] = (plrs[me].skills * 100) / wbs->maxkills;
	cnt_items[0] = (plrs[me].sitems * 100) / wbs->maxitems;
	cnt_secret[0] = (plrs[me].ssecret * 100) / wbs->maxsecret;
	cnt_time = plrs[me].stime / TICRATE;
	cnt_par = wbs->partime / TICRATE;
	S_StartSound(0, sfx_barexp);
	sp_state = 10;
    }

    if (sp_state == 2)
    {
	cnt_kills[0] += 2;

	if (!(bcnt&3))
	    S_StartSound(0, sfx_pistol);

	if (cnt_kills[0] >= (plrs[me].skills * 100) / wbs->maxkills)
	{
	    cnt_kills[0] = (plrs[me].skills * 100) / wbs->maxkills;
	    S_StartSound(0, sfx_barexp);
	    sp_state++;
	}
    }
    else if (sp_state == 4)
    {
	cnt_items[0] += 2;

	if (!(bcnt&3))
	    S_StartSound(0, sfx_pistol);

	if (cnt_items[0] >= (plrs[me].sitems * 100) / wbs->maxitems)
	{
	    cnt_items[0] = (plrs[me].sitems * 100) / wbs->maxitems;
	    S_StartSound(0, sfx_barexp);
	    sp_state++;
	}
    }
    else if (sp_state == 6)
    {
	cnt_secret[0] += 2;

	if (!(bcnt&3))
	    S_StartSound(0, sfx_pistol);

	if (cnt_secret[0] >= (plrs[me].ssecret * 100) / wbs->maxsecret)
	{
	    cnt_secret[0] = (plrs[me].ssecret * 100) / wbs->maxsecret;
	    S_StartSound(0, sfx_barexp);
	    sp_state++;
	}
    }

    else if (sp_state == 8)
    {
	if (!(bcnt&3))
	    S_StartSound(0, sfx_pistol);

	cnt_time += 3;

	if (cnt_time >= plrs[me].stime / TICRATE)
	    cnt_time = plrs[me].stime / TICRATE;

	cnt_par += 3;

	if (cnt_par >= wbs->partime / TICRATE)
	{
	    cnt_par = wbs->partime / TICRATE;

	    if (cnt_time >= plrs[me].stime / TICRATE)
	    {
		S_StartSound(0, sfx_barexp);
		sp_state++;
	    }
	}
    }
    else if (sp_state == 10)
    {
	if (acceleratestage)
	{
	    S_StartSound(0, sfx_sgcock);

	    if (gamemode == commercial)
		WI_initNoState();
	    else
		WI_initShowNextLoc();
	}
    }
    else if (sp_state & 1)
    {
	if (!--cnt_pause)
	{
	    sp_state++;
	    cnt_pause = TICRATE;
	}
    }

}

void WI_drawStats(void)
{
    // line height
    int lh = (3 * num[0].get().height) / 2;

    WI_slamBackground();

    // draw animated background
    WI_drawAnimatedBack();

    WI_drawLF();

    video::DrawScaledGraphic(SP_STATSX, SP_STATSY, *kills);
    WI_drawPercent(VIRTUALWIDTH - SP_STATSX, SP_STATSY, cnt_kills[0]);

    video::DrawScaledGraphic(SP_STATSX, SP_STATSY + lh, *items);
    WI_drawPercent(VIRTUALWIDTH - SP_STATSX, SP_STATSY + lh, cnt_items[0]);

    video::DrawScaledGraphic(SP_STATSX, SP_STATSY + 2 * lh, *sp_secret);
    WI_drawPercent(VIRTUALWIDTH - SP_STATSX, SP_STATSY + 2 * lh, cnt_secret[0]);

    video::DrawScaledGraphic(SP_TIMEX, SP_TIMEY, *timepatch);
    WI_drawTime(VIRTUALWIDTH / 2 - SP_TIMEX, SP_TIMEY, cnt_time);

    if (wbs->epsd < 3)
    {
        video::DrawScaledGraphic(VIRTUALWIDTH / 2 + SP_TIMEX, SP_TIMEY, *par);
        WI_drawTime(VIRTUALWIDTH - SP_TIMEX, SP_TIMEY, cnt_par);
    }
}

void WI_checkForAccelerate(void)
{
    int   i;
    player_t  *player;

    // check for button presses to skip delays
    for (i=0, player = players ; i<MAXPLAYERS ; i++, player++)
    {
	if (playeringame[i])
	{
	    if (player->cmd.buttons & BT_ATTACK)
	    {
		if (!player->attackdown)
		    acceleratestage = 1;
		player->attackdown = true;
	    }
	    else
		player->attackdown = false;
	    if (player->cmd.buttons & BT_USE)
	    {
		if (!player->usedown)
		    acceleratestage = 1;
		player->usedown = true;
	    }
	    else
		player->usedown = false;
	}
    }
}



// Updates stuff each tick
void WI_Ticker(void)
{
    // counter for general background animation
    bcnt++;  

    if (bcnt == 1)
    {
	// intermission music
  	if ( gamemode == commercial )
	  S_ChangeMusic(mus_dm2int, true);
	else
	  S_ChangeMusic(mus_inter, true); 
    }

    WI_checkForAccelerate();

    switch (state)
    {
      case StatCount:
	if (deathmatch) WI_updateDeathmatchStats();
	else if (netgame) WI_updateNetgameStats();
	else WI_updateStats();
	break;
	
      case ShowNextLoc:
	WI_updateShowNextLoc();
	break;
	
      case NoState:
	WI_updateNoState();
	break;
    }

}

// Common load/unload function.  Iterates over all the graphics
// lumps to be loaded/unloaded into memory.
// [AM] For now, this just loads things.

static void WI_loadData2()
{
    int i, j;
    char name[9];
    Animation* a;

    lnames.clear();
    if (gamemode == commercial)
    {
        for (i = 0;i < NUMCMAPS;i++)
        {
            DEH_snprintf(name, 9, "CWILV%2.2d", i);
            lnames.emplace_back(std::cref(video::GraphicsManager::Instance().LoadPatch(name)));
        }
    }
    else
    {
        for (i = 0;i < NUMMAPS;i++)
        {
            DEH_snprintf(name, 9, "WILV%d%d", wbs->epsd, i);
            lnames.emplace_back(std::cref(video::GraphicsManager::Instance().LoadPatch(name)));
        }

        // you are here
        /*callback(DEH_String("WIURH0"), yah[0]);

        // you are here (alt.)
        callback(DEH_String("WIURH1"), yah[1]);

        // splat
        callback(DEH_String("WISPLAT"), splat[0]);*/

        if (wbs->epsd < 3)
        {
            for (j = 0;j < NUMANIMS[wbs->epsd];j++)
            {
                a = &anims[wbs->epsd][j];
                a->p.clear();
                for (i = 0;i < a->nanims;i++)
                {
                    // MONDO HACK!
                    if (wbs->epsd != 1 || j != 8)
                    {
                        // animations
                        DEH_snprintf(name, 9, "WIA%d%.2d%.2d", wbs->epsd, j, i);
                        a->p.emplace_back(std::cref(video::GraphicsManager::Instance().LoadPatch(name)));
                    }
                    else
                    {
                        // HACK ALERT!
                        a->p.emplace_back(anims[1][4].p[i]);
                    }
                }
            }
        }
    }

    // More hacks on minus sign.
    wiminus = &video::GraphicsManager::Instance().LoadPatch(DEH_String("WIMINUS"));

    num.clear();
    for (i = 0;i < 10;i++)
    {
        // numbers 0-9
        DEH_snprintf(name, 9, "WINUM%d", i);
        num.emplace_back(std::cref(video::GraphicsManager::Instance().LoadPatch(name)));
    }

    // percent sign
    percent = &video::GraphicsManager::Instance().LoadPatch(DEH_String("WIPCNT"));

    // "finished"
    finished = &video::GraphicsManager::Instance().LoadPatch(DEH_String("WIF"));

    // "entering"
    entering = &video::GraphicsManager::Instance().LoadPatch(DEH_String("WIENTER"));

    // "kills"
    kills = &video::GraphicsManager::Instance().LoadPatch(DEH_String("WIOSTK"));

    // "scrt"
    secret = &video::GraphicsManager::Instance().LoadPatch(DEH_String("WIOSTS"));

     // "secret"
    sp_secret = &video::GraphicsManager::Instance().LoadPatch(DEH_String("WISCRT2"));

    // french wad uses WIOBJ (?)
    if (W_CheckNumForName(DEH_String("WIOBJ")) >= 0)
    {
    	// "items"
        if (netgame && !deathmatch)
        {
            items = &video::GraphicsManager::Instance().LoadPatch(DEH_String("WIOBJ"));
        }
        else
        {
            items = &video::GraphicsManager::Instance().LoadPatch(DEH_String("WIOSTI"));
        }
    } else {
        items = &video::GraphicsManager::Instance().LoadPatch(DEH_String("WIOSTI"));
    }

    // "frgs"
    frags = &video::GraphicsManager::Instance().LoadPatch(DEH_String("WIFRGS"));

    // ":"
    colon = &video::GraphicsManager::Instance().LoadPatch(DEH_String("WICOLON"));

    // "time"
    timepatch = &video::GraphicsManager::Instance().LoadPatch(DEH_String("WITIME"));

    // "sucks"
    sucks = &video::GraphicsManager::Instance().LoadPatch(DEH_String("WISUCKS"));

    // "par"
    par = &video::GraphicsManager::Instance().LoadPatch(DEH_String("WIPAR"));

    // "killers" (vertical)
    killers = &video::GraphicsManager::Instance().LoadPatch(DEH_String("WIKILRS"));

    // "victims" (horiz)
    victims = &video::GraphicsManager::Instance().LoadPatch(DEH_String("WIVCTMS"));

    // "total"
    total = &video::GraphicsManager::Instance().LoadPatch(DEH_String("WIMSTT"));

    p.clear();
    bp.clear();
    for (i = 0;i < MAXPLAYERS;i++)
    {
        // "1,2,3,4"
        DEH_snprintf(name, 9, "STPB%d", i);
        p.emplace_back(std::cref(video::GraphicsManager::Instance().LoadPatch(name)));

        // "1,2,3,4"
        DEH_snprintf(name, 9, "WIBP%d", i+1);
        bp.emplace_back(std::cref(video::GraphicsManager::Instance().LoadPatch(name)));
    }

    // Background image

    if (gamemode == commercial)
    {
        M_StringCopy(name, DEH_String("INTERPIC"), sizeof(name));
    }
    else if (gameversion >= exe_ultimate && wbs->epsd == 3)
    {
        M_StringCopy(name, DEH_String("INTERPIC"), sizeof(name));
    }
    else
    {
	DEH_snprintf(name, sizeof(name), "WIMAP%d", wbs->epsd);
    }

    // Draw backdrop and save to a temporary buffer
    background = &video::GraphicsManager::Instance().LoadPatch(name);
}

void WI_loadData(void)
{
    if (gamemode == commercial)
    {
        NUMCMAPS = 32;
        lnames.reserve(NUMCMAPS);
    }
    else
    {
        lnames.reserve(NUMMAPS);
    }

    WI_loadData2();

    // These two graphics are special cased because we're sharing
    // them with the status bar code

    // your face
    star = &video::GraphicsManager::Instance().LoadPatch(DEH_String("STFST01"));

    // dead face
    star = &video::GraphicsManager::Instance().LoadPatch(DEH_String("STFDEAD0"));
}

static void WI_unloadCallback(const char *name, patch_t **variable)
{
    W_ReleaseLumpName(name);
    *variable = NULL;
}

void WI_unloadData(void)
{
    // [AM] If we ever decide to unallocate the intermission graphics,
    //      we'll do it here.

    // WI_loadUnloadData(WI_unloadCallback);

    // We do not free these lumps as they are shared with the status
    // bar code.
   
    // W_ReleaseLumpName("STFST01");
    // W_ReleaseLumpName("STFDEAD0");
}

void WI_Drawer (void)
{
    switch (state)
    {
      case StatCount:
	if (deathmatch)
	    WI_drawDeathmatchStats();
	else if (netgame)
	    WI_drawNetgameStats();
	else
	    WI_drawStats();
	break;
	
      case ShowNextLoc:
	WI_drawShowNextLoc();
	break;
	
      case NoState:
	WI_drawNoState();
	break;
    }
}


void WI_initVariables(wbstartstruct_t* wbstartstruct)
{

    wbs = wbstartstruct;

#ifdef RANGECHECKING
    if (gamemode != commercial)
    {
      if (gameversion >= exe_ultimate)
	RNGCHECK(wbs->epsd, 0, 3);
      else
	RNGCHECK(wbs->epsd, 0, 2);
    }
    else
    {
	RNGCHECK(wbs->last, 0, 8);
	RNGCHECK(wbs->next, 0, 8);
    }
    RNGCHECK(wbs->pnum, 0, MAXPLAYERS);
    RNGCHECK(wbs->pnum, 0, MAXPLAYERS);
#endif

    acceleratestage = 0;
    cnt = bcnt = 0;
    firstrefresh = 1;
    me = wbs->pnum;
    plrs = wbs->plyr;

    if (!wbs->maxkills)
	wbs->maxkills = 1;

    if (!wbs->maxitems)
	wbs->maxitems = 1;

    if (!wbs->maxsecret)
	wbs->maxsecret = 1;

    if ( gameversion < exe_ultimate )
      if (wbs->epsd > 2)
	wbs->epsd -= 3;
}

void WI_Start(wbstartstruct_t* wbstartstruct)
{
    WI_initVariables(wbstartstruct);
    WI_loadData();

    if (deathmatch)
	WI_initDeathmatchStats();
    else if (netgame)
	WI_initNetgameStats();
    else
	WI_initStats();
}

}
