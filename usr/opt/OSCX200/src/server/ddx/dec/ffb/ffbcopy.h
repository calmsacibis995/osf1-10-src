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
 * @(#)$RCSfile: ffbcopy.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:08:34 $
 */
/*
 */
/*
 *
 *
 * NAME           ffbcopy.h
 *
 * Description    Extern declarations of all copy procedures that 
 *                compile per depth (i.e., 5 times as opposed to higher
 *		  level procedures that might compile just twice, once
 *		  for 8- and once for 32-bits).
 *
 *		  Also defines various macros used in determining
 *		  which depth copier to use.
 */

#ifndef FFBCOPY_H
#define FFBCOPY_H

/* tiling copiers */

extern void
    /* _PACKED_TO_PACKED */
    ffb8888TileFillArea(),
    /* _PACKED_TO_UNPACKED */
    ffb88328TileFillArea(),
    /* _UNPACKED_TO_PACKED */
    ffb32888TileFillArea(),
    /* _UNPACKED_TO_UNPACKED */
    ffb328328TileFillArea(),
    /* _THIRTYTWO_BITS_DEEP */
    ffb32323232TileFillArea();

extern void
    /* _PACKED_TO_PACKED */
    ffb8888TileFillSpans(),
    /* _PACKED_TO_UNPACKED */
    ffb88328TileFillSpans(),
    /* _UNPACKED_TO_PACKED */
    ffb32888TileFillSpans(),
    /* _UNPACKED_TO_UNPACKED */
    ffb328328TileFillSpans(),
    /* _THIRTYTWO_BITS_DEEP */
    ffb32323232TileFillSpans();

/* CopyArea copiers */

/* When copying to main memory, and not using dma, need to optimize rop.
   Note that depth 8 main memory pixmaps are always packed. */
extern void
    /* _PACKED_TO_PACKED */
    ffb8888BitbltScrMemCopy(),    ffb8888BitbltScrMemCopySPM(),
    ffb8888BitbltScrMemXor(),         ffb8888BitbltScrMemGeneral(),
    ffb8888_GetSpans(),
    /* _UNPACKED_TO_PACKED */
    ffb32888BitbltScrMemCopy(),    ffb32888BitbltScrMemCopySPM(),
    ffb32888BitbltScrMemXor(),        ffb32888BitbltScrMemGeneral(),
    ffb32888_GetSpans(),
    /* _THIRTYTWO_BITS_DEEP */
    ffb32323232BitbltScrMemCopy(),    ffb32323232BitbltScrMemCopySPM(),
    ffb32323232BitbltScrMemXor(),       ffb32323232BitbltScrMemGeneral(),
    ffb32323232_GetSpans();

/* Chip handles all rops for these directions/modes.  Note that we never have
   unpacked 8-bit main memory pixmaps. */
extern void ffb8888BitbltScrScr();
extern void ffb8888BitbltMemScr();
extern void ffb8888BitbltMemScrDMA();

extern void ffb88328BitbltScrScr();
extern void ffb88328BitbltMemScr();
extern void ffb88328BitbltMemScrDMA();

extern void ffb32888BitbltScrScr();

extern void ffb328328BitbltScrScr();

extern void ffb32323232BitbltScrScr();
extern void ffb32323232BitbltMemScr();
extern void ffb32323232BitbltMemScrDMA();

/* dma from screen to main memory is a special case: no rops possible */
extern void ffb32888BitbltScrMemDMA();
extern void ffb8888BitbltScrMemDMA();
extern void ffb32323232BitbltScrMemDMA();

/* row indices into ffbCopyTab[] */
#define _PACKED_TO_PACKED       0
#define _PACKED_TO_UNPACKED     1
#define _UNPACKED_TO_PACKED     2
#define _UNPACKED_TO_UNPACKED   3
#define _THIRTYTWO_BITS_DEEP	4
#define _TWELVE_BITS_DEEP       5

/* column indices into ffbCopyTab[] are rop (if plain scr-mem copier) or */
#define _GET_SPANS			(_TILE_FILL_AREA - 4)
#define _MEM_SCREEN	 		(_TILE_FILL_AREA - 3)
#define _SCREEN_SCREEN	 		(_TILE_FILL_AREA - 2)
#define _TILE_FILL_SPANS		(_TILE_FILL_AREA - 1)
#define _TILE_FILL_AREA  		8

/* Unified table of copiers */
extern VoidProc ffbCopyTab[][_TILE_FILL_AREA+1];

/* how to select a row in the table */
#define FFB_SELECTROW(srcDepth,srcBitsPerPix,dstBitsPerPix,row) 	\
{									\
    if ((srcDepth) > 8) {						\
	if ((srcDepth) == 12) {						\
            (row) = _TWELVE_BITS_DEEP;					\
	} else {							\
	    (row) = _THIRTYTWO_BITS_DEEP;				\
	}								\
    } else {								\
        /* We know we're dealing with flavors of 8-bit things */	\
        if (srcBitsPerPix > 8) {					\
            /* one of the two unpacked src flavors */			\
            if (dstBitsPerPix > 8) {					\
                /* unpacked to unpacked */				\
                (row) = _UNPACKED_TO_UNPACKED;				\
            } else {							\
                (row) = _UNPACKED_TO_PACKED;				\
            }								\
        } else {							\
            /* one of the two packed source flavors */			\
            if (dstBitsPerPix > 8) {					\
                /* packed to unpacked */				\
                (row) = _PACKED_TO_UNPACKED;				\
            } else {							\
                (row) = _PACKED_TO_PACKED;				\
            }                                                           \
        }								\
    }									\
}


/****************************************************************************
 *                    Copy Data from Main Memory to FFB                     *
 ***************************************************************************/

/* These routines depend upon the fact that that read mask never has too many
   leading 0's...that is, we'll always read the first word. */
/*
 * These definitions used to be bracketed by a test for the number of 
 * copy buffer words.  However, the symbol used, "FFBBUFFERWORDS", evaluates 
 * differently. In order to avoid spurious duplication, the bracketing tests
 * based on "FFBBUFFERWORDS" have been eliminated. -peterv
 */

/* ||| FFBBUFFILL would probably be more efficient if ``mask'' were just
   a count instead. */

#if FFBVRAMBITS/FFBBUSBITS == 1
#   define FFBBUFFILL(ffb, psrc, mask)		    \
{						    \
    register Pixel8      *p_;			    \
    register CommandWord mask_;			    \
    register int	 i_;			    \
						    \
    p_ = (psrc);				    \
    mask_ = (mask);				    \
    i_ = 0;					    \
    do {					    \
	FFBBUFWRITE(ffb, i_, ((PixelWord *)p_)[0]); \
	p_ += FFBCOPYALIGNMENT;			    \
	mask_ >>= FFBCOPYPIXELALIGNMENT;	    \
	i_ += FFBVRAMBITS/FFBBUSBITS;		    \
    } while (mask_);				    \
} /* FFBBUFFILL */

#   define FFBBUFFILLALL(ffb, psrc)		    \
{						    \
    FFBBUFWRITE(ffb, 0, ((PixelWord *)(psrc))[0]);  \
    FFBBUFWRITE(ffb, 1, ((PixelWord *)(psrc))[1]);  \
    FFBBUFWRITE(ffb, 2, ((PixelWord *)(psrc))[2]);  \
    FFBBUFWRITE(ffb, 3, ((PixelWord *)(psrc))[3]);  \
    FFBBUFWRITE(ffb, 4, ((PixelWord *)(psrc))[4]);  \
    FFBBUFWRITE(ffb, 5, ((PixelWord *)(psrc))[5]);  \
    FFBBUFWRITE(ffb, 6, ((PixelWord *)(psrc))[6]);  \
    FFBBUFWRITE(ffb, 7, ((PixelWord *)(psrc))[7]);  \
} /* FFBBUFFILLALL */

#elif  FFBVRAMBITS/FFBBUSBITS == 2

/* This code very carefully has to avoid reading any data from the pixmap that
   doesn't actually exist.  It must also always write 2 extra words of data to
   the ffb buffer to make sure that the residue data gets advanced into the
   copy buffer (unless the mask already says write all 8 words).  The mask is
   always a contiguous group of 1's in the least significant bits, but may be
   completely 0.
*/

# define FFBBUFFILL(ffb, psrc, mask)			\
{							\
    register CommandWord mask_;				\
    register PixelWord a_, b_;				\
    if (mask) {						\
	a_ = ((PixelWord *)(psrc))[0];			\
	if ((mask) & 0xf0) {				\
	    b_ = ((PixelWord *)(psrc))[1];		\
	}						\
	FFBBUFWRITE(ffb, 0, a_, b_);			\
	mask_ = (mask) >> FFBCOPYPIXELALIGNMENT;	\
	if (mask_) {					\
	    a_ = ((PixelWord *)(psrc))[2];		\
	    if (mask_ & 0xf0) {				\
		b_ = ((PixelWord *)(psrc))[3];		\
	    }						\
	    FFBBUFWRITE(ffb, 2, a_, b_);		\
	    mask_ >>= FFBCOPYPIXELALIGNMENT;		\
	    if (mask_) {				\
		a_ = ((PixelWord *)(psrc))[4];		\
		if (mask & 0xf0) {			\
		    b_ = ((PixelWord *)(psrc))[5];	\
		}					\
		FFBBUFWRITE(ffb, 4, a_, b_);		\
		mask_ >>= FFBCOPYPIXELALIGNMENT;	\
		if (mask_) {				\
		    a_ = ((PixelWord *)(psrc))[6];	\
		    if (mask & 0xf0) {			\
			b_ = ((PixelWord *)(psrc))[7];	\
		    }					\
		}					\
		FFBBUFWRITE(ffb, 6, a_, b_);		\
	    } else {					\
		FFBBUFWRITE(ffb, 4, 0, 0);		\
	    }						\
	} else {					\
	    FFBBUFWRITE(ffb, 2, 0, 0);			\
	}						\
    } else {						\
	FFBBUFWRITE(ffb, 0, 0, 0);			\
    }							\
} /* FFBBUFFILL */

#   define FFBBUFFILLALL(ffb, psrc)		\
{						\
    register PixelWord a_, b_, c_, d_,		\
		       e_, f_, g_, h_;		\
    a_ = ((PixelWord *)(psrc))[0];		\
    b_ = ((PixelWord *)(psrc))[1];		\
    c_ = ((PixelWord *)(psrc))[2];		\
    d_ = ((PixelWord *)(psrc))[3];		\
    e_ = ((PixelWord *)(psrc))[4];		\
    f_ = ((PixelWord *)(psrc))[5];		\
    g_ = ((PixelWord *)(psrc))[6];		\
    h_ = ((PixelWord *)(psrc))[7];		\
    FFBBUFWRITE(ffb, 0, a_, b_);		\
    FFBBUFWRITE(ffb, 2, c_, d_);		\
    FFBBUFWRITE(ffb, 4, e_, f_);		\
    FFBBUFWRITE(ffb, 6, g_, h_);		\
} /* FFBBUFFILLALL */
#endif 



/****************************************************************************
 *               Macros to Copy Data from FFB to Main Memory                *
 ***************************************************************************/


#if FFBBUSBITS == 32
#   if FFBPIXELBITS == 8
#       define FFBBUFDRAIN(ffb, pdst, mask, bitsmask)	\
{							\
    register Pixel8	    *p_;			\
    register CommandWord    mask_, tmask_;		\
    register PixelWord	    src_;			\
    register volatile PixelWord *psrc_;			\
							\
    p_ = (pdst);					\
    mask_ = (mask);					\
    psrc_ = ffb->buffer;				\
    FFBSYNC(ffb);					\
    do {						\
	src_ = FFBREAD(psrc_);				\
	tmask_ = mask_ & 0xf;				\
	if (tmask_ == 0xf) {				\
	    /* Full word write */			\
	    MYCFBCOPY(src_, (volatile PixelWord *) p_);	\
	} else if (tmask_ != 0) {			\
	    if (mask_ & 1) MYCFBCOPY(src_, p_);		\
	    if (mask_ & 2) MYCFBCOPY(src_ >> 8, p_+1);  \
	    if (mask_ & 4) MYCFBCOPY(src_ >> 16, p_+2); \
	    if (mask_ & 8) MYCFBCOPY(src_ >> 24, p_+3); \
	}						\
	p_ += FFBBUSBYTES;				\
	mask_ >>= FFBBUSPIXELS;				\
	psrc_++;					\
    } while (mask_);					\
} /* FFBBUFDRAIN */

#define FFBBUFDRAINALL(ffb, pdst, bitsmask)		    \
{							    \
    register PixelWord src_;				    \
    register volatile PixelWord *p_;			    \
							    \
    FFBSYNC(ffb);					    \
    src_ = FFBBUFREAD(ffb, 0);				    \
    p_ = (PixelWord *)((pdst) + 0*FFBBUSBYTES);		    \
    MYCFBCOPY(src_, p_);				    \
    src_ = FFBBUFREAD(ffb, 1);				    \
    p_ = (PixelWord *)((pdst) + 1*FFBBUSBYTES);		    \
    MYCFBCOPY(src_, p_);				    \
    src_ = FFBBUFREAD(ffb, 2);				    \
    p_ = (PixelWord *)((pdst) + 2*FFBBUSBYTES);		    \
    MYCFBCOPY(src_, p_);				    \
    src_ = FFBBUFREAD(ffb, 3);				    \
    p_ = (PixelWord *)((pdst) + 3*FFBBUSBYTES);		    \
    MYCFBCOPY(src_, p_);				    \
    src_ = FFBBUFREAD(ffb, 4);				    \
    p_ = (PixelWord *)((pdst) + 4*FFBBUSBYTES);		    \
    MYCFBCOPY(src_, p_);				    \
    src_ = FFBBUFREAD(ffb, 5);				    \
    p_ = (PixelWord *)((pdst) + 5*FFBBUSBYTES);		    \
    MYCFBCOPY(src_, p_);				    \
    src_ = FFBBUFREAD(ffb, 6);				    \
    p_ = (PixelWord *)((pdst) + 6*FFBBUSBYTES);		    \
    MYCFBCOPY(src_, p_);				    \
    src_ = FFBBUFREAD(ffb, 7);				    \
    p_ = (PixelWord *)((pdst) + 7*FFBBUSBYTES);		    \
    MYCFBCOPY(src_, p_);				    \
} /* FFBBUFDRAINALL */

#   elif FFBPIXELBITS == 32
/* ||| Should this be changed ala 8-bit version above? */
#       define FFBBUFDRAIN(ffb, pdst, mask, bitsmask)	\
{							\
    register Pixel8	    *p_;			\
    register CommandWord    mask_, tmask_;		\
    register PixelWord	    src_;			\
    register int	    i_;				\
							\
    p_ = (pdst);					\
    mask_ = (mask);					\
    i_ = 0;						\
    FFBSYNC(ffb);					\
    do {						\
	src_ = FFBBUFREAD(ffb, i_);			\
	tmask_ = mask_ & 0x1;				\
	if (tmask_ != 0) {				\
	    /* Full word write */			\
	    MYCFBCOPY(src_ & bitsmask, (PixelWord *) p_); \
	}						\
	p_ += FFBBUSBYTES;				\
	mask_ >>= FFBBUSPIXELS;				\
	i_++;						\
    } while (mask_);					\
} /* FFBBUFDRAIN */
#define FFBBUFDRAINALL(ffb, pdst, bitsmask)		    \
{							    \
    register PixelWord src_;				    \
    register volatile PixelWord *p_;			    \
							    \
    FFBSYNC(ffb);					    \
    src_ = FFBBUFREAD(ffb, 0);				    \
    p_ = (PixelWord *)((pdst) + 0*FFBBUSBYTES);		    \
    MYCFBCOPY(src_ & bitsmask, p_);			    \
    src_ = FFBBUFREAD(ffb, 1);				    \
    p_ = (PixelWord *)((pdst) + 1*FFBBUSBYTES);		    \
    MYCFBCOPY(src_ & bitsmask, p_);			    \
    src_ = FFBBUFREAD(ffb, 2);				    \
    p_ = (PixelWord *)((pdst) + 2*FFBBUSBYTES);		    \
    MYCFBCOPY(src_ & bitsmask, p_);			    \
    src_ = FFBBUFREAD(ffb, 3);				    \
    p_ = (PixelWord *)((pdst) + 3*FFBBUSBYTES);		    \
    MYCFBCOPY(src_ & bitsmask, p_);			    \
    src_ = FFBBUFREAD(ffb, 4);				    \
    p_ = (PixelWord *)((pdst) + 4*FFBBUSBYTES);		    \
    MYCFBCOPY(src_ & bitsmask, p_);			    \
    src_ = FFBBUFREAD(ffb, 5);				    \
    p_ = (PixelWord *)((pdst) + 5*FFBBUSBYTES);		    \
    MYCFBCOPY(src_ & bitsmask, p_);			    \
    src_ = FFBBUFREAD(ffb, 6);				    \
    p_ = (PixelWord *)((pdst) + 6*FFBBUSBYTES);		    \
    MYCFBCOPY(src_ & bitsmask, p_);			    \
    src_ = FFBBUFREAD(ffb, 7);				    \
    p_ = (PixelWord *)((pdst) + 7*FFBBUSBYTES);		    \
    MYCFBCOPY(src_ & bitsmask, p_);			    \
} /* FFBBUFDRAINALL */
#   endif
#endif



#endif /* FFBCOPY_H */

/*
 * HISTORY
 */
