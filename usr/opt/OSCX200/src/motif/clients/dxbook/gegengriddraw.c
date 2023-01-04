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
static char SccsId[] = "@(#)gegengriddraw.c	1.2\t4/17/89";
/*
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1988 BY                            *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**			   ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
**++
**  FACILITY:
**
**	GEGENGRIDDRAW             	       Draws the grid
**
**  ABSTRACT:
**
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	GNE 04/28/87 Created
**
**--
**/

#include "geGks.h"


geGenGridDraw(clip)
short		clip;
{                       

#ifdef GERAGS

static long 	x, y, y1, xstrt, ystrt, xend, yend, xmajor_inc, ymajor_inc,
		xminor_inc, yminor_inc, xc, yc, lumi;

/*
 * If clip is enabled, find section of grid to be displayed.  Note that
 *"x and ystrt" are forced onto Major grid points - this makes the actual
 *display faster, because if this can be assumed then we don't have to con-
 *tinuously check whether or not a particular line is a Major grid line.
 */
clip = 0;
if (clip)
   {xstrt = geState.APagP->Grid.Xorg +
	    (geClip.x1 / geState.APagP->Grid.MajorX) *
	    geState.APagP->Grid.MajorX;
    xstrt = GETPIX(xstrt);
    ystrt = geState.APagP->Grid.Yorg +
	    (geClip.y1 / geState.APagP->Grid.MajorY) *
    	    geState.APagP->Grid.MajorY;
    ystrt = GETPIX(ystrt);
    xend  = geState.APagP->Grid.Xorg +
	    (geClip.x2 / geState.APagP->Grid.MinorX) *
	    geState.APagP->Grid.MinorX;
    if ((xend  - geClip.x2) > ((geState.APagP->Grid.MinorX) >> 1))
	xend  += geState.APagP->Grid.MinorX;
    xend  = GETPIX(xend);
    yend  = geState.APagP->Grid.Yorg +
	    (geClip.y2 / geState.APagP->Grid.MinorY) *
	    geState.APagP->Grid.MinorY;
    if ((yend  - geClip.y2) > ((geState.APagP->Grid.MinorY) >> 1))
	yend  += geState.APagP->Grid.MinorY;
    yend  = GETPIX(yend);

   }
else
   {xstrt = geState.APagP->Grid.Xorg;
    ystrt = geState.APagP->Grid.Yorg;
    xend  = GETSUI(geState.APagP->Surf.width);
    yend  = GETSUI(geState.APagP->Surf.height);
   }

GEMODE(geGC1, GXcopy);

if (geSeg0->FillStyle != GETRANSPARENT)
   {if ((lumi = (geSeg0->Col.RGB.red + geSeg0->Col.RGB.green +
		 geSeg0->Col.RGB.blue) / 3) <
        ((GEMAXUSHORT) >> 1))
	{GEFORG(geGC1, geWhite.pixel);}
    else
	{GEFORG(geGC1, geBlack.pixel);}
   }
else
   {GEFORG(geGC1, geBlack.pixel);}

xmajor_inc = geState.APagP->Grid.MajorX;
ymajor_inc = geState.APagP->Grid.MajorY;
xminor_inc = geState.APagP->Grid.MinorX;
yminor_inc = geState.APagP->Grid.MinorY;

xmajor_inc = xmajor_inc < 1 ? 1 : xmajor_inc;
ymajor_inc = ymajor_inc < 1 ? 1 : ymajor_inc;
xminor_inc = xminor_inc < 1 ? 1 : xminor_inc;
yminor_inc = yminor_inc < 1 ? 1 : yminor_inc;

for (y = ystrt; y <= yend; y += ymajor_inc)
  {yc = GETPIX(y);
   for (x = xstrt; x <= xend; x += xminor_inc)
	{xc = GETPIX(x);
     	 XDrawPoint(geDispDev, geState.APagP->Surf.self, geGC1, xc, yc);
	}
   for (y1 = y + yminor_inc; y1 < y + ymajor_inc; y1 += yminor_inc)
     	{yc = GETPIX(y1);
	 for (x = xstrt; x <= xend; x += xmajor_inc)
	    {xc = GETPIX(x);
     	     XDrawPoint(geDispDev, geState.APagP->Surf.self, geGC1, xc, yc);
	    }
        }
  }
/*
 * Draw the Horizontal axis

if (geU.YO >= 0 && geU.YO <= GETSUI(geState.APagP->Surf.height))
    XDrawLine(geDispDev, geState.APagP->Surf.self, geGC1,
	      GETPIX(xstrt), GETPIX(geU.YO), GETPIX(xend), GETPIX(geU.YO));
 */
/*
 * Draw the Vertical axis

if (geU.XO >= 0 && geU.XO <= GETSUI(geState.APagP->Surf.width))
    XDrawLine(geDispDev, geState.APagP->Surf.self, geGC1,
	      GETPIX(geU.XO), GETPIX(ystrt), GETPIX(geU.XO), GETPIX(yend));
 */

#endif

}
                                                 
