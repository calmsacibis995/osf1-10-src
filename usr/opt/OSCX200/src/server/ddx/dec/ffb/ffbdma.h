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
/*
 * @(#)$RCSfile: ffbdma.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:08:54 $
 */
/*
 */

#ifndef FFBDMA_H
#define FFBDMA_H

extern CommandWord ffbOnesDMA;
#define FFBDMAREAD_ONES (0xff)

#define FFBMAINPAGEBYTES    (0x800)
#define FFBMAINPAGEMASK     (FFBMAINPAGEBYTES - 1)

/*
 * Declarations needed to lock down, translate, and release pages for DMA
 */

#define	FFBMAXDMAPAGES 2047

extern int		virtualPageMask;       /* (2^n) - 1 bytes	    */
extern int		virtualPageShift;       /* n bits		    */
extern Pixel8		**virtualPageMap;	/* Virtual to physical map  */
extern unsigned long    virtualPageBase;	/* First addr in map	    */
extern unsigned long    virtualPageMapLength;   /* Number of pages in map   */

#ifdef SOFTWARE_MODEL
#define FFBVIRTUALTOBUS(virtAddr, physAddr)			    \
{								    \
    long i_, align_;						    \
    align_ = (long)(virtAddr) & virtualPageMask;		    \
    i_ = (((long)(virtAddr)-virtualPageBase) >> virtualPageShift);  \
    if (i_ < 0 || i_ >= virtualPageMapLength) {			    \
	ErrorF("Virtual page out of range of page map\n");	    \
	abort();						    \
    }								    \
    physAddr = virtualPageMap[i_+1] + align_;			    \
    if (physAddr != virtAddr) {					    \
	ErrorF("Virtual page mapped incorrectly\n");		    \
	abort();						    \
    }								    \
}
#else
#define FFBVIRTUALTOBUS(virtAddr, physAddr)			    \
{								    \
    long i_, align_;						    \
    align_ = (long)(virtAddr) & virtualPageMask;		    \
    i_ = (((long)(virtAddr)-virtualPageBase) >> virtualPageShift);  \
    if (i_ < 0 || i_ >= virtualPageMapLength) {			    \
	ErrorF("Virtual page out of range of page map\n");	    \
	abort();						    \
    }								    \
    physAddr = virtualPageMap[i_+1] + align_;			    \
}
#endif

extern Bool ffbWirePages();
extern Bool ffbUnwirePages();



/*
 * For DMAREAD, the src is main memory, the dst is the screen, which has
 * a write buffer that accepts 32-bit writes, so we only have to 4-byte align.
 * Also note that main memory is never 8-bit unpacked.
 */
#define FFBSRCDMAREADALIGN	(FFBDMAREADSHIFTBYTES)
#define FFBSRCDMAREADALIGNMASK  (FFBSRCDMAREADALIGN - 1)
#define FFBDMAREADALIGN		(FFBDMAREADSHIFTBYTES * FFBUNPACKED)
#define FFBDMAREADALIGNMASK	(FFBDMAREADALIGN - 1)

#define FFBLEFTDMAREADMASK(align, _ones)	\
    (((_ones) << (align)) & (_ones))
#define FFBRIGHTDMAREADMASK(alignedWidth, _ones) \
    ((_ones) >> (-(alignedWidth) & (FFBDMAREADSHIFTBYTES-1)))



/*
 * For DMAWRITE, the src is screen, dst is in main memory.
 * So we are reading from vram through the chip.  Reads have
 * to be 8 pixel aligned.
 */
#define FFBSRCDMAWRITEALIGN	(FFBDMAWRITESHIFTBYTES * FFBSRCUNPACKED)
#define FFBSRCDMAWRITEALIGNMASK (FFBSRCDMAWRITEALIGN - 1)
#define FFBDMAWRITEALIGN	FFBBUSBYTES
#define FFBDMAWRITEALIGNMASK	(FFBDMAWRITEALIGN - 1)

#define FFBLEFTDMAWRITEMASK(a,b) FFBLEFTDMAREADMASK(a,b)
#define FFBRIGHTDMAWRITEMASK(alignedWidth, _ones) \
    ((ones) >> (-(alignedWidth) & (FFBDMAWRITESHIFTBYTES-1)))

#define FFBDMACOMMAND(_leftMask, _rightMask, _wordCount) \
    (((_wordCount) << 16) | ((_rightMask) << 8) | (_leftMask))

#endif /* FFBDMA_H */

/*
 * HISTORY
 */
