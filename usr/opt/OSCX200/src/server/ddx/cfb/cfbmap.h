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
 * $XConsortium: cfbmap.h,v 1.3 91/12/19 18:37:02 keith Exp $
 *
 * Copyright 1991 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/*
 * Map names around so that multiple depths can be supported simultaneously
 */

#if PSZ != 8 || defined (SPARSE_IO)

#ifndef SPARSE_IO
#if PSZ == 32
#define NAME(subname) CATNAME(cfb32,subname)
#endif

#if PSZ == 16
#define NAME(subname) CATNAME(cfb16,subname)
#endif

#if PSZ == 4
#define NAME(subname) CATNAME(cfb4,subname)
#endif

#else /* SPARSE_IO */

#if PSZ == 32
#define NAME(subname) CATNAME(cfb32sparse,subname)
#endif

#if PSZ == 16
#define NAME(subname) CATNAME(cfb16sparse,subname)
#endif

#if PSZ == 8
#define NAME(subname) CATNAME(cfb8sparse,subname)
#endif

#if PSZ == 4
#define NAME(subname) CATNAME(cfb4sparse,subname)
#endif
#endif /* SPARSE_IO */

#ifndef NAME
cfb can not hack PSZ yet
#endif

#if __STDC__ && !defined(UNIXCPP)
#define CATNAME(prefix,subname) prefix##subname
#else
#define CATNAME(prefix,subname) prefix/**/subname
#endif


#define cfbScreenPrivateIndex NAME(ScreenPrivateIndex)
#define QuartetBitsTable NAME(QuartetBitsTable)
#define QuartetPixelMaskTable NAME(QuartetPixelMaskTable)
#define cfbAllocatePrivates NAME(AllocatePrivates)
#define cfbBSFuncRec NAME(BSFuncRec)
#define cfbBitBlt NAME(BitBlt)
#define cfbBresD NAME(BresD)
#define cfbBresS NAME(BresS)
#define cfbChangeClip NAME(ChangeClip)
#define cfbChangeGC NAME(ChangeGC)
#define cfbChangeWindowAttributes NAME(ChangeWindowAttributes)
#define cfbClipPoint NAME(ClipPoint)
#define cfbCloseScreen NAME(CloseScreen)
#define cfbCopyArea NAME(CopyArea)
#define cfbCopyClip NAME(CopyClip)
#define cfbCopyGC NAME(CopyGC)
#define cfbCopyImagePlane NAME(CopyImagePlane)
#define cfbCopyPixmap NAME(CopyPixmap)
#define cfbCopyPlane NAME(CopyPlane)
#define cfbCopyRotatePixmap NAME(CopyRotatePixmap)
#define cfbCopyWindow NAME(CopyWindow)
#define cfbCreateGC NAME(CreateGC)
#define cfbCreateOps NAME(CreateOps)
#define cfbCreatePixmap NAME(CreatePixmap)
#define cfbCreateWindow NAME(CreateWindow)
#define cfbDestroyClip NAME(DestroyClip)
#define cfbDestroyGC NAME(DestroyGC)
#define cfbDestroyOps NAME(DestroyOps)
#define cfbDestroyPixmap NAME(DestroyPixmap)
#define cfbDestroyWindow NAME(DestroyWindow)
#define cfbDoBitblt NAME(DoBitblt)
#define cfbDoBitbltCopy NAME(DoBitbltCopy)
#define cfbDoBitbltGeneral NAME(DoBitbltGeneral)
#define cfbDoBitbltOr NAME(DoBitbltOr)
#define cfbDoBitbltXor NAME(DoBitbltXor)
#define cfbFillBoxSolid NAME(FillBoxSolid)
#define cfbFillBoxTile32 NAME(FillBoxTile32)
#define cfbFillBoxTile32sCopy NAME(FillBoxTile32sCopy)
#define cfbFillBoxTile32sGeneral NAME(FillBoxTile32sGeneral)
#define cfbFillBoxTileOdd NAME(FillBoxTileOdd)
#define cfbFillBoxTileOddCopy NAME(FillBoxTileOddCopy)
#define cfbFillBoxTileOddGeneral NAME(FillBoxTileOddGeneral)
#define cfbFillPoly1RectCopy NAME(FillPoly1RectCopy)
#define cfbFillPoly1RectGeneral NAME(FillPoly1RectGeneral)
#define cfbFillRectSolidCopy NAME(FillRectSolidCopy)
#define cfbFillRectSolidGeneral NAME(FillRectSolidGeneral)
#define cfbFillRectSolidXor NAME(FillRectSolidXor)
#define cfbFillRectTile32Copy NAME(FillRectTile32Copy)
#define cfbFillRectTile32General NAME(FillRectTile32General)
#define cfbFillRectTileOdd NAME(FillRectTileOdd)
#define cfbFillSpanTile32sCopy NAME(FillSpanTile32sCopy)
#define cfbFillSpanTile32sGeneral NAME(FillSpanTile32sGeneral)
#define cfbFillSpanTileOddCopy NAME(FillSpanTileOddCopy)
#define cfbFillSpanTileOddGeneral NAME(FillSpanTileOddGeneral)
#define cfbFinishScreenInit NAME(FinishScreenInit)
#define cfbGCFuncs NAME(GCFuncs)
#define cfbGetImage NAME(GetImage)
#define cfbGetSpans NAME(GetSpans)
#define cfbHorzS NAME(HorzS)
#define cfbImageGlyphBlt8 NAME(ImageGlyphBlt8)
#define cfbLineSD NAME(LineSD)
#define cfbLineSS NAME(LineSS)
#define cfbMapWindow NAME(MapWindow)
#define cfbMatchCommon NAME(MatchCommon)
#define cfbNonTEOps NAME(NonTEOps)
#define cfbNonTEOps1Rect NAME(NonTEOps1Rect)
#define cfbPadPixmap NAME(PadPixmap)
#define cfbPaintWindow NAME(PaintWindow)
#define cfbPolyGlyphBlt8 NAME(PolyGlyphBlt8)
#define cfbPolyGlyphRop8 NAME(PolyGlyphRop8)
#define cfbPolyFillArcSolidCopy NAME(PolyFillArcSolidCopy)
#define cfbPolyFillArcSolidGeneral NAME(PolyFillArcSolidGeneral)
#define cfbPolyFillRect NAME(PolyFillRect)
#define cfbPolyPoint NAME(PolyPoint)
#define cfbPositionWindow NAME(PositionWindow)
#define cfbPutImage NAME(PutImage)
#define cfbReduceRasterOp NAME(ReduceRasterOp)
#define cfbRestoreAreas NAME(RestoreAreas)
#define cfbSaveAreas NAME(SaveAreas)
#define cfbScreenInit NAME(ScreenInit)
#define cfbSegmentSD NAME(SegmentSD)
#define cfbSegmentSS NAME(SegmentSS)
#define cfbSetScanline NAME(SetScanline)
#define cfbSetSpans NAME(SetSpans)
#define cfbSetupScreen NAME(SetupScreen)
#define cfbSolidSpansCopy NAME(SolidSpansCopy)
#define cfbSolidSpansGeneral NAME(SolidSpansGeneral)
#define cfbSolidSpansXor NAME(SolidSpansXor)
#define cfbStippleStack NAME(StippleStack)
#define cfbStippleStackTE NAME(StippleStackTE)
#define cfbTEGlyphBlt NAME(TEGlyphBlt)
#define cfbTEOps NAME(TEOps)
#define cfbTEOps1Rect NAME(TEOps1Rect)
#define cfbTile32FSCopy NAME(Tile32FSCopy)
#define cfbTile32FSGeneral NAME(Tile32FSGeneral)
#define cfbUnmapWindow NAME(UnmapWindow)
#define cfbUnnaturalStippleFS NAME(UnnaturalStippleFS)
#define cfbUnnaturalTileFS NAME(UnnaturalTileFS)
#define cfbValidateGC NAME(ValidateGC)
#define cfbVertS NAME(VertS)
#define cfbXRotatePixmap NAME(XRotatePixmap)
#define cfbYRotatePixmap NAME(YRotatePixmap)
#define cfbendpartial NAME(endpartial)
#define cfbendtab NAME(endtab)
#define cfbmask NAME(mask)
#define cfbrmask NAME(rmask)
#define cfbstartpartial NAME(startpartial)
#define cfbstarttab NAME(starttab)
#define cfb8LineSS1Rect NAME(LineSS1Rect)
#define cfb8SegmentSS1Rect NAME(SegmentSS1Rect)
#define cfb8ClippedLineCopy NAME(ClippedLineCopy)
#define cfb8ClippedLineXor NAME(ClippedLineXor)
#define cfb8ClippedLineGeneral  NAME(ClippedLineGeneral )
#define cfb8SegmentSS1RectCopy NAME(SegmentSS1RectCopy)
#define cfb8SegmentSS1RectXor NAME(SegmentSS1RectXor)
#define cfb8SegmentSS1RectGeneral  NAME(SegmentSS1RectGeneral )
#define cfb8SegmentSS1RectShiftCopy NAME(SegmentSS1RectShiftCopy)
#define cfb8LineSS1RectCopy NAME(LineSS1RectCopy)
#define cfb8LineSS1RectXor NAME(LineSS1RectXor)
#define cfb8LineSS1RectGeneral  NAME(LineSS1RectGeneral )
#define cfb8LineSS1RectPreviousCopy NAME(LineSS1RectPreviousCopy)
#define cfbZeroPolyArcSS8Copy NAME(ZeroPolyArcSSCopy)
#define cfbZeroPolyArcSS8Xor NAME(ZeroPolyArcSSXor)
#define cfbZeroPolyArcSS8General NAME(ZeroPolyArcSSGeneral)

#define cfbCreateDefColormap NAME(CreateDefColormap)
#define cfbExpandDirectColors NAME(ExpandDirectColors)
#define cfbInitVisuals NAME(InitVisuals)
#define cfbInitializeColormap NAME(InitializeColormap)
#define cfbPuntCopyPlane NAME(PuntCopyPlane)
#define cfbResolveColor NAME(ResolveColor)
#define cfbSetVisualTypes NAME(SetVisualTypes)

#endif /* PSZ != 8 */
