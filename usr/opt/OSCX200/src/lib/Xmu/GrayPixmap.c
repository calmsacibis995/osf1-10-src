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
/* $XConsortium: GrayPixmap.c,v 1.7 92/03/06 14:06:02 converse Exp $ */

/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/Xmu/Drawing.h>

typedef struct _PixmapCache {
    Screen *screen;
    Pixmap pixmap;
    Pixel foreground, background;
    unsigned int depth;
    int ref_count;
    struct _PixmapCache *next;
  } CacheEntry;

static CacheEntry *pixmapCache = NULL;



Pixmap XmuCreateStippledPixmap(screen, fore, back, depth)
    Screen *screen;
    Pixel fore, back;
    unsigned int depth;
/*
 *	Creates a stippled pixmap of specified depth
 *	caches these so that multiple requests share the pixmap
 */
{
    register Display *display = DisplayOfScreen(screen);
    CacheEntry *cachePtr;
    Pixmap stippled_pixmap;
    static unsigned char pixmap_bits[] = {
	0x02, 0x01,
    };

/*
 *	Creates a stippled pixmap of depth DefaultDepth(screen)
 *	caches these so that multiple requests share the pixmap
 */

#define pixmap_width 2
#define pixmap_height 2

    /* see if we already have a pixmap suitable for this screen */
    for (cachePtr = pixmapCache; cachePtr; cachePtr = cachePtr->next) {
	if (cachePtr->screen == screen && cachePtr->foreground == fore &&
	    cachePtr->background == back && cachePtr->depth == depth)
	    return( cachePtr->ref_count++, cachePtr->pixmap );
    }

    stippled_pixmap = XCreatePixmapFromBitmapData (display,
			RootWindowOfScreen(screen), (char *)pixmap_bits, 
			pixmap_width, pixmap_height, fore, back, depth);

    /* and insert it at the head of the cache */
    cachePtr = XtNew(CacheEntry);
    cachePtr->screen = screen;
    cachePtr->foreground = fore;
    cachePtr->background = back;
    cachePtr->depth = depth;
    cachePtr->pixmap = stippled_pixmap;
    cachePtr->ref_count = 1;
    cachePtr->next = pixmapCache;
    pixmapCache = cachePtr;

    return( stippled_pixmap );
}

void XmuReleaseStippledPixmap(screen, pixmap)
    Screen *screen;
    Pixmap pixmap;
{
    register Display *display = DisplayOfScreen(screen);
    CacheEntry *cachePtr, **prevP;
    for (prevP = &pixmapCache, cachePtr = pixmapCache; cachePtr;) {
	if (cachePtr->screen == screen && cachePtr->pixmap == pixmap) {
	    if (--cachePtr->ref_count == 0) {
		XFreePixmap( display, pixmap );
		*prevP = cachePtr->next;
		XtFree( (char*)cachePtr );
		break;
	    }
	}
	prevP = &cachePtr->next;
	cachePtr = *prevP;
    }
}
