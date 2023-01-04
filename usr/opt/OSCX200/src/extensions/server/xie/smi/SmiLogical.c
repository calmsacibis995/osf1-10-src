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

/******************************************************************************
**++
**  FACILITY:
**
**      X Imaging Extension
**      Sample Machine Independant DDX
**
**  ABSTRACT:
**
**      This module implements the logical operator which performs
**	logical functions between two photomaps.
**
**  ENVIRONMENT:
**
**	VAX/VMS V5.3
**	ULTRIX  V3.1
**
**  AUTHORS:
**
**	Gary L. Grebus
**
**  CREATION DATE:     
**
**	Fri Apr 20 15:55:27 1990
**
*******************************************************************************/

#include <stdio.h>

#include "SmiLogical.h"

/*
**  Table of contents
*/
int 		SmiLogical();
int             SmiCreateLogical();

static int 	_LogicalAbort();
static int 	_LogicalInitialize();
static int 	_LogicalActivate();
static int 	_LogicalFlush();
static int 	_LogicalDestroy();

static UdpPtr   LogicalGetTempUdp();
static void     LogicalDestroyTempUdp();
static int	LogicalConvertScanline();
static int	LogicalStoreScanline();

static void 	LogicalAndII();
static void 	LogicalAndIC();
static void 	LogicalOrII();
static void 	LogicalOrIC();
static void 	LogicalXorII();
static void 	LogicalXorIC();
static void 	LogicalNotII();
static void 	LogicalNotIC();
static void 	LogicalShiftUpII();
static void 	LogicalShiftUpIC();
static void 	LogicalShiftDownII();
static void 	LogicalShiftDownIC();

/*
**  Equated Symbols
*/

/*
**  External References
*/

/*
**  Local storage
*/
    /*
    **  Logical Element Vector
    */
static PipeElementVector LogicalPipeElement = {
    NULL,				/* FLINK			    */
    NULL,				/* BLINK			    */
    sizeof(PipeElementVector),		/* Size				    */
    TypePipeElementVector,		/* Structure type		    */
    StypeLogicalElement,		/* Structure subtype		    */
    sizeof(LogicalPipeCtx),		/* Context block size		    */
    0,					/* No input flag		    */
    0,					/* Reserved flags		    */
    _LogicalInitialize,			/* Initialize entry		    */
    _LogicalActivate,			/* Activate entry		    */
    _LogicalFlush,			/* Flush entry			    */
    _LogicalDestroy,			/* Destroy entry		    */
    NULL				/* Abort entry logical		    */
    };

/* Dispatch tables to select the correct operation routine */
/* These must match the ordering of the XieK_xxxx constants in XieAppl.h */
static void (*LogicalTableII[])() = {
    NULL,				/* Constants start with 1 */
    LogicalAndII,
    LogicalOrII,
    LogicalXorII,
    LogicalNotII,
    LogicalShiftUpII,
    LogicalShiftDownII,
    };

static void (*LogicalTableIC[])() = {
    NULL,
    LogicalAndIC,
    LogicalOrIC,
    LogicalXorIC,
    LogicalNotIC,
    LogicalShiftUpIC,
    LogicalShiftDownIC,
    };

/*****************************************************************************
**  SmiCreateLogical
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function fills in a newly created context for a LOGICAL
**      pipeline element.
**
**  FORMAL PARAMETERS:
**
**      pipe    - Descriptor for pipe being built..
**      srcsnk1	- Pointer to first source sink
**      srcsnk2 - Pointer to second source sink (NULL if no second source)
**      constant- Array of "constant image" values (used when srcsnk2==NULL)
**      dstsnk  - Pointer to destination sink
**      idc     - Pointer to UDP describing the ROI or CPP (NULL if no IDC)
**      operator- Logical operation function code.
**
**  FUNCTION VALUE:
**
*****************************************************************************/
int 	    SmiCreateLogical(pipe,srcsnk1,srcsnk2,constant,dstsnk,idc,operator)
Pipe		     pipe;
PipeSinkPtr	     srcsnk1;
PipeSinkPtr	     srcsnk2;
unsigned int    constant[XieK_MaxComponents];
PipeSinkPtr	     dstsnk;
UdpPtr		     idc;
int	     	     operator;
{
    int i, dtype;
    LogicalPipeCtxPtr ctx = (LogicalPipeCtxPtr)
			DdxCreatePipeCtx_( pipe, &LogicalPipeElement, FALSE );
    if( !IsPointer_(ctx) ) return( (int) ctx );
    /*
    **	Copy parameters
    */
    LogSrcSnk1_(ctx) = srcsnk1;
    LogSrcSnk2_(ctx) = srcsnk2;
    LogDstSnk_(ctx)  = dstsnk;

    /* See if sink 1 contains word or byte data */
    for(i = 0; i < XieK_MaxComponents && IsPointer_(SnkUdpPtr_(srcsnk1,i)); i++)
	if( SnkLvl_(srcsnk1,i) <= 256 )
	    dtype = UdpK_DTypeBU;
	else
	    {
	    dtype = UdpK_DTypeWU;
	    break;
	    }
    if( dtype == UdpK_DTypeBU )
	/* See if destination sink contains word data */
	for(i=0; i<XieK_MaxComponents && IsPointer_(SnkUdpPtr_(dstsnk,i)); i++)
	    if( SnkLvl_(dstsnk,i) > 256 )
		{
		dtype = UdpK_DTypeWU;
		break;
		}
    if( IsPointer_(srcsnk2) )
	{
	LogFunc_(ctx) = LogicalTableII[operator];
	/* Intersection of two source operands */
	LogRoiX1_(ctx) = SnkX1_(srcsnk2,0) > SnkX1_(srcsnk1,0) ?
			 SnkX1_(srcsnk2,0) : SnkX1_(srcsnk1,0);
	LogRoiX2_(ctx) = SnkX2_(srcsnk2,0) < SnkX2_(srcsnk1,0) ?
	                 SnkX2_(srcsnk2,0) : SnkX2_(srcsnk1,0);
	LogRoiY1_(ctx) = SnkY1_(srcsnk2,0) > SnkY1_(srcsnk1,0) ?
	                 SnkY1_(srcsnk2,0) : SnkY1_(srcsnk1,0);
	LogRoiY2_(ctx) = SnkY2_(srcsnk2,0) < SnkY2_(srcsnk1,0) ?
	                 SnkY2_(srcsnk2,0) : SnkY2_(srcsnk1,0);
	if( dtype == UdpK_DTypeBU )
	    /* See if sink 2 contains word data */
	    for(i=0;i<XieK_MaxComponents&&IsPointer_(SnkUdpPtr_(srcsnk2,i));i++)
		if( SnkLvl_(srcsnk2,i) > 256 )
		    {
		    dtype = UdpK_DTypeWU;
		    break;
		    }
	/* Create a drain on sink 2, process a quantum of 1 scanline */
	LogSrcDrn2_(ctx) = DdxRmCreateDrain_( LogSrcSnk2_(ctx), -1 );
	if( !IsPointer_(LogSrcDrn2_(ctx)) ) return( (int) LogSrcDrn2_(ctx));
	DdxRmSetDType_( LogSrcDrn2_(ctx), dtype, 1<<dtype );
	DdxRmSetQuantum_( LogSrcDrn2_(ctx), 1 );
	}
    else 
	{					/* Using a constant "image" */
	LogFunc_(ctx) = LogicalTableIC[operator];
	for(i=0; i<XieK_MaxComponents && IsPointer_(SnkUdpPtr_(srcsnk1,i)); i++)
	    LogConst_(ctx,i) = constant[i];
	LogRoiX1_(ctx) = SnkX1_(srcsnk1,0);
	LogRoiX2_(ctx) = SnkX2_(srcsnk1,0);
	LogRoiY1_(ctx) = SnkY1_(srcsnk1,0);
	LogRoiY2_(ctx) = SnkY2_(srcsnk1,0);
	}
    LogCpp_(ctx) = IsPointer_(idc) && IsPointer_(idc->UdpA_Base) ? idc : NULL;

    if( IsPointer_(idc) )
	{
	/* Further restrict the area to be processed with the CPP/ROI  */
	LogRoiX1_(ctx) = idc->UdpL_X1 > LogRoiX1_(ctx) ?
			 idc->UdpL_X1 : LogRoiX1_(ctx);
	LogRoiX2_(ctx) = idc->UdpL_X2 < LogRoiX2_(ctx) ?
			 idc->UdpL_X2 : LogRoiX2_(ctx);
	LogRoiY1_(ctx) = idc->UdpL_Y1 > LogRoiY1_(ctx) ?
			 idc->UdpL_Y1 : LogRoiY1_(ctx);
	LogRoiY2_(ctx) = idc->UdpL_Y2 < LogRoiY2_(ctx) ?
			 idc->UdpL_Y2 : LogRoiY2_(ctx);
	}
    LogSrcData1_(ctx) = NULL;
    LogSrcData2_(ctx) = NULL;

    /* Create a drain on sink 1, process a quantum of 1 scanline */
    LogSrcDrn1_(ctx) = DdxRmCreateDrain_( LogSrcSnk1_(ctx), -1 );
    if( !IsPointer_(LogSrcDrn1_(ctx)) ) return( (int) LogSrcDrn1_(ctx));
    DdxRmSetDType_( LogSrcDrn1_(ctx), dtype, 1<<dtype );
    DdxRmSetQuantum_( LogSrcDrn1_(ctx), 1 );

    DdxRmSetDType_( LogDstSnk_(ctx), dtype, 1<<dtype );
    DdxRmSetQuantum_( LogDstSnk_(ctx), 1 );

    return( Success );
}					/* SmiCreateLogical */

/*****************************************************************************
**  _LogicalInitialize
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function initializes the LOGICAL pipeline element prior to
**	activation.
**
**  FORMAL PARAMETERS:
**
**      ctx - logical pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	_LogicalInitialize(ctx)
LogicalPipeCtxPtr	ctx;
{
    int	status  = DdxRmInitializePort_( CtxHead_(ctx), LogSrcDrn1_(ctx) );
    if( status == Success && LogSrcSnk2_(ctx) != NULL )
	status  = DdxRmInitializePort_( CtxHead_(ctx), LogSrcDrn2_(ctx) );
    if( status == Success )
	status  = DdxRmInitializePort_( CtxHead_(ctx), LogDstSnk_(ctx) );

    /* Start reading from the first drain */
    CtxInp_(ctx) = LogSrcDrn1_(ctx);

    return( status );
}

/*****************************************************************************
**  _LogicalActivate
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine reads the data from the source drain(s) and
**      performs the logical operations.
**
**  FORMAL PARAMETERS:
**
**      ctx - logical pipeline element context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _LogicalActivate(ctx)
LogicalPipeCtxPtr    ctx;
{
    PipeDataPtr	src1 = NULL, src2 = NULL, dst = NULL;
    int status, dstpos;

    if( LogSrcSnk2_(ctx) == NULL )
	/* Operating between one image and a constant */
	for(status = Success; status == Success; src1 = DdxRmDeallocData_(src1),
						  dst = DdxRmDeallocData_(dst))
	    {
	    src1 = DdxRmGetData_( ctx, LogSrcDrn1_(ctx) );
	    if( !IsPointer_(src1) ) return( (int) src1 );

	    if( DatY1_(src1) < LogRoiY1_(ctx) || DatY1_(src1) > LogRoiY2_(ctx) )
		{
		/* Scanline is outside the ROI - just pass it through */
		status = DdxRmPutData_( LogDstSnk_(ctx), src1 );
		src1   = NULL;
		}
	    else
		{
		dst = DdxRmAllocData_( LogDstSnk_(ctx), DatCmpIdx_(src1),
				       DatY1_(src1), DatHeight_(src1) );
		status = (int) dst;
		if( !IsPointer_(dst) ) continue;

		/* Copy the portions of this line outside the ROI */
		status = DdxCopy_( DatUdpPtr_(src1),		/* Left  */
				   DatUdpPtr_(dst),
				   DatX1_(src1), 0,
				   DatX1_(dst), DatY1_(dst),
				   LogRoiX1_(ctx) - DatX1_(src1), 1 );
		if( status != Success ) continue;
		
		status = DdxCopy_( DatUdpPtr_(src1),		/* Right */
				   DatUdpPtr_(dst),
				   LogRoiX2_(ctx)+1, 0,
				   LogRoiX2_(ctx)+1, DatY1_(dst),
				   DatX2_(src1) - LogRoiX2_(ctx), 1 );
		if( status != Success ) continue;

		DatPos_(src1) +=
		    (LogRoiX1_(ctx) - DatX1_(src1)) * DatPxlStr_(src1);
		dstpos = DatPos_(dst);
		DatPos_(dst) +=
		    (LogRoiX1_(ctx) - DatX1_(dst)) * DatPxlStr_(dst);;

		(*LogFunc_(ctx))(DatUdpPtr_(src1),
				 LogConst_(ctx,DatCmpIdx_(src1)),
				 DatUdpPtr_(dst),
				 LogCpp_(ctx),
				 LogRoiY1_(ctx),
				 LogRoiX1_(ctx),
				 LogRoiX2_(ctx));

		DatPos_(dst) = dstpos;
		status = DdxRmPutData_( LogDstSnk_(ctx), dst );
		dst    = NULL;
		}
	    }

    else
	/* Operating between two images */
	for( status = Success; status == Success;
		      LogSrcData1_(ctx) = DdxRmDeallocData_(LogSrcData1_(ctx)),
		      LogSrcData2_(ctx) = DdxRmDeallocData_(LogSrcData2_(ctx)),
				    dst = DdxRmDeallocData_(dst) )
	    {
	    if( LogSrcData1_(ctx) == NULL )
		{
		LogSrcData1_(ctx) = DdxRmGetData_( ctx, LogSrcDrn1_(ctx) );
		if( !IsPointer_(LogSrcData1_(ctx)) )
		    return((int)LogSrcData1_(ctx));
		}
	    src1 = LogSrcData1_(ctx);

	    if( DatY1_(src1) <= LogRoiY2_(ctx) )
		{
		/* Keep the two streams synchronized as long as we still
		 * need src2 data */
		if( LogSrcData2_(ctx) == NULL )
		    {
		    LogSrcData2_(ctx) = DdxRmGetData_( ctx, LogSrcDrn2_(ctx) );
		    if( !IsPointer_(LogSrcData2_(ctx)) )
			return((int)LogSrcData2_(ctx));
		    }
		src2 = LogSrcData2_(ctx);
		}
	    if( DatY1_(src1) < LogRoiY1_(ctx) || DatY1_(src1) > LogRoiY2_(ctx) )
		{
		/* Scanline is outside the ROI - just pass it through */
		status = DdxRmPutData_( LogDstSnk_(ctx), src1 );
		LogSrcData1_(ctx) = NULL;
		}
	    else
		{
		dst = DdxRmAllocData_( LogDstSnk_(ctx), DatCmpIdx_(src1),
				       DatY1_(src1), DatHeight_(src1) );
		status = (int) dst;
		if( !IsPointer_(dst) ) continue;

		/* Copy the portions of this line outside the ROI */
		status = DdxCopy_( DatUdpPtr_(src1),		/* Left  */
				   DatUdpPtr_(dst),
				   DatX1_(src1), 0,
				   DatX1_(dst), DatY1_(dst),
				   LogRoiX1_(ctx) - DatX1_(src1), 1 );
		if( status != Success ) continue;
		
		status = DdxCopy_( DatUdpPtr_(src1),		/* Right */
				   DatUdpPtr_(dst),
				   LogRoiX2_(ctx)+1, 0,
				   LogRoiX2_(ctx)+1, DatY1_(dst),
				   DatX2_(src1) - LogRoiX2_(ctx), 1 );
		if( status != Success ) continue;

		DatPos_(src1) +=
		    (LogRoiX1_(ctx) - DatX1_(src1)) * DatPxlStr_(src1);
		DatPos_(src2) +=
		    (LogRoiX1_(ctx) - DatX1_(src2)) * DatPxlStr_(src2);
		dstpos = DatPos_(dst);
		DatPos_(dst) +=
		    (LogRoiX1_(ctx) - DatX1_(dst)) * DatPxlStr_(dst);;

		(*LogFunc_(ctx))(DatUdpPtr_(src1),
				 DatUdpPtr_(src2),
				 DatUdpPtr_(dst),
				 LogCpp_(ctx),
				 LogRoiY1_(ctx),
				 LogRoiX1_(ctx),
				 LogRoiX2_(ctx));
	     
		DatPos_(dst) = dstpos;
		status = DdxRmPutData_( LogDstSnk_(ctx), dst );
		dst    = NULL;
		}
	    }				/* end for */
    return( status );
}

/*****************************************************************************
**  _LogicalFlush
**
**  FUNCTIONAL DESCRIPTION:
**
**  	This routine is called when all preceding pipeline stages have
**      completed. 
**
**
**  FORMAL PARAMETERS:
**
**      ctx - logical pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _LogicalFlush(ctx)
LogicalPipeCtxPtr    ctx;
{
    PipeDataPtr src2;

    _LogicalAbort(ctx);

    /* If the image on drain 2 has more scanlines that the image on drain 1
     * we will have unprocessed scanlines on drain 2.  Flush them.
     */
    if( IsPointer_(LogSrcSnk2_(ctx)) )
	while((src2 = DdxRmGetData_( ctx, LogSrcDrn2_(ctx) )) != NULL )
	    {
	    if( !IsPointer_(src2) ) return( (int) src2 );
	    DdxRmDeallocData_( src2 );
	    }
    return( Success );
}

/*****************************************************************************
**  _LogicalAbort
**
**  FUNCTIONAL DESCRIPTION:
**
**  	This routine is called when all preceding pipeline stages have
**      completed. 
**
**  FORMAL PARAMETERS:
**
**      ctx - logical pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _LogicalAbort(ctx)
LogicalPipeCtxPtr    ctx;
{
    if( IsPointer_(LogSrcData1_(ctx)) )
	LogSrcData1_(ctx) = DdxRmDeallocData_( LogSrcData1_(ctx) );
    if( IsPointer_(LogSrcData2_(ctx)) )
	LogSrcData2_(ctx) = DdxRmDeallocData_( LogSrcData2_(ctx) );

    return( Success );
}

/*****************************************************************************
**  _LogicalDestroy
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine deallocates all resources allocated by the LOGICAL
**	pipeline element.
**
**  FORMAL PARAMETERS:
**
**      ctx - logical pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _LogicalDestroy(ctx)
LogicalPipeCtxPtr    ctx;
{
    int i;

    LogSrcDrn1_(ctx) = DdxRmDestroyDrain_( LogSrcDrn1_(ctx) );
    if (LogSrcSnk2_(ctx) != NULL)
	LogSrcDrn2_(ctx)  = DdxRmDestroyDrain_( LogSrcDrn2_(ctx) );

    return( Success );
}

/*****************************************************************************
**  SmiLogical
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine performs a non-pipelined logical operator, which
**      performs an logical operator between two photomaps, or between
**      a photomap and a constant.
**
**  FORMAL PARAMETERS:
**
**	srcudp1 - Pointer to a UDP describing the first source operand.
**	srcudp2 - Pointer to a UDP describing the second source operand.
**      constant- Array of constants supplied if srcudp2 is NULL.
**      dstudp  - Pointer to a UDP describing the destination buffer.
**      idc     - Pointer to a UDP describing the Region of Interest,
**                or Control Processing Plane.
**                (NULL if no IDC)
**      operator-Code for the operation being performed.
**
**  FUNCTION VALUE:
**
*****************************************************************************/
int 	SmiLogical( srcudp1, srcudp2, constant, dstudp, idc, operator )
UdpPtr			srcudp1;
UdpPtr		  	srcudp2;
unsigned int	constant[];
UdpPtr			dstudp;
UdpPtr	 		idc;
int		operator;

{
    UdpPtr   cpp, src1 = NULL, src2 = NULL, dst = NULL;
    long int dtype, pixels, scanline, status;
    long int X1, X2, Y1, Y2;		/* src/destination common bounds     */
    long int x1, x2, y1, y2;		/* src/destination/ROI common bounds */

    /*
    **  Allocate the destination image array
    */
    if( dstudp->UdpA_Base == NULL )
	{
        dstudp->UdpA_Base = DdxMallocBits_( dstudp->UdpL_ArSize );
        if( dstudp->UdpA_Base == NULL ) return( BadAlloc );
	}
    /*
    **	Determine maximum common dimensions between source1 and destination,
    **	which is the defined region of output.
    */
    X1 = srcudp1->UdpL_X1 > dstudp->UdpL_X1 ?
	 srcudp1->UdpL_X1 : dstudp->UdpL_X1;
    X2 = srcudp1->UdpL_X2 < dstudp->UdpL_X2 ?
	 srcudp1->UdpL_X2 : dstudp->UdpL_X2;
    Y1 = srcudp1->UdpL_Y1 > dstudp->UdpL_Y1 ?
	 srcudp1->UdpL_Y1 : dstudp->UdpL_Y1;
    Y2 = srcudp1->UdpL_Y2 < dstudp->UdpL_Y2 ?
	 srcudp1->UdpL_Y2 : dstudp->UdpL_Y2;
    /*
    **	Restrict the area to be processed by finding the intersection of
    **	the region of output and the second source operand (if used).
    */
    if( IsPointer_(srcudp2) )
	{
	x1 = srcudp2->UdpL_X1 > X1 ? srcudp2->UdpL_X1 : X1;
	x2 = srcudp2->UdpL_X2 < X2 ? srcudp2->UdpL_X2 : X2;
	y1 = srcudp2->UdpL_Y1 > Y1 ? srcudp2->UdpL_Y1 : Y1;
	y2 = srcudp2->UdpL_Y2 < Y2 ? srcudp2->UdpL_Y2 : Y2;
	}
    else
	{
	x1 = X1; x2 = X2;
	y1 = Y1; y2 = Y2;
        }
    /* 
     * Further restrict the area to be processed by finding the intersection
     * with the IDC (ROI or CPP).
     */
    if( IsPointer_(idc) )
	{
	x1 = idc->UdpL_X1 > x1 ? idc->UdpL_X1 : x1;
	x2 = idc->UdpL_X2 < x2 ? idc->UdpL_X2 : x2;
	y1 = idc->UdpL_Y1 > y1 ? idc->UdpL_Y1 : y1;
	y2 = idc->UdpL_Y2 < y2 ? idc->UdpL_Y2 : y2;
	}
    cpp   = IsPointer_(idc) && IsPointer_(idc->UdpA_Base) ? idc : NULL;
    dtype = dstudp->UdpL_Levels > 256 || srcudp1->UdpL_Levels > 256
	       || IsPointer_(srcudp2) && srcudp2->UdpL_Levels > 256
				       ? UdpK_DTypeWU : UdpK_DTypeBU;
    /* Allocate and init temporary UDP's to hold one scanline of data */
    src1 = LogicalGetTempUdp( srcudp1, dtype, y1, x1, x2 );
    if( !IsPointer_(src1) )
	{
	status = (int) src1;
	goto clean_up;
	}
    if( IsPointer_(srcudp2) )
	{
	src2 = LogicalGetTempUdp( srcudp2, dtype, y1, x1, x2 );
	if( !IsPointer_(src2) )
	    {
	    status = (int) src2;
	    goto clean_up;
	    }
	}
    dst = LogicalGetTempUdp( dstudp, dtype, y1, x1, x2 );
    if( !IsPointer_(dst) )
	{
	status = (int) dst;
	goto clean_up;
	}

    /* Just copy the scanlines outside of the ROI */
    /* Top */
    status = DdxCopy_( srcudp1, dstudp, X1, Y1, X1, Y1, X2-X1+1, y1-Y1 );
    if( status != Success ) goto clean_up;
    /* Bottom */
    status = DdxCopy_( srcudp1, dstudp, X1, y2+1, X1, y2+1, X2-X1+1, Y2-y2 );
    if( status != Success ) goto clean_up;
    /* Left */    
    status = DdxCopy_( srcudp1, dstudp, X1, y1, X1, y1, x1-X1, y2-y1+1 );
    if( status != Success ) goto clean_up;
    /* Right */    
    status = DdxCopy_( srcudp1, dstudp, x2+1, y1, x2+1, y1, X2-x2, y2-y1+1 );
    if( status != Success ) goto clean_up;

    /* If the temporary UDP's are pointing to the original data, position
     * to the correct offset.
     */
    if( src1->UdpA_Base == srcudp1->UdpA_Base )
	src1->UdpL_Pos   = srcudp1->UdpL_Pos + srcudp1->UdpL_ScnStride * y1
					     + srcudp1->UdpL_PxlStride *
						  (x1 - srcudp1->UdpL_X1);

    if( srcudp2 != NULL && src2->UdpA_Base == srcudp2->UdpA_Base )
	src2->UdpL_Pos = srcudp2->UdpL_Pos + y1 * srcudp2->UdpL_ScnStride +
			(x1 - srcudp2->UdpL_X1) * srcudp2->UdpL_PxlStride;

    dst->UdpL_Y1 = dst->UdpL_Y2 = y1;
    if( dst->UdpA_Base == dstudp->UdpA_Base )
	dst->UdpL_Pos   = dstudp->UdpL_Pos + y1 * dstudp->UdpL_ScnStride +
			 (x1 - dstudp->UdpL_X1) * dstudp->UdpL_PxlStride;

    /* Now perform the logical operation over the region-of-interest */
    if( IsPointer_(srcudp2) )
	/* Image-to-image operation */
	for( scanline = y1; scanline <= y2 && status == Success; scanline++ )
	    {
	    /* Get a UDP pointing to this scanline */
	    status = LogicalConvertScanline(srcudp1, src1, dtype);
	    if( status != Success ) break;
	    status = LogicalConvertScanline(srcudp2, src2, dtype);
	    if( status != Success ) break;
	    
	    (*LogicalTableII[operator])(src1, src2,
					dst,
					cpp, y1, x1, x2);
	    status = LogicalStoreScanline(dst, dstudp);
	    }
    else
	/* Image-to-constant operation */
	for( scanline = y1; scanline <= y2 && status == Success; scanline++ )
	    {
	    /* Get a UDP pointing to this scanline */
	    status = LogicalConvertScanline(srcudp1, src1, dtype);
	    if( status != Success ) break;
	    
	    (*LogicalTableIC[operator])(src1, constant[src1->UdpL_CompIdx],
					dst,
					cpp, y1, x1, x2);

	    status = LogicalStoreScanline(dst, dstudp);
	    }

    /* Cleanup the temporary descriptors */
clean_up :
    if( IsPointer_(src1) )
	LogicalDestroyTempUdp( src1, srcudp1 );
    if( IsPointer_(src2) )
	LogicalDestroyTempUdp( src2, srcudp2 );
    if( IsPointer_(dst) )
	LogicalDestroyTempUdp( dst, dstudp );

    return( status);
}					/* end SmiLogical */

/*****************************************************************************
**  Logical processing routines
**
**  FUNCTIONAL DESCRIPTION:
**
**	These routines perform the actual logical operation over selected
**      portion of a scanline.  A separate routine is used for each
**      type of operation.
**
**  FORMAL PARAMETERS:
**
**	src1    - Pointer to a UDP describing the first source operand.
**	src2    - Pointer to a UDP describing the second source operand.
**      const  - Constant "image" value if image-to-constant.
**      dst     - Pointer to a UDP describing the destination buffer.
**      cpp     - Pointer to a UDP describing the Control Processing Plane.
**                (NULL if no CPP)
**      y1      - The y corrdinate of the beginning of the region to
*                 process.
**      x1      - The x coordinate of the left endpoint of the CPP, or
**                the ROI.
**      x2      - The x coordinate of the right endpoint of the CPP, or
**                the ROI.
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static void  LogicalAndII(src1, src2, dst, cpp, y1, x1, x2)
UdpPtr	     	src1;
UdpPtr	     	src2;
UdpPtr	     	dst;
UdpPtr		cpp;
long int        y1;
long int	x1;
long int	x2;
{
    int			pixel;
    unsigned char 	*src1ptr;
    unsigned char 	*src2ptr;
    unsigned char       *dstptr;
    long int		cpppxloff;
    int			cppbit;
    int			src1bytes = src1->UdpL_PxlStride >> 3;
    int                 src2bytes = src2->UdpL_PxlStride >> 3;
    int                 dstbytes  = dst->UdpL_PxlStride  >> 3;

    src1ptr = src1->UdpA_Base + (src1->UdpL_Pos >> 3);
    src2ptr = src2->UdpA_Base + (src2->UdpL_Pos >> 3);
    dstptr  = dst->UdpA_Base  + (dst->UdpL_Pos  >> 3);

    if( cpp != NULL )
	cpppxloff = cpp->UdpL_Pos + cpp->UdpL_ScnStride * (src1->UdpL_Y1 - y1);
    else
	cppbit = 1;			/* Fake all 1's in CPP */

    if( dst->UdpB_DType == UdpK_DTypeBU )
	for( pixel = x1; pixel <= x2; pixel++ )
	    {
	    if( cpp != NULL )
		{
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff += cpp->UdpL_PxlStride;
		}	  
	    if( cppbit )
		*dstptr = *src1ptr & *src2ptr;
	    else
		*dstptr = *src1ptr;	/* Not selected by CPP */
	    
	    src1ptr += src1bytes;
	    src2ptr += src2bytes;
	    dstptr  += dstbytes;
	    }
    else
	for( pixel = x1; pixel <= x2; pixel++ )
	    {
	    if( cpp != NULL )
		{
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff += cpp->UdpL_PxlStride;
		}	  
	    if( cppbit )
		*((unsigned short *)dstptr) = *((unsigned short *)src1ptr)
					    & *((unsigned short *)src2ptr);
	    else    /* Not selected by CPP */
		*((unsigned short *)dstptr) = *((unsigned short *)src1ptr);
	    
	    src1ptr += src1bytes;
	    src2ptr += src2bytes;
	    dstptr  += dstbytes;
	    }
}					/* end LogicalAndII */

static void  LogicalOrII(src1, src2, dst, cpp, y1, x1, x2)
UdpPtr	     	src1;
UdpPtr	     	src2;
UdpPtr	     	dst;
UdpPtr		cpp;
long int        y1;
long int	x1;
long int	x2;
{
    int			pixel;
    unsigned long	value, max = dst->UdpL_Levels-1;
    unsigned char 	*src1ptr;
    unsigned char 	*src2ptr;
    unsigned char       *dstptr;
    long int		cpppxloff;
    int			cppbit;
    int			src1bytes = src1->UdpL_PxlStride >> 3;
    int                 src2bytes = src2->UdpL_PxlStride >> 3;
    int                 dstbytes  = dst->UdpL_PxlStride  >> 3;

    src1ptr = src1->UdpA_Base + (src1->UdpL_Pos >> 3);
    src2ptr = src2->UdpA_Base + (src2->UdpL_Pos >> 3);
    dstptr  = dst->UdpA_Base  + (dst->UdpL_Pos  >> 3);

    if( cpp != NULL )
	cpppxloff = cpp->UdpL_Pos + cpp->UdpL_ScnStride * (src1->UdpL_Y1 - y1);
    else
	cppbit = 1;			/* Fake all 1's in CPP */

    if( dst->UdpB_DType == UdpK_DTypeBU )
	for (pixel = x1; pixel <= x2; pixel++)
	    {
	    if( cpp != NULL )
		{
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
		}	  
	    if( cppbit )
		{
		value   = *src1ptr | *src2ptr;
		*dstptr = value <= max ? value : max;
		}
	    else
		*dstptr = *src1ptr;	/* Not selected by CPP */
	    
	    src1ptr += src1bytes;
	    src2ptr += src2bytes;
	    dstptr  += dstbytes;
	    }
    else	
	for (pixel = x1; pixel <= x2; pixel++)
	    {
	    if( cpp != NULL )
		{
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
		}	  
	    if( cppbit )
		{
		value   = *((unsigned short *)src1ptr)
			| *((unsigned short *)src2ptr);
		*((unsigned short *)dstptr) = value <= max ? value : max;
		}
	    else    /* Not selected by CPP */
		*((unsigned short *)dstptr) = *((unsigned short *)src1ptr);
	    
	    src1ptr += src1bytes;
	    src2ptr += src2bytes;
	    dstptr  += dstbytes;
	    }
}					/* end LogicalOrII */

static void  LogicalXorII(src1, src2, dst, cpp, y1, x1, x2)
UdpPtr	     	src1;
UdpPtr	     	src2;
UdpPtr	     	dst;
UdpPtr		cpp;
long int        y1;
long int	x1;
long int	x2;
{
    int			pixel;
    unsigned long	value, max = dst->UdpL_Levels-1;
    unsigned char 	*src1ptr;
    unsigned char 	*src2ptr;
    unsigned char       *dstptr;
    long int		cpppxloff;
    int			cppbit;
    int			src1bytes = src1->UdpL_PxlStride >> 3;
    int                 src2bytes = src2->UdpL_PxlStride >> 3;
    int                 dstbytes  = dst->UdpL_PxlStride  >> 3;

    src1ptr = src1->UdpA_Base + (src1->UdpL_Pos >> 3);
    src2ptr = src2->UdpA_Base + (src2->UdpL_Pos >> 3);
    dstptr  = dst->UdpA_Base  + (dst->UdpL_Pos  >> 3);

    if( cpp != NULL )
	cpppxloff = cpp->UdpL_Pos + cpp->UdpL_ScnStride * (src1->UdpL_Y1 - y1);
    else
	cppbit = 1;			/* Fake all 1's in CPP */

    if( dst->UdpB_DType == UdpK_DTypeBU )
	for( pixel = x1; pixel <= x2; pixel++ )
	    {
	    if( cpp != NULL )
		{
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff += cpp->UdpL_PxlStride;
		}	  
	    if( cppbit )
		{
		value   = *src1ptr ^ *src2ptr;
		*dstptr = value <= max ? value : max;
		}
	    else
		*dstptr = *src1ptr;	/* Not selected by CPP */
	    
	    src1ptr += src1bytes;
	    src2ptr += src2bytes;
	    dstptr  += dstbytes;
	    }
    else
	for( pixel = x1; pixel <= x2; pixel++ )
	    {
	    if( cpp != NULL )
		{
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff += cpp->UdpL_PxlStride;
		}	  
	    if( cppbit )
		{
		value   = *((unsigned short *)src1ptr)
			^ *((unsigned short *)src2ptr);
		*((unsigned short *)dstptr) = value <= max ? value : max;
		}
	    else    /* Not selected by CPP */
		*((unsigned short *)dstptr) = *((unsigned short *)src1ptr);
	    
	    src1ptr += src1bytes;
	    src2ptr += src2bytes;
	    dstptr  += dstbytes;
	    }
}					/* end LogicalXorII */

static void  LogicalNotII(src1, src2, dst, cpp, y1, x1, x2)
UdpPtr	     	src1;
UdpPtr	     	src2;
UdpPtr	     	dst;
UdpPtr		cpp;
long int        y1;
long int	x1;
long int	x2;
{
    /* Since NOT is a unary operation, we just ignore the second
     * operand.
     */
    int			pixel;
    unsigned long	value, max = dst->UdpL_Levels-1;
    unsigned char 	*src1ptr;
    unsigned char       *dstptr;
    long int		cpppxloff;
    int			cppbit;
    int			src1bytes = src1->UdpL_PxlStride >> 3;
    int                 dstbytes  = dst->UdpL_PxlStride  >> 3;
    
    src1ptr = src1->UdpA_Base + (src1->UdpL_Pos >> 3);
    dstptr  = dst->UdpA_Base  + (dst->UdpL_Pos  >> 3);

    if( cpp != NULL )
	cpppxloff = cpp->UdpL_Pos + cpp->UdpL_ScnStride * (src1->UdpL_Y1 - y1);
    else
	cppbit = 1;			/* Fake all 1's in CPP */

    if( dst->UdpB_DType == UdpK_DTypeBU )
	for( pixel = x1; pixel <= x2; pixel++ )
	    {
	    if( cpp != NULL )
		{
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff += cpp->UdpL_PxlStride;
		}	  
	    if( cppbit )
		{
		value   = ~(*src1ptr);
		*dstptr = value <= max ? value : max;
		}
	    else
		*dstptr =   *src1ptr;	/* Not selected by CPP */
	    
	    src1ptr += src1bytes;
	    dstptr  += dstbytes;
	    }
    else
	for( pixel = x1; pixel <= x2; pixel++ )
	    {
	    if( cpp != NULL )
		{
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff += cpp->UdpL_PxlStride;
		}	  
	    if( cppbit )
		{
		value   = ~(*((unsigned short *)src1ptr));
		*((unsigned short *)dstptr) = value <= max ? value : max;
		}
	    else    /* Not selected by CPP */
		*((unsigned short *)dstptr) =   *((unsigned short *)src1ptr);
	    
	    src1ptr += src1bytes;
	    dstptr  += dstbytes;
	    }
}					/* end LogicalNotII */

static void  LogicalShiftUpII(src1, src2, dst, cpp, y1, x1, x2)
UdpPtr	     	src1;
UdpPtr	     	src2;
UdpPtr	     	dst;
UdpPtr		cpp;
long int        y1;
long int	x1;
long int	x2;
{
    int			pixel;
    unsigned long	value, max = dst->UdpL_Levels-1;
    unsigned char 	*src1ptr;
    unsigned char 	*src2ptr;
    unsigned char       *dstptr;
    long int		cpppxloff;
    int			cppbit;
    int			src1bytes = src1->UdpL_PxlStride >> 3;
    int                 src2bytes = src2->UdpL_PxlStride >> 3;
    int                 dstbytes  = dst->UdpL_PxlStride  >> 3;

    src1ptr = src1->UdpA_Base + (src1->UdpL_Pos >> 3);
    src2ptr = src2->UdpA_Base + (src2->UdpL_Pos >> 3);
    dstptr  = dst->UdpA_Base  + (dst->UdpL_Pos  >> 3);

    if( cpp != NULL )
	cpppxloff = cpp->UdpL_Pos + cpp->UdpL_ScnStride * (src1->UdpL_Y1 - y1);
    else
	cppbit = 1;			/* Fake all 1's in CPP */

    if( dst->UdpB_DType == UdpK_DTypeBU )
	for( pixel = x1; pixel <= x2; pixel++ )
	    {
	    if( cpp != NULL )
		{
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff += cpp->UdpL_PxlStride;
		}	  
	    if( cppbit )
		{
		value   = *src1ptr << *src2ptr;
		*dstptr = value <= max ? value : max;
		}
	    else
		*dstptr = *src1ptr;	/* Not selected by CPP */
	    
	    src1ptr += src1bytes;
	    src2ptr += src2bytes;
	    dstptr  += dstbytes;
	    }
    else
	for( pixel = x1; pixel <= x2; pixel++ )
	    {
	    if( cpp != NULL )
		{
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff += cpp->UdpL_PxlStride;
		}	  
	    if( cppbit )
		{
		value   = *((unsigned short *)src1ptr)
		       << *((unsigned short *)src2ptr);
		*((unsigned short *)dstptr) = value <= max ? value : max;
		}
	    else    /* Not selected by CPP */
		*((unsigned short *)dstptr) = *((unsigned short *)src1ptr);
	    
	    src1ptr += src1bytes;
	    src2ptr += src2bytes;
	    dstptr  += dstbytes;
	    }
}					/* end LogicalShiftUpII */

static void  LogicalShiftDownII(src1, src2, dst, cpp, y1, x1, x2)
UdpPtr	     	src1;
UdpPtr	     	src2;
UdpPtr	     	dst;
UdpPtr		cpp;
long int        y1;
long int	x1;
long int	x2;
{
    int			pixel;
    unsigned char 	*src1ptr;
    unsigned char 	*src2ptr;
    unsigned char       *dstptr;
    long int		cpppxloff;
    int			cppbit;
    int			src1bytes = src1->UdpL_PxlStride >> 3;
    int                 src2bytes = src2->UdpL_PxlStride >> 3;
    int                 dstbytes  = dst->UdpL_PxlStride  >> 3;

    src1ptr = src1->UdpA_Base + (src1->UdpL_Pos >> 3);
    src2ptr = src2->UdpA_Base + (src2->UdpL_Pos >> 3);
    dstptr  = dst->UdpA_Base  + (dst->UdpL_Pos  >> 3);

    if( cpp != NULL )
	cpppxloff = cpp->UdpL_Pos + cpp->UdpL_ScnStride * (src1->UdpL_Y1 - y1);
    else
	cppbit = 1;			/* Fake all 1's in CPP */

    if( dst->UdpB_DType == UdpK_DTypeBU )
	for( pixel = x1; pixel <= x2; pixel++ )
	    {
	    if( cpp != NULL )
		{
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff += cpp->UdpL_PxlStride;
		}	  
	    if( cppbit )
		*dstptr = *src1ptr >> *src2ptr;
	    else
		*dstptr = *src1ptr;	/* Not selected by CPP */
	    
	    src1ptr += src1bytes;
	    src2ptr += src2bytes;
	    dstptr  += dstbytes;
	    }
    else
	for( pixel = x1; pixel <= x2; pixel++ )
	    {
	    if( cpp != NULL )
		{
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff += cpp->UdpL_PxlStride;
		}	  
	    if( cppbit )
		*((unsigned short *)dstptr) = *((unsigned short *)src1ptr)
					   >> *((unsigned short *)src2ptr);
	    else    /* Not selected by CPP */
		*((unsigned short *)dstptr) = *((unsigned short *)src1ptr);
	    
	    src1ptr += src1bytes;
	    src2ptr += src2bytes;
	    dstptr  += dstbytes;
	    }
}					/* end LogicalShiftDownII */

static void  LogicalAndIC(src, constant, dst, cpp, y1, x1, x2)
    UdpPtr	        src;
    unsigned long int	constant;
    UdpPtr	        dst;
    UdpPtr	        cpp;
    long int            y1;
    long int	        x1;
    long int	        x2;
{
    int			pixel;
    unsigned char 	*srcptr;
    unsigned char       *dstptr;
    long int		cpppxloff;
    int			cppbit;
    int			srcbytes = src->UdpL_PxlStride >> 3;
    int                 dstbytes = dst->UdpL_PxlStride >> 3;

    srcptr = src->UdpA_Base + (src->UdpL_Pos >> 3);
    dstptr = dst->UdpA_Base + (dst->UdpL_Pos >> 3);

    if( cpp != NULL )
	cpppxloff = cpp->UdpL_Pos + cpp->UdpL_ScnStride * (src->UdpL_Y1 - y1);
    else
	cppbit = 1;			/* Fake all 1's in CPP */

    if( dst->UdpB_DType == UdpK_DTypeBU )
	for( pixel = x1; pixel <= x2; pixel++ )
	    {
	    if( cpp != NULL )
		{
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
		}	  
	    if( cppbit )
		*dstptr = *srcptr & (unsigned char) constant;
	    else
		*dstptr = *srcptr;	/* Not selected by CPP */
	    
	    srcptr += srcbytes;
	    dstptr += dstbytes;
	    }
    else
	for( pixel = x1; pixel <= x2; pixel++ )
	    {
	    if( cpp != NULL )
		{
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
		}	  
	    if( cppbit )
		*((unsigned short *)dstptr) = *((unsigned short *)srcptr)
					      & (unsigned short) constant;
	    else    /* Not selected by CPP */
		*((unsigned short *)dstptr) = *((unsigned short *)srcptr);
	    
	    srcptr += srcbytes;
	    dstptr += dstbytes;
	    }
}					/* end LogicalAndIC */

static void  LogicalOrIC(src, constant, dst, cpp, y1, x1, x2)
    UdpPtr	        src;
    unsigned long int	constant;
    UdpPtr	        dst;
    UdpPtr	        cpp;
    long int            y1;
    long int	        x1;
    long int	        x2;
{
    int			pixel;
    unsigned long	value, max = dst->UdpL_Levels-1;
    unsigned char 	*srcptr;
    unsigned char       *dstptr;
    long int		cpppxloff;
    int			cppbit;
    int			srcbytes = src->UdpL_PxlStride >> 3;
    int                 dstbytes = dst->UdpL_PxlStride >> 3;

    srcptr = src->UdpA_Base + (src->UdpL_Pos >> 3);
    dstptr = dst->UdpA_Base + (dst->UdpL_Pos >> 3);

    if( cpp != NULL )
	cpppxloff = cpp->UdpL_Pos + cpp->UdpL_ScnStride * (src->UdpL_Y1 - y1);
    else
	cppbit = 1;			/* Fake all 1's in CPP */

    if( dst->UdpB_DType == UdpK_DTypeBU )
	for( pixel = x1; pixel <= x2; pixel++ )
	    {
	    if( cpp != NULL )
		{
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
		}	  
	    if( cppbit )
		{
		value   = *srcptr | (unsigned char) constant;
		*dstptr = value <= max ? value : max;
		}
	    else
		*dstptr = *srcptr;	/* Not selected by CPP */
	    
	    srcptr += srcbytes;
	    dstptr += dstbytes;
	    }
    else
	for( pixel = x1; pixel <= x2; pixel++ )
	    {
	    if( cpp != NULL )
		{
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
		}	  
	    if( cppbit )
		{
		value   = *((unsigned short *)srcptr)
			  | (unsigned short) constant;
		*((unsigned short *)dstptr) = value <= max ? value : max;
		}
	    else    /* Not selected by CPP */
		*((unsigned short *)dstptr) = *((unsigned short *)srcptr);
	    
	    srcptr += srcbytes;
	    dstptr += dstbytes;
	    }
}					/* end LogicalOrIC */

static void  LogicalXorIC(src, constant, dst, cpp, y1, x1, x2)
    UdpPtr		src;
    unsigned long int 	constant;
    UdpPtr		dst;
    UdpPtr		cpp;
    long int            y1;
    long int		x1;
    long int		x2;
{
    int			pixel;
    unsigned long	value, max = dst->UdpL_Levels-1;
    unsigned char 	*srcptr;
    unsigned char       *dstptr;
    long int		cpppxloff;
    int			cppbit;
    int			srcbytes = src->UdpL_PxlStride >> 3;
    int                 dstbytes = dst->UdpL_PxlStride >> 3;
    
    srcptr = src->UdpA_Base + (src->UdpL_Pos >> 3);
    dstptr = dst->UdpA_Base + (dst->UdpL_Pos >> 3);

    if( cpp != NULL )
	cpppxloff = cpp->UdpL_Pos + cpp->UdpL_ScnStride * (src->UdpL_Y1 - y1);
    else
	cppbit = 1;			/* Fake all 1's in CPP */

    if( dst->UdpB_DType == UdpK_DTypeBU )
	for( pixel = x1; pixel <= x2; pixel++ )
	    {
	    if( cpp != NULL )
		{
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
		}	  
	    if( cppbit )
		{
		value   = *srcptr ^ (unsigned char) constant;
		*dstptr = value <= max ? value : max;
		}
	    else
		*dstptr = *srcptr;	/* Not selected by CPP */
	    
	    srcptr += srcbytes;
	    dstptr += dstbytes;
	    }
    else
	for( pixel = x1; pixel <= x2; pixel++ )
	    {
	    if( cpp != NULL )
		{
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
		}	  
	    if( cppbit )
		{
		value   = *((unsigned short *)srcptr)
			  ^ (unsigned short) constant;
		*((unsigned short *)dstptr) = value <= max ? value : max;
		}
	    else    /* Not selected by CPP */
		*((unsigned short *)dstptr) = *((unsigned short *)srcptr);
	    
	    srcptr += srcbytes;
	    dstptr += dstbytes;
	    }
}					/* end LogicalXorIC */

static void  LogicalNotIC(src, constant, dst, cpp, y1, x1, x2)
    UdpPtr		src;
    unsigned long int 	constant;
    UdpPtr		dst;
    UdpPtr		cpp;
    long int            y1;
    long int		x1;
    long int		x2;
{
    int			pixel;
    unsigned long	value, max = dst->UdpL_Levels-1;
    unsigned char 	*srcptr;
    unsigned char       *dstptr;
    long int		cpppxloff;
    int			cppbit;
    int			srcbytes = src->UdpL_PxlStride >> 3;
    int                 dstbytes = dst->UdpL_PxlStride >> 3;
    
    srcptr = src->UdpA_Base + (src->UdpL_Pos >> 3);
    dstptr = dst->UdpA_Base + (dst->UdpL_Pos >> 3);

    if( cpp != NULL )
	cpppxloff = cpp->UdpL_Pos + cpp->UdpL_ScnStride * (src->UdpL_Y1 - y1);
    else
	cppbit = 1;			/* Fake all 1's in CPP */

    if( dst->UdpB_DType == UdpK_DTypeBU )
	for( pixel = x1; pixel <= x2; pixel++ )
	    {
	    if( cpp != NULL )
		{
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
		}	  
	    if( cppbit )
		{
		value   = ~(*srcptr);
		*dstptr = value <= max ? value : max;
		}
	    else
		*dstptr =   *srcptr;	/* Not selected by CPP */
	    
	    srcptr += srcbytes;
	    dstptr += dstbytes;
	    }
    else
	for( pixel = x1; pixel <= x2; pixel++ )
	    {
	    if( cpp != NULL )
		{
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
		}	  
	    if( cppbit )
		{
		value   = ~(*((unsigned short *)srcptr));
		*((unsigned short *)dstptr) = value <= max ? value : max;
		}
	    else    /* Not selected by CPP */
		*((unsigned short *)dstptr) =   *((unsigned short *)srcptr);
	    
	    srcptr += srcbytes;
	    dstptr += dstbytes;
	    }
}					/* end LogicalNotIC */

static void  LogicalShiftUpIC(src, constant, dst, cpp, y1, x1, x2)
    UdpPtr		src;
    unsigned long int 	constant;
    UdpPtr		dst;
    UdpPtr		cpp;
    long int            y1;
    long int		x1;
    long int		x2;
{
    int			pixel;
    unsigned long	value, max = dst->UdpL_Levels-1;
    unsigned char 	*srcptr;
    unsigned char       *dstptr;
    long int		cpppxloff;
    int			cppbit;
    int			srcbytes = src->UdpL_PxlStride >> 3;
    int                 dstbytes = dst->UdpL_PxlStride >> 3;
    
    srcptr = src->UdpA_Base + (src->UdpL_Pos >> 3);
    dstptr = dst->UdpA_Base + (dst->UdpL_Pos >> 3);

    if( cpp != NULL )
	cpppxloff = cpp->UdpL_Pos + cpp->UdpL_ScnStride * (src->UdpL_Y1 - y1);
    else
	cppbit = 1;			/* Fake all 1's in CPP */

    if( dst->UdpB_DType == UdpK_DTypeBU )
	for( pixel = x1; pixel <= x2; pixel++ )
	    {
	    if( cpp != NULL )
		{
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
		}	  
	    if( cppbit )
		{
		value   = *srcptr << (unsigned char) constant;
		*dstptr = value <= max ? value : max;
		}
	    else
		*dstptr = *srcptr;	/* Not selected by CPP */
	    
	    srcptr += srcbytes;
	    dstptr += dstbytes;
	    }
    else
	for( pixel = x1; pixel <= x2; pixel++ )
	    {
	    if( cpp != NULL )
		{
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
		}	  
	    if( cppbit )
		{
		value   = *((unsigned short *)srcptr)
			 << (unsigned short) constant;
		*((unsigned short *)dstptr) = value <= max ? value : max;
		}
	    else    /* Not selected by CPP */
		*((unsigned short *)dstptr) = *((unsigned short *)srcptr);
	    
	    srcptr += srcbytes;
	    dstptr += dstbytes;
	    }
}					/* end LogicalShiftUpIC */

static void  LogicalShiftDownIC(src, constant, dst, cpp, y1, x1, x2)
    UdpPtr		src;
    unsigned long int 	constant;
    UdpPtr		dst;
    UdpPtr		cpp;
    long int            y1;
    long int		x1;
    long int		x2;
{
    int			pixel;
    unsigned char 	*srcptr;
    unsigned char       *dstptr;
    long int		cpppxloff;
    int			cppbit;
    int			srcbytes = src->UdpL_PxlStride >> 3;
    int                 dstbytes = dst->UdpL_PxlStride >> 3;

    srcptr = src->UdpA_Base + (src->UdpL_Pos >> 3);
    dstptr = dst->UdpA_Base + (dst->UdpL_Pos >> 3);

    if( cpp != NULL )
	cpppxloff = cpp->UdpL_Pos + cpp->UdpL_ScnStride * (src->UdpL_Y1 - y1);
    else
	cppbit = 1;			/* Fake all 1's in CPP */

    if( dst->UdpB_DType == UdpK_DTypeBU )
	for( pixel = x1; pixel <= x2; pixel++ )
	    {
	    if( cpp != NULL )
		{
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
		}	  
	    if( cppbit )
		*dstptr = *srcptr >> (unsigned char) constant;
	    else
		*dstptr = *srcptr;	/* Not selected by CPP */
	    
	    srcptr += srcbytes;
	    dstptr += dstbytes;
	    }
    else
	for( pixel = x1; pixel <= x2; pixel++ )
	    {
	    if( cpp != NULL )
		{
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
		}	  
	    if( cppbit )
		*((unsigned short *)dstptr) = *((unsigned short *)srcptr)
					     >> (unsigned short) constant;
	    else    /* Not selected by CPP */
		*((unsigned short *)dstptr) = *((unsigned short *)srcptr);
	    
	    srcptr += srcbytes;
	    dstptr += dstbytes;
	    }
}					/* end LogicalShiftDownIC */

/*****************************************************************************
**  LogicalGetTempUdp
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create a UDP which can be used to hold one scanline of the source
**      UDP,  using byte or word data.  If the source UDP is already in
*       the correct format, just point to its data.
**
**  FORMAL PARAMETERS:
**
**	inudp - Pointer to source UDP
**	dtype - dtype required
**      y1    - First scanline we need
**      x1    - Leftmost pixel we need
**      x2    - Rightmost pixel we need
**
**  FUNCTION VALUE:
**
**      Returns a pointer to a UDP describing the data.
**
*****************************************************************************/
static UdpPtr LogicalGetTempUdp(src, dtype, y1, x1, x2)
 UdpPtr   src;
 char     dtype;
 long int y1;
 long int x1, x2;
{
    UdpPtr dst;

    dst = (UdpPtr) DdxMalloc_(sizeof(UdpRec));
    if( dst == NULL ) return( (UdpPtr) BadAlloc );

    if( src->UdpB_DType != dtype )
	{
	dst->UdpW_PixelLength	= dtype == UdpK_DTypeBU ? sizeof(char)  * 8
							: sizeof(short) * 8;
	dst->UdpB_DType		= dtype;
	dst->UdpB_Class		= UdpK_ClassA;
	dst->UdpL_PxlStride	= dst->UdpW_PixelLength;
	dst->UdpL_PxlPerScn	= x2 - x1 + 1;
	dst->UdpL_ScnStride	= dst->UdpL_PxlStride * dst->UdpL_PxlPerScn;
	dst->UdpL_ArSize	= dst->UdpL_ScnStride;
	dst->UdpL_Pos		= 0;
	dst->UdpL_CompIdx	= src->UdpL_CompIdx;
	dst->UdpL_Levels	= src->UdpL_Levels;
	dst->UdpA_Base		= DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL )
	    {
	    DdxFree_(dst);
	    return( (UdpPtr) BadAlloc );
	    }
	}
    else
	{
	*dst = *src;
	dst->UdpL_PxlPerScn	= x2 - x1 + 1;
	dst->UdpL_ScnStride	= dst->UdpL_PxlPerScn * dst->UdpL_PxlStride;
	}
    /* One scanline UDP */
    dst->UdpL_X1	= x1;
    dst->UdpL_X2	= x2;
    dst->UdpL_Y1	= y1;
    dst->UdpL_Y2	= y1;
    dst->UdpL_ScnCnt	=  1;

    return( dst );
}					/* end LogicalGetTempUdp */

/*****************************************************************************
**  LogicalDestroyTempUdp
**
**  FUNCTIONAL DESCRIPTION:
**
**	Free a UDP created by LogicalGetTempUdp.
**
**  FORMAL PARAMETERS:
**
**	udp     - Pointer to the udp to free.
**      realudp - Pointer to a udp which might actually own the storage
**                pointed to by udp.
**
*****************************************************************************/
static void LogicalDestroyTempUdp( udp, realudp )
UdpPtr udp;
UdpPtr realudp;
{
    if( IsPointer_(udp) )
	{
	if( IsPointer_(udp->UdpA_Base) && udp->UdpA_Base != realudp->UdpA_Base )
	    DdxFreeBits_( udp->UdpA_Base );
	DdxFree_( udp );
	}
}					/* end LogicalDestroyTempUdp */

/*****************************************************************************
**  LogicalConvertScanline
**
**  FUNCTIONAL DESCRIPTION:
**
**	Adjust a UDP to point to the specified scanline of the source
**	UDP.  If the source UDP is not of type byte, convert the
**      data and leave it associated with the adjusted UDP.
**
**  FORMAL PARAMETERS:
**
**	src      - Pointer to the source UDP.
**      dst      - Pointer to the UDP to be adjusted.
**	dtype	 - dtype required
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int LogicalConvertScanline(src,  dst, dtype)
 UdpPtr src;
 UdpPtr dst;
 char dtype;
{
    int status = Success;

    dst->UdpL_Y1++;
    dst->UdpL_Y2++;

    if( src->UdpB_DType != dtype )
	status = DdxConvert_( src, dst, XieK_MoveMode );

    else
	dst->UdpL_Pos += src->UdpL_ScnStride;

    return( status );
}					/* end LogicalConvertScanline */

/*****************************************************************************
**  LogicalStoreScanline
**
**  FUNCTIONAL DESCRIPTION:
**
**	Store a scanline of data into the destination UDP.
**      If the scanline UDP already points into the destination UDP,
**      just adjust the scanline UDP to point past the scanline already
**      stored.  If not, copy (and possibly convert) the data into the
**      destination.
**
**  FORMAL PARAMETERS:
**
**	dst        - Pointer to scanline UDP
**      realdst    - Pointer to the destination UDP
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int LogicalStoreScanline(dst, realdst)
    UdpPtr dst;
    UdpPtr realdst;
{
    int status = Success;

    if( dst->UdpA_Base != realdst->UdpA_Base )
	status = DdxConvert_( dst, realdst, XieK_MoveMode );

    else
	dst->UdpL_Pos += realdst->UdpL_ScnStride;

    dst->UdpL_Y1++;
    dst->UdpL_Y2++;

    return( status );
}					/* end LogicalStoreScanline */
