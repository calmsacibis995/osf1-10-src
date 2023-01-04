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
**      Robert NC Shelley
**
**  CREATION DATE:
**
**      October 26, 1989
**
**  MODIFICATION HISTORY:
**
*******************************************************************************/

/*
**  Include files 
*/
#include <stdio.h>

#include "SmiRotate.h"

/*
**  Table of contents
*/
int	SmiRotate();
int     SmiCreateRotate();

static int  _RotateInitialize();
static int  _RotateActivate();
static int  _RotateAbort();
static int  _RotateDestroy();

static void _ProcessRotate();
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
** Local Storage
*/

/* Rotate Pipeline Element Vector */
static PipeElementVector RotatePipeElement = {
    NULL,				/* FLINK			    */
    NULL,				/* BLINK			    */
    sizeof(PipeElementVector),		/* Size				    */
    TypePipeElementVector,		/* Structure type		    */
    StypeRotateElement,			/* Structure subtype		    */
    sizeof(RotatePipeCtx),		/* Context block size		    */
    0,					/* No input flag		    */
    0,					/* Reserved flags		    */
    _RotateInitialize,			/* Initialize entry		    */
    _RotateActivate,			/* Activate entry		    */
    NULL,				/* Flush entry			    */
    _RotateDestroy,			/* Destroy entry		    */
    NULL				/* Abort entry			    */
    };

/*****************************************************************************
**  SmiCreateRotate
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function fills in a newly created ROTATE pipeline element.
**
**  FORMAL PARAMETERS:
**
**      ctx - rotate pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
int 	    SmiCreateRotate(pipe,src,dst,angle,fill)
Pipe		     pipe;
PipeSinkPtr	     src;
PipeSinkPtr	     dst;
double		     angle;
int		     fill[];
{
    int i;
    RotatePipeCtxPtr ctx = (RotatePipeCtxPtr)
			    DdxCreatePipeCtx_(pipe, &RotatePipeElement, FALSE);
    if( !IsPointer_(ctx) ) return( (int) ctx );

    /*
    **	Copy the parameters to the context block.
    */
    RotSrcSnk_(ctx) = src;
    RotDstSnk_(ctx) = dst;
    RotAngle_(ctx)  = angle;
    for( i = 0; i < XieK_MaxComponents && IsPointer_(SnkUdpPtr_(dst,i)); i++ )
	RotFill_(ctx,i) = fill[i] < SnkLvl_(dst,i)
			? fill[i] : SnkLvl_(dst,i) - 1;;
    /*
    **	Setup one drain from source sink, receiving all components
    */
    RotSrcDrn_(ctx) = DdxRmCreateDrain_( src, -1 );
    if( !IsPointer_(RotSrcDrn_(ctx)) ) return( (int) RotSrcDrn_(ctx) );
    /*
    **	Decide on data types we'll allow (we're not too fussy).
    */
    DdxRmSetDType_( RotSrcDrn_(ctx), UdpK_DTypeUndefined, DtM_Any );
    DdxRmSetDType_( RotDstSnk_(ctx), UdpK_DTypeUndefined, DtM_Any );
    /*
    **	Handle only full image.
    */
    DdxRmSetQuantum_( RotSrcDrn_(ctx), SnkHeight_(src,0) );
    DdxRmSetQuantum_( RotDstSnk_(ctx), SnkHeight_(dst,0) );

    return( Success );
}					/* end SmiCreateRotate */

/*****************************************************************************
**  _RotateInitialize
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function initializes the ROTATE pipeline element prior to
**	an activation.
**
**  FORMAL PARAMETERS:
**
**      ctx - rotate pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	_RotateInitialize(ctx)
RotatePipeCtxPtr	ctx;
{
    int dtype, status;

    /*
    **	Initialize the source drain.
    */
    CtxInp_(ctx) = RotSrcDrn_(ctx);
    status = DdxRmInitializePort_( CtxHead_(ctx), RotSrcDrn_(ctx) );
    if( status != Success ) return( status );
    /*
    **	Set our destination sink DType to match our drain and initialize it.
    */
    dtype = DdxRmGetDType_( RotSrcDrn_(ctx) );
    DdxRmSetDType_( RotDstSnk_(ctx), dtype, 1<<dtype );
    status = DdxRmInitializePort_( CtxHead_(ctx), RotDstSnk_(ctx) );

    return( status );
}

/*****************************************************************************
**  _RotateActivate
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine is not called until the pipeline has buffered the
**	entire image.  The rotate is then performed here.
**
**  FORMAL PARAMETERS:
**
**      ctx - rotate pipeline element context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _RotateActivate(ctx)
RotatePipeCtxPtr    ctx;
{
    PipeDataPtr	 src, dst;
    int status;

    for( status = Success; status == Success; src = DdxRmDeallocData_(src) )
	{
	src = DdxRmGetData_(ctx,RotSrcDrn_(ctx));
	if( !IsPointer_(src) ) return( (int) src );

	dst = DdxRmAllocData_( RotDstSnk_(ctx), DatCmpIdx_(src),
			       0, SnkQnt_(RotDstSnk_(ctx)) );
	status = (int) dst;
	if( !IsPointer_(dst) ) continue;

	_ProcessRotate( DatUdpPtr_(src),
			DatUdpPtr_(dst),
			RotAngle_(ctx),
			RotFill_(ctx,DatCmpIdx_(src)) );

	status = DdxRmPutData_( RotDstSnk_(ctx), dst );
	}
    return( status );
}

/*****************************************************************************
**  _RotateDestroy
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine deallocates all resources allocated by the ROTATE
**	pipeline element.
**
**  FORMAL PARAMETERS:
**
**      ctx - dither pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _RotateDestroy(ctx)
RotatePipeCtxPtr    ctx;
{
    RotSrcDrn_(ctx) = DdxRmDestroyDrain_(RotSrcDrn_(ctx));

    return( Success );
}

/******************************************************************************
**  SmiRotate
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine performs a non-pipelined rotate through any number
**	of degrees.  It handles the necessary data type setup and
**	invokes _ProcessRotate() to do the actual work.
**
**  FORMAL PARAMETERS:
**
**	in	- Pointer to source udp (initialized)
**	dst	- Pointer to destination udp (initialized, maybe without array)
**      angle   - Degrees of rotation (positive angle == clockwise rotation)
**      fill	- Fill value
**
******************************************************************************/
int  SmiRotate( in, dst, angle, fill )
UdpPtr		in;
UdpPtr		dst;
double		angle;
unsigned long	fill;
{
UdpPtr		 src;			/* src upd pointer		    */
int status = Success;
unsigned long  value = fill < upLvl_(dst) ? fill : upLvl_(dst) - 1;

/*
**  Convert src to match dst data type if necessary.
*/
src = _CvtSrc( in, dst );
if( !IsPointer_(src) ) return( (int) src );

/*
**  Allocate memory for destination buffer.
*/
if( dst->UdpA_Base == NULL )
    dst->UdpA_Base = DdxMallocBits_( dst->UdpL_ArSize );
if( dst->UdpA_Base == NULL )
    status = BadAlloc;
else
    /* Do the actual work */
    _ProcessRotate(src, dst, angle, value);

if( src != in )
    {					    /* free converted input data    */
    DdxFreeBits_( src->UdpA_Base );
    DdxFree_( src );
    }

return( status );
}				    /* end SmiRotate */

/******************************************************************************
**  ProcessRotate
**
**  FUNCTIONAL DESCRIPTION:
**
**	Rotate an image through any number of degrees using the
**	nearest neighbor algorithm to interpolate the value of pixels
**	located at non-integral coordinates in the source.
**
**  FORMAL PARAMETERS:
**
**	in	- Pointer to source udp (initialized)
**	dst	- Pointer to destination udp (initialized, maybe without array)
**      angle   - Degrees of rotation (positive angle == clockwise rotation)
**      fill	- Fill value
**
******************************************************************************/
static void  _ProcessRotate( src, dst, angle, fill )
UdpPtr		src;
UdpPtr		dst;
double		angle;
unsigned long	fill;
{
double		xsina,  xcosa;		/* x sine and cosine		    */
double	        ysina,  ycosa;		/* y sine and cosine		    */
double		sina,   cosa;		/* sine and cosine of angle	    */
double		xpivot, ypivot;		/* x and y dst pivot coordinates    */
double		xtrans, ytrans;		/* x and y src translation	    */
unsigned long   xclip,  yclip;		/* x and y src clip region limits   */
unsigned long	xsrc,   ysrc;		/* x,y nearest neighbor coordinate  */
unsigned long	pxlind, scnind;	    	/* pixel and scanline indices	    */
unsigned long	pxloff, scnoff;		/* pixel and scanline offsets	    */
unsigned long	smask,  dmask;		/* src and dst pixel value masks    */
unsigned long	pxlval;			/* temp for pixel value		    */

/*
**  Compute the sine and cosine of angle.
*/
sina = sin( angle * RADIANS_PER_DEGREE );
cosa = cos( angle * RADIANS_PER_DEGREE );

/*
**  Translate dst to align it with the center of src, offset by relative X1,Y1.
*/
xtrans = src->UdpL_PxlPerScn / 2.0 + dst->UdpL_X1 - src->UdpL_X1 - 0.5;
ytrans = src->UdpL_ScnCnt    / 2.0 + dst->UdpL_Y1 - src->UdpL_Y1 - 0.5;

/*
**  We will rotate around the center point of the translated dst.
*/
xpivot = dst->UdpL_PxlPerScn / 2.0;
ypivot = dst->UdpL_ScnCnt    / 2.0;

/*
**  Set src clip region limits (by normalizing to 0,0 we take advantage of
**  unsigned arithmetic to make negative values look greater than clip limits).
*/
xclip  = src->UdpL_X2 - src->UdpL_X1;
yclip  = src->UdpL_Y2 - src->UdpL_Y1;

/*
**  Set the initial src scanline sine and cosine (relative to the pivot point), 
**  and the dst scanline data offset .
*/
ysina  = -ypivot * sina;
ycosa  = -ypivot * cosa;
scnoff =  dst->UdpL_Pos;

/*
**  Rotate the image data (code duplication traded for execution efficiency).
*/
switch( DdxGetTyp_( dst ) )
    {

case XieK_UBAV  :
case XieK_UBAVU :
    /*
    **	Rotate aligned or unaligned bit stream image data.
    */
    smask = (1<<src->UdpW_PixelLength)-1;   /* mask for isolating src pixel */
    dmask = (1<<dst->UdpW_PixelLength)-1;   /* mask for isolating dst pixel */

    for( scnind = 0;  scnind < dst->UdpL_ScnCnt;  scnind++ )
	{
	xsina  = -xpivot * sina;	    /* initial src pixel sin offset */
	xcosa  = -xpivot * cosa;	    /* initial src pixel cos offset */
	pxloff =  scnoff;		    /* initial dst pixel bit offset */

	for( pxlind = 0;  pxlind < dst->UdpL_PxlPerScn;  pxlind++ )
	    {
	    xsrc = xcosa - ysina + xtrans;  /* nearest neighbor X coordinate*/
	    ysrc = xsina + ycosa + ytrans;  /* nearest neighbor Y coordinate*/

	    if( xsrc > xclip  ||  ysrc > yclip )
		pxlval = fill;		    /* use fill value		    */
	    else			    /* use nearest src pixel value  */
		pxlval = GET_VALUE_( src->UdpA_Base, src->UdpL_Pos
				   + src->UdpL_ScnStride * ysrc
				   + src->UdpL_PxlStride * xsrc, smask );
	    PUT_VALUE_( dst->UdpA_Base, pxloff, pxlval, dmask );

	    xsina  += sina;		    /* new src pixel sin offset	    */
	    xcosa  += cosa;		    /* new src pixel cos offset	    */
	    pxloff += dst->UdpL_PxlStride;  /* new dst pixel bit offset	    */
	    }
	ysina  += sina;			    /* new src scanline sin offset  */
	ycosa  += cosa;			    /* new src scanline cos offset  */
	scnoff += dst->UdpL_ScnStride;	    /* new dst scanline bit offset  */
	}
    break;		/* end unaligned bit stream image   */

case XieK_ABU :
    /*
    **	Rotate aligned byte size image data.
    */
    for( scnind = 0;  scnind < dst->UdpL_ScnCnt;  scnind++ )
	{
	xsina  = -xpivot * sina;	    /* initial src pixel sin offset */
	xcosa  = -xpivot * cosa;	    /* initial src pixel cos offset */
	pxloff =  scnoff;		    /* initial dst pixel bit offset */

	for( pxlind = 0;  pxlind < dst->UdpL_PxlPerScn;  pxlind++ )
	    {
	    xsrc = xcosa - ysina + xtrans;  /* nearest neighbor X coordinate*/
	    ysrc = xsina + ycosa + ytrans;  /* nearest neighbor Y coordinate*/

	    if( xsrc > xclip  ||  ysrc > yclip )
		/*
		**  Outside src clip region -- so use fill value.
		*/
	       *((unsigned char *)(dst->UdpA_Base + (pxloff >> 3))) = fill;
	    else
		/*
		**  Stash the dst pixel with the nearest src pixel value.
		*/
	       *((unsigned char *)(dst->UdpA_Base + (pxloff >> 3))) =
	       *((unsigned char *)(src->UdpA_Base +
				  (src->UdpL_Pos  +
				   src->UdpL_ScnStride * ysrc +
				   src->UdpL_PxlStride * xsrc >> 3)));

	    xsina  += sina;		    /* new src pixel sin offset	    */
	    xcosa  += cosa;		    /* new src pixel cos offset	    */
	    pxloff += dst->UdpL_PxlStride;  /* new dst pixel bit offset	    */
	    }
	ysina  += sina;			    /* new src scanline sin offset  */
	ycosa  += cosa;			    /* new src scanline cos offset  */
	scnoff += dst->UdpL_ScnStride;	    /* new dst scanline bit offset  */
	}
    break;		/* end aligned byte image   */

case XieK_AWU :
    /*
    **	Rotate aligned word size image data.
    */
    for( scnind = 0;  scnind < dst->UdpL_ScnCnt;  scnind++ )
	{
	xsina  = -xpivot * sina;	    /* initial src pixel sin offset */
	xcosa  = -xpivot * cosa;	    /* initial src pixel cos offset */
	pxloff =  scnoff;		    /* initial dst pixel bit offset */

	for( pxlind = 0;  pxlind < dst->UdpL_PxlPerScn;  pxlind++ )
	    {
	    xsrc = xcosa - ysina + xtrans;  /* nearest neighbor X coordinate*/
	    ysrc = xsina + ycosa + ytrans;  /* nearest neighbor Y coordinate*/

	    if( xsrc > xclip  ||  ysrc > yclip )
		/*
		**  Outside src clip region -- so use fill value.
		*/
	       *((unsigned short *)(dst->UdpA_Base + (pxloff >> 3))) = fill;
	    else
		/*
		**  Stash the dst pixel with the nearest src pixel value.
		*/
	       *((unsigned short *)(dst->UdpA_Base + (pxloff >> 3))) =
	       *((unsigned short *)(src->UdpA_Base +
				  (src->UdpL_Pos  +
				   src->UdpL_ScnStride * ysrc +
				   src->UdpL_PxlStride * xsrc >> 3)));

	    xsina  += sina;		    /* new src pixel sin offset	    */
	    xcosa  += cosa;		    /* new src pixel cos offset	    */
	    pxloff += dst->UdpL_PxlStride;  /* new dst pixel bit offset	    */
	    }
	ysina  += sina;			    /* new src scanline sin offset  */
	ycosa  += cosa;			    /* new src scanline cos offset  */
	scnoff += dst->UdpL_ScnStride;	    /* new dst scanline bit offset  */
	}
    break;		/* end aligned byte image   */

case XieK_AF :
    /*
    **	Rotate aligned float image data.
    */
    smask = (1<<src->UdpW_PixelLength)-1;   /* mask for isolating src pixel */
    dmask = (1<<dst->UdpW_PixelLength)-1;   /* mask for isolating dst pixel */

    for( scnind = 0;  scnind < dst->UdpL_ScnCnt;  scnind++ )
	{
	xsina  = -xpivot * sina;	    /* initial src pixel sin offset */
	xcosa  = -xpivot * cosa;	    /* initial src pixel cos offset */
	pxloff =  scnoff;		    /* initial dst pixel bit offset */

	for( pxlind = 0;  pxlind < dst->UdpL_PxlPerScn;  pxlind++ )
	    {
	    xsrc = xcosa - ysina + xtrans;  /* nearest neighbor X coordinate*/
	    ysrc = xsina + ycosa + ytrans;  /* nearest neighbor Y coordinate*/

	    if( xsrc > xclip  ||  ysrc > yclip )
		/*
		**  Outside src clip region -- so use fill value (fill is really
		**  a float value, but we lie about it to avoid conversion).
		*/
	       *((unsigned long *)(dst->UdpA_Base + (pxloff >> 3))) = fill;
	    else
		/*
		**  Stash the dst pixel with the nearest src pixel value.
		*/
	       *((float *)(dst->UdpA_Base + (pxloff >> 3))) =
	       *((float *)(src->UdpA_Base + (src->UdpL_Pos  +
					     src->UdpL_ScnStride * ysrc +
					     src->UdpL_PxlStride * xsrc >> 3)));


	    xsina  += sina;		    /* new src pixel sin offset	    */
	    xcosa  += cosa;		    /* new src pixel cos offset	    */
	    pxloff += dst->UdpL_PxlStride;  /* new dst pixel bit offset	    */
	    }
	ysina  += sina;			    /* new src scanline sin offset  */
	ycosa  += cosa;			    /* new src scanline cos offset  */
	scnoff += dst->UdpL_ScnStride;	    /* new dst scanline bit offset  */
	}
    break;		/* end aligned float image  */
    }

}				    /* end _ProcessRotate */

/******************************************************************************
**  _CvtSrc
**
**  FUNCTIONAL DESCRIPTION:
**
**	Convert src to match dst if data types don't match.
**
**  FORMAL PARAMETERS:
**
**	in	- Pointer to src udp
**	dst	- Pointer to dst udp
**
******************************************************************************/
static UdpPtr _CvtSrc( in, dst )
UdpPtr	 in, dst;
{
    int status;
    UdpPtr src;

    if( in->UdpB_DType == dst->UdpB_DType )
	return( in );

    src = (UdpPtr) DdxMalloc_(sizeof(UdpRec));
    if( src == NULL ) return( (UdpPtr) BadAlloc );

   *src = *in;
    src->UdpB_DType	  = dst->UdpB_DType;
    src->UdpB_Class	  = dst->UdpB_Class;
    src->UdpW_PixelLength = dst->UdpW_PixelLength;
    src->UdpL_PxlStride   = dst->UdpL_PxlStride;
    src->UdpL_ScnStride   = src->UdpL_PxlStride * in->UdpL_PxlPerScn;
    src->UdpL_ArSize      = src->UdpL_ScnStride * in->UdpL_ScnCnt;
    src->UdpA_Base	  = 0;

    status = DdxConvert_( in, src, XieK_MoveMode );
    if( status != Success )
	{
	DdxFree_(src);
	src = (UdpPtr) status;
	}
    return( src );
}				    /* end _CvtSrc */
/* end module SmiRotate.c */
