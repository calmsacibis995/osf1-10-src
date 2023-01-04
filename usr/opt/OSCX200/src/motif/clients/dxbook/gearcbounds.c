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
static char SccsId[] = "@(#)gearcbounds.c	1.1\t11/22/88";
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
**	GEARCBOUNDS			Determines minimal box encompassing an arc
**
**  ABSTRACT:
**
** Basically this is how it works:
** The two end points and the calculated middle point of the arc are used
** as a first best approximation.  Then the path of the arc is checked to
** determine whether or not it passes through any of the axis (of a coord
** system whose origin is at the center of the circle of which the arc
** is a fraction).  If so, then the appropriate side of the bounding box
** is extended to the intersection of the arc with the axis.
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

geArcBounds(x1,y1, x2,y2, x3,y3, xc, yc, r, lw, xret1, yret1, xret2, yret2)
long	x1, y1, x2, y2, x3, y3;
float	xc, yc, r;
long	lw;
long    *xret1, *yret1, *xret2, *yret2;
{

#define GEX_0      {*xret2 = (int)((xc + r) * GEFSUIPP);}
#define GEX_90     {*yret1 = (int)((yc - r) * GEFSUIPP);}
#define GEX_180    {*xret1 = (int)((xc - r) * GEFSUIPP);}
#define GEX_270    {*yret2 = (int)((yc + r) * GEFSUIPP);}

short	dir;
int	icx, icy, ialpha, ibeta, q1, q3;
unsigned int	side;
float	fx, fy, alpha, beta, gamma;

lw >>= 1;

if (r <= 1.0)
   {*xret1 = min(x1, min(x2, x3)) - lw;
    *yret1 = min(y1, min(y2, y3)) - lw;
    *xret2 = max(x1, max(x2, x3)) + lw;
    *yret2 = max(y1, max(y2, y3)) + lw;
    return;
   }
/*
 * Compute the angles
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

gamma   = alpha + beta / 2.;
x2      = (int)((xc + r * GECOSD(gamma)) * GEFSUIPP);
y2      = (int)((yc - r * GESIND(gamma)) * GEFSUIPP);

*xret1  = min(x1, min(x2, x3));
*yret1  = min(y1, min(y2, y3));
*xret2  = max(x1, max(x2, x3));
*yret2  = max(y1, max(y2, y3));

q1      = geGenQuad(x1, y1, xc, yc);
q3      = geGenQuad(x3, y3, xc, yc);

if (q1 == q3) goto arcbounds_done;
/*
 * The arc is crossing one or more axis (of the coord system whose origin
 *is at the center of the circle defining the radius of curvature) so adjust
 *the extents of the bounding box accordingly.
 */
if (dir == GECLOCK)
  {switch (q1)
     {case 1:
       switch (q3)
	 {case 2: GEX_0; GEX_270; GEX_180; break;
	  case 3: GEX_0; GEX_270; break;
	  case 4: GEX_0; break;
	 }
       break;
      case 2:
       switch (q3)
	 {case 1: GEX_90; break;
	  case 3: GEX_90; GEX_0; GEX_270; break;
	  case 4: GEX_90; GEX_0; break;
	 }
       break;
      case 3:
       switch (q3)
	 {case 1: GEX_180; GEX_90; break;
	  case 2: GEX_180; break;
	  case 4: GEX_180; GEX_90; GEX_0; break;
	 }
       break;
      case 4:
       switch (q3)
	 {case 1: GEX_270; GEX_180; GEX_90; break;
	  case 2: GEX_270; GEX_180; break;
	  case 3: GEX_270; break;
	 }
       break;
     }
  }
else
  {switch (q1)
     {case 1:
       switch (q3)
	 {case 2: GEX_90; break;
	  case 3: GEX_90; GEX_180; break;
	  case 4: GEX_90; GEX_180; GEX_270; break;
	 }
       break;
      case 2:
       switch (q3)
	 {case 1: GEX_180; GEX_270; GEX_0; break;
	  case 3: GEX_180; break;
	  case 4: GEX_180; GEX_270; break;
	 }
       break;
      case 3:
       switch (q3)
	 {case 1: GEX_270; GEX_0; break;
	  case 2: GEX_270; GEX_0; GEX_90; break;
	  case 4: GEX_270; break;
	 }
       break;
      case 4:
       switch (q3)
	 {case 1: GEX_0; break;
	  case 2: GEX_0; GEX_90; break;
	  case 3: GEX_0; GEX_90; GEX_180; break;
	 }
       break;
     }
  }

arcbounds_done:
*xret1 -= lw;
*yret1 -= lw;
*xret2 += lw + GESUI1;
*yret2 += lw + GESUI1;

}
