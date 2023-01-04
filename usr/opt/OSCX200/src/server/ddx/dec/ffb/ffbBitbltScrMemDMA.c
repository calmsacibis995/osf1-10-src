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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: ffbBitbltScrMemDMA.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:05:49 $";
#endif
/*
 */

#include "X.h"
#include "Xprotostr.h"

#include "miscstruct.h"
#include "regionstr.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"

#include "ffb.h"
#include "ffbblt.h"
#include "ffbdma.h"

void
ffbBitbltScrMemDMA(ffb, pbox, ppt, widthSrc, widthDst, psrcBase, pdstBase)
    FFB                 ffb;
    register BoxPtr     pbox;
    register DDXPointPtr ppt;
    register int        widthSrc, widthDst;
    Pixel8              *psrcBase, *pdstBase;
{
    register int        dstAlign;       /* Last few bits of destination ptr */
    register int        srcAlign;       /* last few bits of source ptr      */
    register int        shift;          /* Mostly dstAlign-srcAlign         */
    register Pixel8     *psrc;          /* pointer to current src longword  */
    register Pixel8     *pdst;          /* pointer to current dst longword  */

    Pixel8              *psrcLine;
    Pixel8              *pdstLine;
    Pixel8              *psrcNext;
    CommandWord         ones = ffbOnesDMA;
    CommandWord         leftMask, rightMask;
    int			width, wSav, w, h, chunkCount, avail;
    CommandWord		command;
    Pixel8		*pdstLinePhys;
    CommandWord         rotdepthSrc;
#if !defined(PCI) && FFBDEPTHBITS == 8
    /* Need to mask byte writes to main memory by hand */
    PixelWord		leftMemData, rightMemData;
    Pixel8		*pRightMemData;
    long		widthAlign;
#endif

    /*
     * We are asking the hardware to use dma to write frame buffer contents
     * into host memory.  Alignment constraint is 4 for dst, 8 for src.
     * We also can't cross TC page boundaries.  Recomputing masks at least
     * once per scan line satisfies all constraints.
     */
    
    wSav = (pbox->x2 - pbox->x1) * FFBPIXELBYTES;
    h = pbox->y2 - pbox->y1;

    /* advance pointers to start of read and write areas */
    psrcBase += ppt->y * widthSrc + ppt->x * FFBSRCPIXELBYTES;
    pdstBase += pbox->y1 * widthDst + pbox->x1 * FFBPIXELBYTES;

    /*
     * psrcLine, pdstLine, etc. are byte pointers
     * width is a bytes count
     * srcAlign and dstAlign are byte offsets
     * shift is a byte shift
     */
    do { /* for h */
	w = wSav;
	psrcLine = psrcBase;
	pdstLine = pdstBase;
	do { /* while w */
	    width = w;
            /* check to see if we can do all of this */
            avail = FFBMAINPAGEBYTES - ((long)pdstLine & FFBMAINPAGEMASK);
            if (width > avail) {
                width = avail;
            }
	    w -= width;
	    psrcNext = psrcLine + width * FFBSRCUNPACKED;

	    srcAlign = (long)psrcLine & FFBSRCDMAWRITEALIGNMASK;
	    dstAlign = (long)pdstLine & FFBDMAWRITEALIGNMASK;
	    shift = FFB_FIGURE_SHIFT(dstAlign, srcAlign);
	    /* DMAWRITE mode is weird...first chunk always loads residue reg */
	    if (shift < 0) {
		/* First source chunk has less data than destination needs, so
		   loading residue reg with first chunk is good. */
		shift += FFBDMAWRITESHIFTBYTES;
	    } else {
                /* First source chunk has enough data, but the dumb DMA logic
                   is going to read up a chunk and load it into the residue
                   register.  Back up psrcLine to make this a nop load. */
                psrcLine -= FFBSRCDMAWRITEALIGN;
	    }

	    CYCLE_REGS(ffb);
	    FFBSHIFT(ffb, shift);
	    psrcLine -= srcAlign;
	    pdstLine -= dstAlign;

            /* now find out how many 64-bit chunks we can write */
	    /* For a packed8 dst and an unpacked8 src we just count 1 byte per
	       pix.  So we'll move the number of bytes in the dst, since dst
	       will always be packed */
	    width += dstAlign;

	    /* dst can't be unpacked, so don't have to squash alignment */
	    chunkCount =
		(width + FFBDMAWRITESHIFTBYTES - 1) / FFBDMAWRITESHIFTBYTES;

            /* width += FFBDMAWRITESHIFTBYTES for first word read, but then
               width -= FFBDMAWRITESHIFTBYTES because wordCount is 1 less
	       than # words to transfer, so it's a wash. */

	    /* now compute both masks */
	    leftMask = FFBLEFTDMAWRITEMASK(dstAlign, ones);
	    rightMask = FFBRIGHTDMAWRITEMASK(width, ones);

	    if (chunkCount == 1) {
		/* 1 chunk DMA.  Hardware ignores left mask, uses right mask */
		rightMask &= leftMask;
	    }

	    command = FFBDMACOMMAND(leftMask, rightMask, chunkCount);

	    FFBVIRTUALTOBUS(pdstLine, pdstLinePhys);
	    FFBDMA(ffb, pdstLinePhys);
#if !defined(PCI) && FFBDEPTHBITS == 8
	    /* Get existing data at left and right edges */
	    leftMemData = *((PixelWord *) pdstLine);
	    widthAlign = width & FFBDMAWRITEALIGNMASK;
	    /* May get pRightMemData past DMA, but then masking is nop */
	    pRightMemData = pdstLine + width - widthAlign;
	    rightMemData = *((PixelWord *) pRightMemData);
#endif
	    FFBWRITE(psrcLine, command);
#if !defined(PCI) && FFBDEPTHBITS == 8
	    /* Restore stomped bytes at left and right edges */
	    FFBSYNC(ffb);
	    leftMask = FFBLEFTBUSMASK(dstAlign * 8, FFBBUSALL1);
	    *((PixelWord *) pdstLine) = (*((PixelWord *) pdstLine) & leftMask)
				    | (leftMemData & ~leftMask);
	    rightMask = FFBRIGHTBUSMASK(widthAlign * 8, FFBBUSALL1);
	    *((PixelWord *) pRightMemData) =
		(*((PixelWord *) pRightMemData) & rightMask)
		| (rightMemData & ~rightMask);
#endif

            psrcLine = psrcNext;
	    pdstLine += width;
	} while (w > 0);
	
	h--;
	psrcBase += widthSrc;
	pdstBase += widthDst;

    } while (h > 0);
}

    
    
/*
 * HISTORY
 */
/*
 *
 *
 * NAME           ffbBitbltScrMemDMA.c
 *
 * Description    Implements ffb to main memory dma.  Called only if 
 *                	o dma is available
 *			o the requested transfer is large enough to 
 *				offset the dma overhead
 *
 * Input          ffb		---	pointer to the ffb register set
 *                pbox		---	destination rectangle description
 *		  ppt		---	source rectangle description
 *		  width		---	in pixels, of the rectangle
 *		  h		---	in scanlines, of the rectangle
 *		  widthSrc	---	in bytes, of the source pixmap
 *		  widthDst	---	in bytes, of the destination buffer
 *		  psrcBase	---	address of the source bits
 *		  pdstBase	---	address of the destination bits
 *
 * Output         writes to the ffb registers; causes dma transfers
 *
 * Notes
 */
