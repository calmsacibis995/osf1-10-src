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
/************************************************************
Copyright 1989 by The Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
no- tice appear in all copies and that both that copyright
no- tice and this permission notice appear in supporting
docu- mentation, and that the name of MIT not be used in
advertising or publicity pertaining to distribution of the
software without specific prior written permission.
M.I.T. makes no representation about the suitability of
this software for any purpose. It is provided "as is"
without any express or implied warranty.

********************************************************/

/* $XConsortium: cfbpolypnt.c,v 5.14 91/12/19 14:17:12 keith Exp $ */

#include "X.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "regionstr.h"
#include "scrnintstr.h"
#include "cfb.h"
#include "cfbmskbits.h"

#define isClipped(c,ul,lr)  ((((c) - (ul)) | ((lr) - (c))) & ClipMask)

/* WARNING: pbox contains two shorts. This code assumes they are packed
 * and can be referenced together as a long.
 * changed to int since it would be an int on a 64-bit box
 */
#define PointLoop(fill) { \
    for (nbox = REGION_NUM_RECTS(cclip), pbox = REGION_RECTS(cclip); \
	 --nbox >= 0; \
	 pbox++) \
    { \
	c1 = *((int *) &pbox->x1) - off; \
	c2 = *((int *) &pbox->x2) - off - 0x00010001; \
	for (ppt = (int *) pptInit, i = npt; --i >= 0;) \
	{ \
	    pt = *ppt++; \
	    if (!isClipped(pt,c1,c2)) { \
		fill \
	    } \
	} \
    } \
}

void
cfbPolyPoint(pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr pDrawable;
    GCPtr pGC;
    int mode;
    int npt;
    xPoint *pptInit;
{
    register int   pt;
    register int   c1, c2;
    register unsigned long   ClipMask = 0x80008000;
    register unsigned long   xor;
#ifdef PIXEL_ADDR
    register PixelType   *addrp;
    register int    npwidth;
    PixelType	    *addrpt;
#else
    register unsigned long    *addrl;
    register int    nlwidth;
    unsigned long   *addrlt;
#endif
    register int   *ppt;
    RegionPtr	    cclip;
    int		    nbox;
    register int    i;
    register BoxPtr pbox;
    unsigned long   and;
    int		    rop = pGC->alu;
    int		    off;
    cfbPrivGCPtr    devPriv;
    xPoint	    *pptPrev;

    devPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr); 
    rop = devPriv->rop;
    if (rop == GXnoop)
	return;
    cclip = devPriv->pCompositeClip;
    xor = devPriv->xor;
    if ((mode == CoordModePrevious) && (npt > 1))
    {
	for (pptPrev = pptInit + 1, i = npt - 1; --i >= 0; pptPrev++)
	{
	    pptPrev->x += (pptPrev-1)->x;
	    pptPrev->y += (pptPrev-1)->y;
	}
    }
    off = *((int *) &pDrawable->x);
    off -= (off & 0x8000) << 1;
#ifdef PIXEL_ADDR
    cfbGetPixelWidthAndPointer(pDrawable, npwidth, addrp);
    addrp = addrp + pDrawable->y * npwidth + pDrawable->x;
    if (rop == GXcopy)
    {
	if (!(npwidth & (npwidth - 1)))
	{
	    npwidth = ffs(npwidth) - 1;
	    PointLoop( *(addrp + (intToY(pt) << npwidth) + intToX(pt)) = xor;)
	}
#ifdef sun
	else if (npwidth == 1152)
	{
	    register int    y;
	    PointLoop(y = intToY(pt); *(addrp + (y << 10) + (y << 7) + intToX(pt)) = xor;)
	}
#endif
	else
	{
	    PointLoop(*(addrp + intToY(pt) * npwidth + intToX(pt)) = xor;)
	}
    }
    else
    {
	and = devPriv->and;
	PointLoop(  addrpt = addrp + intToY(pt) * npwidth + intToX(pt);
		    *addrpt = DoRRop (*addrpt, and, xor);)
    }
#else
    cfbGetLongWidthAndPointer(pDrawable, nlwidth, addrl);
    and = devPriv->and;
    PointLoop(	addrlt = addrl + intToY(pt) * nlwidth + intToX(pt);
		*addrlt = DoRRop (*addrlt, and, xor); )
#endif
}
