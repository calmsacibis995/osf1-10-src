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
static char SccsId[] = "@(#)gedrawarcnu.c	1.2\t12/27/89";
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
**	GEDRAWARCNU			Draws an arc
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
**	GNE 04/08/88 Created                            
**
**--
**/

#include "geGks.h"
#include <math.h>

geDrawArcNu(w, gca, xc, yc, r, alpha, beta, lw, lp, pixcol,
	    linstyle, linht, linmode,
	    filstyle, filcol, filht, filmode, x1, y1, x2, y2)

Window  	w;
GC		gca;
float		xc, yc, r;
int		alpha, beta, lw, lp;
unsigned long 	pixcol;
int		linstyle;
short   	linht;
int     	linmode, filstyle;
unsigned long	filcol;
short		filht;
int		filmode;
long    	x1, y1, x2, y2;
{
short	        dir;
int	        ix, iy;
unsigned int	side;
float	        fx, fy;

if (r <= 1.0 || r >= (float)(GEMAXUSHORT >> 1) || abs(beta >> 6) < 2)
  {if (gca == geGC5)
     XDrawLine(geDispDev, w, gca, (int)x1, (int)y1, (int)x2, (int)y2);
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
      XDrawLine(geDispDev, w, gca, (int)x1, (int)y1, (int)x2, (int)y2);
     }
   else
   if (filstyle != GETRANSPARENT)
     {GEFILSTYLE(geGC3, filstyle);
      GEFORG(geGC3, filcol);
      GEMODE(geGC3, filmode);
      GESTIPPLE(geGC3, filht);
      XDrawLine(geDispDev, w, geGC3, (int)x1, (int)y1, (int)x2, (int)y2);
     }
   return;
  }

fx = xc - r;	fy = yc - r;
r *= 2.0;					/* Make it DIAMETER now	     */

GEINT(fx, ix);	GEINT(fy, iy);		GEINT(r, side);

if (gca == geGC5)
  {XDrawArc(geDispDev, w, gca, ix, iy, side, side, alpha, beta);
   return;
  }

if (filstyle != GETRANSPARENT)
   {GEFILSTYLE_FOS(geGC3, filstyle, filcol);
    GEFORG(geGC3, filcol);
    GEMODE(geGC3, filmode);
    GESTIPPLE(geGC3, filht);
    XFillArc(geDispDev, w, geGC3, ix, iy, side, side, alpha, beta);
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
    XDrawArc(geDispDev, w, gca, ix, iy, side, side, alpha, beta);
   }
}
