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
#ifdef VMS
#define IDENT "X-2"
#define MODULE_NAME SFBPIXMAP
#include "module_ident.h"
#endif

#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /alphabits/u3/x11/ode/rcs/x11/src/server/ddx/dec/sfb/sfbpixmap.c,v 1.1.3.9 93/01/22 15:57:46 Jim_Ludwig Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */

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
**
**  AUTHORS:
**
**      Irene McCartney (from Joel McCormack)
**
**  CREATION DATE:     19-Nov-1991
**
**  MODIFICATION HISTORY:
**
** X-002	BIM0011		Irene McCartney			27-Jan-1992
**		Add edit history
**		Merge latest changes from Joel
**
**--
**/

#include "Xmd.h"
#include "X.h"

#include "servermd.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "mi.h"
#include "regionstr.h"

#include "cfbpixmap.h"
#include "sfb.h"
#include "sfbpixmap.h"
/*
 *	Next-fit storage allocation mechanism
 *
 *	Algorithm and variable names stolen from Knuth V1, p 437,
 *		with suggested modifications from Exercise 2.5.6.
 *
 *	Storage is manipulated in terms of UNITs, the amount needed
 *		to store a pointer and length.
 */

#define	UNIT		sizeof(FreeElement)

/*
 *	free list is kept in address order
 *	avail is a dummy header that points to first free element
 *	rover is pointer into free list
 */

/*
 *	sfbScreenMalloc(pScreen, n) returns pointer to n WORDS of storage,
 *      or NULL.
 */

#include <stdio.h>
pointer sfbScreenMalloc(pScreen, n) 
    ScreenPtr   pScreen;
    int		n;
{
    register FreeElement *p, *q, *r, *start;
    register int	size;
    sfbScreenPrivPtr    scrPriv;
    SFB			sfb;

#ifdef SOFTWARE_MODEL
    return (pointer) NULL;
#endif

    scrPriv = SFBSCREENPRIV(pScreen);
    sfb = scrPriv->sfb;
    SFBPLANEMASK(sfb, SFBBUSALL1);
    SFBROP(sfb, GXcopy);
    SFBFLUSHMODE(sfb, SIMPLE);
    scrPriv->lastGC = (GCPtr)NULL;

    size = (n + UNIT - 1) / UNIT;
    if (scrPriv->rover == (FreeElement *)NULL) 
    	scrPriv->rover = &(scrPriv->avail);
    q = scrPriv->rover;
    start = p = q->link;
    /* search for a block large enough */
    while ((p != (FreeElement *)NULL) && (p->size < size)) {
	/* keep looking */
	q = p;
	p = p->link;
    }
    if (p == (FreeElement *)NULL) {
	/* try rest of list */
	q = &(scrPriv->avail);
	p = q->link;
	while ((p != start) && (p->size < size)) {
	    q = p;
	    p = p->link;
	}
	if (p == start) {
	    /* can't allocate to screen memory */
	    return (pointer)NULL;
	}
    }
    if (p->size == size) {
	/* found one of right size. remove it */
	q->link = p->link;
    } else if (p->size > size) {
	/* found one too big. take part of it */
	r = p + size;	/* remaining free area */
	q->link = r;
	r->link = p->link;
	r->size = p->size - size;
    }
    scrPriv->rover = q->link;
    return (pointer)p;
}

/*
 *	sfbScreenFree(pScreen, p0, n) adds the block of n bytes pointed to by p0
 *		 to the free list
 */

void sfbScreenFree(pScreen, p0, n)
    ScreenPtr   pScreen;
    pointer     p0;
    int		n;
{
    register FreeElement *p, *q, *r;
    register int	size;
    sfbScreenPrivPtr    scrPriv;
    SFB			sfb;

#ifdef SOFTWARE_MODEL
    return;
#endif
    scrPriv = SFBSCREENPRIV(pScreen);
    sfb = scrPriv->sfb;
    SFBPLANEMASK(sfb, SFBBUSALL1);
    SFBROP(sfb, GXcopy);
    SFBFLUSHMODE(sfb, SIMPLE);
    scrPriv->lastGC = (GCPtr)NULL;

#ifdef VMS
    size = (n + UNIT - 1) / UNIT;
#else
    size = n / UNIT;
#endif

    r = (FreeElement *)p0;
    q = &(scrPriv->avail);
    p = q->link;
    /* search for the right place */
    while ((p != (FreeElement *)NULL) && (p < r)) {
	/* not the place, keep searching */
	q = p;
	p = p->link;
    }
    /* this is where it should go */
    /* note: since NULL = 0, if p = NULL, p != r + size */
    if (p == r + size) {
	/* new block abuts p, consolidate */
	size += p->size;
	r->link = p->link;
    } else {
	/* does not abut, just connect */
	r->link = p;
    }
    if (r == q + q->size) {
	/* new block abuts q, consolidate */
	q->size += size;
	q->link = r->link;
    } else {
	/* does not abut, just connect */
	q->link = r;
	r->size = size;
    }
    scrPriv->rover = q;	/* start searching here next time */
}

#ifdef SOFTWARE_MODEL
Bool mainOnly = TRUE;
#else
Bool mainOnly = FALSE;
#endif

PixmapPtr sfbCreatePixmap(pScreen, width, height, depth)
    ScreenPtr	pScreen;
    int		width;
    int		height;
    int		depth;
{
    register PixmapPtr pPixmap;
    register pointer   base;
    int size;

    if (mainOnly) return cfbCreatePixmap(pScreen, width, height, depth);
 
    /* Don't bother allocating pixmaps that are so small that they won't really
       benefit from the sfb acceleration. */
    /* ||| Note that for 16, 32 bits/pixel, this means won't allocate anything
       of 8 bits/pixel to off-screen memory.  I don't know if this is good or
       bad, myself.  Could change test to ``depth == 1 || width <= 64'' */
    if (depth != SFBPIXELBITS || width <= 64)
	return cfbCreatePixmap(pScreen, width, height, depth);

    /* Big pixmaps are pretty useless and will screw up server.  Probably we
       got such large numbers because someone passed in a negative height by
       accident, anyway.  We have this weird maximum so that 0-width lines,
       including the cap, never exceed the 15-bit world.  The sfb really likes
       this idea, as it doesn't have to mask the damn length then. */
    if (width > 32766 || height > 32766)
	return NullPixmap;

    /* Try allocating to offscreen memory. */
    size = ((width*SFBPIXELBYTES) + SFBALIGNMASK) & ~SFBALIGNMASK;
    base = sfbScreenMalloc(pScreen, height * size);
    if (!base) return cfbCreatePixmap(pScreen, width, height, depth);

    /* Okay, we got the off-screen memory. */
    pPixmap = (PixmapPtr)xalloc(sizeof(PixmapRec));
    if (!pPixmap) {
	sfbScreenFree(pScreen, base, height * size);
	return NullPixmap;
    }
    pPixmap->drawable.type = DRAWABLE_PIXMAP;
    pPixmap->drawable.class = 0;
    pPixmap->drawable.pScreen = pScreen;
    pPixmap->drawable.depth = depth;
    pPixmap->drawable.bitsPerPixel = SFBPIXELBITS;
    pPixmap->drawable.id = 0;
    pPixmap->drawable.serialNumber = NEXT_SERIAL_NUMBER;
    pPixmap->drawable.x = 0;
    pPixmap->drawable.y = 0;
    pPixmap->drawable.width = width;
    pPixmap->drawable.height = height;
    pPixmap->devKind = size;
    pPixmap->refcnt = 1;
    pPixmap->devPrivate.ptr = base;
/*
    printf("Allocated %d x %d at offscreen address %lx\n", width, height, base);
*/
    return pPixmap;
}

PixmapPtr sfbScreenToMainPixmap(pPixmap)
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
	sfbBitbltScrMemCopy((DrawablePtr)pPixmap, (DrawablePtr)pNew,
	    &region, &point, 0, 0, -1, 0);
	sfbDestroyPixmap(pPixmap);
    }
    return pNew;
}

Bool sfbDestroyPixmap(pPixmap)
    PixmapPtr pPixmap;
{

    if (--pPixmap->refcnt)
	return TRUE;
    if SCREENMEMORY(pPixmap) {
	sfbScreenFree(pPixmap->drawable.pScreen, pPixmap->devPrivate.ptr,
    	     	      pPixmap->drawable.height*pPixmap->devKind);
/*
	printf("Deallocated %d x %d from offscreen address %lx\n",
	    pPixmap->drawable.width,  pPixmap->drawable.height,
	    pPixmap->devPrivate.ptr);
*/
    }
    xfree(pPixmap);
    return TRUE;
}

#if LONG_BIT == 64
/* endtab taken from mfb/maskbits.c
 *
 * PadPixmap taken from cfbPadPixmap and reduced to 32-bits.
 */
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
unsigned long sfb_endtab[32+1] =
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
sfbPadPixmap(pPixmap)
    PixmapPtr pPixmap;
{
    register int width = 
	(pPixmap->drawable.width) * (pPixmap->drawable.bitsPerPixel);
    register int h;
    register unsigned long mask;
    register unsigned long *p;
    register unsigned long bits; /* real pattern bits */
    register int i;
    int rep;                    /* repeat count for pattern */
 
    if (width > 32) {
        cfbPadPixmap(pPixmap);
	return;
    }

    rep = 32/width;
    if (rep*width != 32) {
        cfbPadPixmap(pPixmap);
	return;
    }
 
    mask = sfb_endtab[width];
 
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
    pPixmap->drawable.width = 32/(pPixmap->drawable.bitsPerPixel);
}
#endif /* LONG_BIT == 64 */
