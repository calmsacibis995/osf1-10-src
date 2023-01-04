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

/*****************************************************************************
**
**  FACILITY:
**
**	X Imaging Extension
**      Sample Machine Independant DDX
**
**  ABSTRACT:
**
**      This module contains the support for the scale pipeline element.
**
**  ENVIRONMENT:
**
**      VAX/VMS V5.0
**
**  AUTHOR(S):
**
**      John Weber
**	Rich Hennessy ( 16 bit support )
**
**  CREATION DATE:
**
**      March 25, 1989
**
**  MODIFICATION HISTORY:
**
*****************************************************************************/

/*
**  Include files
*/
#include <stdio.h>

#include "SmiScale.h"

/*
**  Table of contents
*/
int	     SmiScale();
int          SmiCreateScale();

static int  _ScaleInitialize();
static int  _ScaleActivate();
static int  _ScaleDestroy();

static ScaleHorzPtr _ScaleHorizontal();
static ScaleVertPtr _ScaleVertical();

static int  _ScaleVerticalUp();
static int  _ScaleVerticalDown();

static int  _ScaleHorizontalNone();

static int  _ScaleHorizontalDownBits();
static int  _ScaleHorizontalDownBytes();
static int  _ScaleHorizontalDownFloats();
static int  _ScaleHorizontalDownWords();

static int  _ScaleHorizontalUpBits();
static int  _ScaleHorizontalUpBytes();
static int  _ScaleHorizontalUpFloats();
static int  _ScaleHorizontalUpWords();

static UdpPtr _ChkUdp();


/*
**  MACRO definitions
*/

/*
**  Equated Symbols
*/

/*
**  External References
*/

/*
**	Local Storage
*/
    /*
    **  Scale Element Vector
    */
static PipeElementVector ScalePipeElement = {
    NULL,				/* FLINK			    */
    NULL,				/* BLINK			    */
    sizeof(PipeElementVector),		/* Size				    */
    TypePipeElementVector,		/* Structure type		    */
    StypeScaleElement,			/* Structure subtype		    */
    sizeof(ScalePipeCtx),		/* Context block size		    */
    0,					/* No input flag		    */
    0,					/* Reserved flags		    */
    _ScaleInitialize,			/* Initialize entry		    */
    _ScaleActivate,			/* Activate entry		    */
    NULL,				/* Flush entry			    */
    _ScaleDestroy,			/* Destroy entry		    */
    NULL				/* Abort entry			    */
    };

/*****************************************************************************
**  SmiCreateScale
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function initializes the SCALE pipeline element
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
*****************************************************************************/
int SmiCreateScale( pipe, srcsnk, dstsnk )
 Pipe		    pipe;
 PipeSinkPtr	    srcsnk;
 PipeSinkPtr	    dstsnk;
{
    ScalePipeCtxPtr ctx = (ScalePipeCtxPtr)
			   DdxCreatePipeCtx_( pipe, &ScalePipeElement, FALSE );
    if( !IsPointer_(ctx) ) return( (int) ctx );

    /*
    **	Copy parameters into context record
    */
    SclSrcSnk_(ctx) = srcsnk;
    SclDstSnk_(ctx) = dstsnk;
    /*
    **	Create a drain on the source sink to read all components.
    */
    SclSrcDrn_(ctx) = DdxRmCreateDrain_( srcsnk, -1 );
    if( !IsPointer_(SclSrcDrn_(ctx)) ) return( (int) SclSrcDrn_(ctx) );
    /*
    **	Calculate scale factors
    */
    SclXScl_(ctx) = (double) SnkWidth_(dstsnk,0)
		  / (double) SnkWidth_(srcsnk,0);
    SclYScl_(ctx) = (double) SnkHeight_(dstsnk,0)
		  / (double) SnkHeight_(srcsnk,0);
    /*
    **	Set input and output quantums based on scale factors
    */
    DdxRmSetQuantum_( SclSrcDrn_(ctx),
		      (int)(SclYScl_(ctx) < 1.0 ? 1.0 / SclYScl_(ctx) : 1.0) );
    DdxRmSetQuantum_( dstsnk, 1 );
    /*
    **	Set input and output data types.
    **  Assume byte for output, but we might choose float later.
    */	
    DdxRmSetDType_( SclSrcDrn_(ctx), UdpK_DTypeUndefined, DtM_Any );
    DdxRmSetDType_( SclDstSnk_(ctx), UdpK_DTypeUndefined, DtM_Any );

    return( Success );
}					/* end SmiCreateScale */

/*****************************************************************************
**  _ScaleInitialize
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function initializes the SCALE pipeline element
**
**  FORMAL PARAMETERS:
**
**      ctx - scale pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	_ScaleInitialize(ctx)
ScalePipeCtxPtr	ctx;
{
    int dtype, status;

    /*
    **	Initialize input drain.
    */
    status = DdxRmInitializePort_( CtxHead_(ctx), SclSrcDrn_(ctx) );
    if( status != Success ) return( status );
    CtxInp_(ctx) = SclSrcDrn_(ctx);

    /*
    **  Initialize output sink.
    */
    dtype = DdxRmGetDType_( SclSrcDrn_(ctx) );
    DdxRmSetDType_( SclDstSnk_(ctx), dtype, 1<<dtype );
    status = DdxRmInitializePort_( CtxHead_(ctx), SclDstSnk_(ctx) );
    if( status != Success ) return( status );

    /*
    **	Determine horizontal scaling function.
    */
    SclHrzRou_(ctx) = _ScaleHorizontal( SclXScl_(ctx), dtype );
    if( !IsPointer_(SclHrzRou_(ctx)) ) return( (int) SclHrzRou_(ctx) );

    return( Success );
}

/*****************************************************************************
**  _ScaleActivate
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine controls data scaling
**
**  FORMAL PARAMETERS:
**
**      ctx - scale pipeline element context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	_ScaleActivate(ctx)
ScalePipeCtxPtr	ctx;
{
    PipeDataPtr copy, src, dst = NULL;
    int status, sy, idx;

    for( status = Success; status == Success; src = DdxRmDeallocData_(src),
					      dst = DdxRmDeallocData_(dst) )
	{
	src = DdxRmGetData_( ctx, SclSrcDrn_(ctx) );
	if( !IsPointer_(src) ) return( (int) src );

	idx = DatCmpIdx_(src);
	sy  = (double) SclDstScn_(ctx,idx) / SclYScl_(ctx);

	if( SclDstScn_(ctx,idx) < SnkHeight_(SclDstSnk_(ctx),idx)
		      && sy >= DatY1_(src) && sy <= DatY2_(src) )
	    {
	    dst = DdxRmAllocData_(SclDstSnk_(ctx), idx, SclDstScn_(ctx,idx), 1);
	    status = (int) dst;
	    if( !IsPointer_(dst) ) continue;

	    status = (*SclHrzRou_(ctx))( DatUdpPtr_(src),
				         DatUdpPtr_(dst),
				         SclXScl_(ctx),
				         SclYScl_(ctx));
	    
	    for( sy = ++SclDstScn_(ctx,idx) / SclYScl_(ctx);
		 status == Success && sy >= DatY1_(src) && sy <= DatY2_(src)
		     && SclDstScn_(ctx,idx) < SnkHeight_(SclDstSnk_(ctx),idx);
		 sy = ++SclDstScn_(ctx,idx) / SclYScl_(ctx) )
		{   /*
		    **  Hand out reference copies to do vertical scale-up.
		    */
		copy = DdxRmAllocRefDesc_( dst, DatY1_(dst), 1 );
		if( IsPointer_(copy) )
		    {
		    status = DdxRmPutData_( SclDstSnk_(ctx), dst );
		    dst    = copy;
		    DatY1_(dst)++;
		    DatY2_(dst)++;
		    }
		else
		    status = (int) copy;
		}
	    if( status != Success ) continue;

	    status = DdxRmPutData_( SclDstSnk_(ctx), dst );
	    dst    = NULL;
	    }
	}
    return( status );
}

/*****************************************************************************
**  _ScaleDestroy
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine deallocates all resources allocated by the scale
**	pipeline element.
**
**  FORMAL PARAMETERS:
**
**      ctx - scale pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	_ScaleDestroy(ctx)
ScalePipeCtxPtr	ctx;
{
    SclSrcDrn_(ctx) = DdxRmDestroyDrain_( SclSrcDrn_(ctx) );

    return( Success );
}

/*****************************************************************************
**  SmiScale
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
*****************************************************************************/
int	SmiScale(srcudp,dstudp)
UdpPtr	srcudp;
UdpPtr	dstudp;
{
    UdpPtr dst;
    double xscale;
    double yscale;
    int  (*horizontal_scale)();
    int    status = Success;

    if( dstudp->UdpA_Base == NULL )
	{   /*
	    **  Allocate the destination image array
	    */
        dstudp->UdpA_Base = DdxMallocBits_( dstudp->UdpL_ArSize );
        if( dstudp->UdpA_Base == NULL ) return( BadAlloc );
	}

    /*
    **	Calculate horizontal and vertical scale factors.
    */
    xscale = (double) dstudp->UdpL_PxlPerScn / (double) srcudp->UdpL_PxlPerScn;
    yscale = (double) dstudp->UdpL_ScnCnt    / (double) srcudp->UdpL_ScnCnt;
    /*
    **	Determine the proper horizontal scale routine to call based
    **	on source data type and scale factors.
    */
    horizontal_scale = _ScaleHorizontal( xscale, srcudp->UdpB_DType );
    if( !IsPointer_(horizontal_scale) ) return( (int) horizontal_scale );

    /*
    **	If destination is not a supported type, create an intermediate
    **  image.
    */
    dst = _ChkUdp(srcudp, dstudp);
    if( !IsPointer_(dst) ) return( (int) dst );

    /*
    **	Now, call the proper routine based on scale factors.
    */
    status = (*_ScaleVertical(yscale))
			     (srcudp, dst, xscale, yscale, horizontal_scale);
    /*
    **	If a temporary image was necessary, convert to the desired data type
    **	and delete the temporary.
    */
    if( dst != dstudp )
	{
	if( status == Success )
	    status  = DdxConvert_( dst, dstudp, XieK_MoveMode );
	if( IsPointer_(dst->UdpA_Base) )
	   DdxFreeBits_(dst->UdpA_Base);
	DdxFree_(dst);
	}
    return( status );
}


static ScaleHorzPtr _ScaleHorizontal(xscale, dtype)
double		xscale;
char		dtype;
{
    if( xscale < 1.0 )
	/*
	**  Scaling down
	*/
	switch (dtype)
	    {
	case UdpK_DTypeBU:
	    return( _ScaleHorizontalDownBytes );

	case UdpK_DTypeF:
	    return( _ScaleHorizontalDownFloats );

	case UdpK_DTypeVU:
	case UdpK_DTypeV:
	    return( _ScaleHorizontalDownBits );

	case UdpK_DTypeWU:
	    return( _ScaleHorizontalDownWords );

	case UdpK_DTypeUndefined:
	case UdpK_DTypeLU:
	default:
	    return( (ScaleHorzPtr) BadValue );
	    }
    else if( xscale > 1.0 )
	/*
	**  Scaling up
	*/
	switch (dtype)
	    {
	case UdpK_DTypeBU:
	    return( _ScaleHorizontalUpBytes );

	case UdpK_DTypeF:
	    return( _ScaleHorizontalUpFloats );

	case UdpK_DTypeVU:
	case UdpK_DTypeV:
	    return( _ScaleHorizontalUpBits );

	case UdpK_DTypeWU:
	    return( _ScaleHorizontalUpWords );

	case UdpK_DTypeLU:
	case UdpK_DTypeUndefined:
	default:
	    return( (ScaleHorzPtr) BadValue );
	    } 
    else
	/*
	**  No scale
	*/
	return( _ScaleHorizontalNone );
}

static ScaleVertPtr _ScaleVertical(yscale)
double		    yscale;
{
    if( yscale <= 1.0 )
	/*
	**  Scaling down or none, return pointer to scale down routine.
	*/
	return( _ScaleVerticalDown );

    else
	/*
	**  Scaling up, return pointer to scale up routine.
	*/
	return( _ScaleVerticalUp );
}

static int _ScaleVerticalUp( srcudp, dstudp, xscale, yscale, hrz_scale )
UdpPtr	srcudp;
UdpPtr	dstudp;
double	xscale;
double	yscale;
int   (*hrz_scale)();
{
    UdpRec   dscn;		    /* This is a UDP that will describe a   */
				    /* single destination scanline	    */
    int	     sx, sy, dx, dy, status = Success;

    dscn = *dstudp;
    /*
    **	Set up a temporary UDP to describe a single scanline
    */
    dscn.UdpL_ScnCnt = 1;
    dscn.UdpL_Y1 = dscn.UdpL_Y2 = dstudp->UdpL_Y1;

    while( dscn.UdpL_Y1 <= dstudp->UdpL_Y2 )
	{
	sy = (int)((double)dscn.UdpL_Y1 / yscale );

	status = (*hrz_scale)(srcudp,&dscn,xscale,yscale);
	if( status != Success )
	    break;

	dscn.UdpL_Y1 = ++dscn.UdpL_Y2;
	dscn.UdpL_Pos += dscn.UdpL_ScnStride;

	while( (int)((double)dscn.UdpL_Y1 /  yscale ) == sy
			  && dscn.UdpL_Y1 <= dstudp->UdpL_Y2 )
	    {
	    memcpy( dscn.UdpA_Base + dscn.UdpL_Pos / 8,
		    dscn.UdpA_Base+(dscn.UdpL_Pos-dscn.UdpL_ScnStride) / 8,
		    dscn.UdpL_ScnStride / 8 );
	    dscn.UdpL_Y1 = ++dscn.UdpL_Y2;
	    dscn.UdpL_Pos += dscn.UdpL_ScnStride;
	    }	    
	}
    return( status );
}

static int _ScaleVerticalDown( srcudp, dstudp, xscale, yscale, hrz_scale )
UdpPtr	srcudp;
UdpPtr	dstudp;
double	xscale;
double	yscale;
int   (*hrz_scale)();
{
    UdpRec   dscn;		    /* This is a UDP that will describe a   */
				    /* single destination scanline	    */
    int	     sx, sy, dx, dy, status = Success;

    dscn = *dstudp;
    /*
    **	Set up a temporary UDP to describe a single scanline
    */
    dscn.UdpL_ScnCnt = 1;
    dscn.UdpL_Y1 = dscn.UdpL_Y2 = dstudp->UdpL_Y1;

    while( dscn.UdpL_Y1 <= dstudp->UdpL_Y2 )
	{
	status = (*hrz_scale)(srcudp,&dscn,xscale,yscale);
	if( status != Success )
	    break;
	dscn.UdpL_Y1   = ++dscn.UdpL_Y2;
	dscn.UdpL_Pos += dscn.UdpL_ScnStride;
	}
    return( status );
}

static int _ScaleHorizontalNone( srcudp, dstudp, xscale, yscale )
UdpPtr	srcudp;
UdpPtr	dstudp;
double	xscale;
double	yscale;
{
    return( DdxCopy_( srcudp, dstudp,
		      dstudp->UdpL_X1, (int)((double)dstudp->UdpL_Y1 / yscale )
						   - srcudp->UdpL_Y1,
		      dstudp->UdpL_X1, dstudp->UdpL_Y1,
		      dstudp->UdpL_PxlPerScn, dstudp->UdpL_ScnCnt ) );
}

static int _ScaleHorizontalDownBits( srcudp, dstudp, xscale, yscale )
UdpPtr	    srcudp;
UdpPtr	    dstudp;
double	    xscale;
double	    yscale;
{
    int	     s_cmask, d_cmask, value;
    int	     s_scan_offset;
    int	     s_offset;

    int	     d_scan_offset;
    int	     d_offset;

    int	     sx, sy;
    int	     dx, dy;

    s_cmask = srcudp->UdpL_Levels - 1;
    d_cmask = dstudp->UdpL_Levels - 1;
    d_scan_offset = dstudp->UdpL_Pos;

    for( dy = dstudp->UdpL_Y1;  dy <= dstudp->UdpL_Y2;  dy++ )
	{
	sy = (int)((double)dy / yscale );

	s_scan_offset = srcudp->UdpL_Pos + 
			((sy - srcudp->UdpL_Y1) * srcudp->UdpL_ScnStride);
	d_offset = d_scan_offset;

	for( dx = dstudp->UdpL_X1;  dx <= dstudp->UdpL_X2;  dx++ )
	    {
	    sx = (int)((double)dx / xscale );

	    s_offset = s_scan_offset + (srcudp->UdpL_PxlStride * sx);

	    value = GET_VALUE_(srcudp->UdpA_Base,s_offset,s_cmask);
	    PUT_VALUE_(dstudp->UdpA_Base, d_offset, value, d_cmask);	    

	    d_offset += dstudp->UdpL_PxlStride;
	    }
	d_scan_offset += dstudp->UdpL_ScnStride;
	}
    return( Success );
}


static int _ScaleHorizontalDownBytes( srcudp, dstudp, xscale, yscale )
UdpPtr	srcudp;
UdpPtr	dstudp;
double	xscale;
double	yscale;
{
    int	     s_scan_offset;
    int	     s_offset;

    int	     d_scan_offset;
    int	     d_offset;

    int	     sx, sy;
    int	     dx, dy;

    d_scan_offset = dstudp->UdpL_Pos;

    for( dy = dstudp->UdpL_Y1;  dy <= dstudp->UdpL_Y2;  dy++ )
	{
	sy = (int)((double)dy / yscale );

	s_scan_offset = srcudp->UdpL_Pos + 
			((sy - srcudp->UdpL_Y1) * srcudp->UdpL_ScnStride);
	d_offset = d_scan_offset;

	for( dx = dstudp->UdpL_X1;  dx <= dstudp->UdpL_X2;  dx++ )
	    {
	    sx = (int)((double)dx / xscale );

	    s_offset = s_scan_offset + (srcudp->UdpL_PxlStride * sx);

	    *(unsigned char *)(dstudp->UdpA_Base + d_offset / 8) =
		*(unsigned char *)(srcudp->UdpA_Base + s_offset/8);

	    d_offset += dstudp->UdpL_PxlStride;
	    }
	d_scan_offset += dstudp->UdpL_ScnStride;
	}
    return( Success );
}


static int _ScaleHorizontalDownFloats( srcudp, dstudp, xscale, yscale )
UdpPtr	srcudp;
UdpPtr	dstudp;
double	xscale;
double	yscale;
{
    int	     s_scan_offset;
    int	     s_offset;

    int	     d_scan_offset;
    int	     d_offset;

    int	     sx, sy;
    int	     dx, dy;

    d_scan_offset = dstudp->UdpL_Pos;

    for( dy = dstudp->UdpL_Y1;  dy <= dstudp->UdpL_Y2;  dy++ )
	{
	sy = (int)((double)dy / yscale );

	s_scan_offset = srcudp->UdpL_Pos + 
			((sy - srcudp->UdpL_Y1) * srcudp->UdpL_ScnStride);
	d_offset = d_scan_offset;

	for( dx = dstudp->UdpL_X1;  dx <= dstudp->UdpL_X2;  dx++ )
	    {
	    sx = (int)((double)dx / xscale );

	    s_offset = s_scan_offset + (srcudp->UdpL_PxlStride * sx);

	    *(float *)(dstudp->UdpA_Base + d_offset / 8) =
		*(float *)(srcudp->UdpA_Base + s_offset/8);

	    d_offset += dstudp->UdpL_PxlStride;
	    }
	d_scan_offset += dstudp->UdpL_ScnStride;
	}
    return( Success );
}


static int _ScaleHorizontalDownWords( srcudp, dstudp, xscale, yscale )
UdpPtr	srcudp;
UdpPtr	dstudp;
double	xscale;
double	yscale;
{
    int	     s_scan_offset;
    int	     s_offset;

    int	     d_scan_offset;
    int	     d_offset;

    int	     sx, sy;
    int	     dx, dy;

    d_scan_offset = dstudp->UdpL_Pos;

    for( dy = dstudp->UdpL_Y1;  dy <= dstudp->UdpL_Y2;  dy++ )
	{
	sy = (int)((double)dy / yscale );

	s_scan_offset = srcudp->UdpL_Pos + 
			((sy - srcudp->UdpL_Y1) * srcudp->UdpL_ScnStride);
	d_offset = d_scan_offset;

	for( dx = dstudp->UdpL_X1;  dx <= dstudp->UdpL_X2;  dx++ )
	    {
	    sx = (int)((double)dx / xscale );

	    s_offset = s_scan_offset + (srcudp->UdpL_PxlStride * sx);

	    *(unsigned short *)(dstudp->UdpA_Base + d_offset / 8) =
		*(unsigned short *)(srcudp->UdpA_Base + s_offset/8);

	    d_offset += dstudp->UdpL_PxlStride;
	    }
	d_scan_offset += dstudp->UdpL_ScnStride;
	}
    return( Success );
}


static int _ScaleHorizontalUpBits( srcudp, dstudp, xscale, yscale )
UdpPtr	srcudp;
UdpPtr	dstudp;
double	xscale;
double	yscale;
{
    int	     s_cmask, d_cmask, value;
    int	     s_scan_offset;
    int	     s_offset;

    int	     d_scan_offset;
    int	     d_offset;

    int	     sx, sy;
    int	     dx, dy;

    s_cmask = srcudp->UdpL_Levels - 1;
    d_cmask = dstudp->UdpL_Levels - 1;
    d_scan_offset = dstudp->UdpL_Pos;

    for( dy = dstudp->UdpL_Y1; dy <= dstudp->UdpL_Y2; dy++ )
	{
	sy = (int)((double)dy / yscale );

	s_scan_offset = srcudp->UdpL_Pos + 
			((sy - srcudp->UdpL_Y1) * srcudp->UdpL_ScnStride);
	d_offset = d_scan_offset;

	for( dx = dstudp->UdpL_X1;  dx <= dstudp->UdpL_X2; )
	    {
	    sx = (int)((double)dx / xscale );

	    s_offset = s_scan_offset + (srcudp->UdpL_PxlStride * sx);

	    value = GET_VALUE_(srcudp->UdpA_Base,s_offset,s_cmask);

	    do	{
		PUT_VALUE_(dstudp->UdpA_Base, d_offset, value, d_cmask);
		dx++;
		d_offset += dstudp->UdpL_PxlStride;
		}
	    while( sx == (int)((double)dx / xscale ) && dx <= dstudp->UdpL_X2 );
	    }
	d_scan_offset += dstudp->UdpL_ScnStride;
	}
    return( Success );
}


static int _ScaleHorizontalUpBytes( srcudp, dstudp, xscale, yscale )
UdpPtr	srcudp;
UdpPtr	dstudp;
double	xscale;
double	yscale;
{
    int	     s_scan_offset;
    int	     s_offset;

    int	     d_scan_offset;
    int	     d_offset;

    int	     sx, sy;
    int	     dx, dy;

    unsigned char value;

    d_scan_offset = dstudp->UdpL_Pos;

    for( dy = dstudp->UdpL_Y1; dy <= dstudp->UdpL_Y2; dy++ )
	{
	sy = (int)((double)dy / yscale );

	s_scan_offset = srcudp->UdpL_Pos + 
			((sy - srcudp->UdpL_Y1) * srcudp->UdpL_ScnStride);
	d_offset = d_scan_offset;

	for( dx = dstudp->UdpL_X1;  dx <= dstudp->UdpL_X2; )
	    {
	    sx = (int)((double)dx / xscale );

	    s_offset = s_scan_offset + (srcudp->UdpL_PxlStride * sx);

	    value = *(unsigned char *) (srcudp->UdpA_Base + s_offset/8);

	    do	{
		*(unsigned char *)(dstudp->UdpA_Base + d_offset / 8) = value;
		dx++;
		d_offset += dstudp->UdpL_PxlStride;
		}
	    while( sx == (int)((double)dx / xscale ) && dx <= dstudp->UdpL_X2 );
	    }
	d_scan_offset += dstudp->UdpL_ScnStride;
	}
    return( Success );
}


static int _ScaleHorizontalUpFloats( srcudp, dstudp, xscale, yscale )
UdpPtr	srcudp;
UdpPtr	dstudp;
double	xscale;
double	yscale;
{
    int	     s_scan_offset;
    int	     s_offset;

    int	     d_scan_offset;
    int	     d_offset;

    int	     sx, sy;
    int	     dx, dy;

    float    value;

    d_scan_offset = dstudp->UdpL_Pos;

    for( dy = dstudp->UdpL_Y1; dy <= dstudp->UdpL_Y2; dy++ )
	{
	sy = (int)((double)dy / yscale );

	s_scan_offset = srcudp->UdpL_Pos + 
			((sy - srcudp->UdpL_Y1) * srcudp->UdpL_ScnStride);
	d_offset = d_scan_offset;

	for( dx = dstudp->UdpL_X1;  dx <= dstudp->UdpL_X2; )
	    {
	    sx = (int)((double)dx / xscale );

	    s_offset = s_scan_offset + (srcudp->UdpL_PxlStride * sx);

	    value = *(float *) (srcudp->UdpA_Base + s_offset/8);

	    do	{
		*(float *)(dstudp->UdpA_Base + d_offset / 8) = value;
		dx++;
		d_offset += dstudp->UdpL_PxlStride;
		}
	    while( sx == (int)((double)dx / xscale ) && dx <= dstudp->UdpL_X2 );
	    }
	d_scan_offset += dstudp->UdpL_ScnStride;
	}
    return( Success );
}


static int _ScaleHorizontalUpWords( srcudp, dstudp, xscale, yscale )
UdpPtr	srcudp;
UdpPtr	dstudp;
double	xscale;
double	yscale;
{
    int	     s_scan_offset;
    int	     s_offset;

    int	     d_scan_offset;
    int	     d_offset;

    int	     sx, sy;
    int	     dx, dy;

    unsigned short value;

    d_scan_offset = dstudp->UdpL_Pos;

    for( dy = dstudp->UdpL_Y1; dy <= dstudp->UdpL_Y2; dy++ )
	{
	sy = (int)((double)dy / yscale );

	s_scan_offset = srcudp->UdpL_Pos + 
			((sy - srcudp->UdpL_Y1) * srcudp->UdpL_ScnStride);
	d_offset = d_scan_offset;

	for( dx = dstudp->UdpL_X1;  dx <= dstudp->UdpL_X2; )
	    {
	    sx = (int)((double)dx / xscale );

	    s_offset = s_scan_offset + (srcudp->UdpL_PxlStride * sx);

	    value = *(unsigned short *) (srcudp->UdpA_Base + s_offset/8);

	    do	{
		*(unsigned short *)(dstudp->UdpA_Base + d_offset / 8) = value;
		dx++;
		d_offset += dstudp->UdpL_PxlStride;
		}
	    while( sx == (int)((double)dx / xscale ) && dx <= dstudp->UdpL_X2 );
	    }
	d_scan_offset += dstudp->UdpL_ScnStride;
	}
    return( Success );
}


/******************************************************************************
**  _ChkUdp
**
**  FUNCTIONAL DESCRIPTION:
**
**	Check dst udp and convert to one of our supported types if needed.
**
**  FORMAL PARAMETERS:
**
**      in      - Point to input udp, which drives the type selection.
**	out	- Pointer to out udp
**
******************************************************************************/
static UdpPtr _ChkUdp( in, out )
    UdpPtr       in;
    UdpPtr	 out;
{
    int	itype, otype;
    UdpPtr dst = out;

    itype = DdxGetTyp_( in );
    otype = DdxGetTyp_( out );

    if( itype != otype )
	{
	dst = (UdpPtr) DdxMalloc_(sizeof(UdpRec));
	if( dst == NULL ) return( (UdpPtr) BadAlloc );

	*dst = *out;

	switch( itype )
	{
	    case XieK_AF:
		dst->UdpB_DType	  = UdpK_DTypeF;
		dst->UdpW_PixelLength = sizeof(float) * 8;
		break;
	    case XieK_AWU:
		dst->UdpB_DType	  = UdpK_DTypeWU;
		dst->UdpW_PixelLength = sizeof(short) * 8;
		break;
	    case XieK_ABU:
		dst->UdpB_DType	  = UdpK_DTypeBU;
		dst->UdpW_PixelLength = sizeof(char) * 8;
		break;
	    default: 
		dst->UdpB_DType	  = UdpK_DTypeVU;
		dst->UdpW_PixelLength = 1;
		break;
	}

	dst->UdpB_Class	    = dst->UdpB_DType == UdpK_DTypeVU ? UdpK_ClassUBA
							      : UdpK_ClassA;
	dst->UdpL_PxlStride = dst->UdpW_PixelLength;
	dst->UdpL_ScnStride = dst->UdpL_PxlStride*out->UdpL_PxlPerScn;
	dst->UdpL_ArSize    = dst->UdpL_ScnStride*out->UdpL_ScnCnt;
	dst->UdpA_Base	    = DdxMallocBits_( dst->UdpL_ArSize );

	if( dst->UdpA_Base == NULL )
	    dst = (UdpPtr) DdxFree_( dst );
	}
    return( dst );
}				    /* end _ChkUdp */
/* end module SmiScale */
