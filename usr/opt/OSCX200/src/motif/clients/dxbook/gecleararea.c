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
static char SccsId[] = "@(#)gecleararea.c	1.1\t11/22/88";
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
**	GECLEARAREA            	       Clears a region of the drawing window
**
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	GNE 05/20/88 Created
**
**--
**/

#include "geGks.h"

geClearArea(x, y, w, h)
int          x, y;
int 	     w, h;

{                       
int		xt, yt;
unsigned int	width, height, bwidth, depth;
struct GE_SEG   *ASegP;

Window		rootw;

if (!w || !h || !geState.APagP || !geState.APagP->Surf.self) return;

if (w == -1 || h == -1)
  {if (geState.Drawable == geState.Pixmap)
     {geGrf(GEBOUNDS, geSeg0);
      GEVEC(GEEXTENT, geSeg0);
      width          = GETPIX(geClip.x2 - geClip.x1);
      height         = GETPIX(geClip.y2 - geClip.y1);
     }
   else
     {XGetGeometry(geDispDev, geState.APagP->Surf.self, &rootw, &xt, &yt,
		   &width, &height, &bwidth, &depth);
     }
  }

if (w != -1) width  = (unsigned int)w;
if (h != -1) height = (unsigned int)h;

if ((geSeg0 && geSeg0->FillStyle != GETRANSPARENT) ||
    geState.Drawable == geState.Pixmap)
  {if (geSeg0 && geSeg0->FillStyle != GETRANSPARENT)
       {ASegP = geSeg0;
	GE_SET_BG_DISP_ATTR_PAG;
	GEFILSTYLE_FOS(geGC3, geDispAttr.BgStyle, geDispAttr.BgPixel);
	GE_FG_M(geGC3, geDispAttr.BgPixel, geSeg0->FillWritingMode);
	GESTIPPLE(geGC3, geDispAttr.BgHT);
	XFillRectangle(geDispDev, geState.Drawable, geGC3, x, y,
		       width, height);     
       }
   else
       {GEFILSTYLE_FOS(geGC3, FillSolid, geWhite.pixel);
	GE_FG_M(geGC3, geWhite.pixel, GXcopy);
	XFillRectangle(geDispDev, geState.Drawable, geGC3, x, y,
		       width, height);
       }
  }
else
  XClearArea(geDispDev, geState.Drawable, x, y,
	     width, height, FALSE);
}
