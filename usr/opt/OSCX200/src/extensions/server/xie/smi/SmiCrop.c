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
**      This module contains support for the pipeline crop operation.
**	Non-pipelined crop is still handled in the DIX.
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

#include "SmiCrop.h"

/*
**  Table of contents
*/
int             SmiCreateCrop();

static int 	_CropInitialize();
static int 	_CropActivate();
static int 	_CropDestroy();


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
    **  Crop Element Vector
    */
static PipeElementVector CropPipeElement = {
    NULL,				/* FLINK			    */
    NULL,				/* BLINK			    */
    sizeof(PipeElementVector),		/* Size				    */
    TypePipeElementVector,		/* Structure type		    */
    StypeCropElement,			/* Structure subtype		    */
    sizeof(CropPipeCtx),		/* Context block size		    */
    0,					/* No input flag		    */
    0,					/* Reserved flags		    */
    _CropInitialize,			/* Initialize entry		    */
    _CropActivate,			/* Activate entry		    */
    NULL,				/* Flush entry			    */
    _CropDestroy,			/* Destroy entry		    */
    NULL				/* Abort entry point		    */
    };

/*****************************************************************************
**  SmiCreateCrop
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function fills in a newly created CROP pipeline element.
**
**  FORMAL PARAMETERS:
**
**      ctx - crop pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
int 	    SmiCreateCrop( pipe, src, dst, x, y, width, height )
Pipe		     pipe;
PipeSinkPtr	     src;
PipeSinkPtr	     dst;
int		     x;
int		     y;
int		     width;
int		     height;
{
    CropPipeCtxPtr ctx = (CropPipeCtxPtr)
			  DdxCreatePipeCtx_( pipe, &CropPipeElement, FALSE );
    if( !IsPointer_(ctx) ) return( (int) ctx );
    /*
    **	Copy parameters into context block.
    */
    CrpSrcSnk_(ctx)    = src;
    CrpDstSnk_(ctx)    = dst;
    CrpRoiX_(ctx)      = x;
    CrpRoiY_(ctx)      = y;
    CrpRoiWidth_(ctx)  = width;
    CrpRoiHeight_(ctx) = height;
  
    /*
    **	Setup one drain from source sink, receiving all components.
    */
    CrpSrcDrn_(ctx) = DdxRmCreateDrain_( CrpSrcSnk_(ctx), -1 );
    if( !IsPointer_(CrpSrcDrn_(ctx)) ) return( (int) CrpSrcDrn_(ctx) );
    /*
    **	Since cropping is done entirely by manipulating UDP fields,
    **	we can handle any data type or quantum size.
    */
    DdxRmSetDType_( CrpSrcDrn_(ctx), UdpK_DTypeUndefined, DtM_Any );
    DdxRmSetDType_( CrpDstSnk_(ctx), UdpK_DTypeUndefined, DtM_Any );

    return( Success );
}					/* end SmiCreateCrop */

/*****************************************************************************
**  _CropInitialize
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function initializes the CROP pipeline element prior to
**	an activation.
**
**  FORMAL PARAMETERS:
**
**      ctx - crop pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	_CropInitialize(ctx)
CropPipeCtxPtr	ctx;
{
    int dtype, status;

    /*
    **	Initialize the source drain.
    */
    CtxInp_(ctx) = CrpSrcDrn_(ctx);
    status = DdxRmInitializePort_( CtxHead_(ctx), CrpSrcDrn_(ctx) );
    if( status != Success ) return( status );

    /*
    **	Set our destination sink DType to match our drain and initialize it.
    */
    dtype = DdxRmGetDType_( CrpSrcDrn_(ctx) );
    DdxRmSetDType_( CrpDstSnk_(ctx), dtype, 1<<dtype );
    status = DdxRmInitializePort_( CtxHead_(ctx), CrpDstSnk_(ctx) );

    return( status );
}

/*****************************************************************************
**  _CropActivate
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine reads data from the pipeline and crops each quantum,
**	as necessary.  This is done by adjusting the UDP describing the
**	current quantum so that it describes only the region of interest.
**
**  FORMAL PARAMETERS:
**
**      ctx - crop pipeline element context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _CropActivate(ctx)
CropPipeCtxPtr    ctx;
{
    PipeDataPtr	 src;
    PipeSinkPtr	 dstsnk;
    long int idx, status;
    unsigned long int roi_y2;
    unsigned long int crop_y1;  	/* First significant scan line */
    unsigned long int crop_y2;		/* Last significant scan line */

    while( (src = DdxRmGetData_( ctx, CrpSrcDrn_(ctx))) != NULL )
	{
	if( !IsPointer_(src) ) return( (int) src );
	roi_y2 = CrpRoiY_(ctx) + CrpRoiHeight_(ctx) - 1;
	/*
	**  We only need to process that data within the ROI.
	*/
	if ((CrpRoiY_(ctx) <= DatY2_(src)) && (roi_y2 >= DatY1_(src)))
	    {	/*
		**  Calculate the y limits imposed by the ROI on this quantum.
		*/
	    crop_y1 = CrpRoiY_(ctx) < DatY1_(src) ? DatY1_(src) : CrpRoiY_(ctx);
	    crop_y2 =        roi_y2 > DatY2_(src) ? DatY2_(src) : roi_y2;
    	    /*
	    **	Adjust the UDP to describe just the cropped portion.
	    */
	    DatPos_(src) = DatPos_(src) +
		( crop_y1 - DatY1_(src) ) * DatScnStr_(src) +
		( CrpRoiX_(ctx) -  DatX1_(src) ) * DatPxlStr_(src);
	    DatWidth_(src)  = CrpRoiWidth_(ctx);
	    DatHeight_(src) = crop_y2 - crop_y1 + 1;
	    /*
	    **	Adjust the UDP so that the pixel/scanline indices are
	    **  relative to the output image description.
	    */
	    dstsnk = CrpDstSnk_(ctx);
	    idx = DatCmpIdx_(src);
	    
	    DatX1_(src) = SnkX1_(dstsnk,idx);
	    DatX2_(src) = SnkX1_(dstsnk,idx) + CrpRoiWidth_(ctx) - 1 ;

	    DatY1_(src) = SnkY1_(dstsnk,idx) + (crop_y1 - CrpRoiY_(ctx));
	    DatY2_(src) = DatY1_(src) + DatHeight_(src) - 1;

	    status = DdxRmPutData_( CrpDstSnk_(ctx), src );
	    if( status != Success ) return( status );
	    }    
	else	
	    DdxRmDeallocData_( src );
	}
    return( Success );
}

/*****************************************************************************
**  _CropDestroy
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine deallocates all resources allocated by the CROP
**	pipeline element.
**
**  FORMAL PARAMETERS:
**
**      ctx - crop pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _CropDestroy(ctx)
CropPipeCtxPtr    ctx;
{
    CrpSrcDrn_(ctx) = DdxRmDestroyDrain_( CrpSrcDrn_(ctx) );

    return( Success );
}
