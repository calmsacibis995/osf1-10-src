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
**      This module implements the area operator which performs
**	area functions between two photomaps.
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
**	Fri Jun  1 14:21:54 1990
**
*******************************************************************************/

#include <stdio.h>

#include "SmiArea.h"

/*
**  Table of contents
*/
int 		SmiArea();
int             SmiCreateArea();

static int 	_AreaInitialize();
static int 	_AreaActivate();
static int 	_AreaAbort();
static int 	_AreaDestroy();

static UdpPtr   AreaGetTempUdp();
static void     AreaDestroyTempUdp();
static int	AreaConvertScanline();
static int	AreaStoreScanline();

static UdpPtr   AreaGetCppUdp();
static int	AreaConvertCppLine();

static void     AreaDeallocData();
static int	AreaReleaseData();

static int	AreaAdjustKernel();
static void     AreaDestroyKernel();

static void 	AreaAddMin();
static void 	AreaAddMax();
static void 	AreaAddSum();
static void 	AreaMultMin();
static void 	AreaMultMax();
static void 	AreaMultSum();

static void 	AreaAddMinCpp();
static void 	AreaAddMaxCpp();
static void 	AreaAddSumCpp();
static void 	AreaMultMinCpp();
static void 	AreaMultMaxCpp();
static void 	AreaMultSumCpp();

/*
**  Equated Symbols
*/

/*
**  External References
*/

/*
 * Macros
 */
#define Min_(a,b)  ((a) <= (b) ? (a) : (b))
#define Max_(a,b)  ((a) >  (b) ? (a) : (b))
#define Circ_(a,b) ((a) >=  0  ? (a) % (b) : (b) + (a))

/*
**  Local storage
*/
    /*
    **  Area Element Vector
    */
static PipeElementVector AreaPipeElement = {
    NULL,				/* FLINK			    */
    NULL,				/* BLINK			    */
    sizeof(PipeElementVector),		/* Size				    */
    TypePipeElementVector,		/* Structure type		    */
    StypeAreaElement,			/* Structure subtype		    */
    sizeof(AreaPipeCtx),		/* Context block size		    */
    0,					/* No input flag		    */
    0,					/* Reserved flags		    */
    _AreaInitialize,			/* Initialize entry		    */
    _AreaActivate,			/* Activate entry		    */
    _AreaAbort,				/* Flush entry			    */
    _AreaDestroy,			/* Destroy entry		    */
    _AreaAbort				/* Abort entry			    */
    };

/* Dispatch table to select the correct operation routine */
/* This must match the ordering of the XieK_xxxx constants in XieAppl.h */
static void (*AreaOpTable[])() = {
    AreaAddMin,
    AreaAddMax,
    AreaAddSum,
    AreaMultMin,
    AreaMultMax,
    AreaMultSum
    };

static void (*AreaOpTableCpp[])() = {
    AreaAddMinCpp,
    AreaAddMaxCpp,
    AreaAddSumCpp,
    AreaMultMinCpp,
    AreaMultMaxCpp,
    AreaMultSumCpp
    };

/*****************************************************************************
**  SmiCreateArea
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function fills in a newly created context for a AREA
**      pipeline element.
**
**  FORMAL PARAMETERS:
**
**      pipe    - Descriptor for pipe being built.
**      srcsnk	- Pointer to source sink.
**      dstsnk  - Pointer to destination sink.
**      kernel  - Pointer to TmpDatRec describing the area operation kernel.
**      idc     - Pointer to UDP describing the ROI or CPP (NULL if no IDC).
**      op1     - Code for the first operation (kernel to image)
**      op2     - Code for the second operation (between results of op1)
**
**  FUNCTION VALUE:
**
*****************************************************************************/
int 	    SmiCreateArea(pipe, srcsnk, dstsnk, kernel, idc, op1, op2)
Pipe		     pipe;
PipeSinkPtr	     srcsnk;
PipeSinkPtr	     dstsnk;
TmpDatPtr            kernel;
UdpPtr		     idc;
long int	     op1;
long int             op2;
{
    int windowsz = kernel->y_count;
    int i, status, comp;

    AreaPipeCtxPtr ctx = (AreaPipeCtxPtr)
			  DdxCreatePipeCtx_( pipe, &AreaPipeElement, FALSE );
    if( !IsPointer_(ctx) ) return( (int) ctx );
    /*
     ** Copy the parameters.
     */
    AreaSrcSnk_(ctx) = srcsnk;
    AreaDstSnk_(ctx) = dstsnk;

    status = AreaAdjustKernel(kernel, sizeof(float), &AreaKernel_(ctx));
    if( status != Success ) return( status );

    AreaIdc_(ctx)    = idc;
    AreaOpIdx_(ctx)  = 3 * (op1 - XieK_AreaOp1Add) + (op2 - XieK_AreaOp2Min);

    /* Calculate the bounds of the region that will actually be processed */
    if( idc == NULL )
	{
	AreaRoiX1_(ctx) = SnkX1_(srcsnk,0);
	AreaRoiX2_(ctx) = SnkX2_(srcsnk,0);
	AreaRoiY1_(ctx) = SnkY1_(srcsnk,0);
	AreaRoiY2_(ctx) = SnkY2_(srcsnk,0);
	}
    else
	{
	AreaRoiX1_(ctx) = idc->UdpL_X1 > SnkX1_(srcsnk,0) ?
			  idc->UdpL_X1 : SnkX1_(srcsnk,0);
	AreaRoiX2_(ctx) = idc->UdpL_X2 < SnkX2_(srcsnk,0) ?
			  idc->UdpL_X2 : SnkX2_(srcsnk,0);
	AreaRoiY1_(ctx) = idc->UdpL_Y1 > SnkY1_(srcsnk,0) ?
			  idc->UdpL_Y1 : SnkY1_(srcsnk,0);
	AreaRoiY2_(ctx) = idc->UdpL_Y2 < SnkY2_(srcsnk,0) ?
			  idc->UdpL_Y2 : SnkY2_(srcsnk,0);
	}
    /* Set up the circular buffer(s) used to keep the set of scanlines
     * needed to compute one scanline of output.  One set of buffers
     * contains the PipeDataPtr's for the data we are holding.  Another
     * set holds the UdpPtrs needed by the low level routines.
     */
    for( comp = 0; comp < XieK_MaxComponents; comp++ )
	{
	if( SnkUdpPtr_(AreaSrcSnk_(ctx),comp) == NULL )
	    break;
	AreaWindow_(ctx,comp)  = (UdpPtr *) DdxCalloc_(windowsz,sizeof(UdpPtr));
	if( AreaWindow_(ctx,comp) == NULL ) return( BadAlloc );
	AreaDWindow_(ctx,comp) = (PipeDataPtr *)
				       DdxCalloc_(windowsz,sizeof(PipeDataPtr));
	if( AreaDWindow_(ctx,comp) == NULL ) return( BadAlloc );
	}
    AreaSrcDrn_(ctx) = DdxRmCreateDrain_( AreaSrcSnk_(ctx), -1 );
    if( !IsPointer_(AreaSrcDrn_(ctx)) ) return( (int) AreaSrcDrn_(ctx) );

    /* Handle data as floats only, one scanline at a time */
    DdxRmSetDType_( AreaSrcDrn_(ctx), UdpK_DTypeF, DtM_F );
    DdxRmSetQuantum_( AreaSrcDrn_(ctx), 1 );
    DdxRmSetDType_( AreaDstSnk_(ctx), UdpK_DTypeF, DtM_F );
    DdxRmSetQuantum_( AreaDstSnk_(ctx), 1 );
    
    return( Success );
}					/* end SmiCreateArea */

/*****************************************************************************
**  _AreaInitialize
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function initializes the AREA pipeline element prior to
**	activation.  Allocate the temprary descriptors needed to handle
**      the scanlines.
**
**  FORMAL PARAMETERS:
**
**      ctx - area pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int	_AreaInitialize(ctx)
AreaPipeCtxPtr	ctx;
{
    int comp, i, status, next;
    int windowsz = AreaKernel_(ctx)->y_count;
    UdpPtr      *window;
    PipeDataPtr *dwindow;

    for( comp = 0; comp < XieK_MaxComponents; comp++ )
	{
	if( SnkUdpPtr_(AreaSrcSnk_(ctx),comp) == NULL )
	    break;
	window  = AreaWindow_(ctx,comp);
	dwindow = AreaDWindow_(ctx,comp);

	for( i = 0; i < windowsz; i++ )
	    dwindow[AreaWindowNxt_(ctx,comp)] = NULL;

	/* Supply initial temporary UDP's pointing to zero data if the region
	 * to be processed requires scanlines before the beginning of the
	 * image.
	 */
	for( i = AreaRoiY1_(ctx) - AreaKernel_(ctx)->center_y, next = 0;
	     i < SnkY1_(AreaSrcSnk_(ctx), comp); next++, i++ )
	    {
	    window[next] = AreaGetTempUdp(SnkUdpPtr_(AreaSrcSnk_(ctx), comp));
	    if( !IsPointer_(window[next]) ) return( (int) window[next] );

	    status = AreaConvertScanline( SnkUdpPtr_(AreaSrcSnk_(ctx),comp),
					  SnkY1_(AreaSrcSnk_(ctx),comp),
					  SnkY2_(AreaSrcSnk_(ctx),comp),
					  i, window[next] );
	    if( status != Success ) return( status );
	    }
	AreaWindowNxt_(ctx,comp) = next;
	AreaWindowCtr_(ctx,comp) = AreaKernel_(ctx)->center_y - 1;
	}	
    if( AreaIdc_(ctx) && AreaIdc_(ctx)->UdpA_Base )
	{
	AreaCpp_(ctx) = AreaGetCppUdp(AreaIdc_(ctx));
	if( !IsPointer_(AreaCpp_(ctx)) ) return( (int) AreaCpp_(ctx) );
	}	
    status = DdxRmInitializePort_( CtxHead_(ctx), AreaSrcDrn_(ctx) );
    if( status == Success )
	status  = DdxRmInitializePort_( CtxHead_(ctx), AreaDstSnk_(ctx) );

    /* Start reading from the first (and only) drain */
    CtxInp_(ctx) = AreaSrcDrn_(ctx);

    return( status );
}

/*****************************************************************************
**  _AreaActivate
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine reads the data from the source drain(s) and
**      performs the area operations.
**
**      The are five regions of the image stream which must be considered.
**      K1 is the number of kernel scanlines before the center.
**      K2 is the number of kernel scanlines after the center.
**
**      Region         Bounds             Processing
**        I           y < Y1 - K1      Not modified.  Just copy to output.
**       II         Y1-K1 <= y < Y1    Not modified, but needed to compute
**                                     later result scanlines.
**      III         Y1 <= y <= Y2      Area operator applied here.
**       IV         Y2 <  y <= Y2+K2   Not modified, but needed to compute
**                                     previous result scanlines.
**        V          y > Y2 + K1       Not modified. Just copy to output.
**
**  Any of these regions can be empty.
**
**  FORMAL PARAMETERS:
**
**      ctx - area pipeline element context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _AreaActivate(ctx)
AreaPipeCtxPtr    ctx;
{
    PipeDataPtr src = NULL, dst = NULL;
    UdpPtr srcudp;
    PipeDataPtr *dwindow;
    UdpPtr *window;
    
    int windowsz = AreaKernel_(ctx)->y_count;
    int ytail    = AreaKernel_(ctx)->y_count - AreaKernel_(ctx)->center_y - 1;
    int center, comp, i, status;
    
    for( status = Success; status == Success; src = DdxRmDeallocData_(src),
					      dst = DdxRmDeallocData_(dst) )
	{
	src = DdxRmGetData_( ctx, AreaSrcDrn_(ctx) );
	if( !IsPointer_(src) ) return( (int) src );

	comp    = DatCmpIdx_(src);
	window  = AreaWindow_(ctx,comp);
	dwindow = AreaDWindow_(ctx,comp);
	
	if( DatY1_(src) < AreaRoiY1_(ctx) - AreaKernel_(ctx)->center_y ||
	    DatY1_(src) > AreaRoiY2_(ctx) + ytail )
	    {	    
	    /* This scanline is in Region I or V.  Just release it. */
	    status = DdxRmPutData_( AreaDstSnk_(ctx), src );
	    src    = NULL;
	    }	
	else
	    {
	    srcudp = DatUdpPtr_(src);
	    
	    while( 1 )
		{
		if( srcudp->UdpL_Y1 < AreaRoiY1_(ctx) )
		    {
		    /* This scanline is in Region II.  Create a reference copy
		     * to save, then release the original.
		     */
		    PipeDataPtr tmp;
		    tmp = DdxRmAllocRefDesc_(src, DatY1_(src), DatHeight_(src));
		    if (!IsPointer_(tmp))
		      status = (int) tmp;
		    else
		      {
		      status = DdxRmPutData_(AreaDstSnk_(ctx), src);
		      src = tmp;
		      }
		    if( status != Success ) break;
		    }
		/* This scanline is Region II, III, or IV.
		 * Free the oldest scanline in the window, and save this one.
		 */
		AreaDeallocData( ctx, comp, AreaWindowNxt_(ctx,comp) );
		window[AreaWindowNxt_(ctx,comp)]  = srcudp;
		dwindow[AreaWindowNxt_(ctx,comp)] = src;
		src = NULL;
		AreaWindowNxt_(ctx,comp) =
				Circ_(AreaWindowNxt_(ctx,comp)+1, windowsz);

		if( srcudp->UdpL_Y1 < AreaRoiY1_(ctx) + ytail )
		    break;		/* We haven't filled the window yet */

		/* The window is full.  We can compute the next output
		 * scanline.
		 */
		center = Circ_(AreaWindowCtr_(ctx, comp)+1, windowsz);
		AreaWindowCtr_(ctx, comp) = center;

		dst = DdxRmAllocData_( AreaDstSnk_(ctx), comp,
				       DatY1_(dwindow[center]), 1 );
		status = (int) dst;
		if( !IsPointer_(dst) ) break;

		/* Just copy the portion of this scanline outside the ROI*/
		status = DdxCopy_( window[center],		/* Left */
				   DatUdpPtr_(dst),
				   window[center]->UdpL_X1, 0, 
				   window[center]->UdpL_X1, DatY1_(dst),
				   AreaRoiX1_(ctx)-window[center]->UdpL_X1, 1 );
		if( status != Success ) break;
		
		status = DdxCopy_( window[center],		/* Right */
				   DatUdpPtr_(dst),
				   AreaRoiX2_(ctx)+1, 0,
				   AreaRoiX2_(ctx)+1, DatY1_(dst),
				   window[center]->UdpL_X2-AreaRoiX2_(ctx), 1 );
		if( status != Success ) break;
		
		if( AreaCpp_(ctx) == NULL )
		    (*AreaOpTable[AreaOpIdx_(ctx)])(window, center,
						    AreaKernel_(ctx),
						    DatUdpPtr_(dst),
						    AreaRoiX1_(ctx),
						    AreaRoiX2_(ctx));
		else
		    { 
		    status = AreaConvertCppLine(AreaIdc_(ctx), 
					        window[center]->UdpL_Y1 + ytail,
					        AreaCpp_(ctx));
		    if( status != Success ) break;
		    (*AreaOpTableCpp[AreaOpIdx_(ctx)])(window, center,
						       AreaKernel_(ctx),
						       DatUdpPtr_(dst),
						       AreaCpp_(ctx),
						       AreaRoiX1_(ctx),
						       AreaRoiX2_(ctx));
		    }
		status = DdxRmPutData_( AreaDstSnk_(ctx), dst );
		dst    = NULL;
		if( status != Success ) break;
		/* If we have just processed the last scanline of interest,
		 * flush the window.
		 */
		if( window[center]->UdpL_Y1 == AreaRoiY2_(ctx) )
		    {
		    for( i  = Circ_(center+1, windowsz);
			 i != center;
			 i  = Circ_(i+1, windowsz) )
			if( window[i]->UdpL_Y1 < window[center]->UdpL_Y1 )
			    /* This is a Region II or III scanline.
			     * Discard it. */
			    AreaDeallocData(ctx, comp, i);
			
			else
			    if( window[i]->UdpL_Y1 <=
			        window[center]->UdpL_Y1+ytail )
				{   /* This is a Region IV scanline.
				 * Release it */
				status = AreaReleaseData(ctx, comp, i);
				if( status != Success ) break;
				}
			    else
				/* This must be a "zero" scanline
				 * inserted because region V is empty.
				 * Discard it. */
				AreaDeallocData(ctx, comp, i);
		    break;  /* end while -- All done with this component */
		    }
		/* The last scanline of the image has been read, but
		 * we haven't processed the last scanline of interest.
		 * Supply a temporary UDP containing zero data.
		 */
		i = Circ_(center+ytail, windowsz);
		if( window[i]->UdpL_Y1 >= SnkY2_(AreaSrcSnk_(ctx), comp) )
		    {
		    srcudp = AreaGetTempUdp(SnkUdpPtr_(AreaSrcSnk_(ctx),comp));
		    status = (int) srcudp;
		    if( !IsPointer_(srcudp) ) break;

		    status = AreaConvertScanline(SnkUdpPtr_(AreaSrcSnk_(ctx),
						 comp),
						 SnkY1_(AreaSrcSnk_(ctx), comp),
						 SnkY2_(AreaSrcSnk_(ctx), comp),
						 window[i]->UdpL_Y1+1, srcudp);
		    if( status != Success ) break;
		    }
		else
		    break;		/* Go get the next scanline */
		}				/* end while */
	    }					/* end else  */
	}					/* end for   */
    return( status );
}

/*****************************************************************************
**  _AreaAbort
**
**  FUNCTIONAL DESCRIPTION:
**
**  	This routine is called when all preceding pipeline stages have
**      completed. 
**
**
**  FORMAL PARAMETERS:
**
**      ctx - area pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _AreaAbort(ctx)
    AreaPipeCtxPtr    ctx;
{
    int comp, i;

    if( IsPointer_(AreaKernel_(ctx)) )
	for( comp = 0; comp < XieK_MaxComponents; comp++ )
	    {
	    if( SnkUdpPtr_(AreaSrcSnk_(ctx),comp) == NULL )
		break;
	    for( i = 0; i < AreaKernel_(ctx)->y_count; i++ )
		AreaDeallocData(ctx, comp, i);
	    }

    if( IsPointer_(AreaCpp_(ctx)) )
	AreaDestroyTempUdp( AreaCpp_(ctx), AreaIdc_(ctx) );

    return( Success );
}

/*****************************************************************************
**  _AreaDestroy
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine deallocates all resources allocated by the AREA
**	pipeline element.
**
**  FORMAL PARAMETERS:
**
**      ctx - area pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _AreaDestroy(ctx)
AreaPipeCtxPtr    ctx;
{
    int c;

    for( c = 0; c < XieK_MaxComponents; c++ )
	{
	if (SnkUdpPtr_(AreaSrcSnk_(ctx),c) == NULL)
	    break;
	if( IsPointer_(AreaWindow_(ctx,c)) )
	    AreaWindow_(ctx,c)  = (UdpPtr *) DdxFree_(AreaWindow_(ctx,c));
	if( IsPointer_(AreaDWindow_(ctx,c)) )
	    AreaDWindow_(ctx,c) = (PipeDataPtr *) DdxFree_(AreaDWindow_(ctx,c));
	}
    AreaDestroyKernel(AreaKernel_(ctx));
    AreaKernel_(ctx) = NULL;
    AreaSrcDrn_(ctx) = DdxRmDestroyDrain_( AreaSrcDrn_(ctx) );

    return( Success );
}

/*****************************************************************************
**  SmiArea
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine performs a non-pipelined area operator, which
**      performs an area operator between a UDP and a kernel
**      supplied from a Template IDC.
**
**      Only the region of the source UDP which intersects the destination
**      UDP is processed.  If an IDC is supplied, the region selected
**      by the IDC is processed, while the source UDP data is copied
**      to the unselected pixels.
**
**      Source data outside the intersection (and IDC) is used when
**      computing results at the boundary of the processed region.
**      Points outside the source image are considered to be zero.
**
**      Note that the destination photomap must be of type float or
**      the data will be transformed from unconstrained to a constrained
**      type in an undefined manner.
**
**  FORMAL PARAMETERS:
**
**	srcudp  - Pointer to a UDP describing the source image.
**      dstudp  - Pointer to a UDP describing the destination buffer.
**      in_kernel- Pointer to a TmpDatRec describing the area kernel.
**      idc     - Pointer to a UDP describing the Region of Interest,
**                or Control Processing Plane.
**                (NULL if no IDC)
**      op1     - Code for the first operation (kernel to image)
**      op2     - Code for the second operation (between results of op1)
**
**  FUNCTION VALUE:
**
*****************************************************************************/
int 	SmiArea( srcudp,  dstudp, in_kernel, idc, op1, op2 )

UdpPtr			srcudp;
UdpPtr			dstudp;
TmpDatPtr               in_kernel;
UdpPtr	 		idc;
long int		op1;
long int                op2;
{
    UdpPtr   dst = NULL, cpp = NULL, *window = NULL;
    TmpDatPtr kernel = NULL;
    long int  window_next, window_center;
    long int pixels, scanline, status;
    long int X1, X2, Y1, Y2;		/* src/destination common bounds     */
    long int x1, x2, y1, y2;		/* src/destination/ROI common bounds */
    long int windowsz;
    long int i;
    int  opidx = 3 * (op1 - XieK_AreaOp1Add) + (op2 - XieK_AreaOp2Min);


    /*
    **  Allocate the destination image array
    */
    if( dstudp->UdpA_Base == NULL )
    {
        dstudp->UdpA_Base = DdxMallocBits_( dstudp->UdpL_ArSize );
        if( dstudp->UdpA_Base == NULL ) return( BadAlloc );
    }

    /*
     * Determine maximum common dimensions between source and destination,
     * which is the defined region of output.
     */
    X1 = srcudp->UdpL_X1 > dstudp->UdpL_X1 ? srcudp->UdpL_X1 : dstudp->UdpL_X1;
    X2 = srcudp->UdpL_X2 < dstudp->UdpL_X2 ? srcudp->UdpL_X2 : dstudp->UdpL_X2;
    Y1 = srcudp->UdpL_Y1 > dstudp->UdpL_Y1 ? srcudp->UdpL_Y1 : dstudp->UdpL_Y1;
    Y2 = srcudp->UdpL_Y2 < dstudp->UdpL_Y2 ? srcudp->UdpL_Y2 : dstudp->UdpL_Y2;

    /*
     * Determine the maximum common dimensions between the above region
     * and the IDC (ROI or CPP), which is the region that will actually
     * be processed.
     */
    if( idc == NULL )
	{
	x1 = X1;
	x2 = X2;
	y1 = Y1;
	y2 = Y2;
	}
    else
	{
	x1 = idc->UdpL_X1 > X1 ? idc->UdpL_X1 : X1;
	x2 = idc->UdpL_X2 < X2 ? idc->UdpL_X2 : X2;
	y1 = idc->UdpL_Y1 > Y1 ? idc->UdpL_Y1 : Y1;
	y2 = idc->UdpL_Y2 < Y2 ? idc->UdpL_Y2 : Y2;
	if( idc->UdpA_Base != NULL )
	    {
	    cpp = AreaGetCppUdp(idc);
	    if( !IsPointer_(cpp) )
		{
		status = (int) cpp;
		goto clean_up;
		}
	    }
	}
    windowsz = in_kernel->y_count;
    window   = (UdpPtr *) DdxCalloc_(windowsz, sizeof(UdpPtr));
    if( window == NULL )
	{
	status = BadAlloc;
	goto clean_up;
	}
    /* Window is an array of UdpPtr's managed as a circular buffer.
     * Initialize the window to contain enough temporary float UDP's to
     * hold the data needed to compute one result scanline.
     * Initialize a similar window for the CPP data, converted to byte form.
     */
    for( i = 0; i < windowsz; i++ )
	{
	window[i] = AreaGetTempUdp(srcudp);
	if( !IsPointer_(window[i]) )
	    {
	    status = (int) window[i];
	    goto clean_up;
	    }
	}
     /* Fill in the UDP's with the data needed to compute the first result
     * scanline (except for one line).  Results along the image boundaries
     * are computed by treating values outside the image as zero.
     */
    for( scanline = y1 - in_kernel->center_y, window_next = 0;
	 scanline < y1 + windowsz - in_kernel->center_y - 1;
	 scanline++, window_next++ )
	{
	status = AreaConvertScanline( srcudp, srcudp->UdpL_Y1, srcudp->UdpL_Y2,
				      scanline, window[window_next] );
	if( status != Success ) goto clean_up;
	}

    /* Create a copy of the kernel containing the appropriate increments. */
    status = AreaAdjustKernel(in_kernel, window[0]->UdpL_PxlStride>>3, &kernel);

    dst = AreaGetTempUdp(dstudp);
    if( !IsPointer_(dst) )
	{
	status = (int) dst;
	goto clean_up;
	}
    /* Just copy the rectangles that are outside of the ROI */
    /* Top */
    status = DdxCopy_( srcudp, dstudp, X1, Y1, X1, Y1, X2-X1+1, y1-Y1 );
    if( status != Success ) goto clean_up;
    /* Bottom  */
    status = DdxCopy_( srcudp, dstudp, X1, y2+1, X1, y2+1, X2-X1+1, Y2-y2 );
    if( status != Success ) goto clean_up;
    /* Left */
    status = DdxCopy_( srcudp, dstudp, X1, y1, X1, y1, x1-X1, y2-y1+1 );
    if( status != Success ) goto clean_up;
    /* Right */
    status = DdxCopy_( srcudp, dstudp, x2+1, y1, x2+1, y1, X2-x2, y2-y1+1 );
    if( status != Success ) goto clean_up;

    /* Now perform the area operation over the scanlines within the ROI */

    dst->UdpL_Y1 = dst->UdpL_Y2 = y1;
    if( dst->UdpA_Base == dstudp->UdpA_Base )
	dst->UdpL_Pos   = dst->UdpL_Pos + y1 * dst->UdpL_ScnStride;

    window_center = kernel->center_y - 1;
    window_next   = windowsz - 1;
				 
    for( scanline = y1; scanline <= y2; scanline++ )
	{
	/* Advance the window, and add the next scanline to it */
	window_center = Circ_(window_center+1, windowsz);
	status = AreaConvertScanline( srcudp, srcudp->UdpL_Y1, srcudp->UdpL_Y2,
			    	      scanline+windowsz-kernel->center_y-1,
				      window[window_next]);
	if( status != Success ) goto clean_up;
	
	window_next = Circ_(window_next+1, windowsz);

	if( cpp == NULL )
	    (*AreaOpTable[opidx])(window, window_center,  kernel, dst, x1, x2);
	else
	    {
	    status = AreaConvertCppLine( idc, 
				         scanline+windowsz-kernel->center_y-1,
					 cpp );
	    if( status != Success ) goto clean_up;
	    (*AreaOpTableCpp[opidx])(window, window_center,  kernel, dst,
				     cpp, x1, x2);
	    }
	status = AreaStoreScanline(dst, scanline, dstudp);
	if( status != Success ) goto clean_up;
	}

    /* Cleanup the temporary descriptors */
clean_up :
    if( IsPointer_(window) )
	{
	for( i = 0; i < windowsz; i++ )
	    if( IsPointer_(window[i]) )
		AreaDestroyTempUdp( window[i], srcudp );
	DdxFree_(window);
	}
    if( IsPointer_(dst) )
	AreaDestroyTempUdp( dst, dstudp );

    if( IsPointer_(kernel) )
	AreaDestroyKernel(kernel);

    if( IsPointer_(cpp) )
	AreaDestroyTempUdp( cpp, idc );

    return( status );
}					/* end SmiArea */

/*****************************************************************************
**  Area processing routines
**
**  FUNCTIONAL DESCRIPTION:
**
**	These routines the actual area operation over a selected
**      portion of a scanline.  A separate routine is used for each
**      type of operation.
**
**  FORMAL PARAMETERS:
**
**	window    - Array of pointers to Udps containing the scanlines
**                  needed to process the current scanline.
**      center    - Index into window of the pointer to the current scanline.
**      kernel    - Pointer to TmpDatRec containing the kernel.
**      dst       - Pointer to a UDP describing the destination buffer.
**      x1        - The x coordinate of the left endpoint of the ROI.
**      x2        - The x coordinate of the right endpoint of the  ROI.
**
*****************************************************************************/
static void  AreaAddMin(window, center, kernel, dst, x1, x2)
UdpPtr	     	window[];
int             center;
TmpDatPtr       kernel;
UdpPtr	     	dst;
long int	x1;
long int	x2;
{
    unsigned char       *dststart;
    unsigned char       *dstptr;
    int                 dstbytes;
    int			srcbytes;

    UdpPtr              curudp;
    unsigned char       *curstart;
    unsigned char       *curptr;
    unsigned char       *curedge;

    TmpDatEntryPtr      k1;
    TmpDatEntryPtr      k2;

    int			ky;
    float               value;

    int                 windowsz = kernel->y_count;
    int                 kerntail = kernel->x_count - kernel->center_x - 1;

    srcbytes = window[center]->UdpL_PxlStride >> 3;
    dstbytes  = dst->UdpL_PxlStride >> 3;

    dststart = dst->UdpA_Base + (dst->UdpL_Pos >> 3) +
				(x1 - dst->UdpL_X1) * dstbytes;

    /* Initialize the destination scanline with the first candidate for the
     * minimum value, the value corresponding to the upper left corner of the
     * kernel.
     */
    dstptr = dststart;
    k2 = kernel->array;	     /* 1st value in first row of kernel */

    curudp = window[Circ_(center - kernel->center_y, windowsz)];
    curptr = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	     ((x1 - kernel->center_x) - curudp->UdpL_X1) * srcbytes;
    curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3);

    while (curptr < curedge) {
	/* The relevant source pixel is off the left edge of the image */
	*(float *)dstptr = k2->value + 0.0;
	curptr += srcbytes;
	dstptr += dstbytes;
	}

    curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	                          (x2 - curudp->UdpL_X1) * srcbytes;
    while (curptr < curedge) {
	*(float *)dstptr = k2->value + *(float *)curptr;
	curptr += srcbytes;
	dstptr += dstbytes;
    }

    /*
     * Now calculate the rest, saving the minimum.
     */
    k1 = kernel->array;

    for (ky = -kernel->center_y; ky < windowsz - kernel->center_y; ky++) {
	curudp = window[Circ_(center + ky, windowsz)];
	dstptr = dststart;
	
	curstart = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3)
	    + (x1 - kernel->center_x - curudp->UdpL_X1) * srcbytes;

	/* Do the region where pixels outside the left edge of the
	 * image are needed */
	curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3);
	while (curstart < curedge) {
	    k2 = k1;
	    curptr = curstart - srcbytes;
	    while (k2->increment != 0) {
		curptr += k2->increment;
		if (curptr < curedge)
		    value = k2->value + 0.0;
		else
		    value = *(float *)curptr + k2->value;
		if (value < *(float *)dstptr)
		    *(float *)dstptr = value;
		k2++;
	    }
	    curstart += srcbytes;
	    dstptr += dstbytes;
	}

	/* Do the pixels where the kernel totally overlaps the image */
	curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	     (Min_(x2, curudp->UdpL_X2-kerntail) - kernel->center_x
	      - curudp->UdpL_X1) * srcbytes;
	while (curstart <= curedge) {
	    k2 = k1;
	    curptr = curstart - srcbytes;
	    while (k2->increment != 0) {
		curptr += k2->increment;
		value = *(float *)curptr + k2->value;
		if (value < *(float *)dstptr)
		    *(float *)dstptr = value;
		k2++;
	    }
	    curstart += srcbytes;
	    dstptr += dstbytes;
	}

	/* Do the region where pixels outside the right edge of the
	 * image are needed */
	curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	    (x2 - curudp->UdpL_X1) * srcbytes;
	while (curstart < curedge) {
	    k2 = k1;
	    curptr = curstart - srcbytes;
	    while (k2->increment != 0) {
		curptr += k2->increment;
		if (curptr <= curedge)
		    value = *(float *)curptr + k2->value;
		else
		    value = k2->value + 0.0;
		if (value < *(float *)dstptr)
		    *(float *)dstptr = value;
		k2++;
	    }
	    curstart += srcbytes;
	    dstptr += dstbytes;
	}

	k1 += (kernel->x_count+1);
    }
}					/* end AreaAddMin */

static void  AreaAddMax(window, center, kernel, dst,  x1, x2)
UdpPtr	     	window[];
int             center;
TmpDatPtr       kernel;
UdpPtr	     	dst;
long int	x1;
long int	x2;
{
    unsigned char       *dststart;
    unsigned char       *dstptr;
    int                 dstbytes;
    int			srcbytes;

    UdpPtr              curudp;
    unsigned char       *curstart;
    unsigned char       *curptr;
    unsigned char       *curedge;

    TmpDatEntryPtr      k1;
    TmpDatEntryPtr      k2;

    int			ky;
    float               value;

    int                 windowsz = kernel->y_count;
    int                 kerntail = kernel->x_count - kernel->center_x - 1;

    srcbytes = window[center]->UdpL_PxlStride >> 3;
    dstbytes  = dst->UdpL_PxlStride >> 3;

    dststart = dst->UdpA_Base + (dst->UdpL_Pos >> 3) +
				(x1 - dst->UdpL_X1) * dstbytes;

    /* Initialize the destination scanline with the first candidate for the
     * minimum value, the value corresponding to the upper left corner of the
     * kernel.
     */
    dstptr = dststart;
    k2 = kernel->array;	     /* 1st value in first row of kernel */

    curudp = window[Circ_(center - kernel->center_y, windowsz)];
    curptr = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	     ((x1 - kernel->center_x) - curudp->UdpL_X1) * srcbytes;
    curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3);

    while (curptr < curedge) {
	/* The relevant source pixel is off the left edge of the image */
	*(float *)dstptr = k2->value + 0.0;
	curptr += srcbytes;
	dstptr += dstbytes;
	}

    curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	                          (x2 - curudp->UdpL_X1) * srcbytes;
    while (curptr < curedge) {
	*(float *)dstptr = k2->value + *(float *)curptr;
	curptr += srcbytes;
	dstptr += dstbytes;
    }

    /*
     * Now calculate the rest, saving the minimum.
     */
    k1 = kernel->array;

    for (ky = -kernel->center_y; ky < windowsz - kernel->center_y; ky++) {
	curudp = window[Circ_(center + ky, windowsz)];
	dstptr = dststart;
	
	curstart = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3)
	    + (x1 - kernel->center_x - curudp->UdpL_X1) * srcbytes;

	/* Do the region where pixels outside the left edge of the
	 * image are needed */
	curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3);
	while (curstart < curedge) {
	    k2 = k1;
	    curptr = curstart - srcbytes;
	    while (k2->increment != 0) {
		curptr += k2->increment;
		if (curptr < curedge)
		    value = k2->value + 0.0;
		else
		    value = *(float *)curptr + k2->value;
		if (value > *(float *)dstptr)
		    *(float *)dstptr = value;
		k2++;
	    }
	    curstart += srcbytes;
	    dstptr += dstbytes;
	}

	/* Do the pixels where the kernel totally overlaps the image */
	curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	     (Min_(x2, curudp->UdpL_X2-kerntail) - kernel->center_x
	      - curudp->UdpL_X1) * srcbytes;
	while (curstart <= curedge) {
	    k2 = k1;
	    curptr = curstart - srcbytes;
	    while (k2->increment != 0) {
		curptr += k2->increment;
		value = *(float *)curptr + k2->value;
		if (value > *(float *)dstptr)
		    *(float *)dstptr = value;
		k2++;
	    }
	    curstart += srcbytes;
	    dstptr += dstbytes;
	}

	/* Do the region where pixels outside the right edge of the
	 * image are needed */
	curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	    (x2 - curudp->UdpL_X1) * srcbytes;
	while (curstart < curedge) {
	    k2 = k1;
	    curptr = curstart - srcbytes;
	    while (k2->increment != 0) {
		curptr += k2->increment;
		if (curptr <= curedge)
		    value = *(float *)curptr + k2->value;
		else
		    value = k2->value + 0.0;
		if (value > *(float *)dstptr)
		    *(float *)dstptr = value;
		k2++;
	    }
	    curstart += srcbytes;
	    dstptr += dstbytes;
	}

	k1 += (kernel->x_count+1);
    }
}					/* end AreaAddMax */

static void  AreaAddSum(window, center, kernel, dst, x1, x2)
UdpPtr	     	window[];
int             center;
TmpDatPtr       kernel;
UdpPtr	     	dst;
long int	x1;
long int	x2;
{
    long int		pixel;

    unsigned char       *dststart;
    unsigned char       *dstptr;
    int                 dstbytes;
    int			srcbytes;

    UdpPtr              curudp;
    unsigned char       *curstart;
    unsigned char       *curptr;
    unsigned char       *curedge;

    TmpDatEntryPtr      k1;
    TmpDatEntryPtr      k2;

    int			ky;

    int                 windowsz = kernel->y_count;
    int                 kerntail = kernel->x_count - kernel->center_x - 1;

    srcbytes = window[center]->UdpL_PxlStride >> 3;
    dstbytes  = dst->UdpL_PxlStride >> 3;

    dststart = dst->UdpA_Base + (dst->UdpL_Pos >> 3) +
				(x1 - dst->UdpL_X1) * dstbytes;

    /* Zero the destination scanline so we can accumulate into it */
    dstptr = dststart;
    for (pixel = x1; pixel <= x2; pixel++) {
	*(float *)dstptr = 0.0;
	dstptr+=dstbytes;
    }

    /*
     * Calculate the summation.
     */
    k1 = kernel->array;

    for (ky = -kernel->center_y; ky < windowsz - kernel->center_y; ky++) {
	curudp = window[Circ_(center + ky, windowsz)];
	dstptr = dststart;
	
	curstart = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3)
	    + (x1 - kernel->center_x - curudp->UdpL_X1) * srcbytes;

	/* Do the region where pixels outside the left edge of the
	 * image are needed */
	curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3);
	while (curstart < curedge) {
	    k2 = k1;
	    curptr = curstart - srcbytes;
	    while (k2->increment != 0) {
		curptr += k2->increment;
		if (curptr < curedge)
		    *(float *)dstptr += (k2->value + 0.0);
		else
		    *(float *)dstptr += (*(float *)curptr + k2->value);
		k2++;
	    }
	    curstart += srcbytes;
	    dstptr += dstbytes;
	}

	/* Do the pixels where the kernel totally overlaps the image */
	curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	     (Min_(x2, curudp->UdpL_X2-kerntail) - kernel->center_x
	      - curudp->UdpL_X1) * srcbytes;
	while (curstart <= curedge) {
	    k2 = k1;
	    curptr = curstart - srcbytes;
	    while (k2->increment != 0) {
		curptr += k2->increment;
		*(float *)dstptr += (*(float *)curptr + k2->value);
		k2++;
	    }
	    curstart += srcbytes;
	    dstptr += dstbytes;
	}

	/* Do the region where pixels outside the right edge of the
	 * image are needed */
	curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	    (x2 - curudp->UdpL_X1) * srcbytes;
	while (curstart < curedge) {
	    k2 = k1;
	    curptr = curstart - srcbytes;
	    while (k2->increment != 0) {
		curptr += k2->increment;
		if (curptr <= curedge)
		    *(float *)dstptr += (*(float *)curptr + k2->value);
		else
		    *(float *)dstptr += (k2->value + 0.0);
		k2++;
	    }
	    curstart += srcbytes;
	    dstptr += dstbytes;
	}

	k1 += (kernel->x_count+1);
    }

}					/* end AreaAddSum */

static void  AreaMultMin(window, center, kernel, dst, x1, x2)
UdpPtr	     	window[];
int             center;
TmpDatPtr       kernel;
UdpPtr	     	dst;
long int	x1;
long int	x2;
{
    unsigned char       *dststart;
    unsigned char       *dstptr;
    int                 dstbytes;
    int			srcbytes;

    UdpPtr              curudp;
    unsigned char       *curstart;
    unsigned char       *curptr;
    unsigned char       *curedge;

    TmpDatEntryPtr      k1;
    TmpDatEntryPtr      k2;

    int			ky;
    float               value;

    int                 windowsz = kernel->y_count;
    int                 kerntail = kernel->x_count - kernel->center_x - 1;

    srcbytes = window[center]->UdpL_PxlStride >> 3;
    dstbytes  = dst->UdpL_PxlStride >> 3;

    dststart = dst->UdpA_Base + (dst->UdpL_Pos >> 3) +
				(x1 - dst->UdpL_X1) * dstbytes;

    /* Initialize the destination scanline with the first candidate for the
     * minimum value, the value corresponding to the upper left corner of the
     * kernel.
     */
    dstptr = dststart;
    k2 = kernel->array;	     /* 1st value in first row of kernel */

    curudp = window[Circ_(center - kernel->center_y, windowsz)];
    curptr = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	     ((x1 - kernel->center_x) - curudp->UdpL_X1) * srcbytes;
    curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3);

    while (curptr < curedge) {
	/* The relevant source pixel is off the left edge of the image */
	*(float *)dstptr = 0.0 * k2->value;
	curptr += srcbytes;
	dstptr += dstbytes;
	}

    curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	                          (x2 - curudp->UdpL_X1) * srcbytes;
    while (curptr < curedge) {
	*(float *)dstptr = k2->value * *(float *)curptr;
	curptr += srcbytes;
	dstptr += dstbytes;
    }

    /*
     * Now calculate the rest, saving the minimum.
     */
    k1 = kernel->array;

    for (ky = -kernel->center_y; ky < windowsz - kernel->center_y; ky++) {
	curudp = window[Circ_(center + ky, windowsz)];
	dstptr = dststart;
	
	curstart = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3)
	    + (x1 - kernel->center_x - curudp->UdpL_X1) * srcbytes;

	/* Do the region where pixels outside the left edge of the
	 * image are needed */
	curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3);
	while (curstart < curedge) {
	    k2 = k1;
	    curptr = curstart - srcbytes;
	    while (k2->increment != 0) {
		curptr += k2->increment;
		if (curptr < curedge)
		    value = 0.0 * k2->value;
		else
		    value = *(float *)curptr * k2->value;
		if (value < *(float *)dstptr)
		    *(float *)dstptr = value;
		k2++;
	    }
	    curstart += srcbytes;
	    dstptr += dstbytes;
	}

	/* Do the pixels where the kernel totally overlaps the image */
	curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	     (Min_(x2, curudp->UdpL_X2-kerntail) - kernel->center_x
	      - curudp->UdpL_X1) * srcbytes;
	while (curstart <= curedge) {
	    k2 = k1;
	    curptr = curstart - srcbytes;
	    while (k2->increment != 0) {
		curptr += k2->increment;
		value = *(float *)curptr * k2->value;
		if (value < *(float *)dstptr)
		    *(float *)dstptr = value;
		k2++;
	    }
	    curstart += srcbytes;
	    dstptr += dstbytes;
	}

	/* Do the region where pixels outside the right edge of the
	 * image are needed */
	curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	    (x2 - curudp->UdpL_X1) * srcbytes;
	while (curstart < curedge) {
	    k2 = k1;
	    curptr = curstart - srcbytes;
	    while (k2->increment != 0) {
		curptr += k2->increment;
		if (curptr <= curedge)
		    value = *(float *)curptr * k2->value;
		else
		    value = 0.0 * k2->value ;
		if (value < *(float *)dstptr)
		    *(float *)dstptr = value;
		k2++;
	    }
	    curstart += srcbytes;
	    dstptr += dstbytes;
	}

	k1 += (kernel->x_count+1);
    }
}					/* end AreaMultMin */

static void  AreaMultMax(window, center, kernel, dst,  x1, x2)
UdpPtr	     	window[];
int             center;
TmpDatPtr       kernel;
UdpPtr	     	dst;
long int	x1;
long int	x2;
{
    unsigned char       *dststart;
    unsigned char       *dstptr;
    int                 dstbytes;
    int			srcbytes;

    UdpPtr              curudp;
    unsigned char       *curstart;
    unsigned char       *curptr;
    unsigned char       *curedge;

    TmpDatEntryPtr      k1;
    TmpDatEntryPtr      k2;

    int			ky;
    float               value;

    int                 windowsz = kernel->y_count;
    int                 kerntail = kernel->x_count - kernel->center_x - 1;

    srcbytes = window[center]->UdpL_PxlStride >> 3;
    dstbytes  = dst->UdpL_PxlStride >> 3;

    dststart = dst->UdpA_Base + (dst->UdpL_Pos >> 3) +
				(x1 - dst->UdpL_X1) * dstbytes;

    /* Initialize the destination scanline with the first candidate for the
     * minimum value, the value corresponding to the upper left corner of the
     * kernel.
     */
    dstptr = dststart;
    k2 = kernel->array;	     /* 1st value in first row of kernel */

    curudp = window[Circ_(center - kernel->center_y, windowsz)];
    curptr = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	     ((x1 - kernel->center_x) - curudp->UdpL_X1) * srcbytes;
    curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3);

    while (curptr < curedge) {
	/* The relevant source pixel is off the left edge of the image */
	*(float *)dstptr = 0.0 * k2->value;
	curptr += srcbytes;
	dstptr += dstbytes;
	}

    curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	                          (x2 - curudp->UdpL_X1) * srcbytes;
    while (curptr < curedge) {
	*(float *)dstptr = k2->value * *(float *)curptr;
	curptr += srcbytes;
	dstptr += dstbytes;
    }

    /*
     * Now calculate the rest, saving the minimum.
     */
    k1 = kernel->array;

    for (ky = -kernel->center_y; ky < windowsz - kernel->center_y; ky++) {
	curudp = window[Circ_(center + ky, windowsz)];
	dstptr = dststart;
	
	curstart = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3)
	    + (x1 - kernel->center_x - curudp->UdpL_X1) * srcbytes;

	/* Do the region where pixels outside the left edge of the
	 * image are needed */
	curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3);
	while (curstart < curedge) {
	    k2 = k1;
	    curptr = curstart - srcbytes;
	    while (k2->increment != 0) {
		curptr += k2->increment;
		if (curptr < curedge)
		    value = 0.0 * k2->value;
		else
		    value = *(float *)curptr * k2->value;
		if (value > *(float *)dstptr)
		    *(float *)dstptr = value;
		k2++;
	    }
	    curstart += srcbytes;
	    dstptr += dstbytes;
	}

	/* Do the pixels where the kernel totally overlaps the image */
	curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	     (Min_(x2, curudp->UdpL_X2-kerntail) - kernel->center_x
	      - curudp->UdpL_X1) * srcbytes;
	while (curstart <= curedge) {
	    k2 = k1;
	    curptr = curstart - srcbytes;
	    while (k2->increment != 0) {
		curptr += k2->increment;
		value = *(float *)curptr * k2->value;
		if (value > *(float *)dstptr)
		    *(float *)dstptr = value;
		k2++;
	    }
	    curstart += srcbytes;
	    dstptr += dstbytes;
	}

	/* Do the region where pixels outside the right edge of the
	 * image are needed */
	curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	    (x2 - curudp->UdpL_X1) * srcbytes;
	while (curstart < curedge) {
	    k2 = k1;
	    curptr = curstart - srcbytes;
	    while (k2->increment != 0) {
		curptr += k2->increment;
		if (curptr <= curedge)
		    value = *(float *)curptr * k2->value;
		else
		    value = 0.0 * k2->value ;
		if (value > *(float *)dstptr)
		    *(float *)dstptr = value;
		k2++;
	    }
	    curstart += srcbytes;
	    dstptr += dstbytes;
	}

	k1 += (kernel->x_count+1);
    }
}					/* end AreaMultMax */

static void  AreaMultSum(window, center, kernel, dst, x1, x2)
UdpPtr	     	window[];
int             center;
TmpDatPtr          kernel;
UdpPtr	     	dst;
long int	x1;
long int	x2;
{
    long int		pixel;

    unsigned char       *dststart;
    unsigned char       *dstptr;
    int                 dstbytes;
    int			srcbytes;

    UdpPtr              curudp;
    unsigned char       *curstart;
    unsigned char       *curptr;
    unsigned char       *curedge;

    TmpDatEntryPtr      k1;
    TmpDatEntryPtr      k2;

    int			ky;

    int                 windowsz = kernel->y_count;
    int                 kerntail = kernel->x_count - kernel->center_x - 1;

    srcbytes = window[center]->UdpL_PxlStride >> 3;
    dstbytes  = dst->UdpL_PxlStride >> 3;

    dststart = dst->UdpA_Base + (dst->UdpL_Pos >> 3) +
				(x1 - dst->UdpL_X1) * dstbytes;

    /* Zero the destination scanline so we can accumulate into it */
    dstptr = dststart;
    for (pixel = x1; pixel <= x2; pixel++) {
	*(float *)dstptr = 0.0;
	dstptr+=dstbytes;
    }

    /*
     * Calculate the summation.
     */
    k1 = kernel->array;

    for (ky = -kernel->center_y; ky < windowsz - kernel->center_y; ky++) {
	curudp = window[Circ_(center + ky, windowsz)];
	dstptr = dststart;
	
	curstart = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3)
	    + (x1 - kernel->center_x - curudp->UdpL_X1) * srcbytes;

	/* Do the region where pixels outside the left edge of the
	 * image are needed */
	curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3);
	while (curstart < curedge) {
	    k2 = k1;
	    curptr = curstart - srcbytes;
	    while (k2->increment != 0) {
		curptr += k2->increment;
		if (curptr < curedge)
		    *(float *)dstptr += (k2->value * 0.0);
		else
		    *(float *)dstptr += (*(float *)curptr * k2->value);
		k2++;
	    }
	    curstart += srcbytes;
	    dstptr += dstbytes;
	}

	/* Do the pixels where the kernel totally overlaps the image */
	curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	     (Min_(x2, curudp->UdpL_X2-kerntail) - kernel->center_x
	      - curudp->UdpL_X1) * srcbytes;
	while (curstart <= curedge) {
	    k2 = k1;
	    curptr = curstart - srcbytes;
	    while (k2->increment != 0) {
		curptr += k2->increment;
		*(float *)dstptr += (*(float *)curptr * k2->value);
		k2++;
	    }
	    curstart += srcbytes;
	    dstptr += dstbytes;
	}

	/* Do the region where pixels outside the right edge of the
	 * image are needed */
	curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	    (x2 - curudp->UdpL_X1) * srcbytes;
	while (curstart < curedge) {
	    k2 = k1;
	    curptr = curstart - srcbytes;
	    while (k2->increment != 0) {
		curptr += k2->increment;
		if (curptr <= curedge)
		    *(float *)dstptr += (*(float *)curptr * k2->value);
		else
		    *(float *)dstptr += (k2->value * 0.0);
		k2++;
	    }
	    curstart += srcbytes;
	    dstptr += dstbytes;
	}

	k1 += (kernel->x_count+1);
    }
}					/* end AreaMultSum */

/*****************************************************************************
**  Area processing routines including CPP
**
**  FUNCTIONAL DESCRIPTION:
**
**	These routines the actual area operation over a selected
**      portion of a scanline.  A separate routine is used for each
**      type of operation.
**
**  FORMAL PARAMETERS:
**
**	window  - Array of pointers to Udps containing the scanlines
**                needed to process the current scanline.
**      center  - Index into window of the pointer to the current scanline.
**      kernel  - Pointer to a TmpDatRec describing the area kernel.
**      dst     - Pointer to a UDP describing the destination buffer.
**      cpp     - Pointer to a UDP describing the Control Processing Plane.
**      y       - The y coordinate of the beginning of the CPP or ROI.
**      x1      - The x coordinate of the left endpoint of the CPP, or ROI.
**      x2      - The x coordinate of the right endpoint of the CPP, or ROI.
**
*****************************************************************************/
static void  AreaAddMinCpp(window, center, kernel, dst, cpp,  x1, x2)
UdpPtr	     	window[];
int             center;
TmpDatPtr       kernel;
UdpPtr	     	dst;
UdpPtr		cpp;
long int	x1;
long int	x2;
{
    unsigned char       *dststart;
    unsigned char       *dstptr;
    int                 dstbytes;
    int			srcbytes;

    UdpPtr              curudp;
    unsigned char       *curstart;
    unsigned char       *curptr;
    unsigned char       *curedge;
    unsigned char       *srcptr;

    unsigned char	*cppstart;
    unsigned char       *cppptr;
    
    TmpDatEntryPtr      k1;
    TmpDatEntryPtr      k2;

    int			ky;
    float               value;

    int                 windowsz = kernel->y_count;
    int                 kerntail = kernel->x_count - kernel->center_x - 1;

    srcbytes = window[center]->UdpL_PxlStride >> 3;
    dstbytes  = dst->UdpL_PxlStride >> 3;

    dststart = dst->UdpA_Base + (dst->UdpL_Pos >> 3) +
				(x1 - dst->UdpL_X1) * dstbytes;
    cppstart = cpp->UdpA_Base + (cpp->UdpL_Pos >> 3);


    /* Initialize the destination scanline with the first candidate for the
     * minimum value, the value corresponding to the upper left corner of the
     * kernel.
     */
    dstptr = dststart;
    k2 = kernel->array;	     /* 1st value in first row of kernel */

    curudp = window[Circ_(center - kernel->center_y, windowsz)];
    curptr = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	     ((x1 - kernel->center_x) - curudp->UdpL_X1) * srcbytes;
    cppptr = cppstart;
    srcptr = window[center]->UdpA_Base +
	      (window[center]->UdpL_Pos >> 3) +
	      (x1 - window[center]->UdpL_X1) * srcbytes;
    curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3);

    while (curptr < curedge) {
	/* The relevant source pixel is off the left edge of the image */
	if (*cppptr)
	    *(float *)dstptr = k2->value + 0.0;
	else
	    *(float *)dstptr = *(float *)srcptr;
	curptr += srcbytes;
	dstptr += dstbytes;
	cppptr++;
	srcptr += srcbytes;
	}

    curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	                          (x2 - curudp->UdpL_X1) * srcbytes;
    while (curptr < curedge) {
	if (*cppptr)
	    *(float *)dstptr = k2->value + *(float *)curptr;
	else
	    *(float *)dstptr = *(float *)srcptr;
	curptr += srcbytes;
	dstptr += dstbytes;
	cppptr++;
	srcptr += srcbytes;
    }

    /*
     * Now calculate the rest, saving the minimum.
     */
    k1 = kernel->array;

    for (ky = -kernel->center_y; ky < windowsz - kernel->center_y; ky++) {
	curudp = window[Circ_(center + ky, windowsz)];
	dstptr = dststart;
	cppptr = cppstart;
	
	curstart = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3)
	    + (x1 - kernel->center_x - curudp->UdpL_X1) * srcbytes;

	/* Do the region where pixels outside the left edge of the
	 * image are needed */
	curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3);
	while (curstart < curedge) {
	    k2 = k1;
	    curptr = curstart - srcbytes;
	    if (*cppptr) {
		while (k2->increment != 0) {
		    curptr += k2->increment;
		    if (curptr < curedge)
			value = k2->value + 0.0;
		    else
			value = *(float *)curptr + k2->value;
		    if (value < *(float *)dstptr)
			*(float *)dstptr = value;
		k2++;
		}
	    }
	    curstart += srcbytes;
	    dstptr += dstbytes;
	    cppptr++;
	}

	/* Do the pixels where the kernel totally overlaps the image */
	curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	     (Min_(x2, curudp->UdpL_X2-kerntail) - kernel->center_x
	      - curudp->UdpL_X1) * srcbytes;
	while (curstart <= curedge) {
	    k2 = k1;
	    curptr = curstart - srcbytes;
	    if (*cppptr) {
		while (k2->increment != 0) {
		    curptr += k2->increment;
		    value = *(float *)curptr + k2->value;
		    if (value < *(float *)dstptr)
			*(float *)dstptr = value;
		    k2++;
		}
	    }
	    curstart += srcbytes;
	    dstptr += dstbytes;
	    cppptr++;
	}

	/* Do the region where pixels outside the right edge of the
	 * image are needed */
	curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	    (x2 - curudp->UdpL_X1) * srcbytes;
	while (curstart < curedge) {
	    k2 = k1;
	    curptr = curstart - srcbytes;
	    if (*cppptr) {
		while (k2->increment != 0) {
		    curptr += k2->increment;
		    if (curptr <= curedge)
			value = *(float *)curptr + k2->value;
		    else
			value = k2->value + 0.0;
		    if (value < *(float *)dstptr)
			*(float *)dstptr = value;
		    k2++;
		}
	    }
	    curstart += srcbytes;
	    dstptr += dstbytes;
	    cppptr++;
	}

	k1 += (kernel->x_count+1);
    }
}					/* end AreaAddMinCpp */

static void  AreaAddMaxCpp(window, center, kernel, dst, cpp,  x1, x2)
UdpPtr	     	window[];
int             center;
TmpDatPtr          kernel;
UdpPtr	     	dst;
UdpPtr		cpp;
long int	x1;
long int	x2;
{
    unsigned char       *dststart;
    unsigned char       *dstptr;
    int                 dstbytes;
    int			srcbytes;

    UdpPtr              curudp;
    unsigned char       *curstart;
    unsigned char       *curptr;
    unsigned char       *curedge;
    unsigned char       *srcptr;

    unsigned char	*cppstart;
    unsigned char       *cppptr;

    TmpDatEntryPtr      k1;
    TmpDatEntryPtr      k2;

    int			ky;
    float               value;

    int                 windowsz = kernel->y_count;
    int                 kerntail = kernel->x_count - kernel->center_x - 1;

    srcbytes = window[center]->UdpL_PxlStride >> 3;
    dstbytes  = dst->UdpL_PxlStride >> 3;

    dststart = dst->UdpA_Base + (dst->UdpL_Pos >> 3) +
				(x1 - dst->UdpL_X1) * dstbytes;
    cppstart = cpp->UdpA_Base + (cpp->UdpL_Pos >> 3);

    /* Initialize the destination scanline with the first candidate for the
     * minimum value, the value corresponding to the upper left corner of the
     * kernel.
     */
    dstptr = dststart;
    k2 = kernel->array;	     /* 1st value in first row of kernel */

    curudp = window[Circ_(center - kernel->center_y, windowsz)];
    curptr = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	     ((x1 - kernel->center_x) - curudp->UdpL_X1) * srcbytes;
    cppptr = cppstart;
    srcptr = window[center]->UdpA_Base +
	      (window[center]->UdpL_Pos >> 3) +
	      (x1 - window[center]->UdpL_X1) * srcbytes;
    curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3);

    while (curptr < curedge) {
	/* The relevant source pixel is off the left edge of the image */
	*(float *)dstptr = k2->value + 0.0;
	curptr += srcbytes;
	dstptr += dstbytes;
	cppptr++;
	srcptr += srcbytes;
	}

    curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	                          (x2 - curudp->UdpL_X1) * srcbytes;
    while (curptr < curedge) {
	if (*cppptr)
	    *(float *)dstptr = k2->value + *(float *)curptr;
	else
	    *(float *)dstptr = *(float *)srcptr;
	curptr += srcbytes;
	dstptr += dstbytes;
	cppptr++;
	srcptr += srcbytes;
    }

    /*
     * Now calculate the rest, saving the minimum.
     */
    k1 = kernel->array;

    for (ky = -kernel->center_y; ky < windowsz - kernel->center_y; ky++) {
	curudp = window[Circ_(center + ky, windowsz)];
	dstptr = dststart;
	cppptr = cppstart;
	
	curstart = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3)
	    + (x1 - kernel->center_x - curudp->UdpL_X1) * srcbytes;

	/* Do the region where pixels outside the left edge of the
	 * image are needed */
	curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3);
	while (curstart < curedge) {
	    k2 = k1;
	    curptr = curstart - srcbytes;
	    if (*cppptr) {
		while (k2->increment != 0) {
		    curptr += k2->increment;
		    if (curptr < curedge)
			value = k2->value + 0.0;
		    else
			value = *(float *)curptr + k2->value;
		    if (value > *(float *)dstptr)
			*(float *)dstptr = value;
		    k2++;
		}
	    }
	    curstart += srcbytes;
	    dstptr += dstbytes;
	    cppptr++;
	}

	/* Do the pixels where the kernel totally overlaps the image */
	curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	     (Min_(x2, curudp->UdpL_X2-kerntail) - kernel->center_x
	      - curudp->UdpL_X1) * srcbytes;
	while (curstart <= curedge) {
	    k2 = k1;
	    curptr = curstart - srcbytes;
	    if (*cppptr) {
		while (k2->increment != 0) {
		    curptr += k2->increment;
		    value = *(float *)curptr + k2->value;
		    if (value > *(float *)dstptr)
			*(float *)dstptr = value;
		    k2++;
		}
	    }
	    curstart += srcbytes;
	    dstptr += dstbytes;
	    cppptr++;
	}

	/* Do the region where pixels outside the right edge of the
	 * image are needed */
	curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	    (x2 - curudp->UdpL_X1) * srcbytes;
	while (curstart < curedge) {
	    k2 = k1;
	    curptr = curstart - srcbytes;
	    if (*cppptr) {
		while (k2->increment != 0) {
		    curptr += k2->increment;
		    if (curptr <= curedge)
			value = *(float *)curptr + k2->value;
		    else
			value = k2->value + 0.0;
		    if (value > *(float *)dstptr)
			*(float *)dstptr = value;
		    k2++;
		}
	    }
	    curstart += srcbytes;
	    dstptr += dstbytes;
	    cppptr++;
	}

	k1 += (kernel->x_count+1);
    }
}					/* end AreaAddMaxCpp */

static void  AreaAddSumCpp(window, center, kernel, dst, cpp,  x1, x2)
UdpPtr	     	window[];
int             center;
TmpDatPtr       kernel;
UdpPtr	     	dst;
UdpPtr		cpp;
long int	x1;
long int	x2;
{
    long int		pixel;

    unsigned char       *dststart;
    unsigned char       *dstptr;
    int                 dstbytes;
    int			srcbytes;

    UdpPtr              curudp;
    unsigned char       *curstart;
    unsigned char       *curptr;
    unsigned char       *curedge;
    unsigned char       *srcptr;

    unsigned char	*cppstart;
    unsigned char       *cppptr;

    TmpDatEntryPtr      k1;
    TmpDatEntryPtr      k2;

    int			ky;

    int                 windowsz = kernel->y_count;
    int                 kerntail = kernel->x_count - kernel->center_x - 1;

    srcbytes = window[center]->UdpL_PxlStride >> 3;
    dstbytes  = dst->UdpL_PxlStride >> 3;

    dststart = dst->UdpA_Base + (dst->UdpL_Pos >> 3) +
				(x1 - dst->UdpL_X1) * dstbytes;
    cppstart = cpp->UdpA_Base + (cpp->UdpL_Pos >> 3);

    /* Zero the destination scanline so we can accumulate into it.
     * Copy the source data to those points not selected by the CPP */
    dstptr = dststart;
    cppptr = cppstart;
    srcptr = window[center]->UdpA_Base +
	      (window[center]->UdpL_Pos >> 3) +
	      (x1 - window[center]->UdpL_X1) * srcbytes;

    for (pixel = x1; pixel <= x2; pixel++) {
	*(float *)dstptr = (*cppptr) ? 0.0 
	                             : *(float *)srcptr;
	dstptr+=dstbytes;
	srcptr+=srcbytes;
	cppptr++;
    }

    /*
     * Calculate the summation.
     */
    k1 = kernel->array;

    for (ky = -kernel->center_y; ky < windowsz - kernel->center_y; ky++) {
	curudp = window[Circ_(center + ky, windowsz)];
	dstptr = dststart;
	cppptr = cppstart;
	
	curstart = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3)
	    + (x1 - kernel->center_x - curudp->UdpL_X1) * srcbytes;

	/* Do the region where pixels outside the left edge of the
	 * image are needed */
	curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3);
	while (curstart < curedge) {
	    k2 = k1;
	    curptr = curstart - srcbytes;
	    if (*cppptr) {
		while (k2->increment != 0) {
		    curptr += k2->increment;
		    if (curptr < curedge)
			*(float *)dstptr += (k2->value + 0.0);
		    else
			*(float *)dstptr += (*(float *)curptr + k2->value);
		    k2++;
		}
	    }
	    curstart += srcbytes;
	    dstptr += dstbytes;
	    cppptr++;
	}

	/* Do the pixels where the kernel totally overlaps the image */
	curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	     (Min_(x2, curudp->UdpL_X2-kerntail) - kernel->center_x
	      - curudp->UdpL_X1) * srcbytes;
	while (curstart <= curedge) {
	    k2 = k1;
	    curptr = curstart - srcbytes;
	    if (*cppptr) {
		while (k2->increment != 0) {
		    curptr += k2->increment;
		    *(float *)dstptr += (*(float *)curptr + k2->value);
		    k2++;
		}
	    }
	    curstart += srcbytes;
	    dstptr += dstbytes;
	    cppptr++;
	}

	/* Do the region where pixels outside the right edge of the
	 * image are needed */
	curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	    (x2 - curudp->UdpL_X1) * srcbytes;
	while (curstart < curedge) {
	    k2 = k1;
	    curptr = curstart - srcbytes;
	    if (*cppptr) {
		while (k2->increment != 0) {
		    curptr += k2->increment;
		    if (curptr <= curedge)
			*(float *)dstptr += (*(float *)curptr + k2->value);
		    else
			*(float *)dstptr += (k2->value + 0.0);
		    k2++;
		}
	    }
	    curstart += srcbytes;
	    dstptr += dstbytes;
	    cppptr++;
	}

	k1 += (kernel->x_count+1);
    }

}					/* end AreaAddSumCpp */

static void  AreaMultMinCpp(window, center, kernel, dst, cpp,  x1, x2)
UdpPtr	     	window[];
int             center;
TmpDatPtr          kernel;
UdpPtr	     	dst;
UdpPtr		cpp;
long int	x1;
long int	x2;
{
    unsigned char       *dststart;
    unsigned char       *dstptr;
    int                 dstbytes;
    int			srcbytes;

    UdpPtr              curudp;
    unsigned char       *curstart;
    unsigned char       *curptr;
    unsigned char       *curedge;
    unsigned char       *srcptr;

    unsigned char	*cppstart;
    unsigned char       *cppptr;
    
    TmpDatEntryPtr      k1;
    TmpDatEntryPtr      k2;

    int			ky;
    float               value;

    int                 windowsz = kernel->y_count;
    int                 kerntail = kernel->x_count - kernel->center_x - 1;

    srcbytes = window[center]->UdpL_PxlStride >> 3;
    dstbytes  = dst->UdpL_PxlStride >> 3;

    dststart = dst->UdpA_Base + (dst->UdpL_Pos >> 3) +
				(x1 - dst->UdpL_X1) * dstbytes;
    cppstart = cpp->UdpA_Base + (cpp->UdpL_Pos >> 3);


    /* Initialize the destination scanline with the first candidate for the
     * minimum value, the value corresponding to the upper left corner of the
     * kernel.
     */
    dstptr = dststart;
    k2 = kernel->array;	     /* 1st value in first row of kernel */

    curudp = window[Circ_(center - kernel->center_y, windowsz)];
    curptr = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	     ((x1 - kernel->center_x) - curudp->UdpL_X1) * srcbytes;
    cppptr = cppstart;
    srcptr = window[center]->UdpA_Base +
	      (window[center]->UdpL_Pos >> 3) +
	      (x1 - window[center]->UdpL_X1) * srcbytes;
    curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3);

    while (curptr < curedge) {
	/* The relevant source pixel is off the left edge of the image */
	if (*cppptr)
	    *(float *)dstptr = k2->value * 0.0;
	else
	    *(float *)dstptr = *(float *)srcptr;
	curptr += srcbytes;
	dstptr += dstbytes;
	cppptr++;
	srcptr += srcbytes;
	}

    curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	                          (x2 - curudp->UdpL_X1) * srcbytes;
    while (curptr < curedge) {
	if (*cppptr)
	    *(float *)dstptr = k2->value * *(float *)curptr;
	else
	    *(float *)dstptr = *(float *)srcptr;
	curptr += srcbytes;
	dstptr += dstbytes;
	cppptr++;
	srcptr += srcbytes;
    }

    /*
     * Now calculate the rest, saving the minimum.
     */
    k1 = kernel->array;

    for (ky = -kernel->center_y; ky < windowsz - kernel->center_y; ky++) {
	curudp = window[Circ_(center + ky, windowsz)];
	dstptr = dststart;
	cppptr = cppstart;
	
	curstart = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3)
	    + (x1 - kernel->center_x - curudp->UdpL_X1) * srcbytes;

	/* Do the region where pixels outside the left edge of the
	 * image are needed */
	curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3);
	while (curstart < curedge) {
	    k2 = k1;
	    curptr = curstart - srcbytes;
	    if (*cppptr) {
		while (k2->increment != 0) {
		    curptr += k2->increment;
		    if (curptr < curedge)
			value = k2->value * 0.0;
		    else
			value = *(float *)curptr * k2->value;
		    if (value < *(float *)dstptr)
			*(float *)dstptr = value;
		k2++;
		}
	    }
	    curstart += srcbytes;
	    dstptr += dstbytes;
	    cppptr++;
	}

	/* Do the pixels where the kernel totally overlaps the image */
	curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	     (Min_(x2, curudp->UdpL_X2-kerntail) - kernel->center_x
	      - curudp->UdpL_X1) * srcbytes;
	while (curstart <= curedge) {
	    k2 = k1;
	    curptr = curstart - srcbytes;
	    if (*cppptr) {
		while (k2->increment != 0) {
		    curptr += k2->increment;
		    value = *(float *)curptr * k2->value;
		    if (value < *(float *)dstptr)
			*(float *)dstptr = value;
		    k2++;
		}
	    }
	    curstart += srcbytes;
	    dstptr += dstbytes;
	    cppptr++;
	}

	/* Do the region where pixels outside the right edge of the
	 * image are needed */
	curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	    (x2 - curudp->UdpL_X1) * srcbytes;
	while (curstart < curedge) {
	    k2 = k1;
	    curptr = curstart - srcbytes;
	    if (*cppptr) {
		while (k2->increment != 0) {
		    curptr += k2->increment;
		    if (curptr <= curedge)
			value = *(float *)curptr * k2->value;
		    else
			value = k2->value * 0.0;
		    if (value < *(float *)dstptr)
			*(float *)dstptr = value;
		    k2++;
		}
	    }
	    curstart += srcbytes;
	    dstptr += dstbytes;
	    cppptr++;
	}

	k1 += (kernel->x_count+1);
    }

}					/* end AreaMultMinCpp */

static void  AreaMultMaxCpp(window, center, kernel, dst, cpp,  x1, x2)
UdpPtr	     	window[];
int             center;
TmpDatPtr       kernel;
UdpPtr	     	dst;
UdpPtr		cpp;
long int	x1;
long int	x2;
{
    unsigned char       *dststart;
    unsigned char       *dstptr;
    int                 dstbytes;
    int			srcbytes;

    UdpPtr              curudp;
    unsigned char       *curstart;
    unsigned char       *curptr;
    unsigned char       *curedge;
    unsigned char       *srcptr;

    unsigned char	*cppstart;
    unsigned char       *cppptr;

    TmpDatEntryPtr      k1;
    TmpDatEntryPtr      k2;

    int			ky;
    float               value;

    int                 windowsz = kernel->y_count;
    int                 kerntail = kernel->x_count - kernel->center_x - 1;

    srcbytes = window[center]->UdpL_PxlStride >> 3;
    dstbytes  = dst->UdpL_PxlStride >> 3;

    dststart = dst->UdpA_Base + (dst->UdpL_Pos >> 3) +
				(x1 - dst->UdpL_X1) * dstbytes;
    cppstart = cpp->UdpA_Base + (cpp->UdpL_Pos >> 3);

    /* Initialize the destination scanline with the first candidate for the
     * minimum value, the value corresponding to the upper left corner of the
     * kernel.
     */
    dstptr = dststart;
    k2 = kernel->array;	     /* 1st value in first row of kernel */

    curudp = window[Circ_(center - kernel->center_y, windowsz)];
    curptr = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	     ((x1 - kernel->center_x) - curudp->UdpL_X1) * srcbytes;
    cppptr = cppstart;
    srcptr = window[center]->UdpA_Base +
	      (window[center]->UdpL_Pos >> 3) +
	      (x1 - window[center]->UdpL_X1) * srcbytes;
    curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3);

    while (curptr < curedge) {
	/* The relevant source pixel is off the left edge of the image */
	*(float *)dstptr = k2->value * 0.0;
	curptr += srcbytes;
	dstptr += dstbytes;
	cppptr++;
	srcptr += srcbytes;
	}

    curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	                          (x2 - curudp->UdpL_X1) * srcbytes;
    while (curptr < curedge) {
	if (*cppptr)
	    *(float *)dstptr = k2->value * *(float *)curptr;
	else
	    *(float *)dstptr = *(float *)srcptr;
	curptr += srcbytes;
	dstptr += dstbytes;
	cppptr++;
	srcptr += srcbytes;
    }

    /*
     * Now calculate the rest, saving the minimum.
     */
    k1 = kernel->array;

    for (ky = -kernel->center_y; ky < windowsz - kernel->center_y; ky++) {
	curudp = window[Circ_(center + ky, windowsz)];
	dstptr = dststart;
	cppptr = cppstart;
	
	curstart = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3)
	    + (x1 - kernel->center_x - curudp->UdpL_X1) * srcbytes;

	/* Do the region where pixels outside the left edge of the
	 * image are needed */
	curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3);
	while (curstart < curedge) {
	    k2 = k1;
	    curptr = curstart - srcbytes;
	    if (*cppptr) {
		while (k2->increment != 0) {
		    curptr += k2->increment;
		    if (curptr < curedge)
			value = k2->value * 0.0;
		    else
			value = *(float *)curptr * k2->value;
		    if (value > *(float *)dstptr)
			*(float *)dstptr = value;
		    k2++;
		}
	    }
	    curstart += srcbytes;
	    dstptr += dstbytes;
	    cppptr++;
	}

	/* Do the pixels where the kernel totally overlaps the image */
	curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	     (Min_(x2, curudp->UdpL_X2-kerntail) - kernel->center_x
	      - curudp->UdpL_X1) * srcbytes;
	while (curstart <= curedge) {
	    k2 = k1;
	    curptr = curstart - srcbytes;
	    if (*cppptr) {
		while (k2->increment != 0) {
		    curptr += k2->increment;
		    value = *(float *)curptr * k2->value;
		    if (value > *(float *)dstptr)
			*(float *)dstptr = value;
		    k2++;
		}
	    }
	    curstart += srcbytes;
	    dstptr += dstbytes;
	    cppptr++;
	}

	/* Do the region where pixels outside the right edge of the
	 * image are needed */
	curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	    (x2 - curudp->UdpL_X1) * srcbytes;
	while (curstart < curedge) {
	    k2 = k1;
	    curptr = curstart - srcbytes;
	    if (*cppptr) {
		while (k2->increment != 0) {
		    curptr += k2->increment;
		    if (curptr <= curedge)
			value = *(float *)curptr * k2->value;
		    else
			value = k2->value * 0.0;
		    if (value > *(float *)dstptr)
			*(float *)dstptr = value;
		    k2++;
		}
	    }
	    curstart += srcbytes;
	    dstptr += dstbytes;
	    cppptr++;
	}

	k1 += (kernel->x_count+1);
    }
}					/* end AreaMultMaxCpp */

static void  AreaMultSumCpp(window, center, kernel, dst, cpp, x1, x2)
UdpPtr	     	window[];
int             center;
TmpDatPtr       kernel;
UdpPtr	     	dst;
UdpPtr		cpp;
long int	x1;
long int	x2;
{
    long int		pixel;

    unsigned char       *dststart;
    unsigned char       *dstptr;
    int                 dstbytes;
    int			srcbytes;

    UdpPtr              curudp;
    unsigned char       *curstart;
    unsigned char       *curptr;
    unsigned char       *curedge;
    unsigned char       *srcptr;

    unsigned char	*cppstart;
    unsigned char       *cppptr;

    TmpDatEntryPtr      k1;
    TmpDatEntryPtr      k2;

    int			ky;

    int                 windowsz = kernel->y_count;
    int                 kerntail = kernel->x_count - kernel->center_x - 1;

    srcbytes = window[center]->UdpL_PxlStride >> 3;
    dstbytes  = dst->UdpL_PxlStride >> 3;

    dststart = dst->UdpA_Base + (dst->UdpL_Pos >> 3) +
				(x1 - dst->UdpL_X1) * dstbytes;
    cppstart = cpp->UdpA_Base + (cpp->UdpL_Pos >> 3);

    /* Zero the destination scanline so we can accumulate into it.
     * Copy the source data to those points not selected by the CPP */
    cppptr = cppstart;
    dstptr = dststart;
    srcptr = window[center]->UdpA_Base +
	      (window[center]->UdpL_Pos >> 3) +
	      (x1 - window[center]->UdpL_X1) * srcbytes;

    for (pixel = x1; pixel <= x2; pixel++) {
	*(float *)dstptr = (*cppptr) ? 0.0 
	                             : *(float *)srcptr;
	dstptr+=dstbytes;
	srcptr+=srcbytes;
	cppptr++;
    }

    /*
     * Calculate the summation.
     */
    k1 = kernel->array;

    for (ky = -kernel->center_y; ky < windowsz - kernel->center_y; ky++) {
	curudp = window[Circ_(center + ky, windowsz)];
	dstptr = dststart;
	cppptr = cppstart;
	
	curstart = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3)
	    + (x1 - kernel->center_x - curudp->UdpL_X1) * srcbytes;

	/* Do the region where pixels outside the left edge of the
	 * image are needed */
	curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3);
	while (curstart < curedge) {
	    k2 = k1;
	    curptr = curstart - srcbytes;
	    if (*cppptr) {
		while (k2->increment != 0) {
		    curptr += k2->increment;
		    if (curptr < curedge)
			*(float *)dstptr += (k2->value * 0.0);
		    else
			*(float *)dstptr += (*(float *)curptr * k2->value);
		    k2++;
		}
	    }
	    curstart += srcbytes;
	    dstptr += dstbytes;
	    cppptr++;
	}

	/* Do the pixels where the kernel totally overlaps the image */
	curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	     (Min_(x2, curudp->UdpL_X2-kerntail) - kernel->center_x
	      - curudp->UdpL_X1) * srcbytes;
	while (curstart <= curedge) {
	    k2 = k1;
	    curptr = curstart - srcbytes;
	    if (*cppptr) {
		while (k2->increment != 0) {
		    curptr += k2->increment;
		    *(float *)dstptr += (*(float *)curptr * k2->value);
		    k2++;
		}
	    }
	    curstart += srcbytes;
	    dstptr += dstbytes;
	    cppptr++;
	}

	/* Do the region where pixels outside the right edge of the
	 * image are needed */
	curedge = curudp->UdpA_Base + (curudp->UdpL_Pos >> 3) +
	    (x2 - curudp->UdpL_X1) * srcbytes;
	while (curstart < curedge) {
	    k2 = k1;
	    curptr = curstart - srcbytes;
	    if (*cppptr) {
		while (k2->increment != 0) {
		    curptr += k2->increment;
		    if (curptr <= curedge)
			*(float *)dstptr += (*(float *)curptr * k2->value);
		    else
			*(float *)dstptr += (k2->value * 0.0);
		    k2++;
		}
	    }
	    curstart += srcbytes;
	    dstptr += dstbytes;
	    cppptr++;
	}

	k1 += (kernel->x_count+1);
    }
}					/* end AreaMultSumCpp */

/*****************************************************************************
**  AreaGetTempUdp
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create a UDP which can be used to hold one scanline of the 
**      source UDP, but using float data.  If the source UDP is already
*       in float format, just point to its data.
**
**  FORMAL PARAMETERS:
**
**	src - Pointer to source UDP
**
**  FUNCTION VALUE:
**
**      Returns a pointer to a UDP describing floating point data.
**
*****************************************************************************/
static UdpPtr AreaGetTempUdp(src)
    UdpPtr src;
{
    UdpPtr dst;

    dst = (UdpPtr) DdxMalloc_(sizeof(UdpRec));
    if( dst == NULL ) return( (UdpPtr) dst );

    *dst = *src;
    dst->UdpL_Y2     = src->UdpL_Y1;
    dst->UdpL_ScnCnt = 1;

    if( src->UdpB_DType != UdpK_DTypeF )
	{
	dst->UdpW_PixelLength	= sizeof(float) * 8;
	dst->UdpB_DType		= UdpK_DTypeF;
	dst->UdpB_Class		= UdpK_ClassA;
	dst->UdpL_PxlStride	= dst->UdpW_PixelLength;
	dst->UdpL_ScnStride	= dst->UdpL_PxlPerScn * dst->UdpL_PxlStride;
	dst->UdpL_ArSize	= dst->UdpL_ScnStride;
	dst->UdpL_Pos		= 0;
	dst->UdpA_Base		= DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL )
	    {
	    DdxFree_(dst);
	    return( (UdpPtr) BadAlloc );
	    }
	}
    return( dst );
}					/* end AreaGetTempUdp */

/*****************************************************************************
**  AreaDestroyTempUdp
**
**  FUNCTIONAL DESCRIPTION:
**
**	Free a UDP created by AreGetTempUdp.
**
**  FORMAL PARAMETERS:
**
**	udp     - Pointer to the udp to free.
**      realudp - Pointer to a udp which might actually own the storage
**                pointed to by udp.
**
*****************************************************************************/
static void AreaDestroyTempUdp( udp, realudp )
UdpPtr udp;
UdpPtr realudp;

{
    if( IsPointer_(udp) )
	{
	if( IsPointer_(udp->UdpA_Base) && udp->UdpA_Base != realudp->UdpA_Base )
	    DdxFreeBits_( udp->UdpA_Base );
	DdxFree_( udp );
	}
}					/* end AreaDestroyTempUdp */

/*****************************************************************************
**  AreaConvertScanline
**
**  FUNCTIONAL DESCRIPTION:
**
**	Adjust a UDP to point to the specified scanline of the source
**	UDP.  If the source UDP is not of type float, convert the
**      data and leave it associated with the adjusted UDP.
**
**  FORMAL PARAMETERS:
**
**	src      - Pointer to the source UDP.
**      srcy1    - Beginning scanline index of entire source image.
**      srcy2    - Ending scanline index of entire source image.
**      scanline - The scanline being processed.
**      dst      - Pointer to the UDP to be adjusted.
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int AreaConvertScanline(src, srcy1, srcy2, scanline, dst)
    UdpPtr src;
    long int srcy1, srcy2;
    long int scanline;
    UdpPtr dst;
{
    int status = Success;

    dst->UdpL_Y1 = dst->UdpL_Y2 = scanline;

    if( src->UdpB_DType != UdpK_DTypeF )
	if( scanline < srcy1 || scanline > srcy2 )
	    {
	    /* Supply a zero "image" outside of the image boundaries */
	    dst->UdpL_Y1 = dst->UdpL_Y2 = 0;
	    status = DdxFillRegion_( dst, 0, 0, src->UdpL_PxlPerScn, 1, 0 );
	    dst->UdpL_Y1 = dst->UdpL_Y2 = scanline;
	    }
	else 
	    status = DdxConvert_( src, dst, XieK_MoveMode );
	
    else
	/* This UDP normally points to the input data.  If we need to
	 * supply a scanline of "zero" image, we have to swap to a
	 * temporary buffer.
	 */
	if( scanline < srcy1 || scanline > srcy2 )
	    {
	    /* Supply a zero "image" outside of the image boundaries */
	    if (dst->UdpA_Base == src->UdpA_Base)
		dst->UdpA_Base = 0;	/* Let DdxFill_ give us a temp buffer */
	    dst->UdpL_Y1 = dst->UdpL_Y2 = 0;
	    status = DdxFillRegion_( dst, 0, 0, src->UdpL_PxlPerScn, 1, 0 );
	    dst->UdpL_Y1 = dst->UdpL_Y2 = scanline;
	    }
	else
	    {
	    if( dst->UdpA_Base != src->UdpA_Base )
		{
		DdxFreeBits_(dst->UdpA_Base);
		dst->UdpA_Base = src->UdpA_Base;
		}
	    dst->UdpL_Pos = src->UdpL_Pos + scanline * src->UdpL_ScnStride;
	    }
    return( status );
}					/* end AreaConvertScanline */

/*****************************************************************************
**  AreaStoreScanline
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
**      scanline   - The scanline number
**      realdst    - Pointer to the destination UDP
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int AreaStoreScanline(dst, scanline, realdst)
    UdpPtr dst;
    long int scanline;
    UdpPtr realdst;
{
    int status = Success;

    if( dst->UdpA_Base != realdst->UdpA_Base )
	{
	status = DdxConvert_( dst, realdst, XieK_MoveMode );
	dst->UdpL_Y1++;
	dst->UdpL_Y2++;
	}
    else
	dst->UdpL_Pos = realdst->UdpL_Pos+(scanline+1)*realdst->UdpL_ScnStride;

    return( status );
}					/* end AreaStoreScanline */

/*****************************************************************************
**  AreaGetCppUdp
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create a UDP which can be used to hold one scanline of the 
**      CPP, but using byte data.
**
**  FORMAL PARAMETERS:
**
**	inudp - Pointer to source UDP
**
**  FUNCTION VALUE:
**
**      Returns a pointer to a UDP describing byte data.
**
*****************************************************************************/
static UdpPtr AreaGetCppUdp( src )
    UdpPtr src;
{
    UdpPtr dst;

    dst = (UdpPtr) DdxMalloc_(sizeof(UdpRec));
    if( dst == NULL ) return( (UdpPtr) BadAlloc );

    *dst = *src;
    dst->UdpW_PixelLength = 8;
    dst->UdpB_DType	  = UdpK_DTypeBU;
    dst->UdpB_Class       = UdpK_ClassA;
    dst->UdpL_ScnCnt      = 1;
    dst->UdpL_PxlStride   = dst->UdpW_PixelLength;
    dst->UdpL_ScnStride   = dst->UdpL_PxlStride * dst->UdpL_PxlPerScn;
    dst->UdpL_ArSize	  = dst->UdpL_ScnStride * dst->UdpL_ScnCnt;
    dst->UdpA_Base	  = DdxMallocBits_( dst->UdpL_ArSize );
    if( dst->UdpA_Base == NULL )
	{
	DdxFree_(dst);
	return( (UdpPtr) BadAlloc );
	}
    dst->UdpL_Pos	  = 0;
    dst->UdpL_CompIdx     = 0;
    dst->UdpL_Levels	  = 2;

    return( dst );
}					/* end AreaGetCppUdp */

/*****************************************************************************
**  AreaConvertCppLine
**
**  FUNCTIONAL DESCRIPTION:
**
**	Adjust a UDP to point to the specified scanline of the source
**	CPP UDP.  Convert the CPP's bit data to bytes for more
**      efficient access.
**
**  FORMAL PARAMETERS:
**
**	src      - Pointer to the source UDP.
**      scanline - The scanline being processed.
**      dst      - Pointer to the UDP to be adjusted.
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int AreaConvertCppLine(src,  scanline, dst)
    UdpPtr src;
    long int scanline;
    UdpPtr dst;
{
    dst->UdpL_Y1 = dst->UdpL_Y2 = scanline;
    return( DdxConvert_( src, dst, XieK_MoveMode ) );
}					/* end AreaConvertCppLine */

/*****************************************************************************
**  AreaDeallocData
**
**  FUNCTIONAL DESCRIPTION:
**
**	Deallocate a data segment queued in the window.
**
**  FORMAL PARAMETERS:
**
**	ctx      - Area pipeline context
**      comp     - Component index
**      idx      - Index in the window of the data segment.
**
*****************************************************************************/
static void AreaDeallocData (ctx, comp, idx)
    AreaPipeCtxPtr	ctx;
    int                 comp;
    int                 idx;
{
    UdpPtr	*window  = AreaWindow_(ctx,comp);
    PipeDataPtr *dwindow = AreaDWindow_(ctx,comp);

    if( IsPointer_(dwindow[idx]) )
	{
	/* Window must contain the UDP from the data segment */
	window[idx]  = NULL;
	dwindow[idx] = DdxRmDeallocData_( dwindow[idx] );
	}
    
    if( IsPointer_(window[idx]) )
	{
	AreaDestroyTempUdp( window[idx], SnkUdpPtr_(AreaSrcSnk_(ctx),comp) );
	window[idx] = NULL;
	}
}

/*****************************************************************************
**  AreaReleaseData
**
**  FUNCTIONAL DESCRIPTION:
**
**	Remove a data segment queued in the window and send it to the
**      next pipeline stage.
**
**  FORMAL PARAMETERS:
**
**	ctx      - Area pipeline context
**      comp     - Component index
**      idx      - Index in the window of the data segment.
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int AreaReleaseData (ctx, comp, idx)
    AreaPipeCtxPtr	ctx;
    int                 comp;
    int                 idx;
{
    int		  status = Success;
    UdpPtr      *window  = AreaWindow_(ctx,comp);
    PipeDataPtr *dwindow = AreaDWindow_(ctx,comp);

    if( IsPointer_(dwindow[idx]) )
	{
	/* Window must contain the UDP from the data segment */
	window[idx]  = NULL;
	status	     = DdxRmPutData_( AreaDstSnk_(ctx), dwindow[idx] );
	dwindow[idx] = NULL;
	}
    return( status );    
}

/*****************************************************************************
**  AreaAdjustKernel
**
**  FUNCTIONAL DESCRIPTION:
**
**	Allocate and initialize a copy of the supplied kernel, 
**      adjusting the increment fields to match the current
**      source image.
**
**  FORMAL PARAMETERS:
**
**	in       - source template data
**      pxlbytes - number of bytes to increment by one pixel.
**      out      - destination for pointer to the new template data.
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int AreaAdjustKernel (in, pxlbytes, out)
    TmpDatPtr	in;
    long int    pxlbytes;
    TmpDatPtr   *out;
{
    TmpDatPtr new;
    TmpDatEntryPtr src, dst, srow, drow;
    int i;

    /* Allocate a structure and copy */
    new = (TmpDatPtr) DdxMalloc_( sizeof(TmpDatRec) );
    if( new == NULL ) return( BadAlloc );
    *new = *in;

    /* Allocate the data array */
    new->array = (TmpDatEntryPtr)
		 DdxMalloc_(in->y_count*(in->x_count+1)*sizeof(TmpDatEntryRec));
    if( new->array == NULL )
	{
	DdxFree_(new);
	return( BadAlloc );
	}
    /* Copy and adjust the data */
    srow = in->array;
    drow = new->array;
    for( i = 0; i < in->y_count; i++ )
	{
	src = srow;
	dst = drow;
	do  {
	    dst->value = src->value;
	    dst->increment = src->increment * pxlbytes;
	    dst++;
	} while( (src++)->increment != 0 );
	srow += in->x_count+1;
	drow += in->x_count+1;
	}
    *out = new;

    return( Success );
}					/* end AreaAdjustKernel */

/*****************************************************************************
**  AreaDestroyKernel
**
**  FUNCTIONAL DESCRIPTION:
**
**	Deallocate a template data structure create by AreaAdjustKernel.
**
**  FORMAL PARAMETERS:
**
**	in       - source Template Idc
**
*****************************************************************************/
static void AreaDestroyKernel(in)
TmpDatPtr   in;
{
    int i;

    if( IsPointer_(in) )
	{
	if( IsPointer_(in->array) )
	    DdxFree_(in->array);
	DdxFree_(in);
	}
}					/* end AreaDestroyKernel */
