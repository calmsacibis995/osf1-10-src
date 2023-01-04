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
 * @(#)$RCSfile: ffb.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:04:44 $
 */
/*
 */

#ifndef FFB_H
#define FFB_H

/* this crap is here due to dependencies induced through ffbpixmap.h */
#include "X.h"
#include "Xmd.h"
#include "Xproto.h"
#include "misc.h"
#include "windowstr.h"
/* end crap */

#include "cfb.h"
#ifdef MITR5
#include "cfbdec.h"
#endif	/* MITR5 */
 
#include "ffbparams.h"
#include "ffbregs.h"
#include "ffbcpu.h"
#include "ffbmap.h"
#include "ffbbuf.h"

/****************************************************************************
 *                         Private per-Screen Data                          *
 ***************************************************************************/

/* Private screen-specific information about ffb: we need a pointer to the
   device register so we can talk to the hardware, a pointer to the last GC
   that was loaded into the hardware so that we know when to load a new 
   hardware state, and information about the off-screen memory supported by
   the ffb so we can stash the pixmap in there.  If multiple screens supported
   by a single chip, then all such screens point to same private ffb record. */
   
typedef struct FreeElement_ {
        struct FreeElement_ *next;  /* Next FreeElement descriptor	    */
        Pixel8              *addr;  /* Address of free ffb block	    */
        int 		    size;   /* Size of block in bytes		    */
} FreeElement;

typedef struct {
    FFB             ffb;	    /* Pointer to ffb registers		    */
    GCPtr           lastGC;	    /* Pointer to last GC loaded into ffb   */
    pointer         firstScreenMem; /* First byte of screen memory	    */
    pointer         lastScreenMem;  /* Past last byte of screen memory      */
    FreeElement     avail;	    /* Free list for off-screen memory      */
    FreeElement     *rover;	    /* Roving pointer in free list	    */
    int		    fbdepth;	    /* actual depth of the frame buffer     */
    FFBInfo	    info;	    /* Shared information w/driver	    */
} ffbScreenPrivRec, *ffbScreenPrivPtr;

extern int ffbScreenPrivateIndex;

#define FFBSCREENPRIV(pScreen) \
    ((ffbScreenPrivPtr)(pScreen->devPrivates[ffbScreenPrivateIndex].ptr))

#define SCREENMEMORY(pPixmap)						    \
    (   FFBSCREENPRIV((pPixmap)->drawable.pScreen)->firstScreenMem <=       \
	    (pPixmap)->devPrivate.ptr					    \
     && (pPixmap)->devPrivate.ptr <					    \
	FFBSCREENPRIV((pPixmap)->drawable.pScreen)->lastScreenMem)

#define MAINMEMORY(pPixmap)   (!SCREENMEMORY(pPixmap))



/*****************************************************************************
 *                           Private per-GC Data			     *
 ****************************************************************************/

/* We need to know if last GC validation was for ffb or cfb.  If we
   cross-dress, we need to fill in EVERYTHING in the ops record from scratch;
   we can't make use of ANY of the existing procedures.  Note that this also
   means that both ffb and cfb need to fill in even those procedures that they
   never change, and so might never bother to set after GC initialization. */

typedef struct {
    Bool8           lastValidateWasFFB; /* Did we or cfb validate gc? */
    Bool8	    canBlock;		/* block mode okay */
    unsigned char   depthSpec;		/* visual and rotate from rop         */
    unsigned char   dashAdvance;        /* How much FFBLINEBITS advances      */
    unsigned int    dashLength;         /* Length (in bits) of dash pattern   */
    Bits32          dashPattern;        /* Expanded dash bit pattern	      */
    unsigned int    planemask;		/* holds our derived planemask        */
    void            (* FillArea) ();    /* FillArea routine                   */
} ffbGCPrivRec, *ffbGCPrivPtr;

extern int  ffbGCPrivateIndex;

#define FFBGCPRIV(pGC) ((ffbGCPrivPtr)(pGC->devPrivates[ffbGCPrivateIndex].ptr))
#define FFBGCBLOCK(pGC)	(FFBGCPRIV(pGC)->canBlock == TRUE)

/* And since cfb.h uses a different naming scheme... */
#define CFBGCPRIV(pGC)  cfbGetGCPrivate(pGC)

#include "ffbmacros.h"


/****************************************************************************
 *                        Private per-Drawable Data                         *
 ***************************************************************************/

/*
 * Right now (X11R5) there are no pixmap privates.  We expect this to change
 * in X11R6, at which point this junk will change.
 */

typedef struct {
    PixmapRec	pixmap;
    ffbBufDesc	hdweState;
    WindowPtr	parentWin;	/* set when pixmap is a packed backbuf */
                                /* see ffbgc.c and drawlib */
} ffbPixmapRec;

int              ffbWindowPrivateIndex;

#define FFBBUFDESCRIPTOR(pdraw)	 ( 					       \
    (((DrawablePtr)(pdraw))->type == DRAWABLE_PIXMAP) ?			       \
    (ffbBufDPtr)(&((ffbPixmapRec *)(pdraw))->hdweState) :		       \
    (ffbBufDPtr)(((WindowPtr)(pdraw))->devPrivates[ffbWindowPrivateIndex].ptr) \
				  )
#define FFBPARENTWINDOW(ppix)  (((ffbPixmapRec *)(ppix))->parentWin)



#include "ffbstate.h"	/* all drawing files need this */



/****************************************************************************
 *                   Macros to Load Drawable Information                    *
 ***************************************************************************/

#define DrawableBaseAndWidthPlus(pDraw, pBase, width, windowCode, pixCode)  \
{									    \
    if (pDraw->type == DRAWABLE_WINDOW) {				    \
	windowCode;							    \
	pBase = (Pixel8 *)						    \
	    (((PixmapPtr)(pDraw->pScreen->devPrivate))->devPrivate.ptr);    \
	width = (int)(((PixmapPtr)(pDraw->pScreen->devPrivate))->devKind);  \
    } else {								    \
	pixCode;							    \
	pBase = (Pixel8 *)(((PixmapPtr)pDraw)->devPrivate.ptr);		    \
	width = (int)(((PixmapPtr)pDraw)->devKind);			    \
    }									    \
} /* DrawableBaseAndWidthPlus */

#define DrawableBaseAndWidth(pDraw, pBase, width) \
    DrawableBaseAndWidthPlus(pDraw, pBase, width, ;/* Nada */, ;/* Nada */)

#endif /* FFB_H */
/*
 * HISTORY
 */
