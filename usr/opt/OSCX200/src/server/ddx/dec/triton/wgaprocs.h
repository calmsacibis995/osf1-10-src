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
 * @(#)$RCSfile: wgaprocs.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/22 17:36:04 $
 */

#include "cursorstr.h"
#include "dixfontstr.h"
#include "windowstr.h"

/*----------- WGACURS.C -------------*/

extern void
pwgaSetNewCursor(
    struct _Screen *pScr,
    struct _Cursor *pCurs );

extern void
pwgaDrawCursor(
    int x,
    int y );

extern void
pwgaUndrawCursor(
    void );

extern void
pwgaCursorInit(
    struct _Screen *pScr );

/*----------- WGAINIT.C -------------*/

extern Bool
pwgaScreenInit(
    int index,
    struct _Screen *pScreen,
    int argc,
    char **argv );

/*----------- WGAPROCS.C -------------*/

extern void
pwgaBlit(
    ScreenPtr pScreen,
    int xSrc,
    int ySrc,
    int wSrc,
    int hSrc,
    int xDst,
    int yDst,
    unsigned int alu,
    unsigned int planemask );

extern void
pwgaReplicateScans(
    ScreenPtr pScreen,
    int xSrc,
    int ySrc,
    int wSrc,
    int hSrc,
    int hDst,
    unsigned int planemask );

extern void
pwgaFillSolid(
    ScreenPtr pScreen,
    unsigned int fg,
    unsigned int alu,
    unsigned int planemask,
    int xDst,
    int yDst,
    int wDst,
    int hDst,
    int first );

extern void
pwgaDrawColorImage(
    ScreenPtr pScreen,
    int xDst,
    int yDst,
    int wDst,
    int hDst,
    unsigned char *pSrc,
    int widthSrc,
    unsigned int alu,
    unsigned int planemask,
    int first);

extern void
pwgaReadColorImage(
    ScreenPtr pScreen,
    int xSrc,
    int ySrc,
    int wSrc,
    int hSrc,
    unsigned long planemask,
    unsigned char *pDst,
    int widthDst );

extern void
pwgaOpaqueStipple(
    ScreenPtr pScreen,
    int xSrc,
    int ySrc,
    int wSrc,
    int hSrc,
    unsigned char *psrcBase,
    int widthSrc,
    int xDst,
    int yDst,
    int first,
    unsigned int alu,
    unsigned int planemask,
    unsigned int fg,
    unsigned int bg );

extern void
pwgaStipple(
    ScreenPtr pScreen,
    int xSrc,
    int ySrc,
    int wSrc,
    int hSrc,
    unsigned char *psrcBase,
    int widthSrc,
    int xDst,
    int yDst,
    int first,
    unsigned int alu,
    unsigned int planemask,
    unsigned int fg );

extern void
pwgaBresS(
    ScreenPtr pScreen,
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

extern void
pwgaBresOnOff(
    ScreenPtr pScreen,
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

extern void
pwgaBresDouble(
    ScreenPtr pScreen,
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

extern void
pwgaSetPalette(
    Pixel pixel,
    unsigned short red,
    unsigned short green,
    unsigned short blue );

/*----------- WGALINE.C -------------*/

extern void
wgaLineSS(
    struct _Drawable *pDrawable,
    struct _GC *pGC,
    int mode,
    int npt,
    struct _DDXPoint *pptInit );

/*----------- WGASEG.C -------------*/

extern void
wgaSegmentSS(
    struct _Drawable *pDrawable,
    struct _GC *pGC,
    int nseg,
    xSegment *pSeg );

/*-------------------------------------*/

#if defined (DWDOS286)
#include "LINTGLOB.H"
#endif

