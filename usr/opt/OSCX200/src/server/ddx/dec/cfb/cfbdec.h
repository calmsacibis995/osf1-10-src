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
/****************************************************************************
**                                                                          *
**                       COPYRIGHT (c)  1991 BY                             *
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

#ifdef MITR5

/*
	NOTE:

	This file defines macros used for the DEC cfb code.  For the
	macros defined in server/ddx/dec/cfb/cfbmskbits.h, MIT R5 uses
	a new set of macros with the generic cfb code in
	server/ddx/cfb/cfbmskbits.h.  If and when the new macros are used,
	this file may be obsolete.

	Caution: some definitions may be required by sfb code
*/

#define Pixel8 		unsigned char
#define Pixel32		unsigned int
#define Bits8		unsigned char
#define Bits32		unsigned int
#define Bool8       	unsigned char

/*
 * This replicates an 8-bit pixel across all 4 bytes
 ** This is from the ultrix version of cfbmskbits.h  **
 */
#define Pixel8To32(pixel)	    \
{				    \
    register Pixel32 tpixel_;       \
    tpixel_ = pixel & 0xff;	    \
    tpixel_ |= ((tpixel_) << 8);    \
    tpixel_ |= ((tpixel_) << 16);   \
    pixel = tpixel_;		    \
} /* Pixel8To32 */

/* A mod operator that works even when the dividend is negative */
#define SafeMod(result, dividend, divisor)  \
{					    \
    result = (dividend) % (divisor);	    \
    if (result < 0) result += (divisor);    \
}

#ifndef VMS
extern Bits32 cfbbitendtab[];
#endif

#ifndef VMS
#ifndef mips
extern Bits32 cfbloadright[];
extern Bits32 cfbloadleft[];
extern Bits32 cfbstoreright[];
extern Bits32 cfbstoreleft[];
#endif
#endif

/* Lots of times we compute values to rotate a word.  Since the MIPS
   architecture ignores all but the bottom 5 bits of a shift quantity, we can
   really sleaze and not bother masking off the high bits.  However, you really
   MUST be careful to use the computed left and right parameters ONLY with
   the shifts << and >>.  If you use them for any sort of computation, you 
   probably want to mask them explicitly yourself. */

#ifdef mips
#define ComputeRotateAmounts(right, left, rotaterightamount)    \
{								\
    right = (rotaterightamount);				\
    left = -(right);						\
} /* ComputeRotateAmounts */
#else
#define ComputeRotateAmounts(right, left, rotaterightamount)    \
{								\
    right = (rotaterightamount) & 31;				\
    left = 32 - (right);					\
} /* ComputeRotateAmounts */
#endif


#ifdef mips

/* StoreWordRight and StoreWordLeft implemented directly in loadstore.s. */

#define LoadWordRight(data, base)   data = LWR(base)

#define LoadWordLeft(data, base)    data = LWL(base)

#else /* mips */

#define StoreWordRight(data, base)				            \
{									    \
    /* Store right bytes of data into offset(base), offset+1(base), etc. */ \
    register Pixel32 *ibase;						    \
    register int offset;						    \
									    \
    ibase = (Pixel32 *) (base);						    \
    offset = ((int) ibase) & 3;						    \
    ibase = (Pixel32 *) (((long) (ibase)) & ~3L);			    \
    *ibase = (*ibase & cfbstoreright[offset])				    \
	      | (((Pixel32) data) << (offset * 8));			    \
} /* StoreWordRight */

#define StoreWordLeft(data, base)				    	    \
{									    \
    /* Store left bytes of data into offset(base), offset-1(base), etc. */  \
    register Pixel32 *ibase;						    \
    register int offset;						    \
									    \
    ibase = (Pixel32 *) (base);						    \
    offset = ((int) ibase) & 3;						    \
    ibase = (Pixel32 *) (((long) (ibase)) & ~3L);			    \
    *ibase = (*ibase & cfbstoreleft[offset])				    \
	      | (((Pixel32) data) >> ((3-offset) * 8));			    \
} /* StoreWordLeft */

#define LoadWordRight(data, base)				            \
{									    \
    /* Load offset(base), offset+1(base), etc. into right bytes of data. */ \
    register Pixel32 *ibase;						    \
    register int offset;						    \
									    \
    ibase = (Pixel32 *) (base);						    \
    offset = ((int) ibase) & 3;						    \
    ibase = (Pixel32 *) (((long) (ibase)) & ~3L);			    \
    data = (data & cfbloadright[offset]) | (*ibase >> (offset * 8));	    \
} /* LoadWordRight */

#define LoadWordLeft(data, base)				    	    \
{									    \
    /* Load offset(base), offset-1(base), etc. into left bytes of data. */  \
    register Pixel32 *ibase;						    \
    register int offset;						    \
									    \
    ibase = (Pixel32 *) (base);						    \
    offset = ((int) ibase) & 3;						    \
    ibase = (Pixel32 *) (((long) (ibase)) & ~3L);			    \
    data = (data & cfbloadleft[offset]) | (*ibase << ((3-offset) * 8));	    \
} /* LoadWordLeft */

#endif /* mips */

/*

The following DCxxx and DFxxx macro definition are usually used only indirectly.

All ``fill' painting routines use the generic functions CFBFILL, CFBFILLLEFT,
and CFBFILLRIGHT, which the Imakefile defines as DFxxx, DFxxxLEFT, and
DFxxxRIGHT.  Each file is compiled 3 times, with xxx replaced by COPY, 
XOR, and GENERAL.

CopyArea uses the generic functions CFBCOPY, CFBCOPYLEFT, and CFBCOPYRIGHT,
which the Imakefile defines as DCxxx, DCxxxLEFT, and DCxxxRIGHT.  The bitblt.c
ile is compiled 4 times, with xxx replaced by COPY, COPYSPM, XOR, and GENERAL.

What a hassle, eh?

*/

/* DFCOPY definitions */
#define DFCOPY(pdst, andbits, xorbits) 			\
	*(pdst) = xorbits

#define DFCOPYLEFT(pdst, andbits, xorbits) 		\
	StoreWordLeft(xorbits, pdst)

#define DFCOPYRIGHT(pdst, andbits, xorbits) 		\
	StoreWordRight(xorbits, pdst)



/* DFXOR definitions */
#define DFXOR(pdst, andbits, xorbits) 			\
	*(pdst) ^= (xorbits)

#define DFXORLEFT(pdst, andbits, xorbits) 		\
{							\
	register Pixel32 t;				\
	LoadWordLeft(t, pdst);				\
	t ^= (xorbits);					\
	StoreWordLeft(t, pdst);				\
}

#define DFXORRIGHT(pdst, andbits, xorbits) 		\
{							\
	register Pixel32 t;				\
	LoadWordRight(t, pdst);				\
	t ^= (xorbits);					\
	StoreWordRight(t, pdst);			\
}


/* DFGENERAL definitions */
#define DFGENERAL(pdst, andbits, xorbits) 		\
	*(pdst) = ((*(pdst)) & (andbits)) ^ (xorbits)

#define DFGENERALLEFT(pdst, andbits, xorbits) 		\
{							\
	register Pixel32 t;				\
	LoadWordLeft(t, pdst);				\
	t = (t & (andbits)) ^ (xorbits);		\
	StoreWordLeft(t, pdst);				\
}

#define DFGENERALRIGHT(pdst, andbits, xorbits) 		\
{							\
	register Pixel32 t;				\
	LoadWordRight(t, pdst);				\
	t = (t & (andbits)) ^ (xorbits);		\
	StoreWordRight(t, pdst);			\
}


/* DCCOPY definitions */
#define DCCOPY(src, pdst, andbits1, xorbits1, andbits2, xorbits2)	\
	*(pdst) = src

#define DCCOPYLEFT(src, pdst, andbits1, xorbits1, andbits2, xorbits2)	\
	StoreWordLeft(src, pdst)

#define DCCOPYRIGHT(src, pdst, andbits1, xorbits1, andbits2, xorbits2)	\
	StoreWordRight(src, pdst)


/* DCCOPYSPM definitions */
#define DCCOPYSPM(src, pdst, andbits1, xorbits1, andbits2, xorbits2) 	\
	*(pdst) = ((*(pdst)) & (xorbits1)) ^ ((src) & (andbits2))


#define DCCOPYSPMLEFT(src, pdst, andbits1, xorbits1, andbits2, xorbits2)\
{									\
	register Pixel32 t;						\
	LoadWordLeft(t, pdst);						\
	t = (t & (xorbits1)) ^ ((src) & (andbits2));			\
	StoreWordLeft(t, pdst);						\
}

#define DCCOPYSPMRIGHT(src, pdst, andbits1, xorbits1, andbits2, xorbits2)\
{									\
	register Pixel32 t;						\
	LoadWordRight(t, pdst);						\
	t = (t & (xorbits1)) ^ ((src) & (andbits2));			\
	StoreWordRight(t, pdst);					\
}


/* DCXOR definitions */
#define DCXOR(src, pdst, andbits1, xorbits1, andbits2, xorbits2)	\
	*(pdst) ^= (((src) & (andbits2)) ^ (xorbits2))

#define DCXORLEFT(src, pdst, andbits1, xorbits1, andbits2, xorbits2) 	\
{									\
	register Pixel32 t;						\
	LoadWordLeft(t, pdst);						\
	t ^= (((src) & (andbits2)) ^ (xorbits2));			\
	StoreWordLeft(t, pdst);						\
}

#define DCXORRIGHT(src, pdst, andbits1, xorbits1, andbits2, xorbits2) 	\
{									\
	register Pixel32 t;						\
	LoadWordRight(t, pdst);						\
	t ^= (((src) & (andbits2)) ^ (xorbits2));			\
	StoreWordRight(t, pdst);					\
}


/* DCGENERAL definitions */
#define DCGENERAL(src, pdst, andbits1, xorbits1, andbits2, xorbits2)	\
{									\
	*(pdst) = (*(pdst) & (((src) & (andbits1)) ^ (xorbits1)))	\
		^ (((src) & (andbits2)) ^ (xorbits2));			\
}

#define DCGENERALLEFT(src, pdst, andbits1, xorbits1, andbits2, xorbits2)\
{									\
	register Pixel32 t;						\
	LoadWordLeft(t, pdst);						\
	t = (t & (((src) & (andbits1)) ^ (xorbits1)))			\
		^ (((src) & (andbits2)) ^ (xorbits2));			\
	StoreWordLeft(t, pdst);						\
}

#define DCGENERALRIGHT(src, pdst, andbits1, xorbits1, andbits2, xorbits2)\
{									\
	register Pixel32 t;						\
	LoadWordRight(t, pdst);						\
	t = (t & (((src) & (andbits1)) ^ (xorbits1)))			\
		^ (((src) & (andbits2)) ^ (xorbits2));			\
	StoreWordRight(t, pdst);					\
}

/*
  This section was moved from ddx/dec/cfb/cfb.h
*/

typedef void (* VoidProc) ();		/*  Procedure returning void   */

/*** should this be in servermd.h?   ***/
#ifndef DS_5000
#define DS_5000		20
#endif
#ifndef DS_3100
#define DS_3100		16
#endif

#ifndef VMS
extern int ws_cpu;
#define isPMAX (ws_cpu == DS_3100)
#endif

/* Derived raster ops: gc.alu is reduced to one of these */
typedef unsigned char DXop;

#define DXcopy		0
#define DXcopySPM       1
#define DXxor		2
#define DXgeneral	3

/* Parameters to general derived raster functions */

typedef struct {
    Pixel32 andbits;
    Pixel32 xorbits;
} RopBits;
    
/* private fields of GC */
typedef RopBits     *FGBGMap;

typedef struct _FGBG {
    struct _FGBG    *next;
    unsigned char   hash;
    Pixel32	    key;
    long int	    refcount;
    RopBits	    map[16];
} FGBGRec, *FGBG;

#ifndef VMS
/* Write fg byte where there is a 1 in srcbits, for bottom 4 bits of srcbits. */
/* Assume that p is word-aligned, fg byte is replicated across all 4 bytes. */

#define Splat4FGBits(p, srcbits, andbits, xorbits)		\
{								\
    switch ((srcbits) & 0xf) {					\
	case 15:						\
	    CFBFILL((int *) (p), andbits, xorbits);		\
	    break;						\
	case 14: /* ||| Need access to partial word stores */	\
	    CFBFILL(p+1, andbits, xorbits);			\
	    CFBFILL((short *) (p+2), andbits, xorbits);		\
	    break;						\
	case 13:						\
	    CFBFILL(p, andbits, xorbits);			\
	    /* fall through */					\
	case 12:						\
	    CFBFILL((short *) (p+2), andbits, xorbits);		\
	    break;						\
	case 10:						\
	    CFBFILL(p+1, andbits, xorbits);			\
	    /* fall through */					\
	case 8:							\
	    CFBFILL(p+3, andbits, xorbits);			\
	    break;						\
	case 7: /* ||| Need access to partial word stores */	\
	    CFBFILL((short *) (p), andbits, xorbits);		\
	    CFBFILL(p+2, andbits, xorbits);			\
	    break;						\
	case 5:							\
	    CFBFILL(p, andbits, xorbits);			\
	    /* fall through */					\
	case 4:							\
	    CFBFILL(p+2, andbits, xorbits);			\
	    break;						\
	case 11:						\
	    CFBFILL(p+3, andbits, xorbits);			\
	    /* fall through */					\
	case 3:							\
	    CFBFILL((short *) (p), andbits, xorbits);		\
	    break;						\
	case 6:							\
	    CFBFILL(p+2, andbits, xorbits);			\
	    /* fall through */					\
	case 2:							\
	    CFBFILL(p+1, andbits, xorbits);			\
	    break;						\
	case 9:							\
	    CFBFILL(p+3, andbits, xorbits);			\
	    /* fall through */					\
	case 1:							\
	    CFBFILL(p, andbits, xorbits);			\
	    break;						\
	case 0:							\
	    break;						\
    }								\
} /* Splat4FGBits */

/* Write fg byte where there is a 1 in srcbits, 4 bits at a time till done. */
/* Assume that p is word-aligned, fg byte is replicated across all 4 bytes. */
/* p and srcbits are destroyed.						    */

#define SplatAllFGBits(p, srcbits, andbits, xorbits)	\
{							\
    while (srcbits) {					\
	Splat4FGBits(p, srcbits, andbits, xorbits);	\
	p += 4;						\
	srcbits >>= 4;					\
    } /* end while srcbits */				\
} /* SplatAllFGBits */

/* Defines for painting opaque stipples.  Paint first four bits using normal
   array indexing, then paint the rest using byte arithmetic to get rid of
   one unneeded shift. */

#define FirstFourOSBits(pdst, byteOffset, srcbits, map)			    \
{									    \
    RopBits *ropbits;							    \
    ropbits = &map[srcbits & 0xf];					    \
    CFBFILL((int *)(pdst+byteOffset), ropbits->andbits, ropbits->xorbits);  \
    srcbits >>= 1; /* Leave setup for faster indexing into map */	    \
} /* FirstFourOSBits */

#define NextFourOSBits(pdst, byteOffset, srcbits, map)			    \
{									    \
    RopBits *ropbits;							    \
    ropbits = (RopBits *) ((char *) map + (srcbits & 0x78));		    \
    CFBFILL((int *)(pdst+byteOffset), ropbits->andbits, ropbits->xorbits);  \
    srcbits >>= 4;							    \
} /* NextFourOSBits */

#define LastFourOSBits(pdst, byteOffset, srcbits, map)			    \
{									    \
    RopBits *ropbits;							    \
    ropbits = (RopBits *) ((char *) map + (srcbits & 0x78));		    \
    CFBFILL((int *)(pdst+byteOffset), ropbits->andbits, ropbits->xorbits);  \
} /* LastFourOSBits */

/* If we know we have 29 srcbits or less to paint, we don't need to paint
   the first word specially.  Instead, just shift srcbits left by the
   appropriate amount */
#define SetupOSBits(srcbits)    (srcbits) <<= 3

/* Paint the last 1-3 ragged right opaque stipple bits */
#define RaggedRightOSBits(pdst, rightbytes, srcbits, map)		    \
{									    \
    RopBits *ropbits;							    \
    srcbits <<= (4 - rightbytes);					    \
    ropbits = (RopBits *) ((char *) map + (srcbits & 0x78));		    \
    CFBFILLLEFT(pdst+rightbytes-1, ropbits->andbits, ropbits->xorbits);	    \
} /* RaggedRightOSBits */

#endif   /* not VMS */
/*
  End of section was moved from ddx/dec/cfb/cfb.h
*/

#endif   /*  MITR5  */
