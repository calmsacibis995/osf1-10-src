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

/*
 *   NOTE: This file is now only included by other files. (Bothner, Aug 89)
 *   Created by djb, May 1986 (adapted from Ray's QDline.c)
 *   edited by drewry, 13 june 1986: added clipping
 *   edited by kelleher, 27 june 1986: added pixeltypes, interp code.
 */

#include <sys/types.h>

#include "X.h"
#include "windowstr.h"
#include "gcstruct.h"
#include "qdgc.h"

#define MININT	0x8000
#define MAXINT	0x7fff
/*
	From the Dragon Video System Hardware Specification: 
	 "...registers are 14 significant bits, signed ..."

	The REDUCE16to14() macro is responsible for correctly reducing 16
	bit signed values to 14 bit signed values. Note we are concatenating:
		> 2**13 	to 2**13 - 1
	and 
		< -1 * 2**13 	to -1 * 2**13

	REDUCE32to14 was written to handle subtraction underflow, as in:
		((-1 * 2**13)  - 10) by allowing this to be handled
		as an int.

*/

#undef	B13
#define	B13	0x2000
#define	MAX_POS_SIGNED14	0x1fff
#define	MIN_NEG_SIGNED14	B13


/* Reduce a short to 14 bit signed */
#define	REDUCE16to14(value)	\
	((value < 0) ? \
		((value < (short) (0xc000 | MIN_NEG_SIGNED14)) ? \
			MIN_NEG_SIGNED14 : (value & 0x3fff)) : \
		((value > MAX_POS_SIGNED14 ) ? \
			MAX_POS_SIGNED14 : value ))

/* Reduce an int to 14 bit signed */
#define	REDUCE32to14(value)	\
	((value < 0) ? \
		((value < (int) (0xffffc000 | MIN_NEG_SIGNED14)) ? \
			MIN_NEG_SIGNED14 : (value & 0x3fff)) : \
		((value > MAX_POS_SIGNED14 ) ? \
			MAX_POS_SIGNED14 : value ))


/* This is what the code did when I found it */
#define	OLD_WAY_REDUCE14(value)	(value & 0x3fff)

/*
	This macro was incorrect for inverted line segments
	(point2 lower than point1). 
	Now it allows pb1 to be inverted, but not pb2. My judgement call
	is that inverted line segments are allowed in the X protocol,
	but pb2 in this routine is always a clipping box, and never
	appears inverted.
*/
#define NOINTERSECTION( pb1, pb2)  \
	(    min((pb1)->x1,(pb1)->x2) >= (pb2)->x2  \
	  || max((pb1)->x1,(pb1)->x2) <= (pb2)->x1  \
	  || min((pb1)->y1,(pb1)->y2) >= (pb2)->y2  \
	  || max((pb1)->y1,(pb1)->y2) <= (pb2)->y1)



/*
 * driver headers
 */
#include "Ultrix2.0inc.h"
#include <vaxuba/qduser.h>
#include <vaxuba/qdreg.h>

#include "qd.h"

#include "tl.h"
#include "tltemplabels.h"

#define NLINES (req_buf_size/4)

/*
 * does the thin lines case only
 */
void
POLYLINES( pWin, pGC, mode, npt, pptInit)
    WindowPtr		pWin;
    GCPtr		pGC;
    int			mode;           /* Origin or Previous */
    int 		npt;            /* number of points */
    register DDXPointPtr pptInit;
{
    register DDXPointPtr abspts;  	/* actually, window-relative */
    register int	ip;		/* index into polygon list */
    register int	ir;		/* index into rectangle list */
    BoxRec		ptbounds;	/* used for trivial reject */
    register BoxPtr	pclip;		/* used for trivial reject */
    BoxPtr		pclipInit;
    int			nclip;		/* used for trivial reject */
    RegionPtr		pdrawreg = QDGC_COMPOSITE_CLIP(pGC);
    register unsigned short *p;
#ifdef DASHED
    int dashPlane; /* mask for plane containing dash pattern */
#endif

    ptbounds.x1	= MAXINT;
    ptbounds.y1	= MAXINT;
    ptbounds.x2	= MININT;
    ptbounds.y2	= MININT;

    if ( mode == CoordModeOrigin)
	abspts = pptInit;
    else	/* CoordModePrevious */
    {
	if ( npt == 0)		/* make sure abspts[0] is valid */
	    return;
	abspts = (DDXPointPtr) ALLOCATE_LOCAL( npt * sizeof( DDXPointRec));
	abspts[ 0].x = pptInit[ 0].x;
	abspts[ 0].y = pptInit[ 0].y;
	for ( ip=1; ip<npt; ip++)
	{
	    abspts[ ip].x = abspts[ ip-1].x + pptInit[ ip].x;
	    abspts[ ip].y = abspts[ ip-1].y + pptInit[ ip].y;
	}
    }

    /*
     * Prune list of clip rectangles by trivial rejection of entire polyline.
     * Note that we have to add one to the right and bottom bounds, because
     * lines are non-zero width.
     */
    for ( ip=0; ip<npt; ip++)
    {
	ptbounds.x1 = min( ptbounds.x1, abspts[ ip].x);
	ptbounds.y1 = min( ptbounds.y1, abspts[ ip].y);
	ptbounds.x2 = max( ptbounds.x2, abspts[ ip].x+1);
	ptbounds.y2 = max( ptbounds.y2, abspts[ ip].y+1);
    }
    /*
     * translate ptbounds to absolute screen coordinates
     */
    ptbounds.x1 += pGC->lastWinOrg.x;
    ptbounds.y1 += pGC->lastWinOrg.y;
    ptbounds.x2 += pGC->lastWinOrg.x;
    ptbounds.y2 += pGC->lastWinOrg.y;

    nclip = 0;
    ir = REGION_NUM_RECTS(pdrawreg);
    pclipInit = pclip =
	(BoxPtr) ALLOCATE_LOCAL( ir * sizeof(BoxRec));
    for (pclip = REGION_RECTS(pdrawreg) ; --ir >= 0; pclip++)
	if ( ! NOINTERSECTION( &ptbounds, pclip))
	    pclipInit[ nclip++] = *pclip;

    /* draw the polyline */

#ifdef DASHED
    dashPlane = InstallDashes(pGC);
#ifdef DEBUG
    if (!dashPlane) abort();
#endif
#endif
    INSTALL_FILLSTYLE(pGC, &pWin->drawable);

    SETTRANSLATEPOINT(pGC->lastWinOrg.x, pGC->lastWinOrg.y);

    for (pclip = pclipInit ; nclip-- > 0; pclip++) {
	register DDXPointRec *pPts = abspts;
	int nlines = npt-1;

	/*
	 *  Break the polyline into reasonable size packets so we
	 *  have enough space in the dma buffer.
	 */
#ifdef DASHED
	Need_dma (16+NCOLORSHORTS);
	if (pGC->lineStyle == LineOnOffDash)
	    *p++ = JMPT_INITFGLINE;
	else
	    *p++ = JMPT_INITFGBGLINE;
	*p++ = dashPlane;
#else
	Need_dma (10+NCOLORSHORTS);	/* per-clip initialization */
#endif
	*p++ = JMPT_SET_MASKED_ALU;
	*p++ = pGC->planemask;
	*p++ = umtable[pGC->alu];
	*p++ = JMPT_SETCLIP;
	*p++ = REDUCE16to14(pclip->x1);
	*p++ = REDUCE16to14(pclip->x2);
	*p++ = REDUCE16to14(pclip->y1);
	*p++ = REDUCE16to14(pclip->y2);
	*p++ = MAC_SETCOLOR;
	SETCOLOR(p, pGC->fgPixel);
#ifdef DASHED
	*p++ = JMPT_INITPATTERNPOLYLINE;
	*p++ = GC_DASH_LENGTH(pGC); /* src_1_dx */
	*p++ = 1; /* src_1_dy; */
	*p++ = 0; /* src_x */
	*p++ = DASH_Y; /* src_y */
#else
	*p++ = JMPT_INITPOLYLINE;
#endif
	Confirm_dma();
	while ( nlines > 0) {
	    register int nlinesThisTime = min(nlines, NLINES);
	    nlines -= NLINES;
	    Need_dma(nlinesThisTime * 4);
	    while (--nlinesThisTime >= 0) {
		*p++ = REDUCE16to14(pPts->x);
		*p++ = REDUCE16to14(pPts->y);
		*p++ = REDUCE32to14(((int)(pPts+1)->x - (int)pPts->x));
		*p++ = REDUCE32to14(((int)(pPts+1)->y - (int)pPts->y));
		pPts++;
	    }
	    Confirm_dma();
	}
	/* X11 wants the last point drawn, if not coincident with the first */
	if (pGC->capStyle != CapNotLast
	    && (abspts[0].x != pPts->x || abspts[0].y != pPts->y )) {
	    Need_dma(4);
	    *p++ = REDUCE16to14(pPts->x);
	    *p++ = REDUCE16to14(pPts->y);
	    *p++ = 0;
	    *p++ = 1;
	    Confirm_dma();
	}
    }
    Need_dma(4);
    *p++ = JMPT_RESET_FAST_DY_SLOW_DX;
#ifdef DASHED
    *p++ = JMPT_RESETRASTERMODE;
#endif
    *p++ = JMPT_SETMASK;
    *p++ = 0xFFFF;
    Confirm_dma();
    DEALLOCATE_LOCAL(pclipInit);
    if (mode == CoordModePrevious) DEALLOCATE_LOCAL(abspts);
}

void
POLYSEGMENT(pWin, pGC, nseg, pSegs)
    WindowPtr	pWin;
    GCPtr 	pGC;
    int		nseg;
    xSegment	*pSegs;
{
    register unsigned short *p;
    int i;
    BoxRec	ptbounds;	/* used for trivial reject */
    register xSegment	*curSeg;
    RegionPtr	pdrawreg = QDGC_COMPOSITE_CLIP(pGC);
    int		nclip = REGION_NUM_RECTS(pdrawreg);
    register BoxPtr pclip;

#ifdef DASHED
    int dashPlane = InstallDashes(pGC);
#ifdef DEBUG
    if (!dashPlane) abort();
#endif
#endif
    INSTALL_FILLSTYLE(pGC, &pWin->drawable);

    SETTRANSLATEPOINT(pGC->lastWinOrg.x, pGC->lastWinOrg.y);
    ptbounds.x1	= MAXINT;
    ptbounds.y1	= MAXINT;
    ptbounds.x2	= MININT;
    ptbounds.y2	= MININT;

    for (i=nseg, curSeg = pSegs; --i >= 0; curSeg++) {
	ptbounds.x1 = min( ptbounds.x1, curSeg->x1);
	ptbounds.y1 = min( ptbounds.y1, curSeg->y1);
	ptbounds.x2 = max( ptbounds.x2, curSeg->x1+1);
	ptbounds.y2 = max( ptbounds.y2, curSeg->y1+1);
	ptbounds.x1 = min( ptbounds.x1, curSeg->x2);
	ptbounds.y1 = min( ptbounds.y1, curSeg->y2);
	ptbounds.x2 = max( ptbounds.x2, curSeg->x2+1);
	ptbounds.y2 = max( ptbounds.y2, curSeg->y2+1);
    }
    /*
     * translate ptbounds to absolute screen coordinates
     */
    ptbounds.x1 += pGC->lastWinOrg.x;
    ptbounds.y1 += pGC->lastWinOrg.y;
    ptbounds.x2 += pGC->lastWinOrg.x;
    ptbounds.y2 += pGC->lastWinOrg.y;

    for (pclip = REGION_RECTS(pdrawreg); --nclip >= 0; pclip++) {
	int nlines = nseg;
	register int nlinesThisTime;
	curSeg = pSegs;

	if ( NOINTERSECTION( &ptbounds, pclip))
	    continue;

	/*
	 *  Break the polyline into reasonable size packets so we
	 *  have enough space in the dma buffer.
	 */
#ifdef DASHED
	Need_dma (16+NCOLORSHORTS);
	if (pGC->lineStyle == LineOnOffDash)
	    *p++ = JMPT_INITFGLINE;
	else
	    *p++ = JMPT_INITFGBGLINE;
	*p++ = dashPlane;
#else
	Need_dma (10+NCOLORSHORTS);	/* per-clip initialization */
#endif
	*p++ = JMPT_SET_MASKED_ALU;
	*p++ = pGC->planemask;
	*p++ = umtable[pGC->alu];
	*p++ = JMPT_SETCLIP;
	*p++ = REDUCE16to14(pclip->x1);
	*p++ = REDUCE16to14(pclip->x2);
	*p++ = REDUCE16to14(pclip->y1);
	*p++ = REDUCE16to14(pclip->y2);
	*p++ = MAC_SETCOLOR;
	SETCOLOR(p, pGC->fgPixel);
#ifdef DASHED
	*p++ = JMPT_INITPATTERNPOLYLINE;
	*p++ = GC_DASH_LENGTH(pGC); /* src_1_dx */
	*p++ = 1; /* src_1_dy; */
	*p++ = 0; /* src_x */
	*p++ = DASH_Y; /* src_y */
#else
	*p++ = JMPT_INITPOLYLINE;
#endif
	Confirm_dma();
	if (pGC->capStyle == CapNotLast)
	    while (nlines > 0) {
		nlinesThisTime = min(nlines, NLINES);
		nlines -= nlinesThisTime;
		Need_dma(nlinesThisTime * 4);
		for (; --nlinesThisTime >= 0; curSeg++) {
		    *p++ = REDUCE16to14(curSeg->x1);
		    *p++ = REDUCE16to14(curSeg->y1);
		    *p++ = REDUCE32to14(((int)curSeg->x2 - (int)curSeg->x1));
		    *p++ = REDUCE32to14(((int)curSeg->y2 - (int)curSeg->y1));
		}
		Confirm_dma();
	    }
	else
	    while (nlines > 0) {
		nlinesThisTime = min(nlines, NLINES>>1);
		Need_dma(nlinesThisTime * 8);
		nlines -= nlinesThisTime;
		for (; --nlinesThisTime >= 0; curSeg++) {
		   int xover,yover;
		    *p++ = REDUCE16to14(curSeg->x1);
		    *p++ = REDUCE16to14(curSeg->y1);
		    *p++ = REDUCE32to14(((int)curSeg->x2 - (int)curSeg->x1));
		    *p++ = REDUCE32to14(((int)curSeg->y2 - (int)curSeg->y1));
		    *p++ = REDUCE16to14(curSeg->x2);
		    *p++ = REDUCE16to14(curSeg->y2);
		    *p++ = 0;
		    *p++ = 1;
		}
		Confirm_dma();
	    }
    }

    Need_dma(4);
    *p++ = JMPT_RESET_FAST_DY_SLOW_DX;
#ifdef DASHED
    *p++ = JMPT_RESETRASTERMODE;
#endif
    *p++ = JMPT_SETMASK;
    *p++ = 0xFFFF;
    Confirm_dma();
}
