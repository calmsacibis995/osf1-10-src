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

#include "servermd.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "dix.h"
#include "cfb.h"

extern int cfb32ScreenPrivateIndex;
static unsigned long cfb32DrawGeneration = 0;


void
cfbDrawClose(pScreen)
    ScreenPtr pScreen;
{
    xfree(pScreen->devPrivate);
}

/*
Initialize cfb drawing code.  Xfb cannot call cfbScreenInit() directly
because it does too much.
Inputs:
    cfbWindowPrivateIndex = window index to be used by cfb code, this
        is ignored because dec/cfb does not use window privates
    cfbGCPrivateIndex = GC index to be used by cfb code
*/
Bool
cfbDrawInit(pScreen, pbits, xsize, ysize, width, WindowPrivateIndex,
    GCPrivateIndex)
    register ScreenPtr pScreen;
    pointer pbits;              /* pointer to screen bitmap */
    int xsize, ysize;           /* in pixels */
    int width;                  /* pixel width of frame buffer */
    int WindowPrivateIndex;
    int GCPrivateIndex;
{
    PixmapPtr pPixmap;

    if (!AllocateGCPrivate(pScreen, GCPrivateIndex, sizeof(cfbPrivGC)))
        return FALSE;

    pPixmap = (PixmapPtr ) xalloc(sizeof(PixmapRec));
    if (!pPixmap)
        return FALSE;
    pPixmap->drawable.type = DRAWABLE_PIXMAP;
    pPixmap->drawable.depth = 8;
    pPixmap->drawable.pScreen = pScreen;
    pPixmap->drawable.serialNumber = 0;
    pPixmap->drawable.x = 0;
    pPixmap->drawable.y = 0;
    pPixmap->drawable.width = xsize;
    pPixmap->drawable.height = ysize;
    pPixmap->refcnt = 1;
    pPixmap->devPrivate.ptr = pbits;
    pPixmap->devKind = PixmapBytePad(width, 8);
    pScreen->devPrivate = (pointer)pPixmap;

    return (TRUE);
}


/*
This allows the xfbBankSwitch module to change the address of the screen
pixmap
*/
void
xfbDrawSetFb24(pScreen, fb24)
    ScreenPtr pScreen;
    pointer fb24;
{
    ((PixmapPtr) pScreen->devPrivates[cfb32ScreenPrivateIndex].ptr)
	->devPrivate.ptr = fb24;
}

void
cfb32DrawClose(pScreen)
    ScreenPtr pScreen;
{
    xfree(pScreen->devPrivates[cfb32ScreenPrivateIndex].ptr);
}

/*
Initialize tfb drawing code.  Xfb cannot call tfbScreenInit() directly
because it does too much.  See tfbscreen.h to see how drawing routines
access screen pixmap.
Inputs:
    cfbWindowPrivateIndex = window index to be used by tfb code
    cfbGCPrivateIndex = GC index to be used by tfb code

Bugs: Need to get rid of window private someday.  Rotated pixmaps don't
    make sense for 1 PPW.
*/
Bool
cfb32DrawInit(pScreen, fb24, xsize, ysize, width, WindowPrivateIndex,
    GCPrivateIndex)
    ScreenPtr pScreen;
    pointer fb24;		/* address of 24-bit framebuffer */
    int xsize, ysize;		/* in pixels */
    int width;			/* pixel width of frame buffer */
    int WindowPrivateIndex;
    int GCPrivateIndex;
{
    PixmapPtr pPixmap;

    if (cfb32DrawGeneration != serverGeneration) {
	if ((cfb32ScreenPrivateIndex = AllocateScreenPrivateIndex()) < 0) {
	    return (FALSE);
	}
	cfb32DrawGeneration = serverGeneration;

	/* allocate the screen privates for the cfb32 layer. If not,
	 * then cfb32GCPrivateIndex won't get initialized and it needs
	 * to if we plan to use these things. cfb32PuntCopyPlane
	 * will also never get initialized.
	 */
        if (!cfb32AllocatePrivates(pScreen, (int *) 0L, (int *) 0L))
	    return FALSE;
    }

    if (!AllocateWindowPrivate(pScreen, WindowPrivateIndex,
			       sizeof(cfbPrivWin)) ||
	!AllocateGCPrivate(pScreen, GCPrivateIndex, sizeof(cfbPrivGC))) {
	return FALSE;
    }

    /* store 24-bit frame buffer pixmap in screen private: */
    pPixmap = (PixmapPtr) xalloc(sizeof(PixmapRec));
    if (!pPixmap)
	return FALSE;
    pPixmap->drawable.type = DRAWABLE_PIXMAP;
    pPixmap->drawable.class = 0;
    pPixmap->drawable.pScreen = pScreen;
    pPixmap->drawable.depth = 24;
    pPixmap->drawable.bitsPerPixel = 32;
    pPixmap->drawable.id = 0;
    pPixmap->drawable.serialNumber = NEXT_SERIAL_NUMBER;
    pPixmap->drawable.x = 0;
    pPixmap->drawable.y = 0;
    pPixmap->drawable.width = xsize;
    pPixmap->drawable.height = ysize;
    pPixmap->devKind = PixmapBytePad(width, 24);
    pPixmap->refcnt = 1;
    pPixmap->devPrivate.ptr = fb24;
    pScreen->devPrivates[cfb32ScreenPrivateIndex].ptr = (pointer) pPixmap;

    return (TRUE);
}
