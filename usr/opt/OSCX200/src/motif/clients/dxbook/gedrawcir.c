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
static char SccsId[] = "@(#)gedrawcir.c	1.3\t5/9/89";
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
**	GEDRAWCIR			Draws a circle with a spline
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


geDrawCir(w, gca, xc, yc, r, lw, lp, pixcol,
	  linstyle, linht, linmode,
	  filstyle, filcol, filht, filmode)
Window  	w;
GC		gca;
long		xc, yc, r;
int		lw, lp;
unsigned long	pixcol;
int		linstyle;
short   	linht;
int     	linmode, filstyle;
unsigned long	filcol;
short		filht;
int		filmode;
{
/*
 * Radius limit is 1
 */
if (r < 1)
   {XDrawPoint(geDispDev, w, geGC5, xc, yc);
    return;
   }

if (gca == geGC5)
   {XDrawArc(geDispDev, w, geGC5, xc - r, yc - r, r << 1, r << 1,
	     0, 360 << 6);
    return;
   }

if (filstyle != GETRANSPARENT)
   {GEFILSTYLE_FOS(geGC3, filstyle, filcol);
    GEFORG(geGC3, filcol);
    GEMODE(geGC3, filmode);
    GESTIPPLE(geGC3, filht);
    XFillArc(geDispDev, w, geGC3, (xc - r), (yc - r), (r << 1), (r << 1), 0,
	     (360 << 6));
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
   XDrawArc  (geDispDev, w, gca, xc - r, yc - r, r << 1, r << 1, 0, 360 << 6);
  }
}
                                     
