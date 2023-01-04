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
Copyright 1989,1990 by Digital Equipment Corporation, Maynard, Massachusetts,
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

/*****************************************************************************
**
**  FACILITY:
**
**      X Image Extension
**	Sample Machine Independant DDX
**
**  ABSTRACT:
**
**	This module contains the connection to the Machine Independent(MI)
**	software that is the base of the XIE server. Another module, SmiDdx.c,
**	contains the connection to the DDX version of the base.
**
**	The connection consists of:
**	    - MI function dispatch table
**	    - MI pipeline CREATE dispatch table
**	    - MI image array alignment dispatch table
**
**  ENVIRONMENT:
**
**	VAX/VMS V5.x >
**	ULTRIX  V4.0 >
**
**  AUTHOR(S):
**
**      Richard D. Hennessy
**	Robert NC Shelley
**
**  CREATION DATE:
**
**      October 12, 1990
**
**  MODIFICATION HISTORY:
**
*****************************************************************************/

/*
**  Include files
*/
#include <stdio.h>
#include "XieDdx.h"
#include "XieUdpDef.h"

#include "SmiArea.h"
#include "SmiArithmetic.h"
#include "SmiChangeList.h"
#include "SmiChromeCom.h"
#include "SmiChromeSep.h"
#include "SmiCompare.h"
#include "SmiConstrain.h"
#include "SmiConvert.h"
#include "SmiCrop.h"
#include "SmiDctDef.h"
#include "SmiDecodeDct.h"
#include "SmiDecodeG4.h"
#include "SmiDither.h"
#include "SmiEncodeDct.h"
#include "SmiEncodeG4.h"
#include "SmiFfx.h"
#include "SmiFill.h"
#include "SmiHistogram.h"
#include "SmiLogical.h"
#include "SmiLuminance.h"
#include "SmiMatchHistogram.h"
#include "SmiMath.h"
#include "SmiMemMgt.h"
#include "SmiMirror.h"
#include "SmiPipe.h"
#include "SmiPipeRsm.h"
#include "SmiPipeSched.h"
#include "SmiPoint.h"
#include "SmiRotate.h"
#include "SmiScale.h"
#include "SmiStats.h"
#include "SmiTranslate.h"
#include "SmiUtil.h"

/*
**  Table of contents
*/

/*	 
**  Externs
*/
/*
**	Local Storage
*/

/*****************************************************************************
**
**  xieMiProc -
**
**  FUNCTIONAL DESCRIPTION:
**
**      The miProc dispatch table is utilized by MI callers to access MI
**	routines without having to know the explicit MI name of the routine.
**	This independence will allow substitution of an MI layer in the XIE
**	server without code changes in the remainder of the server..
**
*****************************************************************************/
    /*	 
    **  MI dispatch table
    */	 
externaldef(xieMiProc) unsigned long (*xieMiProc[])() = {
	(Fcard32)SmiArea,		/* 0	    DDX Atomic operations   */
	(Fcard32)SmiStatsArea,
	(Fcard32)SmiArithmetic,
	(Fcard32)SmiHistogram,
	(Fcard32)SmiConvert,
	(Fcard32)SmiConvert,		/* 5 */
	(Fcard32)SmiCompare,
	(Fcard32)SmiConstrain,
	(Fcard32)SmiCopy,
	(Fcard32)SmiDither,
	(Fcard32)SmiFill,		/* 10 */
	(Fcard32)SmiLogical,
	(Fcard32)SmiLuminance,
	(Fcard32)SmiMatchHistogram,
	(Fcard32)SmiMath,	
	(Fcard32)SmiMirror,		/* 15 */
	(Fcard32)SmiPoint,
	(Fcard32)SmiStatsPoint,	
	(Fcard32)SmiRotate,	
	(Fcard32)SmiScale,	
	(Fcard32)SmiTranslate,		/* 20 */

	(Fcard32)RmOwnData,	/* 21..32 -- was compression handlers */
	NULL,
	NULL,
	NULL,
	NULL,	/* 25 */
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,	/* 30 */
	NULL,
	NULL,
	NULL,				/* Only defined in the DDX table */
	NULL,
	NULL,				/* 35 */

	(Fcard32)SmiConvert,		/* MI Support routines	    */
	(Fcard32)SmiCopy,
	(Fcard32)SmiGetTyp,
	(Fcard32)SmiCopyBits,	
	(Fcard32)SmiResolveUdp,	        /* 40 */

#ifdef  CLIENT
        (Fcard32)XieCalloc,                     /* CLIENT needs to use Xie  */
        (Fcard32)XieCallocBits,
        (Fcard32)XieCfree,
        (Fcard32)XieFree,
        (Fcard32)XieFreeBits,                   /* 45 */
        (Fcard32)XieMalloc,
        (Fcard32)XieMallocBits,
        (Fcard32)XieRealloc,
        (Fcard32)XieReallocBits,
#else
	(Fcard32)SmiCalloc,		/* MI Memory mgt routines	    */
	(Fcard32)SmiCallocBits,	
	(Fcard32)SmiCfree,
	(Fcard32)SmiFree,	
	(Fcard32)SmiFreeBits,           /* 45 */
	(Fcard32)SmiMalloc,	
	(Fcard32)SmiMallocBits,	
	(Fcard32)SmiRealloc,	
	(Fcard32)SmiReallocBits,		
#endif
	(Fcard32)SmiCreatePipeline,	/* 50 Pipeline routines	    */
	(Fcard32)SmiDestroyPipeline,
	(Fcard32)SmiPipeHasYielded,
	(Fcard32)SmiInitiatePipeline,	
	(Fcard32)SmiResumePipeline,
	(Fcard32)SmiFlushPipeline,      /* 55 */
	(Fcard32)SmiAbortPipeline,

	(Fcard32)RmAllocData,		/* Pipe rsrc mgr routines*/
	(Fcard32)RmAllocDataDesc,
	(Fcard32)RmAllocPermDesc,
	(Fcard32)RmAllocRefDesc,	/* 60 */
	(Fcard32)RmBestDType,	
	(Fcard32)RmCreateDrain,		
	(Fcard32)RmCreateSink,
	(Fcard32)RmDeallocData,	
	(Fcard32)RmDeallocDataDesc,     /* 65 */
	(Fcard32)RmDestroyDrain,
	(Fcard32)RmDestroySink,	
	(Fcard32)RmGetData,	
	(Fcard32)RmGetDType,	
	(Fcard32)RmGetMaxQuantum,       /* 70 */
	(Fcard32)RmGetQuantum,	
	(Fcard32)RmHasQuantum,
	(Fcard32)RmInitializePort,
	NULL,
	(Fcard32)RmMergeBuffers,	/* 75 */
	(Fcard32)RmObtainQuantum,
	(Fcard32)RmPutData,	
	(Fcard32)RmSetDType,	
	(Fcard32)RmSetQuantum,
	(Fcard32)RmSplitBuffers,	/* 80 */

	(Fcard32)SchedProcessComplete,	/* Pipe schedule funcs */
	(Fcard32)SchedProcessReady,
	(Fcard32)SchedOkToRun,

	(Fcard32)MiBuildChangeList,	/* Change list funcs   */
	(Fcard32)MiPutRuns,		/* 85 */
	(Fcard32)MiFfsLong,
	(Fcard32)MiFfcLong,

	(Fcard32)SmiFillRegion,	
	(Fcard32)SmiConvertCpp,	
	(Fcard32)SmiCopyCpp,	       /* 90 */
	(Fcard32)SmiEncodeG4,
	(Fcard32)SmiEncodeDct,
	(Fcard32)SmiDecodeG4,
	(Fcard32)SmiDecodeDct,
	(Fcard32)SmiPosCl,		/* 95 */
	NULL,
	(Fcard32)RmAllocCmpDataDesc,    /* More RM functions        */
        (Fcard32)RmSetAlignment,
	(Fcard32)RmCreateDataDesc,

	(Fcard32)SmiCreatePipeCtx,	/* 100 */
	(Fcard32)SmiUsingCL,
	(Fcard32)SmiCmpPreferred,
	(Fcard32)RmAllocDataArray,
	(Fcard32)RmAllocCmpPermDesc,
	(Fcard32)SchedProcessNoInput,  /* 105 */
	NULL
	}; 

/*****************************************************************************
**
**  XieMiSetup -
**
**  FUNCTIONAL DESCRIPTION:
**
**      XieMiSetup is the DDX dispatch table used to find MI versions of
**	the Create Pipe Element routines.
**
**	Routines in this table should be called through the macros
**      defined in XieDdx.h.
**
*****************************************************************************/
    /*
    **	MI process dispatch table
    */
externaldef(xieMiSetup) card32 (*xieMiSetup[])() = {
	(Fcard32)SmiCreateArea,		/* 0 */
	(Fcard32)SmiCreateArithmetic,   
	(Fcard32)SmiCreateChromeCom,
	(Fcard32)SmiCreateChromeSep,
	(Fcard32)SmiCreateCompare,
	(Fcard32)SmiCreateConstrain,   /* 5 */
	(Fcard32)SmiCreateCrop,
	(Fcard32)SmiCreateDither,
	(Fcard32)SmiCreateFill,
	(Fcard32)SmiCreateLogical,
	(Fcard32)SmiCreateLuminance,   /* 10 */
	(Fcard32)SmiCreateMatchHistogram,
	(Fcard32)SmiCreateMath,
	(Fcard32)SmiCreateMirror,
	(Fcard32)SmiCreatePoint,
	(Fcard32)SmiCreateRotate,	/* 15 */
	(Fcard32)SmiCreateScale,
	(Fcard32)SmiCreateTranslate,
	(Fcard32)SmiCreateDecodeDct,
	(Fcard32)SmiCreateEncodeDct,
	(Fcard32)SmiCreateDecodeG4,
	(Fcard32)SmiCreateEncodeG4,
	NULL
	};

/*****************************************************************************
**
**  XieMiAlign -
**
**  FUNCTIONAL DESCRIPTION:
**
**      XieMiAlign is the DDX dispatch table used to call the
**      MI versions of the image array alignment functions.
**
**      A NULL value is used by alignment logic throughout the XIE server
**      to indicate "don't care", "no alignment needed".
**
**      If alignment is needed, an alignment function must entered in this
**      table in the operator/pipeline element named slot. 
**
**      These functions should be called using the macros defined in XieDdx.h.
**
*****************************************************************************/

externaldef(xieMiAlign) void (*xieMiAlign[])() = {
        NULL,                   /* 0 */
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,                   /* 5 */
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,                   /* 10 */
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,                   /* 15 */
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,                   /* 20 */
        NULL,
        NULL,
        NULL,
        NULL
        };
/* end of module SmiMi.c */
