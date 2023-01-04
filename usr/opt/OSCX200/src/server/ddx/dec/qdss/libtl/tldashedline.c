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
/***********************************************************
Copyright 1989 by Digital Equipment Corporation, Maynard, Massachusetts,
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

#define DASHED
#define POLYSEGMENT tlPolySegmentDashed
#define POLYLINES tlPolylinesDashed
/* compile dashed versions of routines */
#include "tlline.c"

static GCPtr (_DashMap_[NPLANES+1]) = {0};
#define DashMap (_DashMap_+1)
extern int Nplanes;

/* This is also used by tlBNitmap{Bichrome,Stipple} to
 * find some temporary space.
 */
int GetFreeDashPlane()
{
    int planeNum;
    for (planeNum = Nplanes; ; ) {
	if (--planeNum < 0) {
	    static PlaneToRemove = 0;
	    /* throw out a "random" plane */
	    planeNum = PlaneToRemove;
	    PlaneToRemove++;
	    if (PlaneToRemove >= Nplanes) PlaneToRemove = 0;
	    DashMap[planeNum] = 0;
	    break;
	}
	if (DashMap[planeNum] == 0)
	    break;
    }
    return planeNum;
}

/* return plane containing installed dash pattern */
int
InstallDashes(pGC)
     GCPtr pGC;
{
    register unsigned short *p;
    int planeNum;
    int length = GC_DASH_LENGTH(pGC);
    int nDashPairs;
    register int odd, x;
    int oddLength;
    unsigned char *pDash;
    int maxDashPerBlock;
    planeNum = GC_DASH_PLANE(pGC);
    if (DashMap[planeNum] == pGC)
	return 1<<planeNum;
    planeNum = GetFreeDashPlane();
    DashMap[planeNum] = pGC;
    GC_DASH_PLANE(pGC) = planeNum;

    if (pGC->numInDashList & 1) {
	oddLength = length;
        odd = 0;
    }
    else
	odd = -1;
    if (length > 1024) return 0; /* should never happen */

    SETTRANSLATEPOINT(0, 0);
    Need_dma(13);
    *p++ = JMPT_SET_MASKED_ALU;
    *p++ = 1<<planeNum;
    *p++ = LF_SOURCE | FULL_SRC_RESOLUTION;
    *p++ = JMPT_RESETCLIP;
    *p++ = JMPT_SETCOLOR;
    *p++ = 0;
    *p++ = JMPT_SOLIDSPAN;
    *p++ = 0;
    *p++ = DASH_Y;
    *p++ = length;    
    *p++ = JMPT_SETCOLOR;
    *p++ = 0xFFFF;
    *p++ = JMPT_SOLIDSPAN;
    Confirm_dma();
    x = - (pGC->dashOffset % length);
    pDash = (unsigned char *)pGC->dash;
    nDashPairs = pGC->numInDashList;
    maxDashPerBlock = MAXDMAWORDS/6;
    if (odd < 0) nDashPairs >>= 1;
    do {
	register int i = min(maxDashPerBlock, nDashPairs);
	nDashPairs -= i;
	Need_dma(6*i);
	while ( --i >= 0) {
	    int w = *pDash++;
	     /*
	      * If numInDashList is odd (i.e. odd >= 0), then protocol says
	      * to concatenate the dash list with itself. So the even dashes
	      * are drawn as normal. However, the odd dashes become the
	      * even dashes of the second (concatenated) half, which is
	      * oddLength beyond the actual position of x.
	      */
	     int xx = odd > 0 ? x + oddLength : x;
	     /*The dash pattern is logically rotated left by pGC->dashOffset.*/
	     /*So, each dash is written twice: First offset by -dashOffset...*/
	     *p++ = xx & 0x3FFF;
	     *p++ = DASH_Y;
	     *p++ = w;
	     /* ... then each dash is offset by length-dashOffset */
	     *p++ = xx + length;
	     *p++ = DASH_Y;
	     *p++ = w;
	     x += w;
	     if (odd < 0) x += *pDash++; /* skip odd dashes */
	     else odd = 1 - odd;
	 }
	 Confirm_dma();
     } while (nDashPairs > 0);
    return 1<<planeNum;
}
