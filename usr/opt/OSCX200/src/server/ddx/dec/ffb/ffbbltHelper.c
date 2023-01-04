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
static char *rcsid = "@(#)$RCSfile: ffbbltHelper.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:07:37 $";
#endif
/*
 */

#include "X.h"
#include "Xprotostr.h"

#include "miscstruct.h"
#include "regionstr.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"

#include "ffb.h"
#include "ffbblt.h"
#include "ffbdma.h"

/* Sort rectangles and points into the correct order to process them, so as
   to not destroy information if the source and destination overlap.  Returns
   FALSE if allocations fail. */
/*
  well... there's one of these in sfb/ but how do we handle this with shared
  libs?
 */

Bool ffbSortRectsAndPoints(ppboxInit, ppptInit, nbox, xdir, ydir)
    BoxPtr		    *ppboxInit;
    DDXPointPtr		    *ppptInit;
    register int	    nbox;
    int			    xdir; /* 0: left to right, 1: right to left */
    int			    ydir; /* 0: top to bottom, 1: bottom to top */
{
    register BoxPtr	    pboxInit;
    register DDXPointPtr    pptInit;
    register BoxPtr	    pboxSorted;
    register DDXPointPtr    pptSorted;
    register int	    i, j, k, oldj, y1;

    /* Get pointers to existing data */
    pboxInit = *ppboxInit;
    pptInit  = *ppptInit;

    /* Create space for sorted data */
    pboxSorted = (BoxPtr)ALLOCATE_LOCAL(sizeof(BoxRec) * nbox);
    if (!pboxSorted)
	return FALSE;
    pptSorted = (DDXPointPtr)ALLOCATE_LOCAL(sizeof(DDXPointRec) * nbox);
    if (!pptSorted)
    {
	DEALLOCATE_LOCAL(pboxSorted);
	return FALSE;
    }
    *ppboxInit = pboxSorted;
    *ppptInit = pptSorted;

    if (ydir) {
        /* Walk source botttom to top */
	if (xdir) {
	    /* Bottom to top, and right to left.  Just reverse lists. */
	    for (i = 0; nbox != 0; /* increments in loop */) {
		nbox--;
		pboxSorted[i] = pboxInit[nbox];
		pptSorted [i] = pptInit [nbox];
		i++;
	    }
	} else {
	    /* Bottom to top, and left to right.  Reverse bands while keeping
	       order inside bands the same. */
	    for (i = 0, j = nbox-1; j >= 0; /* increments in loop */) {
		/* Find beginning of j band */
		y1 = pboxInit[j].y1;
		oldj = j;
		do {
		    j--;
		} while (j >= 0 && pboxInit[j].y1 == y1);
		/* Now copy the band */
		k = j;
		do {
		    k++;
		    pboxSorted[i] = pboxInit[k];
		    pptSorted[i] = pptInit[k];
		    i++;
		} while (k != oldj);
	    } /* end for */
	}
    } else { /* ydir == 0 */
	/* Top to bottom, but right to left.  (This routine isn't called if
	   top to bottom, left to right, so we don't have to worry about that.)
	   Reverse rectangle order inside bands, but maintain band order. */
	for (i = 0, j = 0; i != nbox; /* increments in loop */) {
	    /* Find end of j band */
	    y1 = pboxInit[i].y1;
	    oldj = j;
	    do {
		j++;
	    } while (j < nbox && pboxInit[j].y1 == y1);
	    /* Now reverse copy the band */
	    k = j;
	    do {
		k--;
		pboxSorted[i] = pboxInit[k];
		pptSorted[i] = pptInit[k];
		i++;
	    } while (k != oldj);
	} /* end for */
    } /* end if bottom to top else top to bottom */
    return TRUE;
} /* ffbSortRectsAndPoints */


/* ||| These numbers really need to be determined with some precision, or
determined dynamically.  Especially need to test mem->scr with TURBOchannel
block mode transfers turned on. */

#define FFBDMAREAD_WIDTH_MINIMUM	     50
#define FFBDMAREAD_AREA_MINIMUM		(100*100)

#define FFBDMAWRITE_WIDTH_MINIMUM	    10
#define FFBDMAWRITE_AREA_MINIMUM	(10*10)

typedef struct {
    int     widthMinimum;
    int     areaMinimum;
    char    *name;
} DMAInfo;

static DMAInfo dmaInfo[2] = {
    {FFBDMAREAD_WIDTH_MINIMUM,      FFBDMAREAD_AREA_MINIMUM, "DMA read"},
    {FFBDMAWRITE_WIDTH_MINIMUM,     FFBDMAWRITE_AREA_MINIMUM, "DMA write"}
};

#define DMANEVER    0
#define DMACOMPARE  1
#define DMAALWAYS   2

#ifdef VMS
int doDMA = DMANEVER;    /* set this to never for initial startup */
#else
/* Set this according to what you want to test */
int doDMA = DMACOMPARE;
#endif

Bool
ffbDoDMA(pMem, widthMem, pixelShift, pRegion, mode)
    Pixel8      *pMem;
    int		widthMem;
    int		pixelShift;
    RegionPtr   pRegion;
    FFBMode     mode;
{
    int		width, height;
    Bool	dmaWrite;

    /*
     * It pays to do dma if area to be copied is wide enough, and the total
     * area is large enough, and we can actually use lw_wire to lock down 
     * the pages.  (lw_wire should always work, but right now that isn't
     * the case.)
     */

/* ||| This should (1) separate into (a) DMAWRITE directly for large enough
GXcopy screen to memory, (b) DMAWRITE then copy (w/rasterops) for smaller
screen to memory or rasterop not GXcopy, (c) DMAREAD large enough; and (2)
become dynamic as different systems will behave differently due to memory
implementation, bus speed, etc.
*/

#if defined(mips) && !defined(SOFTWARE_MODEL)
    return FALSE;
#else
    dmaWrite = (mode == DMAWRITECOPY);
    if (doDMA == DMANEVER)
	return FALSE;
    width = (pRegion->extents.x2 - pRegion->extents.x1) << pixelShift;
    if (doDMA == DMACOMPARE && width < dmaInfo[dmaWrite].widthMinimum)
	return FALSE;

    height = pRegion->extents.y2 - pRegion->extents.y1;
    if (doDMA == DMACOMPARE && width * height < dmaInfo[dmaWrite].areaMinimum)
	return FALSE;

    /* Compute total bytes to lock, then try to lock down pages. */
    if (!ffbWirePages(pMem, (height-1) * widthMem + width))
	return FALSE;

/*
    ErrorF("Using %s on %d x %d rectangle\n", 
	    dmaInfo[dmaWrite].name, width, height);
*/
    return TRUE;
#endif
}

/*
 * HISTORY
 */
