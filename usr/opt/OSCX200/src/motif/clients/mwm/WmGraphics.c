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
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: WmGraphics.c,v $ $Revision: 1.1.4.4 $ $Date: 1994/01/18 18:33:36 $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

/*
 * Included Files:
 */

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/StringDefs.h>
#include <X11/Xutil.h>
#include <X11/Intrinsic.h>
#include <X11/X.h>
#include <Xm/Xm.h>

#include "WmGlobal.h"

#define RLIST_EXTENSION_SIZE	10

#ifndef MIN
#define MAX(x,y) ((x)>(y)?(x):(y))
#endif

/*
 * include extern functions
 */
#include "WmGraphics.h"
#include "WmError.h"



/*
 * Global Variables:
 */


/*
 * Macros:
 */

/* test if > 0 and return 1 if true, 0 if false. */
#define GE1(x) ((x)>0?1:0)



/*************************************<->*************************************
 *
 *   Procedure:	BevelRectangle (prTop, prBot, x, y, 
 *                      width, height, top_wid, right_wid, bot_wid, left_wid)
 *
 *  Description:
 *  -----------
 *  Generate data for top- and bottom-shadow bevels on a box.
 *
 *  Inputs:
 *  ------
 *  prTop	- ptr to top shadow RList
 *  prBot	- ptr to bottom shadow RList
 *  x,y		- position of rectangle to bevel
 *  width 	- (outside) width of rectangle
 *  height 	- (outside) height of rectangle
 *  top_wid	- width of beveling on top side of rectangle
 *  right_wid	- width of beveling on right side of rectangle
 *  bot_wid	- width of beveling on bottom side of rectangle
 *  left_wid	- width of beveling on left side of rectangle
 * 
 *  Outputs:
 *  -------
 *  prTop	- top shadows for this rectangle added to list
 *  prBot	- bottom shadows for this rectangle added to list
 *  
 *
 *  Comments:
 *  --------
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void BevelRectangle (prTop, prBot, x, y, width, height, top_wid, right_wid, bot_wid, left_wid)
    RList *prTop, *prBot;
    int x, y; 
    unsigned int width, height, top_wid, right_wid, bot_wid, left_wid;
#else /* _NO_PROTO */
void BevelRectangle (RList *prTop, RList *prBot, int x, int y, unsigned int width, unsigned int height, unsigned int top_wid, unsigned int right_wid, unsigned int bot_wid, unsigned int left_wid)
#endif /* _NO_PROTO */
{
    XRectangle *prect;		/* pointer to "current" rectangle */
    register int count;		/* counter used for beveling operation */
    int join1, join2;		/* used to compute "good" bevel joints */
    int x1, y1, len;		/* used to compute bevel parameters */
    int *piTop, *piBot;


    /* build the rectangles to implement the beveling on each side */

    /* top side */

    if (((prTop->used + (top_wid + left_wid)) > prTop->allocated) &&
	(!ExtendRList (prTop, MAX (top_wid+left_wid, RLIST_EXTENSION_SIZE))))
    {
	return;		/* not enough memory */
    }

    piTop = &(prTop->used);
    prect = &(prTop->prect[*piTop]);

    join1 = left_wid;
    join2 = right_wid;
    x1 = x;
    y1 = y;
    len = width;
    for (count=top_wid; count>0; count--, prect++, (*piTop)++) 
    {
	prect->x = x1;
	prect->y = y1;
	prect->width = len;
	prect->height = 1;
	x1 += GE1(--join1);
	y1 += 1;
	len -= GE1(join1) + GE1(--join2);
    }

    /* left side */

    join1 = top_wid;
    join2 = bot_wid;
    x1 = x;
    y1 = y+GE1(join1);
    len = height-GE1(join1);
    for (count=left_wid; count >0; count--, prect++, (*piTop)++) 
    {	
	prect->x = x1;
	prect->y = y1;
	prect->width = 1;
	prect->height = len;
	x1 += 1;
	y1 += GE1(--join1);
	len -= GE1(join1) + GE1(--join2);
    }


    /* bottom side */

    if (((prBot->used + (bot_wid + right_wid)) > prBot->allocated) &&
	(!ExtendRList(prBot, MAX (bot_wid+right_wid, RLIST_EXTENSION_SIZE))))
    {
	return;
    }

    piBot = &(prBot->used);
    prect = &(prBot->prect[*piBot]);

    join1 = left_wid;
    join2 = right_wid;
    x1 = x+GE1(join1);
    y1 = y+height-1;
    len = width-GE1(join1);
    /* fudge fat bottom shadow to overwrite corner of skinny left shadow */
    if (GE1(join1) && (bot_wid > left_wid)) {
	len++;
	x1--;
	join1++;
    }
    for (count=bot_wid; count >0; count--, prect++, (*piBot)++) 
    {
	prect->x = x1;
	prect->y = y1;
	prect->width = len;
	prect->height = 1;
	x1 += GE1(--join1);
	y1 -= 1;
	len -= GE1(join1) + GE1(--join2);
    }

    /* right side */

    join1 = top_wid;
    join2 = bot_wid;
    x1 = x+width-1;
    y1 = y+GE1(join1);
    len = height - GE1(join1) - GE1(join2);
    /* fudge fat right shadow to overwrite corner of skinny top shadow */
    if (GE1(join1) && (right_wid > top_wid)) {
	len++;
	y1--;
	join1++;
    }
    for (count=right_wid; count >0; count--, prect++, (*piBot)++) 
    {
	prect->x = x1;
	prect->y = y1;
	prect->width = 1;
	prect->height = len;
	x1 -= 1;
	y1 += GE1(--join1);
	len -= GE1(join1) + GE1(--join2);
    }

} /* END OF FUNCTION BevelRectangle */




/*************************************<->*************************************
 *
 *   Procedure:	BevelDepressedRectangle (prTop, prBot, x, y, 
 *                      width, height, top_wid, right_wid, bot_wid, left_wid
 *                      in_wid)
 *
 *  Description:
 *  -----------
 *  Generate data for top- and bottom-shadow bevels on a rectangle with
 *  the center part depressed.
 *
 *  Inputs:
 *  ------
 *  prTop	- ptr to top shadow RList
 *  prBot	- ptr to bottom shadow RList
 *  x,y		- position of rectangle to bevel
 *  width 	- (outside) width of rectangle
 *  height 	- (outside) height of rectangle
 *  top_wid	- width of beveling on top side of rectangle
 *  right_wid	- width of beveling on right side of rectangle
 *  bot_wid	- width of beveling on bottom side of rectangle
 *  left_wid	- width of beveling on left side of rectangle
 *  in_wid	- width of depressed beveling inside of rectangle
 * 
 *  Outputs:
 *  -------
 *  prTop	- top shadows for this rectangle added to list
 *  prBot	- bottom shadows for this rectangle added to list
 *  
 *
 *  Comments:
 *  --------
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void BevelDepressedRectangle (prTop, prBot, x, y, width, height, top_wid, right_wid, bot_wid, left_wid, in_wid)
    RList *prTop, *prBot;
    int x, y; 
    unsigned int width, height, top_wid, right_wid, bot_wid, left_wid, in_wid;
#else /* _NO_PROTO */
void BevelDepressedRectangle (RList *prTop, RList *prBot, int x, int y, unsigned int width, unsigned int height, unsigned int top_wid, unsigned int right_wid, unsigned int bot_wid, unsigned int left_wid, unsigned int in_wid)
#endif /* _NO_PROTO */
{
    XRectangle *prect;		/* pointer to "current" rectangle */
    register int count;		/* counter used for beveling operation */
    int join1, join2;		/* used to compute "good" bevel joints */
    int x1, y1, len;		/* used to compute bevel parameters */
    int *piTop, *piBot;


    /* 
     *  Build the rectangles to implement the beveling on each side 
     *  First, guarantee that there is enough memory.
     */


    if (((prTop->used + (top_wid + left_wid)) > prTop->allocated) &&
	(!ExtendRList (prTop, MAX (top_wid+left_wid, RLIST_EXTENSION_SIZE))))
    {
	return;		/* not enough memory */
    }

    if (((prBot->used + (bot_wid + right_wid)) > prBot->allocated) &&
	(!ExtendRList(prBot, MAX (bot_wid+right_wid, RLIST_EXTENSION_SIZE))))
    {
	return;		/* not enought memory */
    }



    /* top side (normal beveling) */

    piTop = &(prTop->used);
    prect = &(prTop->prect[*piTop]);

    join1 = left_wid;
    join2 = right_wid;
    x1 = x;
    y1 = y;
    len = width;
    for (count=top_wid - in_wid; count>0; count--, prect++, (*piTop)++) 
    {
	prect->x = x1;
	prect->y = y1;
	prect->width = len;
	prect->height = 1;
	x1 += GE1(--join1);
	y1 += 1;
	len -= GE1(join1) + GE1(--join2);
    }

    /* top side (inverted beveling) */

    piBot = &(prBot->used);
    prect = &(prBot->prect[*piBot]);

    for (count=in_wid; count>0; count--, prect++, (*piBot)++) 
    {
	prect->x = x1;
	prect->y = y1;
	prect->width = len;
	prect->height = 1;
	x1 += GE1(--join1);
	y1 += 1;
	len -= GE1(join1) + GE1(--join2);
    }



    /* left side (normal beveling) */

    piTop = &(prTop->used);
    prect = &(prTop->prect[*piTop]);

    join1 = top_wid;
    join2 = bot_wid;
    x1 = x;
    y1 = y+GE1(join1);
    len = height-GE1(join1);
    for (count=left_wid-in_wid; count >0; count--, prect++, (*piTop)++) 
    {	
	prect->x = x1;
	prect->y = y1;
	prect->width = 1;
	prect->height = len;
	x1 += 1;
	y1 += GE1(--join1);
	len -= GE1(join1) + GE1(--join2);
    }

    /* left side (inverted beveling) */

    piBot = &(prBot->used);
    prect = &(prBot->prect[*piBot]);

    for (count=in_wid; count >0; count--, prect++, (*piBot)++) 
    {	
	prect->x = x1;
	prect->y = y1;
	prect->width = 1;
	prect->height = len;
	x1 += 1;
	y1 += GE1(--join1);
	len -= GE1(join1) + GE1(--join2);
    }



    /* bottom side (normal beveling) */

    piBot = &(prBot->used);
    prect = &(prBot->prect[*piBot]);

    join1 = left_wid;
    join2 = right_wid;
    x1 = x+GE1(join1);
    y1 = y+height-1;
    len = width-GE1(join1);
    /* fudge fat bottom shadow to overwrite corner of skinny left shadow */
    if (GE1(join1) && (bot_wid > left_wid)) {
	len++;
	x1--;
	join1++;
    }
    for (count=bot_wid-in_wid; count >0; count--, prect++, (*piBot)++) 
    {
	prect->x = x1;
	prect->y = y1;
	prect->width = len;
	prect->height = 1;
	x1 += GE1(--join1);
	y1 -= 1;
	len -= GE1(join1) + GE1(--join2);
    }

    /* bottom side (inverted beveling) */

    piTop = &(prTop->used);
    prect = &(prTop->prect[*piTop]);

    for (count=in_wid; count >0; count--, prect++, (*piTop)++) 
    {
	prect->x = x1;
	prect->y = y1;
	prect->width = len;
	prect->height = 1;
	x1 += GE1(--join1);
	y1 -= 1;
	len -= GE1(join1) + GE1(--join2);
    }



    /* right side (normal beveling) */

    piBot = &(prBot->used);
    prect = &(prBot->prect[*piBot]);

    join1 = top_wid;
    join2 = bot_wid;
    x1 = x+width-1;
    y1 = y+GE1(join1);
    len = height - GE1(join1) - GE1(join2);
    /* fudge fat right shadow to overwrite corner of skinny top shadow */
    if (GE1(join1) && (right_wid > top_wid)) {
	len++;
	y1--;
	join1++;
    }
    for (count=right_wid-in_wid; count >0; count--, prect++, (*piBot)++) 
    {
	prect->x = x1;
	prect->y = y1;
	prect->width = 1;
	prect->height = len;
	x1 -= 1;
	y1 += GE1(--join1);
	len -= GE1(join1) + GE1(--join2);
    }

    /* right side (inverted beveling) */

    piTop = &(prTop->used);
    prect = &(prTop->prect[*piTop]);

    for (count=in_wid; count >0; count--, prect++, (*piTop)++) 
    {
	prect->x = x1;
	prect->y = y1;
	prect->width = 1;
	prect->height = len;
	x1 -= 1;
	y1 += GE1(--join1);
	len -= GE1(join1) + GE1(--join2);
    }

} /* END OF FUNCTION BevelDepressedRectangle */




/*************************************<->*************************************
 *
 *   Procedure:	StretcherCorner (prTop, prBot, x, y, cnum, 
 *                               swidth,  cwidth, cheight);
 *
 *  Description:
 *  -----------
 *  Generate data to draw a corner of the stretcher border.
 *
 *  Inputs:
 *  ------
 *  prTop	- ptr to top shadow RList
 *  prBot	- ptr to bottom shadow RList
 *  x,y		- position of rectangle enclosing the cornern
 *  cnum 	- corner number; which corner to draw
 *                ASSUMES only NW, NE, SE, SW for mwm ()
 *  swidth 	- width (thickness) of border (includes bevels)
 *  cwidth 	- corner width from corner to end of horizontal run
 *  cheight 	- corner height from corner to end of vertical run
 * 
 *  Outputs:
 *  -------
 *  prTop	- array filled in for top shadows 
 *  prBot	- array filledin for bottom shadows
 *
 *  Comments:
 *  --------
 *  o Uses only 1 pixel bevels. Beveling is hard coded.
 *  o XFillRectangles assumed as an optimization to take
 *    advantage of the block mover hardware.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void StretcherCorner (prTop, prBot, x, y, cnum, swidth, cwidth, cheight)
    RList *prTop, *prBot;
    int x, y, cnum;
    unsigned int swidth, cwidth, cheight;
#else /* _NO_PROTO */
void StretcherCorner (RList *prTop, RList *prBot, int x, int y, int cnum, unsigned int swidth, unsigned int cwidth, unsigned int cheight)
#endif /* _NO_PROTO */
{
    XRectangle *prect;		/* pointer to "current" rectangle */
    int *piTop, *piBot;

    switch (cnum) {

	case STRETCH_NORTH_WEST:
		if (((prTop->used + 4) > prTop->allocated) &&
		    (!ExtendRList (prTop, (unsigned int) RLIST_EXTENSION_SIZE)))
		{
		    return;
		}

		piTop = &(prTop->used);
		prect = &(prTop->prect[*piTop]);

		prect->x = x;			/* top (row 1) */
		prect->y = y;
		prect->width = cwidth;
		prect->height = 1;
		prect++;
		(*piTop)++;

		prect->x = x+1;			/* top (row 2) */
		prect->y = y+1;
		prect->width = cwidth-2;
		prect->height = 1;
		prect++;
		(*piTop)++;

		prect->x = x;			/* left (col 1) */
		prect->y = y+1;
		prect->width = 1;
		prect->height = cheight-1;
		prect++;
		(*piTop)++;

		prect->x = x+1;			/* left (col 2) */
		prect->y = y+2;
		prect->width = 1;
		prect->height = cheight-3;
		(*piTop)++;

		if (((prBot->used + 4) > prBot->allocated) &&
		    (!ExtendRList (prBot, (unsigned int) RLIST_EXTENSION_SIZE)))
		{
		    return;
		}

		piBot = &(prBot->used);
		prect = &(prBot->prect[*piBot]); /* bottom shadow parts */


		prect->x = x+1;			/* bottom end */
		prect->y = y+cheight-1;
		prect->width = swidth-1;
		prect->height = 1;
		prect++;
		(*piBot)++;

		prect->x = x+swidth-1;		/* right inside */
		prect->y = y+swidth-1;
		prect->width = 1;
		prect->height = cheight-swidth;
		prect++;
		(*piBot)++;

		prect->x = x+swidth;		/* bottom inside */
		prect->y = y+swidth-1;
		prect->width = cwidth-swidth;
		prect->height = 1;
		prect++;
		(*piBot)++;

		prect->x = x+cwidth-1;		/* right end */
		prect->y = y+1;
		prect->width = 1;
		prect->height = swidth-2;
		(*piBot)++;

		break;

	case STRETCH_NORTH_EAST:
		if (((prTop->used + 4) > prTop->allocated) &&
		    (!ExtendRList (prTop, (unsigned int) RLIST_EXTENSION_SIZE)))
		{
		    return;
		}

		piTop = &(prTop->used);
		prect = &(prTop->prect[*piTop]);

		prect->x = x;			/* top (row 1) */
		prect->y = y;
		prect->width = cwidth;
		prect->height = 1;
		prect++;
		(*piTop)++;

		prect->x = x+1;			/* top (row 2) */
		prect->y = y+1;
		prect->width = cwidth-2;
		prect->height = 1;
		prect++;
		(*piTop)++;

		prect->x = x;			/* left end */
		prect->y = y+1;
		prect->width = 1;
		prect->height = swidth-1;
		prect++;
		(*piTop)++;

		prect->x = x+cwidth-swidth;	/* left inside (col 1) */
		prect->y = y+swidth;
		prect->width = 1;
		prect->height = cheight-swidth;
		(*piTop)++;

		if (((prBot->used + 4) > prBot->allocated) &&
		    (!ExtendRList (prBot, (unsigned int) RLIST_EXTENSION_SIZE)))
		{
		    return;
		}

		piBot = &(prBot->used);
		prect = &(prBot->prect[*piBot]); /* bottom shadow parts */


		prect->x = x+cwidth-swidth+1;	/* bottom end */
		prect->y = y+cheight-1;
		prect->width = swidth-1;
		prect->height = 1;
		prect++;
		(*piBot)++;

		prect->x = x+cwidth-1;		/* right (col 2) */
		prect->y = y+1;
		prect->width = 1;
		prect->height = cheight-2;
		prect++;
		(*piBot)++;

		prect->x = x+cwidth-2;		/* right (col 1) */
		prect->y = y+2;
		prect->width = 1;
		prect->height = cheight-3;
		prect++;
		(*piBot)++;

		prect->x = x+1;			/* bottom inside (row 2) */
		prect->y = y+swidth-1;
		prect->width = cwidth-swidth;
		prect->height = 1;
		(*piBot)++;

		break;

	case STRETCH_SOUTH_EAST:
		if (((prTop->used + 4) > prTop->allocated) &&
		    (!ExtendRList (prTop, (unsigned int) RLIST_EXTENSION_SIZE)))
		{
		    return;
		}

		piTop = &(prTop->used);
		prect = &(prTop->prect[*piTop]);

		prect->x = x;			/* top inside */
		prect->y = y+cheight-swidth;
		prect->width = cwidth-swidth+1;
		prect->height = 1;
		prect++;
		(*piTop)++;

		prect->x = x+cwidth-swidth;	/* left inside */
		prect->y = y;
		prect->width = 1;
		prect->height = cheight-swidth;
		prect++;
		(*piTop)++;

		prect->x = x+cwidth-swidth+1;	/* top end */
		prect->y = y;
		prect->width = swidth-2;
		prect->height = 1;
		prect++;
		(*piTop)++;

		prect->x = x;			/* left end */
		prect->y = y+cheight-swidth+1;
		prect->width = 1;
		prect->height = swidth-2;
		(*piTop)++;

		if (((prBot->used + 4) > prBot->allocated) &&
		    (!ExtendRList (prBot, (unsigned int) RLIST_EXTENSION_SIZE)))
		{
		    return;
		}

		piBot = &(prBot->used);
		prect = &(prBot->prect[*piBot]); /* bottom shadow parts */


		prect->x = x+1;			/* bottom - row 1 */
		prect->y = y+cheight-2;
		prect->width = cwidth-1;
		prect->height = 1;
		prect++;
		(*piBot)++;

		prect->x = x;			/* bottom - row 2 */
		prect->y = y+cheight-1;
		prect->width = cwidth;
		prect->height = 1;
		prect++;
		(*piBot)++;

		prect->x = x+cwidth-2;		/* right  - column 1 */
		prect->y = y+1;
		prect->width = 1;
		prect->height = cheight-3;
		prect++;
		(*piBot)++;

		prect->x = x+cwidth-1;		/* right  - column 2 */
		prect->y = y;
		prect->width = 1;
		prect->height = cheight-2;
		(*piBot)++;

		break;

	case STRETCH_SOUTH_WEST:
		if (((prTop->used + 4) > prTop->allocated) &&
		    (!ExtendRList (prTop, (unsigned int) RLIST_EXTENSION_SIZE)))
		{
		    return;
		}

		piTop = &(prTop->used);
		prect = &(prTop->prect[*piTop]);

		prect->x = x;			/* top end */
		prect->y = y;
		prect->width = swidth;
		prect->height = 1;
		prect++;
		(*piTop)++;

		prect->x = x;			/* left (col 1) */
		prect->y = y+1;
		prect->width = 1;
		prect->height = cheight-1;
		prect++;
		(*piTop)++;

		prect->x = x+1;			/* left (col 2) */
		prect->y = y+1;
		prect->width = 1;
		prect->height = cheight-2;
		prect++;
		(*piTop)++;

		prect->x = x+swidth;		/* top inside (row 2) */
		prect->y = y+cheight-swidth;
		prect->width = cwidth-swidth;
		prect->height = 1;
		(*piTop)++;

		if (((prBot->used + 4) > prBot->allocated) &&
		    (!ExtendRList (prBot, (unsigned int) RLIST_EXTENSION_SIZE)))
		{
		    return;
		}

		piBot = &(prBot->used);
		prect = &(prBot->prect[*piBot]); /* bottom shadow parts */


		prect->x = x+swidth-1;		/* right inside (col 2) */
		prect->y = y+1;
		prect->width = 1;
		prect->height = cheight-swidth;
		prect++;
		(*piBot)++;

		prect->x = x+cwidth-1;		/* right end */
		prect->y = y+cheight-swidth+1;
		prect->width = 1;
		prect->height = swidth-1;
		prect++;
		(*piBot)++;

		prect->x = x+2;			/* bottom (row 1) */
		prect->y = y+cheight-2;
		prect->width = cwidth-3;
		prect->height = 1;
		prect++;
		(*piBot)++;

		prect->x = x+1;			/* bottom (row 2) */
		prect->y = y+cheight-1;
		prect->width = cwidth-2;
		prect->height = 1;
		(*piBot)++;

		break;
    }
} /* END OF FUNCTION StretcherCorner */



/*************************************<->*************************************
 *
 *  DrawStringInBox (dpy, win, gc, pbox, str)
 *
 *
 *  Description:
 *  -----------
 *  Draws a null-terminated string inside the specified box (rectangle)
 *
 *
 *  Inputs:
 *  ------
 *  dpy		- ptr to Display
 *  win		- an X Window
 *  gc 		- graphics context to use
 *  pfs		- pointer to XFontStruct for the font in "gc"
 *  pbox	- ptr to XRectangle that encloses text
 *  str		- String to write
 * 
 *  Outputs:
 *  -------
 *  none
 *
 *  Comments:
 *  --------
 *  o Assumes 8-bit text for now.
 *  o Algorithm:  
 *			get length of String
 *			if String is short than box width then
 *			    draw string centered in box
 *			otherwise
 *			    draw string left justified and clipped to box
 *  o The clip_x_origin, clip_y_origin, and clip_mask are reset to None
 *    upon exit.
 *  o Due to bugs and / or misunderstanding, I gave up on trying to 
 *    extract the XFontStruct from the GC. I just made it a separate 
 *    parameter.
 *			
 *************************************<->***********************************/
#ifdef _NO_PROTO
void DrawStringInBox (dpy, win, gc, pfs, pbox, str)

    Display *dpy;
    Window win;
    GC gc;
    XFontStruct *pfs;
    XRectangle *pbox;
    String str;
#else /* _NO_PROTO */
void DrawStringInBox (Display *dpy, Window win, GC gc, XFontStruct *pfs, XRectangle *pbox, String str)
#endif /* _NO_PROTO */
{
    XGCValues gcv;
    int textWidth;
    int xCenter;
    XRectangle clipBox;

    /* compute text position */
    textWidth = XTextWidth(pfs, str, strlen(str));

    if (textWidth < pbox->width) {	/* center text if there's room */
	xCenter = pbox->x + (pbox->width - textWidth) / 2 ;
	WmDrawString(dpy, win, gc, xCenter, (pbox->y + pfs->ascent), 
		    str, strlen(str));
    }
    else {				/* left justify & clip text */
	clipBox.x = 0;			/* set up clip rectangle */
	clipBox.y = 0;
	clipBox.width = pbox->width;
	clipBox.height = pbox->height;

	XSetClipRectangles (dpy, gc, pbox->x, pbox->y,	/* put into gc */
			    &clipBox, 1, Unsorted);

	WmDrawString(dpy, win, gc, pbox->x, (pbox->y + pfs->ascent), 
		    str, strlen(str));

	gcv.clip_x_origin = 0;		/* erase clip_mask from gc */
	gcv.clip_y_origin = 0;
	gcv.clip_mask = None;
	XChangeGC (dpy, gc, 
		   GCClipXOrigin | GCClipYOrigin | GCClipMask, &gcv);
    }
} /* END OF FUNCTION DrawStringInBox */




/*************************************<->*************************************
 *
 *  ExtendRList (prl, amt)
 *
 *
 *  Description:
 *  -----------
 *  Extends the size of the RList
 *
 *
 *  Inputs:
 *  ------
 *  prl		- ptr to Display
 *  amt		- how much to extend it by
 * 
 *  Outputs:
 *  -------
 *  Returns True if succeeded, false otherwise.
 *
 *  Comments:
 *  --------
 *			
 *************************************<->***********************************/
#ifdef _NO_PROTO
Boolean ExtendRList (prl, amt)

    RList *prl;
    unsigned int amt;
#else /* _NO_PROTO */
Boolean ExtendRList (RList *prl, unsigned int amt)
#endif /* _NO_PROTO */
{
    unsigned int total, count;
    XRectangle *pNewRect;
    Boolean rval;


    total = prl->allocated + amt;
    if ( (pNewRect = (XRectangle *) XtMalloc (total * sizeof(XRectangle))) 
	 == NULL)
    {
	Warning ("Insufficient memory for graphics data");
	rval = False;
    }
    else 
    {
	prl->allocated = total;
	rval = True;

	if (prl->used != 0) {		/* copy from old structure */
	    count = prl->used * sizeof(XRectangle);
	    (void) memcpy ((void *)pNewRect, (void *)prl->prect, count);
	    if (prl->prect != NULL)
		XtFree ((char *)prl->prect);
	    prl->prect = pNewRect;
	}
    }
    return (rval);
} /* END OF FUNCTION ExtendRList */



/*************************************<->*************************************
 *
 *  AllocateRList (amt)
 *
 *
 *  Description:
 *  -----------
 *  Allocates an RList of size "amt"
 *
 *
 *  Inputs:
 *  ------
 *  amt		- number of XRectangles to allocate in list
 * 
 *  Outputs:
 *  -------
 *  Returns ptr to new RList structure if success, returns NULL ptr otherwise
 *
 *  Comments:
 *  --------
 *			
 *************************************<->***********************************/
RList *AllocateRList (amt)

    unsigned int amt;
{
    RList *prl;


    if ((prl = (RList *) XtMalloc (sizeof (RList))) != NULL) 
    {
	if ( (prl->prect = (XRectangle *) XtMalloc (amt * sizeof(XRectangle))) 
	     == NULL)
	{
	    XtFree ((char *)prl);
	    prl = NULL;
	}
	else 
	{
	    prl->allocated = amt;
	    prl->used = 0;

	}
    }

    return (prl);
} /* END OF FUNCTION AllocateRList */



/*************************************<->*************************************
 *
 *  FreeRList (prl)
 *
 *
 *  Description:
 *  -----------
 *  Frees an RList
 *
 *
 *  Inputs:
 *  ------
 *  prl		- ptr to RList to free
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *			
 *************************************<->***********************************/
#ifdef _NO_PROTO
void FreeRList (prl)

    RList *prl;
#else /* _NO_PROTO */
void FreeRList (RList *prl)
#endif /* _NO_PROTO */
{
    if (prl) 
    {
	if (prl->prect) 
	    XtFree ((char *)prl->prect);

	XtFree ((char *)prl);
    }
}/* END OF FUNCTION FreeRList */


/*************************************<->*************************************
 *
 *  WmDrawString
 *
 *
 *  Description:
 *  -----------
 *  Draws a string
 *
 *
 *  Inputs:
 *  ------
 *  (same parameters used by XDrawString and XDrawImageString)
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *  o If wmGD.cleanText is True, then the text is drawn using 
 *    XDrawImageString. This provides some clean space around the text
 *    if the background area is stippled -- especially useful on 
 *    B/W displays.
 *			
 *************************************<->***********************************/
#ifdef _NO_PROTO
void WmDrawString (dpy, d, gc, x, y, string, length)

    Display *dpy;
    Drawable d;
    GC gc;
    int x, y;
    char *string;
    unsigned int length;
#else /* _NO_PROTO */
void WmDrawString (Display *dpy, Drawable d, GC gc, int x, int y, char *string, unsigned int length)
#endif /* _NO_PROTO */
{
    if (ACTIVE_PSD->cleanText)
    {
	XDrawImageString(dpy, d, gc, x, y, string, length);
    }
    else
    {
	XDrawString(dpy, d, gc, x, y, string, length);     
    }

}/* END OF FUNCTION WmDrawString */



/*************************************<->*************************************
 *
 *  WmXmDrawString
 *
 *
 *  Description:
 *  -----------
 *  Draws a string
 *
 *
 *  Inputs:
 *  ------
 *  (subset of parameters used by XmStringDraw and XmStringDrawImage)
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *  o If wmGD.cleanText is True, then the text is drawn using 
 *    XmStringDrawImage. This provides some clean space around the text
 *    if the background area is stippled -- especially useful on 
 *    B/W displays.
 *			
 *************************************<->***********************************/
#ifdef _NO_PROTO
void WmDrawXmString (dpy, w, xmfontlist, xmstring, gc, x, y, width, pbox)
    Display *dpy;
    Window w;
    XmFontList xmfontlist;
    XmString   xmstring;
    GC gc;
    Position x, y;
    Dimension width;
    XRectangle *pbox;
#else /* _NO_PROTO */
void WmDrawXmString (Display *dpy, Window w, XmFontList xmfontlist, 
		     XmString xmstring, GC gc, Position x, Position y, 
		     Dimension width,  XRectangle *pbox)
#endif /* _NO_PROTO */
{
    Dimension textWidth;
    int alignment = XmALIGNMENT_BEGINNING;
#ifdef DEC_MOTIF_EXTENSION
XmString title, xmblank;
XmStringContext		    context = NULL;
char			    *text;
XmStringCharSet		    charset;
XmStringDirection	    dir;
XmStringComponentType       tag;
short                       len;
char                        *val;
#endif
    

    textWidth = XmStringWidth(xmfontlist, xmstring);

    if (textWidth < pbox->width) {      /* center text if there's room */
	alignment = XmALIGNMENT_CENTER;
    }
    else 
    {                              /* left justify & clip text */
	alignment = XmALIGNMENT_BEGINNING;
    }
    
    if (ACTIVE_PSD->cleanText)
    {
#ifdef DEC_MOTIF_EXTENSION
	XmStringInitContext( &context, xmstring );
	XmStringGetNextComponent( context, &text, &charset, &dir,
				  &tag, &len, &val );
	xmblank = XmStringSegmentCreate( " ", XmSTRING_DEFAULT_CHARSET, 
					 dir, 0 );
	XmStringFreeContext( context );

        title = XmStringConcat( xmblank, xmstring );
        title = XmStringConcat( title, xmblank );
	XmStringDrawImage(dpy, w, xmfontlist, title, gc, x, y, width, 
			  alignment, XmSTRING_DIRECTION_L_TO_R, 
			  pbox);
        XmStringFree( xmblank );
        XmStringFree( title );
#else
	XmStringDrawImage(dpy, w, xmfontlist, xmstring, gc, x, y, width, 
			  alignment, XmSTRING_DIRECTION_L_TO_R, 
     			  pbox);
#endif /* DEC_MOTIF_EXTENSION */
    }
    else
    {
	XmStringDraw (dpy, w, xmfontlist, xmstring, gc, x, y, width, 
		      alignment, XmSTRING_DIRECTION_L_TO_R, pbox);
    }
} /* END OF FUNCTION WmDrawXmString */

