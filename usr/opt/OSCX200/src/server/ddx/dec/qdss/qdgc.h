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
Copyright 1987, 1989 by Digital Equipment Corporation, Maynard, Massachusetts,
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

#include "mfb.h" /* for mfbPrivGC */

/*
 * Private fields of GC.
 * Note the mfbPrivGC is included first, for compatibility.
 * (Mfb routines are sometimes used for bitmaps).
 */
typedef struct {
    unsigned long	mask;		/* see values below, i.e.: QD_*	*/
    int			igreen;		/* index to green table */
    unsigned char *	ptresult;	/* pointer to lookup table */
#if 0
    RegionPtr		pAbsClientRegion; /* non-NULL if and only if
					   * clientClipType in the GC is
					   * CT_REGION */
    RegionPtr   	pCompositeClip;   /* always valid */
    int			freeCompClip;	  /* FREE_CC or REPLACE_CC */
#endif
    short		lastDest; /* any of the drawable types (-1,0,1) */
    unsigned long	GCstate;	/* added to pGC->stateChanges */
    short		dashLength;	/* effective length of dashes */
    short		dashPlane;	/* plane that dashes is in */
} QDPrivGCRec, *QDPrivGCPtr;

extern int mfbGCPrivateIndex, qdGCPrivateIndex;
#define QDGC_COMPOSITE_CLIP(pGC) \
	(((mfbPrivGCPtr)(pGC)->devPrivates[mfbGCPrivateIndex].ptr)->pCompositeClip)
#define GC_DASH_PLANE(pGC) ((QDPrivGCPtr)(pGC)->devPrivates[qdGCPrivateIndex].ptr)->dashPlane
#define GC_DASH_LENGTH(pGC) ((QDPrivGCPtr)(pGC)->devPrivates[qdGCPrivateIndex].ptr)->dashLength

/*
 * mask values:  (to be or-ed together)
 */
#define	QD_LOOKUP	(1L)
#define	QD_SRCBIT	(1L<<1)
#define	QD_DSTBYTE	(1L<<2)
#define	QD_NEWLOGIC	(1L<<3)

/*
 * GCstate values:	(to be or-ed together)
 */
#define	VSFullResetBit	0
#define	VSFullReset	(1L<<VSFullResetBit)
#define	VSDestBit	1		/* never set, but looked-up */
#define	VSDest		(1L<<VSDestBit)
#define	VSNone		0
#define	VSNewBits	2

/* Used to ensure that the location of a Pixmap is consistent with the GC. */
#define CHECK_MOVED(pGC, pDraw) \
   {if ((pGC)->lastWinOrg.x != (pDraw)->x \
     || (pGC)->lastWinOrg.y != (pDraw)->y) \
      qdClipMoved(pGC, (pDraw)->x, (pDraw)->y);}

extern struct _GC *InstalledGC;

#define INSTALL_FILLSTYLE(pGC, pDraw) \
    { if (InstalledGC != pGC) InstallState(pGC, pDraw); }
