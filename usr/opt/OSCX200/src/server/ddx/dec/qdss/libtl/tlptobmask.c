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

#include "X.h"
#include <sys/types.h>
#include "gcstruct.h"
#include "Ultrix2.0inc.h"
#include <vaxuba/qduser.h>
#include <vaxuba/qdreg.h>

#include "qd.h"
#include "qdgc.h"

#include "tltemplabels.h"
#include "tl.h"


#ifdef BICHROME
# define PTOBOVERHEAD	 NMASKSHORTS + 6 + 8 + 2*NCOLORSHORTS
# define PTB_ROP_OVERHEAD 13
#else
# define PTOBOVERHEAD	 NMASKSHORTS + 9 + 7 + 1*NCOLORSHORTS
# define PTB_ROP_OVERHEAD 17
#endif

/*
 * Uses PTOBXY to broadcast a bitmap in the DMA queue to all planes
 *
 * Dragon clipping is set from a single box argument, not from the GC
 */
VOID
#ifdef BICHROME
 tlBitmapBichrome
#else
 tlBitmapStipple
#endif
	( pGC, qbit, fore, back, x0, y0, box)
    GCPtr	pGC;
    PixmapPtr	qbit;		/* x11 bitmap, with the mfb conventions */
    int		fore;
    int		back;		/* used in tlTileBichrome only */
    int		x0, y0;
    BoxPtr	box;		/* clipping */
{
    register unsigned short *p;
    unsigned short *	pshort = (unsigned short *) QD_PIX_DATA(qbit);
    int			slwidthshorts = qbit->devKind>>1;
    int			width = QDPIX_WIDTH(qbit);
    int			height = QDPIX_HEIGHT(qbit);
    int 		nscansperblock;
    unsigned short *	buf = NULL;
    int doCopy = 0;
    int skipx;
    int tmp_srcplane;

    extern int     req_buf_size;
    void doBitmap();

    SETTRANSLATEPOINT( 0, 0);

    /* Do clipping in y */
    if (box->y1 > y0) {
	int skip = box->y1 - y0;
	y0 += skip;
	pshort += skip * slwidthshorts;
	height -= skip;
    }
    if (y0 + height > box->y2)
	height = box->y2 - y0;
    if (height <= 0) return;

    if (x0 & 0xF) {
	nscansperblock =
	    ( min( req_buf_size, MAXDMAPACKET / sizeof(short))
	     - PTB_ROP_OVERHEAD - slwidthshorts) / (slwidthshorts<<1);
	tmp_srcplane = (1024/8) / qbit->devKind;
	nscansperblock = min(nscansperblock, tmp_srcplane);
	if (nscansperblock  > 0) doCopy = 1;
    }

    if (!doCopy) {
	if ((x0 & 0xF) != 0) {
	    /* The GPX does not shift for an XY-mode PTB, so we must do it. */
	    /* Dest scanline may be one short wider than source  */
	    buf = (unsigned short*)
		ALLOCATE_LOCAL( (qbit->devKind+2) * height); /* upper bound */ 
	
	    /* right-shift bitmap to same modulo-16 boundary as x0 */
	    slwidthshorts = bitmapShiftRight(pshort, buf,
					     slwidthshorts,
					     QDPIX_WIDTH(qbit), height,
					     x0&0xf);
	    pshort = buf;
	}
	nscansperblock = ( min( req_buf_size, MAXDMAPACKET / sizeof(short))
			  - 50 - PTOBOVERHEAD - slwidthshorts) / slwidthshorts;
    }
    else {
	/* Do clipping in x */
	if (box->x1 > x0) {
	    skipx = box->x1 - x0;
	    x0 += skipx;
	    width -= skipx;
	}
	else skipx = 0;
	if (x0 + width > box->x2)
	    width = box->x2 - x0;
	if (width <= 0) return;

	tmp_srcplane = 1 << GetFreeDashPlane();
	Need_dma(3);
#ifdef BICHROME
	*p++ = JMPT_INIT2COLORBITMAP; /*set SRC1_OCR_B */
#else
	Need_dma(3);
	*p++ = JMPT_INIT1COLORBITMAP; /*set SRC1_OCR_B */
#endif
	*p++ = tmp_srcplane;
	*p++ = JMPT_RESETCLIP;
	Confirm_dma();
    }

#ifdef BICHROME
    SetPlaneAlu(pGC);
#else
    Need_dma(3);
    *p++ = JMPT_SET_MASKED_ALU;
    *p++ = pGC->planemask;
    *p++ = umtable[ pGC->alu];
    Confirm_dma();
#endif

    for (; ; ) {
	int tmp_x;
	int curHeight = min(nscansperblock, height);
	register unsigned	nshort = slwidthshorts * curHeight;
	height -= curHeight;

	/*
	 * I believe this may all have to fit in one DMA partition,
	 * because we program the dragon to do PTOB
	 */

	if (doCopy) {
	    Need_dma(PTB_ROP_OVERHEAD + nshort + 2 * curHeight);
	    *p++ = JMPT_PTOBXY_PLAIN;
	    *p++ = tmp_srcplane;
	    *p++ = 0; /* x */
	    *p++ = DASH_Y; /* y */
	    *p++ = qbit->devKind * 8 * curHeight; /*width */
	    *p++ = 1; /* height */
	    /* DGA magic bit pattern for PTB, see VCB02 manual pp.3-117 */
	    *p++ = 0x06000 | (0x01fff & -nshort);
	    bcopy( pshort, p, nshort<<1);
	    p += nshort;
	    pshort += nshort;
	    *p++ = JMPT_PTOBXYCLEAN;
#ifndef BICHROME
	    *p++ = JMPT_SETCOLOR;
	    *p++ = pGC->fgPixel;
#endif
	    *p++ = JMPT_INIT_PLANE_BITMAP;
	    *p++ = DASH_Y;		/* source y */
	    *p++ = x0;			/* dest x */
	    *p++ = width;
	    *p++ = 1;			/* height */
	    tmp_x = skipx;
	    for (; --curHeight >= 0; y0++) {
		*p++ = tmp_x;		/* source x */
		*p++ = y0;		/* dest y */
		tmp_x += slwidthshorts << 4;
	    }
#ifndef BICHROME
	    *p++ = JMPT_SETMASK;
	    *p++ = 0xFFFF;
#endif
	    Confirm_dma();
	    if (height <= 0) {
		Need_dma(2);
		/* reset src1_ocr_a to default value */
		*p++ = JMPT_SETSRC1OCRA;
		*p++ = EXT_NONE|INT_SOURCE|NO_ID|BAR_SHIFT_DELAY;
		Confirm_dma();
		return;
	    }

	}
	else { /* ! doCopy */
	    
	    Need_dma( PTOBOVERHEAD + nshort);
#ifndef BICHROME
	    *p++ = MAC_SETCOLOR;
	    SETCOLOR( p, pGC->fgPixel);
#endif

	    *p++ = JMPT_SETCLIP; /* clip to intersection of dest and arg clip*/
	    *p++ = max( box->x1, x0);
	    *p++ = min( box->x2, x0+width);
	    *p++ = y0; /* y is pre-clipped */
	    *p++ = y0+curHeight;
#ifdef BICHROME
	    *p++ = JMPT_PTOBXY;
#else
	    *p++ = JMPT_PTOBXYMASK;
#endif
	    *p++ = pGC->planemask;
	    *p++ = x0 & 0x03fff;
	    *p++ = y0 & 0x03fff;
	    if ((x0 & 0xF) == 0)
		/* We used the original 32-bit-aligned bitmap, so pretend
		 * it was the full 32-bit-aligned width.
		 * Clipping will trim the excess short, if any.
		 */
		*p++ = (width + 31) & 0x3fe0;
	    else
		*p++ = width & 0x03fff;
	    *p++ = curHeight & 0x03fff;
	    /* DGA magic bit pattern for PTB, see VCB02 manual pp.3-117 */
	    *p++ = 0x06000 | (0x01fff & -nshort);
	    bcopy( pshort, p, nshort<<1);
	    p += nshort;
	    *p++ = JMPT_PTOBXYCLEAN;
#ifdef BICHROME
	    if (pGC->alu == GXcopy)
		*p++ = JMPT_RESET_FORE_BACK;
#else
	    *p++ = JMPT_SETMASK;
	    *p++ = 0xFFFF;
#endif
	    Confirm_dma ();

	    if (height <= 0) break;
	    y0 += nscansperblock;
	    pshort += nscansperblock*slwidthshorts;
	} /* end !doCopy */
    }
    if (buf) { DEALLOCATE_LOCAL(buf); }
}

#ifdef BICHROME		/* only one instance of bitmapShiftRight in library */
/*
 * Shift "right" in the frame buffer sense.  This is an algebraic left shift.
 * Pad each scan line of the destination to an integral number of shorts.
 * Source is known to be padded to an integral number of longwords.
 */
int
bitmapShiftRight( psrc, pdst, srcShorts, width, height, nbits)
    register unsigned short *	psrc;
    register unsigned short *	pdst;
    int			srcShorts;	/* source width in shorts */
    int			width;
    int			height;
    int			nbits;	/* number of bit places to shift: 0-15 */
{
    int		ir;	/* row index */
    register	ids = -1;
    int		dstShorts;
    register	nshift = 16 - nbits;

    dstShorts = (width + nbits + 15) >> 4;
    /*
     * for each scan line
     */
    for ( ir=0; ir<height; ir++, psrc+=srcShorts-(dstShorts-1))
    {
	/*
	 * for each short on the destination line,
	 * find the containing longword in the source and extract the bits
	 */
	*pdst++ = *psrc << nbits;
	ids += dstShorts; /* ids was: -1, now: dstShorts-1 */
	for (; --ids >= 0; psrc++)
	    *pdst++ = *(long *)psrc >> nshift;
    }
    return dstShorts;
}
#endif
