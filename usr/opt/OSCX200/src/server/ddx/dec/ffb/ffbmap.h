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
 * @(#)$RCSfile: ffbmap.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:11:54 $
 */
/*
 */

/*
 *
 *
 * NAME           ffbmap.h
 *
 * Description    Transforms generic procedure names into depth specific
 *                variants.
 *
 * Notes
 */

#ifndef FFBMAP_H
#define FFBMAP_H

#define SortRectsAndPoints	ffbSortRectsAndPoints

#define ffbPolyArc1  		FFB_NAME(PolyArc1)
#define ffbPolyPoint  		FFB_NAME(PolyPoint)
#define ffb_InitProc  		FFB_NAME(_InitProc)
#define ffbTEImageGlyphBlt  	FFB_NAME(TEImageGlyphBlt)
#define ffbSplatGlyphs  	FFB_NAME(SplatGlyphs)
#define ffbLineArc  		FFB_NAME(LineArc)
#define ffbLineArcI  		FFB_NAME(LineArcI)
#define ffbLineArcD  		FFB_NAME(LineArcD)
#define ffbFillPolyHelper  	FFB_NAME(FillPolyHelper)
#define ffbFillRectPolyHelper   FFB_NAME(FillRectPolyHelper)
#define ffbLineJoin 		FFB_NAME(LineJoin)
#define ffbLineProjectingCap    FFB_NAME(LineProjectingCap)
#define ffbWideSegment		FFB_NAME(WideSegment)
#define ffbWideLine		FFB_NAME(WideLine)
#define ffbWideDashSegment      FFB_NAME(WideDashSegment)
#define ffbWideDash             FFB_NAME(WideDash)
#define ffbZeroPolyArc          FFB_NAME(ZeroPolyArc)

#ifdef Bits64
#define ffbTESplatGlyphs6       FFB_NAME(TESplatGlyphs6)
#define ffbTESplatGlyphs8       FFB_NAME(TESplatGlyphs8)
#define ffbTESplatGlyphs10      FFB_NAME(TESplatGlyphs10)
#define ffbTESplatGlyphs15      FFB_NAME(TESplatGlyphs15)
#define ffbTESplatGlyphs16      FFB_NAME(TESplatGlyphs16)
#define ffbTESplatGlyphs20      FFB_NAME(TESplatGlyphs20)
#define ffbTESplatGlyphs30      FFB_NAME(TESplatGlyphs30)
#define ffbTESplatGlyphs32      FFB_NAME(TESplatGlyphs32)
#define ffbTEImageGlyphs6       FFB_NAME(TEImageGlyphs6)
#define ffbTEImageGlyphs8       FFB_NAME(TEImageGlyphs8)
#define ffbTEImageGlyphs10      FFB_NAME(TEImageGlyphs10)
#define ffbTEImageGlyphs15      FFB_NAME(TEImageGlyphs15)
#define ffbTEImageGlyphs16      FFB_NAME(TEImageGlyphs16)
#define ffbTEImageGlyphs20      FFB_NAME(TEImageGlyphs20)
#define ffbTEImageGlyphs30      FFB_NAME(TEImageGlyphs30)
#define ffbTEImageGlyphs32      FFB_NAME(TEImageGlyphs32)
#else
#define ffbTESplatGlyphs7       FFB_NAME(TESplatGlyphs7)
#define ffbTESplatGlyphs8       FFB_NAME(TESplatGlyphs8)
#define ffbTESplatGlyphs14      FFB_NAME(TESplatGlyphs14)
#define ffbTESplatGlyphs16      FFB_NAME(TESplatGlyphs16)
#define ffbTESplatGlyphs32      FFB_NAME(TESplatGlyphs32)
#define ffbTEImageGlyphs7       FFB_NAME(TEImageGlyphs7)
#define ffbTEImageGlyphs8       FFB_NAME(TEImageGlyphs8)
#define ffbTEImageGlyphs14      FFB_NAME(TEImageGlyphs14)
#define ffbTEImageGlyphs16      FFB_NAME(TEImageGlyphs16)
#define ffbTEImageGlyphs32      FFB_NAME(TEImageGlyphs32)
#endif

#define ffbSegS1                FFB_NAME(SegS1)
#define ffbLineS1               FFB_NAME(LineS1)
#define ffbFillBoxSolid         FFB_NAME(FillBoxSolid)
#define ffbFillBoxTile	        FFB_NAME(FillBoxTile)
#define ffbTEPolyGlyphBlt       FFB_NAME(TEPolyGlyphBlt)
#define ffbPolyGlyphBlt         FFB_NAME(PolyGlyphBlt)
#define ffbImageGlyphBlt        FFB_NAME(ImageGlyphBlt)
#define ffbPolyFillArc          FFB_NAME(PolyFillArc)
#define ffbPolyFillRectSolid    FFB_NAME(PolyFillRectSolid)
#define ffbOSPlane	        FFB_NAME(OSPlane)
#define ffbTileSpansWord	FFB_NAME(TileSpansWord)
#define ffbTilePolyAreaWord    	FFB_NAME(TilePolyAreaWord)
#define ffbTileFillAreaWord    	FFB_NAME(TileFillAreaWord)
#define ffbTileFillAreaWord2   	ffbTileFillAreaWord
#define ffbSolidFillSpans       FFB_NAME(SolidFillSpans)
#define ffbTSFillSpans		FFB_NAME(TSFillSpans)
#define ffbOSFillSpans		FFB_NAME(OSFillSpans)
#define ffbTSSpansWord		FFB_NAME(TSSpansWord)
#define ffbOSSpansWord		FFB_NAME(OSSpansWord)
#define ffbFillArea             FFB_NAME(FillArea)
#define ffbTSFillArea		FFB_NAME(TSFillArea)
#define ffbOSFillArea		FFB_NAME(OSFillArea)
#define ffbTSFillAreaWord       FFB_NAME(TSFillAreaWord)
#define ffbOSFillAreaWord       FFB_NAME(OSFillAreaWord)
#define ffbTSFillAreaWord2      FFB_NAME(TSFillAreaWord2)
#define ffbOSFillAreaWord2      FFB_NAME(OSFillAreaWord2)
#define ffbLineSS               FFB_NAME(LineSS)
#define ffbSegmentSS            FFB_NAME(SegmentSS)
#define ffbDashSegment          FFB_NAME(DashSegment)
#define ffbDashLine             FFB_NAME(DashLine)
#define ffbDashSegment16        FFB_NAME(DashSegment16)
#define ffbDashLine16           FFB_NAME(DashLine16)
#define ffbPaintWindow 		FFB_NAME(PaintWindow)
#define ffbScreenInit		FFB_NAME(ScreenInit)
#define ffbCloseScreen		FFB_NAME(CloseScreen)
#define ffbSetupScreen		FFB_NAME(SetupScreen)
#define ffbFillPolygon		FFB_NAME(FillPolygon)
#define ffbPolyRectangle	FFB_NAME(PolyRectangle)
#define ffbValidateGC		FFB_NAME(ValidateGC)
#define ffbCreateGC		FFB_NAME(CreateGC)
#define ffbDestroyGC		FFB_NAME(DestroyGC)
#define ffbWriteWindowTypeTable	FFB_NAME(WriteWindowTypeTable)
#define ffbWindowID		FFB_NAME(WindowID)
/* don't need rop variants for copies where dst==screen because chip does it all */
#define ffbBitbltMemScrDMA      FFB_COPY_NAME(BitbltMemScrDMA)
#define ffbBitbltScrScr         FFB_COPY_NAME(BitbltScrScr)
#define ffbBitbltMemScr		FFB_COPY_NAME(BitbltMemScr)

/* dma from screen to main memory can only be used where rop is copy */
#define ffbBitbltScrMemDMA      FFB_COPY_NAME(BitbltScrMemDMA)

#define ffbBitbltScrMemCopy     FFB_COPY_NAME(BitbltScrMemCopy)
#define ffbBitbltScrMemCopySPM  FFB_COPY_NAME(BitbltScrMemCopySPM)
#define ffbBitbltScrMemGeneral  FFB_COPY_NAME(BitbltScrMemGeneral)
#define ffbBitbltScrMemXor      FFB_COPY_NAME(BitbltScrMemXor)
#define ffbTileFillSpans	FFB_COPY_NAME(TileFillSpans)
#define ffbTileFillArea         FFB_COPY_NAME(TileFillArea)
#define ffb_GetSpans            FFB_COPY_NAME(_GetSpans)

#endif /* FFBMAP_H */

/*
 * HISTORY
 */
