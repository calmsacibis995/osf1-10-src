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
static char SccsId[] = "@(#)gedrawarc.c	1.1\t11/22/88";
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
**	GEDRAWARC			Draws an arc
**
**  ABSTRACT:
**
** The arc is drawn FROM (x1,y1) TO (x3,y3) THRU (x2,y2) - which implies
** that it may proceed either CLOCKWISE or counter CLOCKWISE, depending on
** the relative positioning of the three points.
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	GNE 01/09/87 Created                            
**
**--
**/

#include "geGks.h"
#include <math.h>

extern	float	geGenAngle();

geDrawArc(w, gca, x1,y1, x2,y2, x3,y3, xc, yc, r, lw, lp, pixcol,
	  filstyle, filcol, filht, mode)

Window  w;
GC	gca;
long	x1, y1, x2, y2, x3, y3;
float	xc, yc, r;
int	lw, lp, pixcol, filstyle, filcol;
short	filht;
int	mode;
{
short	dir;
int	ix, iy, ialpha, ibeta;
unsigned int	side;
float	fx, fy, alpha, beta;

if (r <= 1.0)
   {XDrawPoint(geDispDev, w, geGC5, x1, y1);
    return;
   }
if ((x1 == x2 && y1 == y2) || (x1 == x3 && y1 == y3))
   {GEINT(xc, ix);	GEINT(yc, iy);
    if (lw && gca != geGC5)
	{if (mode == GXxor && pixcol == geBlack.pixel) pixcol = geWhite.pixel;
   	 GEL_FG_P_M(gca, lw, lp, pixcol, mode);
	}
    XDrawLine(geDispDev, w, gca, x1,y1, ix, iy);
   }
/*
 * Compute the angles for XDrawArc.
 */
alpha = geGenAngle(x1,y1, xc,yc, r);
beta  = geGenAngle(x3,y3, xc,yc, r);
dir   = geGenDir(x1,y1, x2,y2, x3,y3, xc,yc);
if (alpha > beta)
   {if (dir == GECLOCK) beta = -(alpha - beta);
    else                beta = 360.0 - alpha + beta;
   }
else                                   
   {if (dir == GECLOCK) beta = -(360.0 - beta + alpha);
    else                beta = beta - alpha;
   }                                        
fx = xc - r;	fy = yc - r;
r *= 2.0;					/* Make it DIAMETER now	     */

GEINT(fx, ix);	GEINT(fy, iy);		GEINT(r, side);
alpha *= 64.0;	beta *= 64.0;
GEINT(alpha, ialpha);			GEINT(beta, ibeta);

if (filstyle != GETRANSPARENT)
   {GEFILSTYLE(geGC3, filstyle);
    GEFORG(geGC3, filcol);
    GESTIPPLE(geGC3, filht);
    XFillArc(geDispDev, w, geGC3, ix, iy, side, side, ialpha, ibeta);
   }

if (lw)
   {if (gca != geGC5)
	{if (mode == GXxor && pixcol == geBlack.pixel) pixcol = geWhite.pixel;
   	 GEL_FG_P_M(gca, lw, lp, pixcol, mode);
	}
    XDrawArc(geDispDev, w, gca, ix, iy, side, side, ialpha, ibeta);
   }
}
