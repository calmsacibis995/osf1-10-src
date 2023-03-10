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
/* Combined Purdue/PurduePlus patches, level 2.0, 1/17/89 */
/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/* $XConsortium: mfbpolypnt.c,v 5.3 89/09/11 10:12:05 rws Exp $ */

#include "X.h"
#include "Xprotostr.h"
#include "pixmapstr.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "miscstruct.h"
#include "regionstr.h"
#include "scrnintstr.h"

#include "mfb.h"
#include "maskbits.h"

void
mfbPolyPoint(pDrawable, pGC, mode, npt, pptInit)
    register DrawablePtr pDrawable;
    GCPtr	pGC;
    int		mode;		/* Origin or Previous */
    int		npt;
    xPoint 	*pptInit;
{

    register BoxPtr pbox;
    register int nbox;

    register unsigned long *addrl;
    int nlwidth;

    int nptTmp;
    register xPoint *ppt;

    register int x;
    register int y;
    register int rop;
    mfbPrivGC	*pGCPriv;

    if (!(pGC->planemask & 1))
	return;

    pGCPriv = (mfbPrivGC *) pGC->devPrivates[mfbGCPrivateIndex].ptr;
    rop = pGCPriv->rop;

    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	addrl = ( unsigned long *)
	        (((PixmapPtr)(pDrawable->pScreen->devPrivate))->devPrivate.ptr);
	nlwidth = (int)
		  (((PixmapPtr)(pDrawable->pScreen->devPrivate))->devKind) /
		      sizeof(long);
    }
    else
    {
	addrl = ( unsigned long *)(((PixmapPtr)pDrawable)->devPrivate.ptr);
	nlwidth = (int)(((PixmapPtr)pDrawable)->devKind) / sizeof(long);
    }

    if ((mode == CoordModePrevious) && (npt > 1))
    {
	for (ppt = pptInit + 1, nptTmp = npt - 1; --nptTmp >= 0; ppt++)
	{
	    ppt->x += (ppt-1)->x;
	    ppt->y += (ppt-1)->y;
	}
    }

    nbox = REGION_NUM_RECTS(pGCPriv->pCompositeClip);
    pbox = REGION_RECTS(pGCPriv->pCompositeClip);
    for (; --nbox >= 0; pbox++)
    {
	if (rop == RROP_BLACK)
	{
	    for (ppt = pptInit, nptTmp = npt; --nptTmp >= 0; ppt++)
	    {
		x = ppt->x + pDrawable->x;
		y = ppt->y + pDrawable->y;
		if ((x >= pbox->x1) && (x < pbox->x2) &&
		    (y >= pbox->y1) && (y < pbox->y2))
		    *(addrl + (y * nlwidth) + (x >> PWSH)) &= rmask[x & PIM];
	    }
	}
	else if (rop == RROP_WHITE)
	{
	    for (ppt = pptInit, nptTmp = npt; --nptTmp >= 0; ppt++)
	    {
		x = ppt->x + pDrawable->x;
		y = ppt->y + pDrawable->y;
		if ((x >= pbox->x1) && (x < pbox->x2) &&
		    (y >= pbox->y1) && (y < pbox->y2))
		    *(addrl + (y * nlwidth) + (x >> PWSH)) |= mask[x & PIM];
	    }
	}
	else if (rop == RROP_INVERT)
	{
	    for (ppt = pptInit, nptTmp = npt; --nptTmp >= 0; ppt++)
	    {
		x = ppt->x + pDrawable->x;
		y = ppt->y + pDrawable->y;
		if ((x >= pbox->x1) && (x < pbox->x2) &&
		    (y >= pbox->y1) && (y < pbox->y2))
		    *(addrl + (y * nlwidth) + (x >> PWSH)) ^= mask[x & PIM];
	    }
	}
    }
}
