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
 * @(#)$RCSfile: ffbblt.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:07:31 $
 */
/*
 */

#ifndef FFBBLT_H
#define FFBBLT_H

/****************************************************************************
 *         Compute Masks for Left and Right Edges of a Copied Span          *
 * 32-bit scr-scr copies operate on 16-bit pixels,                          *
 * 32-bit mem-scr,scr-mem can only handle 8-bit pixels                      *
 *  so we can't use one symbol to handle both cases                         *
 ***************************************************************************/

extern CommandWord ffbCopyAll1;

/* UNPACKED constants are 4 if unpacked 8-bit, else 1 */
#define FFBUNPACKED	    (FFBPIXELBITS / FFBDEPTHBITS)
#define FFBSRCUNPACKED      (FFBSRCPIXELBITS / FFBSRCDEPTHBITS)

/* Alignment for copies */
#define FFBCOPYALIGNMENT    (FFBCOPYSHIFTBYTES * FFBUNPACKED)
#define FFBSRCCOPYALIGNMENT (FFBCOPYSHIFTBYTES * FFBSRCUNPACKED)

#if   FFBCOPYBITS == 8
#define FFBMASKEDCOPYBITS 8
#define FFBCOPYALL1 ((CommandWord)0xff)

#elif FFBCOPYBITS == 16
#define FFBMASKEDCOPYBITS 16
#define FFBCOPYALL1  ((CommandWord)0xffff)

#elif FFBCOPYBITS == 32
# if (FFBSRCDEPTHBITS == 32)
#   define FFBCOPYALL1_SCRSCR  ((CommandWord)0xffff)
#   define FFBCOPYALL1  ((CommandWord)0xff)
# else
#   define FFBCOPYALL1  ((CommandWord)0xffffffff)
#   define FFBCOPYALL1_SCRSCR FFBCOPYALL1
# endif
#define FFBMASKEDCOPYBITS 32
#endif

/* 64byte copies move 64 8-bit pixels, packed or unpacked, or 16 larger pixels */
#define FFBUNMASKEDCOPYPIXELS (FFBCOPYBUFFERBYTES / FFBDEPTHBYTES)
/* masked copies (32-byte copy buffer visible to host, 64-bytes visible to chip) */
#if (FFBSRCDEPTHBITS == 32)
#define FFBCOPYPIXELS	 8
#define FFBCOPYPIXELS_SCRSCR 16
#elif (FFBSRCDEPTHBITS == 8)
#define FFBCOPYPIXELS	32
#define FFBCOPYPIXELS_SCRSCR FFBCOPYPIXELS
#endif

#define FFBCOPYBITSMASK     (FFBCOPYBITS - 1)

#define FFBCOPYBYTESDONE        (FFBCOPYPIXELS * FFBPIXELBYTES)
#define FFBSRCCOPYBYTESDONE     (FFBCOPYPIXELS * FFBSRCPIXELBYTES)
#define FFBCOPYBYTESDONE_SCRSCR (FFBCOPYPIXELS_SCRSCR * FFBPIXELBYTES)
#define FFBSRCCOPYBYTESDONE_SCRSCR (FFBCOPYPIXELS_SCRSCR * FFBSRCPIXELBYTES)


#define FFBCOPYBYTESDONEUNMASKED (FFBCOPYBUFFERBYTES * FFBUNPACKED)
#define FFBSRCCOPYBYTESDONEUNMASKED (FFBCOPYBUFFERBYTES * FFBSRCUNPACKED)
#define FFBMASKEDCOPYBITSMASK (FFBMASKEDCOPYBITS - 1)

#define FFBMASKEDCOPYPIXELSMASK_SCRSCR (FFBCOPYPIXELS_SCRSCR - 1)
#define FFBMASKEDCOPYPIXELSMASK (FFBCOPYPIXELS - 1)

/* XXX: we aren't distinguishing between src and dst for alignment on
 * non-dma copies.  Things like shift, etc. take this non-specific
 * alignment and convert to src and dst specific pixel offsets...
 */
#define FFBCOPYPIXELALIGNMENT   (FFBCOPYALIGNMENT / FFBPIXELBYTES)
#define FFBCOPYPIXELALIGNMASK   (FFBCOPYALIGNMASK / FFBPIXELBYTES)

#define FFBCOPYALIGNMENTBYTES   (FFBCOPYALIGNMENT * FFBPIXELBYTES)
#define FFBSRCCOPYALIGNMENTBYTES (FFBSRCCOPYALIGNMENT * FFBSRCPIXELBYTES)

/* masks for ragged edges */
#if defined(MODULOSHIFTS) && (FFBCOPYPIXELS == WORDBITS)
#define FFBLEFTCOPYMASK(align, ones, unusedarg) \
    (((ones) << (align)) & (ones))
#define FFBRIGHTCOPYMASK(alignedWidth, ones, unusedarg) \
    ((ones) >> -(alignedWidth))

#else /* use longer sequences */

# if FFBCOPYBITS == 8
/* Copy mode isn't smart enough to through away high-order bits of mask if
   limited to 8 iterations in 32 bits/pixel mode. */
#define FFBLEFTCOPYMASK(align, ones, maskedcopypixelsmask) \
    (((ones) << ((align) & maskedcopypixelsmask)) & (ones))
# else
#define FFBLEFTCOPYMASK(align, ones, maskedcopypixelsmask) \
    (((ones) << ((align) & maskedcopypixelsmask)) & (ones))
# endif
#define FFBRIGHTCOPYMASK(alignedWidth, ones, maskedcopypixelsmask) \
    ((ones) >> (-(alignedWidth) & maskedcopypixelsmask))
#endif

/* Computation of masks for left and right edges when copying backwards */
#if FFBDEPTHBITS == 32
# define FFBBACKLEFTCOPYMASK(alignedWidth, all1s, copypixelsmask) \
    (ffb32BackLeftMask[(alignedWidth) & copypixelsmask] & all1s)
# define FFBBACKRIGHTCOPYMASK(align, all1s, copypixelsmask) \
    (ffb32BackRightMask[(align) + FFBCOPYPIXELALIGNMENT] & all1s)
#else
extern unsigned ffbBackRightMask[];
extern unsigned ffbBackLeftMask[];
# define FFBBACKLEFTCOPYMASK(alignedWidth, all1s, copypixelsmask) \
    (ffbBackLeftMask[(alignedWidth) & copypixelsmask] & all1s)
# define FFBBACKRIGHTCOPYMASK(align, all1s, copypixelsmask) \
    (ffbBackRightMask[(align) + FFBCOPYPIXELALIGNMENT] & all1s)
#endif
/* end stuff specifically for masks */

#define COMPUTE_EXTRA_BITS_MASK(_depth,_mask)      			\
	if(_depth == 24 )						\
	{								\
	     _mask = FFB_24BIT_PLANEMASK;				\
	}else{							        \
	     _mask = FFB_12_BUF0_PLANEMASK;				\
	}    	

#define FFB_FIGURE_SHIFT(dstAlign, srcAlign) \
    ((dstAlign)/FFBUNPACKED - (srcAlign)/FFBSRCUNPACKED)

#define CONJUGATE_FORWARD_ARGUMENTS(psrc, pdst, srcAlign, dstAlign, shift,  \
				    width, leftM, rightM, srcX, dstX,       \
				    all1, copypixelsmask)		    \
{									    \
    psrc += (srcX) * FFBSRCPIXELBYTES;					    \
    pdst += (dstX) * FFBPIXELBYTES;					    \
    srcAlign = ((long)(psrc)) & FFBSRCCOPYALIGNMASK;			    \
    dstAlign = ((long)(pdst)) & FFBCOPYALIGNMASK;			    \
    shift = FFB_FIGURE_SHIFT(dstAlign, srcAlign);			    \
    if (shift < 0) {							    \
        /*								    \
         * Ooops.  First source word has less data in it than we need to    \
	 * write to destination, so first word written to internal ffb      \
	 * copy buffer will be junk that just primes the pump.  Adjust      \
	 * shift and dstAlign to reflect this fact.			    \
         */								    \
        shift += FFBCOPYSHIFTBYTES;					    \
        dstAlign += FFBCOPYALIGNMENT;					    \
    }									    \
    psrc -= srcAlign;							    \
    pdst -= dstAlign;							    \
    FFBBYTESTOPIXELS(dstAlign);						    \
    width += dstAlign;							    \
    leftM = FFBLEFTCOPYMASK(dstAlign, all1, copypixelsmask);		    \
    rightM = FFBRIGHTCOPYMASK(width, all1, copypixelsmask);		    \
}

#define CONJUGATE_BACKWARD_ARGUMENTS(psrc, pdst, srcAlign, dstAlign, shift, \
				     width, leftM, rightM, srcX, dstX,      \
				     all1, copypixelsmask)		    \
{									    \
    psrc += ((srcX) + (width) - 1) * FFBSRCPIXELBYTES;			    \
    pdst += ((dstX) - 1) * FFBPIXELBYTES;				    \
    srcAlign = ((long)(psrc)) & FFBSRCCOPYALIGNMASK;			    \
    dstAlign = ((long)(pdst)) & FFBCOPYALIGNMASK;			    \
    shift = FFB_FIGURE_SHIFT(dstAlign, srcAlign);			    \
    if (shift >= 0) {							    \
        /*								    \
         * Ooops.  First source word has less data in it than we need to    \
	 * write to destination, so first word written to internal ffb      \
 	 * copy buffer will be junk that just primes the pump.  Adjust      \
	 * shift and dstAlign to reflect this fact.			    \
         */								    \
        shift -= FFBCOPYSHIFTBYTES;					    \
        dstAlign -= FFBCOPYALIGNMENT;					    \
    }									    \
    psrc -= srcAlign;							    \
    pdst -= dstAlign;							    \
    FFBBYTESTOPIXELS(dstAlign);						    \
    width += FFBCOPYPIXELALIGNMASK - dstAlign;				    \
    rightM = FFBBACKRIGHTCOPYMASK(dstAlign, all1, copypixelsmask);	    \
    leftM = FFBBACKLEFTCOPYMASK(width, all1, copypixelsmask);		    \
}


#define COPY_MASKED_AND_UNMASKED(ffb, psrc, pdst, shift, width,		\
				startMask, endMask,			\
				cpybytesMasked, cpybytesSrcMasked,	\
				cpybytesUnMasked, cpybytesSrcUnMasked)  \
{									\
    int			m_;						\
    Pixel8		*ps_, *pd_;					\
    volatile Pixel32    *preg;						\
									\
    ps_ = psrc; /* both guaranteed to be aligned now */			\
    pd_ = pdst;								\
    FFBWRITE(psrc, FFBCOPYALL1_SCRSCR); /* starting source addr */	\
    FFBWRITE(pdst, startMask);						\
    ps_ += cpybytesSrcMasked;						\
    pd_ += cpybytesMasked;						\
    /* Cycle preg among 4 copy64 source/dst address aliases */		\
    for (m_ = width - FFBCOPYPIXELS_SCRSCR;				\
         m_ >= 4*FFBUNMASKEDCOPYPIXELS;					\
	 m_ -= 4*FFBUNMASKEDCOPYPIXELS) {				\
	CYCLE_REGS(ffb);						\
	preg = &ffb->copy64src0;					\
	FFBWRITE(preg, (long)ps_ / FFBSRCPIXELBYTES);			\
	FFBWRITE(preg+1, (long)pd_ / FFBPIXELBYTES);			\
        ps_ += cpybytesSrcUnMasked;					\
        pd_ += cpybytesUnMasked;					\
	FFBWRITE(preg+2, (long)ps_ / FFBSRCPIXELBYTES);			\
	FFBWRITE(preg+3, (long)pd_ / FFBPIXELBYTES);			\
        ps_ += cpybytesSrcUnMasked;					\
        pd_ += cpybytesUnMasked;					\
	FFBWRITE(preg+4, (long)ps_ / FFBSRCPIXELBYTES);			\
	FFBWRITE(preg+5, (long)pd_ / FFBPIXELBYTES);			\
        ps_ += cpybytesSrcUnMasked;					\
        pd_ += cpybytesUnMasked;					\
	FFBWRITE(preg+6, (long)ps_ / FFBSRCPIXELBYTES);			\
	FFBWRITE(preg+7, (long)pd_ / FFBPIXELBYTES);			\
        ps_ += cpybytesSrcUnMasked;					\
        pd_ += cpybytesUnMasked;					\
    }									\
    if (m_ >= FFBUNMASKEDCOPYPIXELS) {					\
	CYCLE_REGS(ffb);						\
	preg = &ffb->copy64src0;					\
	do {								\
	    FFBWRITE(preg, (long)ps_ / FFBSRCPIXELBYTES);		\
	    FFBWRITE((preg+1), (long)pd_ / FFBPIXELBYTES);		\
	    preg += 2;							\
	    ps_ += cpybytesSrcUnMasked;					\
	    pd_ += cpybytesUnMasked;					\
	    m_ -= FFBUNMASKEDCOPYPIXELS;				\
	} while (m_ >= FFBUNMASKEDCOPYPIXELS);				\
    }									\
    if (m_ > FFBCOPYPIXELS_SCRSCR) {					\
        /*								\
         * Less than ffbunmaskedcopypixels and more than masked bits.	\
         * In fact, there are masked_bits pixels to copy, in		\
	 * addition to the ones handled by the endmask.			\
         */								\
        FFBWRITE(ps_, FFBCOPYALL1_SCRSCR);				\
        FFBWRITE(pd_, FFBCOPYALL1_SCRSCR);				\
        ps_ += cpybytesSrcMasked;					\
        pd_ += cpybytesMasked;						\
    }									\
    if (m_) {								\
        FFBWRITE(ps_, endMask);						\
        FFBWRITE(pd_, endMask);						\
    }									\
}

#define COPY_ONE_MEMSCR(ffb,psrc,pdst,srcMask,dstMask,wS,wD)	\
{								\
    /* Read source words and stuff them into ffb buffer */	\
    FFBBUFFILL(ffb, psrc, srcMask);				\
    FFBWRITE(pdst, dstMask);					\
    psrc += wS;							\
    pdst += wD;							\
}

#define COPY_MULTIPLE_MEMSCR(ffb, psrcM, pdstM, w, wS, wD,	    \
				startMask, endMask)		    \
{								    \
    CommandWord         ones_ = FFBCOPYALL1;			    \
    int                 m_;					    \
    Pixel8 *ps_, *pd_;						    \
								    \
    ps_ = psrcM;/* both guaranteed to be aligned now */		    \
    pd_ = pdstM;						    \
    FFBBUFFILLALL(ffb, psrcM);					    \
    FFBWRITE(pdstM, startMask);					    \
    for (m_ = w - 2*FFBCOPYPIXELS; m_ > 0; m_ -= FFBCOPYPIXELS) {   \
	ps_ += FFBSRCCOPYBYTESDONE;				    \
        pd_ += FFBCOPYBYTESDONE;				    \
	CYCLE_REGS(ffb);					    \
        FFBBUFFILLALL(ffb, ps_);				    \
        FFBWRITE(pd_, ones_);					    \
    }								    \
    ps_ += FFBSRCCOPYBYTESDONE;					    \
    pd_ += FFBCOPYBYTESDONE;					    \
    CYCLE_REGS(ffb);						    \
    FFBBUFFILLALL(ffb, ps_);					    \
    FFBWRITE(pd_, endMask);					    \
								    \
    psrcM += wS;						    \
    pdstM += wD;						    \
}

#define COPY_ONE_SCRMEM(ffb,psrc,pdstM,srcMask,dstMask,bitsmask,wS,wD)	\
{                                                               \
    /* Read source words and stuff them into ffb buffer */      \
    FFBWRITE(psrc, srcMask);                                   	\
    FFBBUFDRAIN(ffb, pdstM, dstMask, bitsmask);               	\
    psrc += wS;                                             	\
    pdstM += wD;                                             	\
}

#define COPY_MULTIPLE_SCRMEM(ffb, psrc, pdstM, width, wS, wD,		\
                                startMask, endMask, bitsmask)		\
{									\
    CommandWord         ones_ = FFBCOPYALL1;				\
    int                 m_;						\
    Pixel8 *ps_, *pd_;							\
									\
    ps_ = psrc;/* both guaranteed to be aligned now */			\
    pd_ = pdstM;							\
    FFBWRITE(psrc, ones_);						\
    FFBBUFDRAIN(ffb, pdstM, startMask, bitsmask);			\
    for (m_ = width - 2*FFBCOPYPIXELS; m_ > 0; m_ -= FFBCOPYPIXELS) {   \
        ps_ += FFBSRCCOPYBYTESDONE;					\
        pd_ += FFBCOPYBYTESDONE;					\
	FFBWRITE(ps_, ones_);						\
        FFBBUFDRAINALL(ffb, pd_, bitsmask);				\
    }									\
    ps_ += FFBSRCCOPYBYTESDONE;						\
    pd_ += FFBCOPYBYTESDONE;						\
    FFBWRITE(ps_, endMask);						\
    FFBBUFDRAIN(ffb, pd_, endMask, bitsmask);				\
									\
    psrc += wS;								\
    pdstM += wD;							\
}

/****************************************************************************
 *               Macros to Copy Data Using Simple Mode                      *
****************************************************************************/

#define     FFBSIMPLEALL1 0x0f
  
extern  CommandWord ffbSimpleAll1;

#define FFBLEFTSIMPLEMASK(align, ones) \
    ((ones) << ((align) & FFBBUSBYTESMASK))
#define FFBRIGHTSIMPLEMASK(alignedWidth, ones) \
    ((ones) >> (-(alignedWidth) & FFBBUSBYTESMASK))

#ifndef USE_64
#define READMAINMEM4 							\
	sB = *((Pixel32 *)psrc);					\
        sC = *((Pixel32 *)(psrc + 4));					\
        sD = *((Pixel32 *)(psrc + 8));					\
        sE = *((Pixel32 *)(psrc + 12))					

#define READMAINMEM8                                                    \
        READMAINMEM4;                                                   \
	sF = *((Pixel32 *)(psrc + 16));					\
        sG = *((Pixel32 *)(psrc + 20));					\
        sH = *((Pixel32 *)(psrc + 24));					\
        sI = *((Pixel32 *)(psrc + 28))
  
#define SIMPLE_COPY_COMPUTE_AND_WRITE4					\
        dB = (sA >> crotate) | (sB << rotate);				\
        FFBWRITE((Pixel32 *)pdst, dB);					\
        dB = (sB >> crotate) | (sC << rotate);				\
        FFBWRITE((Pixel32 *)(pdst + 4 * FFBUNPACKED), dB);		\
        dB = (sC >> crotate) | (sD << rotate);				\
        FFBWRITE((Pixel32 *)(pdst + 8 * FFBUNPACKED), dB);		\
        dB = (sD >> crotate) | (sE << rotate);				\
        FFBWRITE((Pixel32 *)(pdst + 12 * FFBUNPACKED), dB)
 
#define SIMPLE_COPY_COMPUTE_AND_WRITE8					\
	SIMPLE_COPY_COMPUTE_AND_WRITE4;					\
        dB = (sE >> crotate) | (sF << rotate);				\
        FFBWRITE((Pixel32 *)(pdst + 16 * FFBUNPACKED), dB);		\
        dB = (sF >> crotate) | (sG << rotate);				\
        FFBWRITE((Pixel32 *)(pdst + 20 * FFBUNPACKED), dB);		\
        dB = (sG >> crotate) | (sH << rotate);				\
        FFBWRITE((Pixel32 *)(pdst + 24 * FFBUNPACKED), dB);		\
        dB = (sH >> crotate) | (sI << rotate);				\
        FFBWRITE((Pixel32 *)(pdst + 28 * FFBUNPACKED), dB)
#else
#define READMAINMEM4                                                    \
        sB = *((Pixel64 *)psrc);                                        \
        sC = *((Pixel64 *)(psrc + 8));

#define READMAINMEM8                                                    \
        READMAINMEM4;                                                   \
        sD = *((Pixel64 *)(psrc + 16));                                 \
        sE = *((Pixel64 *)(psrc + 24));
 
#define SIMPLE_COPY_COMPUTE_AND_WRITE4                                  \
        dB = (sA >> crotate) | (sB << rotate);                          \
        FFBWRITE64((Pixel64 *)pdst, dB);                                \
        dB = (sB >> crotate) | (sC << rotate);                          \
        FFBWRITE64((Pixel64 *)(pdst + 8 * FFBUNPACKED),dB);

#define SIMPLE_COPY_COMPUTE_AND_WRITE8                                  \
        SIMPLE_COPY_COMPUTE_AND_WRITE4;                                 \
        dB = (sC >> crotate) | (sD << rotate);                          \
        FFBWRITE64((Pixel64 *)(pdst + 16 * FFBUNPACKED), dB);           \
        dB = (sD >> crotate) | (sE << rotate);                          \
        FFBWRITE64((Pixel64 *)(pdst + 24 * FFBUNPACKED), dB);
#endif    

 /* for extern declarations */
#include "ffbcopy.h"

#endif /* FFBBLT_H */



/*
 * HISTORY
 */
