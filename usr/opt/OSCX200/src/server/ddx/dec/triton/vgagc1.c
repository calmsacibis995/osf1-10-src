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
static char *rcsid = "@(#)$RCSfile: vgagc1.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/08/02 20:38:46 $";
#endif

#include "X.h"
#include "Xmd.h"
#include "Xproto.h"
#include "scrnintstr.h"
#include "dixfontstr.h"
#include "fontstruct.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "region.h"
#include "mi.h"
#include "mfb.h"
#include "vga.h"
#include "vgaprocs.h"

void vgaValidateGC();

extern void mfbChangeGC();
extern void mfbCopyGC();
extern void mfbDestroyGC();
extern void mfbChangeClip();
extern void mfbDestroyClip();
extern void mfbCopyClip();


static GCFuncs vgaFuncs = {
    vgaValidateGC,
    mfbChangeGC,     /***** MFB *****/
    mfbCopyGC,       /***** MFB *****/
    mfbDestroyGC,    /***** MFB *****/
    mfbChangeClip,   /***** MFB *****/
    mfbDestroyClip,  /***** MFB *****/
    mfbCopyClip,     /***** MFB *****/
};

/*
  This is emuneration taken to the extreme, but what the heck, it works
  and eliminates the need to ever allocate OP structures on the fly.
  This may become a pain to maintain if more specialization is done in
  the future, but this doesn't seem too likely right now.
*/
/* added, why are these not predefined? */
extern void miPolyPoint();
extern void miZeroDashLine();

/***** Window Solid Ops *****/

static GCOps	vgaWindowSolidThinSolidOps = {
  vgaSolidWindowFS,	/* FillSpans */
  (void (*)())NULL,	/* SetSpans */
  vgaPutImage,		/* PutImage */
  vgaCopyArea,		/* CopyArea */
  vgaCopyPlane,		/* CopyPlane */
  miPolyPoint,		/* PolyPoint */
  vgaLineSS,		/* Polylines */
  vgaSegmentSS,		/* PolySegment */
  miPolyRectangle,	/* PolyRectangle */
  miZeroPolyArc,	/* PolyArc */
  miFillPolygon,	/* FillPolygon */
  vgaPolyFillRectSolid,	/* PolyFillRect */
  miPolyFillArc,	/* PolyFillArc */
  miPolyText8,		/* PolyText8 */
  miPolyText16,		/* PolyText16 */
  miImageText8,		/* ImageText8 */
  miImageText16,	/* ImageText16 */
  vgaImageGlyphBlt,	/* ImageGlyphBlt */
  vgaPolyGlyphBlt,	/* PolyGlyphBlt */
  mfbPushPixels,	/* PushPixels */
  (void (*)())NULL,	/* LineHelper */
 /* { (pointer) 0 }	*/		/* devPrivate */
};

static GCOps	vgaWindowSolidThinDashOps = {
  vgaSolidWindowFS,	/* FillSpans */
  (void (*)())NULL,	/* SetSpans */
  vgaPutImage,		/* PutImage */
  vgaCopyArea,		/* CopyArea */
  vgaCopyPlane,		/* CopyPlane */
  miPolyPoint,		/* PolyPoint */
  vgaLineSD,		/* Polylines */
  vgaSegmentSD,		/* PolySegment */
  miPolyRectangle,	/* PolyRectangle */
  miZeroPolyArc,	/* PolyArc */
  miFillPolygon,	/* FillPolygon */
  vgaPolyFillRectSolid,	/* PolyFillRect */
  miPolyFillArc,	/* PolyFillArc */
  miPolyText8,		/* PolyText8 */
  miPolyText16,		/* PolyText16 */
  miImageText8,		/* ImageText8 */
  miImageText16,	/* ImageText16 */
  vgaImageGlyphBlt,	/* ImageGlyphBlt */
  vgaPolyGlyphBlt,	/* PolyGlyphBlt */
  mfbPushPixels,	/* PushPixels */
  (void (*)())NULL,	/* LineHelper */
/*  0	*/		/* devPrivate */
};

static GCOps	vgaWindowSolidWideSolidOps = {
  vgaSolidWindowFS,	/* FillSpans */
  (void (*)())NULL,	/* SetSpans */
  vgaPutImage,		/* PutImage */
  vgaCopyArea,		/* CopyArea */
  vgaCopyPlane,		/* CopyPlane */
  miPolyPoint,		/* PolyPoint */
  miWideLine,		/* Polylines */
  miPolySegment,	/* PolySegment */
  miPolyRectangle,	/* PolyRectangle */
  miPolyArc,		/* PolyArc */
  miFillPolygon,	/* FillPolygon */
  vgaPolyFillRectSolid,	/* PolyFillRect */
  miPolyFillArc,	/* PolyFillArc */
  miPolyText8,		/* PolyText8 */
  miPolyText16,		/* PolyText16 */
  miImageText8,		/* ImageText8 */
  miImageText16,	/* ImageText16 */
  vgaImageGlyphBlt,	/* ImageGlyphBlt */
  vgaPolyGlyphBlt,	/* PolyGlyphBlt */
  mfbPushPixels,	/* PushPixels */
  (void (*)())NULL,	/* LineHelper */
/*  0	*/ 		/* devPrivate */
};

static GCOps	vgaWindowSolidWideDashOps = {
  vgaSolidWindowFS,	/* FillSpans */
  (void (*)())NULL,	/* SetSpans */
  vgaPutImage,		/* PutImage */
  vgaCopyArea,		/* CopyArea */
  vgaCopyPlane,		/* CopyPlane */
  miPolyPoint,		/* PolyPoint */
  miWideDash,		/* Polylines */
  miPolySegment,	/* PolySegment */
  miPolyRectangle,	/* PolyRectangle */
  miPolyArc,		/* PolyArc */
  miFillPolygon,	/* FillPolygon */
  vgaPolyFillRectSolid,	/* PolyFillRect */
  miPolyFillArc,	/* PolyFillArc */
  miPolyText8,		/* PolyText8 */
  miPolyText16,		/* PolyText16 */
  miImageText8,		/* ImageText8 */
  miImageText16,	/* ImageText16 */
  vgaImageGlyphBlt,	/* ImageGlyphBlt */
  vgaPolyGlyphBlt,	/* PolyGlyphBlt */
  mfbPushPixels,	/* PushPixels */
  (void (*)())NULL,	/* LineHelper */
/*  0	*/		/* devPrivate */
};

/***** Window Tile Ops *****/

static GCOps	vgaWindowTileThinSolidOps = {
  vgaTileWindowFS,	/* FillSpans */
  (void (*)())NULL,	/* SetSpans */
  vgaPutImage,		/* PutImage */
  vgaCopyArea,		/* CopyArea */
  vgaCopyPlane,		/* CopyPlane */
  miPolyPoint,		/* PolyPoint */
  miZeroLine,		/* Polylines */
  miPolySegment,	/* PolySegment */
  miPolyRectangle,	/* PolyRectangle */
  miZeroPolyArc,	/* PolyArc */
  miFillPolygon,	/* FillPolygon */
  vgaPolyFillRectTile,	/* PolyFillRect */
  miPolyFillArc,	/* PolyFillArc */
  miPolyText8,		/* PolyText8 */
  miPolyText16,		/* PolyText16 */
  miImageText8,		/* ImageText8 */
  miImageText16,	/* ImageText16 */
  vgaImageGlyphBlt,	/* ImageGlyphBlt */
  miPolyGlyphBlt,	/* PolyGlyphBlt */
  mfbPushPixels,	/* PushPixels */
  (void (*)())NULL,	/* LineHelper */
/*  0	*/		/* devPrivate */
};

static GCOps	vgaWindowTileThinDashOps = {
  vgaTileWindowFS,	/* FillSpans */
  (void (*)())NULL,	/* SetSpans */
  vgaPutImage,		/* PutImage */
  vgaCopyArea,		/* CopyArea */
  vgaCopyPlane,		/* CopyPlane */
  miPolyPoint,		/* PolyPoint */
  miZeroDashLine,	/* Polylines */
  miPolySegment,	/* PolySegment */
  miPolyRectangle,	/* PolyRectangle */
  miZeroPolyArc,	/* PolyArc */
  miFillPolygon,	/* FillPolygon */
  vgaPolyFillRectTile,	/* PolyFillRect */
  miPolyFillArc,	/* PolyFillArc */
  miPolyText8,		/* PolyText8 */
  miPolyText16,		/* PolyText16 */
  miImageText8,		/* ImageText8 */
  miImageText16,	/* ImageText16 */
  vgaImageGlyphBlt,	/* ImageGlyphBlt */
  miPolyGlyphBlt,	/* PolyGlyphBlt */
  mfbPushPixels,	/* PushPixels */
  (void (*)())NULL,	/* LineHelper */
/*  0	*/		/* devPrivate */
};

static GCOps	vgaWindowTileWideSolidOps = {
  vgaTileWindowFS,	/* FillSpans */
  (void (*)())NULL,	/* SetSpans */
  vgaPutImage,		/* PutImage */
  vgaCopyArea,		/* CopyArea */
  vgaCopyPlane,		/* CopyPlane */
  miPolyPoint,		/* PolyPoint */
  miWideLine,		/* Polylines */
  miPolySegment,	/* PolySegment */
  miPolyRectangle,	/* PolyRectangle */
  miPolyArc,		/* PolyArc */
  miFillPolygon,	/* FillPolygon */
  vgaPolyFillRectTile,	/* PolyFillRect */
  miPolyFillArc,	/* PolyFillArc */
  miPolyText8,		/* PolyText8 */
  miPolyText16,		/* PolyText16 */
  miImageText8,		/* ImageText8 */
  miImageText16,	/* ImageText16 */
  vgaImageGlyphBlt,	/* ImageGlyphBlt */
  miPolyGlyphBlt,	/* PolyGlyphBlt */
  mfbPushPixels,	/* PushPixels */
  (void (*)())NULL,	/* LineHelper */
/*  0	*/		/* devPrivate */
};

static GCOps	vgaWindowTileWideDashOps = {
  vgaTileWindowFS,	/* FillSpans */
  (void (*)())NULL,	/* SetSpans */
  vgaPutImage,		/* PutImage */
  vgaCopyArea,		/* CopyArea */
  vgaCopyPlane,		/* CopyPlane */
  miPolyPoint,		/* PolyPoint */
  miWideDash,		/* Polylines */
  miPolySegment,	/* PolySegment */
  miPolyRectangle,	/* PolyRectangle */
  miPolyArc,		/* PolyArc */
  miFillPolygon,	/* FillPolygon */
  vgaPolyFillRectTile,	/* PolyFillRect */
  miPolyFillArc,	/* PolyFillArc */
  miPolyText8,		/* PolyText8 */
  miPolyText16,		/* PolyText16 */
  miImageText8,		/* ImageText8 */
  miImageText16,	/* ImageText16 */
  vgaImageGlyphBlt,	/* ImageGlyphBlt */
  miPolyGlyphBlt,	/* PolyGlyphBlt */
  mfbPushPixels,	/* PushPixels */
  (void (*)())NULL,	/* LineHelper */
/*  0	*/		/* devPrivate */
};


/***** Window Stipple Ops *****/

static GCOps	vgaWindowStippleThinSolidOps = {
  vgaStippleWindowFS,	/* FillSpans */
  (void (*)())NULL,	/* SetSpans */
  vgaPutImage,		/* PutImage */
  vgaCopyArea,		/* CopyArea */
  vgaCopyPlane,		/* CopyPlane */
  miPolyPoint,		/* PolyPoint */
  miZeroLine,		/* Polylines */
  miPolySegment,	/* PolySegment */
  miPolyRectangle,	/* PolyRectangle */
  miZeroPolyArc,	/* PolyArc */
  miFillPolygon,	/* FillPolygon */
  vgaPolyFillRectStipple, /* PolyFillRect */
  miPolyFillArc,	/* PolyFillArc */
  miPolyText8,		/* PolyText8 */
  miPolyText16,		/* PolyText16 */
  miImageText8,		/* ImageText8 */
  miImageText16,	/* ImageText16 */
  vgaImageGlyphBlt,	/* ImageGlyphBlt */
  miPolyGlyphBlt,	/* PolyGlyphBlt */
  mfbPushPixels,	/* PushPixels */
  (void (*)())NULL,	/* LineHelper */
/*  0	*/		/* devPrivate */
};

static GCOps	vgaWindowStippleThinDashOps = {
  vgaStippleWindowFS,	/* FillSpans */
  (void (*)())NULL,	/* SetSpans */
  vgaPutImage,		/* PutImage */
  vgaCopyArea,		/* CopyArea */
  vgaCopyPlane,		/* CopyPlane */
  miPolyPoint,		/* PolyPoint */
  miZeroDashLine,	/* Polylines */
  miPolySegment,	/* PolySegment */
  miPolyRectangle,	/* PolyRectangle */
  miZeroPolyArc,	/* PolyArc */
  miFillPolygon,	/* FillPolygon */
  vgaPolyFillRectStipple, /* PolyFillRect */
  miPolyFillArc,	/* PolyFillArc */
  miPolyText8,		/* PolyText8 */
  miPolyText16,		/* PolyText16 */
  miImageText8,		/* ImageText8 */
  miImageText16,	/* ImageText16 */
  vgaImageGlyphBlt,	/* ImageGlyphBlt */
  miPolyGlyphBlt,	/* PolyGlyphBlt */
  mfbPushPixels,	/* PushPixels */
  (void (*)())NULL,	/* LineHelper */
/*  0	*/		/* devPrivate */
};

static GCOps	vgaWindowStippleWideSolidOps = {
  vgaStippleWindowFS,	/* FillSpans */
  (void (*)())NULL,	/* SetSpans */
  vgaPutImage,		/* PutImage */
  vgaCopyArea,		/* CopyArea */
  vgaCopyPlane,		/* CopyPlane */
  miPolyPoint,		/* PolyPoint */
  miWideLine,		/* Polylines */
  miPolySegment,	/* PolySegment */
  miPolyRectangle,	/* PolyRectangle */
  miPolyArc,		/* PolyArc */
  miFillPolygon,	/* FillPolygon */
  vgaPolyFillRectStipple, /* PolyFillRect */
  miPolyFillArc,	/* PolyFillArc */
  miPolyText8,		/* PolyText8 */
  miPolyText16,		/* PolyText16 */
  miImageText8,		/* ImageText8 */
  miImageText16,	/* ImageText16 */
  vgaImageGlyphBlt,	/* ImageGlyphBlt */
  miPolyGlyphBlt,	/* PolyGlyphBlt */
  mfbPushPixels,	/* PushPixels */
  (void (*)())NULL,	/* LineHelper */
/*  0	*/		/* devPrivate */
};

static GCOps	vgaWindowStippleWideDashOps = {
  vgaStippleWindowFS,	/* FillSpans */
  (void (*)())NULL,	/* SetSpans */
  vgaPutImage,		/* PutImage */
  vgaCopyArea,		/* CopyArea */
  vgaCopyPlane,		/* CopyPlane */
  miPolyPoint,		/* PolyPoint */
  miWideDash,		/* Polylines */
  miPolySegment,	/* PolySegment */
  miPolyRectangle,	/* PolyRectangle */
  miPolyArc,		/* PolyArc */
  miFillPolygon,	/* FillPolygon */
  vgaPolyFillRectStipple, /* PolyFillRect */
  miPolyFillArc,	/* PolyFillArc */
  miPolyText8,		/* PolyText8 */
  miPolyText16,		/* PolyText16 */
  miImageText8,		/* ImageText8 */
  miImageText16,	/* ImageText16 */
  vgaImageGlyphBlt,	/* ImageGlyphBlt */
  miPolyGlyphBlt,	/* PolyGlyphBlt */
  mfbPushPixels,	/* PushPixels */
  (void (*)())NULL,	/* LineHelper */
/*  0	*/		/* devPrivate */
};




/***** Pixmap Solid Ops *****/

static GCOps	vgaPixmapSolidThinSolidOps = {
  vgaSolidPixmapFS,	/* FillSpans */
  (void (*)())NULL,	/* SetSpans */
  vgaPutImage,		/* PutImage */
  vgaCopyArea,		/* CopyArea */
  vgaCopyPlane,		/* CopyPlane */
  miPolyPoint,		/* PolyPoint */
  miZeroLine,		/* Polylines */
  miPolySegment,	/* PolySegment */
  miPolyRectangle,	/* PolyRectangle */
  miZeroPolyArc,	/* PolyArc */
  miFillPolygon,	/* FillPolygon */
  miPolyFillRect,	/* PolyFillRect */
  miPolyFillArc,	/* PolyFillArc */
  miPolyText8,		/* PolyText8 */
  miPolyText16,		/* PolyText16 */
  miImageText8,		/* ImageText8 */
  miImageText16,	/* ImageText16 */
  miImageGlyphBlt,	/* ImageGlyphBlt */
  miPolyGlyphBlt,	/* PolyGlyphBlt */
  mfbPushPixels,	/* PushPixels */
  (void (*)())NULL,	/* LineHelper */
/*  0	*/		/* devPrivate */
};

static GCOps	vgaPixmapSolidThinDashOps = {
  vgaSolidPixmapFS,	/* FillSpans */
  (void (*)())NULL,	/* SetSpans */
  vgaPutImage,		/* PutImage */
  vgaCopyArea,		/* CopyArea */
  vgaCopyPlane,		/* CopyPlane */
  miPolyPoint,		/* PolyPoint */
  miZeroDashLine,	/* Polylines */
  miPolySegment,	/* PolySegment */
  miPolyRectangle,	/* PolyRectangle */
  miZeroPolyArc,	/* PolyArc */
  miFillPolygon,	/* FillPolygon */
  miPolyFillRect,	/* PolyFillRect */
  miPolyFillArc,	/* PolyFillArc */
  miPolyText8,		/* PolyText8 */
  miPolyText16,		/* PolyText16 */
  miImageText8,		/* ImageText8 */
  miImageText16,	/* ImageText16 */
  miImageGlyphBlt,	/* ImageGlyphBlt */
  miPolyGlyphBlt,	/* PolyGlyphBlt */
  mfbPushPixels,	/* PushPixels */
  (void (*)())NULL,	/* LineHelper */
/*  0	*/		/* devPrivate */
};

static GCOps	vgaPixmapSolidWideSolidOps = {
  vgaSolidPixmapFS,	/* FillSpans */
  (void (*)())NULL,	/* SetSpans */
  vgaPutImage,		/* PutImage */
  vgaCopyArea,		/* CopyArea */
  vgaCopyPlane,		/* CopyPlane */
  miPolyPoint,		/* PolyPoint */
  miWideLine,		/* Polylines */
  miPolySegment,	/* PolySegment */
  miPolyRectangle,	/* PolyRectangle */
  miPolyArc,		/* PolyArc */
  miFillPolygon,	/* FillPolygon */
  miPolyFillRect,	/* PolyFillRect */
  miPolyFillArc,	/* PolyFillArc */
  miPolyText8,		/* PolyText8 */
  miPolyText16,		/* PolyText16 */
  miImageText8,		/* ImageText8 */
  miImageText16,	/* ImageText16 */
  miImageGlyphBlt,	/* ImageGlyphBlt */
  miPolyGlyphBlt,	/* PolyGlyphBlt */
  mfbPushPixels,	/* PushPixels */
  (void (*)())NULL,	/* LineHelper */
/*  0	*/		/* devPrivate */
};

static GCOps	vgaPixmapSolidWideDashOps = {
  vgaSolidPixmapFS,	/* FillSpans */
  (void (*)())NULL,	/* SetSpans */
  vgaPutImage,		/* PutImage */
  vgaCopyArea,		/* CopyArea */
  vgaCopyPlane,		/* CopyPlane */
  miPolyPoint,		/* PolyPoint */
  miWideDash,		/* Polylines */
  miPolySegment,	/* PolySegment */
  miPolyRectangle,	/* PolyRectangle */
  miPolyArc,		/* PolyArc */
  miFillPolygon,	/* FillPolygon */
  miPolyFillRect,	/* PolyFillRect */
  miPolyFillArc,	/* PolyFillArc */
  miPolyText8,		/* PolyText8 */
  miPolyText16,		/* PolyText16 */
  miImageText8,		/* ImageText8 */
  miImageText16,	/* ImageText16 */
  miImageGlyphBlt,	/* ImageGlyphBlt */
  miPolyGlyphBlt,	/* PolyGlyphBlt */
  mfbPushPixels,	/* PushPixels */
  (void (*)())NULL,	/* LineHelper */
/*  0	*/		/* devPrivate */
};

/***** Pixmap Tile Ops *****/

static GCOps	vgaPixmapTileThinSolidOps = {
  vgaTilePixmapFS,	/* FillSpans */
  (void (*)())NULL,	/* SetSpans */
  vgaPutImage,		/* PutImage */
  vgaCopyArea,		/* CopyArea */
  vgaCopyPlane,		/* CopyPlane */
  miPolyPoint,		/* PolyPoint */
  miZeroLine,		/* Polylines */
  miPolySegment,	/* PolySegment */
  miPolyRectangle,	/* PolyRectangle */
  miZeroPolyArc,	/* PolyArc */
  miFillPolygon,	/* FillPolygon */
  miPolyFillRect,	/* PolyFillRect */
  miPolyFillArc,	/* PolyFillArc */
  miPolyText8,		/* PolyText8 */
  miPolyText16,		/* PolyText16 */
  miImageText8,		/* ImageText8 */
  miImageText16,	/* ImageText16 */
  miImageGlyphBlt,	/* ImageGlyphBlt */
  miPolyGlyphBlt,	/* PolyGlyphBlt */
  mfbPushPixels,	/* PushPixels */
  (void (*)())NULL,	/* LineHelper */
/*  0	*/		/* devPrivate */
};

static GCOps	vgaPixmapTileThinDashOps = {
  vgaTilePixmapFS,	/* FillSpans */
  (void (*)())NULL,	/* SetSpans */
  vgaPutImage,		/* PutImage */
  vgaCopyArea,		/* CopyArea */
  vgaCopyPlane,		/* CopyPlane */
  miPolyPoint,		/* PolyPoint */
  miZeroDashLine,	/* Polylines */
  miPolySegment,	/* PolySegment */
  miPolyRectangle,	/* PolyRectangle */
  miZeroPolyArc,	/* PolyArc */
  miFillPolygon,	/* FillPolygon */
  miPolyFillRect,	/* PolyFillRect */
  miPolyFillArc,	/* PolyFillArc */
  miPolyText8,		/* PolyText8 */
  miPolyText16,		/* PolyText16 */
  miImageText8,		/* ImageText8 */
  miImageText16,	/* ImageText16 */
  miImageGlyphBlt,	/* ImageGlyphBlt */
  miPolyGlyphBlt,	/* PolyGlyphBlt */
  mfbPushPixels,	/* PushPixels */
  (void (*)())NULL,	/* LineHelper */
/*  0	*/		/* devPrivate */
};

static GCOps	vgaPixmapTileWideSolidOps = {
  vgaTilePixmapFS,	/* FillSpans */
  (void (*)())NULL,	/* SetSpans */
  vgaPutImage,		/* PutImage */
  vgaCopyArea,		/* CopyArea */
  vgaCopyPlane,		/* CopyPlane */
  miPolyPoint,		/* PolyPoint */
  miWideLine,		/* Polylines */
  miPolySegment,	/* PolySegment */
  miPolyRectangle,	/* PolyRectangle */
  miPolyArc,		/* PolyArc */
  miFillPolygon,	/* FillPolygon */
  miPolyFillRect,	/* PolyFillRect */
  miPolyFillArc,	/* PolyFillArc */
  miPolyText8,		/* PolyText8 */
  miPolyText16,		/* PolyText16 */
  miImageText8,		/* ImageText8 */
  miImageText16,	/* ImageText16 */
  miImageGlyphBlt,	/* ImageGlyphBlt */
  miPolyGlyphBlt,	/* PolyGlyphBlt */
  mfbPushPixels,	/* PushPixels */
  (void (*)())NULL,	/* LineHelper */
/*  0	*/		/* devPrivate */
};

static GCOps	vgaPixmapTileWideDashOps = {
  vgaTilePixmapFS,	/* FillSpans */
  (void (*)())NULL,	/* SetSpans */
  vgaPutImage,		/* PutImage */
  vgaCopyArea,		/* CopyArea */
  vgaCopyPlane,		/* CopyPlane */
  miPolyPoint,		/* PolyPoint */
  miWideDash,		/* Polylines */
  miPolySegment,	/* PolySegment */
  miPolyRectangle,	/* PolyRectangle */
  miPolyArc,		/* PolyArc */
  miFillPolygon,	/* FillPolygon */
  miPolyFillRect,	/* PolyFillRect */
  miPolyFillArc,	/* PolyFillArc */
  miPolyText8,		/* PolyText8 */
  miPolyText16,		/* PolyText16 */
  miImageText8,		/* ImageText8 */
  miImageText16,	/* ImageText16 */
  miImageGlyphBlt,	/* ImageGlyphBlt */
  miPolyGlyphBlt,	/* PolyGlyphBlt */
  mfbPushPixels,	/* PushPixels */
  (void (*)())NULL,	/* LineHelper */
/*  0	*/		/* devPrivate */
};


/***** Pixmap Stipple Ops *****/

static GCOps	vgaPixmapStippleThinSolidOps = {
  vgaStipplePixmapFS,	/* FillSpans */
  (void (*)())NULL,	/* SetSpans */
  vgaPutImage,		/* PutImage */
  vgaCopyArea,		/* CopyArea */
  vgaCopyPlane,		/* CopyPlane */
  miPolyPoint,		/* PolyPoint */
  miZeroLine,		/* Polylines */
  miPolySegment,	/* PolySegment */
  miPolyRectangle,	/* PolyRectangle */
  miZeroPolyArc,	/* PolyArc */
  miFillPolygon,	/* FillPolygon */
  miPolyFillRect,	/* PolyFillRect */
  miPolyFillArc,	/* PolyFillArc */
  miPolyText8,		/* PolyText8 */
  miPolyText16,		/* PolyText16 */
  miImageText8,		/* ImageText8 */
  miImageText16,	/* ImageText16 */
  miImageGlyphBlt,	/* ImageGlyphBlt */
  miPolyGlyphBlt,	/* PolyGlyphBlt */
  mfbPushPixels,	/* PushPixels */
  (void (*)())NULL,	/* LineHelper */
/*  0	*/		/* devPrivate */
};

static GCOps	vgaPixmapStippleThinDashOps = {
  vgaStipplePixmapFS,	/* FillSpans */
  (void (*)())NULL,	/* SetSpans */
  vgaPutImage,		/* PutImage */
  vgaCopyArea,		/* CopyArea */
  vgaCopyPlane,		/* CopyPlane */
  miPolyPoint,		/* PolyPoint */
  miZeroDashLine,	/* Polylines */
  miPolySegment,	/* PolySegment */
  miPolyRectangle,	/* PolyRectangle */
  miZeroPolyArc,	/* PolyArc */
  miFillPolygon,	/* FillPolygon */
  miPolyFillRect,	/* PolyFillRect */
  miPolyFillArc,	/* PolyFillArc */
  miPolyText8,		/* PolyText8 */
  miPolyText16,		/* PolyText16 */
  miImageText8,		/* ImageText8 */
  miImageText16,	/* ImageText16 */
  miImageGlyphBlt,	/* ImageGlyphBlt */
  miPolyGlyphBlt,	/* PolyGlyphBlt */
  mfbPushPixels,	/* PushPixels */
  (void (*)())NULL,	/* LineHelper */
/*  0	*/		/* devPrivate */
};

static GCOps	vgaPixmapStippleWideSolidOps = {
  vgaStipplePixmapFS,	/* FillSpans */
  (void (*)())NULL,	/* SetSpans */
  vgaPutImage,		/* PutImage */
  vgaCopyArea,		/* CopyArea */
  vgaCopyPlane,		/* CopyPlane */
  miPolyPoint,		/* PolyPoint */
  miWideLine,		/* Polylines */
  miPolySegment,	/* PolySegment */
  miPolyRectangle,	/* PolyRectangle */
  miPolyArc,		/* PolyArc */
  miFillPolygon,	/* FillPolygon */
  miPolyFillRect,	/* PolyFillRect */
  miPolyFillArc,	/* PolyFillArc */
  miPolyText8,		/* PolyText8 */
  miPolyText16,		/* PolyText16 */
  miImageText8,		/* ImageText8 */
  miImageText16,	/* ImageText16 */
  miImageGlyphBlt,	/* ImageGlyphBlt */
  miPolyGlyphBlt,	/* PolyGlyphBlt */
  mfbPushPixels,	/* PushPixels */
  (void (*)())NULL,	/* LineHelper */
/*  0	*/		/* devPrivate */
};

static GCOps	vgaPixmapStippleWideDashOps = {
  vgaStipplePixmapFS,	/* FillSpans */
  (void (*)())NULL,	/* SetSpans */
  vgaPutImage,		/* PutImage */
  vgaCopyArea,		/* CopyArea */
  vgaCopyPlane,		/* CopyPlane */
  miPolyPoint,		/* PolyPoint */
  miWideDash,		/* Polylines */
  miPolySegment,	/* PolySegment */
  miPolyRectangle,	/* PolyRectangle */
  miPolyArc,		/* PolyArc */
  miFillPolygon,	/* FillPolygon */
  miPolyFillRect,	/* PolyFillRect */
  miPolyFillArc,	/* PolyFillArc */
  miPolyText8,		/* PolyText8 */
  miPolyText16,		/* PolyText16 */
  miImageText8,		/* ImageText8 */
  miImageText16,	/* ImageText16 */
  miImageGlyphBlt,	/* ImageGlyphBlt */
  miPolyGlyphBlt,	/* PolyGlyphBlt */
  mfbPushPixels,	/* PushPixels */
  (void (*)())NULL,	/* LineHelper */
/*  0	*/		/* devPrivate */
};


static GCOps *vgaGCOps[] = {
  &vgaWindowSolidThinSolidOps,
  &vgaWindowSolidThinDashOps,
  &vgaWindowSolidWideSolidOps,
  &vgaWindowSolidWideDashOps,
  &vgaWindowTileThinSolidOps,
  &vgaWindowTileThinDashOps,
  &vgaWindowTileWideSolidOps,
  &vgaWindowTileWideDashOps,
  &vgaWindowStippleThinSolidOps,
  &vgaWindowStippleThinDashOps,
  &vgaWindowStippleWideSolidOps,
  &vgaWindowStippleWideDashOps,
  &vgaWindowStippleThinSolidOps,
  &vgaWindowStippleThinDashOps,
  &vgaWindowStippleWideSolidOps,
  &vgaWindowStippleWideDashOps,
  &vgaPixmapSolidThinSolidOps,
  &vgaPixmapSolidThinDashOps,
  &vgaPixmapSolidWideSolidOps,
  &vgaPixmapSolidWideDashOps,
  &vgaPixmapTileThinSolidOps,
  &vgaPixmapTileThinDashOps,
  &vgaPixmapTileWideSolidOps,
  &vgaPixmapTileWideDashOps,
  &vgaPixmapStippleThinSolidOps,
  &vgaPixmapStippleThinDashOps,
  &vgaPixmapStippleWideSolidOps,
  &vgaPixmapStippleWideDashOps,
  &vgaPixmapStippleThinSolidOps,
  &vgaPixmapStippleThinDashOps,
  &vgaPixmapStippleWideSolidOps,
  &vgaPixmapStippleWideDashOps
};


GCOps **colorGCOps = &vgaGCOps[0];


#if 0 /* Copied here for reference from mfb.h */
typedef struct {
    unsigned char	rop;		/* reduction of rasterop to 1 of 3 */
    unsigned char	ropOpStip;	/* rop for opaque stipple */
    unsigned char	ropFillArea;	/*  == alu, rop, or ropOpStip */
    unsigned	fExpose:1;		/* callexposure handling ? */
    unsigned	freeCompClip:1;
    PixmapPtr	pRotatedPixmap;		/* tile/stipple rotated to align */
    RegionPtr	pCompositeClip;		/* free this based on freeCompClip
					   flag rather than NULLness */
    void 	(* FillArea)();		/* fills regions; look at the code */
    } mfbPrivGC;
typedef mfbPrivGC	*mfbPrivGCPtr;
#endif

Bool
vgaCreateGC(pGC)
    GCPtr pGC;
{
    mfbPrivGC  *pPriv;

    switch (pGC->depth) {
    case 1:
	return (mfbCreateGC(pGC)); /***** MFB *****/
    case 4:
    case 8:
	break;
    default:
	return FALSE;
    }
    pGC->clientClip = NULL;
    pGC->clientClipType = CT_NONE;

    /*
     * some of the output primitives aren't really necessary, since they
     * will be filled in ValidateGC because of dix/CreateGC() setting all
     * the change bits.  Others are necessary because although they depend
     * on being a color frame buffer, they don't change 
     */

    pGC->ops = colorGCOps[0];
    pGC->funcs = &vgaFuncs;

    /* vga wants to translate before scan conversion */
    pGC->miTranslate = 1;

    pPriv = (mfbPrivGC *)(pGC->devPrivates[mfbGCPrivateIndex].ptr);
    pPriv->fExpose = TRUE;
    pPriv->freeCompClip = FALSE;
    pPriv->pRotatedPixmap = (PixmapPtr) NULL;
    return TRUE;
}


/* Clipping conventions
	if the drawable is a window
	    CT_REGION ==> pCompositeClip really is the composite
	    CT_other ==> pCompositeClip is the window clip region
	if the drawable is a pixmap
	    CT_REGION ==> pCompositeClip is the translated client region
		clipped to the pixmap boundary
	    CT_other ==> pCompositeClip is the pixmap bounding box
*/
void
vgaValidateGC(pGC, changes, pDrawable)
     register GCPtr pGC;
     Mask           changes;
     DrawablePtr    pDrawable;
{
  register mfbPrivGCPtr	devPriv;
  WindowPtr pWin;
  int OPindex;

  if (pDrawable->type == DRAWABLE_WINDOW) {
    pWin = (WindowPtr)pDrawable;
    OPindex = 0;
  }
  else {
    pWin = (WindowPtr)NULL;
    OPindex = (1 << 4);
  }

  if (pGC->fillStyle == FillTiled) {
    OPindex |= (1 << 2);
  }
  else if (pGC->fillStyle == FillStippled) {
    OPindex |= (2 << 2);
  }
  else if (pGC->fillStyle == FillOpaqueStippled) {
    OPindex |= (3 << 2);
  }

  if (pGC->lineWidth > 0) {
    OPindex |= (1 << 1);
  }

  if (pGC->lineStyle != LineSolid) {
    OPindex |= 1;
  }

  pGC->ops = colorGCOps[OPindex];

  devPriv = ((mfbPrivGCPtr) (pGC->devPrivates[mfbGCPrivateIndex].ptr));

  /*
    if the client clip is different or moved OR
    the subwindowMode has changed OR
    the window's clip has changed since the last validation
    we need to recompute the composite clip
  */
  if ((changes & (GCClipXOrigin|GCClipYOrigin|GCClipMask|GCSubwindowMode)) ||
      (pDrawable->serialNumber != (pGC->serialNumber & DRAWABLE_SERIAL_BITS))
      )
    {
      ScreenPtr pScreen = pGC->pScreen;
      
      if (pWin)
	{
	  Bool freeTmpClip, freeCompClip;
	  RegionPtr pregWin;		/* clip for this window, without
					   client clip */

	  if (pGC->subWindowMode == IncludeInferiors)
	    {
	      pregWin = NotClippedByChildren(pWin);
	      freeTmpClip = TRUE;
	    }
	  else
	    {
	      pregWin = &pWin->clipList;
	      freeTmpClip = FALSE;
	    }
	  freeCompClip = devPriv->freeCompClip;

	  /* if there is no client clip, we can get by with
	     just keeping the pointer we got, and remembering
	     whether or not should destroy (or maybe re-use)
	     it later.  this way, we avoid unnecessary copying
	     of regions.  (this wins especially if many clients clip
	     by children and have no client clip.)
	   */
	  if (pGC->clientClipType == CT_NONE)
	    {
	      if(freeCompClip) 
		(*pScreen->RegionDestroy) (devPriv->pCompositeClip);
	      devPriv->pCompositeClip = pregWin;
	      devPriv->freeCompClip = freeTmpClip;
	    }
	  else
	    {
	      /* we need one 'real' region to put into the composite
		 clip.
		      if pregWin and the current composite clip 
		 are real, we can get rid of one.
		      if the current composite clip is real and
		 pregWin isn't, intersect the client clip and
		 pregWin into the existing composite clip.
		      if pregWin is real and the current composite
		 clip isn't, intersect pregWin with the client clip
		 and replace the composite clip with it.
		      if neither is real, create a new region and
		 do the intersection into it.
	      */

	      (*pScreen->TranslateRegion)(pGC->clientClip,
					  pDrawable->x + pGC->clipOrg.x,
					  pDrawable->y + pGC->clipOrg.y);
						  
	      if (freeCompClip)
		{
		  (*pScreen->Intersect)(devPriv->pCompositeClip,
					pregWin, pGC->clientClip);
		  if (freeTmpClip)
		    (*pScreen->RegionDestroy)(pregWin);
		}
	      else if (freeTmpClip)
		{
		  (*pScreen->Intersect)(pregWin, pregWin, pGC->clientClip);
		  devPriv->pCompositeClip = pregWin;
		}
	      else
		{
		  devPriv->pCompositeClip = (*pScreen->RegionCreate)(NullBox,
								     0);
		  (*pScreen->Intersect)(devPriv->pCompositeClip,
					pregWin, pGC->clientClip);
		}
	      devPriv->freeCompClip = TRUE;
	      (*pScreen->TranslateRegion)(pGC->clientClip,
					  -(pDrawable->x + pGC->clipOrg.x),
					  -(pDrawable->y + pGC->clipOrg.y));
	      
	    }
	} /* end of composite clip for a window */
      else
	{
	  BoxRec pixbounds;

	  /* XXX should we translate by drawable.x/y here ? */
	  pixbounds.x1 = 0;
	  pixbounds.y1 = 0;
	  pixbounds.x2 = pDrawable->width;
	  pixbounds.y2 = pDrawable->height;
	  
	  if (devPriv->freeCompClip)
	    (*pScreen->RegionReset)(devPriv->pCompositeClip, &pixbounds);
	  else
	    {
	      devPriv->freeCompClip = TRUE;
	      devPriv->pCompositeClip = (*pScreen->RegionCreate)(&pixbounds,
								 1);
	    }
	  
	  if (pGC->clientClipType == CT_REGION)
	    {
	      (*pScreen->TranslateRegion)(devPriv->pCompositeClip,
					  -pGC->clipOrg.x, -pGC->clipOrg.y);
	      (*pScreen->Intersect)(devPriv->pCompositeClip,
				    devPriv->pCompositeClip,
				    pGC->clientClip);
	      (*pScreen->TranslateRegion)(devPriv->pCompositeClip,
					  pGC->clipOrg.x, pGC->clipOrg.y);
	    }
	} /* end of composite clip for pixmap */
    }
}




