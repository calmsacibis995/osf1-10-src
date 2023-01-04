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
static char SccsId[] = "@(#)gedrawelp.c	1.2\t10/27/89";
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
**	GEDRAWELP			Draws an ellipse with vectors or
**					just steps around the edges of the
**					generating a list of coordinates.
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
**	GNE 01/08/87 Created                            
**
**--
**/

#include "geGks.h"


geDrawElp(w, gca, cx,cy, a,b, gamma, lw, lp, pixcol,
	  linstyle, linht, linmode,
	  filstyle, filcol, filht, filmode, cmd)

Window  	w;
GC		gca;
float   	cx,cy, a,b, gamma;
int		lw, lp;
unsigned long	pixcol;
int		linstyle;
short   	linht;
int     	linmode, filstyle;
unsigned long	filcol;
short		filht;
int		filmode;
long		cmd;
{
static XPoint	v[361];
register int	i, j, theta, inc;
static float	ft, ftx, fty, fa, fb, ftsin, ftcos;
int		itx, ity;
/*
 * Compute angle increment for going around the ellipse to generate the line
 *segments.  The angle increments must be such that they -
 *	1 - divide evenly into 360
 *	2 - large enough so that there is the minimum of overlap between
 *	    successive segments, yet small enough so that there are sufficient
 *	    number of segments to relatively smoothly circumnavigate the object
 * Below is a table of selected angle increments which satisfy the 1st
 *condition, followed by the corresponding # of segments that the increment
 *will engender.
 *
 * ANGLE INC	90  36  20     18  12  10       9       6   5   4      3   2   1
 * # SEGS	 4  10  18     20  30  36      40      60  72  90    120 180 360
 *Basically this is how it works - take 1/2 of the minor axis half and using it
 *as the # of segments conduct a binary search of the above table to locate
 *the corresponding angle increment which will be used in the loop below.
 *The search is organized so that angle increments for smaller 
 *ellipses will be located MORE quickly than those for larger ones.  The
 *selection is skewed toward the MIDDLE - angle increments of 90 and 360
 *should only very rarely be selected.
 */

if (a < 1. && b < 1.)
  {if (cmd == GEADDVERT || cmd == GESELPT) return;

   GEINT(cx, i);
   GEINT(cy, j);
   if (gca == geGC5)
     XDrawPoint(geDispDev, w, gca, i, j);
   else
   if (linstyle != GETRANSPARENT)
     {if (linmode == GXxor &&
	  (pixcol == geBlack.pixel || pixcol == GEXORPIXEL))
	{GEL_FG_P_M_PM(gca, lw, lp, GEXORPIXEL, linmode,
		       geXWPixel ^ geXBPixel);}
      else
	{GEL_FG_P_M_PM(gca, lw, lp, pixcol, linmode, AllPlanes);}
      if (linht == 100)
	{GEFILSTYLE(gca, FillSolid);}
      else
	{GEFILSTYLE(gca, linstyle);
         GESTIPPLE(gca, linht);
        }
      XDrawPoint(geDispDev, w, gca, i, j);
     }
   else
   if (filstyle != GETRANSPARENT)
     {GEFILSTYLE_FOS(geGC3, filstyle, filcol);
      GEFORG(geGC3, filcol);
      GEMODE(geGC3, filmode);
      GESTIPPLE(geGC3, filht);
      XFillPolygon(geDispDev, w, geGC3, v, j, Complex, CoordModeOrigin);
      XDrawPoint(geDispDev, w, gca, i, j);
     }
   return;
  }

inc = (min((int)a, (int)b));
if (inc <= 50)
   {if (inc <=  19)
	{if (inc <=   2) inc = 90;
	 else
	 if (inc <=  14) inc = 36;
	 else		 inc = 20;
	}
    else
    if (inc <=  40)
	{if (inc <=  26) inc = 18;
	 else
	 if (inc <=  34) inc = 12;
	 else		 inc = 10;
	}
    else		 inc = 9;
  }
else
   {if (inc <= 118)
	{if (inc <=  70) inc = 6;
	 else
	 if (inc <=  88) inc = 5;
	 else		 inc = 4;
	}
    else
	{if (inc <= 170) inc = 3;
	 else
	 if (inc <= 320) inc = 2;
	 else		 inc = 1;
	}
   }

theta = j = 0;

if (gamma)
for (i = 0; i < 360; i += inc)
   {ftx	      = ft = a * geCos[theta];
    fty       = b * geSin[theta];

    GEFSIN(gamma);
    ftsin     = geFT;
    GEFCOS(gamma);
    ftcos     = geFT;
    ftx       = cx + ft * ftcos - fty * ftsin;
    fty       = cy - ft * ftsin - fty * ftcos;
    if (cmd == GEADDVERT || cmd == GESELPT)
	{GEINT(ftx, itx);
  	 GEINT(fty, ity);
	 GESTOREVERT(geVn, itx, ity);
	 geVn++;
	}
    else
	{GEINT(ftx, v[j].x);
  	 GEINT(fty, v[j].y);
	}
    theta    += inc; j++;
   }
else
for (i = 0; i < 360; i += inc)
   {ft        = cx + a * geCos[theta];
    if (cmd == GEADDVERT || cmd == GESELPT)
	{GEINT(ft, itx);}
    else
    	{GEINT(ft, v[j].x);}
    ft        = cy - b * geSin[theta];
    if (cmd == GEADDVERT || cmd == GESELPT)
	{GEINT(ft, ity);
	 GESTOREVERT(geVn, itx, ity);
	 geVn++;
	}
    else
    	{GEINT(ft, v[j].y);}
    theta    += inc; j++;
   }


if (cmd == GEADDVERT || cmd == GESELPT)
    {geVert[geVn++] = geVert[0];
     return;
    }
else
    v[j++] = v[0];

if (gca == geGC5)
   {XDrawLines(geDispDev, w, geGC5, v, j, CoordModeOrigin);
    return;
   }

if (filstyle != GETRANSPARENT)
   {GEFILSTYLE_FOS(geGC3, filstyle, filcol);
    GEFORG(geGC3, filcol);
    GEMODE(geGC3, filmode);
    GESTIPPLE(geGC3, filht);
    XFillPolygon(geDispDev, w, geGC3, v, j, Complex, CoordModeOrigin);
   }

if (linstyle != GETRANSPARENT)
  {if (linmode == GXxor && (pixcol == geBlack.pixel || pixcol == GEXORPIXEL))
      {GEL_FG_P_M_PM(gca, lw, lp, GEXORPIXEL, linmode, geXWPixel ^ geXBPixel);}
    else
      {GEL_FG_P_M_PM(gca, lw, lp, pixcol, linmode, AllPlanes);}
    if (linht == 100)
      {GEFILSTYLE(gca, FillSolid);}
    else
      {GEFILSTYLE(gca, linstyle);
       GESTIPPLE(gca, linht);
      }
    XDrawLines(geDispDev, w, gca, v, j, CoordModeOrigin);
   }
}
