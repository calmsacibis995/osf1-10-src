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
static char SccsId[] = "@(#)gegendir.c	1.1\t11/22/88";
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
**	GEGENDIR	             	       Determine whether arc should be C/CC-wise
**
**  ABSTRACT:
**
** Given three points, what we want to do is to determine whether it is a
** clock-wise or counter-clock-wise motion to move FROM pt1 TO pt3 THROUGH ptm
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


geGenDir(x1,y1, xm,ym, x3,y3, fxc,fyc)
long		x1,y1, xm,ym, x3,y3;
float		fxc,fyc;
{                       
static short	qp1, qpm, qp3;

qp1 = geGenQuad(x1, y1, fxc, fyc);		/* Quadrants are numbered    */
qpm = geGenQuad(xm, ym, fxc, fyc);		/*C-CLOCKWISE from 1 to 4,   */
qp3 = geGenQuad(x3, y3, fxc, fyc);		/*with 1 being the UPPER RT  */
/*
 * All three points are in the SAME quadrant
 */
if (qp1 == qpm && qpm == qp3)
    switch (qp1)
	{case 1:
	   if (x1 <= xm && (xm < x3 || (xm == x3 && ym < y3))) return(GECLOCK);
 	   return(GECCLOCK);
	 case 2:
	   if (x1 <= xm && (xm < x3 || (xm == x3 && ym > y3))) return(GECLOCK);
 	   return(GECCLOCK);
	 case 3:
	   if (x1 >= xm && (xm > x3 || (xm == x3 && ym > y3))) return(GECLOCK);
 	   return(GECCLOCK);
	 case 4:
	   if (x1 >= xm && (xm > x3 || (xm == x3 && ym < y3))) return(GECLOCK);
 	   return(GECCLOCK);
	}
/*
 * Pt1 and ptm are in the same quadrant and the pt3 is in some other quadrant-
 *doesn't matter which.  Note that the arc CANNOT be the SHORT arc between
 *pt1 and ptm.
 */
if (qp1 == qpm)
    switch (qp1)
	{case 1:
	   if (x1 < xm || (x1 == xm && y1 < ym)) 	       return(GECLOCK);
 	   return(GECCLOCK);
	 case 2:
	   if (x1 < xm || (x1 == xm && y1 > ym)) 	       return(GECLOCK);
 	   return(GECCLOCK);
	 case 3:
	   if (x1 > xm || (x1 == xm && y1 > ym)) 	       return(GECLOCK);
 	   return(GECCLOCK);
	 case 4:
	   if (x1 > xm || (x1 == xm && y1 < ym)) 	       return(GECLOCK);
 	   return(GECCLOCK);
	}
/*
 * Pt1 && pt3 are in the same quadrant && ptm is in some other quadrant-
 *doesn't matter which.  Note that this && the above tests are precisely the
 *same, except that the results are reversed.
 */
if (qp1 == qp3)
    switch (qp1)
	{case 1:
	   if (x1 < x3 || (x1 == x3 && y1 < y3)) 	       return(GECCLOCK);
 	   return(GECLOCK);
	 case 2:
	   if (x1 < x3 || (x1 == x3 && y1 > y3)) 	       return(GECCLOCK);
 	   return(GECLOCK);
	 case 3:
	   if (x1 > x3 || (x1 == x3 && y1 > y3)) 	       return(GECCLOCK);
 	   return(GECLOCK);
	 case 4:
	   if (x1 > x3 || (x1 == x3 && y1 < y3)) 	       return(GECCLOCK);
 	   return(GECLOCK);
	}
/*
 * Pt3 && ptm are in the same quadrant an pt1 is in some other quadrant-
 *again, it doesn't matter which.
 */
if (qp3 == qpm)
    switch (qpm)
	{case 1:
	   if (xm < x3 || (xm == x3 && ym < y3)) 	       return(GECLOCK);
 	   return(GECCLOCK);
	 case 2:
	   if (xm < x3 || (xm == x3 && ym > y3)) 	       return(GECLOCK);
 	   return(GECCLOCK);
	 case 3:
	   if (xm > x3 || (xm == x3 && ym > y3)) 	       return(GECLOCK);
 	   return(GECCLOCK);
	 case 4:
	   if (xm > x3 || (xm == x3 && ym < y3)) 	       return(GECLOCK);
 	   return(GECCLOCK);
	}
/*
 * Finally, the simplest case, none of the points are in the same quadrant.
 */
switch (qp1)
	{case 1:
	   if (qpm == 4 || (qpm == 3 && qp3 == 2))	       return(GECLOCK);
 	   return(GECCLOCK);
	 case 2:
	   if (qpm == 1 || (qpm == 4 && qp3 == 3))	       return(GECLOCK);
 	   return(GECCLOCK);
	 case 3:
	   if (qpm == 2 || (qpm == 1 && qp3 == 4))	       return(GECLOCK);
 	   return(GECCLOCK);
	 case 4:
	   if (qpm == 3 || (qpm == 2 && qp3 == 1))	       return(GECLOCK);
 	   return(GECCLOCK);
	}
}
                                                 
