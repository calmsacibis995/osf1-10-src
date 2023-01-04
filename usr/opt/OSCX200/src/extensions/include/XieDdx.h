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
Copyright 1989 by Digital Equipment Corporation, Maynard, Massachusetts,
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

/************************************************************************
**
**  FACILITY:
**
**      XIE - X11 Image Extension
**		  
**
**  ABSTRACT:
**
**	This module contains definitions required by the XIE  library 
**	and XIE server DDX layers.
**	
**  ENVIRONMENT:
**
**	VAX/VMS V5.3
**	ULTRIX  V3.1
**
**  AUTHOR(S):
**
**      Richard J. Piccolo
**      Robert NC Shelley
**
**  CREATION DATE:
**
**      September 12, 1989
**
************************************************************************/
    /*
    **	Symbol XIEDDX allows XieDdx.h to be included multiple times.
    */
#ifndef XIEDDX
#define XIEDDX		/* the "endif" MUST be the last line of this file   */

/*
**  Include files
*/
#ifndef XIEAPPL
#ifdef VMS
#include <XieAppl.h>
#else
#include <X11/extensions/XieAppl.h>
#endif
#endif	/* XIEAPPL */

/*
**  MACRO definitions
*/
    /*
    **	Return the minimum number of bits required to contain the specified
    **	number of levels.  Levels == 0 indicates unconstrained data of type
    **	float.		   <<<< remember to include math.h >>>>
    */
#define BitsFromLevels_(levels) \
	((levels) > 2 ? ((int)(ceil(log10((double)levels)/log10(2.0)))) : \
	 (levels) > 0 ? 1 : sizeof(float) * 8)
    /*
    **	Determine if a value is a valid pointer or an error code.
    **			   <<<< remember to include X.h >>>>
    */
#ifdef	BadImplementation
#define IsPointer_(ptr) (((unsigned long)(ptr)) > BadImplementation)
#endif
    /*
    **  Convenience macros for accessing generic UdpPtr fields.
    */
#define upPxlLen_(udp)	((udp)->UdpW_PixelLength)
#define upDType_(udp)	((udp)->UdpB_DType)
#define upClass_(udp)	((udp)->UdpB_Class)
#define upBase_(udp)	((udp)->UdpA_Base)
#define upArSize_(udp)	((udp)->UdpL_ArSize)
#define upPxlStr_(udp)	((udp)->UdpL_PxlStride)
#define upScnStr_(udp)	((udp)->UdpL_ScnStride)
#define upX1_(udp)	((udp)->UdpL_X1)
#define upX2_(udp)	((udp)->UdpL_X2)
#define upY1_(udp)	((udp)->UdpL_Y1)
#define upY2_(udp)	((udp)->UdpL_Y2)
#define upWidth_(udp)	((udp)->UdpL_PxlPerScn)
#define upHeight_(udp)	((udp)->UdpL_ScnCnt)
#define upPos_(udp)	((udp)->UdpL_Pos)
#define upCmpIdx_(udp)	((udp)->UdpL_CompIdx)
#define upLvl_(udp)	((udp)->UdpL_Levels)
    /*
    **  Convenience macros for accessing generic UdpRec fields.
    */
#define urPxlLen_(udp)	((udp).UdpW_PixelLength)
#define urDType_(udp)	((udp).UdpB_DType)
#define urClass_(udp)	((udp).UdpB_Class)
#define urBase_(udp)	((udp).UdpA_Base)
#define urArSize_(udp)	((udp).UdpL_ArSize)
#define urPxlStr_(udp)	((udp).UdpL_PxlStride)
#define urScnStr_(udp)	((udp).UdpL_ScnStride)
#define urX1_(udp)	((udp).UdpL_X1)
#define urX2_(udp)	((udp).UdpL_X2)
#define urY1_(udp)	((udp).UdpL_Y1)
#define urY2_(udp)	((udp).UdpL_Y2)
#define urWidth_(udp)	((udp).UdpL_PxlPerScn)
#define urHeight_(udp)	((udp).UdpL_ScnCnt)
#define urPos_(udp)	((udp).UdpL_Pos)
#define urCmpIdx_(udp)	((udp).UdpL_CompIdx)
#define urLvl_(udp)	((udp).UdpL_Levels)

    /*
    **  The _MD forms access memory in machine-dependant (bigendian or 
    **  littleendian) format.  The other forms access memory in
    **  image format, which is always little endian.
    */
#ifdef vax
#define READ32_(address) \
    (*((int *)(address)))
#define READ32_MD READ32_
#else
    /*
    **  For machine architectures that can't access longwords
    **  on non-longword aligned boundaries.
    **  The (unsigned int) casts seem redundant, but they are needed
    **  to make the ULTRIX/RISC compiler happy.
    */
#define READ32_(address) \
   ((unsigned int) *((unsigned char *)(address))         + \
   ((unsigned int) *(((unsigned char *)(address))+1)<< 8) + \
   ((unsigned int) *(((unsigned char *)(address))+2)<<16) + \
   ((unsigned int) *(((unsigned char *)(address))+3)<<24))
#endif

#ifdef BIGENDIAN
#define READ32_MD(address) \
   (*(((unsigned char *)(address))+3)      + \
   (*(((unsigned char *)(address))+2)<< 8) + \
   (*(((unsigned char *)(address))+1)<<16) + \
   (* ((unsigned char *)(address))   <<24))
#else
#define READ32_MD READ32_
#endif
  
#define GET_VALUE_(base,offset,mask) \
   (READ32_((base)+((offset)>>3))>>((offset)&7)&(mask))
#define GET_VALUE_MD(base,offset,mask) \
   (READ32_MD((base)+((offset)>>3))>>((offset)&7)&(mask))
  
#ifdef vax
#define WRITE32_(address,value) \
    (*((int *)(address)) = (int)(value))
#define WRITE32_MD WRITE32_
#else
#define WRITE32_(address,value) \
   ((* ((unsigned char *)(address))    =  ((int)(value))     &0xff), \
    (*(((unsigned char *)(address))+1) = (((int)(value))>> 8)&0xff), \
    (*(((unsigned char *)(address))+2) = (((int)(value))>>16)&0xff), \
    (*(((unsigned char *)(address))+3) = (((int)(value))>>24)&0xff))
#ifdef BIGENDIAN
#define WRITE32_MD(address,value) \
   ((*(((unsigned char *)(address))+3) =  ((int)(value))     &0xff), \
    (*(((unsigned char *)(address))+2) = (((int)(value))>> 8)&0xff), \
    (*(((unsigned char *)(address))+1) = (((int)(value))>>16)&0xff), \
    (* ((unsigned char *)(address))    = (((int)(value))>>24)&0xff))
#else
#define WRITE32_MD WRITE32_
#endif
#endif

#ifdef vax
#define XOR_WRITE32_(address,value) \
    (*((int *)(address)) ^= (int)(value))
#define XOR_WRITE32_MD XOR_WRITE32_
#else
#define XOR_WRITE32_(address,value) \
   ((* ((unsigned char *)(address))    ^=  ((int)(value))     &0xff), \
    (*(((unsigned char *)(address))+1) ^= (((int)(value))>> 8)&0xff), \
    (*(((unsigned char *)(address))+2) ^= (((int)(value))>>16)&0xff), \
    (*(((unsigned char *)(address))+3) ^= (((int)(value))>>24)&0xff))
#ifdef BIGENDIAN
#define XOR_WRITE32_MD(address,value) \
   ((*(((unsigned char *)(address))+3) ^=  ((int)(value))     &0xff), \
    (*(((unsigned char *)(address))+2) ^= (((int)(value))>> 8)&0xff), \
    (*(((unsigned char *)(address))+1) ^= (((int)(value))>>16)&0xff), \
    (*(((unsigned char *)(address))    ^= (((int)(value))>>24)&0xff))
#else
#define XOR_WRITE32_MD XOR_WRITE32_
#endif
#endif

#define PUT_VALUE_(base,offset,value,mask) \
      (WRITE32_((base)+((offset)>>3), \
        READ32_((base)+((offset)>>3)) & ~((mask)<<((offset)&7)) \
				      | (((mask)&(value))<<((offset)&0x7))))

#define PUT_VALUE_MD(base,offset,value,mask) \
      (WRITE32_MD((base)+((offset)>>3), \
        READ32_MD((base)+((offset)>>3)) & ~((mask)<<((offset)&7)) \
				        | (((mask)&(value))<<((offset)&0x7))))


    /*
    **	Degrees to radians conversion factor -- with inverted Y axis.
    */
#define RADIANS_PER_DEGREE -0.0174532925199432959
    /*
    **  These symbols define valid UdpRec data type and class combinations.
    */
#define XieK_UBAVU	0   /* unaligned bit array			    */
#define XieK_UBAV	1   /* unaligned bit array that is aligned ??	    */
#define XieK_UBSVU	2   /* unaligned bit stream			    */
#define XieK_UBSV	3   /* unaligned bit stream that is aligned ??	    */
#define XieK_CLCL	4   /* change list				    */
#define XieK_ABU	5   /* aligned byte array			    */
#define XieK_AF		6   /* aligned float array			    */
#define XieK_AWU	7   /* aligned word array			    */
/*	    Not yet implemented
#define XieK_ALU	8
*/
#define XieK_DATA_TYPE_COUNT 8

    /*	 
    **  Defs for DDX table entry points
    */	 
#ifndef externalref
#if (defined(VMS) && defined(VAXC))
#define externalref globalref
#else
#define externalref extern
#endif
#endif

    /*
    **  Typedefs
    */
typedef unsigned long    card32;
typedef unsigned long (*Fcard32)();	    /* Used in DDX dispatch table   */

    /*
    **  Data structures for passing Template IDC (kernel) data to DDX
    **  routines.
    */
typedef struct {
    float	value;			/* A template value		    */
    long int	increment;		/* Number of kernel elements skipped
					 * between previous and this element,
					 * plus 1. Zero marks end of row    */
} TmpDatEntryRec, *TmpDatEntryPtr;

typedef struct {
    long int	    center_x;		/* Template center X coordinate	    */
    long int	    center_y;		/* Template center Y coordinate	    */
    long int        x_count;		/* Template x dimension		    */
    long int        y_count;		/* Template y dimension		    */
    TmpDatEntryPtr  array;		/* Pointer to the template data
				         * records			    */
} TmpDatRec, *TmpDatPtr;

/*****************************************************************************
**
**  Xie Ddx Index - 
**
**  FUNCTIONAL DESCRIPTION:
**
**      The following symbols define the indices into the dispatch table
**      used to call "atomic" DDX functions.  These functions include both
**      processing routines for the protocol functions and DDX support
**      routines.
**
**	    WARNING: The indices here AND the order of the XieDdxProc table
**		     in SmiDdx.c MUST match.
**
*****************************************************************************/
    /*	 
    **  Protocol processing routines
    */	 
#define D_dxArea		    0	/* Protocol functions */
#define D_dxStatsArea               1
#define D_dxArith                   2
#define D_dxHistogram               3
#define D_dxChromeCom               4
#define D_dxChromeSep               5
#define D_dxCompare                 6
#define D_dxConstrain               7
#define D_dxCrop		    8
#define D_dxDither		    9
#define D_dxFill		    10
#define D_dxLogical		    11
#define D_dxLuminance		    12
#define D_dxMatchHistogram	    13
#define D_dxMath		    14
#define D_dxMirror		    15
#define D_dxPoint		    16
#define D_dxStatsPoint   	    17
#define D_dxRotate		    18
#define D_dxScale		    19
#define D_dxTranslate		    20
#define D_dxRmOwnData		    21
				 /* 21..32 -- was Compression handlers  */
    /*	 
    **  DDX specific function indices
    */	 
#define D_dxInit		    33  /* DDX Init routine		    */
#define D_dxReset		    34  /*    "   Reset			    */
#define D_dxOptimizePipe	    35  /*    "   Pipeline optimizer	    */
#define D_dxConvert		    36	/* Udp support routines		    */
#define D_dxCopy		    37
#define D_dxGetTyp		    38
#define D_dxCopyBits		    39
#define D_dxResolveUdp		    40
#define D_dxCalloc		    41  /* Memory mgt routines          */
#define D_dxCallocBits		    42
#define D_dxCfree		    43
#define D_dxFree		    44
#define D_dxFreeBits		    45
#define D_dxMalloc		    46
#define D_dxMallocBits		    47
#define D_dxRealloc		    48
#define D_dxReallocBits		    49
#define D_dxCreatePipeline	    50	/* Pipeline routines		*/
#define D_dxDestroyPipeline	    51
#define D_dxPipeHasYielded	    52	    /* was D_dxExecutePipeline */
#define D_dxInitiatePipeline	    53
#define D_dxResumePipeline	    54
#define D_dxFlushPipeline	    55
#define D_dxAbortPipeline	    56
#define D_dxRmAllocData		    57	/* Pipe resource mgr routines	*/
#define D_dxRmAllocDataDesc	    58
#define D_dxRmAllocPermDesc	    59
#define D_dxRmAllocRefDesc	    60
#define D_dxRmBestDType		    61
#define D_dxRmCreateDrain	    62
#define D_dxRmCreateSink	    63
#define D_dxRmDeallocData	    64
#define D_dxRmDeallocDataDesc	    65
#define D_dxRmDestroyDrain	    66
#define D_dxRmDestroySink	    67
#define D_dxRmGetData		    68
#define D_dxRmGetDType		    69
#define D_dxRmGetMaxQuantum	    70
#define D_dxRmGetQuantum	    71
#define D_dxRmHasQuantum	    72
#define D_dxRmInitializePort	    73
#define D_dxRmMergeBuffers	    75
#define D_dxRmObtainQuantum	    76
#define D_dxRmPutData		    77
#define D_dxRmSetDType		    78
#define D_dxRmSetQuantum	    79
#define D_dxRmSplitBuffers	    80
#define D_dxSchedProcessComplete    81	/*  Pipe scheduling routines	    */
#define D_dxSchedProcessReady	    82
#define D_dxSchedOkToRun	    83
#define D_dxBuildChangeList	    84	/*  Change list routines	    */
#define D_dxPutRuns		    85
#define D_dxFfsLong		    86
#define D_dxFfcLong		    87
#define D_dxFillRegion		    88	/* Miscellaneous additions */
#define D_dxConvertCpp		    89
#define D_dxCopyCpp		    90
#define D_dxEncodeG4                91
#define D_dxEncodeDct               92
#define D_dxDecodeG4                93
#define D_dxDecodeDct               94
#define D_dxPosCl		    95
#define D_dxCheckLicense	    96
					/* More RM functions		    */
#define	D_dxRmAllocCmpDataDesc	    97	/* was D_dxRmGetAlignment */
#define D_dxRmSetAlignment	    98
#define D_dxRmCreateDataDesc	    99
#define D_dxCreatePipeCtx	    100
#define D_dxUsingCL		    101
#define D_dxCmpPreferred	    102	/* was D_dxSmiDecodeG4IsAtomic */
#define D_dxRmAllocDataArray	    103
#define D_dxRmAllocCmpPermDesc      104
#define D_dxSchedProcessNoInput     105

/*****************************************************************************
**
**  DDX Access Macros
**
**  FUNCTIONAL DESCRIPTION:
**
**	These macros provide access to the DDX functions while hiding
**	the implementation from the caller.
**
*****************************************************************************/
    /*
    **  DDX function access macro
    **  This macro calls an arbitrary DDX routine through the 
    **  xieDdxProc dispatch table.
    **
    **  func = the Mi function index from this file.
    **
    */
externalref card32 (*xieDdxProc[])();
#define CallDdx_(func) (*(xieDdxProc)[func])
    /*	 
    ** Image functions 
    */	 
#define DdxArea_ CallDdx_(D_dxArea)
#define DdxStatsArea_ CallDdx_(D_dxStatsArea)
#define DdxArith_ CallDdx_(D_dxArith)
#define DdxHistogram_ CallDdx_(D_dxHistogram)
#define DdxChromeCom_ CallDdx_(D_dxChromeCom)
#define DdxChromeSep_ CallDdx_(D_dxChromeSep)
#define DdxCompare_ CallDdx_(D_dxCompare)
#define DdxConstrain_ CallDdx_(D_dxConstrain)
#define DdxCrop_ CallDdx_(D_dxCrop)
#define DdxDither_ CallDdx_(D_dxDither)
#define DdxFill_ CallDdx_(D_dxFill)
#define DdxLogical_ CallDdx_(D_dxLogical)
#define DdxLuminance_ CallDdx_(D_dxLuminance)
#define DdxMatchHistogram_ CallDdx_(D_dxMatchHistogram)
#define DdxMath_ CallDdx_(D_dxMath)
#define DdxMirror_ CallDdx_(D_dxMirror)
#define DdxPoint_ CallDdx_(D_dxPoint)
#define DdxStatsPoint_ CallDdx_(D_dxStatsPoint)
#define DdxRotate_ CallDdx_(D_dxRotate)
#define DdxScale_ CallDdx_(D_dxScale)
#define DdxTranslate_ CallDdx_(D_dxTranslate)
#define DdxRmOwnData_ CallDdx_(D_dxRmOwnData)
    /*	 
    **  Ddx specific functions
    */	 
#define DdxInit_ CallDdx_(D_dxInit)
#define DdxReset_ CallDdx_(D_dxReset)
#define DdxOptimizePipe_ CallDdx_(D_dxOptimizePipe)
    /*	 
    **	MI software functions
    */	 
#define DdxConvert_ CallDdx_(D_dxConvert)
#define DdxCopy_ CallDdx_(D_dxCopy)
#define DdxGetTyp_ CallDdx_(D_dxGetTyp)
#define DdxCopyBits_ CallDdx_(D_dxCopyBits)		/* 65 */
#define DdxResolveUdp_ CallDdx_(D_dxResolveUdp)
    /*	 
    **  Memory management routines
    */	 
#define DdxCalloc_ CallDdx_(D_dxCalloc)
#define DdxCallocBits_ (unsigned char *)CallDdx_(D_dxCallocBits)
#define DdxCfree_ CallDdx_(D_dxCfree)
#define DdxFree_ CallDdx_(D_dxFree)
#define DdxFreeBits_ (unsigned char *)CallDdx_(D_dxFreeBits)
#define DdxMalloc_ CallDdx_(D_dxMalloc)
#define DdxMallocBits_ (unsigned char *)CallDdx_(D_dxMallocBits)
#define DdxRealloc_ CallDdx_(D_dxRealloc)
#define DdxReallocBits_ (unsigned char *)CallDdx_(D_dxReallocBits)
    /*	 
    **  Pipeline support routines
    */	 
#define DdxCreatePipeline_ (Pipe)CallDdx_(D_dxCreatePipeline)
#define DdxDestroyPipeline_ (int)CallDdx_(D_dxDestroyPipeline)
#define DdxPipeHasYielded_ (int)CallDdx_(D_dxPipeHasYielded)
#define DdxInitiatePipeline_ (int)CallDdx_(D_dxInitiatePipeline)
#define DdxResumePipeline_ (int)CallDdx_(D_dxResumePipeline)
#define DdxFlushPipeline_ (int)CallDdx_(D_dxFlushPipeline)
#define DdxAbortPipeline_ (int)CallDdx_(D_dxAbortPipeline)
    /*	 
    **  Pipeline resource manager routines
    */	 
#define DdxRmAllocData_ (PipeDataPtr)CallDdx_(D_dxRmAllocData)
#define DdxRmAllocDataDesc_ (PipeDataPtr)CallDdx_(D_dxRmAllocDataDesc)
#define DdxRmAllocPermDesc_ (PipeDataPtr)CallDdx_(D_dxRmAllocPermDesc)
#define DdxRmAllocRefDesc_ (PipeDataPtr)CallDdx_(D_dxRmAllocRefDesc)
#define DdxRmBestDType_ (char)CallDdx_(D_dxRmBestDType)
#define DdxRmCreateDrain_ (PipeDrainPtr)CallDdx_(D_dxRmCreateDrain)
#define DdxRmCreateSink_ (PipeSinkPtr)CallDdx_(D_dxRmCreateSink)
#define DdxRmDeallocData_ (PipeDataPtr)CallDdx_(D_dxRmDeallocData)
#define DdxRmDeallocDataDesc_ (PipeDataPtr)CallDdx_(D_dxRmDeallocDataDesc)
#define DdxRmDestroyDrain_ (PipeDrainPtr)CallDdx_(D_dxRmDestroyDrain)
#define DdxRmDestroySink_ (PipeSinkPtr)CallDdx_(D_dxRmDestroySink)
#define DdxRmGetData_ (PipeDataPtr)CallDdx_(D_dxRmGetData)
#define DdxRmGetDType_ (int)CallDdx_(D_dxRmGetDType)
#define DdxRmGetMaxQuantum_ (int)CallDdx_(D_dxRmGetMaxQuantum)
#define DdxRmGetQuantum_ (int)CallDdx_(D_dxRmGetQuantum)
#define DdxRmHasQuantum_ (int)CallDdx_(D_dxRmHasQuantum)
#define DdxRmInitializePort_ (int)CallDdx_(D_dxRmInitializePort)
#define DdxRmMergeBuffers_ (PipeDataPtr)CallDdx_(D_dxRmMergeBuffers)
#define DdxRmObtainQuantum_ (PipeDataPtr)CallDdx_(D_dxRmObtainQuantum)
#define DdxRmPutData_ (int)CallDdx_(D_dxRmPutData)
#define DdxRmSetDType_ CallDdx_(D_dxRmSetDType)
#define DdxRmSetQuantum_ CallDdx_(D_dxRmSetQuantum)
#define DdxRmSplitBuffers_ (PipeDataPtr)CallDdx_(D_dxRmSplitBuffers)
    /*	 
    **  Pipe scheduling routines
    */	 
#define DdxSchedProcessComplete_ (PipeElementCtxPtr)CallDdx_(D_dxSchedProcessComplete)
#define DdxSchedProcessReady_ (PipeElementCtxPtr)CallDdx_(D_dxSchedProcessReady)
#define DdxSchedOkToRun_ (int)CallDdx_(D_dxSchedOkToRun)
    /*	 
    ** Change list routines
    */	 
#define DdxBuildChangeList_ CallDdx_(D_dxBuildChangeList)
#define DdxPutRuns_ CallDdx_(D_dxPutRuns)
#define DdxFfsLong_ CallDdx_(D_dxFfsLong)
#define DdxFfcLong_ CallDdx_(D_dxFfcLong)
    /*	 
    **  Additions
    */	 
#define DdxFillRegion_ CallDdx_(D_dxFillRegion)
#define DdxConvertCpp_ CallDdx_(D_dxConvertCpp)
#define DdxCopyCpp_ CallDdx_(D_dxCopyCpp)
#define DdxEncodeG4_ CallDdx_(D_dxEncodeG4)
#define DdxEncodeDct_ CallDdx_(D_dxEncodeDct)
#define DdxDecodeG4_ CallDdx_(D_dxDecodeG4)
#define DdxDecodeDct_ CallDdx_(D_dxDecodeDct)
#define DdxPosCl_ CallDdx_(D_dxPosCl)
#define DdxCheckLicense_ CallDdx_(D_dxCheckLicense)
    /*
    **  More RM functions
    */
#define DdxRmAllocCmpDataDesc_  (PipeDataPtr)CallDdx_(D_dxRmAllocCmpDataDesc)
#define DdxRmSetAlignment_  CallDdx_(D_dxRmSetAlignment)
#define DdxRmCreateDataDesc_ (PipeDataPtr)CallDdx_(D_dxRmCreateDataDesc)
#define DdxCreatePipeCtx_ (PipeElementCtxPtr)CallDdx_(D_dxCreatePipeCtx)
#define DdxUsingCL_       CallDdx_(D_dxUsingCL)
#define DdxCmpPreferred_    CallDdx_(D_dxCmpPreferred)
#define DdxRmAllocDataArray_ CallDdx_(D_dxRmAllocDataArray)
#define DdxRmAllocCmpPermDesc_ (PipeDataPtr)CallDdx_(D_dxRmAllocCmpPermDesc)
#define DdxSchedProcessNoInput_ (PipeElementCtxPtr)CallDdx_(D_dxSchedProcessNoInput)

/*****************************************************************************
**
**  Xie Ddx Setup Index - 
**
**  FUNCTIONAL DESCRIPTION:
**
**      The following symbols define the indices into the dispatch table
**      used to call "pipeline create" DDX functions.  Each of these
**      functions creates a new pipeline element.
**
**	    WARNING: These indices and the entries in the XieDdxSetup table 
**		     in SmiDdx.c MUST be in the same order.
**
*****************************************************************************/
#define D_dxCreateArea		    0
#define D_dxCreateArith		    1
#define D_dxCreateChromeCom	    2
#define D_dxCreateChromeSep	    3
#define D_dxCreateCompare           4
#define D_dxCreateConstrain         5
#define D_dxCreateCrop		    6
#define D_dxCreateDither	    7
#define D_dxCreateFill		    8
#define D_dxCreateLogical	    9
#define D_dxCreateLuminance	    10
#define D_dxCreateMatchHistogram    11
#define D_dxCreateMath		    12
#define D_dxCreateMirror	    13
#define D_dxCreatePoint		    14
#define D_dxCreateRotate	    15
#define D_dxCreateScale		    16
#define D_dxCreateTranslate	    17
#define D_dxCreateDecodeDct         18  /*  Compression pipeline creates    */
#define D_dxCreateEncodeDct         19
#define D_dxCreateDecodeG4          20
#define D_dxCreateEncodeG4          21

/*****************************************************************************
**
**  Pipeline Create Access Macros
**
**  FUNCTIONAL DESCRIPTION:
**
**	These macros provide access to the Pipeline Create functions while
**	hiding the implementation from the caller.
**
*****************************************************************************/
    /*
    **  Pipe create function access macro
    **  This macro calls a create function through the 
    **  xieDdxSetup dispatch table.
    **
    **  func = the setup function index from this file.
    **
    */
externalref card32 (*xieDdxSetup[])();
#define CallCreate_(func) (*(xieDdxSetup)[func])
        /*	 
        **  Image processing pipe element creates
        */	 
#define DdxCreateArea_ CallCreate_(D_dxCreateArea)
#define DdxCreateArith_ CallCreate_(D_dxCreateArith)
#define DdxCreateChromeCom_ CallCreate_(D_dxCreateChromeCom)
#define DdxCreateChromeSep_ CallCreate_(D_dxCreateChromeSep)
#define DdxCreateCompare_ CallCreate_(D_dxCreateCompare)
#define DdxCreateConstrain_ CallCreate_(D_dxCreateConstrain)
#define DdxCreateCrop_ CallCreate_(D_dxCreateCrop)
#define DdxCreateDither_ CallCreate_(D_dxCreateDither)
#define DdxCreateFill_ CallCreate_(D_dxCreateFill)
#define DdxCreateLogical_ CallCreate_(D_dxCreateLogical)
#define DdxCreateLuminance_ CallCreate_(D_dxCreateLuminance)
#define DdxCreateMatchHistogram_ CallCreate_(D_dxCreateMatchHistogram)
#define DdxCreateMath_ CallCreate_(D_dxCreateMath)
#define DdxCreateMirror_ CallCreate_(D_dxCreateMirror)
#define DdxCreatePoint_ CallCreate_(D_dxCreatePoint)
#define DdxCreateRotate_ CallCreate_(D_dxCreateRotate)
#define DdxCreateScale_ CallCreate_(D_dxCreateScale)
#define DdxCreateTranslate_ CallCreate_(D_dxCreateTranslate)
        /*	 
        **  Compression creates, used by Transport
        */	 
#define DdxCreateDecodeDct_ CallCreate_(D_dxCreateDecodeDct)
#define DdxCreateEncodeDct_ CallCreate_(D_dxCreateEncodeDct)
#define DdxCreateDecodeG4_ CallCreate_(D_dxCreateDecodeG4)
#define DdxCreateEncodeG4_ CallCreate_(D_dxCreateEncodeG4)

/*****************************************************************************
**
**  Ddx Alignment Function Access Index
**
**  FUNCTIONAL DESCRIPTION:
**
**      The following symbols define the indices into the DDX Alignment 
**	 dispatch table used to call DDX image array alignment functions.  
**	Each of these functions performs alignments to meet individual 
**	 algorithms or the default DDX needs.
**
**	    WARNING: These indices and the entries in the XieDdxAlign table 
**		     in SmiDdx.c MUST be in the same order.
**
**
*****************************************************************************/
#define D_dxAlignArea		    0
#define D_dxAlignArith		    1
#define D_dxAlignChromeCom	    2
#define D_dxAlignChromeSep	    3
#define D_dxAlignCompare            4
#define D_dxAlignConstrain          5
#define D_dxAlignCrop		    6
#define D_dxAlignDither		    7
#define D_dxAlignFill		    8
#define D_dxAlignLogical	    9
#define D_dxAlignLuminance	    10
#define D_dxAlignMatchHistogram     11
#define D_dxAlignMath		    12
#define D_dxAlignMirror		    13
#define D_dxAlignPoint		    14
#define D_dxAlignRotate		    15
#define D_dxAlignScale		    16
#define D_dxAlignTranslate	    17
#define D_dxAlignDecodeDct	    18
#define D_dxAlignEncodeDct	    19
#define D_dxAlignDecodeG4	    20
#define D_dxAlignEncodeG4	    21
#define D_dxAlignDefault	    22
#define	D_dxAlignExport		    23

/*****************************************************************************
**
**  DDX Alignment Function Access Macros
**
**  FUNCTIONAL DESCRIPTION:
**
**	These macros provide access to the DDX image array alignment functions
**
*****************************************************************************/
    /*
    **  Align function access macro
    **  This macro calls an arbitrary Alignment routine through the
    **  xieDdxAlign dispatch table.
    **
    **  func = the DDX alignment function index from this file.
    **
    */
externalref card32 (*xieDdxAlign[])();
#define GetAlign_(func) (xieDdxAlign[func])

#define DdxAlignArea_		GetAlign_(D_dxAlignArea)
#define DdxAlignStatsArea_	GetAlign_(D_dxAlignStatsArea)
#define DdxAlignArith_		GetAlign_(D_dxAlignArith)
#define DdxAlignHistogram_	GetAlign_(D_dxAlignHistogram)
#define DdxAlignChromeCom_	GetAlign_(D_dxAlignChromeCom)
#define DdxAlignChromeSep_	GetAlign_(D_dxAlignChromeSep)
#define DdxAlignCompare_	GetAlign_(D_dxAlignCompare)
#define DdxAlignConstrain_	GetAlign_(D_dxAlignConstrain)
#define DdxAlignCrop_		GetAlign_(D_dxAlignCrop)
#define DdxAlignDither_		GetAlign_(D_dxAlignDither)
#define DdxAlignFill_		GetAlign_(D_dxAlignFill)
#define DdxAlignLogical_	GetAlign_(D_dxAlignLogical)
#define DdxAlignLuminance_	GetAlign_(D_dxAlignLuminance)
#define DdxAlignMatchHistogram_ GetAlign_(D_dxAlignMatchHistogram)
#define DdxAlignMath_		GetAlign_(D_dxAlignMath)
#define DdxAlignMirror_		GetAlign_(D_dxAlignMirror)
#define DdxAlignPoint_		GetAlign_(D_dxAlignPoint)
#define DdxAlignStatsPoint_	GetAlign_(D_dxAlignStatsPoint)
#define DdxAlignRotate_		GetAlign_(D_dxAlignRotate)
#define DdxAlignScale_		GetAlign_(D_dxAlignScale)
#define DdxAlignTranslate_	GetAlign_(D_dxAlignTranslate)
#define DdxAlignDecodeDct_	GetAlign_(D_dxAlignDecodeDct)
#define DdxAlignEncodeDct_	GetAlign_(D_dxAlignEncodeDct)
#define DdxAlignDecodeG4_	GetAlign_(D_dxAlignDecodeG4)
#define DdxAlignEncodeG4_	GetAlign_(D_dxAlignEncodeG4)
#define DdxAlignDefault_	GetAlign_(D_dxAlignDefault)
#define	DdxAlignExport_		GetAlign_(D_dxAlignExport)

/*****************************************************************************
**
**  Mi Access Macros
**
**  FUNCTIONAL DESCRIPTION:
**
**	These macros provide access to the "atomic" DDX functions provided
**      by a Machine Independent layer of the DDX.
**
*****************************************************************************/
    /*
    **  Mi function access macro
    **  This macro calls an arbitrary Mi routine through the
    **  xieMiProc dispatch table.
    **
    **  func = the Mi function index from this file.
    **
    */
externalref card32 (*xieMiProc[])();
#define CallMi_(func) (*(xieMiProc)[func])
    	/*	 
        ** Image functions 
        */	 
#define MiArea_ CallMi_(D_dxArea)
#define MiStatsArea_ CallMi_(D_dxStatsArea)
#define MiArith_ CallMi_(D_dxArith)
#define MiHistogram_ CallMi_(D_dxHistogram)
#define MiChromeCom_ CallMi_(D_dxChromeCom)
#define MiChromeSep_ CallMi_(D_dxChromeSep)
#define MiCompare_ CallMi_(D_dxCompare)
#define MiConstrain_ CallMi_(D_dxConstrain)
#define MiCrop_ CallMi_(D_dxCrop)
#define MiDither_ CallMi_(D_dxDither)
#define MiFill_ CallMi_(D_dxFill)		
#define MiLogical_ CallMi_(D_dxLogical)
#define MiLuminance_ CallMi_(D_dxLuminance)
#define MiMatchHistogram_ CallMi_(D_dxMatchHistogram)
#define MiMath_ CallMi_(D_dxMath)			
#define MiMirror_ CallMi_(D_dxMirror)		
#define MiPoint_ CallMi_(D_dxPoint)
#define MiStatsPoint_ CallMi_(D_dxStatsPoint)
#define MiRotate_ CallMi_(D_dxRotate)
#define MiScale_ CallMi_(D_dxScale)
#define MiTranslate_ CallMi_(D_dxTranslate)	
    /*	 
    **  Mi specific functions
    */	 
#define MiInit_ CallMi_(D_dxInit)
#define MiReset_ CallMi_(D_dxReset)
#define MiOptimizePipe_ CallMi_(D_dxOptimizePipe)
    /*	 
    **  MI software functions
    */	 
#define MiConvert_ CallMi_(D_dxConvert)
#define MiCopy_ CallMi_(D_dxCopy)
#define MiGetTyp_ CallMi_(D_dxGetTyp)
#define MiCopyBits_ CallMi_(D_dxCopyBits)	
#define MiResolveUdp_ CallMi_(D_dxResolveUdp)
    /*	 
    **  Memory management routines
    */	 
#define MiCalloc_ CallMi_(D_dxCalloc)
#define MiCallocBits_ (unsigned char *)CallMi_(D_dxCallocBits)
#define MiCfree_ CallMi_(D_dxCfree)
#define MiFree_ CallMi_(D_dxFree)
#define MiFreeBits_ (unsigned char *)CallMi_(D_dxFreeBits)
#define MiMalloc_ CallMi_(D_dxMalloc)
#define MiMallocBits_ (unsigned char *)CallMi_(D_dxMallocBits)
#define MiRealloc_ CallMi_(D_dxRealloc)
#define MiReallocBits_ (unsigned char *)CallMi_(D_dxReallocBits)
    /*	 
    **  Pipeline support routines
    */	 
#define MiCreatePipeline_ (Pipe)CallMi_(D_dxCreatePipeline)
#define MiDestroyPipeline_ (int)CallMi_(D_dxDestroyPipeline)
#define MiPipeHasYielded_ (int)CallMi_(D_dxPipeHasYielded)
#define MiInitiatePipeline_ (int)CallMi_(D_dxInitiatePipeline)
#define MiResumePipeline_ (int)CallMi_(D_dxResumePipeline)
#define MiFlushPipeline_ (int)CallMi_(D_dxFlushPipeline)
#define MiAbortPipeline_ (int)CallMi_(D_dxAbortPipeline)
    /*	 
    **  Pipeline resource manager routines
    */	 
#define MiRmAllocData_ (PipeDataPtr)CallMi_(D_dxRmAllocData)		
#define MiRmAllocDataDesc_ (PipeDataPtr)CallMi_(D_dxRmAllocDataDesc)
#define MiRmAllocPermDesc_ (PipeDataPtr)CallMi_(D_dxRmAllocPermDesc)
#define MiRmAllocRefDesc_ (PipeDataPtr)CallMi_(D_dxRmAllocRefDesc)
#define MiRmBestDType_ (char)CallMi_(D_dxRmBestDType)
#define MiRmCreateDrain_ (PipeDrainPtr)CallMi_(D_dxRmCreateDrain)	
#define MiRmCreateSink_ (PipeSinkPtr)CallMi_(D_dxRmCreateSink)
#define MiRmDeallocData_ (PipeDataPtr)CallMi_(D_dxRmDeallocData)
#define MiRmDeallocDataDesc_ (PipeDataPtr)CallMi_(D_dxRmDeallocDataDesc)
#define MiRmDestroyDrain_ (PipeDrainPtr)CallMi_(D_dxRmDestroyDrain)
#define MiRmDestroySink_ (PipeSinkPtr)CallMi_(D_dxRmDestroySink)	
#define MiRmGetData_ (PipeDataPtr)CallMi_(D_dxRmGetData)
#define MiRmGetDType_ (int)CallMi_(D_dxRmGetDType)
#define MiRmGetMaxQuantum_ (int)CallMi_(D_dxRmGetMaxQuantum)
#define MiRmGetQuantum_ (int)CallMi_(D_dxRmGetQuantum)
#define MiRmHasQuantum_ (int)CallMi_(D_dxRmHasQuantum)
#define MiRmInitializePort_ (int)CallMi_(D_dxRmInitializePort)
#define MiRmMergeBuffers_ (PipeDataPtr)CallMi_(D_dxRmMergeBuffers)
#define MiRmObtainQuantum_ (PipeDataPtr)CallMi_(D_dxRmObtainQuantum)
#define MiRmPutData_ (int)CallMi_(D_dxRmPutData)
#define MiRmSetDType_ CallMi_(D_dxRmSetDType)
#define MiRmSetQuantum_ CallMi_(D_dxRmSetQuantum)
#define MiRmSplitBuffers_ (PipeDataPtr)CallMi_(D_dxRmSplitBuffers)
    /*	 
    **  Pipe scheduling routines
    */	 
#define MiSchedProcessComplete_ (PipeElementCtxPtr)CallMi_(D_dxSchedProcessComplete)
#define MiSchedProcessReady_ (PipeElementCtxPtr)CallMi_(D_dxSchedProcessReady)
#define MiSchedOkToRun_ (int)CallMi_(D_dxSchedOkToRun)
    /*	 
    ** Change list routines
    */	 
#define MiBuildChangeList_ CallMi_(D_dxBuildChangeList)
#define MiPutRuns_ CallMi_(D_dxPutRuns)	
#define MiFfsLong_ CallMi_(D_dxFfsLong)
#define MiFfcLong_ CallMi_(D_dxFfcLong)
    /*	 
    **  Additions
    */	 
#define MiFillRegion_ CallMi_(D_dxFillRegion)
#define MiConvertCpp_ CallMi_(D_dxConvertCpp)
#define MiCopyCpp_ CallMi_(D_dxCopyCpp)
#define MiEncodeG4_ CallMi_(D_dxEncodeG4)
#define MiEncodeDct_ CallMi_(D_dxEncodeDct)
#define MiDecodeG4_ CallMi_(D_dxDecodeG4)
#define MiDecodeDct_ CallMi_(D_dxDecodeDct)
#define MiPosCl_ CallMi_(D_dxPosCl)
#define MiCheckLicense_ CallMi_(D_dxCheckLicense)
    /*	 
    **  More resource manager
    */
#define MiRmAllocCmpDataDesc_  CallMi_(D_dxRmAllocCmpDataDesc)
#define MiRmSetAlignment_  CallMi_(D_dxRmSetAlignment)
#define MiRmCreateDataDesc_ (PipeDataPtr)CallMi_(D_dxRmCreateDataDesc)
#define MiCreatePipeCtx_ (PipeElementCtxPtr)CallMi_(D_dxCreatePipeCtx)
#define MiUsingCL_       CallMi_(D_dxUsingCL)
#define MiCmpPreferred_	CallMi_(D_dxCmpPreferred)
#define MiRmAllocDataArray_ CallMi_(D_dxRmAllocDataArray)	
#define MiRmAllocCmpPermDesc_ (PipeDataPtr)CallMi_(D_dxRmAllocCmpPermDesc)
#define MiSchedProcessNoInput_ (PipeElementCtxPtr)CallMi_(D_dxSchedProcessNoInput)

/*****************************************************************************
**
**  Pipeline Create Access Macros
**
**  FUNCTIONAL DESCRIPTION:
**
**	These macros provide access to the "pipeline create" DDX functions 
**	provided by a Machine Independent layer of the DDX.
**
*****************************************************************************/
    /*
    **  Pipe create function access macro
    **  This macro calls a create function through the 
    **  xieMiSetup dispatch table.
    **
    **  func = the setup function index from this file.
    **
    */
externalref card32 (*xieMiSetup[])();
#define CallMiCreate_(func) (*(xieMiSetup)[func])
    /*	 
    **  Image processing pipe element creates
    */	 
#define MiCreateArea_ CallMiCreate_(D_dxCreateArea)
#define MiCreateArith_ CallMiCreate_(D_dxCreateArith)
#define MiCreateChromeCom_ CallMiCreate_(D_dxCreateChromeCom)
#define MiCreateChromeSep_ CallMiCreate_(D_dxCreateChromeSep)
#define MiCreateCompare_ CallMiCreate_(D_dxCreateCompare)
#define MiCreateConstrain_ CallMiCreate_(D_dxCreateConstrain)
#define MiCreateCrop_ CallMiCreate_(D_dxCreateCrop)
#define MiCreateDither_ CallMiCreate_(D_dxCreateDither)
#define MiCreateFill_ CallMiCreate_(D_dxCreateFill)
#define MiCreateLogical_ CallMiCreate_(D_dxCreateLogical)
#define MiCreateLuminance_ CallMiCreate_(D_dxCreateLuminance)
#define MiCreateMatchHistogram_ CallMiCreate_(D_dxCreateMatchHistogram)
#define MiCreateMath_ CallMiCreate_(D_dxCreateMath)
#define MiCreateMirror_ CallMiCreate_(D_dxCreateMirror)
#define MiCreatePoint_ CallMiCreate_(D_dxCreatePoint)
#define MiCreateRotate_ CallMiCreate_(D_dxCreateRotate)
#define MiCreateScale_ CallMiCreate_(D_dxCreateScale)
#define MiCreateTranslate_ CallMiCreate_(D_dxCreateTranslate)
    /*	 
    **  Compression creates, used by Transport
    */	 
#define MiCreateDecodeDct_ CallMiCreate_(D_dxCreateDecodeDct)
#define MiCreateEncodeDct_ CallMiCreate_(D_dxCreateEncodeDct)
#define MiCreateDecodeG4_ CallMiCreate_(D_dxCreateDecodeG4)
#define MiCreateEncodeG4_ CallMiCreate_(D_dxCreateEncodeG4)

/*
**  This "endif" MUST be the last line of this file.
*/
#endif
/* end of XIEDDX -- NO DEFINITIONS ALLOWED BEYOND THIS POINT */
