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
**      This module implements the compare operator which performs
**	compare functions between two photomaps or a photomap and a
**      constant.
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
**	Fri Jul  6 16:16:22 1990
**
*******************************************************************************/

#include <stdio.h>

#include "SmiCompare.h"

/*
**  Table of contents
*/
int 		SmiCompare();
int             SmiCreateCompare();

static int 	_CompareInitialize();
static int 	_CompareActivate();
static int 	_CompareFlush();
static int 	_CompareAbort();
static int 	_CompareDestroy();

static UdpPtr   CompareGetTempUdp();
static void     CompareDestroyTempUdp();
static int	CompareConvertScanline();
static int      CollectComponents();

static void 	CompareLTII();
static void 	CompareLTIC();
static void 	CompareLEII();
static void 	CompareLEIC();
static void 	CompareEQII();
static void 	CompareEQIC();
static void 	CompareNEII();
static void 	CompareNEIC();
static void 	CompareGTII();
static void 	CompareGTIC();
static void 	CompareGEII();
static void 	CompareGEIC();

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
    **  Compare Element Vector
    */
static PipeElementVector ComparePipeElement = {
    NULL,				/* FLINK			    */
    NULL,				/* BLINK			    */
    sizeof(PipeElementVector),		/* Size				    */
    TypePipeElementVector,		/* Structure type		    */
    StypeCompareElement,		/* Structure subtype		    */
    sizeof(ComparePipeCtx),		/* Context block size		    */
    0,					/* No input flag		    */
    0,					/* Reserved flags		    */
    _CompareInitialize,			/* Initialize entry		    */
    _CompareActivate,			/* Activate entry		    */
    _CompareFlush,			/* Flush entry			    */
    _CompareDestroy,			/* Destroy entry		    */
    _CompareAbort			/* Abort entry			    */
    };

/* Dispatch tables to select the correct operation routine */
/* These must match the ordering of the XieK_xxxx constants in XieAppl.h */
static void (*CompareTableII[])() = {
    NULL,				/* Constants start with 1 */
    CompareLTII,
    CompareLEII,
    CompareEQII,
    CompareNEII,
    CompareGTII,
    CompareGEII
    };

static void (*CompareTableIC[])() = {
    NULL,
    CompareLTIC,
    CompareLEIC,
    CompareEQIC,
    CompareNEIC,
    CompareGTIC,
    CompareGEIC
    };


/*****************************************************************************
**  SmiCreateCompare
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function fills in a newly created context for a COMPARE
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
**      operator- Compare operation function code.
**      combine - Produce one combined photomap for multiple component
**                input (Boolean)
**
**  FUNCTION VALUE:
**
*****************************************************************************/
int 	    SmiCreateCompare(pipe,srcsnk1,srcsnk2,constant,
				     dstsnk,idc,operator,combine)
Pipe		     pipe;
PipeSinkPtr	     srcsnk1;
PipeSinkPtr	     srcsnk2;
unsigned int    constant[XieK_MaxComponents];
PipeSinkPtr	     dstsnk;
UdpPtr		     idc;
int	     operator;
int             combine;
{
    int i;
    int combine_op;

    ComparePipeCtxPtr ctx = (ComparePipeCtxPtr)
		     DdxCreatePipeCtx_( pipe, &ComparePipeElement, FALSE );
    if( !IsPointer_(ctx) ) return( (int) ctx );
    /*
    **	Copy parameters
    */
    CmpSrcSnk1_(ctx) = srcsnk1;
    CmpSrcSnk2_(ctx) = srcsnk2;
    CmpDstSnk_(ctx)  = dstsnk;

    for( i = 0; i < XieK_MaxComponents; i++ )
	if( SnkUdpPtr_(srcsnk1,i) == NULL )
	    break;

    CmpCmpIn_(ctx)  = i;
    CmpCmpOut_(ctx) = combine ? 1 : i;

    /* The "combined" option is only really defined for XieK_EQ and XieK_NE.
     * If asked, we'll do combined mode of the other ops using the
     * same semantics as for XieK_EQ.
     */
    CmpCombOp_(ctx) = (operator == XieK_NE) ? XieK_OR : XieK_AND;

    if( srcsnk2 == NULL )
	{					/* Using a constant "image" */
	CmpFunc_(ctx) = CompareTableIC[operator];

	for( i = 0; i < CmpCmpIn_(ctx); i++ )
	    CmpConst_(ctx,i) = constant[i];
	}
    else 
	CmpFunc_(ctx) = CompareTableII[operator];

    CmpCpp_(ctx) = idc != NULL && idc->UdpA_Base != NULL ? idc : NULL;

    /* Calculate the bounds of the region that will actually be processed */
    if( srcsnk2 == NULL )
	{
	CmpRoiX1_(ctx) = SnkX1_(srcsnk1,0);
	CmpRoiX2_(ctx) = SnkX2_(srcsnk1,0);
	CmpRoiY1_(ctx) = SnkY1_(srcsnk1,0);
	CmpRoiY2_(ctx) = SnkY2_(srcsnk1,0);
	}
    else
	{
	/* Intersection of two source operands */
	CmpRoiX1_(ctx) = SnkX1_(srcsnk2,0) > SnkX1_(srcsnk1,0) ?
			 SnkX1_(srcsnk2,0) : SnkX1_(srcsnk1,0);
	CmpRoiX2_(ctx) = SnkX2_(srcsnk2,0) < SnkX2_(srcsnk1,0) ?
	                 SnkX2_(srcsnk2,0) : SnkX2_(srcsnk1,0);
	CmpRoiY1_(ctx) = SnkY1_(srcsnk2,0) > SnkY1_(srcsnk1,0) ?
	                 SnkY1_(srcsnk2,0) : SnkY1_(srcsnk1,0);
	CmpRoiY2_(ctx) = SnkY2_(srcsnk2,0) < SnkY2_(srcsnk1,0) ?
	                 SnkY2_(srcsnk2,0) : SnkY2_(srcsnk1,0);
	}

    if( idc != NULL )
	{
	/* Further restrict the area to be processed with the CPP/ROI  */
	CmpRoiX1_(ctx) = idc->UdpL_X1 > CmpRoiX1_(ctx) ?
			 idc->UdpL_X1 : CmpRoiX1_(ctx);
	CmpRoiX2_(ctx) = idc->UdpL_X2 < CmpRoiX2_(ctx) ?
			 idc->UdpL_X2 : CmpRoiX2_(ctx);
	CmpRoiY1_(ctx) = idc->UdpL_Y1 > CmpRoiY1_(ctx) ?
			 idc->UdpL_Y1 : CmpRoiY1_(ctx);
	CmpRoiY2_(ctx) = idc->UdpL_Y2 < CmpRoiY2_(ctx) ?
			 idc->UdpL_Y2 : CmpRoiY2_(ctx);
	}

    for( i = 0; i < XieK_MaxComponents; i++ )
	{
	CmpSrcData1_(ctx,i) = NULL;
	CmpSrcData2_(ctx,i) = NULL;
	}

    /* Create drains on the sink(s).
     * Handle any data format.
     * Process a quantum of 1 scanline.
     */
    for( i = 0; i < CmpCmpIn_(ctx); i++ )
	{
	CmpSrcDrn1_(ctx,i) = DdxRmCreateDrain_( CmpSrcSnk1_(ctx), 1 << i );
	if( !IsPointer_(CmpSrcDrn1_(ctx,i)) )
	    return((int)CmpSrcDrn1_(ctx,i));
	DdxRmSetDType_( CmpSrcDrn1_(ctx,i), UdpK_DTypeUndefined, DtM_Any );
	DdxRmSetQuantum_( CmpSrcDrn1_(ctx,i), 1 );

	if( srcsnk2 != NULL )
	    {
	    CmpSrcDrn2_(ctx,i) = DdxRmCreateDrain_( CmpSrcSnk2_(ctx), 1 << i );
	    if( !IsPointer_(CmpSrcDrn2_(ctx,i)) )
		return((int)CmpSrcDrn2_(ctx,i));
	    DdxRmSetDType_( CmpSrcDrn2_(ctx,i), UdpK_DTypeUndefined, DtM_Any );
	    DdxRmSetQuantum_( CmpSrcDrn2_(ctx,i), 1 );
	    }
	}

    /* The result is an IDC, so we always produce bits */
    DdxRmSetDType_( CmpDstSnk_(ctx), UdpK_DTypeVU, DtM_VU );
    DdxRmSetQuantum_( CmpDstSnk_(ctx), 1 );

    return( Success ); 
}					/* SmiCreateCompare */

/*****************************************************************************
**  _CompareInitialize
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function initializes the COMPARE pipeline element prior to
**	activation.
**
**  FORMAL PARAMETERS:
**
**      ctx - compare pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	_CompareInitialize(ctx)
ComparePipeCtxPtr	ctx;
{
    int i, status;

    for( i = 0; i < CmpCmpIn_(ctx); i++ )
	{
	status = DdxRmInitializePort_( CtxHead_(ctx), CmpSrcDrn1_(ctx,i) );
	if( status != Success ) return( status );
	}
    if( CmpSrcSnk2_(ctx) != NULL )
	{
	for( i = 0; i < CmpCmpIn_(ctx); i++ )
	    {
	    status = DdxRmInitializePort_( CtxHead_(ctx), CmpSrcDrn2_(ctx,i) );
	    if( status != Success ) return( status );
	    }
	/* If one of the drains is constrained, and the other is unconstrained,
	 * we have to force conversion of the constrained data to unconstrained
	 */
	if( DrnDtyp_(CmpSrcDrn1_(ctx,0)) == UdpK_DTypeF ||
	    DrnDtyp_(CmpSrcDrn2_(ctx,0)) == UdpK_DTypeF )
	    for( i = 0; i < CmpCmpIn_(ctx); i++ )
		{
		DrnDtyp_(CmpSrcDrn1_(ctx,i)) = UdpK_DTypeF;
		DrnDtyp_(CmpSrcDrn2_(ctx,i)) = UdpK_DTypeF;
		}
	}

    status = DdxRmInitializePort_( CtxHead_(ctx), CmpDstSnk_(ctx) );

    /* Start reading from the first drain */
    CtxInp_(ctx) = CmpSrcDrn1_(ctx,0);

    return( status ); 
}

/*****************************************************************************
**  _CompareActivate
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine reads the data from the source drain(s) and
**      performs the compare operations.
**
**  FORMAL PARAMETERS:
**
**      ctx - compare pipeline element context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _CompareActivate(ctx)
ComparePipeCtxPtr    ctx;
{
    PipeDataPtr	dst[XieK_MaxComponents], tmp = NULL;
    int      dstpos[XieK_MaxComponents], status, i;

    if( CmpSrcSnk2_(ctx) == NULL )
	/* Operating between one image and a constant */
	for( status = Success; status == Success; )
	    {
	    status = CollectComponents( &CmpSrcDrn1_(ctx,0),
					&CmpSrcData1_(ctx,0),
					 ctx, CmpCmpIn_(ctx) );
	    if( status != Success ) break;

	    /* Allocate a destination data seg for each output component */
	    for( i =0; i < CmpCmpOut_(ctx); i++ )
		{
		dst[i] = DdxRmAllocData_( CmpDstSnk_(ctx), i,
					  DatY1_(CmpSrcData1_(ctx,0)),
					  DatHeight_(CmpSrcData1_(ctx,0)) );
		if( !IsPointer_(dst[i]) )
		    status = (int) dst[i];
		}
	    /* If combining results, get a temporary data seg. */
	    if( status == Success && CmpCmpIn_(ctx) > CmpCmpOut_(ctx) )
		{
		tmp = DdxRmAllocData_( CmpDstSnk_(ctx),0,
				       DatY1_(CmpSrcData1_(ctx,0)),
				       DatHeight_(CmpSrcData1_(ctx,0)) );
		if( !IsPointer_(tmp) )
		    status = (int) tmp;
		}
	    if( DatY1_(CmpSrcData1_(ctx,0)) < CmpRoiY1_(ctx) ||
	        DatY1_(CmpSrcData1_(ctx,0)) > CmpRoiY2_(ctx) )
		/* Scanline is outside the ROI - return zeros in the
		 * destination.
		 */
		for( i = 0; i < CmpCmpOut_(ctx) && status == Success; i++ )
		    {
		    status = DdxFillRegion_( DatUdpPtr_(dst[i]),
					     DatX1_(dst[i]), DatY1_(dst[i]),
					     DatWidth_(dst[i]),
					     DatHeight_(dst[i]), 0 );
		    dstpos[i] = DatPos_(dst[i]);
		    }
	    else
		{
		/* Zero the portions of this line outside the ROI */
		for( i = 0; i < CmpCmpOut_(ctx) && status == Success; i++ )
		    {
		    status = DdxFillRegion_( DatUdpPtr_(dst[i]),    /* Left   */
					     DatX1_(CmpSrcData1_(ctx,0)),
					     DatY1_(CmpSrcData1_(ctx,0)),
					     CmpRoiX1_(ctx)
						- DatX1_(CmpSrcData1_(ctx,0)),
					     1, 0 );
		    if( status != Success ) break;

		    status = DdxFillRegion_( DatUdpPtr_(dst[i]),    /* Right  */
					     CmpRoiX2_(ctx)+1,
					     DatY1_(CmpSrcData1_(ctx,0)),
					     DatX2_(CmpSrcData1_(ctx,0))
							- CmpRoiX2_(ctx),
					     1, 0 );
		    if( status != Success ) break;
		
		    DatPos_(CmpSrcData1_(ctx,i)) +=
			(CmpRoiX1_(ctx) - DatX1_(CmpSrcData1_(ctx,i))) *
			    DatPxlStr_(CmpSrcData1_(ctx,i));
		    dstpos[i] = DatPos_(dst[i]);
		    DatPos_(dst[i]) +=
			(CmpRoiX1_(ctx) - DatX1_(dst[i])) * DatPxlStr_(dst[i]);
		    }

		/* Compare all the components of this scanline */
		if( status == Success )
		    (*CmpFunc_(ctx))(DatUdpPtr_(CmpSrcData1_(ctx,0)),
				     CmpConst_(ctx,0),
				     DatUdpPtr_(dst[0]),
				     CmpCpp_(ctx),
				     CmpRoiY1_(ctx),
				     CmpRoiX1_(ctx),
				     CmpRoiX2_(ctx));

		for( i = 1; i < CmpCmpIn_(ctx) && status == Success; i++ )
		    {
		    if( CmpCmpIn_(ctx) > CmpCmpOut_(ctx) )
			{
			/* Combining output */
			DatCmpIdx_(tmp) = i;
			(*CmpFunc_(ctx))(DatUdpPtr_(CmpSrcData1_(ctx,i)),
					 CmpConst_(ctx,i),
					 DatUdpPtr_(tmp),
					 CmpCpp_(ctx),
					 CmpRoiY1_(ctx),
					 CmpRoiX1_(ctx),
					 CmpRoiX2_(ctx));
			status = DdxLogical_( DatUdpPtr_(dst[0]),
					      DatUdpPtr_(tmp),
					      NULL, DatUdpPtr_(dst[0]),
					      NULL, CmpCombOp_(ctx) );
			}			
		    else
			/* Not combining output */
			(*CmpFunc_(ctx))(DatUdpPtr_(CmpSrcData1_(ctx,i)),
					 CmpConst_(ctx,i),
					 DatUdpPtr_(dst[i]),
					 CmpCpp_(ctx),
					 CmpRoiY1_(ctx),
					 CmpRoiX1_(ctx),
					 CmpRoiX2_(ctx));
		    }
		}
	    for( i = 0; i < CmpCmpOut_(ctx); i++ )
		if( IsPointer_(dst[i]) )
		    {
		    DatPos_(dst[i]) = dstpos[i];
		    if( status == Success )
			status  = DdxRmPutData_( CmpDstSnk_(ctx), dst[i] );
		    else
			DdxRmDeallocData_( dst[i] );
		    dst[i] = NULL;
		    }
	    for( i = 0; i < CmpCmpIn_(ctx); i++ )
		CmpSrcData1_(ctx,i) = DdxRmDeallocData_( CmpSrcData1_(ctx,i) );

	    if( tmp != NULL )
		tmp  = DdxRmDeallocData_( tmp );
	    }				/* end for */

    else
	/* Operating between two images */
	for( status = Success; status == Success; )
	    {
	    status = CollectComponents( &CmpSrcDrn1_(ctx,0),
					&CmpSrcData1_(ctx,0),
					 ctx, CmpCmpIn_(ctx) );
	    if( status != Success ) break;

	    if( DatY1_(CmpSrcData1_(ctx,0)) <= CmpRoiY2_(ctx) )
		{   /* Keep the two streams synchronized as long as we still
		     * need data from the second image */
		status = CollectComponents( &CmpSrcDrn2_(ctx,0),
					    &CmpSrcData2_(ctx,0),
					     ctx, CmpCmpIn_(ctx) );
		if( status != Success ) break;
		}
	    /* Allocate a destination data seg for each output component */
	    for( i =0; i < CmpCmpOut_(ctx); i++ )
		{
		dst[i] = DdxRmAllocData_( CmpDstSnk_(ctx), i,
					  DatY1_(CmpSrcData1_(ctx,0)),
					  DatHeight_(CmpSrcData1_(ctx,0)) );
		if( !IsPointer_(dst[i]) )
		    status = (int) dst[i];
		}
	    /* If combining results, get a temporary data seg. */
	    if( CmpCmpIn_(ctx) > CmpCmpOut_(ctx) )
		{
		tmp = DdxRmAllocData_( CmpDstSnk_(ctx), 0,
				       DatY1_(CmpSrcData1_(ctx,0)),
				       DatHeight_(CmpSrcData1_(ctx,0)) );
		if( !IsPointer_(tmp) )
		    status = (int) tmp;
		}
	    if( DatY1_(CmpSrcData1_(ctx,0)) < CmpRoiY1_(ctx) ||
	        DatY1_(CmpSrcData1_(ctx,0)) > CmpRoiY2_(ctx) )
		/* Scanline is outside the ROI - return zeros in the
		 * destination.
		 */
		for( i = 0; i < CmpCmpOut_(ctx) && status == Success; i++ )
		    {
		    status = DdxFillRegion_( DatUdpPtr_(dst[i]),
		    			     DatX1_(dst[i]), DatY1_(dst[i]),
		    			     DatWidth_(dst[i]),
					     DatHeight_(dst[i]), 0 );
		    dstpos[i] = DatPos_(dst[i]);
	            }
	    else
		{
		/* Zero the portions of this line outside the ROI */
		for( i = 0; i < CmpCmpOut_(ctx) && status == Success; i++ )
		    {
		    status = DdxFillRegion_( DatUdpPtr_(dst[i]),    /* Left   */
					     DatX1_(CmpSrcData1_(ctx,0)),
					     DatY1_(CmpSrcData1_(ctx,0)),
					     CmpRoiX1_(ctx)
						- DatX1_(CmpSrcData1_(ctx,0)),
					     1, 0 );
		    if( status != Success ) break;
		
		    status = DdxFillRegion_( DatUdpPtr_(dst[i]),    /* Right  */
					     CmpRoiX2_(ctx)+1,
					     DatY1_(CmpSrcData1_(ctx,0)),
					     DatX2_(CmpSrcData1_(ctx,0))
							- CmpRoiX2_(ctx),
					     1, 0);
		    if( status != Success ) break;
		
		    DatPos_(CmpSrcData1_(ctx,i)) +=
			(CmpRoiX1_(ctx) - DatX1_(CmpSrcData1_(ctx,i))) *
			    DatPxlStr_(CmpSrcData1_(ctx,i));
		    dstpos[i] = DatPos_(dst[i]);
		    DatPos_(dst[i]) +=
			(CmpRoiX1_(ctx) - DatX1_(dst[i])) * DatPxlStr_(dst[i]);
		    }
		/* Compare all the components of this scanline */
		if( status == Success )
		    (*CmpFunc_(ctx))(DatUdpPtr_(CmpSrcData1_(ctx,0)),
				     DatUdpPtr_(CmpSrcData2_(ctx,0)),
				     DatUdpPtr_(dst[0]),
				     CmpCpp_(ctx),
				     CmpRoiY1_(ctx),
				     CmpRoiX1_(ctx),
				     CmpRoiX2_(ctx));

		for( i = 1; i < CmpCmpIn_(ctx) && status == Success; i++ )
		    if( CmpCmpIn_(ctx) > CmpCmpOut_(ctx) )
			{
			/* Combining output */
			DatCmpIdx_(tmp) = i;
			(*CmpFunc_(ctx))(DatUdpPtr_(CmpSrcData1_(ctx,i)),
					 DatUdpPtr_(CmpSrcData2_(ctx,i)),
					 DatUdpPtr_(tmp),
					 CmpCpp_(ctx),
					 CmpRoiY1_(ctx),
					 CmpRoiX1_(ctx),
					 CmpRoiX2_(ctx));
			status = DdxLogical_( DatUdpPtr_(dst[0]),
					      DatUdpPtr_(tmp),
					      NULL, DatUdpPtr_(dst[0]),
					      NULL, CmpCombOp_(ctx) );
			}			
		    else
			/* Not combining output */
			(*CmpFunc_(ctx))(DatUdpPtr_(CmpSrcData1_(ctx,i)),
					 DatUdpPtr_(CmpSrcData2_(ctx,i)),
					 DatUdpPtr_(dst[i]),
					 CmpCpp_(ctx),
					 CmpRoiY1_(ctx),
					 CmpRoiX1_(ctx),
					 CmpRoiX2_(ctx));
		}
	    for( i = 0; i < CmpCmpOut_(ctx); i++ )
		if( IsPointer_(dst[i]) )
		    {
		    DatPos_(dst[i]) = dstpos[i];
		    if( status == Success )
			status = DdxRmPutData_( CmpDstSnk_(ctx), dst[i] );
		    else
			DdxRmDeallocData_( dst[i] );
		    dst[i] = NULL;
		    }
	    for( i = 0; i < CmpCmpIn_(ctx); i++ )
		{
		CmpSrcData1_(ctx,i) = DdxRmDeallocData_( CmpSrcData1_(ctx,i) );
		if( CmpSrcData2_(ctx,i) != NULL )
		    CmpSrcData2_(ctx,i)  =
				      DdxRmDeallocData_( CmpSrcData2_(ctx,i) );
		}		
	    if( tmp != NULL )
		tmp  = DdxRmDeallocData_( tmp );
	    }				/* end for */
    return( status > 0 ? status : Success ); 
}					/* end _CompareActivate */

/*****************************************************************************
**  _CompareFlush
**
**  FUNCTIONAL DESCRIPTION:
**
**  	This routine is called when all preceding pipeline stages have
**      completed. 
**
**
**  FORMAL PARAMETERS:
**
**      ctx - compare pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _CompareFlush(ctx)
ComparePipeCtxPtr    ctx;
{
    PipeDataPtr src2;
    int i;

    _CompareAbort(ctx);

    /* If the second image operand has more scanlines that the first image,
     * we will have unprocessed scanlines on the image2 drains.  Flush them.
     */
    if( CmpSrcSnk2_(ctx) != NULL )
	for( i = 0; i < CmpCmpIn_(ctx); i++ )
	    {
	    src2 = DdxRmGetData_( ctx, CmpSrcDrn2_(ctx,i) );

	    if( IsPointer_(src2) )
		DdxRmDeallocData_( src2 );

	    else if( src2 != NULL )
		return( (int) src2 );
	    }

    return( Success ); 
}

/*****************************************************************************
**  _CompareAbort
**
**  FUNCTIONAL DESCRIPTION:
**
**  	This routine is called when all preceding pipeline stages have
**      completed. 
**
**
**  FORMAL PARAMETERS:
**
**      ctx - compare pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _CompareAbort(ctx)
ComparePipeCtxPtr    ctx;
{
    int i;

    for( i = 0; i < CmpCmpIn_(ctx); i++ )
	{
	if( IsPointer_(CmpSrcData1_(ctx,i)) )
	    CmpSrcData1_(ctx,i) = DdxRmDeallocData_( CmpSrcData1_(ctx,i) );
	if( IsPointer_(CmpSrcData2_(ctx,i)) )
	    CmpSrcData2_(ctx,i) = DdxRmDeallocData_( CmpSrcData2_(ctx,i) );
	}		
    return( Success ); 
}

/*****************************************************************************
**  _CompareDestroy
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine deallocates all resources allocated by the COMPARE
**	pipeline element.
**
**  FORMAL PARAMETERS:
**
**      ctx - compare pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _CompareDestroy(ctx)
ComparePipeCtxPtr    ctx;
{
    int i;

    for( i = 0; i < CmpCmpIn_(ctx); i++ )
	{
	CmpSrcDrn1_(ctx,i) = DdxRmDestroyDrain_( CmpSrcDrn1_(ctx,i) );
	if( CmpSrcSnk2_(ctx) != NULL )
	    CmpSrcDrn2_(ctx,i) = DdxRmDestroyDrain_( CmpSrcDrn2_(ctx,i) );
	}

    return( Success ); 
}

/*****************************************************************************
**  SmiCompare
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine performs a non-pipelined compare operator, which
**      executes an compare operator between two photomaps, or between
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
int 	SmiCompare( srcudp1, srcudp2, constant, dstudp, idc, operator )
UdpPtr			srcudp1;
UdpPtr		  	srcudp2;
unsigned int	constant[];
UdpPtr			dstudp;
UdpPtr	 		idc;
int		operator;
{
    UdpPtr   cpp, src1 = NULL, src2 = NULL, dst = NULL;
    float fconst;
    long int pixels, scanline, status;
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
     * Determine maximum common dimensions between source1 and destination,
     * which is the defined region of output.
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
     * Restrict the area to be processed by finding the intersection of
     * the region of output and the second source operand (if used).
     */
    if (srcudp2 == NULL) {
	x1 = X1; x2 = X2;
	y1 = Y1; y2 = Y2;
    } else {
	x1 = srcudp2->UdpL_X1 > X1 ? srcudp2->UdpL_X1 : X1;
	x2 = srcudp2->UdpL_X2 < X2 ? srcudp2->UdpL_X2 : X2;
	y1 = srcudp2->UdpL_Y1 > Y1 ? srcudp2->UdpL_Y1 : Y1;
	y2 = srcudp2->UdpL_Y2 < Y2 ? srcudp2->UdpL_Y2 : Y2;
    }
	
    /* 
     * Further restrict the area to be processed by finding the intersection
     * with the IDC (ROI or CPP).
     */
    if (idc != NULL) {
	x1 = idc->UdpL_X1 > x1 ? idc->UdpL_X1 : x1;
	x2 = idc->UdpL_X2 < x2 ? idc->UdpL_X2 : x2;
	y1 = idc->UdpL_Y1 > y1 ? idc->UdpL_Y1 : y1;
	y2 = idc->UdpL_Y2 < y2 ? idc->UdpL_Y2 : y2;
    }

    cpp = idc != NULL && idc->UdpA_Base != NULL ? idc : NULL;
	
    /* If one of the input UDP's is unconstrained, then convert the other */
    src1 = CompareGetTempUdp( srcudp1, srcudp2, y1, x1, x2 );
    if( !IsPointer_(src1) )
	{
	status = (int) src1;
	goto clean_up;
	}
    if( srcudp2 != NULL )
	{
	src2 = CompareGetTempUdp( srcudp2, srcudp1, y1, x1, x2 );
	if( !IsPointer_(src2) )
	    {
	    status = (int) src2;
	    goto clean_up;
	    }
	}
    dst = CompareGetTempUdp(dstudp, NULL, y1, x1, x2);
    if( !IsPointer_(dst) )
	{
	status = (int) dst;
	goto clean_up;
	}

    /* Return zero for the scanlines outside the ROI */
    /* Top */
    status = DdxFillRegion_( dstudp, X1, Y1, X2-X1+1, y1 - Y1, 0 );
    if( status != Success ) goto clean_up;
    /* Bottom */
    status = DdxFillRegion_( dstudp, X1, y2+1, X2-X1+1, Y2 - y2, 0);
    if( status != Success ) goto clean_up;
    /* Left */
    status = DdxFillRegion_( dstudp, X1, y1, x1 - X1, y2 - y1 + 1, 0 );
    if( status != Success ) goto clean_up;
    /* Right */    
    status = DdxFillRegion_( dstudp, x2+1, y1, X2 - x2, y2 - y1 + 1, 0 );
    if( status != Success ) goto clean_up;

    /* Now perform the compare operation over the region-of-interest */

    if( srcudp2 != NULL )
	/* Image-to-image operation */
	for( scanline = y1; scanline <= y2; scanline++ )
	    {
	    /* Get a UDP pointing to this scanline */
	    status = CompareConvertScanline(srcudp1, src1);
	    if( status != Success ) break;
	    status = CompareConvertScanline(srcudp2, src2);
	    if( status != Success ) break;
	    status = CompareConvertScanline(dstudp,  dst );
	    if( status != Success ) break;
	    
	    (*CompareTableII[operator])(src1, src2,
					dst,
					cpp, y1, x1, x2);
	    }
    else
	/* Image-to-constant operation */
	for( scanline = y1; scanline <= y2; scanline++ )
	    {
	    /* Get a UDP pointing to this scanline */
	    status = CompareConvertScanline(srcudp1, src1);
	    if( status != Success ) break;
	    status = CompareConvertScanline(dstudp,  dst );
	    if( status != Success ) break;
	    (*CompareTableIC[operator])(src1, constant[src1->UdpL_CompIdx],
					dst,
					cpp, y1, x1, x2);
	    }

    /* Cleanup the temporary descriptors */
clean_up :
    if( IsPointer_(src1) )
	CompareDestroyTempUdp( src1, srcudp1 );
    if( IsPointer_(src2) )
	CompareDestroyTempUdp( src2, srcudp2 );
    if( IsPointer_(dst) )
	CompareDestroyTempUdp( dst, dstudp );

    return( status ); 
}					/* end SmiCompare */

/*****************************************************************************
**  Compare processing routines
**
**  FUNCTIONAL DESCRIPTION:
**
**	These routines perform the actual compare operation over selected
**      portion of a scanline.  A separate routine is used for each
**      type of operation.
**
**  FORMAL PARAMETERS:
**
**	src1    - Pointer to a UDP describing the first source operand.
**	src2    - Pointer to a UDP describing the second source operand.
**      const   - Constant "image" value if image-to-constant.
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
*****************************************************************************/
static void  CompareLTII(src1, src2, dst, cpp, y1, x1, x2)
UdpPtr	     	src1;
UdpPtr          src2;
UdpPtr	     	dst;
UdpPtr		cpp;
long int        y1;
long int	x1;
long int	x2;
{
    int			pixel;

    unsigned int  	src1_off;
    unsigned int  	src2_off;
    unsigned int        dst_off;
    long int		cpppxloff;
    int			cppbit;

    int   		result;
    int                 src1_mask;
    int                 src2_mask;

    src1_off = src1->UdpL_Pos;
    src2_off = src2->UdpL_Pos;
    dst_off =  dst->UdpL_Pos;

    if (cpp != NULL)
	cpppxloff = cpp->UdpL_Pos +
	    cpp->UdpL_ScnStride * (src1->UdpL_Y1 - y1);
    else
	cppbit = 1;			/* Fake all 1's in CPP */


    if (src1->UdpB_DType != UdpK_DTypeF) {

	/* Constrained types */
	src1_mask = (1 << src1->UdpW_PixelLength) -1;
	src2_mask = (1 << src2->UdpW_PixelLength) -1;

	for (pixel = x1; pixel <= x2; pixel++){
	    
	    result = 0;
	    
	    if (cpp != NULL) {
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
	    }	  
	    
	    if (cppbit)
		result = GET_VALUE_(src1->UdpA_Base, src1_off, src1_mask) <
		    GET_VALUE_(src2->UdpA_Base, src2_off, src2_mask);
	    
	    PUT_VALUE_(dst->UdpA_Base, dst_off, result, 1);
	    
	    src1_off+=src1->UdpL_PxlStride;
	    src2_off+=src2->UdpL_PxlStride;
	    dst_off+=dst->UdpL_PxlStride;
	}

    } else {

	/* Unconstrained data */
	for (pixel = x1; pixel <= x2; pixel++){
	    
	    result = 0;

	    if (cpp != NULL) {
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
	    }	  
	    
	    if (cppbit)
		result =  *(float *)(src1->UdpA_Base + (src1_off >> 3)) < 
		    *(float *)(src2->UdpA_Base + (src2_off >> 3));
	    
	    PUT_VALUE_(dst->UdpA_Base, dst_off, result, 1);
	    
	    src1_off+=src1->UdpL_PxlStride;
	    src2_off+=src2->UdpL_PxlStride;
	    dst_off+=dst->UdpL_PxlStride;
	    
	}
    }
}					/* end CompareLTII */

static void  CompareLEII(src1, src2, dst, cpp, y1, x1, x2)
UdpPtr	     	src1;
UdpPtr	     	src2;
UdpPtr	     	dst;
UdpPtr		cpp;
long int        y1;
long int	x1;
long int	x2;
{
    int			pixel;

    unsigned int  	src1_off;
    unsigned int  	src2_off;
    unsigned int        dst_off;
    long int		cpppxloff;
    int			cppbit;

    int   		result;
    int                 src1_mask;
    int                 src2_mask;

    src1_off = src1->UdpL_Pos;
    src2_off = src2->UdpL_Pos;
    dst_off =  dst->UdpL_Pos;

    if (cpp != NULL)
	cpppxloff = cpp->UdpL_Pos +
	    cpp->UdpL_ScnStride * (src1->UdpL_Y1 - y1);
    else
	cppbit = 1;			/* Fake all 1's in CPP */


    if (src1->UdpB_DType != UdpK_DTypeF) {

	/* Constrained types */
	src1_mask = (1 << src1->UdpW_PixelLength) -1;
	src2_mask = (1 << src2->UdpW_PixelLength) -1;

	for (pixel = x1; pixel <= x2; pixel++){
	    
	    result = 0;
	    
	    if (cpp != NULL) {
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
	    }	  
	    
	    if (cppbit)
		result = GET_VALUE_(src1->UdpA_Base, src1_off, src1_mask)
		    <= GET_VALUE_(src2->UdpA_Base, src2_off, src2_mask);
	    
	    PUT_VALUE_(dst->UdpA_Base, dst_off, result, 1);
	    
	    src1_off+=src1->UdpL_PxlStride;
	    src2_off+=src2->UdpL_PxlStride;
	    dst_off+=dst->UdpL_PxlStride;
	}

    } else {

	/* Unconstrained data */
	for (pixel = x1; pixel <= x2; pixel++){
	    
	    result = 0;
	    
	    if (cpp != NULL) {
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
	    }	  
	    
	    if (cppbit)
		result =  *(float *)(src1->UdpA_Base + (src1_off >> 3)) <= 
		    *(float *)(src2->UdpA_Base + (src2_off >> 3));
	    
	    PUT_VALUE_(dst->UdpA_Base, dst_off, result, 1);
	    
	    src1_off+=src1->UdpL_PxlStride;
	    src2_off+=src2->UdpL_PxlStride;
	    dst_off+=dst->UdpL_PxlStride;
	    
	}
    }
}					/* end CompareLEII */

static void  CompareEQII(src1, src2, dst, cpp, y1, x1, x2)
UdpPtr	     	src1;
UdpPtr	     	src2;
UdpPtr	     	dst;
UdpPtr		cpp;
long int        y1;
long int	x1;
long int	x2;
{
    int			pixel;

    unsigned int  	src1_off;
    unsigned int  	src2_off;
    unsigned int        dst_off;
    long int		cpppxloff;
    int			cppbit;

    int   		result;
    int                 src1_mask;
    int                 src2_mask;

    src1_off = src1->UdpL_Pos;
    src2_off = src2->UdpL_Pos;
    dst_off =  dst->UdpL_Pos;

    if (cpp != NULL)
	cpppxloff = cpp->UdpL_Pos +
	    cpp->UdpL_ScnStride * (src1->UdpL_Y1 - y1);
    else
	cppbit = 1;			/* Fake all 1's in CPP */


    if (src1->UdpB_DType != UdpK_DTypeF) {

	/* Constrained types */
	src1_mask = (1 << src1->UdpW_PixelLength) -1;
	src2_mask = (1 << src2->UdpW_PixelLength) -1;

	for (pixel = x1; pixel <= x2; pixel++){
	    
	    result = 0;
	    
	    if (cpp != NULL) {
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
	    }	  
	    
	    if (cppbit)
		result = GET_VALUE_(src1->UdpA_Base, src1_off, src1_mask)
		    == GET_VALUE_(src2->UdpA_Base, src2_off, src2_mask);
	    
	    PUT_VALUE_(dst->UdpA_Base, dst_off, result, 1);
	    
	    src1_off+=src1->UdpL_PxlStride;
	    src2_off+=src2->UdpL_PxlStride;
	    dst_off+=dst->UdpL_PxlStride;
	}

    } else {

	/* Unconstrained data */
	for (pixel = x1; pixel <= x2; pixel++){
	    
	    result = 0;

	    if (cpp != NULL) {
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
	    }	  
	    
	    if (cppbit)
		result =  *(float *)(src1->UdpA_Base + (src1_off >> 3)) ==
		    *(float *)(src2->UdpA_Base + (src2_off >> 3));
	    
	    PUT_VALUE_(dst->UdpA_Base, dst_off, result, 1);
	    
	    src1_off+=src1->UdpL_PxlStride;
	    src2_off+=src2->UdpL_PxlStride;
	    dst_off+=dst->UdpL_PxlStride;
	    
	}
    }
}					/* end CompareEQII */

static void  CompareNEII(src1, src2, dst, cpp, y1, x1, x2)
UdpPtr	     	src1;
UdpPtr	     	src2;
UdpPtr	     	dst;
UdpPtr		cpp;
long int        y1;
long int	x1;
long int	x2;
{
    int			pixel;

    unsigned int  	src1_off;
    unsigned int  	src2_off;
    unsigned int        dst_off;
    long int		cpppxloff;
    int			cppbit;

    int   		result;
    int                 src1_mask;
    int                 src2_mask;

    src1_off = src1->UdpL_Pos;
    src2_off = src2->UdpL_Pos;
    dst_off =  dst->UdpL_Pos;

    if (cpp != NULL)
	cpppxloff = cpp->UdpL_Pos +
	    cpp->UdpL_ScnStride * (src1->UdpL_Y1 - y1);
    else
	cppbit = 1;			/* Fake all 1's in CPP */


    if (src1->UdpB_DType != UdpK_DTypeF) {

	/* Constrained types */
	src1_mask = (1 << src1->UdpW_PixelLength) -1;
	src2_mask = (1 << src2->UdpW_PixelLength) -1;

	for (pixel = x1; pixel <= x2; pixel++){
	    
	    result = 0;
	    
	    if (cpp != NULL) {
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
	    }	  
	    
	    if (cppbit)
		result = GET_VALUE_(src1->UdpA_Base, src1_off, src1_mask)
		    != GET_VALUE_(src2->UdpA_Base, src2_off, src2_mask);

	    PUT_VALUE_(dst->UdpA_Base, dst_off, result, 1);
	    
	    src1_off+=src1->UdpL_PxlStride;
	    src2_off+=src2->UdpL_PxlStride;
	    dst_off+=dst->UdpL_PxlStride;
	}

    } else {

	/* Unconstrained data */
	for (pixel = x1; pixel <= x2; pixel++){
	    
	    result = 0;

	    if (cpp != NULL) {
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
	    }	  
	    
	    if (cppbit)
		result =  *(float *)(src1->UdpA_Base + (src1_off >> 3)) !=
		    *(float *)(src2->UdpA_Base + (src2_off >> 3));
	    
	    PUT_VALUE_(dst->UdpA_Base, dst_off, result, 1);
	    
	    src1_off+=src1->UdpL_PxlStride;
	    src2_off+=src2->UdpL_PxlStride;
	    dst_off+=dst->UdpL_PxlStride;
	    
	}
    }
}					/* end CompareNEII */

static void  CompareGTII(src1, src2, dst, cpp, y1, x1, x2)
UdpPtr	     	src1;
UdpPtr	     	src2;
UdpPtr	     	dst;
UdpPtr		cpp;
long int        y1;
long int	x1;
long int	x2;
{
    int			pixel;

    unsigned int  	src1_off;
    unsigned int  	src2_off;
    unsigned int        dst_off;
    long int		cpppxloff;
    int			cppbit;

    int   		result;
    int                 src1_mask;
    int                 src2_mask;

    src1_off = src1->UdpL_Pos;
    src2_off = src2->UdpL_Pos;
    dst_off =  dst->UdpL_Pos;

    if (cpp != NULL)
	cpppxloff = cpp->UdpL_Pos +
	    cpp->UdpL_ScnStride * (src1->UdpL_Y1 - y1);
    else
	cppbit = 1;			/* Fake all 1's in CPP */


    if (src1->UdpB_DType != UdpK_DTypeF) {

	/* Constrained types */
	src1_mask = (1 << src1->UdpW_PixelLength) -1;
	src2_mask = (1 << src2->UdpW_PixelLength) -1;

	for (pixel = x1; pixel <= x2; pixel++){
	    
	    result = 0;
	    
	    if (cpp != NULL) {
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
	    }	  
	    
	    if (cppbit)
		result = GET_VALUE_(src1->UdpA_Base, src1_off, src1_mask) >
		    GET_VALUE_(src2->UdpA_Base, src2_off, src2_mask);

	    PUT_VALUE_(dst->UdpA_Base, dst_off, result, 1);
	    
	    src1_off+=src1->UdpL_PxlStride;
	    src2_off+=src2->UdpL_PxlStride;
	    dst_off+=dst->UdpL_PxlStride;
	}

    } else {

	/* Unconstrained data */
	for (pixel = x1; pixel <= x2; pixel++){
	    
	    result = 0;

	    if (cpp != NULL) {
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
	    }	  
	    
	    if (cppbit)
		result =  *(float *)(src1->UdpA_Base + (src1_off >> 3)) >
		    *(float *)(src2->UdpA_Base + (src2_off >> 3));
	    
	    PUT_VALUE_(dst->UdpA_Base, dst_off, result, 1);
	    
	    src1_off+=src1->UdpL_PxlStride;
	    src2_off+=src2->UdpL_PxlStride;
	    dst_off+=dst->UdpL_PxlStride;
	    
	}
    }
}					/* end CompareGTII */

static void  CompareGEII(src1, src2, dst, cpp, y1, x1, x2)
UdpPtr	     	src1;
UdpPtr	     	src2;
UdpPtr	     	dst;
UdpPtr		cpp;
long int        y1;
long int	x1;
long int	x2;
{
    int			pixel;

    unsigned int  	src1_off;
    unsigned int  	src2_off;
    unsigned int        dst_off;
    long int		cpppxloff;
    int			cppbit;

    int   		result;
    int                 src1_mask;
    int                 src2_mask;

    src1_off = src1->UdpL_Pos;
    src2_off = src2->UdpL_Pos;
    dst_off =  dst->UdpL_Pos;

    if (cpp != NULL)
	cpppxloff = cpp->UdpL_Pos +
	    cpp->UdpL_ScnStride * (src1->UdpL_Y1 - y1);
    else
	cppbit = 1;			/* Fake all 1's in CPP */


    if (src1->UdpB_DType != UdpK_DTypeF) {

	/* Constrained types */
	src1_mask = (1 << src1->UdpW_PixelLength) -1;
	src2_mask = (1 << src2->UdpW_PixelLength) -1;

	for (pixel = x1; pixel <= x2; pixel++){
	    
	    result = 0;
	    
	    if (cpp != NULL) {
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
	    }	  
	    
	    if (cppbit)
		result = GET_VALUE_(src1->UdpA_Base, src1_off, src1_mask)
		    >= GET_VALUE_(src2->UdpA_Base, src2_off, src2_mask);

	    PUT_VALUE_(dst->UdpA_Base, dst_off, result, 1);
	    
	    src1_off+=src1->UdpL_PxlStride;
	    src2_off+=src2->UdpL_PxlStride;
	    dst_off+=dst->UdpL_PxlStride;
	}

    } else {

	/* Unconstrained data */
	for (pixel = x1; pixel <= x2; pixel++){
	    
	    result = 0;

	    if (cpp != NULL) {
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
	    }	  
	    
	    if (cppbit)
		result =  *(float *)(src1->UdpA_Base + (src1_off >> 3)) >=
		    *(float *)(src2->UdpA_Base + (src2_off >> 3));

	    PUT_VALUE_(dst->UdpA_Base, dst_off, result, 1);
	    
	    src1_off+=src1->UdpL_PxlStride;
	    src2_off+=src2->UdpL_PxlStride;
	    dst_off+=dst->UdpL_PxlStride;
	    
	}
    }
}					/* end CompareGEII */

static void  CompareLTIC(src, constant, dst, cpp, y1, x1, x2)
    UdpPtr	        src;
    unsigned long int	constant;
    UdpPtr	        dst;
    UdpPtr	        cpp;
    long int            y1;
    long int	        x1;
    long int	        x2;
{
    int			pixel;

    unsigned int  	src_off;
    unsigned int        dst_off;
    long int		cpppxloff;
    int			cppbit;

    int                 result;
    int                 src_mask;
    float               fconst;

    src_off = src->UdpL_Pos;
    dst_off = dst->UdpL_Pos;

    if (cpp != NULL)
	cpppxloff = cpp->UdpL_Pos +
	    cpp->UdpL_ScnStride * (src->UdpL_Y1 - y1);
    else
	cppbit = 1;			/* Fake all 1's in CPP */


    if (src->UdpB_DType != UdpK_DTypeF) {

	/* Constrained types */
	src_mask = (1 << src->UdpW_PixelLength) -1;

	for (pixel = x1; pixel <= x2; pixel++){
	    
	    result = 0;
	    
	    if (cpp != NULL) {
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
	    }	  
	    
	    if (cppbit)
		result = GET_VALUE_(src->UdpA_Base, src_off, src_mask)
		    < constant;

	    PUT_VALUE_(dst->UdpA_Base, dst_off, result, 1);
	    
	    src_off+=src->UdpL_PxlStride;
	    dst_off+=dst->UdpL_PxlStride;
	    
	}
    } else {

	/* Unconstrained data */
	fconst = constant;

	for (pixel = x1; pixel <= x2; pixel++){
	    
	    result = 0;

	    if (cpp != NULL) {
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
	    }	  
	    
	    if (cppbit)
		result = *(float *)(src->UdpA_Base + (src_off >> 3))
		    < fconst;

	    PUT_VALUE_(dst->UdpA_Base, dst_off, result, 1);
	    
	    src_off+=src->UdpL_PxlStride;
	    dst_off+=dst->UdpL_PxlStride;
	    
	}
    }
}					/* end CompareLTIC */

static void  CompareLEIC(src, constant, dst, cpp, y1, x1, x2)
    UdpPtr	        src;
    unsigned long int	constant;
    UdpPtr	        dst;
    UdpPtr	        cpp;
    long int            y1;
    long int	        x1;
    long int	        x2;
{
    int			pixel;

    unsigned int  	src_off;
    unsigned int        dst_off;
    long int		cpppxloff;
    int			cppbit;

    int                 result;
    int                 src_mask;
    float               fconst;

    src_off = src->UdpL_Pos;
    dst_off = dst->UdpL_Pos;

    if (cpp != NULL)
	cpppxloff = cpp->UdpL_Pos +
	    cpp->UdpL_ScnStride * (src->UdpL_Y1 - y1);
    else
	cppbit = 1;			/* Fake all 1's in CPP */


    if (src->UdpB_DType != UdpK_DTypeF) {

	/* Constrained types */
	src_mask = (1 << src->UdpW_PixelLength) -1;

	for (pixel = x1; pixel <= x2; pixel++){
	    
	    result = 0;
	    
	    if (cpp != NULL) {
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
	    }	  
	    
	    if (cppbit)
		result = GET_VALUE_(src->UdpA_Base, src_off, src_mask)
		    <= constant;
	    
	    PUT_VALUE_(dst->UdpA_Base, dst_off, result, 1);
	    
	    src_off+=src->UdpL_PxlStride;
	    dst_off+=dst->UdpL_PxlStride;
	    
	}
    } else {

	/* Unconstrained data */
	fconst = constant;

	for (pixel = x1; pixel <= x2; pixel++){
	    
	    result = 0;

	    if (cpp != NULL) {
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
	    }	  
	    
	    if (cppbit)
		result = *(float *)(src->UdpA_Base + (src_off >> 3))
		    <= fconst;
	    
	    PUT_VALUE_(dst->UdpA_Base, dst_off, result, 1);
	    
	    src_off+=src->UdpL_PxlStride;
	    dst_off+=dst->UdpL_PxlStride;
	    
	}
    }
}					/* end CompareLEIC */

static void  CompareEQIC(src, constant, dst, cpp, y1, x1, x2)
    UdpPtr		src;
    unsigned long int 	constant;
    UdpPtr		dst;
    UdpPtr		cpp;
    long int            y1;
    long int		x1;
    long int		x2;
{
    int			pixel;

    unsigned int  	src_off;
    unsigned int        dst_off;
    long int		cpppxloff;
    int			cppbit;

    int                 result;
    int                 src_mask;
    float               fconst;

    src_off = src->UdpL_Pos;
    dst_off = dst->UdpL_Pos;

    if (cpp != NULL)
	cpppxloff = cpp->UdpL_Pos +
	    cpp->UdpL_ScnStride * (src->UdpL_Y1 - y1);
    else
	cppbit = 1;			/* Fake all 1's in CPP */


    if (src->UdpB_DType != UdpK_DTypeF) {

	/* Constrained types */
	src_mask = (1 << src->UdpW_PixelLength) -1;

	for (pixel = x1; pixel <= x2; pixel++){
	    
	    result = 0;
	    
	    if (cpp != NULL) {
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
	    }	  
	    
	    if (cppbit)
		result = GET_VALUE_(src->UdpA_Base, src_off, src_mask)
		    == constant;

	    PUT_VALUE_(dst->UdpA_Base, dst_off, result, 1);
	    
	    src_off+=src->UdpL_PxlStride;
	    dst_off+=dst->UdpL_PxlStride;
	    
	}
    } else {

	/* Unconstrained data */
	fconst = constant;

	for (pixel = x1; pixel <= x2; pixel++){
	    
	    result = 0;

	    if (cpp != NULL) {
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
	    }	  
	    
	    if (cppbit)
		result = *(float *)(src->UdpA_Base + (src_off >> 3))
		    == fconst;

	    PUT_VALUE_(dst->UdpA_Base, dst_off, result, 1);
	    
	    src_off+=src->UdpL_PxlStride;
	    dst_off+=dst->UdpL_PxlStride;
	    
	}
    }
}					/* end CompareEQIC */

static void  CompareNEIC(src, constant, dst, cpp, y1, x1, x2)
    UdpPtr		src;
    unsigned long int 	constant;
    UdpPtr		dst;
    UdpPtr		cpp;
    long int            y1;
    long int		x1;
    long int		x2;
{
    int			pixel;

    unsigned int  	src_off;
    unsigned int        dst_off;
    long int		cpppxloff;
    int			cppbit;

    int                 result;
    int                 src_mask;
    float               fconst;

    src_off = src->UdpL_Pos;
    dst_off = dst->UdpL_Pos;

    if (cpp != NULL)
	cpppxloff = cpp->UdpL_Pos +
	    cpp->UdpL_ScnStride * (src->UdpL_Y1 - y1);
    else
	cppbit = 1;			/* Fake all 1's in CPP */


    if (src->UdpB_DType != UdpK_DTypeF) {

	/* Constrained types */
	src_mask = (1 << src->UdpW_PixelLength) -1;

	for (pixel = x1; pixel <= x2; pixel++){
	    
	    result = 0;
	    
	    if (cpp != NULL) {
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
	    }	  
	    
	    if (cppbit)
		result = GET_VALUE_(src->UdpA_Base, src_off, src_mask)
		    != constant;

	    PUT_VALUE_(dst->UdpA_Base, dst_off, result, 1);
	    
	    src_off+=src->UdpL_PxlStride;
	    dst_off+=dst->UdpL_PxlStride;
	    
	}
    } else {

	/* Unconstrained data */
	fconst = constant;

	for (pixel = x1; pixel <= x2; pixel++){
	    
	    result = 0;

	    if (cpp != NULL) {
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
	    }	  
	    
	    if (cppbit)
		result = *(float *)(src->UdpA_Base + (src_off >> 3))
		    != fconst;
	    
	    PUT_VALUE_(dst->UdpA_Base, dst_off, result, 1);
	    
	    src_off+=src->UdpL_PxlStride;
	    dst_off+=dst->UdpL_PxlStride;
	    
	}
    }
}					/* end CompareNEIC */

static void  CompareGTIC(src, constant, dst, cpp, y1, x1, x2)
    UdpPtr		src;
    unsigned long int 	constant;
    UdpPtr		dst;
    UdpPtr		cpp;
    long int            y1;
    long int		x1;
    long int		x2;
{
    int			pixel;

    unsigned int  	src_off;
    unsigned int        dst_off;
    long int		cpppxloff;
    int			cppbit;

    int                 result;
    int                 src_mask;
    float               fconst;

    src_off = src->UdpL_Pos;
    dst_off = dst->UdpL_Pos;

    if (cpp != NULL)
	cpppxloff = cpp->UdpL_Pos +
	    cpp->UdpL_ScnStride * (src->UdpL_Y1 - y1);
    else
	cppbit = 1;			/* Fake all 1's in CPP */


    if (src->UdpB_DType != UdpK_DTypeF) {

	/* Constrained types */
	src_mask = (1 << src->UdpW_PixelLength) -1;

	for (pixel = x1; pixel <= x2; pixel++){
	    
	    result = 0;
	    
	    if (cpp != NULL) {
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
	    }	  
	    
	    if (cppbit)
		result = GET_VALUE_(src->UdpA_Base, src_off, src_mask)
		    > constant;
	    
	    PUT_VALUE_(dst->UdpA_Base, dst_off, result, 1);
	    
	    src_off+=src->UdpL_PxlStride;
	    dst_off+=dst->UdpL_PxlStride;
	    
	}
    } else {

	/* Unconstrained data */
	fconst = constant;

	for (pixel = x1; pixel <= x2; pixel++){
	    
	    result = 0;

	    if (cpp != NULL) {
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
	    }	  
	    
	    if (cppbit)
		result = *(float *)(src->UdpA_Base + (src_off >> 3))
		    > fconst;

	    PUT_VALUE_(dst->UdpA_Base, dst_off, result, 1);
	    
	    src_off+=src->UdpL_PxlStride;
	    dst_off+=dst->UdpL_PxlStride;
	    
	}
    }
}					/* end CompareGTIC */

static void  CompareGEIC(src, constant, dst, cpp, y1, x1, x2)
    UdpPtr		src;
    unsigned long int 	constant;
    UdpPtr		dst;
    UdpPtr		cpp;
    long int            y1;
    long int		x1;
    long int		x2;
{
    int			pixel;

    unsigned int  	src_off;
    unsigned int        dst_off;
    long int		cpppxloff;
    int			cppbit;

    int                 result;
    int                 src_mask;
    float               fconst;

    src_off = src->UdpL_Pos;
    dst_off = dst->UdpL_Pos;

    if (cpp != NULL)
	cpppxloff = cpp->UdpL_Pos +
	    cpp->UdpL_ScnStride * (src->UdpL_Y1 - y1);
    else
	cppbit = 1;			/* Fake all 1's in CPP */


    if (src->UdpB_DType != UdpK_DTypeF) {

	/* Constrained types */
	src_mask = (1 << src->UdpW_PixelLength) -1;

	for (pixel = x1; pixel <= x2; pixel++){
	    
	    result = 0;
	    
	    if (cpp != NULL) {
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
	    }	  
	    
	    if (cppbit)
		result = GET_VALUE_(src->UdpA_Base, src_off, src_mask)
		    >= constant;

	    PUT_VALUE_(dst->UdpA_Base, dst_off, result, 1);
	    
	    src_off+=src->UdpL_PxlStride;
	    dst_off+=dst->UdpL_PxlStride;
	    
	}
    } else {

	/* Unconstrained data */
	fconst = constant;

	for (pixel = x1; pixel <= x2; pixel++){
	    
	    result = 0;

	    if (cpp != NULL) {
		cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
		cpppxloff+=cpp->UdpL_PxlStride;
	    }	  
	    
	    if (cppbit)
		result = *(float *)(src->UdpA_Base + (src_off >> 3))
		    >= fconst;

	    PUT_VALUE_(dst->UdpA_Base, dst_off, result, 1);
	    
	    src_off+=src->UdpL_PxlStride;
	    dst_off+=dst->UdpL_PxlStride;
	    
	}
    }
}					/* end CompareGEIC */

/*****************************************************************************
**  CompareGetTempUdp
**
**  FUNCTIONAL DESCRIPTION:
**
**	If either of the input UDP's are float data, provide a UDP
**      which describes the source UDP as float data.  Otherwise,
**      provide a UDP which points to a single scan line of the source
**      data.  In either case, initialize the UDP so it points to the
**      scanline before the first to be processed (CompareConvertScanline
**      increments before converting).
**
**  FORMAL PARAMETERS:
**
**	src    - Pointer to source UDP
**      other  - Pointer to the other UDP
**      y1    - First scanline we need
**      x1    - Leftmost pixel we need
**      x2    - Rightmost pixel we need
**
**  FUNCTION VALUE:
**
**      Returns a pointer to a UDP describing the data.
**
*****************************************************************************/
static UdpPtr CompareGetTempUdp(src, other, y1, x1, x2)
    UdpPtr src;
    UdpPtr other;
    long int y1;
    long int x1, x2;
{
    UdpPtr dst;

    dst = (UdpPtr) DdxMalloc_(sizeof(UdpRec));
    if( dst == NULL ) return( (UdpPtr) BadAlloc );

    if( other && other->UdpB_DType == UdpK_DTypeF
		&& src->UdpB_DType != UdpK_DTypeF )
	{
	dst->UdpW_PixelLength	= sizeof(float) * 8;
	dst->UdpB_DType		= UdpK_DTypeF;
	dst->UdpB_Class		= UdpK_ClassA;
	dst->UdpL_PxlStride	= dst->UdpW_PixelLength;
	dst->UdpL_PxlPerScn	= x2 - x1 + 1;
	dst->UdpL_ScnStride	= dst->UdpL_PxlPerScn * dst->UdpL_PxlStride;
	dst->UdpL_ArSize	= dst->UdpL_PxlPerScn * sizeof(float) * 8;
	dst->UdpA_Base		= DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL )
	    {
	    DdxFree_(dst);
	    return( (UdpPtr) BadAlloc );
	    }
	dst->UdpL_Pos		= 0;
	dst->UdpL_CompIdx	= src->UdpL_CompIdx;
	dst->UdpL_Levels	= src->UdpL_Levels;
	}
    else
	{
	*dst = *src;
	dst->UdpL_PxlPerScn = x2 - x1 + 1;
	dst->UdpL_ScnStride = dst->UdpL_PxlPerScn * dst->UdpL_PxlStride;
	dst->UdpL_Pos += ((y1 - src->UdpL_Y1 -1) * src->UdpL_ScnStride +
			  (x1 - src->UdpL_X1) * src->UdpL_PxlStride);
	}
    /* One scanline UDP */
    dst->UdpL_X1     = x1;
    dst->UdpL_X2     = x2;
    dst->UdpL_Y1     = y1 - 1;
    dst->UdpL_Y2     = y1 - 1;
    dst->UdpL_ScnCnt =  1;

    return( dst );
}					/* end CompareGetTempUdp */

/*****************************************************************************
**  CompareDestroyTempUdp
**
**  FUNCTIONAL DESCRIPTION:
**
**	Free a UDP created by CompareGetTempUdp.
**
**  FORMAL PARAMETERS:
**
**	udp     - Pointer to the udp to free.
**      realudp - Pointer to a udp which might actually own the storage
**                pointed to by udp.
**
*****************************************************************************/
static void CompareDestroyTempUdp( udp, realudp )
UdpPtr udp;
UdpPtr realudp;

{
    if( IsPointer_(udp->UdpA_Base) && udp->UdpA_Base != realudp->UdpA_Base )
	DdxFreeBits_( udp->UdpA_Base );
    DdxFree_( udp );
}					/* end CompareDestroyTempUdp */

/*****************************************************************************
**  CollectComponents
**
**  FUNCTIONAL DESCRIPTION:
**
**	Collect the next quantum from each drain, until all
**      drains have provided one scanline.
**
**  FORMAL PARAMETERS:
**
**	drain    - Array of pointers to the source drains.
**      data     - Array to receive the data segment pointers.
**      ctx      - Current pipe context.
**      ncmp     - Number of components (drains) for the image.
**
**  FUNCTION VALUE:
**
**      Success if all drains have contributed, else -1 or error code.
**
*****************************************************************************/
static int CollectComponents(drain, data, ctx, ncmp)
PipeDrainPtr	    drain[XieK_MaxComponents];
PipeDataPtr  	    data[XieK_MaxComponents];
ComparePipeCtxPtr   ctx;
int                 ncmp;
{
    int i;

    /* Collect each component of the current quantum */
    for( i = 0; i < ncmp; i++ )
	if( data[i] == NULL )
	    {
	    data[i] = DdxRmGetData_( ctx, drain[i] );
	    if( !IsPointer_(data[i]) )
		return( data[i] == NULL ? -1 : (int) data[i] );
	    }
    return( Success );
}					/* end CollectComponents */

/*****************************************************************************
**  CompareConvertScanline
**
**  FUNCTIONAL DESCRIPTION:
**
**	Adjust a UDP to point to the specified scanline of the source
**	UDP.  If the destination udp is of type float, convert the 
**      source data if necessary.
**
**  FORMAL PARAMETERS:
**
**	src      - Pointer to the source UDP.
**      dst      - Pointer to the UDP to be adjusted.
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int CompareConvertScanline(src, dst)
    UdpPtr src;
    UdpPtr dst;
{
    int	status = Success;

    dst->UdpL_Y1++;
    dst->UdpL_Y2++;

    if (dst->UdpB_DType == UdpK_DTypeF && src->UdpB_DType != UdpK_DTypeF)
	status = DdxConvert_( src, dst, XieK_MoveMode );
    else
	dst->UdpL_Pos += src->UdpL_ScnStride;

    return( status );
}					/* end CompareConvertScanline */
