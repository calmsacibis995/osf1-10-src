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
 * @(#)$RCSfile: vga.h,v $ $Revision: 1.1.4.5 $ (DEC) $Date: 1993/11/22 17:34:22 $
 */

typedef unsigned char IMAGE_UNIT;
#ifdef DWDOS286
typedef IMAGE_UNIT huge *IMAGE_PTR;
#else
typedef IMAGE_UNIT *IMAGE_PTR;
#endif

#include <cursor.h>

typedef void (*SetPaletteFuncPtr)(
    Pixel pixel,
    unsigned short red,
    unsigned short green,
    unsigned short blue );


typedef void (*BlitFuncPtr)(
    ScreenPtr pScreen,
    int xSrc,
    int ySrc,
    int wSrc,
    int hSrc,
    int xDst,
    int yDst,
    unsigned int alu,
    unsigned int planemask );


typedef void (*ReplicateScansFuncPtr)(
    ScreenPtr pScreen,
    int xSrc,
    int ySrc,
    int wSrc,
    int hSrc,
    int hDst,
    unsigned int planemask );


typedef void (*FillSolidFuncPtr)(
    ScreenPtr pScreen,
    unsigned int fg,
    unsigned int alu,
    unsigned int planemask,
    int xDst,
    int yDst,
    int wDst,
    int hDst,
    int first );


typedef void (*DrawColorImageFuncPtr)(
    ScreenPtr pScreen,
    int xDst,
    int yDst,
    int wDst,
    int hDst,
    unsigned char *pSrc,
    int widthSrc,
    unsigned int alu,
    unsigned int planemask,
    int first );


typedef void (*ReadColorImageFuncPtr)(
    ScreenPtr pScreen,
    int xSrc,
    int ySrc,
    int wSrc,
    int hSrc,
    unsigned long planemask,
    unsigned char *pDst,
    int widthDst );


typedef void (*OpaqueStippleFuncPtr)(
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


typedef void (*StippleFuncPtr)(
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


typedef void (*BresSFuncPtr)(
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


typedef void (*BresOnOffFuncPtr)(
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


typedef void (*BresDoubleFuncPtr)(
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


typedef void (*DrawCursorFuncPtr)(
    int x,
    int y);

typedef void (*UndrawCursorFuncPtr)(
    void);

typedef void (*SetNewCursorFuncPtr)(
    ScreenPtr pScr,
    CursorPtr pCurs);

typedef void (*CursorInitFuncPtr)(
    ScreenPtr pScr);

typedef void (*ShowCursorFuncPtr)(
    void );

typedef void (*HideCursorInXYWHFuncPtr)(
    int x,
    int y,
    int w,
    int h );

typedef void (*HideCursorInSpansFuncPtr)(
    DDXPointPtr ppt,
    int *pwidth,
    int count );

typedef void (*HideCursorInLineFuncPtr)(
    int x1,
    int y1,
    int x2,
    int y2 );


typedef struct _DrawFuncs {
  SetPaletteFuncPtr        SetPaletteFunc;
  BlitFuncPtr		   BlitFunc;
  ReplicateScansFuncPtr	   ReplicateScansFunc;
  FillSolidFuncPtr	   FillSolidFunc;
  DrawColorImageFuncPtr	   DrawColorImageFunc;
  ReadColorImageFuncPtr	   ReadColorImageFunc;
  OpaqueStippleFuncPtr	   OpaqueStippleFunc;
  StippleFuncPtr	   StippleFunc;
  BresSFuncPtr		   BresSFunc;
  BresOnOffFuncPtr	   BresOnOffFunc;
  BresDoubleFuncPtr	   BresDoubleFunc;
#ifdef SOFTWARE_CURSOR
  DrawCursorFuncPtr	   DrawCursorFunc;
  UndrawCursorFuncPtr	   UndrawCursorFunc;
  SetNewCursorFuncPtr	   SetNewCursorFunc;
  CursorInitFuncPtr	   CursorInitFunc;
  ShowCursorFuncPtr        ShowCursorFunc;
  HideCursorInXYWHFuncPtr  HideCursorInXYWHFunc;
  HideCursorInSpansFuncPtr HideCursorInSpansFunc;
  HideCursorInLineFuncPtr  HideCursorInLineFunc;
#endif
} DrawFuncs;

typedef struct {
  pointer	vgaregs;	/* Pointer to vga regs */
  pointer	firstScreenMem;
  pointer	lastScreenMem;
  pointer	avail;
  pointer	*rover;
} vgaScreenPrivRec, *vgaScreenPrivPtr;
