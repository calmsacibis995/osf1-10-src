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
static char *rcsid = "@(#)$RCSfile: ffbpixmap.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:12:06 $";
#endif
/*
 */
#include "Xmd.h"
#include "X.h"

#include "servermd.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "mi.h"
#include "regionstr.h"

#include "cfbpixmap.h"
#include "ffb.h"
#include "ffbpixmap.h"
#include "ffbfill.h"
#include <stdio.h>

#ifdef SOFTWARE_MODEL
int ffbDebug;
#endif

/*
 *	Next-fit storage allocation mechanism
 *
 *	Algorithm and variable names stolen from Knuth V1, p 437,
 *		with suggested modifications from Exercise 2.5.6.
 *
 *	free list is kept in ffb address order
 *	avail is a dummy header that points to first free element
 *	rover is pointer into free list
 */

/*
 *	ffbScreenMalloc(pScreen, size) returns pointer to size bytes of storage,
 *      or NULL.
 */

pointer ffbScreenMalloc(pScreen, size)
    ScreenPtr   pScreen;
    int		size;
{
    register FreeElement *p, *q, *start;
    ffbScreenPrivPtr    scrPriv;
    Pixel8		*addr;

    scrPriv = FFBSCREENPRIV(pScreen);

    if (scrPriv->rover == (FreeElement *)NULL) 
    	scrPriv->rover = &(scrPriv->avail);
    q = scrPriv->rover;
    start = p = q->next;
    /* search for a block large enough */
    while ((p != (FreeElement *)NULL) && (p->size < size)) {
	/* keep looking */
	q = p;
	p = p->next;
    }
    if (p == (FreeElement *)NULL) {
	/* try rest of list */
	q = &(scrPriv->avail);
	p = q->next;
	while ((p != start) && (p->size < size)) {
	    q = p;
	    p = p->next;
	}
	if (p == start) {
	    /* can't allocate to screen memory */
	    return (pointer)NULL;
	}
    }
    addr = p->addr;
    if (p->size == size) {
	/* found one of exactly right size. remove it */
	q->next = p->next;
	xfree(p);
    } else /* p->size > size */ {
	/* found one too big. take part of it */
	p->addr += size;
	p->size -= size;
    }
    scrPriv->rover = q->next;
    return (pointer)addr;
}

/*
 *	ffbScreenFree(pScreen, p0, size) adds the block of size bytes pointed
 *      to by p0 to the free list
 */

void ffbScreenFree(pScreen, p0, size)
    ScreenPtr   pScreen;
    pointer     p0;
    int		size;
{
    FreeElement		*p, *q, *r;
    Pixel8		*addr;
    ffbScreenPrivPtr    scrPriv;

    scrPriv = FFBSCREENPRIV(pScreen);

    addr = (Pixel8 *)p0;
    q = &(scrPriv->avail);
    p = q->next;
    /* search for the right place */
    while ((p != (FreeElement *)NULL) && (p->addr < addr)) {
	/* not the place, keep searching */
	q = p;
	p = p->next;
    }
    /* Free memory goes somewhere between q->addr and p->addr.
       (It should never be before q, as scrPriv->avail->addr == 0).
       It may be immediately after q->addr, or immediately before p->addr,
       or both.  Merge abutting blocks appropriately. */
    if (p != NULL && p->addr == addr + size) {
	/* New free space immediately before p->addr */
	if (q->addr + q->size == addr) {
	    /* Free space also abuts q, merge all three spaces into one */
	    q->size += size + p->size;
	    q->next = p->next;
	    Xfree(p);
	} else {
	    /* Just merge free space with p */
	    p->size += size;
	    p->addr -= size;
	}
    } else if (q->addr + q->size == addr) {
	/* new block abuts q, merge */
	q->size += size;
    } else {
	/* does not abut either q or p */
	r = (FreeElement *)xalloc(sizeof(FreeElement));
	if (r != (FreeElement *)NULL) {
	    q->next = r;
	    r->next = p;
	    r->addr = addr;
	    r->size = size;
	} /* else that off-screen space just leaked into nothingness!? */
    }
    scrPriv->rover = q;	/* start searching here next time */
}

static Bool mainOnly = FALSE;
extern int maxsize;

PixmapPtr ffbCreatePixmap(pScreen, width, height, depth)
    ScreenPtr	pScreen;
    int		width;
    int		height;
    int		depth;
{
    register pointer   	       	base;
    int 		       	size,main_size;
    register ffbPixmapRec       *pffbPixmap;

    /* Big pixmaps are pretty useless and will screw up server.  Probably we
       got such large numbers because someone passed in a negative height by
       accident, anyway.  We have this weird maximum so that 0-width lines,
       including the cap, never exceed the 15-bit world.  The ffb+ really likes
       this idea, as it doesn't have to mask the damn length then. */

    if (width > 32766 || height > 32766)
	return NullPixmap;

    size = FFB_SCANLINE_PAD(width);
    if (depth > 8) {
	/* 32 bits per pixel [12,32] */
	size *= 4;
    }

    /* Try allocating a pixmap to offscreen memory if it is wide enough to
       benefit from such a precious resource, and if it is wide enough so that
       successive scanlines are guaranteed to fall into different CPU write
       buffer entries. */
    base = (pointer)NULL;
    if ((!mainOnly) && (depth != 1) && (size >= CPU_WB_BYTES) 
		    && (width > 64) && ((height * size) > maxsize)) {
        base = ffbScreenMalloc(pScreen, height * size);
    }

    if (base) {
	/* Allocated pixmap array in offscreen memory */
	pffbPixmap = (ffbPixmapRec *)xalloc(sizeof(ffbPixmapRec));
	if (!pffbPixmap) {
	    ffbScreenFree(pScreen, base, height*size);
	    return NullPixmap;
	}
	pffbPixmap->pixmap.devPrivate.ptr = base;
#ifdef SOFTWARE_MODEL
	if (ffbDebug)
        ErrorF("Allocated %d x %d at offscreen address %lx\n", 
		width, height, base);
#endif
    } else {
	/* Have to allocate pixmap in main memory */
	size = PixmapBytePad(width, depth);
	pffbPixmap =
	    (ffbPixmapRec *)xalloc(sizeof(ffbPixmapRec) + (height * size));
	if (!pffbPixmap)
	   return NullPixmap;
	pffbPixmap->pixmap.devPrivate.ptr = (pointer)(pffbPixmap + 1);
    }

    pffbPixmap->pixmap.devKind = size;
    pffbPixmap->pixmap.drawable.bitsPerPixel = BitsPerPixel(depth);
    pffbPixmap->pixmap.drawable.type = DRAWABLE_PIXMAP;
    pffbPixmap->pixmap.drawable.class = 0;
    pffbPixmap->pixmap.drawable.pScreen = pScreen;
    pffbPixmap->pixmap.drawable.depth = depth;
    pffbPixmap->pixmap.drawable.id = 0;
    pffbPixmap->pixmap.drawable.serialNumber = NEXT_SERIAL_NUMBER;
    pffbPixmap->pixmap.drawable.x = 0;
    pffbPixmap->pixmap.drawable.y = 0;
    pffbPixmap->pixmap.drawable.width = width;
    pffbPixmap->pixmap.drawable.height = height;
    pffbPixmap->pixmap.refcnt = 1;
    pffbPixmap->parentWin = NULL; /* non-null iff drawlib makes this a 
				     packed back buffer */


    _ffbFillInBufDesc(pScreen, pffbPixmap);
    return (&pffbPixmap->pixmap);
}

_ffbFillInBufDesc(pScreen, pPixmap)
    ScreenPtr   pScreen;
    PixmapPtr	pPixmap;
{
    /* this is largely a place holder; allocation, done above has to make decisions */
    /* based on depth,pixbits,etc.						    */
    ffbBufDPtr	bdptr;
    ffbScreenPrivPtr    scrPriv;

    scrPriv = FFBSCREENPRIV(pScreen);
    bdptr = FFBBUFDESCRIPTOR(pPixmap);
    bdptr->physDepth = scrPriv->fbdepth;
    bdptr->depthbits = pPixmap->drawable.depth;
    bdptr->pixelbits = pPixmap->drawable.bitsPerPixel;
    if (pPixmap->drawable.depth == 8) {
	bdptr->planemask = FFB_8P_PLANEMASK;
	bdptr->wid = WID_8;
        bdptr->rotateDst = ROTATE_DESTINATION_0 << DST_ROTATE_SHIFT;
        bdptr->visualDst = PACKED_EIGHT_DEST << DST_VISUAL_SHIFT;
	bdptr->rotateSrc = ROTATE_SOURCE_0 << SRC_ROTATE_SHIFT;
        bdptr->visualSrc = PACKED_EIGHT_SRC << SRC_VISUAL_SHIFT;
    } else if (pPixmap->drawable.depth == 12) {
	bdptr->planemask = FFB_12_BUF0_PLANEMASK; /* buffer 0 */
	bdptr->wid = WID_12;
	bdptr->rotateDst = ROTATE_DESTINATION_0 << DST_ROTATE_SHIFT;
	bdptr->visualDst = TWELVE_BIT_DEST << DST_VISUAL_SHIFT;
        bdptr->rotateSrc = ROTATE_DONT_CARE << SRC_ROTATE_SHIFT;
        bdptr->visualSrc = TWELVE_BIT_BUF0_SRC << SRC_VISUAL_SHIFT;
    } else {
	bdptr->planemask = FFB_24BIT_PLANEMASK;
	bdptr->wid = WID_24;
	bdptr->rotateDst = ROTATE_DESTINATION_0 << DST_ROTATE_SHIFT;
	bdptr->visualDst = TWENTYFOUR_BIT_DEST << DST_VISUAL_SHIFT;
        bdptr->rotateSrc = ROTATE_DONT_CARE << SRC_ROTATE_SHIFT;
        bdptr->visualSrc = TWENTYFOUR_BIT_SRC << SRC_VISUAL_SHIFT;
    }
}


PixmapPtr ffbScreenToMainPixmap(pPixmap)
    PixmapPtr pPixmap;
{
    PixmapPtr   pNew;
    DDXPointRec point;
    BoxRec      box;
    RegionRec   region;


    pNew = cfbCreatePixmap(pPixmap->drawable.pScreen, pPixmap->drawable.width,
	        pPixmap->drawable.height, pPixmap->drawable.depth);
    if (pNew) {
	box.x1 = 0;
	box.y1 = 0;
	box.x2 = pPixmap->drawable.width;
	box.y2 = pPixmap->drawable.height;
	point.x = 0;
	point.y = 0;
	miRegionInit(&region, &box, 1);
	/* ||| If 16, 32 bits/pixel, would need to check for compression
	    to 8 bits/pixel needed here. */
        if (pPixmap->drawable.depth > 8) {
            ffb32323232BitbltScrMemCopy((DrawablePtr)pPixmap, (DrawablePtr)pNew,
                                        &region, &point, 0, 0, -1, 0);
        } else {
            /* We know we're dealing with flavors of 8-bit things */
            if (pPixmap->drawable.bitsPerPixel > 8) {
                    /* unpacked to packed */
                    ffb32888BitbltScrMemCopy((DrawablePtr)pPixmap,
			(DrawablePtr)pNew, &region, &point, 0, 0, -1, 0);
            } else {
                    /* packed to packed */
                    ffb8888BitbltScrMemCopy((DrawablePtr)pPixmap,
			(DrawablePtr)pNew, &region, &point, 0, 0, -1, 0);
            }
        }
	/*
	 * Since there's latency between time we issue a command to move data
	 * and the time the data is moved, we can't simply return.  Ensure that
	 * the hardware is actually completed all operations.
	 */
	FFBSYNC(FFBSCREENPRIV(pPixmap->drawable.pScreen)->ffb);
	ffbDestroyPixmap(pPixmap);
    }
    return pNew;
}

Bool ffbDestroyPixmap(pPixmap)
    PixmapPtr pPixmap;
{
    PixmapPtr tptr;

    if (--pPixmap->refcnt)
	return TRUE;
    if SCREENMEMORY(pPixmap) {
	ffbScreenFree(pPixmap->drawable.pScreen, pPixmap->devPrivate.ptr,
    	     	      pPixmap->drawable.height*pPixmap->devKind);
#ifdef SOFTWARE_MODEL
	if (ffbDebug)
	ErrorF("Deallocated %d x %d from offscreen address %lx\n",
	    pPixmap->drawable.width,  pPixmap->drawable.height,
	    pPixmap->devPrivate.ptr);
#endif
    }
    /* the pixmap has the same address as the enclosing ffbpixmap */
    xfree(pPixmap);

    return TRUE;
}


#if defined(__osf__)
#if (BITMAP_BIT_ORDER == IMAGE_BYTE_ORDER)
#define LONG2CHARS(x) (x)
#else
/*
 *  the unsigned case below is for compilers like
 *  the Danbury C and i386cc
 */
#define LONG2CHARS( x ) ( ( ( ( x ) & 0x000000FFL ) << 0x18 ) \
                        | ( ( ( x ) & 0x0000FF00L ) << 0x08 ) \
                        | ( ( ( x ) & 0x00FF0000L ) >> 0x08 ) \
                        | ( ( ( x ) & (unsigned long)0xFF000000L ) >> 0x18 ) )
#endif
unsigned long ffb_endtab[32+1] =
    {
#if (BITMAP_BIT_ORDER == MSBFirst)
	LONG2CHARS( 0x00000000L ),
	LONG2CHARS( 0x80000000L ),
	LONG2CHARS( 0xC0000000L ),
	LONG2CHARS( 0xE0000000L ),
	LONG2CHARS( 0xF0000000L ),
	LONG2CHARS( 0xF8000000L ),
	LONG2CHARS( 0xFC000000L ),
	LONG2CHARS( 0xFE000000L ),
	LONG2CHARS( 0xFF000000L ),
	LONG2CHARS( 0xFF800000L ),
	LONG2CHARS( 0xFFC00000L ),
	LONG2CHARS( 0xFFE00000L ),
	LONG2CHARS( 0xFFF00000L ),
	LONG2CHARS( 0xFFF80000L ),
	LONG2CHARS( 0xFFFC0000L ),
	LONG2CHARS( 0xFFFE0000L ),
	LONG2CHARS( 0xFFFF0000L ),
	LONG2CHARS( 0xFFFF8000L ),
	LONG2CHARS( 0xFFFFC000L ),
	LONG2CHARS( 0xFFFFE000L ),
	LONG2CHARS( 0xFFFFF000L ),
	LONG2CHARS( 0xFFFFF800L ),
	LONG2CHARS( 0xFFFFFC00L ),
	LONG2CHARS( 0xFFFFFE00L ),
	LONG2CHARS( 0xFFFFFF00L ),
	LONG2CHARS( 0xFFFFFF80L ),
	LONG2CHARS( 0xFFFFFFC0L ),
	LONG2CHARS( 0xFFFFFFE0L ),
	LONG2CHARS( 0xFFFFFFF0L ),
	LONG2CHARS( 0xFFFFFFF8L ),
	LONG2CHARS( 0xFFFFFFFCL ),
	LONG2CHARS( 0xFFFFFFFEL ),
	LONG2CHARS( 0xFFFFFFFFL )
#else /* LSBFIRST */
	LONG2CHARS( 0x00000000L ),
	LONG2CHARS( 0x00000001L ),
	LONG2CHARS( 0x00000003L ),
	LONG2CHARS( 0x00000007L ),
	LONG2CHARS( 0x0000000FL ),
	LONG2CHARS( 0x0000001FL ),
	LONG2CHARS( 0x0000003FL ),
	LONG2CHARS( 0x0000007FL ),
	LONG2CHARS( 0x000000FFL ),
	LONG2CHARS( 0x000001FFL ),
	LONG2CHARS( 0x000003FFL ),
	LONG2CHARS( 0x000007FFL ),
	LONG2CHARS( 0x00000FFFL ),
	LONG2CHARS( 0x00001FFFL ),
	LONG2CHARS( 0x00003FFFL ),
	LONG2CHARS( 0x00007FFFL ),
	LONG2CHARS( 0x0000FFFFL ),
	LONG2CHARS( 0x0001FFFFL ),
	LONG2CHARS( 0x0003FFFFL ),
	LONG2CHARS( 0x0007FFFFL ),
	LONG2CHARS( 0x000FFFFFL ),
	LONG2CHARS( 0x001FFFFFL ),
	LONG2CHARS( 0x003FFFFFL ),
	LONG2CHARS( 0x007FFFFFL ),
	LONG2CHARS( 0x00FFFFFFL ),
	LONG2CHARS( 0x01FFFFFFL ),
	LONG2CHARS( 0x03FFFFFFL ),
	LONG2CHARS( 0x07FFFFFFL ),
	LONG2CHARS( 0x0FFFFFFFL ),
	LONG2CHARS( 0x1FFFFFFFL ),
	LONG2CHARS( 0x3FFFFFFFL ),
	LONG2CHARS( 0x7FFFFFFFL ),
	LONG2CHARS( 0xFFFFFFFFL )
#endif /* BITMAP_BIT_ORDER */
};



void
ffbPadPixmap(pPixmap)
    PixmapPtr pPixmap;
{
    register long h;
    register unsigned long mask;
    register unsigned long *p;
    register unsigned long bits;		/* real pattern bits */
    register long i = pPixmap->drawable.bitsPerPixel;
    register long width = pPixmap->drawable.width * i;
    long rep;				/* repeat count for pattern */

#if 0 && LONG_BIT==64
    /* companion code to stipplearea opt.  fully pad out to 64-bits.
     */
    if (i == 1) {
	register long dstwidth, shift, tmask, hbits, tbits;
	register Pixel8 *hp, *tp;
	/*
	 * pad pixmap out to LONG_BITs, leaving no bits unset.  this turns out to be
	 * very useful for general stippling code - to be guaranteed up to 31 bits of
	 * valid data past the official end of the stipple.
	 */
#define LONG_BIT_MASK	(LONG_BIT -1)
	i = width & LONG_BIT_MASK;
	if (i == 0) return;
	/* bits to pad < LONG_BIT */
	tmask = (1L << i) -1;		/* tail mask */
	rep = LONG_BIT -i;		/* # bits to fill */
	mask = (width >= LONG_BIT) ? (~0) : ((1L << width)-1);
	dstwidth = pPixmap->devKind;
	tp = hp = (Pixel8 *)(pPixmap->devPrivate.ptr);
	tp += width / LONG_BIT * sizeof(long);	/* last longword in stipple */

	for ( h=0; h < pPixmap->drawable.height; h++ )
	{
	    i = rep;
	    shift = width & LONG_BIT_MASK;
	    /* should do this as 2 word reads/writes??? */
	    hbits = (*(unsigned long *)hp) & mask;
	    tbits = (*(unsigned long *)tp) & tmask;
	    do {
		tbits |= hbits << shift;
		shift += width;
		i -= width;
	    } while (i > 0);
	    *(unsigned long *)tp = tbits;
	    hp += dstwidth;		/* next scanline */
	    tp += dstwidth;
	}
	return;
    }
#endif
#if 0 
    /* this stuff should never be true, given the conditions for entry
     * into this routine from ValidateGC().  besides, since ValidateGC
     * doesn't make a copy anymore, we can't call cfb code, which will
     * modify the size.  so...
     */
    if (width > FFBBUSBITS) {
        CFB_NAME(PadPixmap)(pPixmap);
	return;
    }
#endif

    rep = FFBBUSBITS/width;

#if 0
    if (rep*width != FFBBUSBITS) {
	CFB_NAME(PadPixmap)(pPixmap);
	return;
    }
#endif

    mask = ffb_endtab[width];
    p = (unsigned long *)(pPixmap->devPrivate.ptr);
    for (h=0; h < pPixmap->drawable.height; h++)
    {
        *p &= mask;
        bits = *p;
        for(i=1; i<rep; i++)
        {
#if (BITMAP_BIT_ORDER == MSBFirst) 
            bits >>= width;
#else
	    bits <<= width;
#endif
            *p |= bits;
        }
        p++;
    }    
/* |||  Let's just try padding, but not changing logical width
    pPixmap->drawable.width = FFBBUSBITS/(pPixmap->drawable.bitsPerPixel);
*/
}
#endif


PixmapPtr ffbExpandTile(ptile)
    PixmapPtr ptile;
{
    int width = (int)ptile->drawable.width;
    int height;
    unsigned int *psrc;
    unsigned int *pdst;
    PixmapPtr ntile;

    /*
     * check to see if this is an 8-pixel-wannabe tile.
     */
    if (width >= FFBBLOCKTILEPIXELS || !PowerOfTwo(width)) {
	return (PixmapPtr)NULL;
    }

    /* Create a new tile */
    height = (int)ptile->drawable.height;
    ntile = ffbCreatePixmap(ptile->drawable.pScreen, FFBBLOCKTILEPIXELS,
	    height, ptile->drawable.depth);

    if (ntile == (PixmapPtr)NULL)
	return ntile;

    psrc = (unsigned int *)(ptile->devPrivate.ptr);
    pdst = (unsigned int *)(ntile->devPrivate.ptr);

    /* not generalized; only 1,2,4 -> 8 handled */
    if (ptile->drawable.depth == 8) {
	while (height-- > 0) {
	    int r0 = *psrc;
	    switch (width) {
	     case 1:
		r0 &= 0xff;
		r0 |= r0 << 8;
	     case 2:
		r0 &= 0xffff;
		r0 |= r0 << 16;
	    }
	    pdst[0] = pdst[1] = r0;
	    pdst += 2;
	    psrc += ptile->devKind/sizeof(unsigned int);
	}
    } else {
	while (height-- > 0) {
	    switch (width) {
	     case 1:
		pdst[0] = pdst[1] = pdst[2] = pdst[3] =
		    pdst[4] = pdst[5] = pdst[6] = pdst[7] = psrc[0];
		break;
	     case 2:
		pdst[0] = pdst[2] = pdst[4] = pdst[6] = psrc[0];
		pdst[1] = pdst[3] = pdst[5] = pdst[7] = psrc[1];
		break;
	     case 4:
		pdst[0] = pdst[4] = psrc[0];
		pdst[1] = pdst[5] = psrc[1];
		pdst[2] = pdst[6] = psrc[2];
		pdst[3] = pdst[7] = psrc[3];
	    }
	    psrc += width;
	    pdst += 8;
	}
    }
    ntile->drawable.width = FFBBLOCKTILEPIXELS;
    return ntile;
}


/*
 * HISTORY
 */
