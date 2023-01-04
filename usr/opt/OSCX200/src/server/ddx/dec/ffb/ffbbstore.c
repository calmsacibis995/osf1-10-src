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
static char *rcsid = "@(#)$RCSfile: ffbbstore.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:07:42 $";
#endif
/*
 */

#include    "X.h"
#include    "regionstr.h"
#include    "scrnintstr.h"
#include    "pixmapstr.h"
#include    "windowstr.h"
#include    "mibstore.h"

#include    "ffb.h"
#include    "ffbbstore.h"
#include    "ffbblt.h"


/*-
 *-----------------------------------------------------------------------
 * ffbSaveAreas --
 *	Function called by miSaveAreas to actually fetch the areas to be
 *	saved into the backing pixmap. This is very simple to do, since
 *	ffbBitbltXxxYyy is designed for this very thing. The region to save is
 *	already destination-relative and we're given the offset to the
 *	window origin, so we have only to create an array of points of the
 *	u.l. corners of the boxes in the region translated to the screen
 *	coordinate system and fetch the screen pixmap out of its devPrivate
 *	field....
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Data are copied from the screen into the pixmap.
 *
 *-----------------------------------------------------------------------
 */

void
ffbSaveAreas(pPixmap, prgnSave, xorg, yorg, pWin)
    PixmapPtr	  	pPixmap;  	/* Backing pixmap */
    RegionPtr	  	prgnSave; 	/* Region to save (pixmap-relative) */
    int	    	  	xorg;	    	/* X origin of region */
    int	    	  	yorg;	    	/* Y origin of region */
    WindowPtr		pWin;
{
    register DDXPointPtr pPt;
    DDXPointPtr		pPtsInit;
    register BoxPtr	pBox;
    register int	i;
    DrawablePtr		pSrcDraw;
    FFB			ffb;
    VoidProc		copyProc;
    int			row;
    ffbScreenPrivPtr    scrPriv;
 
    i = REGION_NUM_RECTS(prgnSave);
    pPtsInit = (DDXPointPtr)ALLOCATE_LOCAL(i * sizeof(DDXPointRec));
    if (!pPtsInit)
	return;

    pBox = REGION_RECTS(prgnSave);
    /* The region to save is relative to the screen.  However, most of the
       information we want about the window depth is in pWin.  Yuck.
       XXX: Maybe not so yucky: the window baseptr is actually the 
       screen baseptr.  So the offsets in the copy code turn out
       right by simply passing in pWin. */
       
    for (pPt = pPtsInit; i != 0; i--) {
	pPt->x = pBox->x1 + xorg;
	pPt->y = pBox->y1 + yorg;
	pPt++;
	pBox++;
    }

    FFB_SELECTROW(pWin->drawable.depth, pWin->drawable.bitsPerPixel,
                  pPixmap->drawable.bitsPerPixel, row);

    scrPriv = FFBSCREENPRIV(pWin->drawable.pScreen);
    scrPriv->lastGC = (GCPtr)NULL;
    ffb = scrPriv->ffb;
    WRITE_MEMORY_BARRIER();
    
    if (SCREENMEMORY(pPixmap)) {
	ffbBufDPtr pbd;

	pbd = FFBBUFDESCRIPTOR(pPixmap);
	
        FFBROP(ffb, GXcopy, pbd->rotateDst, pbd->visualDst);
	CYCLE_REGS(ffb);
	FFBPLANEMASK(scrPriv, ffb, FFBBUSALL1);
        copyProc = ffbCopyTab[row][_SCREEN_SCREEN];
        (*copyProc) (pWin, (DrawablePtr)pPixmap, prgnSave, pPtsInit);
    } else {
	/* might not have been allocated by us, so can't use bufdescriptor;
	   but not as many permutations possible, since we guarantee that 
	   main memory pixmaps are packed, and all 12-bit drawables in main
	   memory have same format, so just figure it out */
	int                 tvisual;

        switch (row) {
	 case _PACKED_TO_PACKED:
         case _UNPACKED_TO_PACKED:
	    tvisual =  PACKED_EIGHT_DEST;
	    break;
	 case _THIRTYTWO_BITS_DEEP:
            tvisual = TWENTYFOUR_BIT_DEST;
            break;
	 case _TWELVE_BITS_DEEP:
	    tvisual = TWELVE_BIT_DEST;
	    break;
         default:
            abort();		
	}

	FFBROP(ffb, GXcopy, ROTATE_DONT_CARE << DST_ROTATE_SHIFT,
	       tvisual << DST_VISUAL_SHIFT);
	copyProc = ffbCopyTab[row][0]; /* scr_mem copy with rop copy */
        (*copyProc) (pWin, (DrawablePtr)pPixmap, prgnSave, pPtsInit,
                        0, 0, -1, 0);
    }

    DEALLOCATE_LOCAL(pPtsInit);

    /*
     * Since there's latency between time we issue a command to move data
     * and the time the data is moved, we can't simply return.  For now
     * we ensure that the hardware has actually completed all operations.
     */
    FFBSYNC(ffb);

}

/*-
 *-----------------------------------------------------------------------
 * ffbRestoreAreas --
 *	Function called by miRestoreAreas to actually fetch the areas to be
 *	restored from the backing pixmap. This is very simple to do, since
 *	ffbBitbltXxxYyy is designed for this very thing.  The region to 
 *      restore is already destination-relative and we're given the offset to
 *	the window origin, so we have only to create an array of points of the
 *	u.l. corners of the boxes in the region translated to the pixmap
 *	coordinate system and fetch the screen pixmap out of its devPrivate
 *	field....
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Data are copied from the pixmap into the screen.
 *
 *-----------------------------------------------------------------------
 */
void
ffbRestoreAreas(pPixmap, prgnRestore, xorg, yorg, pWin)
    PixmapPtr	  	pPixmap;  	/* Backing pixmap */
    RegionPtr	  	prgnRestore; 	/* Region to restore (screen-relative)*/
    int	    	  	xorg;	    	/* X origin of window */
    int	    	  	yorg;	    	/* Y origin of window */
    WindowPtr		pWin;
{
    register DDXPointPtr pPt;
    DDXPointPtr		pPtsInit;
    register BoxPtr	pBox;
    register int	i;
    DrawablePtr		pDstDraw;
    ffbScreenPrivPtr    scrPriv;
    FFB			ffb;
    CommandWord		planemask;
    int			row;
    int			tvisual;
    VoidProc		copyProc;
    ffbBufDPtr 		pbd;

    i = REGION_NUM_RECTS(prgnRestore);
    pPtsInit = (DDXPointPtr)ALLOCATE_LOCAL(i*sizeof(DDXPointRec));
    if (!pPtsInit)
	return;
    
    pBox = REGION_RECTS(prgnRestore);
    pPt = pPtsInit;
    for (; i != 0; i--) {
	pPt->x = pBox->x1 - xorg;
	pPt->y = pBox->y1 - yorg;
	pPt++;
	pBox++;
    }

    /* take care of window id's */
    scrPriv = FFBSCREENPRIV(pWin->drawable.pScreen);
    scrPriv->lastGC = (GCPtr)NULL;
    if (scrPriv->fbdepth == 32) {
        ffb32PaintWindowID(pWin, prgnRestore, FALSE);
	scrPriv->lastGC = (GCPtr)NULL;
    }
    ffb = scrPriv->ffb;

    /*
     * We know that the destination is on screen...
     */
    /* XXX: can we assume that the rotation is fixed? */
    pbd = FFBBUFDESCRIPTOR(pWin);

    FFB_SELECTROW(pPixmap->drawable.depth, pPixmap->drawable.bitsPerPixel,
                  pWin->drawable.bitsPerPixel, row);

    if (SCREENMEMORY(pPixmap)) {
	copyProc = ffbCopyTab[row][_SCREEN_SCREEN];
    } else {
	copyProc = ffbCopyTab[row][_MEM_SCREEN];
    }

    WRITE_MEMORY_BARRIER();
    /* "LOADSTATE" won't happen, since we're calling copy code */
    FFBROP(ffb, GXcopy, pbd->rotateDst, pbd->visualDst);
    CYCLE_REGS(ffb);
    FFBPLANEMASK(scrPriv, ffb, pbd->planemask);

    (*copyProc)((DrawablePtr)pPixmap, pWin, prgnRestore, pPtsInit);

    DEALLOCATE_LOCAL(pPtsInit);

    /*
     * Since there's latency between time we issue a command to move data
     * and the time the data is moved, we can't simply return.  For now
     * we ensure that the hardware is actually completed all operations.
     */
    FFBSYNC(ffb);

}

/*
 * HISTORY
 */
