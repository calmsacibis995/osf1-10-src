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
 * @(#)$RCSfile: vgaprocs.h,v $ $Revision: 1.1.4.5 $ (DEC) $Date: 1993/11/22 17:34:51 $
 */

#include "cursorstr.h"
#include "dixfontstr.h"
#include "windowstr.h"

/*----------- VGASEG.C -------------*/

extern void
vgaSegmentSS(
    struct _Drawable *pDrawable,
    struct _GC *pGC,
    int nseg,
    xSegment *pSeg );

extern void
vgaSegmentSD(
    struct _Drawable *pDrawable,
    struct _GC *pGC,
    int nseg,
    xSegment *pSeg );

/*----------- VGABITBL.C -------------*/

extern struct _Region *
vgaCopyArea(
    struct _Drawable *pSrcDrawable,
    struct _Drawable *pDstDrawable,
    struct _GC *pGC,
    int srcx,
    int srcy,
    int width,
    int height,
    int dstx,
    int dsty );

extern void
vgaDoBitBlt(
    struct _Drawable *pSrcDrawable,
    struct _Drawable *pDstDrawable,
    struct _GC *pGC,
    struct _Region *prgnDst,
    struct _DDXPoint *pptSrc );

extern void
vgaCopyPlane1to4(
    struct _Drawable *pSrcDrawable,
    struct _Drawable *pDstDrawable,
    struct _GC *pGC,
    struct _Region *prgnDst,
    struct _DDXPoint *pptSrc );

extern struct _Region *
vgaCopyPlane(
    struct _Drawable *pSrcDrawable,
    struct _Drawable *pDstDrawable,
    struct _GC *pGC,
    int srcx,
    int srcy,
    int width,
    int height,
    int dstx,
    int dsty,
    unsigned long bitPlane );

extern void
vgaPutImage(
    struct _Drawable *dst,
    struct _GC *pGC,
    int depth,
    int x,
    int y,
    int w,
    int h,
    int leftPad,
    unsigned int format,
    char *pImage );

extern void
vgaGetImage(
    struct _Drawable *pDrawable,
    int sx,
    int sy,
    int w,
    int h,
    unsigned int format,
    unsigned long planeMask,
    pointer pdstLine );

/*----------- VGABITBL.C -------------*/

extern void
vgaSaveAreas(
    struct _Pixmap *pPixmap,
    struct _Region *prgnSave,
    int xorg,
    int yorg );

extern void
vgaRestoreAreas(
     struct _Pixmap *pPixmap,
     struct _Region *prgnRestore,
     int xorg,
     int yorg );

/*----------- VGACMAP.C -------------*/

#if defined(XPROTO_H)

extern void
vgaStoreColors(
    struct _ColormapRec *pmap,
    int ndef,
    xColorItem *pdefs );

extern Bool
vgaCreateColormap(
    struct _ColormapRec *pmap );

extern void
vgaDestroyColormap(
     struct _ColormapRec *pmap );

extern int
vgaListInstalledColormaps(
    struct _Screen *pScreen,
    Colormap *pmaps );

extern void
vgaInstallColormap(
    struct _ColormapRec *pmap );

extern void
vgaRefreshInstalledColormap(
    void );

extern void
vgaUninstallColormap(
    struct _ColormapRec *pmap );

extern void
vgaResolveColor(
    unsigned short *pred,
    unsigned short *pgreen,
    unsigned short *pblue,
    struct _Visual *pVisual );

extern Bool
vgaCreateDefColormap(
    struct _Screen *pScreen );

#endif

/*----------- VGACURS.C -------------*/

extern Bool
colorRealizeCursor(
    struct _Screen *pScr,
    struct _Cursor *pCurs );

extern Bool
colorUnrealizeCursor(
    struct _Screen *pScr,
    struct _Cursor *pCurs );

extern Bool
colorDisplayCursor(
    struct _Screen *pScr,
    struct _Cursor *pCurs );

extern void
colorRecolorCursor(
    struct _Screen *pScr,
    struct _Cursor *pCurs,
    int displayed );

extern void
colorConstrainCursor(
    struct _Screen *pScr,
    struct _Box *pBox );

extern void
colorPointerNonInterestBox(
    struct _Screen *pScr,
    struct _Box *pBox );

extern void
colorCursorLimits(
    struct _Screen *pScr,
    struct _Cursor *pCurs,
    struct _Box *pHotBox,
    struct _Box *pTopLeftBox );

extern Bool colorSetCursorPosition(
    struct _Screen *pScr,
    int x,
    int y,
    Bool generateEvent );

extern void
colorHideCursorAlways(
    ScreenPtr pScreen );

extern void
colorHideCursorInXYWH(
    int x,
    int y,
    int w,
    int h );

#ifdef SOFTWARE_CURSOR
extern void
colorShowCursor(
    void );
#endif

extern void
colorHideCursorInSpans(
    struct _DDXPoint *ppt,
    int *pwidth,
    int count );

extern void
colorHideCursorInLine(
    int x1,
    int y1,
    int x2,
    int y2 );

extern void
vgaSetNewCursor(
    struct _Screen *pScr,
    struct _Cursor *pCurs );

unsigned long
#if defined(DWDOS286)
_loadds
#endif
colorMoveCursor(
    int delta_x,
    int delta_y );

extern void
colorCursorInit(
    struct _Screen *pScr );

extern void
vgaCursorInit(
    struct _Screen *pScr );

/*----------- VGAFILLR.C -------------*/

extern void
vgaPolyFillRectSolid(
    struct _Drawable *pDrawable,
    struct _GC *pGC,
    int nrectFill,
    xRectangle *prectInit );

extern void
vgaPolyFillRectStipple(
    struct _Drawable *pDrawable,
    struct _GC *pGC,
    int nrectFill,
    xRectangle *prectInit );

extern void
vgaPolyFillRectTile(
    struct _Drawable *pDrawable,
    struct _GC *pGC,
    int nrectFill,
    xRectangle *prectInit );

/*----------- VGAFONT.C -------------*/

extern Bool
vgaRealizeFont(
    struct _Screen *pScreen,
    struct _Font *pFont );

extern Bool
vgaUnrealizeFont(
    struct _Screen *pScreen,
    struct _Font *pFont );

/*----------- VGAGBLT.C -------------*/

extern void
vgaPolyGlyphBlt(
    struct _Drawable *pDrawable,
    struct _GC *pGC,
    int x,
    int y,
    unsigned int nglyph,
    struct _CharInfo **ppci,
    unsigned char *pglyphBase );

extern void
vgaImageGlyphBlt(
    struct _Drawable *pDrawable,
    struct _GC *pGC,
    int x,
    int y,
    unsigned int nglyph,
    struct _CharInfo **ppci,
    unsigned char *pglyphBase );

/*----------- VGAGC.C -------------*/

extern Bool
vgaCreateGC(
    struct _GC *pGC );

extern void
vgaValidateGC(
    struct _GC *pGC,
    Mask changes,
    struct _Drawable *pDrawable );

/*----------- VGAIO.C -------------*/

extern void
vgaInitOutput(
    ScreenInfo *screenInfo,
    int argc,
    char **argv );

/*----------- VGAINIT.C -------------*/

extern Bool
vgaCloseScreen(
    int index,
    struct _Screen *pScreen );

extern Bool
vgaSaveScreen(
    struct _Screen *pScreen,
    int on );

extern Bool
vgaScreenInit(
    int index,
    struct _Screen *pScreen,
    int argc,
    char **argv );

/*----------- VGALINE.C -------------*/

extern void
vgaLineSS(
    struct _Drawable *pDrawable,
    struct _GC *pGC,
    int mode,
    int npt,
    struct _DDXPoint *pptInit );

extern void
vgaLineSD(
    struct _Drawable *pDrawable,
    struct _GC *pGC,
    int mode,
    int npt,
    struct _DDXPoint *pptInit );

/*----------- VGAPIX.C -------------*/

extern struct _Pixmap *
vgaCreatePixmap(
    struct _Screen *pScreen,
    int width,
    int height,
    int depth );

extern Bool
vgaDestroyPixmap(
    struct _Pixmap *pPixmap );

/*----------- VGAPNTWI.C -------------*/

extern void
vgaPaintWindow(
    struct _Window *pWin,
    struct _Region *pRegion,
    int what );

/*----------- VGASPANS.C -------------*/

extern void
vgaSolidPixmapFS(
    struct _Drawable *pDrawable,
    struct _GC *pGC,
    int nInit,
    struct _DDXPoint *pptInit,
    int *pwidthInit,
    int fSorted );

extern void
vgaStipplePixmapFS(
    struct _Drawable *pDrawable,
    struct _GC *pGC,
    int nInit,
    struct _DDXPoint *pptInit,
    int *pwidthInit,
    int fSorted );

extern void
vgaTilePixmapFS(
    struct _Drawable *pDrawable,
    struct _GC *pGC,
    int nInit,
    struct _DDXPoint *pptInit,
    int *pwidthInit,
    int fSorted );

extern void
vgaSolidWindowFS(
    struct _Drawable *pDrawable,
    struct _GC *pGC,
    int nInit,
    struct _DDXPoint *pptInit,
    int *pwidthInit,
    int fSorted );

extern void
vgaStippleWindowFS(
    struct _Drawable *pDrawable,
    struct _GC *pGC,
    int nInit,
    struct _DDXPoint *pptInit,
    int *pwidthInit,
    int fSorted );

extern void
vgaTileWindowFS(
    struct _Drawable *pDrawable,
    struct _GC *pGC,
    int nInit,
    struct _DDXPoint *pptInit,
    int *pwidthInit,
    int fSorted );

extern void
vgaGetSpans(
    struct _Drawable *pDrawable,
    int wMax,
    struct _DDXPoint *ppt,
    int *pwidth,
    int nspans,
    unsigned char *pdstStart );

/*----------- VGAWIND.C -------------*/

extern Bool
vgaCreateWindow(
    struct _Window *pWin );

extern Bool
vgaDestroyWindow(
    struct _Window *pWin );

extern Bool
vgaPostionWindow(
    struct _Window *pWin,
    int x,
    int y );

extern Bool
vgaChangeWindowAttributes(
    struct _Window *pWin,
    unsigned long mask );

extern Bool
vgaMapWindow(
    struct _Window *pWin );

extern Bool
vgaUnmapWindow(
    struct _Window *pWin );


extern void
vgaCopyWindow(
    struct _Window *pWin,
    DDXPointRec ptOldOrg,
    struct _Region *prgnSrc );

/*----------- VGABLIT.ASM -------------*/

extern void
vgaBlit(
    int xSrc,
    int ySrc,
    int wSrc,
    int hSrc,
    int xDst,
    int yDst,
    unsigned int alu,
    unsigned int planemask );

/*----------- VGABRES.ASM -------------*/

void
vgaBresS(
    unsigned int fg,
    unsigned int alu,
    unsigned int planemask,
    int signdx,
    int signdy,
    int axis,
    int x1,
    int y1,
    int e,
    int e1,
    int e2,
    int len );

/*----------- VGABRESD.ASM -------------*/

void
vgaBresOnOff(
    unsigned int fg,
    unsigned int alu,
    unsigned int planemask,
    int *pdashIndex,
    unsigned char *pDash,
    int numInDashList,
    int *pdashOffset,
    int signdx,
    int signdy,
    int axis,
    int x1,
    int y1,
    int e,
    int e1,
    int e2,
    int len );

void
vgaBresDouble(
    unsigned int fg,
    unsigned int bg,
    unsigned int alu,
    unsigned int planemask,
    int *pdashIndex,
    unsigned char *pDash,
    int numInDashList,
    int *pdashOffset,
    int signdx,
    int signdy,
    int axis,
    int x1,
    int y1,
    int e,
    int e1,
    int e2,
    int len );

/*----------- VGADCURS.ASM -------------*/

extern void
vgaDrawCursor(
    int x,
    int y );

extern void
vgaUndrawCursor(
    void );

/*----------- VGAIMAGE.ASM -------------*/

extern void
vgaDrawColorImage(
    int xDst,
    int yDst,
    int wDst,
    int hDst,
    unsigned char *pSrc,
    int widthSrc,
    unsigned int alu,
    unsigned int planemask );

extern void
vgaReadColorImage(
    int xSrc,
    int ySrc,
    int wSrc,
    int hSrc,
    unsigned char *pDst,
    int widthDst );

/*----------- VGAREPL.ASM -------------*/

extern void
vgaReplicateScans(
    int xSrc,
    int ySrc,
    int wSrc,
    int hSrc,
    int hDst,
    unsigned int planemask );

/*----------- VGASETUP.ASM -------------*/

extern int
vgaCheck(
    void );

extern void
vgaSetup(
    void );

extern int
egaVideoMemory(
    void );

extern void
vgaSetPalette(
    unsigned int pixel,
    unsigned short red,
    unsigned short green,
    unsigned short blue );

extern void
egaSetPalette(
    unsigned int pixel,
    unsigned short red,
    unsigned short green,
    unsigned short blue );

/*----------- VGASOLID.ASM -------------*/

extern void
vgaFillSolid(
    unsigned int fg,
    unsigned int alu,
    unsigned int planemask,
    int xDst,
    int yDst,
    int wDst,
    int hDst );

/*----------- VGASTIPL.ASM -------------*/

extern void
vgaOpaqueStipple(
    int xSrc,
    int ySrc,
    int wSrc,
    int hSrc,
    unsigned char *psrcBase,
    int widthSrc,
    int xDst,
    int yDst,
    unsigned int alu,
    unsigned int planemask,
    unsigned int fg,
    unsigned int bg );

void
vgaStipple(
    int xSrc,
    int ySrc,
    int wSrc,
    int hSrc,
    unsigned char *psrcBase,
    int widthSrc,
    int xDst,
    int yDst,
    unsigned int alu,
    unsigned int planemask,
    unsigned int fg );

/*-------------------------------------*/

#if defined (DWDOS286)
#include "LINTGLOB.H"
#endif

