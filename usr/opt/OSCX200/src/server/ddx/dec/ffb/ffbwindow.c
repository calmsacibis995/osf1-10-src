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
static char *rcsid = "@(#)$RCSfile: ffbwindow.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:19:23 $";
#endif
/*
 */

#include "X.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include "mistruct.h"
#include "regionstr.h"

#include "ffb.h"
#include "ffbwindow.h"
#include "ffbblt.h"

extern WindowPtr *WindowTable;

Bool
ffbCreateWindow(pWin)
    WindowPtr		pWin;
{
    ffbBufDPtr		pbd;
    ScreenPtr		pScreen;
    ffbScreenPrivPtr	scrPriv;

    /* fill out the devpriv part */
    pScreen = pWin->drawable.pScreen;
    scrPriv = FFBSCREENPRIV(pScreen);

    pWin->drawable.bitsPerPixel = scrPriv->fbdepth;

    pbd = FFBBUFDESCRIPTOR(pWin);
    pbd->physDepth = scrPriv->fbdepth;
    pbd->depthbits = pWin->drawable.depth;
    pbd->pixelbits = scrPriv->fbdepth;
    /* ||| Jean or Sam...when you put in overlay code here, note that the 
       MoveWindow code below has to change also, as marked with ``|||''. */
    if (pWin->drawable.depth == 4) {
	pbd->planemask = FFB_OVRLY_PLANEMASK;
	/* never the rootWindow, so must have a parent */
	pbd->wid = FFBBUFDESCRIPTOR(pWin->parent)->wid;
	/*||| are these right? */
	pbd->rotateDst = ROTATE_DESTINATION_3 << DST_ROTATE_SHIFT;
	pbd->rotateSrc = ROTATE_SOURCE_3 << SRC_ROTATE_SHIFT;
	pbd->visualDst = UNPACKED_EIGHT_DEST << DST_VISUAL_SHIFT;
	pbd->visualSrc = UNPACKED_EIGHT_SRC << SRC_VISUAL_SHIFT;
    } else if (pWin->drawable.depth == 8) {
	pbd->rotateDst = ROTATE_DESTINATION_0 << DST_ROTATE_SHIFT;
	pbd->rotateSrc = ROTATE_SOURCE_0 << SRC_ROTATE_SHIFT;
	pbd->planemask = FFB_8U_PLANEMASK;    /* 8U and 8P planemasks 
						 are really the same value */
	pbd->wid = WID_8;
	if (scrPriv->fbdepth == 8) {
	    pbd->visualDst = PACKED_EIGHT_DEST << DST_VISUAL_SHIFT;
	    pbd->visualSrc = PACKED_EIGHT_SRC << SRC_VISUAL_SHIFT;
	} else {
	    pbd->visualDst = UNPACKED_EIGHT_DEST << DST_VISUAL_SHIFT;
	    pbd->visualSrc = UNPACKED_EIGHT_SRC << SRC_VISUAL_SHIFT;
	}
    } else if (pWin->drawable.depth == 12) {
	pbd->planemask = FFB_12_BUF0_PLANEMASK;
	pbd->wid = WID_12;
	pbd->rotateDst = ROTATE_DONT_CARE << DST_ROTATE_SHIFT;
	pbd->rotateSrc = ROTATE_DONT_CARE << SRC_ROTATE_SHIFT;
	pbd->visualDst = TWELVE_BIT_DEST << DST_VISUAL_SHIFT;
	pbd->visualSrc = TWELVE_BIT_BUF0_SRC << SRC_VISUAL_SHIFT;
    } else {
	pbd->planemask = FFB_24BIT_PLANEMASK;
	pbd->wid = WID_24;
	pbd->rotateDst = ROTATE_DONT_CARE << DST_ROTATE_SHIFT;
	pbd->rotateSrc = ROTATE_DONT_CARE << SRC_ROTATE_SHIFT;
	pbd->visualDst = TWENTYFOUR_BIT_DEST << DST_VISUAL_SHIFT;
	pbd->visualSrc = TWENTYFOUR_BIT_SRC << SRC_VISUAL_SHIFT;
    }
    return TRUE;
}


void ffbCopyWindow(pWin, ptOldOrg, prgnSrc)
    WindowPtr pWin;
    DDXPointRec ptOldOrg;
    RegionPtr prgnSrc;
{
    DDXPointPtr		    pptSrc;
    register DDXPointPtr    ppt;
    RegionRec		    rgnDst;
    register BoxPtr	    pbox;
    register int	    dx, dy;
    register int	    nbox;
    WindowPtr		    pwinRoot;
    ffbScreenPrivPtr	    scrPriv;
    FFB			    ffb;
    VoidProc		    CopyFunc;
    int			    row;
    long		    planemask;

    pwinRoot = WindowTable[pWin->drawable.pScreen->myNum];

    (* pWin->drawable.pScreen->RegionInit) (&rgnDst, NullBox, 0);

    dx = ptOldOrg.x - pWin->drawable.x;
    dy = ptOldOrg.y - pWin->drawable.y;
    (* pWin->drawable.pScreen->TranslateRegion)(prgnSrc, -dx, -dy);
    (* pWin->drawable.pScreen->Intersect)(&rgnDst, &pWin->borderClip, prgnSrc);

    pbox = REGION_RECTS(&rgnDst);
    nbox = REGION_NUM_RECTS(&rgnDst);
    if( !nbox || !(pptSrc = (DDXPointPtr )ALLOCATE_LOCAL(nbox * sizeof(DDXPointRec))))
    {
  	(* pWin->drawable.pScreen->RegionUninit) (&rgnDst);
        return;
    }

    for (ppt = pptSrc; nbox != 0; nbox--, ppt++, pbox++) {
        ppt->x = pbox->x1 + dx;
        ppt->y = pbox->y1 + dy;
    }
 
    scrPriv = FFBSCREENPRIV(pwinRoot->drawable.pScreen);
    scrPriv->lastGC = (GCPtr)NULL;
    ffb = scrPriv->ffb;

    FFB_SELECTROW(pWin->drawable.depth,
                  pWin->drawable.bitsPerPixel, pWin->drawable.bitsPerPixel,
                  row);

    switch (row) {
     case _PACKED_TO_PACKED:
	/* must be on an 8-plane device */
	/* there are no control planes to fuss with */
	WRITE_MEMORY_BARRIER();
        FFBROP(ffb, GXcopy, ROTATE_DESTINATION_0 << DST_ROTATE_SHIFT,
			    PACKED_EIGHT_DEST << DST_VISUAL_SHIFT);
	planemask = 0xffffffff;
	break;

     case _UNPACKED_TO_UNPACKED:
	/* First, fill in window id's in new area.  ||| (This can be made
	   more efficient by painting only the nonoverlapped area.)
	   */
	/* ||| For overlays, we don't need to move the wid, since non-0 values
	 * in the overlay planes make the wid irrelevant.  The wrapper will take
	 * care of telling ValidateTree to clear the vacated overlay area(s).
	 */
	WRITE_MEMORY_BARRIER();
	if (pWin->drawable.depth == 4)
	{
	    FFBROP(ffb, GXcopy, ROTATE_DESTINATION_3 << DST_ROTATE_SHIFT, 
		   UNPACKED_EIGHT_DEST << DST_VISUAL_SHIFT);
	}
	else
	{
	    ffb32PaintWindowID(pWin, &rgnDst, FALSE);
	    FFBROP(ffb, GXcopy, ROTATE_DESTINATION_0 << DST_ROTATE_SHIFT, 
		   UNPACKED_EIGHT_DEST << DST_VISUAL_SHIFT);
	}
	planemask = 0xffffffff;
	break;

     case _TWELVE_BITS_DEEP:
     case _THIRTYTWO_BITS_DEEP:
	/* Copy everything, including window ID bits. */
	WRITE_MEMORY_BARRIER();
        FFBROP(ffb, GXcopy, ROTATE_DESTINATION_0 << DST_ROTATE_SHIFT,
			    TWENTYFOUR_BIT_DEST << DST_VISUAL_SHIFT);
	planemask = 0xf0ffffff;
	break;

     case _PACKED_TO_UNPACKED:
        /* can this happen? */
     case _UNPACKED_TO_PACKED:
        /* can this happen? */
     default:
	abort();
    }

    CYCLE_REGS(ffb);
    /* ||| This is wrong for 4-bit overlays.  We should use a planemask of
       0xf0ffffff for normal windows, and 0x0f000000 for overlays, right? */
    /* okay. */
    FFBPLANEMASK(scrPriv, ffb, planemask);

    CopyFunc = ffbCopyTab[row][_SCREEN_SCREEN];
    (*CopyFunc)(pwinRoot, pwinRoot, &rgnDst, pptSrc);

    DEALLOCATE_LOCAL(pptSrc);
    (* pWin->drawable.pScreen->RegionUninit)(&rgnDst);
}

Bool ffbIgnoreWindow(pWin)
    WindowPtr		pWin;
{
    return TRUE;
}


/*
 * HISTORY
 */
