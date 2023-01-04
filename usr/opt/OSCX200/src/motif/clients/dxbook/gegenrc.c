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
static char SccsId[] = "@(#)gegenrc.c	1.2\t12/27/89";
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
**	GEGENRC	             	       Calcs c & rad of cir. defined by in pts.
**
**  ABSTRACT:
**
** Also computes the start and end angle from 3'oclock for the arc on the
** circle in units of 1/64 of a degree (negative indicates clockwise).
**
** Here's how it works.
**
** Given three points, what we want to do is to construct a circle which inter-
** sects all three points (i.e., calculate the center and radius of the circle).
** Let the three points be P(x1,y1), Q(x2,y2) and S(x3,y3).
**
**	1) Locate the mid points of segments PQ and QS, pts A and B,
**	   respectively.
**	2) The orthogonals to PQ and QS passing through A and B intersect
**	   at the center of the circle.
**	3) Knowing the slopes of PQ and QS yields the slopes of the
**	   orthogonals (radial vector bisecting a chord is othogonal to the
**	   the chord).
**	4) Thus there are two equations (the equations of the two radial
**	   vectors) in two unknowns (cx and cy), which may
**	   be readily solved.
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	GNE 06/23/87 Created
**
**--
**/

#include "geGks.h"
#include <math.h>

extern float geGenAngle();

geGenRC(x1,y1, x2,y2, x3,y3, cx,cy, r, alpha, beta)
long		x1,y1, x2,y2, x3,y3;
float		*cx,*cy, *r;
int             *alpha, *beta;
{                       
short	x_defd, y_defd;
int     dir, loc_alpha, loc_beta;
float   mac, mbc, xa,ya, xb,yb, x,y,
        fx1, fy1, fx2, fy2, fx3, fy3, fcx, fcy, fr;

fx1   = (float)x1;      fy1   = (float)y1;
fx2   = (float)x2;      fy2   = (float)y2;
fx3   = (float)x3;      fy3   = (float)y3;

/*
 * Check for straight line and coincidence of points
 */
if ((x1 == x2 && x1 == x3) || (y1 == y2 && y1 == y3) ||
    (x1 == x2 && y1 == y2) || (x1 == x3 && y1 == y3) ||
    (x3 == x2 && y3 == y2))
   {*r = *alpha = *beta = *cx = *cy = 0; return;}


x_defd = y_defd = 1;
/*
 * Check for horizontal and vertical cords
 */
if (y1 == y2)
   {fcx = (fx1 + fx2) / 2.0;
    x_defd = 1;
    mac = -(fx2 - fx3) / (fy2 - fy3);
    x = (fx2 + fx3) / 2.0;      y = (fy2 + fy3) / 2.0;
   }
else
if (y1 == y3)
   {fcx = (fx1 + fx3) / 2.0;
    mac = -(fx2 - fx3) / (fy2 - fy3);
    x = (fx2 + fx3) / 2.0;      y = (fy2 + fy3) / 2.0;
   }
else
if (y2 == y3)
   {fcx = (fx3 + fx2) / 2.0;
    mac = -(fx2 - fx1) / (fy2 - fy1);
    x = (fx2 + fx1) / 2.0;      y = (fy2 + fy1) / 2.0;
   }
else
    x_defd = 0;
if (x1 == x2)
   {fcy = (fy1 + fy2) / 2.0;
    if (!x_defd)
    	{mac = (fx2 - fx3) / (fy2 - fy3);
    	 x = (fx2 + fx3) / 2.0;      y = (fy2 + fy3) / 2.0;
	}
   }
else
if (x1 == x3)
   {fcy = (fy1 + fy3) / 2.0;
    if (!x_defd)
    	{mac = (fx2 - fx3) / (fy2 - fy3);
    	 x = (fx2 + fx3) / 2.0;      y = (fy2 + fy3) / 2.0;
	}
   }
else
if (x2 == x3)
   {fcy = (fy3 + fy2) / 2.0;
    if (!x_defd)
    	{mac = (fx2 - fx1) / (fy2 - fy1);
    	 x = (fx2 + fx1) / 2.0;      y = (fy2 + fy1) / 2.0;
	}
   }
else
    y_defd = 0;

if (x_defd && !y_defd) fcy = mac * (fcx - x) + y;
else
if (y_defd && !x_defd) fcx = x - (fcy - y) / mac;
else
if (!x_defd && !y_defd)
   {mac = (fx1 - fx2) / (fy2 - fy1);
    mbc = (fx2 - fx3) / (fy3 - fy2);
    if (mac == mbc)
   {*r = 0; *alpha = *beta = 0; return;}

    xa  = (fx2 + fx1) / 2.0;
    ya  = (fy2 + fy1) / 2.0;
    xb  = (fx2 + fx3) / 2.0;
    yb  = (fy2 + fy3) / 2.0;
    fcx = (mac * xa - ya + yb - mbc * xb) / (mac - mbc);
    fcy = mac * (fcx - xa) + ya;
   }

x   = fcx - fx1;
y   = fcy - fy1;
x   = (float)sqrt((double)(x * x) + (double)(y * y));
fr  = x;
/*
 * Compute start angle (alpha) and extent of arc (beta) on the circle.
 */

*cx = fcx / GEFSUIPP;
*cy = fcy / GEFSUIPP;
*r  = fr  / GEFSUIPP;
xa  = geGenAngle(x1,y1, *cx, *cy, *r);
xb  = geGenAngle(x3,y3, *cx, *cy, *r);
dir = geGenDir(x1,y1, x2,y2, x3,y3, *cx, *cy);
if (xa > xb)
  {if (dir == GECLOCK) xb = -(xa - xb);
    else               xb = 360.0 - xa + xb;
  }
else
  {if (dir == GECLOCK) xb = -(360.0 - xb + xa);
    else               xb = xb - xa;
  }
xa *= 64.0; xb *= 64.0;
GEINT(xa, loc_alpha); GEINT(xb, loc_beta);
*alpha = loc_alpha; *beta = loc_beta;
}
                                                 
