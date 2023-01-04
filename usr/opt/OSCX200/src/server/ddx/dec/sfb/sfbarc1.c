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
#define MODULE_NAME SFBARC1
#include "module_ident.h"
#endif
/****************************************************************************
**                                                                          *
**                          COPYRIGHT (c) 1991 BY                           *
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
**		Merge latest changes from Joel which includes support for 
**		SFBSTIPPLEBITS < SFBBUSBITS
**
**--
**/

/* $XConsortium: cfbzerarc.c,v 5.16 89/11/25 15:22:46 rws Exp $ */

#include "X.h"
#include "Xprotostr.h"
#include "miscstruct.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "sfb.h"
#include "sfbarc1.h"

/* If line width = 1, fill style solid, look for small circles and paint them
   using precomputed bitmaps. */

/* Use table generated automatically, assuming you have a correct X11 server,
   in sfbgenarc1.c.  Sorry, but it was way too complicated to generate these
   tables using mi code itself; I just paint the circles into a pixmap and
   then GetImage them. */

#include "sfbarc1bits.c"

extern void miPolyArc();

#define FULLCIRCLE (360 * 64)

void sfbPolyArc1(pDraw, pGC, narcs, parcs)
    register DrawablePtr	pDraw;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    xArc	*arc;
    int		i, diameter;
    BoxRec	box;
    RegionPtr   cclip;
    Pixel8      *addrb;
    int		nlwidth;
    int		xorg, yorg;
    cfbPrivGC   *gcPriv;
    sfbScreenPrivPtr scrPriv;
    SFB		sfb;

    xorg = pDraw->x;
    yorg = pDraw->y;
    DrawableBaseAndWidth(pDraw, addrb, nlwidth);
    CHECKSTATE(pDraw->pScreen, scrPriv, sfb, pGC);
    SFBMODE(sfb, TRANSPARENTSTIPPLE);

    gcPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr);
    cclip = gcPriv->pCompositeClip;

    for (arc = parcs, i = narcs; i > 0; arc++, i--) {
	diameter = (int)arc->width;
	if (diameter == arc->height && diameter < 32
	 && (arc->angle2 >= FULLCIRCLE || arc->angle2 <= -FULLCIRCLE)) {
	    box.x1 = arc->x + xorg;
	    box.y1 = arc->y + yorg;
	    box.x2 = box.x1 + diameter + 1;
	    box.y2 = box.y1 + diameter + 1;
	    if ((*pDraw->pScreen->RectIn)(cclip, &box) == rgnIN) {
		int align, rightShift;
		Pixel8 *pdst, *pdst2;
		CommandWord *psrc;
		CommandWord srcbits, srcbits2;

		pdst = addrb + box.y1 * nlwidth + box.x1 * SFBPIXELBYTES;
		psrc = width1Circles[diameter];
		align = (int)pdst & SFBALIGNMASK;
		pdst -= align;
		pdst2 = CYCLE_FB(pdst) + diameter*nlwidth;
		SFBBYTESTOPIXELS(align);
		if (diameter + align < SFBSTIPPLEBITS) {
		    while (diameter > 0) {
			srcbits = (psrc[0] << align);
			SFBWRITE(pdst, srcbits);
			SFBWRITE(pdst2, srcbits);
			pdst += nlwidth;
			pdst2 -= nlwidth;
			psrc++;
			diameter -= 2;
		    }
		    if (diameter == 0) {
			SFBWRITE(pdst, psrc[0] << align);
		    }
#if SFBSTIPPLEBITS < SFBBUSBITS 
		} else if (diameter + align < 2 * SFBSTIPPLEBITS) {
#else
		} else {
#endif
		    rightShift = SFBSTIPPLEBITS - align;
		    while (diameter > 0) {
			srcbits = psrc[0];
			srcbits2 = srcbits >> rightShift;
			srcbits <<= align;
			SFBWRITE(pdst, srcbits);
			SFBWRITE(pdst+SFBSTIPPLEBYTESDONE, srcbits2);
			SFBWRITE(pdst2, srcbits);
			SFBWRITE(pdst2+SFBSTIPPLEBYTESDONE, srcbits2);
			pdst += nlwidth;
			pdst2 -= nlwidth;
			psrc++;
			diameter -= 2;
		    }
		    if (diameter == 0) {
			srcbits = psrc[0];
			SFBWRITE(pdst, srcbits << align);
			SFBWRITE(pdst+SFBSTIPPLEBYTESDONE,
			    srcbits >> rightShift);
		    }
#if SFBSTIPPLEBITS < SFBBUSBITS
		} else {
		    /* Lots of writes. */
		    Pixel8  *pd, *pd2;
		    CommandWord tsrcbits;

		    rightShift = SFBSTIPPLEBITS - align;
		    while (diameter > 0) {
			srcbits = psrc[0];
			srcbits2 = srcbits >> rightShift;
			srcbits <<= align;
			SFBWRITE(pdst, srcbits);
			SFBWRITE(pdst+SFBSTIPPLEBYTESDONE, srcbits2);
			pd = pdst + 2*SFBSTIPPLEBYTESDONE;
			pd2 = pdst2 + 2*SFBSTIPPLEBYTESDONE;
			tsrcbits = srcbits2 >> SFBSTIPPLEBITS;
			while (tsrcbits != 0) {
			    SFBWRITE(pd, tsrcbits);
			    SFBWRITE(pd2, tsrcbits);
			    pd += SFBSTIPPLEBYTESDONE;
			    pd2 += SFBSTIPPLEBYTESDONE;
			    tsrcbits >>= SFBSTIPPLEBITS;
			}
			SFBWRITE(pdst2, srcbits);
			SFBWRITE(pdst2+SFBSTIPPLEBYTESDONE, srcbits2);
			    
			pdst += nlwidth;
			pdst2 -= nlwidth;
			psrc++;
			diameter -= 2;
		    }
		    if (diameter == 0) {
			srcbits = psrc[0];
			SFBWRITE(pdst, srcbits << align);
			srcbits >>= rightShift;
			SFBWRITE(pdst+SFBSTIPPLEBYTESDONE, srcbits);
			srcbits >>= SFBSTIPPLEBITS;
			do {
			    pdst += SFBSTIPPLEBYTESDONE;
			    SFBWRITE(pdst + SFBSTIPPLEBYTESDONE, srcbits);
			    srcbits >>= SFBSTIPPLEBITS;
			} while (srcbits != 0);
		    }
#endif
		}
		continue;
	    } /* end if use stipple code */
	}
	miPolyArc(pDraw, pGC, i, arc);
	return;
    } /* end for */
}
