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
#ifdef VMS
#define IDENT "X-2"
#define MODULE_NAME SFBFILLARC
#include "module_ident.h"
#endif
/************************************************************
Copyright 1989 by The Massachusetts Institute of Technolgy

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of MIT not be used in
advertising or publicity pertaining to distribution of the
software without specific prior written permission.
M.I.T. makes no representation about the suitability of
this software for any purpose. It is provided "as is"
without any express or implied warranty.

********************************************************/

/****************************************************************************
**                                                                          *
**                       COPYRIGHT (c) 1990, 1991 BY                        *
**              DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.               *
**			     ALL RIGHTS RESERVED                            *
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
****************************************************************************/
/*
**++
**  FACILITY:
**
**      DDXSFB - VMS SFB server
**
**  ABSTRACT:
**
**
**  AUTHORS:
**
**      Irene McCartney (from Joel McCormack)
**
**  CREATION DATE:     19-Nov-1991
**
**  MODIFICATION HISTORY:
**
** X-002	BIM0011		Irene McCartney			27-Jan-1992
**		Add edit history
**		Merge latest changes from Joel 
**
**--
**/

/* $XConsortium: sfbfillarc.c,v 5.8 89/11/24 18:10:58 rws Exp $ */

#include "X.h"
#include "Xprotostr.h"
#include "miscstruct.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "mifillarc.h"
#include "sfb.h"
#include "sfbfillarc.h"
#include "sfbcirclebits.h"

extern void miPolyFillArc();

#define FILLSPAN(sfb, addr, xl, xr, ones)   \
{					    \
    register int w_;			    \
    register Pixel8 *pdst_;		    \
    w_ = (xr - xl + 1);			    \
    if (w_ > 0) {			    \
	pdst_ = addr + xl * SFBPIXELBYTES;  \
	SFBSOLIDSPAN(sfb, pdst_, w_, ones); \
    }					    \
} /* FILLSPAN */

#define FILLSLICESPANS(sfb, flip, addr, ones)   \
{						\
    if (!flip) {				\
	FILLSPAN(sfb, addr, xl, xr, ones);	\
    } else {					\
	xc = xorg - x;				\
	FILLSPAN(sfb, addr, xc, xr, ones);	\
	xc += slw - 1;				\
	addr = CYCLE_FB(addr);			\
	FILLSPAN(sfb, addr, xl, xc, ones);	\
    }						\
} /* FILLSLICESPAN */

void
sfbPolyFillArc(pDraw, pGC, narcs, parcs)
    DrawablePtr     pDraw;
    GCPtr	    pGC;
    int		    narcs;
    xArc	    *parcs;
{
    register xArc   *arc;
    register int    i;
    BoxRec	    box;
    RegionPtr       cclip;
    Pixel8	    *addrb;
    int		    nlwidth;
    int		    xdraworg, ydraworg;
    cfbPrivGC       *gcPriv;
    int		    rightShift;
    CommandWord     ones = sfbStippleAll1;
    sfbScreenPrivPtr scrPriv;
    SFB		    sfb;

    DrawableBaseAndWidth(pDraw, addrb, nlwidth);
    CHECKSTATE(pDraw->pScreen, scrPriv, sfb, pGC);
    xdraworg = pDraw->x;
    ydraworg = pDraw->y;
    gcPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr);
    cclip = gcPriv->pCompositeClip;
    
    SFBMODE(sfb, TRANSPARENTSTIPPLE);

    for (arc = parcs, i = narcs; --i >= 0; arc++) {

	addrb = (Pixel8 *)CYCLE_FB(addrb);

	if (miFillArcEmpty(arc))
	    continue;
	if (miCanFillArc(arc)) {
	    box.x1 = arc->x + xdraworg;
	    box.y1 = arc->y + ydraworg;
	    box.x2 = box.x1 + (int)arc->width + 1;
	    box.y2 = box.y1 + (int)arc->height + 1;
	    if ((*pDraw->pScreen->RectIn)(cclip, &box) == rgnIN) {

		if ((arc->angle2 >= FULLCIRCLE) ||
		    (arc->angle2 <= -FULLCIRCLE)) {
		    int iscircle;
		    int x, y, e, ex;
		    int yk, xk, ym, xm, dx, dy, xorg, yorg;
		    miFillArcRec info;
		    Pixel8 *pdst, *addrlt, *addrlb;
		    register int xpos;
		    register int slw;
		
		    if (arc->width == arc->height && arc->width <= SFBBUSBITS) {
			int j, align;
			Pixel8 *pdst;
			CommandWord *psrc;
			CommandWord srcbits;
			j = arc->width;
			pdst = addrb + box.y1 * nlwidth + box.x1*SFBPIXELBYTES;
			psrc = solidCircles[j];
			align = (int)pdst & SFBALIGNMASK;
			pdst -= align;
			SFBBYTESTOPIXELS(align);
			if (j + align <= SFBSTIPPLEBITS) {
			    if (j & 1) {
				SFBWRITE(pdst, psrc[0] << align);
				pdst += nlwidth;
				psrc++;
				j--;
			    }
			    while (j > 0) {
				SFBWRITE(pdst, psrc[0] << align);
				pdst += nlwidth;
				SFBWRITE(pdst, psrc[1] << align);
				pdst += nlwidth;
				psrc += 2;
				j -= 2;
			    }
#if SFBSTIPPLEBITS < SFBBUSBITS 
			} else if (j + align <= 2 * SFBSTIPPLEBITS) {
#else
			} else {
#endif
			    rightShift = SFBSTIPPLEBITS - align;
			    while (j > 0) {
				srcbits = *psrc;
				SFBWRITE(pdst, srcbits << align);
				srcbits >>= rightShift;
				SFBWRITE(pdst+SFBSTIPPLEBYTESDONE, srcbits);
				pdst += nlwidth;
				psrc++;
				j--;
			    }
#if SFBSTIPPLEBITS < SFBBUSBITS
			} else {
			    /* Lots of writes. */
			    Pixel8 *pd;
			    while (j > 0) {
				srcbits = *psrc;
				SFBWRITE(pdst, srcbits << align);
				srcbits >>= rightShift;
				SFBWRITE(pdst + SFBSTIPPLEBYTESDONE, srcbits);
				pd = pdst + 2 * SFBSTIPPLEBYTESDONE;
				srcbits >>= SFBSTIPPLEBITS;
				while (srcbits != 0) {
				    SFBWRITE(pd, srcbits);
				    srcbits >>= SFBSTIPPLEBITS;
				    pd += SFBSTIPPLEBYTESDONE;
				}
				pdst += nlwidth;
				psrc++;
				j--;
			    }
#endif
			}
			continue;
		    }

		    addrlt = addrb;
		    miFillArcSetup(arc, &info);
		    MIFILLARCSETUP();
		    xorg += xdraworg;
		    yorg += ydraworg;
		    addrlb = CYCLE_FB(addrlt);
		    addrlt += nlwidth * (yorg - y);
		    addrlb += nlwidth * (yorg + y + dy);
		    iscircle = (arc->width == arc->height);
		    while (y) {
			addrlt += nlwidth;
			addrlt = CYCLE_FB(addrlt);
			addrlb -= nlwidth;
			addrlb = CYCLE_FB(addrlb);
			if (iscircle) {
			    MIFILLCIRCSTEP(slw);
			} else {
			    MIFILLELLSTEP(slw);
			    if (!slw)
				continue;
			}
			xpos = xorg - x;
			pdst = addrlt + xpos * SFBPIXELBYTES;
			SFBSOLIDSPAN(sfb, pdst, slw, ones);
			if (miFillArcLower(slw)) {
			    pdst = addrlb + xpos * SFBPIXELBYTES;
			    SFBSOLIDSPAN(sfb, pdst, slw, ones);
			}
		    } /* while y */

		} else {
		    int yk, xk, ym, xm, dx, dy, xorg, yorg, slw;
		    register int x, y, e, ex;
		    miFillArcRec info;
		    miArcSliceRec slice;
		    int xl, xr, xc;
		    int iscircle;
		    Pixel8 *addrlb, *addrlt;
		
		    addrlt = addrb;
		    miFillArcSetup(arc, &info);
		    miFillArcSliceSetup(arc, &slice, pGC);
		    MIFILLARCSETUP();
		    iscircle = (arc->width == arc->height);
		    xorg += xdraworg;
		    yorg += ydraworg;
		    addrlb = CYCLE_FB(addrlt);
		    addrlt += nlwidth * (yorg - y);
		    addrlb += nlwidth * (yorg + y + dy);
		    slice.edge1.x += xdraworg;
		    slice.edge2.x += xdraworg;
		    while (y > 0)
		    {
			addrlt += nlwidth;
			addrlt = CYCLE_FB(addrlt);
			addrlb -= nlwidth;
			addrlb = CYCLE_FB(addrlb);
			if (iscircle)
			{
			    MIFILLCIRCSTEP(slw);
			}
			else
			{
			    MIFILLELLSTEP(slw);
			}
			MIARCSLICESTEP(slice.edge1);
			MIARCSLICESTEP(slice.edge2);
			if (miFillSliceUpper(slice))
			{
			    MIARCSLICEUPPER(xl, xr, slice, slw);
			    FILLSLICESPANS(sfb, slice.flip_top, addrlt, ones);
			}
			if (miFillSliceLower(slice))
			{
			    MIARCSLICELOWER(xl, xr, slice, slw);
			    FILLSLICESPANS(sfb, slice.flip_bot, addrlb, ones);
			}
		    }
		}
		continue;
	    }
	}
	miPolyFillArc(pDraw, pGC, 1, arc);
    }
}


