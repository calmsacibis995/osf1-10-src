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
static char SccsId[] = "@(#)gegenscale.c	1.1\t11/22/88";
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
**	GEGENSCALE	             	       Scale given coords
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
**	GNE 03/03/87 Created
**
**--
**/

#include "geGks.h"


geGenScale(co)
struct GE_CO	*co;
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
  {fact       = ((float)(co->x1 - geEditBox.Pt.x) * dw) / fw;
   GEINT(fact, ifact);
   geState.Dx = geEditBox.Pt.x + ifact - co->x1;
  }
else
  geState.Dx = 0;

if (dy && geEditBox.Sz.h)
  {fact       = ((float)(co->y1 - geEditBox.Pt.y) * dh) / fh;
   GEINT(fact, ifact);
   geState.Dy = geEditBox.Pt.y + ifact - co->y1;
  }
else
  geState.Dy = 0;
  
geGenAln(co->x1, co->y1);

if (dx && geEditBox.Sz.w) co->x1 += geState.Dx;
if (dy && geEditBox.Sz.h) co->y1 += geState.Dy;
/*
 * 2nd point
 */
if (dx && geEditBox.Sz.w)
  {fact       = ((float)(co->x2 - geEditBox.Pt.x) * dw) / fw;
   GEINT(fact, ifact);
   geState.Dx = geEditBox.Pt.x + ifact - co->x2;
  }
else
  geState.Dx = 0;

if (dy && geEditBox.Sz.h)
  {fact       = ((float)(co->y2 - geEditBox.Pt.y) * dh) / fh;
   GEINT(fact, ifact);
   geState.Dy = geEditBox.Pt.y + ifact - co->y2;
  }
else
  geState.Dy = 0;
  
geGenAln(co->x2, co->y2);

if (dx && geEditBox.Sz.w) co->x2 += geState.Dx;
if (dy && geEditBox.Sz.h) co->y2 += geState.Dy;

geState.Dx = dx;
geState.Dy = dy;

#endif
}
                                                 
