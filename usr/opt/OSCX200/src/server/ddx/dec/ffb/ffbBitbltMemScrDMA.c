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
static char *rcsid = "@(#)$RCSfile: ffbBitbltMemScrDMA.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:05:44 $";
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
ffbBitbltMemScrDMA(ffb, pbox, ppt, widthSrc, widthDst, psrcBase, pdstBase)
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
    Pixel8		*psrcNext;
    CommandWord         ones = ffbOnesDMA;
    CommandWord         leftMask, rightMask;
    unsigned		wSav, w, width, h, wordCount, avail;
    CommandWord		command;
    Pixel8		*psrcLinePhys;

    /*
     * We are asking the hardware to use dma to read host main memory into
     * the frame buffer.  This entails writing to frame buffer memory.  Since
     * we recompute alignments and masks continually (as we cannot cross a
     * main memory page boundary), we don't have to worry about special code
     * for even/odd scanlines.
     */

    /* All widths are in # of bytes to transfer over bus */
    wSav = (pbox->x2 - pbox->x1) * FFBSRCPIXELBYTES;
    h = pbox->y2 - pbox->y1;

    /* advance pointers to start of read and write areas */
    psrcBase += (ppt->y * widthSrc) + ppt->x * FFBSRCPIXELBYTES;
    pdstBase += (pbox->y1 * widthDst) + pbox->x1 * FFBPIXELBYTES;

    /*
     * psrcLine, pdstLine, etc. are byte pointers
     * width is a byte count
     * srcAlign and dstAlign are byte offsets
     * shift is a byte shift
     */
    do { /* for h */
	w = wSav;
	psrcLine = psrcBase;
	pdstLine = pdstBase;
	WRITE_MEMORY_BARRIER();
	do { /* while w */
	    width = w;
            /* check to see if we can do all of this */
            avail = FFBMAINPAGEBYTES - ((long)psrcLine & FFBMAINPAGEMASK);
	    if (width > avail) {
		width = avail;
	    }
	    w -= width;
            psrcNext = psrcLine + width;

	    srcAlign = (long)psrcLine & FFBSRCDMAREADALIGNMASK;
	    dstAlign = (long)pdstLine & FFBDMAREADALIGNMASK;
	
	    shift = FFB_FIGURE_SHIFT(dstAlign, srcAlign);
	    if (shift < 0) {
		/* Ooops.  First source word has less data than destination
		   needs, so first word written is junk that primes the pump.
		   Adjust shift and dstAlign to reflect this fact. */
		shift += FFBDMAREADSHIFTBYTES;
		dstAlign += FFBDMAREADALIGN;
	    }
	    CYCLE_REGS(ffb);
	    FFBSHIFT(ffb, shift);
	    psrcLine -= srcAlign;
            pdstLine -= dstAlign;

            /* Compute how many words must be transferred over the bus */
            /* (For a packed8 src and an unpacked8 dst, we just count 1 
	       byte per pix; or generally: the number of bytes in the src,
	       since src will always be packed */
	    dstAlign /= FFBUNPACKED;
	    width += dstAlign;
	    wordCount = (width - shift - 1) / FFBBUSBYTES;

	    /* now compute both masks */
	    leftMask = FFBLEFTDMAREADMASK(dstAlign, ones);
	    rightMask = FFBRIGHTDMAREADMASK(width, ones);
	    if ((rightMask >> (FFBDMAREADSHIFTBYTES + shift)) != 0) {
		/* Don't drain residue case, adjust right mask and wordCount */
		rightMask >>= FFBDMAREADSHIFTBYTES;
	    }
	    if (wordCount == 0) {
		/* 1 word DMA.  Hardware ignores left mask, uses right mask */
		rightMask &= leftMask;
	    } else if (wordCount == 1) {
		/* 2 word DMA.  Chip tosses high 4 bits of left mask */
		rightMask &= ((leftMask >> FFBDMAREADSHIFTBYTES) | 0xf0);
	    }
	    command = FFBDMACOMMAND(leftMask, rightMask, wordCount);

	    FFBVIRTUALTOBUS(psrcLine, psrcLinePhys);
	    FFBDMA(ffb, psrcLinePhys);
	    pdstLine = CYCLE_FB(pdstLine);
	    FFBWRITE(pdstLine, command);

            psrcLine = psrcNext;
            pdstLine += width * FFBUNPACKED;
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
 * NAME           ffbBitbltMemScrDMA.c
 *
 * Description    Implements main memory to ffb dma.  Called only if 
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
