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
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
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
/*
Title:		RasterOps (TX) card XOR Fix Module
Author:		Edwin Goei
Created:	2 Aug 91

The TX card has the following hardware behavior:
    * reading 8-bit frame buffer (fb) pixel gives blue component of 24-bit fb
    * but writing 8-bit fb pixel writes into all (rgb) components of 24-bit fb

Problem: when window managers do rubber banding in the (8-bit) root
window, they use IncludeInferiors and GXxor or GXinvert.  This fails when
rubber banding lines go thru 24-bit windows, because 24-bit fb pixel
values are not restored properly when XORed an even number of times.

This module solves this problem by doing the XOR using 24-bit tfb drawing
code instead of 8-bit cfb drawing code.  This trick does not work for
all ops, however, because CopyArea and PutImage have source drawable
arguments that are in 8 bit-per-pixel format while the tfb drawing ops assume
32 bit-per-pixel format.  So these ops are disabled.

This module does not fix all XOR problems.  Here is a summary of the side
effects:
    1) window manager rubber banding (8-bit root) works over 24-bit windows
    2) CopyArea and PutImage into 8-bit root window break under certain
	conditions (ie. whenever XOR fix is activated) -- this is the tradeoff
	for getting #1
    3) XORing into an arbitrary 8-bit window with 24-bit children with
	IncludeInferiors is not affected by this fix at all

The XOR fix described above is activated under the following conditions
    (refer to the code below in case conditions were changed):
    1) pDrawable is the root window
    2) pGC subwindow mode is IncludeInferiors
    3) pGC alu is GXxor or GXinvert

This code is implemented using screen, func, and op wrappers.  This level of
wrappers can be called on top of xfb screen code directly or on top of the
xfbBankSwitch module so that it will work on all machines including 3max.

Depending on the particular GC, there are 3 basic states it can be in:
    1) no XorFix GC wrapping for 24-bit GCs
    2) "cheap" func-only wrapping for normal 8-bit GCs
    3) func and op wrapping for 8-bit GCs that match activating conditions

Note there are two sets of GC wrappers used in this code: cheap and regular,
corresponding to case 2 and 3 above.  This code is similar in structure to
mibstore.c.  See that file for more info.

Other notes:
I've noticed the following unexpected behavior: writing pixels into the
24-bit fb in an area where the select plane is set to 8-bit fb mode,
can cause unexpected colors to appear on the display -- colors that are
not in the 8-bit psuedocolor map!  This is because the hardware actually
has a 512 entry DirectColor (decomposed) map and the RGB components of
the first 256 entries are identical, to simulate a psuedocolor map.  The
select plane is used to select either the upper or lower half of the
hardware colormap.  Writing into the 8-bit fb duplicates RGB components
into the 24-bit fb.  The Pixel8To32() macro below duplicates RGB components
of the pixel value to match this hardware behavior.
*/

#include "X.h"
#include "gcstruct.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "windowstr.h"
#include "cfb.h"
#include "cfbmskbits.h"
#include "dixfontstr.h"

/* XorFix is activated under the following condition */
#define ActivateXorFix(pGC, pDraw) \
    ((pDraw)->type == DRAWABLE_WINDOW \
	&& ((WindowPtr) (pDraw))->parent == NULL \
	&& (pGC)->subWindowMode == IncludeInferiors \
	&& ((pGC)->alu == GXxor || (pGC)->alu == GXinvert))

/* Replaced with cfb's PFILL */
#define Pixel8To32(pixel)	    \
{				    \
    unsigned long tpixel_;          \
    tpixel_ = pixel & 0xff;	    \
    tpixel_ |= ((tpixel_) << 8);    \
    tpixel_ |= ((tpixel_) << 16);   \
    pixel = tpixel_;		    \
} /* Pixel8To32 */


/* Screen and GC privates for XOR Fix module: */
typedef struct {
    /* screen func wrappers */
    Bool	    (*CloseScreen)();
    Bool	    (*CreateGC)();
} xfbXFScreenRec, *xfbXFScreenPtr;

typedef struct {
    GCFuncs	*wrapFuncs;	    /* wrapped funcs */
    GCOps	*wrapOps;	    /* wrapped ops */
} xfbXFGCRec, *xfbXFGCPtr;


/* Screen stuff */
static int  xfbXFScreenIndex;
static unsigned long xfbXFGeneration = 0;

#define SCREEN_PROLOGUE(pScreen, field)\
  ((pScreen)->field = \
   ((xfbXFScreenPtr) \
    (pScreen)->devPrivates[xfbXFScreenIndex].ptr)->field)

#define SCREEN_EPILOGUE(pScreen, field, wrapper)\
    ((pScreen)->field = wrapper)


/* GC Stuff: */
static int  xfbXFGCIndex;

/* "cheap" GC funcs: */
/*
 * every GC in the server is initially wrapped with these
 * "cheap" functions.  This allocates no memory and is used
 * to discover GCs used by window managers for rubber banding
 */

static void xfbXFCheapValidateGC(), xfbXFCheapCopyGC(), xfbXFCheapDestroyGC();
static void xfbXFCheapChangeGC ();
static void xfbXFCheapChangeClip(),  xfbXFCheapDestroyClip();
static void xfbXFCheapCopyClip();

static GCFuncs xfbXFCheapGCFuncs = {
    xfbXFCheapValidateGC,
    xfbXFCheapChangeGC,
    xfbXFCheapCopyGC,
    xfbXFCheapDestroyGC,
    xfbXFCheapChangeClip,
    xfbXFCheapDestroyClip,
    xfbXFCheapCopyClip,
};

#define CHEAP_FUNC_PROLOGUE(pGC) \
    ((pGC)->funcs = (GCFuncs *) (pGC)->devPrivates[xfbXFGCIndex].ptr)

#define CHEAP_FUNC_EPILOGUE(pGC) \
    ((pGC)->funcs = &xfbXFCheapGCFuncs) 

/* Non-cheap GC funcs and ops: */

static void xfbXFValidateGC(),	xfbXFCopyGC(), xfbXFDestroyGC();
static void xfbXFChangeGC();
static void xfbXFChangeClip(), xfbXFDestroyClip(), xfbXFCopyClip();

static GCFuncs	xfbXFGCFuncs = {
    xfbXFValidateGC,
    xfbXFChangeGC,
    xfbXFCopyGC,
    xfbXFDestroyGC,
    xfbXFChangeClip,
    xfbXFDestroyClip,
    xfbXFCopyClip,
};

#define FUNC_PROLOGUE(pGC, pPriv) \
    ((pGC)->funcs = pPriv->wrapFuncs),\
    ((pGC)->ops = pPriv->wrapOps)

#define FUNC_EPILOGUE(pGC, pPriv) \
    ((pGC)->funcs = &xfbXFGCFuncs),\
    ((pGC)->ops = &xfbXFGCOps)


static void	    xfbXFFillSpans(), xfbXFSetSpans(), xfbXFPutImage();
static RegionPtr    xfbXFCopyArea(), xfbXFCopyPlane();
static void	    xfbXFPolyPoint(), xfbXFPolylines(), xfbXFPolySegment();
static void	    xfbXFPolyRectangle(), xfbXFPolyArc(), xfbXFFillPolygon();
static void	    xfbXFPolyFillRect(), xfbXFPolyFillArc();
static int	    xfbXFPolyText8(), xfbXFPolyText16();
static void	    xfbXFImageText8(), xfbXFImageText16();
static void	    xfbXFImageGlyphBlt(), xfbXFPolyGlyphBlt();
static void	    xfbXFPushPixels(), xfbXFLineHelper();

static GCOps xfbXFGCOps = {
    xfbXFFillSpans,	xfbXFSetSpans,	    xfbXFPutImage,	
    xfbXFCopyArea,	xfbXFCopyPlane,	    xfbXFPolyPoint,
    xfbXFPolylines,	xfbXFPolySegment,    xfbXFPolyRectangle,
    xfbXFPolyArc,	xfbXFFillPolygon,    xfbXFPolyFillRect,
    xfbXFPolyFillArc,	xfbXFPolyText8,	    xfbXFPolyText16,
    xfbXFImageText8,	xfbXFImageText16,    xfbXFImageGlyphBlt,
    xfbXFPolyGlyphBlt,	xfbXFPushPixels,    xfbXFLineHelper,
};

#define OP_INIT(pGC) \
    xfbXFGCPtr pPriv = (xfbXFGCPtr) (pGC)->devPrivates[xfbXFGCIndex].ptr; \
    GCFuncs *oldFuncs = (pGC)->funcs

#define OP_PROLOGUE(pGC) \
    (pGC)->ops = pPriv->wrapOps; \
    (pGC)->funcs = pPriv->wrapFuncs

#define OP_EPILOGUE(pGC) \
    pPriv->wrapOps = (pGC)->ops; \
    (pGC)->ops = &xfbXFGCOps; \
    (pGC)->funcs = oldFuncs

/* Screen procs: */

static Bool
xfbXFCloseScreen(i, pScreen)
    ScreenPtr	pScreen;
{
    xfbXFScreenPtr   pScreenPriv;

    pScreenPriv = (xfbXFScreenPtr)
	pScreen->devPrivates[xfbXFScreenIndex].ptr;

    pScreen->CloseScreen = pScreenPriv->CloseScreen;
    pScreen->CreateGC = pScreenPriv->CreateGC;

    xfree((pointer) pScreenPriv);
    return (*pScreen->CloseScreen)(i, pScreen);
}

static Bool
xfbXFCreateGC(pGC)
    GCPtr   pGC;
{
    ScreenPtr	pScreen = pGC->pScreen;
    Bool	ret;

    SCREEN_PROLOGUE(pScreen, CreateGC);
    
    if ((ret = (*pScreen->CreateGC)(pGC)) && pGC->depth == 8) {
	/* wrap only funcs, no ops */
	pGC->devPrivates[xfbXFGCIndex].ptr = (pointer) pGC->funcs;
	pGC->funcs = &xfbXFCheapGCFuncs;
    }

    SCREEN_EPILOGUE(pScreen, CreateGC, xfbXFCreateGC);
    return (ret);
}


/* "Cheap" GC funcs: */

static void
xfbXFCheapValidateGC(pGC, changes, pDraw)
    GCPtr	    pGC;
    unsigned long   changes;
    DrawablePtr	    pDraw;
{
    xfbXFGCRec	*pPriv;
    Bool (*saveProc)();

    /* if we get here, pGC must be a depth 8 cfb GC */
    if (ActivateXorFix(pGC, pDraw)
	&& (pPriv = (xfbXFGCRec *) xalloc(sizeof(xfbXFGCRec)))) {

	/* turn this into a cfb32 gc: */
	xfbXFCheapDestroyGC(pGC);

	pGC->depth = 24;  /* fake out CreateGC() */

	/* save CreateGC proc because xfbXFCreateGC() will clobber it */
	saveProc = pGC->pScreen->CreateGC;
	/* we can't do anything about it if CreateGC fails so ignore ret */
	(void) xfbXFCreateGC(pGC);
	pGC->pScreen->CreateGC = saveProc;

	pGC->devPrivates[xfbXFGCIndex].ptr = (pointer) pPriv;

	/* wrap both funcs and ops */
	pPriv->wrapFuncs = pGC->funcs;
	pPriv->wrapOps = pGC->ops;
	pGC->funcs = &xfbXFGCFuncs;
	pGC->ops = &xfbXFGCOps;

	pGC->depth = 8;  /* depth must be 8 or else get BadMatch error */
	changes = ~0L;

	/* this will call xfbXFValidateGC() */
	(*pGC->funcs->ValidateGC)(pGC, changes, pDraw);
    } else {
	CHEAP_FUNC_PROLOGUE(pGC);

	(*pGC->funcs->ValidateGC)(pGC, changes, pDraw);

	/* rewrap funcs as Validate may have changed them */
	pGC->devPrivates[xfbXFGCIndex].ptr = (pointer) pGC->funcs;

	CHEAP_FUNC_EPILOGUE(pGC);
    }
}

static void
xfbXFCheapChangeGC (pGC, mask)
    GCPtr   pGC;
    unsigned long   mask;
{
    CHEAP_FUNC_PROLOGUE(pGC);

    (*pGC->funcs->ChangeGC)(pGC, mask);

    CHEAP_FUNC_EPILOGUE(pGC);
}

static void
xfbXFCheapCopyGC (pGCSrc, mask, pGCDst)
    GCPtr   pGCSrc, pGCDst;
    unsigned long   mask;
{
    CHEAP_FUNC_PROLOGUE(pGCDst);

    (*pGCDst->funcs->CopyGC)(pGCSrc, mask, pGCDst);

    CHEAP_FUNC_EPILOGUE(pGCDst);
}

static void
xfbXFCheapDestroyGC (pGC)
    GCPtr   pGC;
{
    CHEAP_FUNC_PROLOGUE(pGC);

    (*pGC->funcs->DestroyGC)(pGC);

    /* leave it unwrapped */
}

static void
xfbXFCheapChangeClip (pGC, type, pvalue, nrects)
    GCPtr   pGC;
    int		type;
    pointer	pvalue;
    int		nrects;
{
    CHEAP_FUNC_PROLOGUE(pGC);

    (*pGC->funcs->ChangeClip)(pGC, type, pvalue, nrects);

    CHEAP_FUNC_EPILOGUE(pGC);
}

static void
xfbXFCheapCopyClip(pgcDst, pgcSrc)
    GCPtr pgcDst, pgcSrc;
{
    CHEAP_FUNC_PROLOGUE(pgcDst);

    (*pgcDst->funcs->CopyClip)(pgcDst, pgcSrc);

    CHEAP_FUNC_EPILOGUE(pgcDst);
}

static void
xfbXFCheapDestroyClip(pGC)
    GCPtr	pGC;
{
    CHEAP_FUNC_PROLOGUE(pGC);

    (*pGC->funcs->DestroyClip)(pGC);

    CHEAP_FUNC_EPILOGUE(pGC);
}


/* Regular (non-cheap) GC Funcs: */

static void
xfbXFValidateGC(pGC, changes, pDraw)
    GCPtr   	  pGC;
    unsigned long changes;
    DrawablePtr   pDraw;
{
    xfbXFGCPtr	pPriv = (xfbXFGCPtr) pGC->devPrivates[xfbXFGCIndex].ptr;
    Bool (*saveProc)();

    /* if we get here, pGC must be a tfb GC */
    if (ActivateXorFix(pGC, pDraw)) {
	/* duplicate fg, bg, and pm into red green blue components */
	PFILL(pGC->fgPixel);  /* comment at beginning explains this */
	PFILL(pGC->bgPixel);
	PFILL(pGC->planemask);

	FUNC_PROLOGUE(pGC, pPriv);
	pGC->funcs->ValidateGC(pGC, changes, pDraw);
	/* rewrap funcs and ops as Validate may have changed them */
	pPriv->wrapFuncs = pGC->funcs;
	pPriv->wrapOps = pGC->ops;
	FUNC_EPILOGUE(pGC, pPriv);
    } else {
	/* turn this back into a cfb GC: */
	xfbXFDestroyGC(pGC);  /* frees XorFix GC private */

	/* save CreateGC proc because xfbXFCreateGC() will clobber it */
	saveProc = pGC->pScreen->CreateGC;
	/* we can't do anything about it if CreateGC fails so ignore ret */
	(void) xfbXFCreateGC(pGC);  /* also wraps funcs */
	pGC->pScreen->CreateGC = saveProc;

	changes = ~0L;

	/* this will call xfbXFCheapValidateGC() */
	pGC->funcs->ValidateGC(pGC, changes, pDraw);
    }
}

static void
xfbXFChangeGC (pGC, mask)
    GCPtr   pGC;
    unsigned long   mask;
{
    xfbXFGCPtr	pPriv = (xfbXFGCPtr) (pGC)->devPrivates[xfbXFGCIndex].ptr;

    FUNC_PROLOGUE(pGC, pPriv);

    (*pGC->funcs->ChangeGC)(pGC, mask);

    FUNC_EPILOGUE(pGC, pPriv);
}

static void
xfbXFCopyGC (pGCSrc, mask, pGCDst)
    GCPtr   pGCSrc, pGCDst;
    unsigned long   mask;
{
    xfbXFGCPtr	pPriv = (xfbXFGCPtr) (pGCDst)->devPrivates[xfbXFGCIndex].ptr;

    FUNC_PROLOGUE(pGCDst, pPriv);

    (*pGCDst->funcs->CopyGC)(pGCSrc, mask, pGCDst);

    FUNC_EPILOGUE(pGCDst, pPriv);
}

static void
xfbXFDestroyGC (pGC)
    GCPtr   pGC;
{
    xfbXFGCPtr	pPriv = (xfbXFGCPtr) (pGC)->devPrivates[xfbXFGCIndex].ptr;

    FUNC_PROLOGUE(pGC, pPriv);

    (*pGC->funcs->DestroyGC)(pGC);

    FUNC_EPILOGUE(pGC, pPriv);

    xfree(pPriv);
}

static void
xfbXFChangeClip(pGC, type, pvalue, nrects)
    GCPtr	pGC;
    int		type;
    pointer	pvalue;
    int		nrects;
{
    xfbXFGCPtr	pPriv = (xfbXFGCPtr) (pGC)->devPrivates[xfbXFGCIndex].ptr;

    FUNC_PROLOGUE(pGC, pPriv);

    (*pGC->funcs->ChangeClip)(pGC, type, pvalue, nrects);

    FUNC_EPILOGUE(pGC, pPriv);
}

static void
xfbXFCopyClip(pgcDst, pgcSrc)
    GCPtr pgcDst, pgcSrc;
{
    xfbXFGCPtr	pPriv = (xfbXFGCPtr) (pgcDst)->devPrivates[xfbXFGCIndex].ptr;

    FUNC_PROLOGUE(pgcDst, pPriv);

    (*pgcDst->funcs->CopyClip)(pgcDst, pgcSrc);

    FUNC_EPILOGUE(pgcDst, pPriv);
}

static void
xfbXFDestroyClip(pGC)
    GCPtr	pGC;
{
    xfbXFGCPtr	pPriv = (xfbXFGCPtr) (pGC)->devPrivates[xfbXFGCIndex].ptr;

    FUNC_PROLOGUE(pGC, pPriv);

    (*pGC->funcs->DestroyClip)(pGC);

    FUNC_EPILOGUE(pGC, pPriv);
}


/* GC Ops */

static void
xfbXFFillSpans(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nInit;			/* number of spans to fill */
    DDXPointPtr pptInit;		/* pointer to list of start points */
    int		*pwidthInit;		/* pointer to list of n widths */
    int 	fSorted;
{
    OP_INIT(pGC);
    OP_PROLOGUE(pGC);
    (*pGC->ops->FillSpans)(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted);
    OP_EPILOGUE(pGC);
}

static void
xfbXFSetSpans(pDrawable, pGC, psrc, ppt, pwidth, nspans, fSorted)
    DrawablePtr		pDrawable;
    GCPtr		pGC;
    unsigned long	*psrc;
    register DDXPointPtr ppt;
    int			*pwidth;
    int			nspans;
    int			fSorted;
{
    OP_INIT(pGC);
    OP_PROLOGUE(pGC);
    (*pGC->ops->SetSpans)(pDrawable, pGC, psrc, ppt, pwidth, nspans, fSorted);
    OP_EPILOGUE(pGC);
}

/* Ignore PutImage */
static void
xfbXFPutImage(pDrawable, pGC, depth, x, y, w, h, leftPad, format, pBits)
    DrawablePtr	  pDrawable;
    GCPtr   	  pGC;
    int		  depth;
    int	    	  x;
    int	    	  y;
    int	    	  w;
    int	    	  h;
    int	    	  format;
    char    	  *pBits;
{
    OP_INIT(pGC);
    OP_PROLOGUE(pGC);
    OP_EPILOGUE(pGC);
    return;
}

/* Ignore CopyArea */
static RegionPtr
xfbXFCopyArea(pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty)
    DrawablePtr	  pSrc;
    DrawablePtr	  pDst;
    GCPtr   	  pGC;
    int	    	  srcx;
    int	    	  srcy;
    int	    	  w;
    int	    	  h;
    int	    	  dstx;
    int	    	  dsty;
{
    OP_INIT(pGC);
    OP_PROLOGUE(pGC);
    OP_EPILOGUE(pGC);
    return (NullRegion);
}

static RegionPtr
xfbXFCopyPlane(pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty, plane)
    DrawablePtr	  pSrc;
    DrawablePtr	  pDst;
    register GC   *pGC;
    int     	  srcx,
		  srcy;
    int     	  w,
		  h;
    int     	  dstx,
		  dsty;
    unsigned long  plane;
{
    RegionPtr prgn;
    OP_INIT(pGC);
    OP_PROLOGUE(pGC);
    prgn = (*pGC->ops->CopyPlane)(pSrc, pDst, pGC, srcx, srcy, w, h,
	dstx, dsty, plane);
    OP_EPILOGUE(pGC);
    return (prgn);
}

static void
xfbXFPolyPoint(pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		mode;		/* Origin or Previous */
    int		npt;
    xPoint 	*pptInit;
{
    OP_INIT(pGC);
    OP_PROLOGUE(pGC);
    (*pGC->ops->PolyPoint)(pDrawable, pGC, mode, npt, pptInit);
    OP_EPILOGUE(pGC);
}

static void
xfbXFPolylines(pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr	  pDrawable;
    GCPtr   	  pGC;
    int	    	  mode;
    int	    	  npt;
    DDXPointPtr	  pptInit;
{
    OP_INIT(pGC);
    OP_PROLOGUE(pGC);
    (*pGC->ops->Polylines)(pDrawable, pGC, mode, npt, pptInit);
    OP_EPILOGUE(pGC);
}

static void
xfbXFPolySegment(pDrawable, pGC, nseg, pSegs)
    DrawablePtr pDrawable;
    GCPtr 	pGC;
    int		nseg;
    xSegment	*pSegs;
{
    OP_INIT(pGC);
    OP_PROLOGUE(pGC);
    (*pGC->ops->PolySegment)(pDrawable, pGC, nseg, pSegs);
    OP_EPILOGUE(pGC);
}

static void
xfbXFPolyRectangle(pDrawable, pGC, nrects, pRects)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		nrects;
    xRectangle	*pRects;
{
    OP_INIT(pGC);
    OP_PROLOGUE(pGC);
    (*pGC->ops->PolyRectangle)(pDrawable, pGC, nrects, pRects);
    OP_EPILOGUE(pGC);
}

static void
xfbXFPolyArc(pDrawable, pGC, narcs, parcs)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    OP_INIT(pGC);
    OP_PROLOGUE(pGC);
    (*pGC->ops->PolyArc)(pDrawable, pGC, narcs, parcs);
    OP_EPILOGUE(pGC);
}

static void
xfbXFFillPolygon(pDrawable, pGC, shape, mode, count, pPts)
    DrawablePtr		pDrawable;
    register GCPtr	pGC;
    int			shape, mode;
    register int	count;
    DDXPointPtr		pPts;
{
    OP_INIT(pGC);
    OP_PROLOGUE(pGC);
    (*pGC->ops->FillPolygon)(pDrawable, pGC, shape, mode, count, pPts);
    OP_EPILOGUE(pGC);
}

static void
xfbXFPolyFillRect(pDrawable, pGC, nrectFill, prectInit)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		nrectFill; 	/* number of rectangles to fill */
    xRectangle	*prectInit;  	/* Pointer to first rectangle to fill */
{
    OP_INIT(pGC);
    OP_PROLOGUE(pGC);
    (*pGC->ops->PolyFillRect)(pDrawable, pGC, nrectFill, prectInit);
    OP_EPILOGUE(pGC);
}

static void
xfbXFPolyFillArc(pDrawable, pGC, narcs, parcs)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    OP_INIT(pGC);
    OP_PROLOGUE(pGC);
    (*pGC->ops->PolyFillArc)(pDrawable, pGC, narcs, parcs);
    OP_EPILOGUE(pGC);
}

static int
xfbXFPolyText8(pDrawable, pGC, x, y, count, chars)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		x, y;
    int 	count;
    char	*chars;
{
    int result;
    OP_INIT(pGC);
    OP_PROLOGUE(pGC);
    result = (*pGC->ops->PolyText8)(pDrawable, pGC, x, y, count, chars);
    OP_EPILOGUE(pGC);
    return (result);
}

static int
xfbXFPolyText16(pDrawable, pGC, x, y, count, chars)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		x, y;
    int		count;
    unsigned short *chars;
{
    int result;
    OP_INIT(pGC);
    OP_PROLOGUE(pGC);
    result = (*pGC->ops->PolyText16)(pDrawable, pGC, x, y, count, chars);
    OP_EPILOGUE(pGC);
    return (result);
}

static void
xfbXFImageText8(pDrawable, pGC, x, y, count, chars)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		x, y;
    int		count;
    char	*chars;
{
    OP_INIT(pGC);
    OP_PROLOGUE(pGC);
    (*pGC->ops->ImageText8)(pDrawable, pGC, x, y, count, chars);
    OP_EPILOGUE(pGC);
}

static void
xfbXFImageText16(pDrawable, pGC, x, y, count, chars)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		x, y;
    int		count;
    unsigned short *chars;
{
    OP_INIT(pGC);
    OP_PROLOGUE(pGC);
    (*pGC->ops->ImageText16)(pDrawable, pGC, x, y, count, chars);
    OP_EPILOGUE(pGC);
}

static void
xfbXFImageGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
    DrawablePtr pDrawable;
    GC 		*pGC;
    int 	x, y;
    unsigned int nglyph;
    CharInfoPtr *ppci;		/* array of character info */
    pointer 	pglyphBase;	/* start of array of glyphs */
{
    OP_INIT(pGC);
    OP_PROLOGUE(pGC);
    (*pGC->ops->ImageGlyphBlt)(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase);
    OP_EPILOGUE(pGC);
}

static void
xfbXFPolyGlyphBlt(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int 	x, y;
    unsigned int nglyph;
    CharInfoPtr *ppci;		/* array of character info */
    char 	*pglyphBase;	/* start of array of glyphs */
{
    OP_INIT(pGC);
    OP_PROLOGUE(pGC);
    (*pGC->ops->PolyGlyphBlt)(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase);
    OP_EPILOGUE(pGC);
}

static void
xfbXFPushPixels(pGC, pBitMap, pDst, w, h, x, y)
    GCPtr	pGC;
    PixmapPtr	pBitMap;
    DrawablePtr pDst;
    int		w, h, x, y;
{
    OP_INIT(pGC);
    OP_PROLOGUE(pGC);
    (*pGC->ops->PushPixels)(pGC, pBitMap, pDst, w, h, x, y);
    OP_EPILOGUE(pGC);
}

static void
xfbXFLineHelper()
{
    FatalError("xfbXFLineHelper called\n");
}


/* Init XOR Fix module, public entry point */

Bool
xfbXorFixInit(pScreen)
    ScreenPtr	pScreen;
{
    xfbXFScreenPtr    pScreenPriv;

    if (xfbXFGeneration != serverGeneration) {
	xfbXFScreenIndex = AllocateScreenPrivateIndex();
	if (xfbXFScreenIndex < 0)
	    return (FALSE);
	xfbXFGCIndex = AllocateGCPrivateIndex();
	xfbXFGeneration = serverGeneration;
    }
    if (!AllocateGCPrivate(pScreen, xfbXFGCIndex, 0))
	return (FALSE);
    pScreenPriv = (xfbXFScreenPtr) xalloc(sizeof(xfbXFScreenRec));
    if (!pScreenPriv)
	return (FALSE);

    pScreenPriv->CloseScreen = pScreen->CloseScreen;
    pScreenPriv->CreateGC = pScreen->CreateGC;

    pScreen->CloseScreen = xfbXFCloseScreen;
    pScreen->CreateGC = xfbXFCreateGC;

    pScreen->devPrivates[xfbXFScreenIndex].ptr = (pointer) pScreenPriv;
}
