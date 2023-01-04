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

/*******************************************************************************
**
**  FACILITY:
**
**      X Image Extension
**	Sample Machine Independant DDX
**
**  ABSTRACT:
**
**      This module contains sample code for image data rotation.
**
**  ENVIRONMENT:
**
**	VAX/VMS V5.3
**	ULTRIX  V3.1
**
**  AUTHOR(S):
**
**      Gary L. Grebus
**
**  CREATION DATE:
**
**      Wed May  9 16:33:00 1990
**
**  MODIFICATION HISTORY:
**
*******************************************************************************/

/*
**  Include files 
*/
#include <stdio.h>

#include "SmiConstrain.h"

/*
**  Table of contents
*/
int	SmiConstrain();
int     SmiCreateConstrain();

static int  _ConstrainInitialize();
static int  _ConstrainActivate();
static int  _ConstrainDestroy();

static void _ProcessConstrain();
static UdpPtr _ChkUdp();
static UdpPtr _CvtSrc();

/*
**  Equated Symbols
*/
#define FALSE 0
#define TRUE  1

/*
**  MACRO definitions
*/

/*
**  External References
*/

/*
 * Local Storage
 */

/*
 ** Constrain Pipeline Element Vector
 */
static PipeElementVector ConstrainPipeElement = {
    NULL,				/* FLINK			    */
    NULL,				/* BLINK			    */
    sizeof(PipeElementVector),		/* Size				    */
    TypePipeElementVector,		/* Structure type		    */
    StypeConstrainElement,		/* Structure subtype		    */
    sizeof(ConstrainPipeCtx),		/* Context block size		    */
    0,					/* No input flag		    */
    0,					/* Reserved flags		    */
    _ConstrainInitialize,		/* Initialize entry		    */
    _ConstrainActivate,			/* Activate entry		    */
    NULL,				/* Flush entry			    */
    _ConstrainDestroy,			/* Destroy entry		    */
    NULL				/* Abort entry point		    */
    };

/*****************************************************************************
**  SmiCreateConstrain
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function fills in a newly created CONSTRAIN pipeline element.
**
**  FORMAL PARAMETERS:
**
**      ctx - constrain pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
int 	    SmiCreateConstrain(pipe,src,dst,model)
Pipe		     pipe;
PipeSinkPtr	     src;
PipeSinkPtr	     dst;
int		     model;
{
    int dtype;

    ConstrainPipeCtxPtr ctx = (ConstrainPipeCtxPtr)
		       DdxCreatePipeCtx_( pipe, &ConstrainPipeElement, FALSE );
    if( !IsPointer_(ctx) ) return( (int) ctx );
    
    /* Save the parameter values */
    ConSrcSnk_(ctx) = src;
    ConDstSnk_(ctx) = dst;
    ConModel_(ctx)  = model;

    /* Setup one drain from source sink, receiving all components */
    ConSrcDrn_(ctx) = DdxRmCreateDrain_(ConSrcSnk_(ctx),-1);
    if( !IsPointer_(ConSrcDrn_(ctx)) ) return( (int) ConSrcDrn_(ctx) );

    /* Ask for the data as floats.  Constrained input types will be
     * converted to float and then reconstrained with the current model.
     * The input quantum depends on model. */
    DdxRmSetDType_( ConSrcDrn_(ctx), UdpK_DTypeF, DtM_F );
    DdxRmSetQuantum_( ConSrcDrn_(ctx), model == XieK_HardClip ? 0
				     : SnkHeight_(ConSrcSnk_(ctx),0) );

    /* The output data type depends on the destination levels.
     * The output quantum depends on model. */
    dtype = (SnkLvl_(dst, 0) <= 2) ? UdpK_DTypeVU :
	    (SnkLvl_(dst, 0) <= 256) ? UdpK_DTypeBU :
		                       UdpK_DTypeWU;
    
    DdxRmSetDType_( ConDstSnk_(ctx), dtype, 1<<dtype );
    DdxRmSetQuantum_( ConDstSnk_(ctx), model == XieK_HardClip ? 0
				     : SnkHeight_(ConDstSnk_(ctx),0) );

    return( Success );
}					/* end SmiCreateConstrain */

/*****************************************************************************
**  _ConstrainInitialize
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function initializes the CONSTRAIN pipeline element prior to
**	an activation.
**
**  FORMAL PARAMETERS:
**
**      ctx - constrain pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	_ConstrainInitialize(ctx)
ConstrainPipeCtxPtr	ctx;
{
    int i, status;
    PipeDataPtr data;

    status = DdxRmInitializePort_( CtxHead_(ctx), ConSrcDrn_(ctx) );
    if( status == Success )
	status  = DdxRmInitializePort_( CtxHead_(ctx), ConDstSnk_(ctx) );
    if( status != Success ) return( status );

    /* Currently processing from first (and only) source drain */
    CtxInp_(ctx) = ConSrcDrn_(ctx);

    return( Success );
}

/*****************************************************************************
**  _ConstrainActivate
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine constrains data read from the pipeline.
**
**  FORMAL PARAMETERS:
**
**      ctx - constrain pipeline element context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _ConstrainActivate(ctx)
ConstrainPipeCtxPtr    ctx;
{
    PipeDataPtr	 src, dst = NULL;
    int status;

    for( status = Success; status == Success; src = DdxRmDeallocData_(src),
					      dst = DdxRmDeallocData_(dst) )
	{
	src = DdxRmGetData_( ctx, ConSrcDrn_(ctx) );
	if( !IsPointer_(src) ) return( (int) src );

	dst = DdxRmAllocData_( ConDstSnk_(ctx), DatCmpIdx_(src),
			       DatY1_(src), DatHeight_(src) );
	status  = (int) dst;
	if( !IsPointer_(dst) ) continue;

	status = DdxConvert_(DatUdpPtr_(src), DatUdpPtr_(dst), ConModel_(ctx));
	if( status != Success ) continue;

	status = DdxRmPutData_( ConDstSnk_(ctx), dst );
	dst    = NULL;
	}
    return( status );
}

/*****************************************************************************
**  _ConstrainDestroy
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine deallocates all resources allocated by the CONSTRAIN
**	pipeline element.
**
**  FORMAL PARAMETERS:
**
**      ctx - dither pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _ConstrainDestroy(ctx)
ConstrainPipeCtxPtr    ctx;
{
    ConSrcDrn_(ctx) = DdxRmDestroyDrain_(ConSrcDrn_(ctx));
    return( Success );
}

/******************************************************************************
**  SmiConstrain
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine performs a non-pipelined constrain.  If the data
**      is currently constrained, it is first unconstrained, then
**      reconstrained using the current model.  All of the work is
**      actually done by DdxConvert_.
**
**  FORMAL PARAMETERS:
**
**	in	- Pointer to source udp (initialized)
**	dst	- Pointer to destination udp (initialized, maybe without array)
**      model   - Constraint model to apply:
**                  XieK_HardClip
**                  XieK_ClipScale
**                  XieK_HardScale
**
******************************************************************************/
int  SmiConstrain( in, dst, model)
UdpPtr		in;
UdpPtr		dst;
int             model;
{
int	status;
UdpPtr	src;				/* src upd pointer		    */


/*
**  Allocate the destination image array
*/
if( dst->UdpA_Base == NULL )
{
    dst->UdpA_Base = DdxMallocBits_( dst->UdpL_ArSize );
    if( dst->UdpA_Base == NULL ) return( BadAlloc );
}

/*
**  Convert src to unconstrained (float) data type if necessary.
*/
src = _ChkUdp( in );
if( !IsPointer_(src) ) return( (int) src );

/* Do the actual work */
status = DdxConvert_(src, dst, model);

if( src != in )
    {					    /* free converted input data    */
    DdxFreeBits_( src->UdpA_Base );
    DdxFree_( src );
    }

return( status );
}				    /* end SmiConstrain */

/******************************************************************************
**  _ChkUdp
**
**  FUNCTIONAL DESCRIPTION:
**
**	Check src udp and convert to unconstrained type if necessary.
**
**  FORMAL PARAMETERS:
**
**	src	- Pointer to src udp
**
******************************************************************************/
static UdpPtr _ChkUdp( src )
    UdpPtr	 src;
{
    int	status, stype;
    UdpPtr	dst = src;

    stype = DdxGetTyp_( src );

    if( stype != XieK_AF )
	{
	dst = (UdpPtr) DdxMalloc_(sizeof(UdpRec));
	if( dst == NULL ) return( (UdpPtr) BadAlloc );
	*dst = *src;
	dst->UdpB_DType	      = UdpK_DTypeF;
	dst->UdpB_Class	      = UdpK_ClassA;
	dst->UdpW_PixelLength = sizeof(float) * 8;
	dst->UdpL_PxlStride   = dst->UdpW_PixelLength;
	dst->UdpL_ScnStride   = dst->UdpL_PxlStride*src->UdpL_PxlPerScn;
	dst->UdpL_ArSize      = dst->UdpL_ScnStride*src->UdpL_ScnCnt;
	dst->UdpA_Base	      = 0;
	status = DdxConvert_( src, dst, XieK_MoveMode );
	if( status != Success )
	    {
	    DdxFreeBits_(dst->UdpA_Base);
	    DdxFree_(dst);
	    return( (UdpPtr) status );
	    }
	}
    return( dst );
}				    /* end _ChkUdp */
/* end module SmiConstrain.c */
