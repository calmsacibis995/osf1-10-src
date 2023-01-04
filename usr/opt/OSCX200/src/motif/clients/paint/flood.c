/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */
/*
**************************************************************************
**                   DIGITAL EQUIPMENT CORPORATION                      **
**                         CONFIDENTIAL                                 **
**    NOT FOR MODIFICATION OR REDISTRIBUTION IN ANY MANNER WHATSOEVER   **
**************************************************************************
*/
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /usr/sde/x11/rcs/x11/src/./motif/clients/paint/flood.c,v 1.1.2.2 92/12/11 08:34:50 devrcs Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
/*
****************************************************************************
**                                                                          *
**  Copyright (c) 1987                                                      *
**  By DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                        *
**                                                                          *
**  This software is furnished under a license and may be used and  copied  *
**  only  in  accordance  with  the  terms  of  such  license and with the  *
**  inclusion of the above copyright notice.  This software or  any  other  *
**  copies  thereof may not be provided or otherwise made available to any  *
**  other person.  No title to and ownership of  the  software  is  hereby  *
**  transferred.                                                            *
**                                                                          *
**  The information in this software is subject to change  without  notice  *
**  and  should  not  be  construed  as  a commitment by DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL assumes no responsibility for the use or  reliability  of  its  *
**  software on equipment which is not supplied by DIGITAL.                 *
**                                                                          *
*****************************************************************************
**++
**  FACILITY:
**
**   DECpaint - VMS DECwindows paint program
**
**  AUTHOR
**
**   Daniel Latham, October 1987
**
**  ABSTRACT:
**
**   FLOOD - fill the entire contiguous one-color region containing the given
**   seed pixel.  Basics for algorithm taken from:
**   Rogers, David F., PROCEDURAL ELEMENTS FOR COMPUTER GRAPHICS. New York,
**   McGraw-Hill, 1985.  Chapter 2, sections 20-24.
**
**  ENVIRONMENT:
**
**   User mode, executable image.
**
**  MODIFICATION HISTORY:
**  jnh	03 Nov 88 Commented out casting of bytes to longwords (with ***).
**		  Removed bit-by-bit searching & filling routines.
**  jnh	26 Sep 88 Added more intelligent stack handling to reduce memory
**		  fragmentation.
**  jnh	24 May 88 Don't destroy the GC when done.
**  jnh	   May 88 Fixed another infinite-loop bug that 
**		  appeared during spraycan testing.
**  jnh	   May 88 Added GC handler.
**		  Implemented search by longwords to increase speed.
**		  Fixed picture-border bugs to prevent infinite loops.
**  jnh	   Apr 88 Changed algorithm.
**
**--
*/

/*
 *  SEARCHING A RANGE:  ALGORITHM
 *
 *	Let B = a blocked pixel, and O = an open pixel.
 *
 *	Let's consider the case of a range at y = Y extending 
 *	from x = L to x = R, where dy = -1 (which means open in the y-1 
 *	direction).  This range corresponds to the ?'s below:
 *
 *	Y   |		?????????????????????????
 *	Y+1 |		BBBBBBBBBBBBBBBBBBBBBBBBB
 *	    |
 *	    +-------------------------------------------------------------
 *			L			R
 *
 *	The algorithm will:
 *
 *	1. Search left from x = L until the first blocked pixel:
 *
 *	Y   |	BOOOOOOOO????????????????????????
 *	Y+1 |		BBBBBBBBBBBBBBBBBBBBBBBBB
 *	    |
 *	    +-------------------------------------------------------------
 *			L			R
 *
 *	2. Search right from x = L until the first blocked pixel:
 *
 *	Y   |	BOOOOOOOOOOB?????????????????????
 *	Y+1 |		BBBBBBBBBBBBBBBBBBBBBBBBB
 *	    |
 *	    +-------------------------------------------------------------
 *			L			R
 *
 *	3. Search right from the pixel found in (2) until the first open pixel:
 *
 *	Y   |	BOOOOOOOOOOBBB???????????????????
 *	Y+1 |		BBBBBBBBBBBBBBBBBBBBBBBBB
 *	    |
 *	    +-------------------------------------------------------------
 *			L			R
 *
 *	4. Search right from the pixel found in (3) until the first blocked 
 *	pixel:
 *
 *	Y   |	BOOOOOOOOOOBBBOOOOOOOOOOB????????
 *	Y+1 |		BBBBBBBBBBBBBBBBBBBBBBBBB
 *	    |
 *	    +-------------------------------------------------------------
 *			L			R
 */

/*
 *	5. Keep searching right as in steps (3) & (4) until x > R:
 *
 *	Y   |	BOOOOOOOOOOBBBOOOOOOOOOOBBBOOOOOOOOOOOOOOB
 *	Y+1 |		BBBBBBBBBBBBBBBBBBBBBBBBB
 *	    |
 *	    +-------------------------------------------------------------
 *			L			R
 *
 *	6. Add new ranges to the search list:
 *	   (the ranges marked "e" are called "ears" in the code comments)
 *
 *	Y-1 |    rrrrrrrrrr   rrrrrrrrrr   rrrrrrrrrrrrrr
 *	Y   |	BOOOOOOOOOOBBBOOOOOOOOOOBBBOOOOOOOOOOOOOOB
 *	Y+1 |	 eeeeee	BBBBBBBBBBBBBBBBBBBBBBBBB eeeeeee
 *	    |
 *	    +-------------------------------------------------------------
 *			L			R
 *
 *	7. Fill the open regions found, and mark them blocked:
 *
 *	Y-1 |    rrrrrrrrrr   rrrrrrrrrr   rrrrrrrrrrrrrr
 *	Y   |	BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB
 *	Y+1 |	 rrrrrr	BBBBBBBBBBBBBBBBBBBBBBBBB rrrrrrr
 *	    |
 *	    +------------------------------------
 *			L			R
 *
 *	The algorithm doesn't proceed in exactly this order (it fills
 *	regions and adds ranges to the stack as it goes along), but the
 *	above conveys the general idea.
 */

#include "paintrefs.h"
#include "DynList.h"

#define RANGE_STACK_QUANTUM 0x100
#define SCANLINE_STACK_SIZE 0x100

#define BIT_EMPTY 0
#define BIT_FILLED (unsigned int) 1

#define UBYTE_EMPTY 0
#define UBYTE_FILLED 0xff	/* maximum value of unsigned byte */
#define LOG2_BITS_PER_BYTE 3
#define BITS_PER_BYTE_MINUS_1 7

#define ULONG_FILLED (unsigned long) 0xffffffff	/* max value of unsigned long */
#define ULONG_EMPTY (unsigned long) 0
#define LOG2_BITS_PER_LONG 5
#define BITS_PER_LONG_MINUS_1 31
#define LOG2_BYTES_PER_LONG 2
#define BYTES_PER_LONG_MINUS_1 3
#define TRUNCATE_BPTR_TO_LPTR 0xfffffffc

typedef struct
{
    int y;
    int xl;
    int xr;
    int dy;	/* +1 means search the y+1 scanline; -1 means the y-1 scln */
} RANGE;

static XImage *flood_image;/* copy of image for "checking off" filled pixels */
static int LastColumn;		/* in flood_image->data */
static int bytes_per_line;	/* in flood_image->data */
static GC FloodGC;		/* graphics context for flooding */
static XSegment *Scanlines;	/* buffer of lines to be drawn to screen */
static int NScanlines;		/* number of lines in Scanlines */
static RANGE *RangeStack;/* stack of x-ranges to be checked for open pixels */
static int NRanges;		/* number of ranges in RangeStack */
static int RangeStackSize;	/* number of ranges allocated for RangeStack */
static unsigned long OpenBit;	/* value of user-selected pixel */
static unsigned long BlockedBit;  /* OpenBit ^ 1 */
static unsigned char OpenByte;	/* a byte filled with OpenBit's */
static unsigned char BlockedByte;   /* a byte filled with BlockedBit's */
static unsigned long OpenLong;	/* a long filled with OpenBit's */
static unsigned long BlockedLong;   /* a long filled with BlockedBit's */
static DYNAMIC_LIST Ranges;
static int RangeCount;

/* color stuff */
/*
    Flood a complex bordered region using the current foreground color.
    Flood starts at the given X, Y coordinate and floods all pixels of
    the same color as the starting pixel (@ X, Y).

    Because flood uses the XDrawSegments function to paint the area all the
    GC components that can be used for XDrawLine can be used for flood.
*/

static int LL, RR;

static unsigned char ReferencePixel, FillPixel;
static XImage **FloodImages;
static int RunCnt;
static int flood_xmin, flood_xmax, flood_ymin, flood_ymax;

typedef struct FillRunClass
{
    int                 X1, X2, Y;
    struct FillRunClass *Next;
}   FillRunInstance;

typedef struct ColorRangeClass
{
    int x1, x2, y;
    int up;
    struct ColorRangeClass *next;
}   ColorRangeInstance;

FillRunInstance *CurrentRun = NULL, *FirstRun = NULL, *NewRun;
ColorRangeInstance *ColorRanges = NULL;

void DrawRuns ()
{
    XSegment *FillRuns;

    FillRuns = (XSegment*)XtMalloc(sizeof(XSegment)*RunCnt);
    for (RunCnt = 0 , CurrentRun = FirstRun;
	 CurrentRun != NULL; 
	 CurrentRun = NewRun, RunCnt++)
    {
        FillRuns[RunCnt].x1 = CurrentRun->X1;
        FillRuns[RunCnt].x2 = CurrentRun->X2 + 1;
        FillRuns[RunCnt].y1 = FillRuns[RunCnt].y2 = CurrentRun->Y;
        NewRun = CurrentRun->Next;
        XtFree((char *)CurrentRun);
	CurrentRun = NULL;
    }
    FirstRun = NULL;

    XDrawSegments(disp, picture, FloodGC, FillRuns, RunCnt);
    Refresh_Picture
    (
	flood_xmin,
	flood_ymin,
	flood_xmax - flood_xmin,
	flood_ymax - flood_ymin
    );

    RunCnt = 0;
    XtFree ((char *)FillRuns);
    FillRuns = NULL;

    shape_xmin = MIN (flood_xmin, shape_xmin);
    shape_xmax = MAX (flood_xmax, shape_xmax);
    shape_ymin = MIN (flood_ymin, shape_ymin);
    shape_ymax = MAX (flood_ymax, shape_ymax);

    flood_xmin = pimage_wd;
    flood_xmax = 0;
    flood_ymin = pimage_ht;
    flood_ymax = 0;
}

ColorRangeInstance *PopColorRange ()
{
    ColorRangeInstance *r;

    r = ColorRanges;
    
    if (r != NULL) {
	ColorRanges = r->next;
    }

    return (r);
}

PushColorRange (x1, x2, y, up)
    int x1, x2, y, up;
{
    ColorRangeInstance *r;

    r = (ColorRangeInstance*)XtMalloc(sizeof(ColorRangeInstance));
    r->x1 = x1;
    r->x2 = x2;
    r->y = y;
    r->up = up;
    r->next = ColorRanges;
    ColorRanges = r;
}

PushColorRun (x1, x2, y)
    int x1, x2, y;
{
    NewRun = (FillRunInstance*)XtMalloc(sizeof(FillRunInstance));

    if(CurrentRun == NULL)
    {
        FirstRun = CurrentRun = NewRun;
    }
    else
    {
        CurrentRun = CurrentRun->Next = NewRun;
    }

    NewRun->X1 = x1;
    NewRun->X2 = x2;
    NewRun->Y = y;
    NewRun->Next = NULL;
    RunCnt++;

    if (flood_xmax <= x2)
	flood_xmax = x2 + 1;
    if (flood_xmin > x1)
	flood_xmin = x1;
    if (flood_ymax <= y)
	flood_ymax = y + 1;
    if (flood_ymin > y)
	flood_ymin = y;

    if (RunCnt >= SCANLINE_STACK_SIZE) {
	DrawRuns ();
    }
}

void GetFloodImage (num)
    int num;
{
    Continue_Save_Picture_State (ui_x(num), ui_y(num), 1, 1);
    FloodImages[num] = XGetImage (disp, picture, ui_x (num), ui_y (num),
				  ui_wd (num), ui_ht (num), bit_plane,
				  img_format);
/*
    if (FloodImages[num]->bitmap_bit_order != NATIVE_BIT_ORDER)
	ConvertImageNative (FloodImages[num]);
*/
}

void FindRuns (left, top, right, bottom)
    int	    left, top, right, bottom;
{
    unsigned char    *ImagePixel, *Data;
    int num, in_x, in_y;
    ColorRangeInstance *r;
    int xleft, xright, x, y;

    for (r = PopColorRange(); r != NULL; r = PopColorRange()) {
	if (((r->up) && (r->y > top)) ||
	    ((!(r->up)) && (r->y < bottom))) {

	    if (r->up) {
		y = r->y - 1;
	    }
	    else {
		y = r->y + 1;
	    }

	    num = y / UG_ht * UG_cols + r->x1 / UG_wd;
	    in_x = r->x1 - ui_x (num);
	    in_y = y - ui_y (num);
	    if (UG_image[num] == 0) {
		GetFloodImage (num);
	    }
	    Data = (unsigned char *)FloodImages[num]->data;
	    ImagePixel = &(Data[in_y * FloodImages[num]->bytes_per_line + in_x]);

	    for (x = r->x1; x <= r->x2; x++) {
		if (*ImagePixel == ReferencePixel) {
		    for (xleft = x;
			 (xleft >= left) && (*ImagePixel == ReferencePixel);
			 xleft--) {
			*ImagePixel = FillPixel;
			if (xleft > left) {
			    in_x--;
			    if (in_x < 0) {
				num--;
				in_x = UG_wd - 1;
				if (UG_image[num] == 0) {
				    GetFloodImage(num);
				}
				Data = (unsigned char *)FloodImages[num]->data;
				ImagePixel = &(Data[in_y * FloodImages[num]->bytes_per_line + in_x]);
			    }
			    else {
				ImagePixel--;
			    }
			}
		    }

		    num = y / UG_ht * UG_cols + (x + 1) / UG_wd;
		    in_x = x + 1 - ui_x (num);

		    if (UG_image[num] == 0) {
			GetFloodImage (num);
		    }

		    Data = (unsigned char *)FloodImages[num]->data;
		    ImagePixel = &(Data[in_y * FloodImages[num]->bytes_per_line + in_x]);

		    for (xright = x + 1;
			 (xright <= right) && (*ImagePixel == ReferencePixel);
			 xright++) {
			*ImagePixel = FillPixel;
			if (xright < right) {
			    in_x++;
			    if (in_x >= UG_wd) {
				num++;
				in_x = 0;
				if (UG_image[num] == 0) {
				    GetFloodImage (num);
				}
				Data = (unsigned char *)FloodImages[num]->data;
				ImagePixel = &(Data[in_y * FloodImages[num]->bytes_per_line + in_x]);
			    }
			    else {
				ImagePixel++;
			    }
			}
		    }
		    
		    xleft++;
		    xright--;

		    PushColorRun (xleft, xright, y);

		    if (xright > (r->x2 + 1)) {
			PushColorRange (r->x2 + 2, xright, y, !(r->up));
		    }

		    if (xleft < (r->x1 - 1)) {
			PushColorRange (xleft, r->x1 - 2, y, !(r->up));
		    }

		    PushColorRange (xleft, xright, y, r->up);

		    x = xright + 1;
		}

		if (x < r->x2) {
		    in_x++;
		    if (in_x >= UG_wd) {
			num++;
			in_x = 0;
			if (UG_image[num] == 0) {
			    GetFloodImage (num);
			}
			Data = (unsigned char *)FloodImages[num]->data;
			ImagePixel = &(Data[in_y * FloodImages[num]->bytes_per_line + in_x]);
		    }
		    else {
			ImagePixel++;
		    }
		}
	    }
	}
	XtFree ((char *)r);
    }    
}

void Flood_Fill_8  (X, Y)
    int X, Y;
{
    int num, i;
    int XL, XR;

    flood_xmin = shape_xmin;
    flood_xmax = shape_xmax;
    flood_ymin = shape_ymin;
    flood_ymax = shape_ymax;


    FloodImages = (XImage **) XtCalloc (UG_num, sizeof(XImage *));
    num = UG_used[0];

    FloodImages[num] = XGetImage (disp, picture, ui_x (num), ui_y (num),
                                  ui_wd (num), ui_ht (num), bit_plane,
                                  img_format);

/*
    if (FloodImages[num]->bitmap_bit_order != NATIVE_BIT_ORDER)
	ConvertImageNative (FloodImages[num]);
*/

    ReferencePixel = XGetPixel(FloodImages[num], X - ui_x (num),
			       Y - ui_y (num));
    FillPixel = ~ReferencePixel;
    RunCnt = 0;

    ColorRanges = NULL;
    PushColorRange (X - LL, X + RR, Y, FALSE);
    PushColorRange (X, X, Y + 1, TRUE);

    FindRuns(pic_xorigin, pic_yorigin, pic_xorigin + pwindow_wd - 1,
	     pic_yorigin + pwindow_ht - 1);
    DrawRuns ();

    for (i = 0; i < UG_num_used; i++)
    {
	if (FloodImages[UG_used[i]] != NULL)
	{
#if 0
	    XtFree (FloodImages[UG_used[i]]->data);
#endif
	    XDestroyImage (FloodImages[UG_used[i]]);
	    FloodImages[UG_used[i]] = NULL;
	}
    }
    XtFree ((char *)FloodImages);
    FloodImages = NULL;

    undo_x = shape_xmin;
    undo_y = shape_ymin;
    undo_width = shape_xmax - shape_xmin;
    undo_height = shape_ymax - shape_ymin;
}

/*
** THE SCRATCH IMAGE (used for 'checking off' processed pixels)
*/

static void CreateFloodImage ()
    /* globalref Display *disp; */
    /* globalref Pixmap picture; */
    /* globalref int picture_wd; */
    /* static XImage *flood_image; */
    /* static int LastColumn; */
    /* static int bytes_per_line; */
{
    flood_image = XGetImage (disp, picture, 
			     pic_xorigin, pic_yorigin, 
			     pwindow_wd, pwindow_ht, 
			     1, XYPixmap);

    if (flood_image->bitmap_bit_order != NATIVE_BIT_ORDER) /*ram*/
	ConvertImageNative (flood_image);                  /*ram*/
    LastColumn = pwindow_wd - 1;
    bytes_per_line = flood_image->bytes_per_line;
}

static void DestroyFloodImage ()
    /* static XImage *flood_image; */
{
    XDestroyImage ( flood_image );
}

/* jj-port -> */
long Pixel_Value(x, y)	/* return is long to support 16+ plane color */
    register int x;
    register int y; /* jj-port <- */
    /* static XImage *flood_image; */
    /* static int bytes_per_line; */
{
    if (flood_image->data[(y * bytes_per_line) + 
			  (x >> LOG2_BITS_PER_BYTE)] & 
				(BIT_FILLED << (x & BITS_PER_BYTE_MINUS_1))) {
	return (1L);
    } else {
	return (0L);
    }
}

/*
** GRAPHIC OUTPUT OF SCANLINE SEGMENTS
*/
static void CreateScanlineStack()
{
    Scanlines = (XSegment *) XtMalloc (SCANLINE_STACK_SIZE * sizeof (XSegment));
    NScanlines = 0;
}

static void DestroyScanlineStack()
    /* globalref Display *disp; */
    /* globalref Pixmap picture; */
    /* globalref int shape_xmin; */
    /* globalref int shape_xmax; */
    /* globalref int shape_ymin; */
    /* globalref int shape_ymax; */
    /* globalref int picture_wd; */
    /* globalref int picture_ht; */
    /* static GC FloodGC; */
    /* static XSegment *Scanlines; */
    /* static int NScanlines; */
{
    XDrawSegments( disp, picture, FloodGC, Scanlines, NScanlines );
    Refresh_Picture(shape_xmin, shape_ymin, shape_xmax-shape_xmin,
	    shape_ymax-shape_ymin);
    shape_ymax = shape_xmax = -1;
    shape_ymin = picture_ht;
    shape_xmin = picture_wd;
    XtFree ((char *)Scanlines);
    Scanlines = NULL;
}

/* jj-port -> */
static void PushScanline (y, xl, xr)
    register int y; 
    register int xl; 
    register int xr; /* jj-port <- */
    /* static XSegment *Scanlines; */
    /* static int NScanlines; */
{
    Scanlines[NScanlines].x1 = xl + pic_xorigin;
    Scanlines[NScanlines].y1 = y + pic_yorigin;
    Scanlines[NScanlines].x2 = xr + pic_xorigin;
    Scanlines[NScanlines++].y2 = y + pic_yorigin;
}

static void DrawScanline(y, xl, xr)
    register int y;	/* y coordinate of scanline */
    register int xl;	/* left x coordinate */
    register int xr; 	/* right x coordinate */ /* jj-port <- */
	/* This routine assumes that xr >= xl. */
    /* globalref Display *disp; */
    /* globalref Pixmap picture; */
    /* globalref int shape_xmin; */
    /* globalref int shape_xmax; */
    /* globalref int shape_ymin; */
    /* globalref int shape_ymax; */
    /* globalref int picture_wd; */
    /* globalref int picture_ht; */
    /* static GC FloodGC; */
    /* static XSegment *Scanlines; */
    /* static int NScanlines; */
{
    if( shape_xmax < (++xr + pic_xorigin)) shape_xmax = xr + pic_xorigin;
    if( shape_xmin > (xl + pic_xorigin)) shape_xmin = xl + pic_xorigin;
    if( shape_ymax <= (y + pic_yorigin)) shape_ymax = y + 1 + pic_yorigin;
    if( shape_ymin > (y + pic_yorigin)) shape_ymin = y + pic_yorigin;

    PushScanline (y, xl, xr);
    if (NScanlines >= SCANLINE_STACK_SIZE) {

	/* Draws both endpoints of each line. */
        XDrawSegments( disp, picture, FloodGC, Scanlines, NScanlines );
	NScanlines = 0;
	Refresh_Picture(shape_xmin, shape_ymin, shape_xmax-shape_xmin,
	    shape_ymax-shape_ymin);
        if( ifocus == zoom_widget )
            Refresh_Zoom_View( shape_xmin, shape_ymin, 
             shape_xmax-shape_xmin, shape_ymax-shape_ymin);
	shape_ymax = shape_xmax = -1;
	shape_xmin = picture_wd;
	shape_ymin = picture_ht;
/*
	XFlush (disp);
*/
    }
}

/*
**  RANGE STACK FOR SCANLINE-STACK ALGORITHM 
*/
static void create_range_stack ()
{
    Ranges = DynListInit (sizeof (RANGE), RANGE_STACK_QUANTUM, 0, "");
    RangeCount = 0;
} /* end of create_range_stack() */

static void destroy_range_stack ()
    /* static Range *RangeStack; */
{
    DynListDestroy (Ranges);
} /* end of destroy_range_stack() */

/* jj-port -> */
static void push_range (y, xl, xr, dy)
    register int y;
    register int xl;
    register int xr;
    register int dy; /* jj-port <- */
  {
    register RANGE *current_range;

    y += dy;
    if ((y >= 0) && (y < pwindow_ht) && (xr >= xl))
      {
        RangeCount++;
	current_range = (RANGE *) DynListNext (Ranges);
	current_range->y = y;
	current_range->xl = xl;
	current_range->xr = xr;
	current_range->dy = dy;
      }
  } /* end of push_range() */

static void pop_range (y, xl, xr, dy)
/*  register */ int *y;
/*  register */ int *xl;
/*  register */ int *xr;
/*  register */ int *dy; /* jj-port <- */
  {
    register RANGE *current_range;

    RangeCount--;
    current_range = (RANGE *) DynListPop (Ranges);
    *y = current_range->y;
    *xl = current_range->xl;
    *xr = current_range->xr;
    *dy = current_range->dy;
  } /* end of pop_range() */

/*
 *  SEARCHING AND FILLING ROUTINES FOR SCANLINE-STACK ALGORITHM
 */

/* fill from the current pixel until the next non-open pixel. */
static int FillLeft (y, x, xmin)
		/* return the x value of the last pixel filled. */
		/* if first pixel is blocked, return x+1. */
		/* if all pixels were open, return xmin.*/
		/* if x < xmin, return xmin. */
    register int y;
    register int x;
    register int xmin;	/* last pixel to fill */ /* jj-port <- */
    /* static int bytes_per_line; */
    /* static unsigned long OpenBit; */
    /* static unsigned char OpenByte; */
    /* static unsigned char BlockedByte; */
    /* static unsigned long OpenLong; */
    /* static unsigned long BlockedLong; */
{
    register unsigned char *byte;
    register unsigned char *last_full_byte;
    register int found = 0;
    register int bit;

    if (x < xmin) return (xmin);

    bit = x & BITS_PER_BYTE_MINUS_1;
    byte = (unsigned char *) flood_image->data + 
	   ((y * bytes_per_line) + (x >> LOG2_BITS_PER_BYTE));
    last_full_byte = (unsigned char *) flood_image->data + 
		     ((y * bytes_per_line) + 
		      ((xmin + BITS_PER_BYTE_MINUS_1) >> LOG2_BITS_PER_BYTE));

    if (byte >= last_full_byte) {
	if (bit < BITS_PER_BYTE_MINUS_1) { /* if first byte is partial */
	    register int mask1;

	    while (bit >= 0) {  /* for all bits in byte */
		if (((*byte & (mask1 = BIT_FILLED << bit)) >> bit) == OpenBit) {
		    *byte ^= mask1;
		    bit--;
		} else {
		    found++;
		    byte++;
		    break;
		}
	    }
	    byte--;
	}

	if (!found) { /* if the search is not over */
	    register unsigned char *last_byte;
/***
	    register unsigned long *lptr;
	    register unsigned long *last_full_long;

	    last_full_long = (unsigned long *) (byte - 
			  ((byte - last_full_byte - BYTES_PER_LONG_MINUS_1) &
			   TRUNCATE_BPTR_TO_LPTR) -
			  BYTES_PER_LONG_MINUS_1);
	    lptr = (unsigned long *) (byte - BYTES_PER_LONG_MINUS_1);
	    while (lptr >= last_full_long && *lptr == OpenLong) {
		*(lptr--) = BlockedLong; 
	    }
	    byte = ((unsigned char *) lptr) + BYTES_PER_LONG_MINUS_1;
***/
	    last_byte = /*** (lptr >= last_full_long) ? 
			    byte - BYTES_PER_LONG_MINUS_1 : ***/
			    last_full_byte;
	    while (byte >= last_byte && *byte == OpenByte) {
		*(byte--) = BlockedByte;
	    }
	    bit = BITS_PER_BYTE_MINUS_1;
	}
    }
	
    if (!found) {
	register int last_bit;
	register int mask2;

	last_bit = (byte >= last_full_byte) ?
		    0 :
		    (((xmin - 1) & BITS_PER_BYTE_MINUS_1) + 1); /* 1 to 8 */
	while (bit >= last_bit) {	/* for all bits in the last byte */
	    if (((*byte & (mask2 = BIT_FILLED << bit)) >> bit) == OpenBit) {
		*byte ^= mask2;
		bit--;
	    } else {
		break;
	    }
	}
    }
/* jj-port */
     return(((((char *)byte - flood_image->data) - (y * bytes_per_line) + 1)
	    << LOG2_BITS_PER_BYTE) + (bit - BITS_PER_BYTE_MINUS_1));
}

/* fill from the current pixel until the next non-open pixel. */
static int FillRight (y, x, xmax)
		/* return the x value of the last pixel filled. */
		/* if first pixel is blocked, return x-1. */
		/* if all pixels were open, return xmax.*/
		/* if x > xmax, return xmax. */
    register int y;
    register int x;
    register int xmax;	/* last pixel to fill */ /* jj-port <- */
    /* static int bytes_per_line; */
    /* static unsigned long OpenBit; */
    /* static unsigned char OpenByte; */
    /* static unsigned char BlockedByte; */
    /* static unsigned long OpenLong; */
    /* static unsigned long BlockedLong; */
{
    register unsigned char *byte;
    register unsigned char *last_full_byte;
    register int found = 0;
    register int bit;

    if (x > xmax) return (xmax);

    bit = x & BITS_PER_BYTE_MINUS_1;
    byte = (unsigned char *) flood_image->data + ((y * bytes_per_line) + 
				(x >> LOG2_BITS_PER_BYTE));
    last_full_byte = (unsigned char *) flood_image->data + 
		     ((y * bytes_per_line) + 
		      ((xmax + 1) >> LOG2_BITS_PER_BYTE) - 1); /* jj-port */

    if (byte <= last_full_byte) {
	if (bit > 0) {	/* if first byte is partial */
	    register int mask1;

	    while (bit <= BITS_PER_BYTE_MINUS_1) {  /* for all bits in byte */
		if (((*byte & (mask1 = BIT_FILLED << bit)) >> bit) == OpenBit) {
		    *byte ^= mask1;
		    bit++;
		} else {
		    found++;
		    byte--;
		    break;
		}
	    }
	    byte++;
	}

	if (!found) { /* if the search is not over */
	    register unsigned char *last_byte;
/***
	    register unsigned long *lptr;
	    register unsigned long *last_full_long;

	    last_full_long = (unsigned long *) (byte + 
		((last_full_byte - byte - BYTES_PER_LONG_MINUS_1) &
		 TRUNCATE_BPTR_TO_LPTR));
	    lptr = (unsigned long *) byte;
	    while (lptr <= last_full_long && *lptr == OpenLong) {
		*(lptr++) = BlockedLong;
	    }
	    byte = (unsigned char *) lptr;
***/
	    last_byte = /*** (lptr <= last_full_long) ? 
			    byte + BYTES_PER_LONG_MINUS_1 : ***/
			    last_full_byte;
	    while (byte <= last_byte && *byte == OpenByte) {
		*(byte++) = BlockedByte;
	    }
	    bit = 0;
	}
    }
	
    if (!found) {
	register int last_bit;
	register int mask2;

	last_bit = (byte <= last_full_byte) ?
		    BITS_PER_BYTE_MINUS_1 :
		    (((xmax + 1) & BITS_PER_BYTE_MINUS_1) - 1); /* -1 to +6 */
	while (bit <= last_bit) {	/* for all bits in the last byte */
	    if (((*byte & (mask2 = BIT_FILLED << bit)) >> bit) == OpenBit) {
		*byte ^= mask2;
		bit++;
	    } else {
		break;
	    }
	}
    }
     return(((((char *)byte - flood_image->data) - (y * bytes_per_line))
            << LOG2_BITS_PER_BYTE) + (bit - 1));
}

/* search from the current pixel for the next open pixel. */
static int SearchRight (y, x, xmax)
		/* return the x value of the first open pixel. */
		/* if first pixel is open, return it. */
		/* if all pixels were blocked, return xmax+1. */
		/* if x > xmax, return x. */
    register int y;
    register int x;
    register int xmax;	/* last pixel to search */ /* jj-port <- */
    /* static int bytes_per_line; */
    /* static unsigned long OpenBit; */
    /* static unsigned char BlockedByte; */
    /* static unsigned long BlockedLong; */
{
    register unsigned char *byte;
    register unsigned char *last_full_byte;
    register int found = 0;
    register int bit;

    if (x > xmax) return (x);

    bit = x & BITS_PER_BYTE_MINUS_1;
    byte = (unsigned char *) flood_image->data + 
	   ((y * bytes_per_line) + (x >> LOG2_BITS_PER_BYTE));

    last_full_byte = (unsigned char *) flood_image->data + 
		     ((y * bytes_per_line) + 
		      ((xmax + 1) >> LOG2_BITS_PER_BYTE) - 1); /* jj-port */

    if (byte <= last_full_byte) {   /* if all bits are not in the same byte */
	if (bit > 0) {			/* if first byte is partial */
	    while (bit <= BITS_PER_BYTE_MINUS_1) {  /* for all bits in byte */
		if (((*byte & (BIT_FILLED << bit)) >> bit) != OpenBit) {
		    bit++;
		} else {
		    found++;
		    byte--;
		    break;
		}
	    }
	    byte++;
	}

	if (!found) { /* if the search is not over */
	    register unsigned char *last_byte;
/***	    register unsigned long *lptr;
	    register unsigned long *last_full_long;

	    last_full_long = (unsigned long *) (byte + 
		((last_full_byte - byte - BYTES_PER_LONG_MINUS_1) &
		 TRUNCATE_BPTR_TO_LPTR));
	    lptr = (unsigned long *) byte;
	    while (lptr <= last_full_long && *lptr == BlockedLong) {
		lptr++;
	    }
	    byte = (unsigned char *) lptr;
***/
	    last_byte = /*** (lptr <= last_full_long) ? 
			    byte + BYTES_PER_LONG_MINUS_1 : ***/
			    last_full_byte;
	    while (byte <= last_byte && *byte == BlockedByte) {
		byte++;
	    }
	    bit = 0;
	}
    }

    if (!found) {
	register int last_bit;

	last_bit = (byte <= last_full_byte) ?
		    BITS_PER_BYTE_MINUS_1 :
		    (((xmax + 1) & BITS_PER_BYTE_MINUS_1) - 1); /* -1 to +6 */
	while (bit <= last_bit) {	/* for all bits in the last byte */
	    if (((*byte & (BIT_FILLED << bit)) >> bit) != OpenBit) {
		bit++;
	    } else {
		break;
	    }
	}
    }
/* jj-port */
     return(((((char *)byte - flood_image->data) - (y * bytes_per_line))
	    << LOG2_BITS_PER_BYTE) + bit);
}

/*
 *  SEARCHING ONE RANGE
 */
static void FloodRange ()
    /* static int LastColumn; */
{   /* jj-port -> */
    /* register */ int range_xl;
    /* register */ int range_xr;
    /* register */ int dy;
    /* register */ int y; /* jj-port <- */
    register int xl;
    register int xr;
    char str[80];

    pop_range (&y, &range_xl, &range_xr, &dy);

    if ((xl = xr = FillLeft (y, range_xl, 0)) <= range_xl) {/* range_xl open */
	if (xl <= (xr = range_xl - 2)) { 
	    push_range (y, xl, xr, -dy);	/* left ear */
	}
	xr = FillRight (y, ++range_xl, LastColumn);
	push_range (y, xl, xr, dy);
	DrawScanline (y, xl, xr);
    }

    while ((xl = SearchRight (y, xr, range_xr)) <= range_xr) {
	xr = FillRight (y, xl, LastColumn);/* xr = last filled */
	push_range (y, xl, xr, dy);
	DrawScanline (y, xl, xr);
    }
    if ((xl = range_xr + 2) <= xr) {
	push_range (y, xl, xr, -dy);	/* right ear */
    }
}

/*
 *  FLOOD FILL USING SCANLINE-SEED ALGORITHM:  MAIN ROUTINE
 */
void Flood_Fill (x, y)
    register int x;  /* seed */
    register int y; /* jj-port <- */
    /* static unsigned long OpenBit; */
    /* static unsigned long BlockedBit; */
    /* static unsigned char OpenByte; */
    /* static unsigned char BlockedByte; */
{
    XImage  *tmp_image;
    long    tmp_pixel, tmp_pixel2;

/* if fill pattern is null, don't flood - dl 10/6/88 */
    flood_same_pattern = FALSE;
    if (!fill_stipple) {
	return;
    }

    tmp_image = XGetImage (disp, picture, x, y, 1, 1, bit_plane, img_format);
    tmp_pixel = XGetPixel (tmp_image, 0, 0);
    if (pdepth == 1) {
	if (((tmp_pixel == 1) && (fill_stipple == solid_fg_stipple)) ||
	    ((tmp_pixel == 0) && (fill_stipple == solid_bg_stipple))) {
	    flood_same_pattern = TRUE;
	}
    }
    else {
	if (((tmp_pixel == colormap[paint_color].pixel) &&
	    (fill_stipple == solid_fg_stipple)) ||
	   ((tmp_pixel == colormap[paint_bg_color].pixel) &&
	    (fill_stipple == solid_bg_stipple))) {
	    flood_same_pattern = TRUE;
	}
    }

    XDestroyImage (tmp_image);
    if (flood_same_pattern)
    {
	return;
    }

    if (x > pic_xorigin)
    {
	tmp_image = XGetImage (disp, picture, x - 1, y, 1, 1, bit_plane, img_format);
	tmp_pixel2 = XGetPixel (tmp_image, 0, 0);
	XDestroyImage (tmp_image);
	if (tmp_pixel2 == tmp_pixel) {
	    LL = 1;
	}
	else {
	    LL = 0;
	}
    }
    else {
	LL = 0;
    }

    if (x < (pic_xorigin + pwindow_wd - 1)) {
	tmp_image = XGetImage (disp, picture, x + 1, y, 1, 1, bit_plane, img_format);
	tmp_pixel2 = XGetPixel (tmp_image, 0, 0);
	XDestroyImage (tmp_image);
	if (tmp_pixel2 == tmp_pixel) {
	    RR = 1;
	}
	else {
	    RR = 0;
	}
    }
    else {
	RR = 0;
    }



    /* Initialize */
    Set_Cursor_Watch(pwindow);
    Save_Picture_State ();	/* Save old picture for the UNDO command */
    FloodGC = Get_GC (GC_PD_FLOOD);
    if (pdepth == 1) {
	x -= pic_xorigin;
	y -= pic_yorigin;
	CreateFloodImage ();
	CreateScanlineStack ();
	create_range_stack ();

        OpenBit = Pixel_Value( x, y );
	BlockedBit = BIT_FILLED - OpenBit;
	OpenByte = OpenBit ? UBYTE_FILLED : UBYTE_EMPTY;
	BlockedByte = BlockedBit ? UBYTE_FILLED : UBYTE_EMPTY;
	OpenLong = OpenBit ? ULONG_FILLED : ULONG_EMPTY;
	BlockedLong = BlockedBit ? ULONG_FILLED : ULONG_EMPTY;

	push_range (y, x-LL, x+RR, 1);	/* seed for searching in +y direction */
	push_range (y+1, x, x, -1);	/* seed for searching in -y direction */
	while (RangeCount > 0) {
	    FloodRange ();
	}
	/* clean up */
	destroy_range_stack ();
	DestroyScanlineStack ();
	DestroyFloodImage ();
	/* puts ("Done."); */
    }
    else {
	Flood_Fill_8 (x, y);
    }
    Set_Cursor(pwindow, current_action);
}
