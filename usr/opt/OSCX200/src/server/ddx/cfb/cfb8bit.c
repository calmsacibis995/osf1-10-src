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
 * cfb8bit.c
 *
 * 8 bit color frame buffer utility routines
 */

/* $XConsortium: cfb8bit.c,v 1.8 92/07/30 10:24:26 rws Exp $ */

#if PSZ == 8

#include	"X.h"
#include	"Xmd.h"
#include	"Xproto.h"
#include	"gcstruct.h"
#include	"windowstr.h"
#include	"scrnintstr.h"
#include	"pixmapstr.h"
#include	"regionstr.h"
#include	"cfb.h"
#include	"cfbmskbits.h"
#include	"cfb8bit.h"

#if LONG_BIT == 32
unsigned long cfb8StippleMasks[NUM_MASKS] = {
    0x00000000, 0x000000ff, 0x0000ff00, 0x0000ffff,
    0x00ff0000, 0x00ff00ff, 0x00ffff00, 0x00ffffff,
    0xff000000, 0xff0000ff, 0xff00ff00, 0xff00ffff,
    0xffff0000, 0xffff00ff, 0xffffff00, 0xffffffff,
};
#else /* LONG_BIT == 64 */
unsigned long cfb8StippleMasks[NUM_MASKS] = {
    0x0000000000000000L,	0x00000000000000ffL,
    0x000000000000ff00L,	0x000000000000ffffL,
    0x0000000000ff0000L,	0x0000000000ff00ffL,
    0x0000000000ffff00L,	0x0000000000ffffffL,
    0x00000000ff000000L,	0x00000000ff0000ffL,
    0x00000000ff00ff00L,	0x00000000ff00ffffL,
    0x00000000ffff0000L,	0x00000000ffff00ffL,
    0x00000000ffffff00L,	0x00000000ffffffffL,
    0x000000ff00000000L,	0x000000ff000000ffL,
    0x000000ff0000ff00L,	0x000000ff0000ffffL,
    0x000000ff00ff0000L,	0x000000ff00ff00ffL,
    0x000000ff00ffff00L,	0x000000ff00ffffffL,
    0x000000ffff000000L,	0x000000ffff0000ffL,
    0x000000ffff00ff00L,	0x000000ffff00ffffL,
    0x000000ffffff0000L,	0x000000ffffff00ffL,
    0x000000ffffffff00L,	0x000000ffffffffffL,
    0x0000ff0000000000L,	0x0000ff00000000ffL,
    0x0000ff000000ff00L,	0x0000ff000000ffffL,
    0x0000ff0000ff0000L,	0x0000ff0000ff00ffL,
    0x0000ff0000ffff00L,	0x0000ff0000ffffffL,
    0x0000ff00ff000000L,	0x0000ff00ff0000ffL,
    0x0000ff00ff00ff00L,	0x0000ff00ff00ffffL,
    0x0000ff00ffff0000L,	0x0000ff00ffff00ffL,
    0x0000ff00ffffff00L,	0x0000ff00ffffffffL,
    0x0000ffff00000000L,	0x0000ffff000000ffL,
    0x0000ffff0000ff00L,	0x0000ffff0000ffffL,
    0x0000ffff00ff0000L,	0x0000ffff00ff00ffL,
    0x0000ffff00ffff00L,	0x0000ffff00ffffffL,
    0x0000ffffff000000L,	0x0000ffffff0000ffL,
    0x0000ffffff00ff00L,	0x0000ffffff00ffffL,
    0x0000ffffffff0000L,	0x0000ffffffff00ffL,
    0x0000ffffffffff00L,	0x0000ffffffffffffL,
    0x00ff000000000000L,	0x00ff0000000000ffL,
    0x00ff00000000ff00L,	0x00ff00000000ffffL,
    0x00ff000000ff0000L,	0x00ff000000ff00ffL,
    0x00ff000000ffff00L,	0x00ff000000ffffffL,
    0x00ff0000ff000000L,	0x00ff0000ff0000ffL,
    0x00ff0000ff00ff00L,	0x00ff0000ff00ffffL,
    0x00ff0000ffff0000L,	0x00ff0000ffff00ffL,
    0x00ff0000ffffff00L,	0x00ff0000ffffffffL,
    0x00ff00ff00000000L,	0x00ff00ff000000ffL,
    0x00ff00ff0000ff00L,	0x00ff00ff0000ffffL,
    0x00ff00ff00ff0000L,	0x00ff00ff00ff00ffL,
    0x00ff00ff00ffff00L,	0x00ff00ff00ffffffL,
    0x00ff00ffff000000L,	0x00ff00ffff0000ffL,
    0x00ff00ffff00ff00L,	0x00ff00ffff00ffffL,
    0x00ff00ffffff0000L,	0x00ff00ffffff00ffL,
    0x00ff00ffffffff00L,	0x00ff00ffffffffffL,
    0x00ffff0000000000L,	0x00ffff00000000ffL,
    0x00ffff000000ff00L,	0x00ffff000000ffffL,
    0x00ffff0000ff0000L,	0x00ffff0000ff00ffL,
    0x00ffff0000ffff00L,	0x00ffff0000ffffffL,
    0x00ffff00ff000000L,	0x00ffff00ff0000ffL,
    0x00ffff00ff00ff00L,	0x00ffff00ff00ffffL,
    0x00ffff00ffff0000L,	0x00ffff00ffff00ffL,
    0x00ffff00ffffff00L,	0x00ffff00ffffffffL,
    0x00ffffff00000000L,	0x00ffffff000000ffL,
    0x00ffffff0000ff00L,	0x00ffffff0000ffffL,
    0x00ffffff00ff0000L,	0x00ffffff00ff00ffL,
    0x00ffffff00ffff00L,	0x00ffffff00ffffffL,
    0x00ffffffff000000L,	0x00ffffffff0000ffL,
    0x00ffffffff00ff00L,	0x00ffffffff00ffffL,
    0x00ffffffffff0000L,	0x00ffffffffff00ffL,
    0x00ffffffffffff00L,	0x00ffffffffffffffL,
    0xff00000000000000L,	0xff000000000000ffL,
    0xff0000000000ff00L,	0xff0000000000ffffL,
    0xff00000000ff0000L,	0xff00000000ff00ffL,
    0xff00000000ffff00L,	0xff00000000ffffffL,
    0xff000000ff000000L,	0xff000000ff0000ffL,
    0xff000000ff00ff00L,	0xff000000ff00ffffL,
    0xff000000ffff0000L,	0xff000000ffff00ffL,
    0xff000000ffffff00L,	0xff000000ffffffffL,
    0xff0000ff00000000L,	0xff0000ff000000ffL,
    0xff0000ff0000ff00L,	0xff0000ff0000ffffL,
    0xff0000ff00ff0000L,	0xff0000ff00ff00ffL,
    0xff0000ff00ffff00L,	0xff0000ff00ffffffL,
    0xff0000ffff000000L,	0xff0000ffff0000ffL,
    0xff0000ffff00ff00L,	0xff0000ffff00ffffL,
    0xff0000ffffff0000L,	0xff0000ffffff00ffL,
    0xff0000ffffffff00L,	0xff0000ffffffffffL,
    0xff00ff0000000000L,	0xff00ff00000000ffL,
    0xff00ff000000ff00L,	0xff00ff000000ffffL,
    0xff00ff0000ff0000L,	0xff00ff0000ff00ffL,
    0xff00ff0000ffff00L,	0xff00ff0000ffffffL,
    0xff00ff00ff000000L,	0xff00ff00ff0000ffL,
    0xff00ff00ff00ff00L,	0xff00ff00ff00ffffL,
    0xff00ff00ffff0000L,	0xff00ff00ffff00ffL,
    0xff00ff00ffffff00L,	0xff00ff00ffffffffL,
    0xff00ffff00000000L,	0xff00ffff000000ffL,
    0xff00ffff0000ff00L,	0xff00ffff0000ffffL,
    0xff00ffff00ff0000L,	0xff00ffff00ff00ffL,
    0xff00ffff00ffff00L,	0xff00ffff00ffffffL,
    0xff00ffffff000000L,	0xff00ffffff0000ffL,
    0xff00ffffff00ff00L,	0xff00ffffff00ffffL,
    0xff00ffffffff0000L,	0xff00ffffffff00ffL,
    0xff00ffffffffff00L,	0xff00ffffffffffffL,
    0xffff000000000000L,	0xffff0000000000ffL,
    0xffff00000000ff00L,	0xffff00000000ffffL,
    0xffff000000ff0000L,	0xffff000000ff00ffL,
    0xffff000000ffff00L,	0xffff000000ffffffL,
    0xffff0000ff000000L,	0xffff0000ff0000ffL,
    0xffff0000ff00ff00L,	0xffff0000ff00ffffL,
    0xffff0000ffff0000L,	0xffff0000ffff00ffL,
    0xffff0000ffffff00L,	0xffff0000ffffffffL,
    0xffff00ff00000000L,	0xffff00ff000000ffL,
    0xffff00ff0000ff00L,	0xffff00ff0000ffffL,
    0xffff00ff00ff0000L,	0xffff00ff00ff00ffL,
    0xffff00ff00ffff00L,	0xffff00ff00ffffffL,
    0xffff00ffff000000L,	0xffff00ffff0000ffL,
    0xffff00ffff00ff00L,	0xffff00ffff00ffffL,
    0xffff00ffffff0000L,	0xffff00ffffff00ffL,
    0xffff00ffffffff00L,	0xffff00ffffffffffL,
    0xffffff0000000000L,	0xffffff00000000ffL,
    0xffffff000000ff00L,	0xffffff000000ffffL,
    0xffffff0000ff0000L,	0xffffff0000ff00ffL,
    0xffffff0000ffff00L,	0xffffff0000ffffffL,
    0xffffff00ff000000L,	0xffffff00ff0000ffL,
    0xffffff00ff00ff00L,	0xffffff00ff00ffffL,
    0xffffff00ffff0000L,	0xffffff00ffff00ffL,
    0xffffff00ffffff00L,	0xffffff00ffffffffL,
    0xffffffff00000000L,	0xffffffff000000ffL,
    0xffffffff0000ff00L,	0xffffffff0000ffffL,
    0xffffffff00ff0000L,	0xffffffff00ff00ffL,
    0xffffffff00ffff00L,	0xffffffff00ffffffL,
    0xffffffffff000000L,	0xffffffffff0000ffL,
    0xffffffffff00ff00L,	0xffffffffff00ffffL,
    0xffffffffffff0000L,	0xffffffffffff00ffL,
    0xffffffffffffff00L,	0xffffffffffffffffL,
};
#endif /* LONG_BIT */

int	cfb8StippleMode, cfb8StippleAlu, cfb8StippleRRop;
unsigned long	cfb8StippleFg, cfb8StippleBg, cfb8StipplePm;
unsigned long	cfb8StippleAnd[NUM_MASKS], cfb8StippleXor[NUM_MASKS];

int
cfb8SetStipple (alu, fg, planemask)
int		alu;
unsigned long	fg, planemask;
{
    unsigned long   and, xor, rrop;
    int	s;
    unsigned long   c;

    cfb8StippleMode = FillStippled;
    cfb8StippleAlu = alu;
    cfb8StippleFg = fg & PMSK;
    cfb8StipplePm = planemask & PMSK;
    rrop = cfbReduceRasterOp (alu, fg, planemask, &and, &xor);
    cfb8StippleRRop = rrop;
    /*
     * create the appropriate pixel-fill bits for current
     * foreground
     */
    for (s = 0; s < NUM_MASKS; s++)
    {
	c = cfb8StippleMasks[s];
	cfb8StippleAnd[s] = and | ~c;
	cfb8StippleXor[s] = xor & c;
    }
}

int
cfb8SetOpaqueStipple (alu, fg, bg, planemask)
int		alu;
unsigned long	fg, bg, planemask;
{
    unsigned long   andfg, xorfg, andbg, xorbg, rropfg, rropbg;
    int	s;
    unsigned long   c;

    cfb8StippleMode = FillOpaqueStippled;
    cfb8StippleAlu = alu;
    cfb8StippleFg = fg & PMSK;
    cfb8StippleBg = bg & PMSK;
    cfb8StipplePm = planemask & PMSK;
    rropfg = cfbReduceRasterOp (alu, cfb8StippleFg, cfb8StipplePm, &andfg, &xorfg);
    rropbg = cfbReduceRasterOp (alu, cfb8StippleBg, cfb8StipplePm, &andbg, &xorbg);
    if (rropfg == rropbg)
	cfb8StippleRRop = rropfg;
    else
	cfb8StippleRRop = GXset;
    /*
     * create the appropriate pixel-fill bits for current
     * foreground
     */
    for (s = 0; s < NUM_MASKS; s++)
    {
	c = cfb8StippleMasks[s];
	cfb8StippleAnd[s] = (andfg | ~c) & (andbg | c);
	cfb8StippleXor[s] = (xorfg & c) | (xorbg & ~c);
    }
}

/*
 * a grungy little routine.  This computes clip masks
 * for partial character blts.  Returns rgnOUT if the
 * entire character is clipped; returns rgnIN if the entire
 * character is unclipped; returns rgnPART if a portion of
 * the character is visible.  Computes clip masks for each
 * longword of the character -- and those with the
 * contents of the glyph to compute the visible bits.
 */

#if LONG_BIT == 32
#if (BITMAP_BIT_ORDER == MSBFirst)
unsigned long	cfb8BitLenMasks[LONG_BIT] = {
    0xffffffff, 0x7fffffff, 0x3fffffff, 0x1fffffff,
    0x0fffffff, 0x07ffffff, 0x03ffffff, 0x01ffffff,
    0x00ffffff, 0x007fffff, 0x003fffff, 0x001fffff,
    0x000fffff, 0x0007ffff, 0x0003ffff, 0x0001ffff,
    0x0000ffff, 0x00007fff, 0x00003fff, 0x00001fff,
    0x00000fff, 0x000007ff, 0x000003ff, 0x000001ff,
    0x000000ff, 0x0000007f, 0x0000003f, 0x0000001f,
    0x0000000f, 0x00000007, 0x00000003, 0x00000001,
};
#else
unsigned long cfb8BitLenMasks[LONG_BIT] = {
    0xffffffff, 0xfffffffe, 0xfffffffc, 0xfffffff8,
    0xfffffff0, 0xffffffe0, 0xffffffc0, 0xffffff80,
    0xffffff00, 0xfffffe00, 0xfffffc00, 0xfffff800,
    0xfffff000, 0xffffe000, 0xffffc000, 0xffff8000,
    0xffff0000, 0xfffe0000, 0xfffc0000, 0xfff80000,
    0xfff00000, 0xffe00000, 0xffc00000, 0xff800000,
    0xff000000, 0xfe000000, 0xfc000000, 0xf8000000,
    0xf0000000, 0xe0000000, 0xc0000000, 0x80000000,
};
#endif
#else /* LONG_BIT == 64 */
#if (BITMAP_BIT_ORDER == MSBFirst)
unsigned long	cfb8BitLenMasks[LONG_BIT] = {
    0xffffffffffffffffL,    0x7fffffffffffffffL,
    0x3fffffffffffffffL,    0x1fffffffffffffffL,
    0x0fffffffffffffffL,    0x07ffffffffffffffL,
    0x03ffffffffffffffL,    0x01ffffffffffffffL,
    0x00ffffffffffffffL,    0x007fffffffffffffL,
    0x003fffffffffffffL,    0x001fffffffffffffL,
    0x000fffffffffffffL,    0x0007ffffffffffffL,
    0x0003ffffffffffffL,    0x0001ffffffffffffL,
    0x0000ffffffffffffL,    0x00007fffffffffffL,
    0x00003fffffffffffL,    0x00001fffffffffffL,
    0x00000fffffffffffL,    0x000007ffffffffffL,
    0x000003ffffffffffL,    0x000001ffffffffffL,
    0x000000ffffffffffL,    0x0000007fffffffffL,
    0x0000003fffffffffL,    0x0000001fffffffffL,
    0x0000000fffffffffL,    0x00000007ffffffffL,
    0x00000003ffffffffL,    0x00000001ffffffffL,
    0x00000000ffffffffL,    0x000000007fffffffL,
    0x000000003fffffffL,    0x000000001fffffffL,
    0x000000000fffffffL,    0x0000000007ffffffL,
    0x0000000003ffffffL,    0x0000000001ffffffL,
    0x0000000000ffffffL,    0x00000000007fffffL,
    0x00000000003fffffL,    0x00000000001fffffL,
    0x00000000000fffffL,    0x000000000007ffffL,
    0x000000000003ffffL,    0x000000000001ffffL,
    0x000000000000ffffL,    0x0000000000007fffL,
    0x0000000000003fffL,    0x0000000000001fffL,
    0x0000000000000fffL,    0x00000000000007ffL,
    0x00000000000003ffL,    0x00000000000001ffL,
    0x00000000000000ffL,    0x000000000000007fL,
    0x000000000000003fL,    0x000000000000001fL,
    0x000000000000000fL,    0x0000000000000007L,
    0x0000000000000003L,    0x0000000000000001L,
};
#else
unsigned long cfb8BitLenMasks[LONG_BIT] = {
    0xffffffffffffffffL,    0xfffffffffffffffeL,
    0xfffffffffffffffcL,    0xfffffffffffffff8L,
    0xfffffffffffffff0L,    0xffffffffffffffe0L,
    0xffffffffffffffc0L,    0xffffffffffffff80L,
    0xffffffffffffff00L,    0xfffffffffffffe00L,
    0xfffffffffffffc00L,    0xfffffffffffff800L,
    0xfffffffffffff000L,    0xffffffffffffe000L,
    0xffffffffffffc000L,    0xffffffffffff8000L,
    0xffffffffffff0000L,    0xfffffffffffe0000L,
    0xfffffffffffc0000L,    0xfffffffffff80000L,
    0xfffffffffff00000L,    0xffffffffffe00000L,
    0xffffffffffc00000L,    0xffffffffff800000L,
    0xffffffffff000000L,    0xfffffffffe000000L,
    0xfffffffffc000000L,    0xfffffffff8000000L,
    0xfffffffff0000000L,    0xffffffffe0000000L,
    0xffffffffc0000000L,    0xffffffff80000000L,
    0xffffffff00000000L,    0xfffffffe00000000L,
    0xfffffffc00000000L,    0xfffffff800000000L,
    0xfffffff000000000L,    0xffffffe000000000L,
    0xffffffc000000000L,    0xffffff8000000000L,
    0xffffff0000000000L,    0xfffffe0000000000L,
    0xfffffc0000000000L,    0xfffff80000000000L,
    0xfffff00000000000L,    0xffffe00000000000L,
    0xffffc00000000000L,    0xffff800000000000L,
    0xffff000000000000L,    0xfffe000000000000L,
    0xfffc000000000000L,    0xfff8000000000000L,
    0xfff0000000000000L,    0xffe0000000000000L,
    0xffc0000000000000L,    0xff80000000000000L,
    0xff00000000000000L,    0xfe00000000000000L,
    0xfc00000000000000L,    0xf800000000000000L,
    0xf000000000000000L,    0xe000000000000000L,
    0xc000000000000000L,    0x8000000000000000L,
};
#endif
#endif /* LONG_BIT */

int
cfb8ComputeClipMasks32 (pBox, numRects, x, y, w, h, clips)
    BoxPtr	pBox;
    int		numRects;
    int		x, y, w, h;
    unsigned long   *clips;
{
    int	    yBand, yBandBot;
    int	    ch;
    unsigned long	    clip;
    int	    partIN = FALSE, partOUT = FALSE;
    int	    result;

    if (numRects == 0)
	return rgnOUT;
    while (numRects && pBox->y2 <= y)
    {
	--numRects;
	++pBox;
    }
    if (!numRects || pBox->y1 >= y + h)
	return rgnOUT;
    yBand = pBox->y1;
    while (numRects && pBox->y1 == yBand && pBox->x2 <= x)
    {
	--numRects;
	++pBox;
    }
    if (!numRects || pBox->y1 >= y + h)
	return rgnOUT;
    if (numRects &&
	x >= pBox->x1 &&
	x + w <= pBox->x2 &&
	y >= pBox->y1 &&
	y + h <= pBox->y2)
    {
	return rgnIN;
    }
    ch = 0;
    while (ch < h && y + ch < pBox->y1)
    {
	partOUT = TRUE;
	clips[ch++] = 0;
    }
    while (numRects && pBox->y1 < y + h)
    {
	yBand = pBox->y1;
	yBandBot = pBox->y2;
    	while (numRects && pBox->y1 == yBand && pBox->x2 <= x)
    	{
	    --numRects;
	    ++pBox;
    	}
    	if (!numRects)
	    break;
	clip = 0L;
    	while (numRects && pBox->y1 == yBand && pBox->x1 < x + w)
    	{
	    if (x < pBox->x1)
		if (pBox->x2 < x + w)
		    clip |= cfb8BitLenMasks[pBox->x1 - x] & ~cfb8BitLenMasks[pBox->x2 - x];
		else
		    clip |= cfb8BitLenMasks[pBox->x1 - x];
 	    else
		if (pBox->x2 < x + w)
		    clip |= ~cfb8BitLenMasks[pBox->x2 - x];
		else
		    clip = ~0L;
	    --numRects;
	    ++pBox;
    	}
	if (clip != 0L)
		partIN = TRUE;
	if (clip != ~0L)
		partOUT = TRUE;
	while (ch < h && y + ch < yBandBot)
	    clips[ch++] = clip;
	while (numRects && pBox->y1 == yBand)
	{
	    --numRects;
	    ++pBox;
	}
    }
    while (ch < h)
    {
	partOUT = TRUE;
	clips[ch++] = 0L;
    }
    result = rgnOUT;
    if (partIN)
    {
	if (partOUT)
	    result = rgnPART;
	else
	    result = rgnIN;
    }
    return result;
}

#endif /* PSZ == 8 */
