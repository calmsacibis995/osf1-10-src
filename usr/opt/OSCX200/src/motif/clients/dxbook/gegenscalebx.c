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
static char SccsId[] = "@(#)gegenscalebx.c	1.1\t11/22/88";
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
**	GEGENSCALEBX	             	       Scale given box coords
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
**	GNE 10/04/87 Created
**
**--
**/

#include "geGks.h"


geGenScaleBx(Bx)
struct GE_BX	*Bx;
{long	dx, dy, ifact;
 float	dw, dh, fact, fh, fw;

#ifdef GERAGS

dx = geState.Dx;
dy = geState.Dy;
fw = (float)geEditBox.Sz.w;
fh = (float)geEditBox.Sz.h;
dw = fw + (float)dx;
dh = fh + (float)dy;
/*
 * 1st point
 */
if (dx && geEditBox.Sz.w)
  {fact       = ((float)(Bx->x1 - geEditBox.Pt.x) * dw) / fw;
   GEINT(fact, ifact);
   geState.Dx = geEditBox.Pt.x + ifact - Bx->x1;
  }
else
  geState.Dx = 0;

if (dy && geEditBox.Sz.h)
  {fact       = ((float)(Bx->y1 - geEditBox.Pt.y) * dh) / fh;
   GEINT(fact, ifact);
   geState.Dy = geEditBox.Pt.y + ifact - Bx->y1;
  }
else
  geState.Dy = 0;
  
geGenAln(Bx->x1, Bx->y1);

if (dx && geEditBox.Sz.w) Bx->x1 += geState.Dx;
if (dy && geEditBox.Sz.h) Bx->y1 += geState.Dy;
/*
 * 2nd point
 */
if (dx && geEditBox.Sz.w)
  {fact       = ((float)(Bx->x2 - geEditBox.Pt.x) * dw) / fw;
   GEINT(fact, ifact);
   geState.Dx = geEditBox.Pt.x + ifact - Bx->x2;
  }
else
  geState.Dx = 0;

if (dy && geEditBox.Sz.h)
  {fact       = ((float)(Bx->y2 - geEditBox.Pt.y) * dh) / fh;
   GEINT(fact, ifact);
   geState.Dy = geEditBox.Pt.y + ifact - Bx->y2;
  }
else
  geState.Dy = 0;
  
geGenAln(Bx->x2, Bx->y2);

if (dx && geEditBox.Sz.w) Bx->x2 += geState.Dx;
if (dy && geEditBox.Sz.h) Bx->y2 += geState.Dy;
/*
 * 3rd point
 */
if (dx && geEditBox.Sz.w)
  {fact       = ((float)(Bx->x3 - geEditBox.Pt.x) * dw) / fw;
   GEINT(fact, ifact);
   geState.Dx = geEditBox.Pt.x + ifact - Bx->x3;
  }
else
  geState.Dx = 0;

if (dy && geEditBox.Sz.h)
  {fact       = ((float)(Bx->y3 - geEditBox.Pt.y) * dh) / fh;
   GEINT(fact, ifact);
   geState.Dy = geEditBox.Pt.y + ifact - Bx->y3;
  }
else
  geState.Dy = 0;
  
geGenAln(Bx->x3, Bx->y3);

if (dx && geEditBox.Sz.w) Bx->x3 += geState.Dx;
if (dy && geEditBox.Sz.h) Bx->y3 += geState.Dy;
/*
 * 4th point
 */
if (dx && geEditBox.Sz.w)
  {fact       = ((float)(Bx->x4 - geEditBox.Pt.x) * dw) / fw;
   GEINT(fact, ifact);
   geState.Dx = geEditBox.Pt.x + ifact - Bx->x4;
  }
else
  geState.Dx = 0;

if (dy && geEditBox.Sz.h)
  {fact       = ((float)(Bx->y4 - geEditBox.Pt.y) * dh) / fh;
   GEINT(fact, ifact);
   geState.Dy = geEditBox.Pt.y + ifact - Bx->y4;
  }
else
  geState.Dy = 0;
  
geGenAln(Bx->x4, Bx->y4);

if (dx && geEditBox.Sz.w) Bx->x4 += geState.Dx;
if (dy && geEditBox.Sz.h) Bx->y4 += geState.Dy;

geState.Dx = dx;
geState.Dy = dy;

#endif
}
                                                 
