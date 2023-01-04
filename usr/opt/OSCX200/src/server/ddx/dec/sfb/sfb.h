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
**      This module is used to define macros and constants used in
**	the SFB modules.
**
**  AUTHORS:
**
**      Tracy Bragdon (from Joel McCormack)
**
**  CREATION DATE:     19-Nov-1991
**
**  MODIFICATION HISTORY:
**
** X-006	BIM0011		Irene McCartney			27-Jan-1992
**		Merge changes from Joel's latest code.
**
** X-001	DMC0002		Dave Coleman			20-Dec-1991
**		Change conditional to define DS_* if undefined.
**
**		Add edit history.
**
**--
**/

/****************************************************************************
 *                  Parameterization of Smart Frame Buffer                  *
 ***************************************************************************/

/*

At the moment (Feb 1991) there is one sfb chip, which accepts 32 bits from the
bus, has a 64 bit interface to VRAM, and supports pixels that are either 8, 16,
or 32 bits deep.  However, I envision many different kinds of sfb chips, so as
much of that variation as possible is parameterized here.  Perhaps one day you
can just change these parameters and everything will work, but the only things
I'm guaranteeing now are the existing chip, plus the same design with a 32 bit
interface to VRAM.

Below are the variations I envision having some chance of ever being built.
Different devices are separated by blank lines; if there are multiple lines
together, it means a single sfb chip supports serveral choices of bits/pixel.
(All pixels in a given configuration must be the same size.)

The masks provided to copy mode, stipple mode, and the pixel mask register are
always packed; one bit in the mask represents one pixel in memory.

``Pixel bits'' is the number of bits per pixel.

``Copy bits'' is the number of bits in a command word that are processed in
copy mode.

``Stipple bits'' is the number of bits in a command word that are processed in
opaque or transparent stipple mode.

``Line bits'' is the number of bits in a command word that are processed in
opaque or transparent stipple line mode.

``Bus bits'' is the number of bits on the bus that sfb pays attention to in
dumb frame buffer mode, and when reading or writing the internal copy buffer.

``VRAM bits'' is the number of bits in the path to VRAM; accelerated modes must
use this alignment for the address they provide the smart frame buffer.

``StartPixels'' is the number of pixels in a VRAM 

``Meg'' is the minimum megabytes of VRAM that the sfb can talk to, assuming
256kx4-bit wide chips.

PixelBits  CopyBits  StippleBits  LineBits  BusBits  VRAMBits  Meg  StartPixels
---------  --------  -----------  --------  -------  --------  ---  -----------
    8	      32	 32	     16        32	 32     1      2048?
   16         16         32          16        32        32     2      
   32	       8	 16	     16	       32	 32     4      

    8	      32	 32	     16        32	 64     2      4096
   16         16         32          16        32        64     4      4096
   32	       8	 16	     16	       32	 64     8      4096

   32	      32	 32          16        32       128	4      ?


The include file sfbparams.h should be a dynamic link to an actual file like
sfbparams8x32x32x64.h (8 pixel bits, 32 stipple, 32 bus bits, 64 VRAM bits),
sfbparams32x16x32x64.h, or sfbparams8x32x32x32.  sfbparams.h defines the
constants:

    SFBPIXELBITS
    SFBSTIPPLEBITS
    SFBCOPYBITS
    SFBLINEBITS
    SFBBUSBITS
    SFBVRAMBITS
    SFBSTARTPIXELS

*/

#include "sfbparams.h"

#define PSZ		 SFBPIXELBITS
#include "cfb.h"
#ifdef MITR5
#include "cfbdec.h"
#endif	/* MITR5 */

#ifndef ExpandPixel
#define ExpandPixel(pixel)      pixel
#define CompressPixel(pixel)    pixel
#endif

/* Derive a bunch of constants from the given constants. */

#define SFBPIXELBYTES    (SFBPIXELBITS / 8)

#if SFBPIXELBITS == 8
typedef Pixel8		    OnePixel;
#define SFBPIXELALL1	    0x000000ff
#define SFBBYTESTOPIXELS(n) /* Nothing */
#define SFBLINESHIFT	    (16 + 0)

#elif SFBPIXELBITS == 16
typedef Pixel16		    OnePixel;
#define SFBPIXELALL1	    0x0000ffff
#define SFBBYTESTOPIXELS(n) (n) /= SFBPIXELBYTES
#define SFBLINESHIFT	    (16 + 1)

#elif SFBPIXELBITS == 32
typedef Pixel32		    OnePixel;
#define SFBPIXELALL1	    0xffffffff
#define SFBBYTESTOPIXELS(n) (n) /= SFBPIXELBYTES
#define SFBLINESHIFT	    (16 + 2)
#endif

#define SFBSTIPPLEBITSMASK  (SFBSTIPPLEBITS - 1)
#define SFBSTIPPLEBYTESDONE (SFBSTIPPLEBITS * SFBPIXELBYTES)

#define SFBCOPYBITSMASK     (SFBCOPYBITS - 1)
#define SFBCOPYBYTESDONE    (SFBCOPYBITS * SFBPIXELBYTES)

#define SFBLINEBITSMASK	    (SFBLINEBITS - 1)

#define SFBBUSBITSMASK      (SFBBUSBITS - 1)
#define SFBBUSBYTES	    (SFBBUSBITS / 8)
#define SFBBUSBYTESMASK     (SFBBUSBYTES - 1)
#define SFBBUSPIXELS	    (SFBBUSBITS / SFBPIXELBITS)

#define SFBALIGNMENT	    (SFBVRAMBITS / 8)
#define SFBALIGNMASK	    (SFBALIGNMENT - 1)

#define SFBPIXELALIGNMENT   (SFBALIGNMENT / SFBPIXELBYTES)
#define SFBPIXELALIGNMASK   (SFBALIGNMASK / SFBPIXELBYTES)

#ifdef SLEAZOID32
#define SFBSLEAZEMULTIPLIER SFBPIXELBYTES
#define SFBSLEAZEBYTESDONE  SFBCOPYBITS
#define SFBSLEAZEPIXELBYTES 1
#define SFBSLEAZEALIGNMASK  (SFBALIGNMASK / SFBPIXELBYTES)
#else
#define SFBSLEAZEMULTIPLIER 1
#define SFBSLEAZEBYTESDONE  SFBCOPYBYTESDONE
#define SFBSLEAZEPIXELBYTES SFBPIXELBYTES
#define SFBSLEAZEALIGNMASK  SFBALIGNMASK
#endif

#if SFBBUSBITS == 32
#define SFBBUSALL1  ((CommandWord)0xffffffff)
# if SFBPIXELBITS == 8 || defined(SLEAZOID32)
# define PixelToPixelWord(pixel) Pixel8To32(pixel)
# elif SFBPIXELBITS == 16
# define PixelToPixelWord(pixel) Pixel16To32(pixel)
# elif SFBPIXELBITS == 32
# define PixelToPixelWord(pixel) /* Nothing */
# endif

#elif SFBBUSBITS == 64
#define SFBBUSALL1  ((CommandWord)0xffffffffffffffff)
# if SFBPIXELBITS == 8
# define PixelToPixelWord(pixel) Pixel8To64(pixel)
# elif SFBPIXELBITS == 16
# define PixelToPixelWord(pixel) Pixel16To64(pixel)
# elif SFBPIXELBITS == 32
# define PixelToPixelWord(pixel) Pixel32To64(pixel)
# elif SFBPIXELBITS == 64
# define PixelToPixelWord(pixel) /* Nothing */
# endif
#endif


/****************************************************************************
 *                 Smart Frame Buffer Register Definitions                  *
 ***************************************************************************/

#include "sfbregs.h"

/****************************************************************************
 *                    Smart Frame Buffer Register Macros                    *
 ***************************************************************************/

/* Macros for writing to command registers. */
#ifdef SOFTWARE_MODEL
#include "defs.h"
#include "types.h"
extern BYTES *vram;

# if SFBBUSBITS == 32
# define LWMASK 0xf
# elif SFBBUSBITS == 64
# define LWMASK 0xff
# endif

#define SFBBUFREAD(sfb, pos, src) \
    src = BusRead(CPYBF0_ADDRESS + (pos*SFBBUSBYTES))

# if SFBVRAMBITS/SFBBUSBITS == 1
# define SFBBUFWRITE(sfb, pos, src) \
    BusWrite(CPYBF0_ADDRESS + (pos*SFBBUSBYTES), src, LWMASK)
# elif SFBVRAMBITS/SFBBUSBITS == 2
/* Must always write a pair of words for them to actually get into buffer. */
# define SFBBUFWRITE(sfb, pos, src0, src1)				    \
{									    \
    BusWrite(CPYBF0_ADDRESS + (pos*SFBBUSBYTES), src0, LWMASK);		    \
    BusWrite(CPYBF0_ADDRESS + (pos*SFBBUSBYTES)+SFBBUSBYTES, src1, LWMASK); \
} /* SFBBUFWRITE */
# elif SFBVRAMBITS/SFBBUSBITS == 4
/* Must always write four words for them to actually get into buffer. */
#define SFBBUFWRITE(sfb, pos, src0, src1, src2, src3)			      \
{									      \
    BusWrite(CPYBF0_ADDRESS + (pos*SFBBUSBYTES), src0, LWMASK);		      \
    BusWrite(CPYBF0_ADDRESS + (pos*SFBBUSBYTES)+SFBBUSBYTES, src1, LWMASK);   \
    BusWrite(CPYBF0_ADDRESS + (pos*SFBBUSBYTES)+2*SFBBUSBYTES, src2, LWMASK); \
    BusWrite(CPYBF0_ADDRESS + (pos*SFBBUSBYTES)+3*SFBBUSBYTES, src3, LWMASK); \
} /* SFBBUFWRITE */
# elif SFBVRAMBITS/SFBBUSBITS == 8
/* Must always write eight words for them to actually get into buffer. */
# define SFBBUFWRITE(sfb, pos, src0, src1, src2, src3, src4, src5, src6, src7) \
{									      \
    BusWrite(CPYBF0_ADDRESS + (pos*SFBBUSBYTES), src0, LWMASK);		      \
    BusWrite(CPYBF0_ADDRESS + (pos*SFBBUSBYTES)+SFBBUSBYTES, src1, LWMASK);   \
    BusWrite(CPYBF0_ADDRESS + (pos*SFBBUSBYTES)+2*SFBBUSBYTES, src2, LWMASK); \
    BusWrite(CPYBF0_ADDRESS + (pos*SFBBUSBYTES)+3*SFBBUSBYTES, src3, LWMASK); \
    BusWrite(CPYBF0_ADDRESS + (pos*SFBBUSBYTES)+4*SFBBUSBYTES, src4, LWMASK); \
    BusWrite(CPYBF0_ADDRESS + (pos*SFBBUSBYTES)+5*SFBBUSBYTES, src5, LWMASK); \
    BusWrite(CPYBF0_ADDRESS + (pos*SFBBUSBYTES)+6*SFBBUSBYTES, src6, LWMASK); \
    BusWrite(CPYBF0_ADDRESS + (pos*SFBBUSBYTES)+7*SFBBUSBYTES, src7, LWMASK); \
} /* SFBBUFWRITE */
# endif

#define SFBFOREGROUND(sfb, data) BusWrite(FG_ADDRESS, ExpandPixel(data), LWMASK)
#define SFBBACKGROUND(sfb, data) BusWrite(BG_ADDRESS, ExpandPixel(data), LWMASK)
#define SFBPLANEMASK(sfb, data)	\
    BusWrite(PLANEMASK_ADDRESS, ExpandPixel(data), LWMASK)
#define SFBPIXELMASK(sfb, data)		BusWrite(PIXMSK_ADDRESS, data, LWMASK)
#define SFBMODE(sfb, data)		BusWrite(MODE_ADDRESS, data, LWMASK)
#define SFBROP(sfb, data)		BusWrite(BOOLOP_ADDRESS, data, LWMASK)
#define SFBSHIFT(sfb, data)		BusWrite(PIXSHFT_ADDRESS, data, LWMASK)
#define SFBADDRESS(sfb, data)		BusWrite(ADDRREG_ADDRESS, data, LWMASK)
#define SFBBRES1(sfb, data)		BusWrite(BRES1_ADDRESS, data, LWMASK)
#define SFBBRES2(sfb, data)		BusWrite(BRES2_ADDRESS, data, LWMASK)
#define SFBBRES3(sfb, data)		BusWrite(BRES3_ADDRESS, data, LWMASK)
#define SFBBRESCONTINUE(sfb, data)	BusWrite(BCONT_ADDRESS, data, LWMASK)
#define SFBDEEP(sfb, data)		BusWrite(DEEP_ADDRESS, data, LWMASK)
#define SFBDEEPREAD(sfb, data)		data = BusRead(DEEP_ADDRESS)
#define SFBSTART(sfb, data)		BusWrite(START_ADDRESS, data, LWMASK)

#else /* SOFTWARE_MODEL */

#define SFBBUFREAD(sfb, pos, src)    src = sfb->buffer[pos]

# if SFBVRAMBITS/SFBBUSBITS == 1
# define SFBBUFWRITE(sfb, pos, src)      sfb->buffer[pos]   = src
# elif SFBVRAMBITS/SFBBUSBITS == 2
/* Must always write a pair of words for them to actually get into buffer. */
# define SFBBUFWRITE(sfb, pos, src0, src1)		      \
{							      \
    sfb->buffer[pos] = src0;				      \
    sfb->buffer[pos+1] = src1;				      \
} /* SFBBUFWRITE */
# elif SFBVRAMBITS/SFBBUSBITS == 4
/* Must always write four words for them to actually get into buffer. */
# define SFBBUFWRITE(sfb, pos, src0, src1, src2, src3)	      \
{							      \
    sfb->buffer[pos] = src0;				      \
    sfb->buffer[pos+1] = src1;				      \
    sfb->buffer[pos+2] = src2;				      \
    sfb->buffer[pos+3] = src3;				      \
} /* SFBBUFWRITE */
# elif SFBVRAMBITS/SFBBUSBITS == 8
/* Must always write eight words for them to actually get into buffer. */
# define SFBBUFWRITE(sfb, pos, src0, src1, src2, src3, src4, src5, src6, src7)\
{							      \
    sfb->buffer[pos] = src0;				      \
    sfb->buffer[pos+1] = src1;				      \
    sfb->buffer[pos+2] = src2;				      \
    sfb->buffer[pos+3] = src3;				      \
    sfb->buffer[pos+4] = src4;				      \
    sfb->buffer[pos+5] = src5;				      \
    sfb->buffer[pos+6] = src6;				      \
    sfb->buffer[pos+7] = src7;				      \
} /* SFBBUFWRITE */
# endif

#ifdef __alpha
#  ifdef VMS
#    include <builtins.h>     	/* __MB(void) */
#    define WBFLUSH()		__MB
#    define SFBSCRPRIVOFF	scrPriv->offset
     extern int FFSS();
#  else /* OSF */
#    include <c_asm.h>		/* define asm() */
#    define WBFLUSH()		(void)asm("mb")
#    define SFBSCRPRIVOFF	0
#    define FFSS(word)		ffs(word)
     extern int ffs();
#  endif /* VMS/OSF */
   /* SFB can be aliased 8 times at 512 byte intervals */
#  define CYCLE_SFB_RESET	~0x1000L	/* reset to base address */
#  define CYCLE_SFB_INCREMENT	 0x200L		/* increment address */
#  define CYCLE_SFB_MASK	~0xe00L		/* alias bits */
#  define CYCLE_REGS(sfb)	\
	(sfb) = (SFB)((((long)sfb) + CYCLE_SFB_INCREMENT) & CYCLE_SFB_RESET)

   /* frame buffer can be aliased 8 times at 4M intervals.
    * This is only for base sfb, must be changed for sfb+
    * 32M space restriction exists for built in hx on flamingo
    * while 128M could be used for turbo channel options.
    * We restrict to 32M to provide for flamingo.
    */
#  define CYCLE_FB_RESET	~0x2000000L	/* reset to base address */
#  define CYCLE_FB_INCREMENT	 0x400000L	/* increment address */
#  define CYCLE_FB_MASK	  	~0x1c00000	/* alias bits */
#  define CYCLE_FB(p) 						   \
    (Pixel8 *)(((unsigned long)(p) +  CYCLE_FB_INCREMENT) &  CYCLE_FB_RESET)
#  define CYCLE_AND_SET_FB(p)	(p = CYCLE_FB(p))
   /* arc code (sfbzerarc) needs to cycle 8 different address variables
    * and keep them all from conflicting. The easiet way to do this
    * (and neatest) is to or each with a current cycle mask
    */
#  define CYCLE_FB_GLOBAL_DECL register Pixel8 * cycle_fb_mask = 0L
#  define CYCLE_FB_GLOBAL(p) 					   \
    (Pixel8 *)(((unsigned long)(p) | 				   \
    (unsigned long)CYCLE_AND_SET_FB(cycle_fb_mask)))

#else /* __alpha */
#    define WBFLUSH		/*-noop-*/
#    define SFBSCRPRIVOFF	0
#    define FFSS(word)		ffs(word)
     extern int ffs();
#    define CYCLE_REGS(sfb)
#    define CYCLE_FB(p) 	(p)
#    define CYCLE_AND_SET_FB(p)	(p)
#    define CYCLE_FB_GLOBAL_DECL 
#    define CYCLE_FB_GLOBAL(p) 	(p)
#endif /* __alpha */

#define SFBFOREGROUND(sfb, data)	sfb->foreground = data
#define SFBBACKGROUND(sfb, data)	sfb->background = data
#define SFBPLANEMASK(sfb, data)		sfb->planemask  = data
#define SFBPIXELMASK(sfb, data)		sfb->pixelmask  = data;CYCLE_REGS(sfb)
#define SFBMODE(sfb, data)		sfb->mode       = data;CYCLE_REGS(sfb)
#define SFBROP(sfb, data)		sfb->rop	= data
#define SFBSHIFT(sfb, data)		sfb->shift      = data;CYCLE_REGS(sfb)
#define SFBADDRESS(sfb, data)		\
	sfb->address    = (Pixel32)((int)data & CYCLE_FB_MASK);\
	CYCLE_REGS(sfb)
#define SFBBRES1(sfb, data)		sfb->bres1      = data
#define SFBBRES2(sfb, data)		sfb->bres2      = data
#define SFBBRES3(sfb, data)		sfb->bres3      = data;CYCLE_REGS(sfb)
#define SFBBRESCONTINUE(sfb, data)	sfb->brescont   = data;CYCLE_REGS(sfb)
#define SFBDEEP(sfb, data)		sfb->depth      = data
#define SFBDEEPREAD(sfb, data)		data		= sfb->depth
#define SFBSTART(sfb, data)		sfb->start      = data;CYCLE_REGS(sfb)
#endif /* SOFTWARE_MODEL */

/* Macros for reading and writing data to frame buffer portion of sfb */
#ifdef SOFTWARE_MODEL
# define SFBREAD(psrc, data)		data = BusRead(psrc)

# define SFBPIXELREAD(psrc, data)		    \
{						    \
    int align_;					    \
    align_ = (int)(psrc) & SFBBUSBYTESMASK;	    \
    data = BusRead(psrc) >> (align_ * 8);	    \
    data &= SFBPIXELALL1;			    \
}

# define SFBWRITE(pdst, data)		BusWrite(pdst, data, LWMASK)
# define SFBWRITE_CYCLE(pdst, data)	SFBWRITE(pdst, data)

# if SFBPIXELBITS == 8
# define SFBPIXELWRITE(pdst, data)					\
{									\
    int align_;								\
    align_ = (int)(pdst) & SFBBUSBYTESMASK;				\
    BusWrite((pdst) - align_, (data) << (align_ * 8), 1 << align_);    \
}
# elif SFBPIXELBITS == 16
# define SFBPIXELWRITE(pdst, data)					\
{									\
    int align_;								\
    align_ = ((int)(pdst) & SFBBUSBYTESMASK) >> 1;			\
    BusWrite((pdst) - align_, (data) << (align_ * 16), 3 << align_);    \
}
# elif (SFBPIXELBITS == 32) && (SFBBUSBITS > 32)
# define SFBPIXELWRITE(pdst, data)					\
{									\
    int align_;								\
    align_ = ((int)(pdst) & SFBBUSBYTESMASK) >> 2;			\
    BusWrite((pdst) - align_, (data) << (align_ * 32), 15 << align_);   \
}
# elif SFBPIXELBITS == SFBBUSBITS
# define SFBPIXELWRITE(pdst, data)      SFBWRITE(pdst, data)
# endif

#else /* SOFTWARE_MODEL */

#define SFBREAD(psrc, data)		\
	WBFLUSH();data = *((volatile PixelWord *)(psrc));
#define SFBWRITE(pdst, data)		*((volatile PixelWord *)(pdst)) = data
#define SFBWRITE_CYCLE(pdst, data)	SFBWRITE(CYCLE_AND_SET_FB(pdst), data)

#define SFBPIXELREAD(psrc, data)	\
	WBFLUSH();data = *((volatile OnePixel *) (psrc));
#define SFBPIXELWRITE(pdst, data)	*((volatile OnePixel *) (pdst)) = data

#define SFBSTOREWORDLEFT(data, base)    StoreWordLeft(data, base)
#define SFBSTOREWORDRIGHT(data, base)   StoreWordRight(data, base)
#endif



/****************************************************************************
 *                         CPU Hardware Definitions                         *
 ***************************************************************************/

#include "cpu.h"



/****************************************************************************
 *                         Private per-Screen Data                          *
 ***************************************************************************/

/* Private screen-specific information about sfb: we need a pointer to the
   device register so we can talk to the hardware, a pointer to the last GC
   that was loaded into the hardware so that we know when to load a new 
   hardware state, and information about the off-screen memory supported by
   the sfb so we can stash the pixmap in there.  If multiple screens supported
   by a single chip, then all such screens point to same private sfb record. */
   
extern int sfbScreenPrivateIndex;

typedef struct FreeElement_ {
	struct FreeElement_ *link;
	int size;
} FreeElement;

typedef struct {
    SFB		    sfb;		/* Pointer to sfb registers	      */
    GCPtr	    lastGC;		/* Pointer to last GC loaded into sfb */
    pointer	    firstScreenMem;     /* First byte of screen memory        */
    pointer	    lastScreenMem;      /* Past last byte of screen memory    */
    FreeElement	    avail;		/* Free list for off-screen memory    */
    FreeElement	    *rover;		/* Roving pointer in free list	      */
#ifdef VMS
	/*
	 * For VMS, the frame buffer is not necessarily on an 8 meg boundary,
	 * so adjust the address to make it look like it is (scrPriv->offset
	 * contains the offset to the nearest 8meg boundary).
	 */
    int		    offset;		/* offset from frame buffer to     */
					/* nearest 8Meg boundary           */
#endif					   
} sfbScreenPrivRec, *sfbScreenPrivPtr;

#define SFBSCREENPRIV(pScreen) \
    ((sfbScreenPrivPtr)(pScreen->devPrivates[sfbScreenPrivateIndex].ptr))

#define SCREENMEMORY(pPixmap)						    \
    (   SFBSCREENPRIV((pPixmap)->drawable.pScreen)->firstScreenMem <=       \
	    (pPixmap)->devPrivate.ptr					    \
     && (pPixmap)->devPrivate.ptr <					    \
	SFBSCREENPRIV((pPixmap)->drawable.pScreen)->lastScreenMem)

#define MAINMEMORY(pPixmap)   (!SCREENMEMORY(pPixmap))



/****************************************************************************
 *                           Private per-GC Data                            *
 ***************************************************************************/

extern int  sfbGCPrivateIndex;

/* We need to know if last GC validation was for sfb or cfb.  If we
   cross-dress, we need to fill in EVERYTHING in the ops record from scratch;
   we can't make use of ANY of the existing procedures.  Note that this also
   means that both sfb and cfb need to fill in even those procedures that they
   never change, and so might never bother to set after GC initialization. */
   
typedef struct {
    Bool8	    lastValidateWasSFB; /* Did we or cfb validate gc?       */
    unsigned char   dashAdvance;	/* How much SFBLINEBITS advances    */
    unsigned int    dashLength; 	/* Length (in bits) of dash pattern */
    Bits32	    dashPattern;	/* Expanded dash bit pattern	    */
    void	    (* FillArea) ();	/* FillArea routine                 */
} sfbGCPrivRec, *sfbGCPrivPtr;

#define SFBGCPRIV(pGC) ((sfbGCPrivPtr)(pGC->devPrivates[sfbGCPrivateIndex].ptr))
#ifdef MITR5
#define PRIVFILLAREA sfbGCPriv->FillArea
#else
#define PRIVFILLAREA gcPriv->FillAREA
#endif



/****************************************************************************
 *                     Useful Macros for Talking to SFB                     *
 ***************************************************************************/

/*
   All macros that declare local variables always append an underscore_, so
   that they won't ever be confused with the idiotic C ``parameters'' being
   substituted.
*/



/****************************************************************************
 *                   Write Exactly One Word on a Scanline                 *
 ***************************************************************************/

#ifdef TLBFAULTS
/* We evidently have something narrow, so we assume it'll be cheaper to do two
   writes--to the address and continue registers--rather than one write to the
   frame buffer which may mean an expensive TLB miss. */

#define SFBWRITEONEWORD(sfb, pdst, data)	\
{						\
    SFBADDRESS(sfb, pdst);			\
    SFBSTART(sfb, data);			\
} /* SFBWRITEONEWORD */
#else
#define SFBWRITEONEWORD(sfb, pdst, data)    SFBWRITE(pdst, data)
#endif



/****************************************************************************
 *                     Ensure Hardware State Matches GC                     *
 ***************************************************************************/

#define LOADSTATE(scrPriv, sfb, pGC)		\
{						\
    SFBFOREGROUND(sfb, pGC->fgPixel);		\
    SFBBACKGROUND(sfb, pGC->bgPixel);		\
    SFBPLANEMASK(sfb, pGC->planemask);		\
    SFBROP(sfb, pGC->alu);			\
    scrPriv->lastGC = pGC;			\
} /* LOADSTATE */
    
#define CHECKSTATE(pScreen, scrPriv, sfb, pGC)  \
{						\
    scrPriv = SFBSCREENPRIV(pScreen);		\
    sfb = scrPriv->sfb;				\
    if (pGC != scrPriv->lastGC) {		\
	LOADSTATE(scrPriv, sfb, pGC);		\
    }						\
} /* CHECKSTATE */



/****************************************************************************
 *          Compute Masks for Left and Right Edges of a Bus Word            *
 ***************************************************************************/

extern CommandWord sfbBusAll1;

#if defined(MODULOSHIFTS) && (SFBBUSBITS == WORDBITS)
#define SFBLEFTBUSMASK(align, ones)	    ((ones) << (align))
#define SFBRIGHTBUSMASK(alignedWidth, ones) ((ones) >> -(alignedWidth))

#else /* use longer sequences */

#define SFBLEFTBUSMASK(align, ones) \
    ((ones) << ((align) & SFBBUSBITSMASK))
#define SFBRIGHTBUSMASK(alignedWidth, ones) \
    ((ones) >> (-(alignedWidth) & SFBBUSBITSMASK))
#endif



/****************************************************************************
 *         Compute Masks for Left and Right Edges of a Stipple Span         *
 ***************************************************************************/

extern CommandWord sfbStippleAll1;

#if SFBSTIPPLEBITS == 16
#define SFBSTIPPLEALL1  ((CommandWord)0xffff)
#elif SFBSTIPPLEBITS == 32
#define SFBSTIPPLEALL1  ((CommandWord)0xffffffff)
#elif SFBSTIPPLEBITS == 64
#define SFBSTIPPLEALL1  ((CommandWord)0xffffffffffffffff)
#endif

#if defined(MODULOSHIFTS) && (SFBSTIPPLEBITS == WORDBITS)
#define SFBLEFTSTIPPLEMASK(align, ones)		((ones) << (align))
#define SFBRIGHTSTIPPLEMASK(alignedWidth, ones) ((ones) >> -(alignedWidth))

#else /* use longer sequences */

#define SFBLEFTSTIPPLEMASK(align, ones) \
    ((ones) << ((align) & SFBSTIPPLEBITSMASK))
#define SFBRIGHTSTIPPLEMASK(alignedWidth, ones) \
    ((ones) >> (-(alignedWidth) & SFBSTIPPLEBITSMASK))
#endif

/* Computation of right shift amount when stippling a word of BUSBITS into
   pixels, ala text and 32-bit stipples. */
#if defined(MODULOSHIFTS) && (WORDBITS == SFBSTIPPLEBITS)
#define SFBRIGHTSTIPPLESHIFT(align)    (-(align))
#else
#define SFBRIGHTSTIPPLESHIFT(align)    (SFBSTIPPLEBITS - (align))
#endif



/****************************************************************************
 *         Compute Masks for Left and Right Edges of a Copied Span          *
 ***************************************************************************/

extern CommandWord sfbCopyAll1;

#if   SFBCOPYBITS == 8
#define SFBCOPYALL1 ((CommandWord)0xff)
#elif SFBCOPYBITS == 16
#define SFBCOPYALL1  ((CommandWord)0xffff)
#elif SFBCOPYBITS == 32
#define SFBCOPYALL1  ((CommandWord)0xffffffff)
#elif SFBCOPYBITS == 64
#define SFBCOPYALL1  ((CommandWord)0xffffffffffffffff)
#endif

#if defined(MODULOSHIFTS) && (SFBCOPYBITS == WORDBITS)
#define SFBLEFTCOPYMASK(align, ones)		(((ones) << (align)) & (ones))
#define SFBRIGHTCOPYMASK(alignedWidth, ones)    ((ones) >> -(alignedWidth))

#else /* use longer sequences */

# if SFBCOPYBITS == 8
/* Copy mode isn't smart enough to through away high-order bits of mask if
   limited to 8 iterations in 32 bits/pixel mode. */
#define SFBLEFTCOPYMASK(align, ones) \
    (((ones) << ((align) & SFBCOPYBITSMASK)) & (ones))
# else
#define SFBLEFTCOPYMASK(align, ones) \
    ((ones) << ((align) & SFBCOPYBITSMASK))
# endif
#define SFBRIGHTCOPYMASK(alignedWidth, ones) \
    ((ones) >> (-(alignedWidth) & SFBCOPYBITSMASK))
#endif

/* Computation of masks for left and right edges when copying backwards */
extern unsigned sfbBackRightMask[];
extern unsigned sfbBackLeftMask[];
#define SFBBACKLEFTCOPYMASK(alignedWidth) \
    sfbBackLeftMask[(alignedWidth) & SFBCOPYBITSMASK]
#define SFBBACKRIGHTCOPYMASK(align) \
    sfbBackRightMask[(align) + SFBALIGNMENT/SFBPIXELBYTES]

/* Extra data bits to avoid race condition on copies that require a single
   read from the source. */

#if SFBVRAMBITS == 32
#define SFBRACECOPYMASK 0xf0
#elif SFBVRAMBITS == 64
#define SFBRACECOPYMASK 0xff00
#endif


/****************************************************************************
 *                              Paint a Point                               *
 ***************************************************************************/

#ifdef PARTIALWRITES
# define SFBPOINT(pdst, foreground) SFBPIXELWRITE(pdst, ExpandPixel(foreground))
#else
/* Assume foreground already loaded, mode is TRANSPARENTSTIPPLE, and depth is
   properly set. */
/* In addition, on an alpha, be careful with this. If you write to the same
 * quadword more than once, only the last write will work which is not what
 * you want. Not all uses of SFBPOINT require flushing, but many do.
 */
#define SFBPOINT(pdst, foreground)	    \
{					    \
    register int align_;		    \
    align_ = ((int)(pdst)) & SFBALIGNMASK;  \
    SFBWRITE((pdst)-align_, 1 << align_);   \
} /* SFBPOINT */
#endif



/****************************************************************************
 *                           Paint a Solid Span                             *
 ***************************************************************************/

#ifdef TLBFAULTS
#define SFBSOLIDSPAN(sfb, pdst, widthInPixels, ones)	\
{							\
    int align_, width_;					\
    CommandWord leftMask_, rightMask_;			\
    Pixel8  *p_;					\
							\
    p_ = (pdst);					\
    align_ = (int)p_ & SFBALIGNMASK;			\
    p_  -= align_;					\
    SFBBYTESTOPIXELS(align_);				\
    width_ = (widthInPixels) + align_;			\
    leftMask_ = SFBLEFTSTIPPLEMASK(align_, ones);	\
    rightMask_ = SFBRIGHTSTIPPLEMASK(width_, ones);	\
    if (width_ <= SFBSTIPPLEBITS) {			\
	SFBADDRESS(sfb, p_);				\
	SFBSTART(sfb, leftMask_ & rightMask_);		\
    } else {						\
	SFBWRITE(p_, leftMask_);			\
	width_ -= 2*SFBSTIPPLEBITS;			\
	while (width_ > 0) {				\
	    p_ += SFBSTIPPLEBYTESDONE;			\
	    SFBWRITE(p_, (ones));			\
	    width_ -= SFBSTIPPLEBITS;			\
	}						\
	SFBWRITE(p_+SFBSTIPPLEBYTESDONE, rightMask_);   \
    }							\
} /* SFBSOLIDSPAN */
#else
#define SFBSOLIDSPAN(sfb, pdst, widthInPixels, ones)	\
{							\
    int align_, width_;					\
    CommandWord leftMask_, rightMask_;			\
    Pixel8  *p_;					\
							\
    p_ = (pdst);					\
    align_ = (int)p_ & SFBALIGNMASK;			\
    p_  -= align_;					\
    SFBBYTESTOPIXELS(align_);				\
    width_ = (widthInPixels) + align_;			\
    leftMask_ = SFBLEFTSTIPPLEMASK(align_, ones);	\
    rightMask_ = SFBRIGHTSTIPPLEMASK(width_, ones);	\
    if (width_ <= SFBSTIPPLEBITS) {			\
	SFBWRITE(p_, leftMask_ & rightMask_);		\
    } else {						\
	SFBWRITE(p_, leftMask_);			\
	width_ -= 2*SFBSTIPPLEBITS;			\
	while (width_ > 0) {				\
	    p_ += SFBSTIPPLEBYTESDONE;			\
	    SFBWRITE(p_, (ones));			\
	    width_ -= SFBSTIPPLEBITS;			\
	}						\
	SFBWRITE(p_+SFBSTIPPLEBYTESDONE, rightMask_);	\
    }							\
} /* SFBSOLIDSPAN */
#endif



/****************************************************************************
 *                    Copy Data from Main Memory to SFB                     *
 ***************************************************************************/

/* These routines depend upon the fact that that read mask never has too many
   leading 0's...that is, we'll always read the first word. */

#if SFBVRAMBITS/SFBBUSBITS == 1
#define SFBBUFFILL(sfb, psrc, mask)		    \
{						    \
    register Pixel8      *p_;			    \
    register CommandWord mask_;			    \
    register int	 i_;			    \
						    \
    p_ = (psrc);				    \
    mask_ = (mask);				    \
    i_ = 0;					    \
    do {					    \
	SFBBUFWRITE(sfb, i_, ((PixelWord *)p_)[0]); \
	p_ += SFBALIGNMENT;			    \
	mask_ >>= SFBPIXELALIGNMENT;		    \
	i_ += SFBVRAMBITS/SFBBUSBITS;		    \
    } while (mask_);				    \
    CYCLE_REGS(sfb);				    \
} /* SFBBUFFILL */

# if SFBBUFFERWORDS == 8
# define SFBBUFFILLALL(sfb, psrc)		    \
{						    \
    SFBBUFWRITE(sfb, 0, ((PixelWord *)(psrc))[0]);  \
    SFBBUFWRITE(sfb, 1, ((PixelWord *)(psrc))[1]);  \
    SFBBUFWRITE(sfb, 2, ((PixelWord *)(psrc))[2]);  \
    SFBBUFWRITE(sfb, 3, ((PixelWord *)(psrc))[3]);  \
    SFBBUFWRITE(sfb, 4, ((PixelWord *)(psrc))[4]);  \
    SFBBUFWRITE(sfb, 5, ((PixelWord *)(psrc))[5]);  \
    SFBBUFWRITE(sfb, 6, ((PixelWord *)(psrc))[6]);  \
    SFBBUFWRITE(sfb, 7, ((PixelWord *)(psrc))[7]);  \
    CYCLE_REGS(sfb);				    \
} /* SFBBUFFILLALL */
# endif

#elif  SFBVRAMBITS/SFBBUSBITS == 2

# ifdef SLEAZOID32

# define SFBBUFFILL(sfb, psrc, mask)		    \
{						    \
    register Pixel8      *p_;			    \
    register CommandWord mask_;			    \
    register int	 i_;			    \
						    \
    p_ = (psrc);				    \
    mask_ = (mask);				    \
    i_ = 0;					    \
    do {					    \
	SFBBUFWRITE(sfb, i_, ExpandPixel(p_[0]),    \
			     ExpandPixel(p_[1]));   \
	p_ += SFBALIGNMENT/SFBPIXELBYTES;	    \
	mask_ >>= SFBPIXELALIGNMENT;		    \
	i_ += SFBVRAMBITS/SFBBUSBITS;		    \
    } while (mask_);				    \
    CYCLE_REGS(sfb);				    \
} /* SFBBUFFILL */

#  if SFBBUFFERWORDS == 8
#  define SFBBUFFILLALL(sfb, psrc)		\
{						\
    register PixelWord a_, b_, c_, d_,		\
		       e_, f_, g_, h_;		\
    a_ = ExpandPixel(psrc[0]);			\
    b_ = ExpandPixel(psrc[1]);			\
    c_ = ExpandPixel(psrc[2]);			\
    d_ = ExpandPixel(psrc[3]);			\
    e_ = ExpandPixel(psrc[4]);			\
    f_ = ExpandPixel(psrc[5]);			\
    g_ = ExpandPixel(psrc[6]);			\
    h_ = ExpandPixel(psrc[7]);			\
    SFBBUFWRITE(sfb, 0, a_, b_);		\
    SFBBUFWRITE(sfb, 2, c_, d_);		\
    SFBBUFWRITE(sfb, 4, e_, f_);		\
    SFBBUFWRITE(sfb, 6, g_, h_);		\
    CYCLE_REGS(sfb);				    \
} /* SFBBUFFILLALL */
#  endif
# else
# define SFBBUFFILL(sfb, psrc, mask)			\
{							\
    register CommandWord mask_;				\
							\
    SFBBUFWRITE(sfb, 0,					\
	((PixelWord *)(psrc))[0],			\
	((PixelWord *)(psrc))[1]);			\
    mask_ = (mask) >> SFBPIXELALIGNMENT;		\
    if (mask_) {					\
	SFBBUFWRITE(sfb, 2,				\
	    ((PixelWord *)(psrc))[2],			\
	    ((PixelWord *)(psrc))[3]);			\
	mask_ >>= SFBPIXELALIGNMENT;			\
	if (mask_) {					\
	    SFBBUFWRITE(sfb, 4,				\
		((PixelWord *)(psrc))[4],		\
		((PixelWord *)(psrc))[5]);		\
	    mask_ >>= SFBPIXELALIGNMENT;		\
	    if (mask_) {				\
		SFBBUFWRITE(sfb, 6,			\
		    ((PixelWord *)(psrc))[6],		\
		    ((PixelWord *)(psrc))[7]);		\
	    }						\
	}						\
    }							\
    CYCLE_REGS(sfb);				    \
} /* SFBBUFFILL */
#  if SFBBUFFERWORDS == 8
#  define SFBBUFFILLALL(sfb, psrc)		\
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
    SFBBUFWRITE(sfb, 0, a_, b_);		\
    SFBBUFWRITE(sfb, 2, c_, d_);		\
    SFBBUFWRITE(sfb, 4, e_, f_);		\
    SFBBUFWRITE(sfb, 6, g_, h_);		\
    CYCLE_REGS(sfb);				    \
} /* SFBBUFFILLALL */
#  endif
# endif
#endif


/****************************************************************************
 *               Macros to Copy Data from SFB to Main Memory                *
 ***************************************************************************/


#if SFBBUSBITS == 32
# if SFBPIXELBITS == 8
#  ifdef SOFTWARE_MODEL
#  define SFBBUFDRAIN(sfb, pdst, mask)			\
{							\
    register Pixel8	    *p_;			\
    register CommandWord    mask_, tmask_;		\
    register PixelWord	    src_;			\
    register int	    i_;				\
							\
    SFBFASTFLUSH();					\
    p_ = (pdst);					\
    mask_ = (mask);					\
    i_ = 0;						\
    do {						\
	SFBBUFREAD(sfb, i_, src_);			\
	tmask_ = mask_ & 0xf;				\
	if (tmask_ == 0xf) {				\
	    /* Full word write */			\
	    MYCFBCOPY(src_, (PixelWord *) p_);		\
	} else if (tmask_ != 0) {			\
	    if (mask_ & 1) MYCFBCOPY(src_, p_);		\
	    if (mask_ & 2) MYCFBCOPY(src_ >> 8, p_+1);  \
	    if (mask_ & 4) MYCFBCOPY(src_ >> 16, p_+2); \
	    if (mask_ & 8) MYCFBCOPY(src_ >> 24, p_+3); \
	}						\
	p_ += SFBBUSBYTES;				\
	mask_ >>= SFBBUSPIXELS;				\
	i_++;						\
    } while (mask_);					\
} /* SFBBUFDRAIN */
#  else /* SOFTWARE_MODEL */

#  define SFBBUFDRAIN(sfb, pdst, mask)			\
{							\
    register Pixel8	    *p_;			\
    register CommandWord    mask_, tmask_;		\
    register PixelWord	    src_;			\
    register volatile PixelWord *psrc_;			\
							\
    SFBFASTFLUSH();					\
    p_ = (pdst);					\
    mask_ = (mask);					\
    psrc_ = sfb->buffer;				\
    do {						\
	src_ = *psrc_;					\
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
	p_ += SFBBUSBYTES;				\
	mask_ >>= SFBBUSPIXELS;				\
	psrc_++;					\
    } while (mask_);					\
    CYCLE_REGS(sfb);					\
} /* SFBBUFDRAIN */
#  endif /* SOFTWARE_MODEL */
# elif SFBPIXELBITS == 16
# define SFBBUFDRAIN(sfb, pdst, mask)					\
{									\
    register Pixel8	    *p_;					\
    register CommandWord    mask_, tmask_;				\
    register volatile PixelWord	    src_;				\
    register int	    i_;						\
									\
    SFBFASTFLUSH();							\
    p_ = (pdst);							\
    mask_ = (mask);							\
    i_ = 0;								\
    do {								\
	SFBBUFREAD(sfb, i_, src_);					\
	tmask_ = mask_ & 0x3;						\
	if (tmask_ == 0x3) {						\
	    /* Full word write */					\
	    MYCFBCOPY(src_, (volatile PixelWord *) p_);			\
	} else if (tmask_ != 0) {					\
	    if (mask_ & 0x1) MYCFBCOPY(src_, (Pixel16 *) p_);		\
	    if (mask_ & 0x2) MYCFBCOPY(src_ >> 16, (Pixel16 *)(p_+2));  \
	}								\
	p_ += SFBBUSBYTES;						\
	mask_ >>= SFBBUSPIXELS;						\
	i_++;								\
    } while (mask_);							\
    CYCLE_REGS(sfb);							\
} /* SFBBUFDRAIN */
# elif SFBPIXELBITS == 32
#  ifdef SLEAZOID32
# define SFBBUFDRAIN(sfb, pdst, mask)			\
{							\
    register Pixel8	    *p_;			\
    register CommandWord    mask_, tmask_;		\
    register PixelWord	    src_;			\
    register int	    i_;				\
							\
    SFBFASTFLUSH();					\
    p_ = (pdst);					\
    mask_ = (mask);					\
    i_ = 0;						\
    do {						\
	SFBBUFREAD(sfb, i_, src_);			\
	src_ = CompressPixel(src_);			\
	tmask_ = mask_ & 0x1;				\
	if (tmask_ != 0) {				\
	    /* Full ``word'' write */			\
	    MYCFBCOPY(src_, p_);			\
	}						\
	p_ += SFBBUSBYTES/SFBSLEAZEMULTIPLIER;		\
	mask_ >>= SFBBUSPIXELS;				\
	i_++;						\
    } while (mask_);					\
    CYCLE_REGS(sfb);					\
} /* SFBBUFDRAIN */
#  else
# define SFBBUFDRAIN(sfb, pdst, mask)			\
{							\
    register Pixel8	    *p_;			\
    register CommandWord    mask_, tmask_;		\
    register volatile PixelWord	    src_;		\
    register int	    i_;				\
							\
    SFBFASTFLUSH();					\
    p_ = (pdst);					\
    mask_ = (mask);					\
    i_ = 0;						\
    do {						\
	SFBBUFREAD(sfb, i_, src_);			\
	tmask_ = mask_ & 0x1;				\
	if (tmask_ != 0) {				\
	    /* Full word write */			\
	    MYCFBCOPY(src_, (volatile PixelWord *) p_);	\
	}						\
	p_ += SFBBUSBYTES;				\
	mask_ >>= SFBBUSPIXELS;				\
	i_++;						\
    } while (mask_);					\
    CYCLE_REGS(sfb);					\
} /* SFBBUFDRAIN */
#  endif
# endif
#endif

#if SFBBUFFERWORDS == 8
# ifdef SLEAZOID32
#define SFBBUFDRAINALL(sfb, pdst)				\
{								\
    register PixelWord src_;					\
								\
    SFBFASTFLUSH();						\
    SFBBUFREAD(sfb, 0, src_);					\
    src_ = CompressPixel(src_);					\
    MYCFBCOPY(src_, pdst + 0*SFBBUSBYTES/SFBSLEAZEMULTIPLIER);  \
    SFBBUFREAD(sfb, 1, src_);					\
    src_ = CompressPixel(src_);					\
    MYCFBCOPY(src_, pdst + 1*SFBBUSBYTES/SFBSLEAZEMULTIPLIER);  \
    SFBBUFREAD(sfb, 2, src_);					\
    src_ = CompressPixel(src_);					\
    MYCFBCOPY(src_, pdst + 2*SFBBUSBYTES/SFBSLEAZEMULTIPLIER);  \
    SFBBUFREAD(sfb, 3, src_);					\
    src_ = CompressPixel(src_);					\
    MYCFBCOPY(src_, pdst + 3*SFBBUSBYTES/SFBSLEAZEMULTIPLIER);  \
    SFBBUFREAD(sfb, 4, src_);					\
    src_ = CompressPixel(src_);					\
    MYCFBCOPY(src_, pdst + 4*SFBBUSBYTES/SFBSLEAZEMULTIPLIER);  \
    SFBBUFREAD(sfb, 5, src_);					\
    src_ = CompressPixel(src_);					\
    MYCFBCOPY(src_, pdst + 5*SFBBUSBYTES/SFBSLEAZEMULTIPLIER);  \
    SFBBUFREAD(sfb, 6, src_);					\
    src_ = CompressPixel(src_);					\
    MYCFBCOPY(src_, pdst + 6*SFBBUSBYTES/SFBSLEAZEMULTIPLIER);  \
    SFBBUFREAD(sfb, 7, src_);					\
    src_ = CompressPixel(src_);					\
    MYCFBCOPY(src_, pdst + 7*SFBBUSBYTES/SFBSLEAZEMULTIPLIER);  \
    CYCLE_REGS(sfb);						\
} /* SFBBUFDRAINALL */
# else
#define SFBBUFDRAINALL(sfb, pdst)			    		\
{							    		\
    register PixelWord src_;				    		\
							    		\
    SFBFASTFLUSH();					    		\
    SFBBUFREAD(sfb, 0, src_);				    		\
    MYCFBCOPY(src_, (volatile PixelWord *)((pdst) + 0*SFBBUSBYTES)); 	\
    SFBBUFREAD(sfb, 1, src_);				    		\
    MYCFBCOPY(src_, (volatile PixelWord *)((pdst) + 1*SFBBUSBYTES)); 	\
    SFBBUFREAD(sfb, 2, src_);				    		\
    MYCFBCOPY(src_, (volatile PixelWord *)((pdst) + 2*SFBBUSBYTES)); 	\
    SFBBUFREAD(sfb, 3, src_);				    		\
    MYCFBCOPY(src_, (volatile PixelWord *)((pdst) + 3*SFBBUSBYTES)); 	\
    SFBBUFREAD(sfb, 4, src_);				    		\
    MYCFBCOPY(src_, (volatile PixelWord *)((pdst) + 4*SFBBUSBYTES)); 	\
    SFBBUFREAD(sfb, 5, src_);				    		\
    MYCFBCOPY(src_, (volatile PixelWord *)((pdst) + 5*SFBBUSBYTES)); 	\
    SFBBUFREAD(sfb, 6, src_);				    		\
    MYCFBCOPY(src_, (volatile PixelWord *)((pdst) + 6*SFBBUSBYTES)); 	\
    SFBBUFREAD(sfb, 7, src_);				    		\
    MYCFBCOPY(src_, (volatile PixelWord *)((pdst) + 7*SFBBUSBYTES)); 	\
    CYCLE_REGS(sfb);							\
} /* SFBBUFDRAINALL */
# endif
#endif


/****************************************************************************
 *                   Macros to Load Drawable Information                    *
 ***************************************************************************/

/* ||| This should really be moved into cfb and used there, too. */

#define DrawableBaseAndWidthPlus(pDraw, pBase, width, windowCode, pixCode)  \
{									    \
    if (pDraw->type == DRAWABLE_WINDOW) {				    \
	windowCode;							    \
	pBase = (Pixel8 *)						    \
	    (((PixmapPtr)(pDraw->pScreen->devPrivate))->devPrivate.ptr);    \
	width = (int)(((PixmapPtr)(pDraw->pScreen->devPrivate))->devKind);  \
    } else {								    \
	pixCode;							    \
	pBase = (Pixel8 *)(((PixmapPtr)pDraw)->devPrivate.ptr);		    \
	width = (int)(((PixmapPtr)pDraw)->devKind);			    \
    }									    \
} /* DrawableBaseAndWidthPlus */

#define DrawableBaseAndWidth(pDraw, pBase, width) \
    DrawableBaseAndWidthPlus(pDraw, pBase, width, /* Nada */, /* Nada */)



/****************************************************************************
 *                  Macro to Stipple a Word of SFBBUSBITS                   *
 ***************************************************************************/

#if SFBSTIPPLEBITS == SFBBUSBITS
#define StippleEntireWord(p, srcbits)					\
{									\
    SFBWRITE(p + 0*SFBSTIPPLEBYTESDONE, srcbits >> 0*SFBSTIPPLEBITS);   \
} /* StippleEntireWord */

#elif 2*SFBSTIPPLEBITS == SFBBUSBITS
#define StippleEntireWord(p, srcbits)					\
{									\
    SFBWRITE(p + 0*SFBSTIPPLEBYTESDONE, srcbits >> 0*SFBSTIPPLEBITS);   \
    SFBWRITE(p + 1*SFBSTIPPLEBYTESDONE, srcbits >> 1*SFBSTIPPLEBITS);   \
} /* StippleEntireWord */

#elif 4*SFBSTIPPLEBITS == SFBBUSBITS
#define StippleEntireWord(p, srcbits)					\
{									\
    SFBWRITE(p + 0*SFBSTIPPLEBYTESDONE, srcbits >> 0*SFBSTIPPLEBITS);   \
    SFBWRITE(p + 1*SFBSTIPPLEBYTESDONE, srcbits >> 1*SFBSTIPPLEBITS);   \
    SFBWRITE(p + 2*SFBSTIPPLEBYTESDONE, srcbits >> 2*SFBSTIPPLEBITS);   \
    SFBWRITE(p + 3*SFBSTIPPLEBYTESDONE, srcbits >> 3*SFBSTIPPLEBITS);   \
} /* StippleEntireWord */

#elif 8*SFBSTIPPLEBITS == SFBBUSBITS
#define StippleEntireWord(p, srcbits)					\
{									\
    SFBWRITE(p + 0*SFBSTIPPLEBYTESDONE, srcbits >> 0*SFBSTIPPLEBITS);   \
    SFBWRITE(p + 1*SFBSTIPPLEBYTESDONE, srcbits >> 1*SFBSTIPPLEBITS);   \
    SFBWRITE(p + 2*SFBSTIPPLEBYTESDONE, srcbits >> 2*SFBSTIPPLEBITS);   \
    SFBWRITE(p + 3*SFBSTIPPLEBYTESDONE, srcbits >> 3*SFBSTIPPLEBITS);   \
    SFBWRITE(p + 4*SFBSTIPPLEBYTESDONE, srcbits >> 4*SFBSTIPPLEBITS);   \
    SFBWRITE(p + 5*SFBSTIPPLEBYTESDONE, srcbits >> 5*SFBSTIPPLEBITS);   \
    SFBWRITE(p + 6*SFBSTIPPLEBYTESDONE, srcbits >> 6*SFBSTIPPLEBITS);   \
    SFBWRITE(p + 7*SFBSTIPPLEBYTESDONE, srcbits >> 7*SFBSTIPPLEBITS);   \
} /* StippleEntireWord */
#endif



/****************************************************************************
 *          Macros for turning packed x and y into separate values          *
 ***************************************************************************/

#define Int32ToX(i)     (i & 0xffff)
#define Int32ToY(i)     ((int)i >> 16)

