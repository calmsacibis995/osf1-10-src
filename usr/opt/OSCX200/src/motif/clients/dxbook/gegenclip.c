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
static char SccsId[] = "@(#)gegenclip.c	1.19\t10/16/89";
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
**	GEGENCLIP	             	       Set/Clears x clip region
**
**  ABSTRACT:
**
**        If the "set" input parameter is TRUE, this routine establishes
**	  geClip as the X clipping region.  Otherwise it resets the x clip
**	  region to be the window.
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	GNE 05/08/90 Created
**
**--
**/

#include "geGks.h"

geGenClip(Set)
int		Set;
{                       

XRectangle	 xrect[1];

if (Set)
   {xrect[0].x      = xrect[0].y 	= 0;
    xrect[0].width  = GETPIX(geClip.x2 - geClip.x1);
    xrect[0].height = GETPIX(geClip.y2 - geClip.y1);
    XSetClipRectangles(geDispDev, geGC2,
		       GETPIX(geClip.x1), GETPIX(geClip.y1),
		       xrect, 1, Unsorted);
    XSetClipRectangles(geDispDev, geGC3,
		       GETPIX(geClip.x1), GETPIX(geClip.y1),
		       xrect, 1, Unsorted);
    XSetClipRectangles(geDispDev, geGC4,
		       GETPIX(geClip.x1), GETPIX(geClip.y1),
		       xrect, 1, Unsorted);
    XSetClipRectangles(geDispDev, geGC5,
		       GETPIX(geClip.x1), GETPIX(geClip.y1),
		       xrect, 1, Unsorted);
    XSetClipRectangles(geDispDev, geGCImg,
		       GETPIX(geClip.x1), GETPIX(geClip.y1),
		       xrect, 1, Unsorted);
   }
else
   {XSetClipOrigin(geDispDev, geGC2,   0, 0);
    XSetClipOrigin(geDispDev, geGC3,   0, 0);
    XSetClipOrigin(geDispDev, geGC4,   0, 0);
    XSetClipOrigin(geDispDev, geGC5,   0, 0);
    XSetClipOrigin(geDispDev, geGCImg, 0, 0);
    XSetClipMask  (geDispDev, geGC2,   None);
    XSetClipMask  (geDispDev, geGC3,   None);
    XSetClipMask  (geDispDev, geGC4,   None);
    XSetClipMask  (geDispDev, geGC5,   None);
    XSetClipMask  (geDispDev, geGCImg, None);
   }

}

/*
 * Determines whether or not the given segment intersects the currently
 * set clip box contained in geClip.
 */
geGenInClip(ASegP)
struct GE_SEG *ASegP;

{
if (GETPIX(ASegP->Co.x1) > GETPIX(geClip.x2) ||
    GETPIX(ASegP->Co.y1) > GETPIX(geClip.y2) ||
    GETPIX(ASegP->Co.x2) < GETPIX(geClip.x1) ||
    GETPIX(ASegP->Co.y2) < GETPIX(geClip.y1))
  return(FALSE);
else
  return(TRUE);
}

/*
 * Merges the two given clipping regions, returning the maximized rectangle
 * in the first structure.
 */
geGenClipMerge(Clip1, Clip2)
struct GE_CO *Clip1, *Clip2;

{
Clip1->x1 = min(Clip1->x1, Clip2->x1);
Clip1->y1 = min(Clip1->y1, Clip2->y1);
Clip1->x2 = max(Clip1->x2, Clip2->x2);
Clip1->y2 = max(Clip1->y2, Clip2->y2);
}
