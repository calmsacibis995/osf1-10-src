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

/*******************************************************************************
**  Copyright (c) Digital Equipment Corporation, 1990 All Rights Reserved.
**  Unpublished rights reserved under the copyright laws of the United States.
**  The software contained on this media is proprietary to and embodies the
**  confidential technology of Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and media is authorized only
**  pursuant to a valid written license from Digital Equipment Corporation.

**  RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the U.S.
**  Government is subject to restrictions as set forth in Subparagraph
**  (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
**
**  Derived from work bearing the following copyright and permissions:
**
**  Copyright 1989 by Digital Equipment Corporation, Maynard, Massachusetts,
**  and the Massachusetts Institute of Technology, Cambridge, Massachusetts.
**  
**                          All Rights Reserved
**  
**  Permission to use, copy, modify, and distribute this software and its 
**  documentation for any purpose and without fee is hereby granted, 
**  provided that the above copyright notice appear in all copies and that
**  both that copyright notice and this permission notice appear in 
**  supporting documentation, and that the names of Digital or MIT not be
**  used in advertising or publicity pertaining to distribution of the
**  software without specific, written prior permission.  
**  
**  DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
**  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
**  DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
**  ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
**  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
**  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
**  SOFTWARE.
**  
*******************************************************************************/

/*****************************************************************************
**
**  FACILITY:
**
**      X Image Extension
**	Digital Machine Independant DDX
**
**  ABSTRACT:
**
**	This module implements the connection between the
**      XIE server DIX, and the XIE server DMI (Digital Machine
**      Independent) DDX.  This interface is used when the 
**      DIX is layered directly on top of the DMI DDX.  If 
**      the DIX is layered on a hardware specific DDX, then the
**      interface in the module DmiMi is used by the hardware
**      specific DDX to access DMI services.
**
**	The connection consists of:
**	    - dispatch tables
**	    - DDX supplied Init Reset and OptimizePipe routines
**          - Default DDX image array alignment function
**
**  ENVIRONMENT:
**
**	VAX/VMS V5.3 >
**	ULTRIX  V4.0 >
**
**  AUTHOR(S):
**
**      Gary L. Grebus 
**
**  CREATION DATE:
**
**      Tue Oct  9 09:23:16 1990
**
**  MODIFICATION HISTORY:
**
*****************************************************************************/

/*
**  Include files
*/
#ifdef  VMS
#include <ssdef>
#endif
#include <stdio.h>
#include <XieDdx.h>
#include <XieUdpDef.h>
    /*	 
    **  Include needed DDX, SMI, MI entry points
    */	 
#include <SmiArea.h>
#include <SmiArithmetic.h>
#include "DmiChangeList.h"
#include <SmiChromeCom.h>
#include <SmiChromeSep.h>
#include <SmiCompare.h>
#include <SmiConstrain.h>
#include "DmiConvert.h"
#include <SmiCrop.h>

#ifndef XIESMI
#define XIESMI		/* Use the XIESMI-style definitions in the DCT code */
#endif
#include <SmiDctDef.h>	/* Must be before DCT files			    */
#include <SmiDecodeDct.h>
#include <SmiEncodeDct.h>
#undef XIESMI
#include "DmiDecodeG4.h"
#include <SmiEncodeG4.h>
#include "DmiExtendInstruct.h"
#include <SmiDither.h>
#include "DmiFill.h"
#include <SmiHistogram.h>
#include <SmiLogical.h>
#include <SmiLuminance.h>
#include <SmiMatchHistogram.h>
#include <SmiMath.h>
#include <SmiMemMgt.h>
#include <SmiMirror.h>
#include <SmiPipeRsm.h>
#include <SmiPipeSched.h>
#include <SmiPipe.h>
#include "DmiPoint.h"
#include <SmiRotate.h>
#include "DmiScale.h"
#include <SmiTranslate.h>
#include <SmiStats.h>
#include <SmiUtil.h>
#include "DmiUtil.h"

/*
**  Table of contents
*/
int	DmiAlignDefault();	/* DDX image array alignment		    */
int	DmiInitialize();	/* DDX Init				    */
int	DmiOptimizePipe();	/* DDX specific pipe optimizer		    */
int	DmiReset();		/* DDX Reset				    */


/*
**  MACRO definitions
*/

/*
**  Equated Symbols
*/
#ifdef VMS
#define XIE_PRODUCT_NAME "IMAGE-XIE-SERV-V"
#define XIE_PRODUCER "DEC"
#define XIE_PRODUCT_RELEASE_DATE "05-AUG-1991"
#define XIE_VERSION_MAJOR  1
#define XIE_VERSION_MINOR  0
#endif

/*
**  External References
*/

/*
**	Local Storage
*/

/*****************************************************************************
**  xieDdxProc -
**
**  FUNCTIONAL DESCRIPTION:
**
**      XieDdxProc is the DDX dispatch table, used by all callers of 
**	functions implemented by the DDX.
**
**	This table should be accessed using the macros defined in XieDdx.h.
**
*****************************************************************************/
externaldef(xieDdxProc) unsigned long (*xieDdxProc[])() = {
	(Fcard32)SmiArea,			/* 0 Protocol operations   */
	(Fcard32)SmiStatsArea,
	(Fcard32)SmiArithmetic,
	(Fcard32)SmiHistogram,
	(Fcard32)DmiConvert,
	(Fcard32)DmiConvert,			/* 5 */
	(Fcard32)SmiCompare,
	(Fcard32)SmiConstrain,
	(Fcard32)DmiCopy,
	(Fcard32)SmiDither,
	(Fcard32)DmiFill,			/* 10 */
	(Fcard32)SmiLogical,
	(Fcard32)SmiLuminance,
	(Fcard32)SmiMatchHistogram,
	(Fcard32)SmiMath,	
	(Fcard32)SmiMirror,			/* 15 */
	(Fcard32)DmiPoint,
	(Fcard32)SmiStatsPoint,	
	(Fcard32)SmiRotate,	
	(Fcard32)DmiScale,	
	(Fcard32)SmiTranslate,			/* 20 */

	(Fcard32)RmOwnData,	/* 21..32 -- was compression handlers */
	NULL,
	NULL,
	NULL,
	NULL,					/* 25 */
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,					/* 30 */
	NULL,
	NULL,

	(Fcard32)DmiInitialize,			/*  DDX specific functions  */
	(Fcard32)DmiReset,
	(Fcard32)DmiOptimizePipe,	        /* 35 */

	(Fcard32)DmiConvert,			/* MI Support routines	    */
	(Fcard32)DmiCopy,
	(Fcard32)DmiGetTyp,
	(Fcard32)DmiCopyBits,			
	(Fcard32)DmiResolveUdp,		       /* 40 */
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
	(Fcard32)SmiCalloc,			/* Memory mgt routines	    */
	(Fcard32)SmiCallocBits,	
	(Fcard32)SmiCfree,	
	(Fcard32)SmiFree,
	(Fcard32)SmiFreeBits,			/* 45 */
	(Fcard32)SmiMalloc,	
	(Fcard32)SmiMallocBits,	
	(Fcard32)SmiRealloc,	
	(Fcard32)SmiReallocBits,
#endif
	(Fcard32)SmiCreatePipeline,		/* 50 Pipeline routines	    */
	(Fcard32)SmiDestroyPipeline,
	(Fcard32)SmiPipeHasYielded,
	(Fcard32)SmiInitiatePipeline,
	(Fcard32)SmiResumePipeline,
	(Fcard32)SmiFlushPipeline,		/* 55 */
	(Fcard32)SmiAbortPipeline,

	(Fcard32)RmAllocData,			/* Pipe rsrc mgr routines*/
	(Fcard32)RmAllocDataDesc,
	(Fcard32)RmAllocPermDesc,
	(Fcard32)RmAllocRefDesc,		/* 60 */
	(Fcard32)RmBestDType,	
	(Fcard32)RmCreateDrain,	
	(Fcard32)RmCreateSink,
	(Fcard32)RmDeallocData,	
	(Fcard32)RmDeallocDataDesc,		/* 65 */
	(Fcard32)RmDestroyDrain,
	(Fcard32)RmDestroySink,	
	(Fcard32)RmGetData,	
	(Fcard32)RmGetDType,	
	(Fcard32)RmGetMaxQuantum,		/* 70 */
	(Fcard32)RmGetQuantum,
	(Fcard32)RmHasQuantum,
	(Fcard32)RmInitializePort,
	NULL,
	(Fcard32)RmMergeBuffers,		/* 75 */
	(Fcard32)RmObtainQuantum,
	(Fcard32)RmPutData,	
	(Fcard32)RmSetDType,	
	(Fcard32)RmSetQuantum,
	(Fcard32)RmSplitBuffers,		/* 80 */

	(Fcard32)SchedProcessComplete,		/* Pipe schedule funcs */
	(Fcard32)SchedProcessReady,
	(Fcard32)SchedOkToRun,

	(Fcard32)DmiBuildChangeList,		/* Change list funcs   */
	(Fcard32)DmiPutRuns,			/* 85 */
	(Fcard32)_DmiFfsLong,
	(Fcard32)_DmiFfcLong,

	(Fcard32)DmiFillRegion,			/*  Additions		    */
	(Fcard32)DmiConvertCpp,
	(Fcard32)DmiCopyCpp,			/* 90 */
	(Fcard32)SmiEncodeG4,
	(Fcard32)SmiEncodeDct,
	(Fcard32)DmiDecodeG4,
	(Fcard32)SmiDecodeDct,
	(Fcard32)DmiPosCl,			/* 95 */
	(Fcard32)DmiCheckLicense,
				  		/* More RM functions        */
        (Fcard32)RmAllocCmpDataDesc,
        (Fcard32)RmSetAlignment,
	(Fcard32)RmCreateDataDesc,

	(Fcard32)SmiCreatePipeCtx,		/* 100 */
        (Fcard32)DmiUsingCL,
        (Fcard32)DmiCmpPreferred,
        (Fcard32)RmAllocDataArray,
	(Fcard32)RmAllocCmpPermDesc,
	(Fcard32)SchedProcessNoInput,	      /* 105 */
	NULL
	};

/*****************************************************************************
**  XieDdxSetup -
**
**  FUNCTIONAL DESCRIPTION:
**
**      XieDdxSetup is the DDX dispatch table used to call the DDX Create
**	Pipe Element routines.
**
**	These routines should be called using the macros defined in XieDdx.h.
**
*****************************************************************************/
externaldef(xieDdxSetup) card32 (*xieDdxSetup[])() = {
	(Fcard32)SmiCreateArea,		/* 0 */
	(Fcard32)SmiCreateArithmetic,
	(Fcard32)SmiCreateChromeCom,
	(Fcard32)SmiCreateChromeSep,	
	(Fcard32)SmiCreateCompare,
	(Fcard32)SmiCreateConstrain,	/* 5 */
	(Fcard32)SmiCreateCrop,
	(Fcard32)SmiCreateDither,
	(Fcard32)DmiCreateFill,
	(Fcard32)SmiCreateLogical,	
	(Fcard32)SmiCreateLuminance,    /* 10 */
	(Fcard32)SmiCreateMatchHistogram,
	(Fcard32)SmiCreateMath,
	(Fcard32)SmiCreateMirror,
	(Fcard32)DmiCreatePoint,
	(Fcard32)SmiCreateRotate,	/* 15 */
	(Fcard32)DmiCreateScale,
	(Fcard32)SmiCreateTranslate,
	(Fcard32)SmiCreateDecodeDct,
	(Fcard32)SmiCreateEncodeDct,
	(Fcard32)DmiCreateDecodeG4,	/* 20 */
	(Fcard32)SmiCreateEncodeG4,
	NULL
	};

/*****************************************************************************
**  XieDdxAlign -
**
**  FUNCTIONAL DESCRIPTION:
**
**      XieDdxAlign is the DDX dispatch table used to call the
**      DDX image array alignment functions.
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
externaldef(xieDdxAlign) card32 (*xieDdxAlign[])() = {
        (Fcard32)DmiAlignDefault,		/* 0 */
        (Fcard32)DmiAlignDefault,
        (Fcard32)DmiAlignDefault,
        (Fcard32)DmiAlignDefault,
        (Fcard32)DmiAlignDefault,
        (Fcard32)DmiAlignDefault,		/* 5 */
        (Fcard32)DmiAlignDefault,
        (Fcard32)DmiAlignDefault,
        (Fcard32)DmiAlignDefault,
        (Fcard32)DmiAlignDefault,
        (Fcard32)DmiAlignDefault,		/* 10 */
        (Fcard32)DmiAlignDefault,
        (Fcard32)DmiAlignDefault,
        (Fcard32)DmiAlignDefault,
        (Fcard32)DmiAlignDefault,
        (Fcard32)DmiAlignDefault,		/* 15 */
        (Fcard32)DmiAlignDefault,
        (Fcard32)DmiAlignDefault,
        (Fcard32)DmiAlignDefault,
        (Fcard32)DmiAlignDefault,
        (Fcard32)DmiAlignDefault,		/* 20 */
        (Fcard32)DmiAlignDefault,
        (Fcard32)DmiAlignDefault,
        (Fcard32)DmiAlignDefault,
        NULL
        };

/*******************************************************************************
**  DmiAlignDefault -
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function performs DDX specific image array alignment adjustments
**      on the passed Udp's pixel stride and scanline stride.
**
**  FORMAL PARAMETERS:
**
**      udp  ==  ptr to the Udp which will be aligned
**
**  FUNCTION VALUE:
**
**      status  ==  Success, if everything ok
**
*******************************************************************************/
int    DmiAlignDefault( udp )
UdpPtr  udp;
{
#define SCN_PAD	     32		    /* default: longword aligned scanlines  */
#define SCN_MSK	    (SCN_PAD-1)

    if( upScnStr_(udp) > 0 )
	{
	upScnStr_(udp) = upPxlStr_(udp) *  upWidth_(udp) + SCN_MSK & ~SCN_MSK;
	upArSize_(udp) = upScnStr_(udp) * upHeight_(udp) + upPos_(udp);
	}
    return( Success );
}

/*******************************************************************************
**  DmiInitialize -
**
**  FUNCTIONAL DESCRIPTION:
**
**	Perform DDX-specific initializations.
**      Since this is a software-only DDX, the only initialization
**      required is to perform the LMF license check.
**
**      The Dmi DDX is layered on top of the Smi DDX, so this routine
**      could be responsible for building the above tables at runtime.
**      But, since it is unlikely we'll ever need more than two levels
**      of Mi DDX, everything is hardwired.
**  
**  FORMAL PARAMETERS:
**  
**	None.
**
**  FUNCTION VALUE:
**  
**	0     = DDX init successful
**	non 0 = failure 
**	
*******************************************************************************/
int DmiInitialize() 
{
#ifdef VMS
    static char first_time = TRUE;

    if( first_time && getenv("XIE_DEBUG") != NULL )
	{   /*
	    **  It saves time to have the server mostly running before
	    **  invoking the debugger.
	    */
	first_time = FALSE;
	LIB$SIGNAL(SS$_DEBUG);
	}
#endif

#ifdef NEVER
    return ( DdxCheckLicense_( XIE_PRODUCT_NAME,
			       XIE_PRODUCER,
			       XIE_PRODUCT_RELEASE_DATE,
			       XIE_VERSION_MAJOR,
			       XIE_VERSION_MINOR ) );
#else
    return( Success );			/* No separate XIE license for now  */
#endif
}

/*******************************************************************************
**  DmiOptimizePipe -
**
**  FUNCTIONAL DESCRIPTION:
**
**	Perform DDx-specific XIE pipeline optimizations.
**  
**  FORMAL PARAMETERS:
**  
**	None.
**
**  FUNCTION VALUE:
**  
**	0     = Pipeline optimization successful
**	non 0 = failure 
**	
*******************************************************************************/
int DmiOptimizePipe() 
{
    return( Success );
}

/*******************************************************************************
**  DmiReset -
**
**  FUNCTIONAL DESCRIPTION:
**
**	Perform DDX-specific reset actions.
**	If an accelerator exists, get it too.
**  
**  FORMAL PARAMETERS:
**  
**	None.
**
**  FUNCTION VALUE:
**  
**	0     = DDX reset successful
**	non 0 = failure 
**	
*******************************************************************************/
int DmiReset() 
{
    return( Success );
}
/*  End of DmiDdx.c  */
