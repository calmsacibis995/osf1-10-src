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
**      This module contains support for the TRANSLATE operation.
**
**  ENVIRONMENT:
**
**	VAX/VMS V5.3
**	VAX ULTRIX  V3.1
**	RISC ULTRIX 3.1
**
**  AUTHORS:
**
**	Gary Grebus
**
**  CREATION DATE:     
**
**	Thu Feb 22 14:39:51 1990
**
*******************************************************************************/

#include <stdio.h>

#include "SmiTranslate.h"

/*
**  Table of contents
*/
int             SmiCreateTranslate();
int             SmiTranslate();

static int 	_TranslateInitialize();
static int 	_TranslateActivate();
static int 	_TranslateAbort();
static int 	_TranslateDestroy();

static int	_TranslateResolveCpp();
static PipeDataPtr _TranslateNxtSrc1Data();

/*
**  Equated Symbols
*/

/*
**  External References
*/

/*
 * Macros
 */
#define Min_(a,b) ((a) <= (b) ? (a) : (b))
#define Max_(a,b) ((a) > (b) ? (a) : (b))
/*
**  Local storage
*/
    /*
    **  Translate Element Vector
    */
static PipeElementVector TranslatePipeElement = {
    NULL,				/* FLINK			    */
    NULL,				/* BLINK			    */
    sizeof(PipeElementVector),		/* Size				    */
    TypePipeElementVector,		/* Structure type		    */
    StypeTranslateElement, 	        /* Structure subtype		    */
    sizeof(TranslatePipeCtx),		/* Context block size		    */
    0,					/* No input flag		    */
    0,					/* Reserved flags		    */
    _TranslateInitialize,		/* Initialize entry		    */
    _TranslateActivate,			/* Activate entry		    */
    _TranslateAbort,			/* Flush entry			    */
    _TranslateDestroy,			/* Destroy entry		    */
    _TranslateAbort			/* Abort entry			    */
    };

/*****************************************************************************
**  SmiCreateTranslate
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function creates and fills in a TRANSLATE pipeline element.
**
**  FORMAL PARAMETERS:
**
**	pipe   - Handle for pipe being created
**      src1   - Pointer to sink containing source data being translated
**      src2   - Pointer to sink containing source data being translated into
**      dst    - Pointer to sink to receive the result
**      srcidc - Pointer to UDP describing the region to be copied from src1
**      dstidc - Pointer to UDP describing the region to be copied into
**      
**  FUNCTION VALUE:
**
**      Success or standard X error code.
**
*****************************************************************************/
int 	    SmiCreateTranslate(pipe,src1,src2,dst,srcidc,dstidc)
Pipe		     pipe;
PipeSinkPtr	     src1;
PipeSinkPtr	     src2;
PipeSinkPtr	     dst;
UdpPtr		     srcidc;
UdpPtr		     dstidc;
{
    long             width, height, status;
    UdpPtr           newcpp;
    long             sx1, sx2, sy1, sy2, dx1, dx2, dy1, dy2;
    int              i;

    TranslatePipeCtxPtr ctx = (TranslatePipeCtxPtr)
			DdxCreatePipeCtx_( pipe, &TranslatePipeElement, FALSE );
    if( !IsPointer_(ctx) ) return( (int) ctx );


    /*
     * Calculate the location and bounds of the region to be translated.
     */
    if (srcidc == NULL){
	sx1 = SnkX1_(src1, 0);
	sy1 = SnkY1_(src1, 0);
	sx2 = SnkX2_(src1, 0);
	sy2 = SnkY2_(src1, 0);
    } else {
	sx1 = Max_(srcidc->UdpL_X1, SnkX1_(src1, 0));
	sy1 = Max_(srcidc->UdpL_Y1, SnkY1_(src1, 0));
	sx2 = Min_(srcidc->UdpL_X2, SnkX2_(src1, 0));
	sy2 = Min_(srcidc->UdpL_Y2, SnkY2_(src1, 0));
    }

    if (dstidc == NULL) {
	dx1 = SnkX1_(dst, 0);
	dy1 = SnkY1_(dst, 0);
	dx2 = SnkX2_(dst, 0);
	dy2 = SnkY2_(dst, 0);
    } else {
	dx1 = Max_(dstidc->UdpL_X1, SnkX1_(dst, 0));
	dy1 = Max_(dstidc->UdpL_Y1, SnkY1_(dst, 0));
	dx2 = Min_(dstidc->UdpL_X2, SnkX2_(dst, 0));
	dy2 = Min_(dstidc->UdpL_Y2, SnkY2_(dst, 0));
    }
    width  = Min_(sx2 - sx1, dx2 - dx1) + 1;
    height = Min_(sy2 - sy1, dy2 - dy1) + 1;
    if (width <= 0 || height <= 0) {
	DdxFree_( ctx );		/* DIX is supposed to catch this */
	return( BadValue );
    }

    TrnSrcX1_(ctx) = sx1;
    TrnSrcY1_(ctx) = sy1;
    TrnSrcY2_(ctx) = sy1 + height - 1;
    TrnRoiX1_(ctx) = dx1;
    TrnRoiY1_(ctx) = dy1;
    TrnRoiX2_(ctx) = dx1 + width - 1;
    TrnRoiY2_(ctx) = dy1 + height - 1;

    /*
    **	Copy parameters into context block.
    */
    TrnSrcSnk1_(ctx)   = src1;
    TrnSrcSnk2_(ctx)   = src2;
    TrnDstSnk_(ctx)    = dst;
    TrnSrcIdc_(ctx)    = srcidc;
    TrnDstIdc_(ctx)    = dstidc;
    
    /*
    **  Setup one drain per component on the first source sink.
    **	Setup one drain on the second source sink.
    */
    for( i = 0; i < XieK_MaxComponents && IsPointer_(SnkUdpPtr_(src1,i)); i++ )
	{
	TrnSrcDrn1_(ctx, i) = DdxRmCreateDrain_( src1, 1<<i );
	if( !IsPointer_(TrnSrcDrn1_(ctx, i))) return( (int)TrnSrcDrn1_(ctx,i));
	TrnSrc1State_(ctx, i) = 0;
	}
    TrnNcmp_(ctx) = i;

    TrnSrcDrn2_(ctx) = DdxRmCreateDrain_( src2, -1 );
    if( !IsPointer_(TrnSrcDrn2_(ctx)) ) return( (int) TrnSrcDrn2_(ctx) );

    for( i = 0; i < TrnNcmp_(ctx); i++ )
	TrnDat1_(ctx, i) = NULL;

    TrnDat2_(ctx) = NULL;
    /*
     * Derive which CPP (if any) to use.
     */
    status = _TranslateResolveCpp(srcidc, dstidc, &newcpp);
    if( status != Success ) return( status );
    TrnCpp_(ctx) = newcpp;

    /*
    **	Handle any data type.
    */
    for( i = 0; i < TrnNcmp_(ctx); i++ )
	DdxRmSetDType_( TrnSrcDrn1_(ctx,i), UdpK_DTypeUndefined, DtM_Any );
    DdxRmSetDType_( TrnSrcDrn2_(ctx), UdpK_DTypeUndefined, DtM_Any );
    DdxRmSetDType_( TrnDstSnk_(ctx),  UdpK_DTypeUndefined, DtM_Any );

    /*
    **  Use quantum of 1 on the src2 and destination drains.  Quantum
    **  on the src1 drain will be adjusted dynamically.
    */
    DdxRmSetQuantum_( TrnSrcDrn2_(ctx), 1 );
    DdxRmSetQuantum_( TrnDstSnk_(ctx),  1 );

    return( Success );
}					/* end SmiCreateTranslate */

/*****************************************************************************
**  _TranslateInitialize
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function initializes the TRANSLATE pipeline element prior to
**	an activation.
**
**  FORMAL PARAMETERS:
**
**      ctx - translate pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	_TranslateInitialize(ctx)
TranslatePipeCtxPtr	ctx;
{
    int i, dtype, status;

    /*
    **	Initialize source 1 drains.
    */
    for( i = 0; i < TrnNcmp_(ctx); i++ )
	{
	status  = DdxRmInitializePort_( CtxHead_(ctx), TrnSrcDrn1_(ctx,i) );
	if( status != Success ) return( status );

	if( i == 0 )
	    dtype = DdxRmGetDType_( TrnSrcDrn1_(ctx,i) );

	else if( dtype != DdxRmGetDType_(TrnSrcDrn1_(ctx,i)) )
	    return( BadMatch );
	}
    /*
    **	Initialize source 2 drain and make it our current input.
    */
    status = DdxRmInitializePort_( CtxHead_(ctx), TrnSrcDrn2_(ctx) );
    if( status != Success ) return( status );
    CtxInp_(ctx) = TrnSrcDrn2_(ctx);

    /*
    **	Set our destination sink DType to match drain 2 and initialize it.
    */
    dtype = DdxRmGetDType_( TrnSrcDrn2_(ctx) );
    DdxRmSetDType_( TrnDstSnk_(ctx), dtype, 1<<dtype );
    status = DdxRmInitializePort_( CtxHead_(ctx), TrnDstSnk_(ctx) );

    return( status );
}

/*****************************************************************************
**  _TranslateActivate
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine reads data from the pipeline and copies background
**	(source2) data to the destination, inserting to-be-translated 
**      (source1) data as necessary.  
**
**  FORMAL PARAMETERS:
**
**      ctx - translate pipeline element context
**
**  FUNCTION VALUE:
**
**      Success or standard X error code.
**
*****************************************************************************/
static int 	    _TranslateActivate(ctx)
TranslatePipeCtxPtr    ctx;
{

    PipeDataPtr src1, src2, dst;
    int status;
    int comp;

    status = Success;

    while( status == Success ) {

	/* Get the next unprocessed scanline of background data */
	src2 = (TrnDat2_(ctx) == NULL)
	     ? DdxRmGetData_( ctx, TrnSrcDrn2_(ctx) ) : TrnDat2_(ctx);

	if (!IsPointer_(src2)) {
	    status = (int) src2;
	    break;
	}
	TrnDat2_(ctx) = src2;

	if (DatY1_(src2) < TrnRoiY1_(ctx) || DatY1_(src2) > TrnRoiY2_(ctx) ) {

	    /* This scanline comes before or after the destination ROI.
	     * Just copy it through. */
	    status = DdxRmPutData_( TrnDstSnk_(ctx), src2 );
	    TrnDat2_(ctx) = NULL;

	} else {

	    /* This scanline falls within destination ROI */
	    comp = DatCmpIdx_(src2);

	    /* Get the next chunk of to-be-translated data */
	    src1 = _TranslateNxtSrc1Data(ctx, comp);

	    if (!IsPointer_(src1)) {
		status = (int) src1;
		break;
	    }
	    TrnDat1_(ctx, comp) = src1;
	    
	    if( DatY1_(src1) < TrnSrcY1_(ctx) ||
	       DatY1_(src1) > TrnSrcY2_(ctx)) {

		/* This segment falls before or after the ROI.
		 * Throw it away.  */
		DdxRmDeallocData_( src1 );
		TrnDat1_(ctx, comp) = NULL;

	    } else {
		/* We have the segment of to-be-translated data, and
		 * a background scanline within the destination ROI.
		 * Translate a scanline.
		 */
		dst = DdxRmAllocData_(TrnDstSnk_(ctx), comp,  DatY1_(src2), 1);
		if( !IsPointer_(dst) )
		    status = (int) dst;

		if( TrnCpp_(ctx) == NULL ) {

		    if( status == Success && TrnRoiX1_(ctx) > DatX1_(src2) )
			/* Copy the region left of the destination ROI */
			status = DdxCopy_( DatUdpPtr_(src2), DatUdpPtr_(dst),
					   0, 0,
					   DatX1_(dst), DatY1_(dst),
					   TrnRoiX1_(ctx) - DatX1_(dst), 1 );

		    if( status == Success && TrnRoiX2_(ctx) < DatX2_(src2) )
			/* Copy the region right of the destination ROI */
			status = DdxCopy_( DatUdpPtr_(src2), DatUdpPtr_(dst),
					   TrnRoiX2_(ctx) + 1, 0,
					   TrnRoiX2_(ctx) + 1, DatY1_(dst),
					   DatX2_(dst) - TrnRoiX2_(ctx), 1 );
			
		    if( status == Success )
			/* Translate the region in the ROI */
			status = DdxCopy_( DatUdpPtr_(src1), DatUdpPtr_(dst),
					   TrnSrcX1_(ctx),
					   DatY1_(dst) - TrnRoiY1_(ctx),
					   TrnRoiX1_(ctx), DatY1_(dst),
					   TrnRoiX2_(ctx)-TrnRoiX1_(ctx)+1, 1 );
		} else {

		    /* Copy the background data to the destination */
		    if ( status == Success )
			status = DdxConvert_( DatUdpPtr_(src2),
					      DatUdpPtr_(dst), XieK_MoveMode );

		    if( status == Success )
			/* Translate the pixels selected by the CPP */
			status = DdxCopyCpp_(DatUdpPtr_(src1), DatUdpPtr_(dst),
					     TrnCpp_(ctx), 
					     TrnSrcX1_(ctx), 0,
					     TrnRoiX1_(ctx), DatY1_(dst),
					     TrnRoiX2_(ctx)-TrnRoiX1_(ctx)+1,1);
		}

		if( status == Success )
		    status  = DdxRmPutData_( TrnDstSnk_(ctx), dst );
		else
		    DdxRmDeallocData_(dst);

		if (DatY2_(src2) == TrnRoiY2_(ctx)) {
		    /* This is the last scanline which will receive translated
		     * data.  Throw away the segment of to-be-translated data
		     */
		    DdxRmDeallocData_( src1 );
		    TrnDat1_(ctx, comp) = NULL;
		}

		DdxRmDeallocData_(src2);
		TrnDat2_(ctx) = NULL;

	    }
	}
    }
    return( status );
}

/*****************************************************************************
**  _TranslateAbort
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine cleans up any data queued on the drains, and
**      deallocates any data held in the local context.
**
**  FORMAL PARAMETERS:
**
**      ctx - translate pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _TranslateAbort(ctx)
TranslatePipeCtxPtr    ctx;
{
    int i;
    PipeDataPtr src;

    for( i = 0; i < TrnNcmp_(ctx); i++ )
	if( IsPointer_(TrnDat1_(ctx, i)) )
	    TrnDat1_(ctx,i) = DdxRmDeallocData_( TrnDat1_(ctx,i) );

    if( IsPointer_(TrnDat2_(ctx)) )
	TrnDat2_(ctx) = DdxRmDeallocData_( TrnDat2_(ctx) );

    /* If the destination ROI ended at the end of the image,
     * we will not have read any data beyond the source ROI in
     * the to-be-translated image.  Flush that data off of the drain. */
    for( i = 0; i < TrnNcmp_(ctx); i++ )
	{
	src = _TranslateNxtSrc1Data(ctx,i);
	if (IsPointer_(src))
	    DdxRmDeallocData_( src );
	}

    return( Success );
}

/*****************************************************************************
**  _TranslateDestroy
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine deallocates all resources allocated by the TRANSLATE
**	pipeline element.
**
**  FORMAL PARAMETERS:
**
**      ctx - translate pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _TranslateDestroy(ctx)
TranslatePipeCtxPtr    ctx;
{
    int i;

    for( i = 0; i < TrnNcmp_(ctx); i++)
	TrnSrcDrn1_(ctx, i) = DdxRmDestroyDrain_( TrnSrcDrn1_(ctx,i) );
    TrnSrcDrn2_(ctx) = DdxRmDestroyDrain_( TrnSrcDrn2_(ctx) );

    if( IsPointer_(TrnCpp_(ctx)) ) 
	{
	if( IsPointer_(TrnCpp_(ctx)->UdpA_Base)
	 && TrnCpp_(ctx)->UdpA_Base != TrnSrcIdc_(ctx)->UdpA_Base
	 && TrnCpp_(ctx)->UdpA_Base != TrnDstIdc_(ctx)->UdpA_Base )
	    /* We allocated additional UDP storage - free it */
	    DdxFreeBits_(TrnCpp_(ctx)->UdpA_Base);

	TrnCpp_(ctx) = (UdpPtr) DdxFree_( TrnCpp_(ctx) );
	}

    return( Success );
}

/*****************************************************************************
**  SmiTranslate
**
**  FUNCTIONAL DESCRIPTION:
**
**      Copy a region of an input UDP into a region of an output UDP
**
**  FORMAL PARAMETERS:
**
**      src     - source data descriptor
**	dst	- destination data descriptor
**      srcidc  - source IDC
**      dstidc  - destination IDC
**
**  FUNCTION VALUE:
**
*****************************************************************************/
int 	    SmiTranslate( src1, src2, dst, srcidc, dstidc)
UdpPtr		src1;
UdpPtr		src2;
UdpPtr		dst;
UdpPtr          srcidc;
UdpPtr          dstidc;
{
    unsigned    width, height;
    int         status;
    UdpPtr      newcpp;
    UdpPtr      inroi, outroi;
    
    /* Copy the "background" image to the destination */
    status = DdxConvert_( src2, dst, XieK_MoveMode );
    if( status != Success ) return( status );

    /* Determine the bounds of the destination region */
    inroi = srcidc == NULL ? src1 : srcidc;
    inroi->UdpL_X1 = Max_(inroi->UdpL_X1, src1->UdpL_X1);
    inroi->UdpL_X2 = Min_(inroi->UdpL_X2, src1->UdpL_X2);
    inroi->UdpL_Y1 = Max_(inroi->UdpL_Y1, src1->UdpL_Y1);
    inroi->UdpL_Y2 = Min_(inroi->UdpL_Y2, src1->UdpL_Y2);
    if (inroi->UdpL_X1 > inroi->UdpL_X2 || inroi->UdpL_Y1 > inroi->UdpL_Y2)
	return( BadValue );
    inroi->UdpL_PxlPerScn = inroi->UdpL_X2 - inroi->UdpL_X1 + 1;
    inroi->UdpL_ScnCnt    = inroi->UdpL_Y2 - inroi->UdpL_Y1 + 1;

    outroi = dstidc == NULL ? dst : dstidc;
    outroi->UdpL_X1 = Max_(outroi->UdpL_X1, dst->UdpL_X1);
    outroi->UdpL_X2 = Min_(outroi->UdpL_X2, dst->UdpL_X2);
    outroi->UdpL_Y1 = Max_(outroi->UdpL_Y1, dst->UdpL_Y1);
    outroi->UdpL_Y2 = Min_(outroi->UdpL_Y2, dst->UdpL_Y2);
    if (outroi->UdpL_X1 > outroi->UdpL_X2 || outroi->UdpL_Y1 > outroi->UdpL_Y2)
	return( BadValue );
    outroi->UdpL_PxlPerScn = outroi->UdpL_X2 - outroi->UdpL_X1 + 1;
    outroi->UdpL_ScnCnt    = outroi->UdpL_Y2 - outroi->UdpL_Y1 + 1;

    width  = Min_(inroi->UdpL_PxlPerScn,  outroi->UdpL_PxlPerScn);
    height = Min_(inroi->UdpL_ScnCnt, outroi->UdpL_ScnCnt);

    /* Determine which CPP to use (if any) */
    status = _TranslateResolveCpp( srcidc, dstidc, &newcpp );
    if( status != Success ) return( status );

    /*
    **	Translate the region of interest.
    */
    if( newcpp == NULL )
	status = DdxCopy_( src1, dst,
			   inroi->UdpL_X1,  inroi->UdpL_Y1,
			   outroi->UdpL_X1, outroi->UdpL_Y1, width, height );
    else
	status = DdxCopyCpp_( src1, dst, newcpp,
			      inroi->UdpL_X1,  inroi->UdpL_Y1,
			      outroi->UdpL_X1, outroi->UdpL_Y1, width, height );

    /* Free the temporary CPP if one was created */
    if( newcpp != NULL && newcpp != srcidc && newcpp != dstidc )
	{
	if( IsPointer_(newcpp->UdpA_Base) )
	   DdxFreeBits_(newcpp->UdpA_Base);
	DdxFree_(newcpp);
	}
    return( status );
}				    /* end SmiTranslate */

/*****************************************************************************
**  _TranslateResolveCpp
**
**  FUNCTIONAL DESCRIPTION:
**
**      Determine the set of pixels to be processed given UDP's representing
**      the IDC's.  The source IDC may be omitted (NULL).  Either IDC
**      could represent only an ROI.
**
**  FORMAL PARAMETERS:
**
**      srcidc     - Udp pointer for source CPP (may be NULL)
**	dstidc	   - Udp Pointer for destination CPP
**      newcpp     - Pointer to return location for result CPP.
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int _TranslateResolveCpp( srcidc, dstidc, outcpp)
UdpPtr          srcidc;
UdpPtr          dstidc;
UdpPtr          *outcpp;
{
    long int         width, height, status;
    UdpRec           tmpsrc, tmpdst;
    UdpPtr           newcpp;

    if( srcidc == NULL || srcidc->UdpA_Base == NULL )
	/* Source does not have a CPP */
	if( dstidc == NULL || dstidc->UdpA_Base == NULL )
	    /* There is no CPP at all */
	    newcpp = NULL;
	else
	    {
	    /* Only the destination is a CPP */
	    newcpp = (UdpPtr) DdxMalloc_(sizeof(UdpRec));
	    if( newcpp == NULL ) return( BadAlloc );
	    *newcpp = *dstidc;
	    }
    else
	/* Source does have a CPP */
	if( dstidc == NULL || dstidc->UdpA_Base == NULL )
	    {
	    /* Only the source has a CPP.  Adjust its position to be
	     * relative to the destination.
	     */
	    newcpp = (UdpPtr) DdxMalloc_(sizeof(UdpRec));
	    if( newcpp == NULL ) return( BadAlloc );
	    *newcpp = *srcidc;
	    newcpp->UdpL_X1 = dstidc->UdpL_X1;
	    newcpp->UdpL_X2 = dstidc->UdpL_X2;
	    newcpp->UdpL_Y1 = dstidc->UdpL_Y1;
	    newcpp->UdpL_Y2 = dstidc->UdpL_Y2;
	    }
	else
	    {
	    /* Both IDC's contained a CPP.  Use the intersection. */
	    newcpp = (UdpPtr) DdxMalloc_(sizeof(UdpRec));
	    if( newcpp == NULL ) return( BadAlloc );

	    /* Align the two input CPP's at upper left corner */
	    tmpsrc = *srcidc;
	    tmpsrc.UdpL_X2 = tmpsrc.UdpL_X2 - tmpsrc.UdpL_X1;
	    tmpsrc.UdpL_Y2 = tmpsrc.UdpL_Y2 - tmpsrc.UdpL_Y1;
	    tmpsrc.UdpL_X1 = 0;
	    tmpsrc.UdpL_Y1 = 0;

	    tmpdst = *dstidc;
	    tmpdst.UdpL_X2 = tmpdst.UdpL_X2 - tmpdst.UdpL_X1;
	    tmpdst.UdpL_Y2 = tmpdst.UdpL_Y2 - tmpdst.UdpL_Y1;
	    tmpdst.UdpL_X1 = 0;
	    tmpdst.UdpL_Y1 = 0;
	    
	    /* Create the new, intersected CPP */
	    width  = Min_(srcidc->UdpL_PxlPerScn, dstidc->UdpL_PxlPerScn);
	    height = Min_(srcidc->UdpL_ScnCnt,    dstidc->UdpL_ScnCnt);

	    newcpp->UdpW_PixelLength = 1;
	    newcpp->UdpB_DType	     = UdpK_DTypeVU;
	    newcpp->UdpB_Class	     = UdpK_ClassUBA;
	    newcpp->UdpL_PxlStride   = 1;
	    newcpp->UdpL_ScnStride   = width;
	    newcpp->UdpL_X1	     = 0;
	    newcpp->UdpL_X2	     = width - 1;
	    newcpp->UdpL_Y1	     = 0;
	    newcpp->UdpL_Y2	     = height - 1;
	    newcpp->UdpL_PxlPerScn   = width;
	    newcpp->UdpL_ScnCnt	     = height;
	    newcpp->UdpL_Pos	     = 0;
	    newcpp->UdpL_CompIdx     = 0;
	    newcpp->UdpL_Levels	     = 2;
	    newcpp->UdpL_ArSize	     = height * width;
	    newcpp->UdpA_Base	     = DdxMallocBits_(newcpp->UdpL_ArSize);
	    status = DdxLogical_(&tmpsrc,&tmpdst, NULL, newcpp, NULL, XieK_AND);
	    if( status != Success )
		{
		DdxFree_(newcpp);
		return( status );
		}
	    newcpp->UdpL_X1	     = dstidc->UdpL_X1;
	    newcpp->UdpL_X2	     = dstidc->UdpL_X1 + width - 1;
	    newcpp->UdpL_Y1	     = dstidc->UdpL_Y1;
	    newcpp->UdpL_Y2	     = dstidc->UdpL_Y1 + height - 1;
	    }
    *outcpp = newcpp;

    return( Success );
}

/*****************************************************************************
**  _TranslateNxtSrc1Data
**
**  FUNCTIONAL DESCRIPTION:
**
**      Attempt to read the next quantum of translated (src1) data
**      for the specified component.  The size of the quantum is set
**      based on the state of that component.
**
**  FORMAL PARAMETERS:
**
**      ctx     - Pipe context pointer
**	comp	- Component to read
**
**  FUNCTION VALUE:
**
**      Pointer to the data segmnt read, or error code.
**
*****************************************************************************/
static PipeDataPtr _TranslateNxtSrc1Data( ctx, comp )
    TranslatePipeCtxPtr ctx;
    int comp;
{
    PipeDataPtr src;
    int quantum;
    int next_state;

    quantum = 0;

    if ( TrnDat1_(ctx, comp) != NULL )
	src = TrnDat1_(ctx, comp);	/* Already have the segment */

    else {
	next_state = TrnSrc1State_(ctx, comp);
	switch (TrnSrc1State_(ctx, comp)) {
	  case 0 :
	      /* Get all the data before the source ROI */
	      quantum = TrnSrcY1_(ctx) - SnkY1_(TrnSrcSnk1_(ctx), comp);
	      next_state++;
	      if (quantum != 0) break;
					/* else fall through */
	  case 1 :
	      /* Get all the data within the source ROI */
	      quantum = TrnSrcY2_(ctx) - TrnSrcY1_(ctx) + 1;
	      next_state++;
	      if (quantum != 0) break;

	  case 2 :
	      /* Get all the data after the source ROI */
	      quantum = SnkY2_(TrnSrcSnk1_(ctx), comp) - TrnSrcY2_(ctx);
	      next_state++;
	      break;

	  default :
	      /* Something is amiss */
	      return( (PipeDataPtr)BadImplementation );
	  }

	DdxRmSetQuantum_( TrnSrcDrn1_(ctx,comp), quantum );
	src = DdxRmGetData_( ctx, TrnSrcDrn1_(ctx,comp) );
	if (IsPointer_(src))
	    TrnSrc1State_(ctx,comp) = next_state;
    }
	    
    return( src );
}
