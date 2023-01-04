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
static char SccsId[] = "@(#)gegenangle.c	1.1\t11/22/88";
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
**	GEGENANGLE	             	       Determine angle of x,y wrt xc,yc
**
**  ABSTRACT:
**
** The determined angle is that made by a line connecting (x,y) to (xc,yc),
** measured COUNTER-CLOCKWISE from the 3 o'clock position.
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	GNE 06/26/87 Created
**
**--
**/

#include "geGks.h"
#include <math.h>


float
geGenAngle(x,y, fxc,fyc, fr)
long		x,y;
float		fxc,fyc, fr;
{                       
int		quadrant;
long            xc,yc, r;
float		alpha, fx, fy, ftemp;
double		tf;

fx = (float)x / GEFSUIPP;    fy = (float)y / GEFSUIPP;
GEINT(fxc, xc);		GEINT(fyc, yc);		GEINT(fr, r);
/*
 * 1	Determine which quadrant the point lies in
 * 2	Find the COUNTER-CLOCKWISE angle within the quadrant
 * 3	Add in the quadrant's counter-clockwise offset from 3 o'clock.
 *Note that the 
 *normalization of the vertical coordinates wrt to the center
 *is inverted (e.g. yc-y1, rather than y1-yc) because in X the y axis
 *increases DOWNWARD.
 */

quadrant = geGenQuad(x, y, fxc, fyc);
switch (quadrant)
   {case 1:
	if (!(ftemp = fyc - fy)) 	alpha = 0.0;
	else
	if (ftemp >= fr)		alpha = 90.0;
	else
	  alpha = (float)asin((double)(fyc - fy) / (double)fr) * GE_RAD_TO_DEG;
	break;

    case 2:
	if (!(ftemp = fxc - fx)) 	alpha = 90;
	else
	if (ftemp >= fr)		alpha = 180;
	else
	  {alpha  = (float)asin((double)(fxc - fx) / (double)fr) * GE_RAD_TO_DEG;
	   alpha += 90.0;
	  }
	break;

    case 3:
	if (!(ftemp = fy - fyc)) 	alpha = 180.0;
	else
	if (ftemp >= fr)		alpha = 270.0;
	else
	  {alpha  = (float)asin((double)(fy - fyc) / (double)fr) * GE_RAD_TO_DEG;
	   alpha += 180.0;
	  }
	break;

    case 4:
	if (!(ftemp = fx - fxc)) 	alpha = 270.0;
	else
	if (ftemp >= fr)		alpha = 360.0;
	else
	  {alpha  = (float)asin((double)(fx - fxc) / (double)fr) * GE_RAD_TO_DEG;
	   alpha += 270;
	  }
	break;
   }
return(alpha);
}
                                                 
