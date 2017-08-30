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
//	The actual span/column drawing functions.
//	Here find the main potential for optimization,
//	 e.g. inline assembly, different algorithms.
//




#include "doomdef.h"
#include "deh_main.h"

#include "i_system.h"
#include "z_zone.h"
#include "w_wad.h"

#include "r_local.h"

// Needs access to LFB (guess what).
#include "v_video.h"

// State.
#include "doomstat.h"

// status bar height at bottom of screen
#define SBARHEIGHT		32

//
// All drawing to the view buffer is accomplished in this file.
// The other refresh files only know about ccordinates,
//  not the architecture of the frame buffer.
// Conveniently, the frame buffer is a linear one,
//  and we need only the base address,
//  and the total size == width*height*depth/8.,
//


byte*		viewimage; 
int		viewwidth;
int		viewheight;
int		viewwindowx;
int		viewwindowy; 

// Given a Y coordinate, find the leftmost pixel in the world buffer.
pixel_t*	ylookup[MAXHEIGHT];

// Given an X coordinate, find the proper amount of bytes to offset into
// the given row.  In theory, this used to be used as a way to scale down
// the renderer horizontally.  In practice, it's no longer used, since
// the world buffer is always the correct size.
int		columnofs[MAXWIDTH]; 

// Color tables for different players,
//  translate a limited part to another
//  (color ramps used for  suit colors).
//
byte		translations[3][256];	
 
// Backing buffer containing the bezel drawn around the screen and 
// surrounding background.

static pixel_t *background_buffer = NULL;


//
// R_DrawColumn
// Source is the top of the column to scale.
//
lighttable_t*		dc_colormap; 
int			dc_x; 
int			dc_yl; 
int			dc_yh; 
fixed_t			dc_iscale; 
fixed_t			dc_texturemid;
int			dc_texheight; // [crispy] Tutti-Frutti fix

// first pixel in a column (possibly virtual) 
byte*			dc_source;		

// just for profiling 
int			dccount;

//
// A column is a vertical slice/span from a wall texture that,
//  given the DOOM style restrictions on the view orientation,
//  will always have constant z depth.
// Thus a special case loop for very fast rendering can
//  be used. It has also been used with Wolfenstein 3D.
// 
// [crispy] replace R_DrawColumn() with Lee Killough's implementation
// found in MBF to fix Tutti-Frutti, taken from mbfsrc/R_DRAW.C:99-1979

void R_DrawColumn(void)
{
    int			count;
    byte*		dest;
    fixed_t		frac;
    fixed_t		fracstep;
    int			heightmask = dc_texheight - 1;

    count = dc_yh - dc_yl;

    // Zero length, column does not exceed a pixel.
    if (count < 0)
        return;

#ifdef RANGECHECK 
    if ((unsigned)dc_x >= viewwidth
        || dc_yl < 0
        || dc_yh >= viewheight)
        I_Error("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
#endif 

    // Framebuffer destination address.
    // Use ylookup LUT to avoid multiply with ScreenWidth.
    // Use columnofs LUT for subwindows? 
    dest = ylookup[dc_yl] + columnofs[dc_x];

    // Determine scaling,
    //  which is the only mapping to be done.
    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl - centery)*fracstep;

    // Inner loop that does the actual texture mapping,
    //  e.g. a DDA-lile scaling.
    // This is as fast as it gets.

    // heightmask is the Tutti-Frutti fix -- killough
    if (dc_texheight & heightmask) // not a power of 2 -- killough
    {
        heightmask++;
        heightmask <<= FRACBITS;

        if (frac < 0)
        {
            while ((frac += heightmask) < 0);
        }
        else
        {
            while (frac >= heightmask)
            {
                frac -= heightmask;
            }
        }

        do
        {
            *dest = dc_colormap[dc_source[frac >> FRACBITS]];
            dest += viewwidth;

            if ((frac += fracstep) >= heightmask)
            {
                frac -= heightmask;
            }
        } while (count--);
    }
    else // texture height is a power of 2 -- killough
    {
        do
        {
            // Re-map color indices from wall texture column
            //  using a lighting/special effects LUT.
            *dest = dc_colormap[dc_source[(frac >> FRACBITS)&heightmask]];
            dest += viewwidth;
            frac += fracstep;
        } while (count--);
    }
}

//
// Spectre/Invisibility.
//
#define FUZZTABLE		50 
#define FUZZOFF	(VIRTUALWIDTH) // FIXME: Set to horizonal resolution


int	fuzzoffset[FUZZTABLE] =
{
    FUZZOFF,-FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
    FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
    FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,
    FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
    FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,
    FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,
    FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF 
}; 

int	fuzzpos = 0; 


//
// Framebuffer postprocessing.
// Creates a fuzzy image by copying pixels
//  from adjacent ones to left and right.
// Used with an all black colormap, this
//  could create the SHADOW effect,
//  i.e. spectres and invisible players.
//
void R_DrawFuzzColumn (void) 
{ 
    int			count; 
    pixel_t*		dest;
    fixed_t		frac;
    fixed_t		fracstep;	 

    // Adjust borders. Low... 
    if (!dc_yl) 
	dc_yl = 1;

    // .. and high.
    if (dc_yh == viewheight-1) 
	dc_yh = viewheight - 2; 
		 
    count = dc_yh - dc_yl; 

    // Zero length.
    if (count < 0) 
	return; 

#ifdef RANGECHECK 
    if ((unsigned)dc_x >= viewwidth || dc_yl < 0 || dc_yh >= viewheight)
    {
        I_Error ("R_DrawFuzzColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
    }
#endif
    
    dest = ylookup[dc_yl] + columnofs[dc_x];

    // Looks familiar.
    fracstep = dc_iscale; 
    frac = dc_texturemid + (dc_yl-centery)*fracstep; 

    // Looks like an attempt at dithering,
    //  using the colormap #6 (of 0-31, a bit
    //  brighter than average).
    do 
    {
	// Lookup framebuffer, and retrieve
	//  a pixel that is either one column
	//  left or right of the current one.
	// Add index from colormap to index.
	*dest = colormaps[6*256+dest[fuzzoffset[fuzzpos]]]; 

	// Clamp table lookup index.
	if (++fuzzpos == FUZZTABLE) 
	    fuzzpos = 0;
	
	dest += viewwidth;

	frac += fracstep; 
    } while (count--); 
} 

//
// R_DrawTranslatedColumn
// Used to draw player sprites
//  with the green colorramp mapped to others.
// Could be used with different translation
//  tables, e.g. the lighter colored version
//  of the BaronOfHell, the HellKnight, uses
//  identical sprites, kinda brightened up.
//
byte*	dc_translation;
byte*	translationtables;

void R_DrawTranslatedColumn (void) 
{ 
    int			count; 
    pixel_t*		dest;
    fixed_t		frac;
    fixed_t		fracstep;	 
 
    count = dc_yh - dc_yl; 
    if (count < 0) 
	return; 
				 
#ifdef RANGECHECK 
    if ((unsigned)dc_x >= viewwidth || dc_yl < 0 || dc_yh >= viewheight)
    {
        I_Error ( "R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
    }
#endif 


    dest = ylookup[dc_yl] + columnofs[dc_x]; 

    // Looks familiar.
    fracstep = dc_iscale; 
    frac = dc_texturemid + (dc_yl-centery)*fracstep; 

    // Here we do an additional index re-mapping.
    do 
    {
	// Translation tables are used
	//  to map certain colorramps to other ones,
	//  used with PLAY sprites.
	// Thus the "green" ramp of the player 0 sprite
	//  is mapped to gray, red, black/indigo. 
	*dest = dc_colormap[dc_translation[dc_source[frac>>FRACBITS]]];
	dest += viewwidth;
	
	frac += fracstep; 
    } while (count--); 
} 



//
// R_InitTranslationTables
// Creates the translation tables to map
//  the green color ramp to gray, brown, red.
// Assumes a given structure of the PLAYPAL.
// Could be read from a lump instead.
//
void R_InitTranslationTables (void)
{
    int		i;
	
    translationtables = static_cast<byte*>(Z_Malloc (256*3, PU_STATIC, 0));
    
    // translate just the 16 green colors
    for (i=0 ; i<256 ; i++)
    {
	if (i >= 0x70 && i<= 0x7f)
	{
	    // map green ramp to gray, brown, red
	    translationtables[i] = 0x60 + (i&0xf);
	    translationtables [i+256] = 0x40 + (i&0xf);
	    translationtables [i+512] = 0x20 + (i&0xf);
	}
	else
	{
	    // Keep all other colors as is.
	    translationtables[i] = translationtables[i+256] 
		= translationtables[i+512] = i;
	}
    }
}




//
// R_DrawSpan 
// With DOOM style restrictions on view orientation,
//  the floors and ceilings consist of horizontal slices
//  or spans with constant z depth.
// However, rotation around the world z axis is possible,
//  thus this mapping, while simpler and faster than
//  perspective correct texture mapping, has to traverse
//  the texture at an angle in all but a few cases.
// In consequence, flats are not stored by column (like walls),
//  and the inner loop has to step in texture space u and v.
//
int			ds_y; 
int			ds_x1; 
int			ds_x2;

lighttable_t*		ds_colormap; 

fixed_t			ds_xfrac; 
fixed_t			ds_yfrac; 
fixed_t			ds_xstep; 
fixed_t			ds_ystep;

// start of a 64*64 tile image 
byte*			ds_source;	

// just for profiling
int			dscount;


//
// Draws the actual span.
void R_DrawSpan (void) 
{ 
    unsigned int position, step;
    pixel_t *dest;
    int count;
    int spot;
    unsigned int xtemp, ytemp;

#ifdef RANGECHECK
    if (ds_x2 < ds_x1 || ds_x1 < 0 || ds_x2 >= viewwidth || (unsigned)ds_y > viewheight)
    {
        I_Error( "R_DrawSpan: %i to %i at %i", ds_x1, ds_x2, ds_y);
    }
//	dscount++;
#endif

    dest = ylookup[ds_y] + columnofs[ds_x1];

    // We do not check for zero spans here?
    count = ds_x2 - ds_x1;

    // [AM] An unrolled version of the loop at the bottom.  Seems to save
    //      a few hundreths of a ms.
    while (count >= 3)
    {
        ytemp = (ds_yfrac >> 10) & 0x0fc0;
        xtemp = (ds_xfrac >> 16) & 0x3f;
        int spot1 = xtemp | ytemp;

        ytemp = (ds_yfrac + ds_ystep >> 10) & 0x0fc0;
        xtemp = (ds_xfrac + ds_xstep >> 16) & 0x3f;
        int spot2 = xtemp | ytemp;

        ytemp = (ds_yfrac + (ds_ystep * 2) >> 10) & 0x0fc0;
        xtemp = (ds_xfrac + (ds_xstep * 2) >> 16) & 0x3f;
        int spot3 = xtemp | ytemp;

        ytemp = (ds_yfrac + (ds_ystep * 3) >> 10) & 0x0fc0;
        xtemp = (ds_xfrac + (ds_xstep * 3) >> 16) & 0x3f;
        int spot4 = xtemp | ytemp;

        dest[0] = ds_colormap[ds_source[spot1]];
        dest[1] = ds_colormap[ds_source[spot2]];
        dest[2] = ds_colormap[ds_source[spot3]];
        dest[3] = ds_colormap[ds_source[spot4]];

        ds_xfrac += (ds_xstep * 4);
        ds_yfrac += (ds_ystep * 4);

        dest += 4;
        count -= 4;
    }

    while (count >= 0)
    {
	// Calculate current texture index in u,v.
        // [crispy] fix flats getting more distorted the closer they are to the right
        ytemp = (ds_yfrac >> 10) & 0x0fc0;
        xtemp = (ds_xfrac >> 16) & 0x3f;
        spot = xtemp | ytemp;

	// Lookup pixel from flat texture tile,
	//  re-index using light/colormap.
	*dest++ = ds_colormap[ds_source[spot]];

        ds_xfrac += ds_xstep;
        ds_yfrac += ds_ystep;

        count--;
    }
}



//
// R_InitBuffer 
// Creats lookup tables that avoid
//  multiplies and other hazzles
//  for getting the framebuffer address
//  of a pixel to draw.
//
void
R_InitBuffer
( int		width,
  int		height ) 
{ 
    int		i; 

    // Handle resize,
    //  e.g. smaller view windows
    //  with border and/or status bar.
    viewwindowx = (viewwidth - width) >> 1; 

    // Column offset. For windows.
    for (i=0 ; i<width ; i++) 
	columnofs[i] = viewwindowx + i;

    // Samw with base row offset.
    if (width == viewwidth)
	viewwindowy = 0; 
    else 
	viewwindowy = (viewheight - SBARHEIGHT - height) >> 1;

    // Preclaculate all row offsets.
    for (i=0 ; i<height ; i++) 
	ylookup[i] = I_VideoBuffer->GetRawPixels() + (i + viewwindowy) * viewwidth;
} 
 
 


//
// R_FillBackScreen
// Fills the back screen with a pattern
//  for variable screen sizes
// Also draws a beveled edge.
//
void R_FillBackScreen (void) 
{ 
    byte*	src;
    pixel_t*	dest;
    int		x;
    int		y; 
    patch_t*	patch;

    // No.  No backscreen.
    return;

    /*
    // DOOM border patch.
    const char       *name1 = DEH_String("FLOOR7_2");

    // DOOM II border patch.
    const char *name2 = DEH_String("GRNROCK");

    const char *name;

    // If we are running full screen, there is no need to do any of this,
    // and the background buffer can be freed if it was previously in use.

    if (viewwidth == SCREENWIDTH)
    {
        if (background_buffer != NULL)
        {
            Z_Free(background_buffer);
            background_buffer = NULL;
        }

	return;
    }

    // Allocate the background buffer if necessary
	
    if (background_buffer == NULL)
    {
        background_buffer = static_cast<pixel_t*>(Z_Malloc(SCREENWIDTH * (SCREENHEIGHT - SBARHEIGHT) * sizeof(*background_buffer),
                                     PU_STATIC, NULL));
    }

    if (gamemode == commercial)
	name = name2;
    else
	name = name1;
    
    src = static_cast<byte*>(W_CacheLumpName(name, PU_CACHE));
    dest = background_buffer;
	 
    for (y=0 ; y<SCREENHEIGHT-SBARHEIGHT ; y++) 
    { 
	for (x=0 ; x<SCREENWIDTH/64 ; x++) 
	{ 
	    memcpy (dest, src+((y&63)<<6), 64); 
	    dest += 64; 
	} 

	if (SCREENWIDTH&63) 
	{ 
	    memcpy (dest, src+((y&63)<<6), SCREENWIDTH&63); 
	    dest += (SCREENWIDTH&63); 
	} 
    } 
     
    // Draw screen and bezel; this is done to a separate screen buffer.

    V_UseBuffer(background_buffer);

    patch = static_cast<patch_t*>(W_CacheLumpName(DEH_String("brdr_t"),PU_CACHE));

    for (x=0 ; x<viewwidth ; x+=8)
	V_DrawPatch(viewwindowx+x, viewwindowy-8, patch);
    patch = static_cast<patch_t*>(W_CacheLumpName(DEH_String("brdr_b"),PU_CACHE));

    for (x=0 ; x<viewwidth ; x+=8)
	V_DrawPatch(viewwindowx+x, viewwindowy+viewheight, patch);
    patch = static_cast<patch_t*>(W_CacheLumpName(DEH_String("brdr_l"),PU_CACHE));

    for (y=0 ; y<viewheight ; y+=8)
	V_DrawPatch(viewwindowx-8, viewwindowy+y, patch);
    patch = static_cast<patch_t*>(W_CacheLumpName(DEH_String("brdr_r"),PU_CACHE));

    for (y=0 ; y<viewheight ; y+=8)
	V_DrawPatch(viewwindowx+ viewwidth, viewwindowy+y, patch);

    // Draw beveled edge. 
    V_DrawPatch(viewwindowx-8,
                viewwindowy-8,
                static_cast<patch_t*>(W_CacheLumpName(DEH_String("brdr_tl"),PU_CACHE)));
    
    V_DrawPatch(viewwindowx+viewwidth,
                viewwindowy-8,
                static_cast<patch_t*>(W_CacheLumpName(DEH_String("brdr_tr"),PU_CACHE)));
    
    V_DrawPatch(viewwindowx-8,
                viewwindowy+viewheight,
                static_cast<patch_t*>(W_CacheLumpName(DEH_String("brdr_bl"),PU_CACHE)));
    
    V_DrawPatch(viewwindowx+viewwidth,
                viewwindowy+viewheight,
                static_cast<patch_t*>(W_CacheLumpName(DEH_String("brdr_br"),PU_CACHE)));

    V_RestoreBuffer();
    */
} 
 

//
// Copy a screen buffer.
//
void
R_VideoErase
( unsigned	ofs,
  int		count ) 
{ 
  // LFB copy.
  // This might not be a good idea if memcpy
  //  is not optiomal, e.g. byte by byte on
  //  a 32bit CPU, as GNU GCC/Linux libc did
  //  at one point.

    if (background_buffer != NULL)
    {
        memcpy(I_VideoBuffer->GetRawPixels() + ofs, background_buffer + ofs, count * I_VideoBuffer->GetSize());
    }
} 


//
// R_DrawViewBorder
// Draws the border around the view
//  for different size windows?
//
void R_DrawViewBorder (void) 
{ 
    int		top;
    int		side;
    int		ofs;
    int		i; 
 
    if (viewwidth == viewwidth || true)
	return; 
  
    top = ((viewheight - SBARHEIGHT) - viewheight) / 2;
    side = (viewwidth - viewwidth) / 2;
 
    // copy top and one line of left side 
    R_VideoErase (0, top * viewwidth + side);
 
    // copy one line of right side and bottom 
    ofs = (viewheight + top) * viewwidth - side;
    R_VideoErase(ofs, top * viewwidth + side);
 
    // copy sides using wraparound 
    ofs = top * viewwidth + viewwidth - side;
    side <<= 1;
    
    for (i=1 ; i<viewheight ; i++) 
    { 
	R_VideoErase (ofs, side); 
	ofs += viewwidth;
    } 

    // ? 
    V_MarkRect(0, 0, viewwidth, viewheight - SBARHEIGHT);
} 
 
 
