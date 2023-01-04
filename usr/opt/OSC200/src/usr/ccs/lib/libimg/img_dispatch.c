
/***************************************************************************** 
**
**  Copyright (c) Digital Equipment Corporation, 1990 All Rights Reserved.
**  Unpublished rights reserved under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary to and embodies the 
**  confidential technology of Digital Equipment Corporation. Possession, use,
**  duplication or dissemination of the software and media is authorized only 
**  pursuant to a valid written license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the 
**  U.S. Government is subject to restrictions as set forth in 
**  Subparagraph (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**
*****************************************************************************/

/************************************************************************
**
**  FACILITY:
**
**      Image Service Library
**
**  ABSTRACT:
**
**      This module contains the initialization code for the ISL dispatch
**	table - ImgA_VectorTable.  The contents of each entry of this table
**	include the address of the Layer II function as well as those classes
**	and data types the Layer II function supports.
**
**  ENVIRONMENT:
**
**      VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
**      Richard Piccolo
**
**  CREATION DATE:
**
**      8-Aug-1989
**
**  MODIFICATION HISTORY:
**
************************************************************************/

/*
**  Include files:
*/

/*
**  External References for each Layer II function supported
*/
extern long _IpsScaleBitonal();		/* scale bitonal		    */
extern long _IpsScaleInterpolation();   /* scale bilinear interpolation	    */
extern long _IpsScaleNearestNeighbor();	/* scale sampling code		    */

extern long _IpsFlipHorizontalBitonal();/* flip horizontal bitonal only	    */
extern long _IpsFlipHorizontal();	/* flip horizontal		    */
extern long _IpsFlipVerticalBitonal();	/* flip vertical bitonal only	    */
extern long _IpsFlipVertical();		/* flip vertical		    */

extern long _IpsRemapUdp();		/* remap LUT			    */
extern long _ImgAlloc();		/* memory allocation		    */
extern long _ImgDealloc();		/* memory deallocation		    */
extern long _ImgRealloc();		/* memory reallocation		    */
extern long _ImgAllocateDataPlane();	/* image allocation                 */
extern long _ImgFreeDataPlane();	/* image deallocation               */
extern long _ImgReallocateDataPlane();	/* image reallocation               */
extern long _IpsCombine();		/* align and combine dst and src    */

extern long _IpsRotateBitonal();	/* rotate bitonal image             */
extern long _IpsRotateOrthogonal();	/* rotate image plane +-(90,180,270)*/
extern long _IpsRotateNearestNeighbor();/* rotate image plane               */
extern long _IpsRotateInterpolation();  /* rotate image plane               */

extern long _IpsDitherBluenoise();	/* bluenoise dither		    */
extern long _IpsDitherOrdered();	/* dispersed and clustered dither   */

extern long _IpsEncodeG31d();           /* compress image plane             */
extern long _IpsEncodeG32d();           /* compress image plane             */
extern long _IpsEncodeG42d();           /* compress image plane             */
extern long _IpsEncodeDct();            /* compress image plane             */

extern long _IpsDecodeG31d();           /* decompress image plane           */
extern long _IpsDecodeG32d();           /* decompress image plane           */
extern long _IpsDecodeG42d();           /* decompress image plane           */
extern long _IpsDecodeDct();            /* decompress image plane           */

extern long _IpsHistogram();            /* implicit indexed histogram       */
extern long _IpsHistogramSorted();      /* explicit sorted  histogram       */

extern long _IpsCopyBitonal();		/* copy	image plane bitonal only    */
extern long _IpsCopy();			/* copy	image plane		    */

extern long _IpsConvolve();		/* convolve an image plane	    */

extern long _IpsMergePlanes();		/* merge several image planes	    */

extern long _IpsLogicalBitonal();	/* logical point operation (bitonal)*/
extern long _IpsLogical();		/* logical point operation	    */

/*
**  ISL Vector Table
*/
#if defined(__VAXC) || defined(VAXC)
globaldef {"$$z_ad_IMGA_VECTORTABLE"} noshare
#endif
long (*ImgA_VectorTable[512])() =
	{
	_IpsScaleInterpolation,		/* 0 */
	_IpsScaleNearestNeighbor,	/* 1 */
	_IpsFlipHorizontal,		/* 2 */
	_IpsFlipVertical,		/* 3 */
	_IpsRemapUdp,			/* 4 */
	_ImgAlloc,			/* 5 */
	_ImgDealloc,			/* 6 */
	_ImgRealloc,			/* 7 */
	_ImgAllocateDataPlane,		/* 8 */
	_ImgFreeDataPlane,		/* 9 */
	_ImgReallocateDataPlane,	/* 10 */
	0,				/* 11 */
	_IpsCombine,			/* 12 */
	_IpsRotateBitonal,		/* 13 */
	_IpsRotateOrthogonal,		/* 14 */
	_IpsRotateNearestNeighbor,	/* 15 */
	_IpsRotateInterpolation,	/* 16 */
	_IpsDitherBluenoise,		/* 17 */
	_IpsDitherOrdered,		/* 18 */
        _IpsEncodeG31d,                 /* 19 */
        _IpsEncodeG32d,                 /* 20 */
        _IpsEncodeG42d,                 /* 21 */
        _IpsDecodeG31d,                 /* 22 */
        _IpsDecodeG32d,                 /* 23 */
        _IpsDecodeG42d,                 /* 24 */
	_IpsScaleBitonal,		/* 25 */
	_IpsHistogram,			/* 26 */
	_IpsHistogramSorted,		/* 27 */
	_IpsCopy,			/* 28 */
	_IpsConvolve,			/* 29 */
/*
**  The new guys for Ips
*/
	_IpsCopyBitonal,		/* 30 */
	_IpsFlipHorizontalBitonal,	/* 31 */
	_IpsFlipVerticalBitonal,	/* 32 */
	_IpsMergePlanes	,		/* 33 */
	_IpsLogicalBitonal,		/* 34 */
	_IpsLogical,			/* 35 */
	_IpsEncodeDct,			/* 36 */
	_IpsDecodeDct			/* 37 */ 
	};
