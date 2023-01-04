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
 * @(#)$RCSfile: ffbmacros.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:11:48 $
 */
/*
 */
#ifndef FFBMACROS_H
#define FFBMACROS_H

/****************************************************************************
 *                  Parameterization of Smart Frame Buffer                  *
 ***************************************************************************/

/*

As of 1993, there is one ffb+ chip, which accepts 32 bits from the bus, has a
64 bit interface to VRAM, and supports pixels that are 8, 12, or 32 bits deep.
To support other possible chips, several key definitions are parameterized.
Perhaps you can just change these parameters and everything will work.  No
guarantees, though.

Since a single ffb+ graphics system can support multiple pixel depths
simultaneously, some of the parameters come from the Imakefile, so that some .c
files can be compiled up for different depth pixels.  (Many files have been
made depth-independent, so only need be compiled up once for all pixel depths.)

FFBPIXELBITS is the physical number of bits per pixel in memory for the
destination drawable.  8 for packed 8-plane visuals; 32 for unpacked 8-plane
visuals; and 32 for 12-plane and 24-plane visuals.

FFBDEPTHBITS is the logical number of bits per pixel that the ffb+ operates on
for the destination drawable.  8 for 8-plane visuals; 32 for 12-plane and
24-plane visuals.

If FFBPIXELBITS/FFBDEPTHBITS = 4, you have an unpacked 8-bit destination
visual.  Otherwise FFBPIXELBITS/FFBDEPTHBITS = 1.

FFBSRCPIXELBITS is the physical number of bits per pixel in memory for the
source drawable (copies only).

FFBSRCDEPTHBITS is the logical number of bits per pixel that the ffb+ operates
on for the source drawable (copies only).

If FFBSRCPIXELBITS/FFBSRCDEPTHBITS = 4, you have an unpacked 8-bit source
visual.  Otherwise FFBSRCPIXELBITS/FFBSRCDEPTHBITS = 1.

Other parameters come from the ffbparams.h file.

The masks provided to copy mode, stipple mode, and the pixel mask register are
always packed; one bit in the mask represents one pixel in memory.

``Copy bits'' is the number of bits in a command word that are processed in
copy mode.

``Stipple bits'' is the number of bits in a command word that are processed in
opaque or transparent stipple mode.

``Line bits'' is the number of bits in a command word that are processed in
opaque or transparent stipple line mode.

``Bus bits'' is the number of bits on the bus that ffb pays attention to in
dumb frame buffer mode, and when reading or writing the internal copy buffer.

``VRAM bits'' is the number of bits in the path to VRAM; accelerated modes must
use this alignment for the address they provide the smart frame buffer.

``StartPixels'' is the number of pixels in a VRAM 

``Meg'' is the minimum megabytes of VRAM that the ffb can talk to, assuming
256kx4-bit wide chips.

Parameters from the Imakefile:
		 FFBPIXELBITS   FFBSRCPIXELBITS   FFBDEPTHBITS   FFBSRCDEPTHBITS
		 ------------   ---------------   ------------   ---------------
Packed 8-bit	      8
Unpacked 8-bit
12- and 32-bit

FFBPIXELBITS	    8

PixelBits  CopyBits  StippleBits  LineBits  BusBits  VRAMBits  Meg  StartPixels
---------  --------  -----------  --------  -------  --------  ---  -----------
    8	      32	 32	     16        32	 32     1      2048?
   16         16         32          16        32        32     2      
   32	       8	 16	     16	       32	 32     4      

    8	      32	 32	     16        32	 64     2      4096
   16         16         32          16        32        64     4      4096
   32	       8	 16	     16	       32	 64     8      4096

   32	      32	 32          16        32       128	4      ?


The include file ffbparams.h should be a dynamic link to an actual file like
ffbparams8x32x32x64.h (8 pixel bits, 32 stipple, 32 bus bits, 64 VRAM bits),
ffbparams32x16x32x64.h, or ffbparams8x32x32x32.  ffbparams.h defines the
constants:

    FFBPIXELBITS	now (usually) given on command line
    FFBSRCPIXELBITS	now (usually) given on command line
    FFBDEPTHBITS	now (usually) given on command line
    FFBSRCDEPTHBITS	now (usually) given on command line
    FFBSTIPPLEBITS
    FFBCOPYBITS
    FFBLINEBITS
    FFBBUSBITS
    FFBVRAMBITS
    FFBSTARTPIXELS

*/


/* set the capends bit */

#define CAPENDSBIT(capEnds)  ((capEnds) << 15)

/* Derive a bunch of constants from the given constants. */

#define FFBPIXELBYTES    (FFBPIXELBITS / 8)	/* physical bytes/pixel, dst  */
#define FFBDEPTHBYTES	 (FFBDEPTHBITS / 8)     /* logical bytes/pixel, dst   */
#define FFBSRCPIXELBYTES (FFBSRCPIXELBITS / 8)	/* physical bytes/pixel, src  */
#define FFBSRCDEPTHBYTES (FFBSRCDEPTHBITS / 8)	/* logical bytes/pixel, src   */

#if FFBPIXELBITS == 8
typedef Pixel8		    OnePixel;
#define FFBPIXELALL1	    0x000000ff
#define FFBLINESHIFT	    (16 + 0)

#elif FFBPIXELBITS == 32
typedef Pixel32		    OnePixel;
#define FFBPIXELALL1	    0xffffffff
#define FFBLINESHIFT	    (16 + 2)
#endif

#define FFBBYTESTOPIXELS(n)     (n) /= FFBPIXELBYTES
#define FFBSRCBYTESTOPIXELS(n)  (n) /= FFBSRCPIXELBYTES
#define FFBPIXELSTOBYTES(n)     (n) *= FFBPIXELBYTES
#define FFBSRCPIXELSTOBYTES(n)  (n) *= FFBSRCPIXELBYTES

/*
 * Mechanisms used in multi-naming/multi-compilation. If the mips cpp worked
 * we could just use the appropriate compile-time defines directly, 
 * albeit with a level of indirection in the macros.
 */

#if __STDC__ && !defined(UNIXCPP)
#define CAT_NAME2(prfx,subname) prfx##subname
#define CAT_NAME3(prfx,subname,suffix) prfx##subname##suffix
#define CAT_NAME4(prfx,subname,suffix1,suffix2) prfx##subname##suffix1##suffix2
#else
#define CAT_NAME2(prfx,subname) prfx/**/subname
#define CAT_NAME3(prfx,subname,suffix) prfx/**/subname/**/suffix
#define CAT_NAME4(prfx,subname,suffix1,suffix2) prfx/**/subname/**/suffix1/**/suffix2
#endif

#if ((FFBSRCPIXELBITS == 8) && (FFBSRCDEPTHBITS==8) && (FFBPIXELBITS==8) && (FFBDEPTHBITS==8))
#define FFB_COPY_NAME(exp)  CAT_NAME2(ffb8888,exp)
#elif ((FFBSRCPIXELBITS==8) && (FFBSRCDEPTHBITS==8) && (FFBPIXELBITS==32) && (FFBDEPTHBITS==8))
#define FFB_COPY_NAME(exp)  CAT_NAME2(ffb88328,exp)
#elif ((FFBSRCPIXELBITS==32) && (FFBSRCDEPTHBITS==8) && (FFBPIXELBITS==8) && (FFBDEPTHBITS==8))
#define FFB_COPY_NAME(exp)  CAT_NAME2(ffb32888,exp)
#elif ((FFBSRCPIXELBITS==32) && (FFBSRCDEPTHBITS==8) && (FFBPIXELBITS==32) && (FFBDEPTHBITS==8))
#define FFB_COPY_NAME(exp)  CAT_NAME2(ffb328328,exp)
#elif ((FFBSRCPIXELBITS==32) && (FFBSRCDEPTHBITS==32) && (FFBPIXELBITS==32) && (FFBDEPTHBITS==32))
#define FFB_COPY_NAME(exp)  CAT_NAME2(ffb32323232,exp)
#else /* depth independent code */
#define FFB_COPY_NAME(exp) CAT_NAME2(ffb8888,exp)
#endif

#if FFBPIXELBITS==8
#define CFB_NAME(exp)	CAT_NAME2(cfb,exp)
#define FFB_NAME(exp)	CAT_NAME2(ffb8,exp)
#else /* FFBPIXELBITS == 32 */
#define CFB_NAME(exp)	CAT_NAME2(cfb32,exp)
#define FFB_NAME(exp)	CAT_NAME2(ffb32,exp)
#endif




/* end stuff for multi-naming/multi-compilation */

/* rotation and visual field definitions */
 
#define PACKED_EIGHT_DEST       0
#define UNPACKED_EIGHT_DEST     1
#define TWELVE_BIT_DEST         2
#define TWENTYFOUR_BIT_DEST     3

#define PACKED_EIGHT_SRC        0
#define UNPACKED_EIGHT_SRC      1
#define TWELVE_BIT_BUF0_SRC     6 
#define TWELVE_BIT_BUF1_SRC     2 
#define TWENTYFOUR_BIT_SRC      3

#define ROTATE_DESTINATION_0    0
#define ROTATE_DESTINATION_1    1
#define ROTATE_DESTINATION_2    2
#define ROTATE_DESTINATION_3    3


#define ROTATE_SOURCE_0         0
#define ROTATE_SOURCE_1         1
#define ROTATE_SOURCE_2         2
#define ROTATE_SOURCE_3         3

#define ROTATE_DONT_CARE        0

#define DST_VISUAL_SHIFT        8
#define DST_ROTATE_SHIFT        10

#define SRC_VISUAL_SHIFT        8
#define SRC_ROTATE_SHIFT        11
      
/* end rotation and visual field definitions */

#define FFBLINEDXDY(dx, dy)  (((dy) << 16) | (dx))
#define FFBLOADBLOCKDATA(_align,_count) (((_align) << 16) | ((_count) -1))

/*
 * FFBMAX<1><2><3>PIX<4>
 *	<1> := B(LOCK) or F(ILL)
 *	<2> := S(OLID) or P(ATTERNED)
 *	<3> := W(RITE) or R(EAD/WRITE)
 *	<4> := ELS - any
 *	    := 8   - 8-bit packed/unpacked
 *	    := 32  - 32-bit (12/24)
 *
 * Bus timeout: 5uS
 *
 * 8-bit systems and 8-bit unpacked on 32-bit systems:
 * 
 * Any Block Fill   32 pixels       60 nsec			2048
 * Normal Fill       8 pixels       60 nsec			666
 * Xor Normal Fill   8 pixels      180 nsec			222
 * 
 * 8-bit packed on 32-bit systems:
 * 
 * Any Block Fill    INVALID
 * Normal Fill       8 pixels       60 nsec			666
 * Xor Normal Fill   8 pixels      180 nsec			222
 * 
 * 32-bit on 32-bit systems:
 * 
 * Solid Block Fill 32 pixels       60nS mid, 240nS edge	2048
 * Stip Block Fill  32 pixels      240 nsec			666
 * Normal Fill       2 pixels       60 nsec			166
 * Xor Normal Fill   2 pixels      180 nsec			55
 */
#define FFBMAXBSWPIXELS		2048
#define FFBMAXBPWPIXELS		 664
#define FFBMAXFPWPIX8		 664	/* same for solid */
#define FFBMAXFPRPIX8		 220	/* ditto */
#define FFBMAXFPWPIX32		 164	/* ditto */
#define FFBMAXFPRPIX32		  52	/* ditto */

#if FFBPIXELBITS == 8
#define FFBLOADCOLORREGS(ffb, c0, depth)	\
{						\
    FFBCOLOR0(ffb, c0);				\
    FFBCOLOR1(ffb, c0);				\
}
#elif FFBPIXELBITS == 32
#define FFBLOADCOLORREGS(ffb, c0, depth)	\
{						\
    FFBCOLOR0(ffb, c0);				\
    FFBCOLOR1(ffb, c0);				\
    if ((depth) != 8) {				\
        FFBCOLOR2(ffb, c0);			\
        FFBCOLOR3(ffb, c0);			\
        FFBCOLOR4(ffb, c0);			\
        FFBCOLOR5(ffb, c0);			\
        FFBCOLOR6(ffb, c0);			\
        FFBCOLOR7(ffb, c0);			\
    }						\
}
#endif

#define FFBSTIPPLEBITSMASK  (FFBSTIPPLEBITS - 1)
#define FFBSTIPPLEBYTESDONE (FFBSTIPPLEBITS * FFBPIXELBYTES)

#define FFBLINEBITSMASK	    (FFBLINEBITS - 1)

#define FFBBUSBITSMASK      (FFBBUSBITS - 1)
#define FFBBUSBYTES	    (FFBBUSBITS / 8)
#define FFBBUSBYTESMASK     (FFBBUSBYTES - 1)
#define FFBBUSPIXELS	    (FFBBUSBITS / FFBPIXELBITS)

#define FFBSTIPPLEALIGNMASK (FFBSTIPPLEALIGNMENT - 1)
#define FFBCOPYALIGNMASK    (FFBCOPYALIGNMENT - 1)
#define FFBSRCCOPYALIGNMASK (FFBSRCCOPYALIGNMENT - 1)

#if FFBBUSBITS == 32
#define FFBBUSALL1  ((CommandWord)0xffffffff)
#define Pixel8ToPixelWord(pixel) Pixel8To32(pixel)

#elif FFBBUSBITS == 64
#define FFBBUSALL1  ((CommandWord)0xffffffffffffffff)
#define Pixel8ToPixelWord(pixel) Pixel8To64(pixel)
#endif


/****************************************************************************
 *                    Smart Frame Buffer Cycling Macros                     *
 ***************************************************************************/


#if defined(SOFTWARE_MODEL) || defined(WB_MODEL)
#   define READ_MEMORY_BARRIER()    wbMB(LWMASK)
#   define WRITE_MEMORY_BARRIER()   wbMB(LWMASK)    /* WMB with EV4-5 */
#ifndef VMS
#   define CYCLE_FB_INC		0X2000000L	    /* 32 MBytes per alias  */
#else
#   define CYCLE_FB_INC		0X400000L  /* 4 MBytes per alias change to 16 later  */
#endif
#else /* not SOFTWARE_MODEL */
#   if defined(__alpha)
#       ifdef VMS
#	    include <builtins.h>       /* __MB(void) */
#	    define READ_MEMORY_BARRIER() __MB()
#	    define WRITE_MEMORY_BARRIER() __MB()    /* WMB with EV4-5 */
	    extern int FFSS();
#       else
#	    include <c_asm.h>		/*_#pragma asm()_*/
#	    define READ_MEMORY_BARRIER()  (void)asm("mb")
#	    define WRITE_MEMORY_BARRIER()  (void)asm("mb") /* WMB with EV4-5 */
#	    define FFSS(word)		ffs(word)
	    extern int ffs();
#       endif
#   elif defined(mips)
#       define READ_MEMORY_BARRIER() /*-noop-*/
#       define WRITE_MEMORY_BARRIER() /*-noop-*/
#       define FFSS(word)            ffs(word)
	extern int ffs();
#   endif

#ifndef VMS
#   define CYCLE_FB_INC		0x2000000L	    /* 32 MBytes per alias  */
#else
#   define CYCLE_FB_INC		0x400000L	/* 4 MBytes per alias change to 16M later */
#endif
#endif

#if CPU_WB_WORDS == 0
/* Don't need to worry about write buffer merging/reordering */
#   define CYCLE_REGS(ffb)      /* Nothing */
#   define CYCLE_FB(p)		(p)
#   define CYCLE_FB_DOUBLE(p)   (p)
#   define CYCLE_AND_SET_FB(p)  (p)
#   define CYCLE_FB_GLOBAL_DECL /* Nothing */
#   define CYCLE_FB_GLOBAL(p)	(p)
#   define CYCLE_FB_CURR(B,P)
#   define CYCLE_FB_NEXT(B,P)
#   define CYCLE_FB_DSTWIDTH(B,W)

#else
#   define CYCLE_FB_RESET	(~(4*CYCLE_FB_INC))     /* 4 aliases	    */
#   define CYCLE_FB_MASK	(~(3*CYCLE_FB_INC))     /* Junk alias bits  */
#   define CYCLE_FB(p) 	(Pixel8 *)(((long)(p)+CYCLE_FB_INC) & CYCLE_FB_RESET)
#   define CYCLE_FB_DOUBLE(p)                                                \
        (Pixel8 *)(((long)(p)+2*CYCLE_FB_INC) & CYCLE_FB_RESET)
#   define CYCLE_FB_CURR(B,P) ((Pixel8*)(((long)(B) & CYCLE_FB_MASK) | ((long)(P) &~ CYCLE_FB_MASK)))
#   define CYCLE_FB_NEXT(B,P) ((Pixel8*)(((long)(B) & CYCLE_FB_MASK) | (((long)(P)+CYCLE_FB_INC) &~ CYCLE_FB_MASK)))
#   define CYCLE_FB_DSTWIDTH(B,W)   ((Pixel8 *)(((long)(p)+(long)(W)) & CYCLE_FB_RESET))
 
#   define CYCLE_FFB_INC	0x400L			/* 1024 byte inc    */
#   define CYCLE_FFB_RESET      (~(4*CYCLE_FFB_INC))    /* 4 aliases	    */
#   define CYCLE_REGS(ffb)						      \
	ffb = (FFB)((((long)(ffb))+CYCLE_FFB_INC) & CYCLE_FFB_RESET)
#   define CYCLE_AND_SET_FB(p) (p = CYCLE_FB(p))
#   define CYCLE_FB_GLOBAL_DECL Pixel8 *cycle_fb_mask = (Pixel8 *)0L
#   define CYCLE_FB_GLOBAL(poffset)					      \
	(Pixel8 *)(((unsigned long)(poffset) |				      \
		(unsigned long)CYCLE_AND_SET_FB(cycle_fb_mask)))
#endif

/****************************************************************************
 *                    Smart Frame Buffer Register Macros                    *
 ***************************************************************************/

/* Macros for writing to command registers. */
#if defined(SOFTWARE_MODEL) || defined(WB_MODEL)

#   if FFBBUSBITS == 32
#       define LWMASK 0xf
#   elif FFBBUSBITS == 64
#       define LWMASK 0xff
#   endif

/*
 * Macros for reading and writing data to frame buffer portion of ffb
 */
    
#   define FFBBusWrite(addr, data, mask) \
	wbBusWrite((unsigned long)(addr), data, mask)
#   define FFBBusRead(addr)		wbBusRead((unsigned long)(addr))  

#   define FFBREAD(psrc)		FFBBusRead(psrc)

#   define FFBWRITE(pdst, data)	        FFBBusWrite(pdst, data, LWMASK)

#   if FFBPIXELBITS == 8
#       define FFBPIXELWRITE(pdst, data)				\
{									\
    int align_;								\
    align_ = (long)(pdst) & FFBBUSBYTESMASK;				\
    FFBBusWrite((pdst) - align_, (data) << (align_ * 8), 1 << align_);  \
}
#   elif FFBPIXELBITS == FFBBUSBITS
#       define FFBPIXELWRITE(pdst, data)      FFBWRITE(pdst, data)
#   endif

#   define FFBRegWrite(field, data)	FFBBusWrite(&(field), data, LWMASK)
#   define FFBRegRead(field)		FFBBusRead(&(field))


#else /* SOFTWARE_MODEL */

#   define FFBREAD(psrc)		*((volatile PixelWord *)(psrc))
#   define FFBWRITE(pdst, data)		*((volatile PixelWord *)(pdst)) = data

#   define FFBPIXELWRITE(pdst, data)    *((OnePixel *) (pdst)) = data

#   define FFBRegWrite(field, data)	field = data
#   define FFBRegRead(field)		(field)

#endif /* SOFTWARE_MODEL ... else ... */

#ifdef SOFTWARE_MODEL 
#define FFBWRITE64(pdst, data)                                          \
{                                                                       \
        Bits32 hiword;                                                  \
        Bits32 loword;                                                  \
        hiword = data >> 32;                                            \
        loword = data  & 0xffffffff;                                    \
        FFBWRITE((Pixel32 *)pdst, loword);                              \
        FFBWRITE((Pixel32 *)pdst + 1, hiword);                          \
}
#else
#define FFBWRITE64(pdst, data)                                          \
  *((volatile Pixel64 *)(pdst)) = data;
#endif

#define FFBBUFREAD(ffb, pos)		FFBRegRead(ffb->buffer[pos])

#if FFBVRAMBITS/FFBBUSBITS == 1
#   define FFBBUFWRITE(ffb, pos, src)   FFBRegWrite(ffb->buffer[pos], src)

#elif FFBVRAMBITS/FFBBUSBITS == 2
/* Must always write a pair of words for them to actually get into buffer. */
#   define FFBBUFWRITE(ffb, pos, src0, src1)		\
{							\
    FFBRegWrite(ffb->buffer[pos],   src0);		\
    FFBRegWrite(ffb->buffer[pos+1], src1);		\
} /* FFBBUFWRITE */

#elif FFBVRAMBITS/FFBBUSBITS == 4
/* Must always write four words for them to actually get into buffer. */
#   define FFBBUFWRITE(ffb, pos, src0, src1, src2, src3)\
{							\
    FFBRegWrite(ffb->buffer[pos],   src0);		\
    FFBRegWrite(ffb->buffer[pos+1], src1);		\
    FFBRegWrite(ffb->buffer[pos+2], src2);		\
    FFBRegWrite(ffb->buffer[pos+3], src3);		\
} /* FFBBUFWRITE */
#endif

#define FFBFOREGROUND(ffb, data)	    FFBRegWrite(ffb->foreground,  data)
#define FFBBACKGROUND(ffb, data)	    FFBRegWrite(ffb->background,  data)
#define FFBPLANEMASK(scrPriv, ffb, data)				    \
{									    \
    scrPriv->info->planemask = data;	    /* Stash away for driver */     \
    FFBRegWrite(ffb->planemask, data);					    \
}
#define FFBPIXELMASK(ffb, data)		    FFBRegWrite(ffb->pixelmask,   data)
#define FFBMODE(ffb, data)		    FFBRegWrite(ffb->mode,        data)
#define FFBROP(ffb, data, rotation, visual) \
		FFBRegWrite(ffb->rop, ((data) | (rotation) | (visual)))
#define FFBSHIFT(ffb, data)		    FFBRegWrite(ffb->shift,       data)
#define FFBADDRESS(ffb, data) \
		FFBRegWrite(ffb->address, (Bits32) data)

#define FFBBRES1(ffb, data)		    FFBRegWrite(ffb->bres1,       data)
#define FFBBRES2(ffb, data)		    FFBRegWrite(ffb->bres2,       data)
#define FFBBRES3(ffb, data)		    FFBRegWrite(ffb->bres3,       data)
#define FFBCONTINUE(ffb, data)		    FFBRegWrite(ffb->brescont,    data)
#ifdef SOFTWARE_MODEL
#   define FFBDEEP(ffb, data)		    FFBRegWrite(ffb->deep,        data)
#else
/* FFBDEEP left undefined...only the ROM code should write it */
#endif
#define FFBDEEPREAD(ffb)		    FFBRegRead(ffb->deep)
/* FFBSTART left undefined...was only there for sfb compatibility */
#define FFBSTENCIL(ffb, data)		    FFBRegWrite(ffb->stencil,     data)
#define FFBPERSISTENTPIXELMASK(ffb, data) \
	FFBRegWrite(ffb->persistent_pixelmask, data)

#define FFBCURSORBASE(ffb, data)	    FFBRegWrite(ffb->cursor_base, data)
#define FFBHORIZCTL(ffb, data)		    FFBRegWrite(ffb->horiz_ctl,   data)
#define FFBVERTCTL(ffb, data)		    FFBRegWrite(ffb->vert_ctl,    data)
#define FFBVERTCTLREAD(ffb)	            FFBRegRead(ffb->vert_ctl)
#define FFBVIDEOBASE(ffb, data)		    FFBRegWrite(ffb->video_base,  data)
#define FFBVIDEOVALID(ffb, data)	    FFBRegWrite(ffb->video_valid, data)
#define FFBCURSOR(ffb, data)		    FFBRegWrite(ffb->cursor,      data)
#define FFBVIDEOSHIFT(ffb, data)	    FFBRegWrite(ffb->video_shift, data)
#define FFBINTSTAT(ffb, data)		    FFBRegWrite(ffb->int_stat,    data)

#define FFBDATA(ffb, data)		    FFBRegWrite(ffb->ffbdata,     data)
#define FFBREDINC(ffb, data)		    FFBRegWrite(ffb->red_incr,    data)
#define FFBGREENINC(ffb, data)		    FFBRegWrite(ffb->green_incr,  data)
#define FFBBLUEINC(ffb, data)		    FFBRegWrite(ffb->blue_incr,   data)
#define FFBZFRINC(ffb, data)		    FFBRegWrite(ffb->z_fr_incr,  data)
#define FFBZWHINC(ffb, data)		    FFBRegWrite(ffb->z_wh_incr,  data)
#if defined(__alpha) && defined(SOFTWARE_MODEL)
#    define FFBDMA(ffb, data)				\
{							\
    Bits32  hiword;                                     \
    Bits32  loword;                                     \
    hiword = (unsigned long)data >> 32;                 \
    loword = (unsigned long)data & 0xffffffff;          \
    FFBRegWrite(ffb->dma_addr, loword);                 \
    FFBRegWrite(ffb->bogus_dma_high, hiword);		\
}
#else
#    define FFBDMA(ffb, data)           FFBRegWrite(ffb->dma_addr,(Bits32)data)
#endif
#define FFBBRESWIDTH(ffb, data)	        FFBRegWrite(ffb->breswidth,       data)

#define FFBZFRVALUE(ffb, data)		FFBRegWrite(ffb->z_fr_value,      data)
#define FFBZWHVALUE(ffb, data)		FFBRegWrite(ffb->z_wh_value,	  data)
#define FFBZBASE(ffb, data)		FFBRegWrite(ffb->z_base,	  data)
#define FFBADDRESSALIAS(ffb, data)      FFBRegWrite(ffb->address_alias,   data)
#define FFBRED(ffb, data)	        FFBRegWrite(ffb->red,		  data)
#define FFBGREEN(ffb, data)	        FFBRegWrite(ffb->green,		  data)
#define FFBBLUE(ffb, data)	        FFBRegWrite(ffb->blue,		  data)
#define FFBSPANWIDTH(ffb, data)	        FFBRegWrite(ffb->span_width,	  data)

#define FFBRAMDACSETUP(ffb, data)	FFBRegWrite(ffb->ramdac_setup,    data)

#define FFBSLPNGO0(ffb, data)	        FFBRegWrite(ffb->sng_ndx_lt_ndy,  data)
#define FFBSLPNGO1(ffb, data)           FFBRegWrite(ffb->sng_ndx_lt_dy,   data)
#define FFBSLPNGO2(ffb, data)           FFBRegWrite(ffb->sng_dx_lt_ndy,   data)
#define FFBSLPNGO3(ffb, data)           FFBRegWrite(ffb->sng_dx_lt_dy,    data)
#define FFBSLPNGO4(ffb, data)           FFBRegWrite(ffb->sng_ndx_gt_ndy,  data)
#define FFBSLPNGO5(ffb, data)           FFBRegWrite(ffb->sng_ndx_gt_dy,   data)
#define FFBSLPNGO6(ffb, data)           FFBRegWrite(ffb->sng_dx_gt_ndy,   data)
#define FFBSLPNGO7(ffb, data)           FFBRegWrite(ffb->sng_dx_gt_dy,    data)

#define FFBSLP0(ffb, data)              FFBRegWrite(ffb->slope_ndx_lt_ndy, data)
#define FFBSLP1(ffb, data)              FFBRegWrite(ffb->slope_ndx_lt_dy, data)
#define FFBSLP2(ffb, data)              FFBRegWrite(ffb->slope_dx_lt_ndy, data)
#define FFBSLP3(ffb, data)              FFBRegWrite(ffb->slope_dx_lt_dy,  data)
#define FFBSLP4(ffb, data)              FFBRegWrite(ffb->slope_ndx_gt_ndy, data)
#define FFBSLP5(ffb, data)              FFBRegWrite(ffb->slope_ndx_gt_dy, data)
#define FFBSLP6(ffb, data)              FFBRegWrite(ffb->slope_dx_gt_ndy, data)
#define FFBSLP7(ffb, data)              FFBRegWrite(ffb->slope_dx_gt_dy,  data)

#define FFBCOLOR0(ffb, data)	        FFBRegWrite(ffb->color0,	  data)
#define FFBCOLOR1(ffb, data)            FFBRegWrite(ffb->color1,	  data)
#define FFBCOLOR2(ffb, data)            FFBRegWrite(ffb->color2,	  data)
#define FFBCOLOR3(ffb, data)            FFBRegWrite(ffb->color3,	  data)
#define FFBCOLOR4(ffb, data)	        FFBRegWrite(ffb->color4,	  data)
#define FFBCOLOR5(ffb, data)	        FFBRegWrite(ffb->color5,	  data)
#define FFBCOLOR6(ffb, data)            FFBRegWrite(ffb->color6,	  data)
#define FFBCOLOR7(ffb, data)	        FFBRegWrite(ffb->color7,	  data)

#define FFBCOPY64SRC(ffb, data)	        FFBRegWrite(ffb->copy64src0,      data)
#define FFBCOPY64DST(ffb, data)	        FFBRegWrite(ffb->copy64dst0,      data)
#define FFBCOPY64SRC1(ffb, data)        FFBRegWrite(ffb->copy64src1,      data)
#define FFBCOPY64DST1(ffb, data)        FFBRegWrite(ffb->copy64dst1,      data)
#define FFBCOPY64SRC2(ffb, data)        FFBRegWrite(ffb->copy64src2,      data)
#define FFBCOPY64DST2(ffb, data)        FFBRegWrite(ffb->copy64dst2,      data)
#define FFBCOPY64SRC3(ffb, data)        FFBRegWrite(ffb->copy64src3,      data)
#define FFBCOPY64DST3(ffb, data)        FFBRegWrite(ffb->copy64dst3,      data)

#define FFBEPROM(ffb, data)		FFBRegWrite(ffb->eprom_write,     data)
#define FFBCLOCK(ffb, data)		FFBRegWrite(ffb->clock,		  data)
#define FFBRAMDAC(ffb, data)		FFBRegWrite(ffb->ramdac_int,      data)
#define FFBCOMMANDSTATUS(ffb, data)     FFBRegWrite(ffb->command_status,  data)
#define FFBCOMMANDSTATUSREAD(ffb)       FFBRegRead(ffb->command_status)


/****************************************************************************
 *                     Useful Macros for Talking to FFB                     *
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

#   define FFBWRITEONEWORD(ffb, pdst, data)	\
{						\
    CYCLE_REGS(ffb);				\
    FFBADDRESS(ffb, pdst);			\
    FFBCONTINUE(ffb, data);			\
} /* FFBWRITEONEWORD */
#else
#   define FFBWRITEONEWORD(ffb, pdst, data)    FFBWRITE(pdst, data)
#endif


/****************************************************************************
 *          Compute Masks for Left and Right Edges of a Bus Word            *
 ***************************************************************************/

extern CommandWord ffbBusAll1;

#if defined(MODULOSHIFTS) && (FFBBUSBITS == WORDBITS)
#   define FFBLEFTBUSMASK(align, ones)		((ones) << (align))
#   define FFBRIGHTBUSMASK(alignedWidth, ones) ((ones) >> -(alignedWidth))

#else /* use longer sequences */

#   define FFBLEFTBUSMASK(align, ones)  ((ones) << ((align) & FFBBUSBITSMASK))
#   define FFBRIGHTBUSMASK(alignedWidth, ones) \
	((ones) >> (-(alignedWidth) & FFBBUSBITSMASK))
#endif


/****************************************************************************
 *         Compute Masks for Left and Right Edges of a Stipple Span         *
 ***************************************************************************/

extern CommandWord ffbStippleAll1;

#if FFBSTIPPLEBITS == 32
#   define FFBSTIPPLEALL1  ((CommandWord)0xffffffff)
#elif FFBSTIPPLEBITS == 64
#   define FFBSTIPPLEALL1  ((CommandWord)0xffffffffffffffff)
#endif

#if defined(MODULOSHIFTS) && (FFBSTIPPLEBITS == WORDBITS)
#   define FFBLEFTSTIPPLEMASK(align, ones)	    ((ones) << (align))
#   define FFBRIGHTSTIPPLEMASK(alignedWidth, ones)  ((ones) >> -(alignedWidth))

#else /* use longer sequences */

#   define FFBLEFTSTIPPLEMASK(align, ones) \
	((ones) << ((align) & FFBSTIPPLEBITSMASK))
#   define FFBRIGHTSTIPPLEMASK(alignedWidth, ones) \
	((ones) >> (-(alignedWidth) & FFBSTIPPLEBITSMASK))
#endif

/* Computation of right shift amount when stippling a word of BUSBITS into
   pixels, ala text and 32-bit stipples. */
#if defined(MODULOSHIFTS) && (WORDBITS == FFBSTIPPLEBITS)
#   define FFBRIGHTSTIPPLESHIFT(align)    (-(align))
#else
#   define FFBRIGHTSTIPPLESHIFT(align)    (FFBSTIPPLEBITS - (align))
#endif



/****************************************************************************
 *                              Paint a Point                               *
 ***************************************************************************/

#ifdef PARTIALWRITES
#   define FFBPOINT(pdst, foreground) FFBPIXELWRITE(pdst, foreground)
#else
/* Assume foreground already loaded, mode is TRANSPARENTSTIPPLE, and depth is
   properly set. */
#   define FFBPOINT(pdst, foreground)					      \
{									      \
    long align_, pdst_;							      \
    pdst_ = (long)(pdst);						      \
    align_ = pdst_ & FFBSTIPPLEALIGNMASK;				      \
    pdst_ -= align_;							      \
    FFBBYTESTOPIXELS(align_);						      \
    FFBWRITE(pdst_, 1 << align_);					      \
} /* FFBPOINT */
#endif


/****************************************************************************
 *                           Paint a Solid Span                             *
 ***************************************************************************/

#define FFBSOLIDSPAN(ffb, pdst, widthInPixels, maxPixels)		\
{									\
	int     blockAlign_;						\
	Pixel8  *pdstBlock_ = (pdst);					\
	int	width_ = (widthInPixels);				\
	blockAlign_ = (long)pdstBlock_ & FFBBUSBYTESMASK;		\
	pdstBlock_ -= blockAlign_;					\
	while (width_ > (maxPixels)) {					\
	    FFBWRITEONEWORD(ffb, pdstBlock_,				\
		    FFBLOADBLOCKDATA(blockAlign_, maxPixels));		\
	    width_ -= (maxPixels);					\
	    pdstBlock_ += (maxPixels) * FFBPIXELBYTES;			\
	}								\
	FFBWRITE(pdstBlock_, FFBLOADBLOCKDATA(blockAlign_, width_));	\
} /* FFBSOLIDSPAN */


/****************************************************************************
 *          Macros for turning packed x and y into separate values          *
 ***************************************************************************/

#define Int32ToX(i)     ((i) & 0xffff)
#define Int32ToY(i)     ((int)(i) >> 16)


/****************************************************************************
 *                 Macro for checking some useful condition                 *
 ***************************************************************************/

#if defined(SOFTWARE_MODEL) || defined(DEBUG)
#   define Assert(bool, message)    if (~bool) { ErrorF(message); abort(); }
#else
#   define Assert(bool, message)   /* Nothing */
#endif



/****************************************************************************
 *              Macro to determine if a value is a power of two             *
 ***************************************************************************/

#define PowerOfTwo(x)   (!((x) & ((x)-1)))


/**************************************************************************
 *       Macros for setting the planemasks for the different visuals      *
 *************************************************************************/

#define FFB_24BIT_PLANEMASK      0x00ffffff
#define FFB_8U_PLANEMASK         0xffffffff
#define FFB_8P_PLANEMASK         0xffffffff
#define FFB_12_BUF0_PLANEMASK    0x00f0f0f0
#define FFB_12_BUF1_PLANEMASK    0x000f0f0f
#define FFB_OVRLY_PLANEMASK      0x0f0f0f0f
                                                 

/**************************************************************************
 *       Macro for checking to see which code to use for opaque stipples  *
 *************************************************************************/

#define FFB_OS_WIDTH_CHECK(w)            ((w) > 50)
      

/**************************************************************************
 *        Macros for synchronizing with the hardware			  *
 **************************************************************************/

#ifdef SOFTWARE_MODEL
#   define FFBSYNC(ffb)				\
{						\
    READ_MEMORY_BARRIER();			\
    MakeIdle();					\
} /* FFBSYNC */
#else
#   define FFBSYNC(ffb)				\
{						\
    int _status;				\
    READ_MEMORY_BARRIER();			\
    do {					\
	_status = FFBCOMMANDSTATUSREAD(ffb);	\
    } while (_status != 0);			\
} /* FFBSYNC */
#endif

#endif /* FFBMACROS_H */



/******************************************************************************
 * HISTORY
 */
