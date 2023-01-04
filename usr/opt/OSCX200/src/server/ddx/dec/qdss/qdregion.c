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
/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
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

#include "regionstr.h"

/*
 * Does what you really want; creates a region and initializes it
 * to contain the argument boxes.
 *
 * Always creates at least one box, of zero size if necessary, although it is
 * hidden from the caller, as numRects is set to zero
 */
RegionPtr
qdRegionInit( boxes, nbox)
    register BoxPtr	boxes;	/* length must agree with nbox */
    register int	nbox;	/* */
{
/* The code is simplified from miRectsToRegion in miregion.c */
    extern RegionPtr miRegionCreate();
    register RegionPtr	pRgn;
    register BoxPtr	pBox;
    register int        i;
    Bool overlap; /* result ignored */

    pRgn = miRegionCreate(NullBox, 0);
    if (!nbox)
	return pRgn;
    if (nbox == 1)
    {
	pRgn->extents = *boxes;
	pRgn->data = (RegDataPtr)NULL;
	return pRgn;
    }
    pRgn->data = (RegDataPtr)Xalloc(REGION_SZOF(nbox));
    if (pRgn->data == NULL) {
	return pRgn;
    }
    pRgn->data->size = nbox;
    pRgn->data->numRects = nbox;
    for (i = nbox, pBox = REGION_BOXPTR(pRgn); --i >= 0; boxes++)
    {
	*pBox = *boxes;
	if ((pBox->x2 <= pBox->x1) || (pBox->y2 <= pBox->y1))
	    pRgn->data->numRects--;
	else
	    pBox++;
    }
    pRgn->extents.x1 = pRgn->extents.x2 = 0;
    miRegionValidate(pRgn, &overlap);
    return pRgn;
}
