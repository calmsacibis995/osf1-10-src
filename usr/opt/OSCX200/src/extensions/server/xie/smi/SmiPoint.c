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
**      This module implements the point operator which remaps pixel
**	values using a lookup table.  The result values can have more or
**      fewer levels than the input value.
**
**	The width of each data array (input, table, and output) must be
**      accounted for when performing the remapping.  This produces an n^3
**      number of cases which must be handled:
**
** 	o The width of the input data can be larger or smaller than the
**        width of the table.  The only requirement is that the length of
**        the table be at least 2^input_width.  The input data is accessed
**        using GET_VALUE_ to eliminate one level of code duplication, at the
**        cost of reduced performance.
**
**      o The width of the table can vary (in theory) from 1 to 32 bits. 
**        To improve performance, and reduce duplication, only aligned,
**        atomic, unpadded tables are handled. In the unlikely case the
**        supplied table doesn't meet these criteria, a suitable copy is
**        made.  A separate routine exists to handle each table data type
**        (byte, word, longword).
**
**      o The width of the output should be the same as the width of the
**        table, but data type, alignment, and padding are not guaranteed.
**        We'll also handle the cases where the output is narrower than the
**        table and assume that the caller knows what it is doing.
**        The routines for each table data type replicate blocks of
**        code to handle bit, byte, and word aligned output data.
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
**	Fri Apr  6 15:05:31 1990
**
*******************************************************************************/

#include <stdio.h>

#include "SmiPoint.h"

/*
**  Table of contents
*/
int 		SmiPoint();
int 	        SmiCreatePoint();

static int 	_PointInitialize();
static int 	_PointActivate();
static int 	_PointDestroy();

static void	_PointProcessByte();
static void	_PointProcessWord();
static void	_PointProcessLong();
static UdpPtr   _TempCvtLut();
static void     _TempDestroyUdp();

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
    **  Point Element Vector
    */
static PipeElementVector PointPipeElement = {
    NULL,				/* FLINK			    */
    NULL,				/* BLINK			    */
    sizeof(PipeElementVector),		/* Size				    */
    TypePipeElementVector,		/* Structure type		    */
    StypePointElement,			/* Structure subtype		    */
    sizeof(PointPipeCtx),		/* Context block size		    */
    0,					/* No input flag		    */
    0,					/* Reserved flags		    */
    _PointInitialize,			/* Initialize entry		    */
    _PointActivate,			/* Activate entry		    */
    NULL,				/* Flush entry			    */
    _PointDestroy,			/* Destroy entry		    */
    NULL				/* Abort entry point		    */
    };

/*****************************************************************************
**  SmiCreatePoint
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function fills in a newly created context for a POINT
**      pipeline element.
**
**  FORMAL PARAMETERS:
**
**      pipe    - Descriptor for pipe being built.
**      srcsnk	- Pointer to source sink.
**      dstsnk  - Pointer to destination sink.
**      idc     - Pointer to UDP describing the ROI or CPP (NULL if no IDC).
**      lut     - Array of pointesr to UDPs describing the point remap
**                lookup table.
**
**  FUNCTION VALUE:
**
*****************************************************************************/
int 	    SmiCreatePoint(pipe,srcsnk,dstsnk,idc,luts)
Pipe		     pipe;
PipeSinkPtr	     srcsnk;
PipeSinkPtr	     dstsnk;
UdpPtr		     idc;
UdpPtr	             luts[XieK_MaxComponents];

{
    int i;
    int dst_dtype;

    PointPipeCtxPtr ctx = (PointPipeCtxPtr)
			   DdxCreatePipeCtx_( pipe, &PointPipeElement, FALSE );
    if( !IsPointer_(ctx) ) return( (int) ctx );
    /*
    **	Copy parameters
    */
    PntSrcSnk_(ctx) = srcsnk;
    PntDstSnk_(ctx) = dstsnk;

    PntCpp_(ctx) = (idc != NULL && idc->UdpA_Base != NULL) ?
	             idc :		/* It's a CPP */
		     NULL;

    for( i = 0; i < XieK_MaxComponents; i++ )
	if( IsPointer_(SnkUdpPtr_(srcsnk,i)) )
	    PntCmpIn_(ctx)++;
	else
	    break;

    for (i = 0; i < XieK_MaxComponents; i++)
	{
	if (SnkUdpPtr_(dstsnk,i) == NULL)
	    break;
	/* Convert the input lookup table for efficient access */
	PntInTable_(ctx,i) = luts[i];
	PntTable_(ctx,i) = _TempCvtLut(luts[i]);
	if( !IsPointer_(PntTable_(ctx,i)) ) return( (int) PntTable_(ctx,i) );
	PntCmpOut_(ctx)++;
	}

    /* Calculate the region to be remapped */
    if (idc == NULL)
	{
	PntRoiX1_(ctx) = SnkX1_(srcsnk,0); PntRoiX2_(ctx) = SnkX2_(srcsnk,0);
	PntRoiY1_(ctx) = SnkY1_(srcsnk,0); PntRoiY2_(ctx) = SnkY2_(srcsnk,0);
	}
    else
	{
	PntRoiX1_(ctx) = idc->UdpL_X1 > SnkX1_(srcsnk,0) ? idc->UdpL_X1
	                                                : SnkX1_(srcsnk,0);
	PntRoiX2_(ctx) = idc->UdpL_X2 < SnkX2_(srcsnk,0) ? idc->UdpL_X2
	                                                : SnkX2_(srcsnk,0);
	PntRoiY1_(ctx) = idc->UdpL_Y1 > SnkY1_(srcsnk,0) ? idc->UdpL_Y1
	                                                : SnkY1_(srcsnk,0);
	PntRoiY2_(ctx) = idc->UdpL_Y2 < SnkY2_(srcsnk,0) ? idc->UdpL_Y2
	                                                : SnkY2_(srcsnk,0);
	}
    PntSrcDrn_(ctx) = DdxRmCreateDrain_( PntSrcSnk_(ctx), -1 );
    if( !IsPointer_(PntSrcDrn_(ctx)) ) return( (int) PntSrcDrn_(ctx) );

    /* Handle any constrained input data type */
    DdxRmSetDType_( PntSrcDrn_(ctx), UdpK_DTypeUndefined, DtM_Constrained );
    DdxRmSetQuantum_( PntSrcDrn_(ctx), 1 );

    /* Since this operator can explicitly cause a change in the number
     * of bits per pixel, determine the result datatype by looking at
     * the UDP(s) in the destination sink.
     */
     /* Handle one scanline at a time */
    dst_dtype = SnkDatTyp_(dstsnk,0);
    DdxRmSetDType_( PntDstSnk_(ctx), dst_dtype, DtM_Constrained );
    DdxRmSetQuantum_( PntDstSnk_(ctx), 1 );

    return( Success );
}					/* end SmiCreatePoint */

/*****************************************************************************
**  _PointInitialize
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function initializes the POINT pipeline element prior to
**	activation.
**
**  FORMAL PARAMETERS:
**
**      ctx - point pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	_PointInitialize(ctx)
PointPipeCtxPtr	ctx;
{
    int status  = DdxRmInitializePort_( CtxHead_(ctx), PntSrcDrn_(ctx) );
    if( status == Success )
	status  = DdxRmInitializePort_( CtxHead_(ctx), PntDstSnk_(ctx) );

    /* We are reading from the only drain */
    CtxInp_(ctx) = PntSrcDrn_(ctx);

    return( status );
}

/*****************************************************************************
**  _PointActivate
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine maps the selected pixels in the input image
**      using a transfer function supplied in the form of a lookup
**      table (LUT).
**
**  FORMAL PARAMETERS:
**
**      ctx - point pipeline element context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _PointActivate(ctx)
PointPipeCtxPtr    ctx;
{
    PipeDataPtr	 src, dst = NULL;
    int status, i, outidx, copies = PntCmpOut_(ctx) - PntCmpIn_(ctx) + 1;

    for( status = Success; status == Success; src = DdxRmDeallocData_(src),
					      dst = DdxRmDeallocData_(dst) )
	{
	src = DdxRmGetData_( ctx, PntSrcDrn_(ctx) );
	if( !IsPointer_(src) ) return( (int) src );

	/* If there are more output components than input components,
	 * we will duplicate the data. */
	for( outidx = DatCmpIdx_(src), i = 0; i < copies; outidx++, i++ )
	    {
	    dst = DdxRmAllocData_( PntDstSnk_(ctx), outidx, DatY1_(src), 1 );
	    status = (int) dst;
	    if( !IsPointer_(dst) ) break;

	    if( DatY1_(src) < PntRoiY1_(ctx) || DatY1_(src) > PntRoiY2_(ctx))
		status = DdxConvert_( DatUdpPtr_(src),
				      DatUdpPtr_(dst), XieK_MoveMode );
	    else
		{
		/* Copy the portions of this line outside the ROI */
		status = DdxCopy_( DatUdpPtr_(src),		/* Left  */
				   DatUdpPtr_(dst),
				   DatX1_(src), 0,
				   DatX1_(dst), DatY1_(dst),
				   PntRoiX1_(ctx) - DatX1_(src), 1 );
		if( status != Success ) break;
		
		status = DdxCopy_( DatUdpPtr_(src),		/* Right */
				   DatUdpPtr_(dst),
				   PntRoiX2_(ctx)+1, 0,
				   PntRoiX2_(ctx)+1, DatY1_(dst),
				   DatX2_(src) - PntRoiX2_(ctx), 1 );
		if( status != Success ) break;
		
		/* Call appropriate routine based on lookup table type */
		switch( PntTable_(ctx,outidx)->UdpB_DType )
		    {
		case UdpK_DTypeBU :
		    _PointProcessByte(DatUdpPtr_(src),
				    DatUdpPtr_(dst),
				    PntCpp_(ctx),
				    PntRoiX1_(ctx),
				    DatY1_(src),
				    PntRoiX2_(ctx),
				    DatY1_(src),
				    PntTable_(ctx,outidx));
		  break;
		  
		case UdpK_DTypeWU :
		    _PointProcessWord(DatUdpPtr_(src),
				      DatUdpPtr_(dst),
				      PntCpp_(ctx),
				      PntRoiX1_(ctx),
				      DatY1_(src),
				      PntRoiX2_(ctx),
				      DatY1_(src),
				      PntTable_(ctx,outidx));
		  break;
		  
		case UdpK_DTypeLU :
		     _PointProcessLong(DatUdpPtr_(src),
				      DatUdpPtr_(dst),
				      PntCpp_(ctx),
				      PntRoiX1_(ctx),
				      DatY1_(src),
				      PntRoiX2_(ctx),
				      DatY1_(src),
				      PntTable_(ctx,outidx));
		  break;
		    }
		}
	    if( status == Success )
		{
		status  = DdxRmPutData_( PntDstSnk_(ctx), dst );
		dst     = NULL;
		}
	    if( status != Success )
		break;
	    }
	}
    return( status );
}

/*****************************************************************************
**  _PointDestroy
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine deallocates all resources allocated by the POINT
**	pipeline element.
**
**  FORMAL PARAMETERS:
**
**      ctx - point pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _PointDestroy(ctx)
PointPipeCtxPtr    ctx;
{
    int i;

    PntSrcDrn_(ctx) = DdxRmDestroyDrain_( PntSrcDrn_(ctx) );

    /* Deallocate the converted lookup tables */
    for(i = 0; i < PntCmpOut_(ctx); i++)
	_TempDestroyUdp(PntTable_(ctx,i), PntInTable_(ctx,i));

    return( Success );
}

/*****************************************************************************
**  SmiPoint
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine performs a non-pipelined point operator, which
**      remaps pixel values using a lookup table.  Only one component
**      is remapped.
**
**  FORMAL PARAMETERS:
**
**	srcudp - Pointer to a UDP describing the source data.  
**      dstudp - Pointer to a UDP describing the destination buffer.
**      idc    - Pointer to a UDP describing the Region of Interest,
**                or Control Processing Plane.  (NULL if no IDC)
**      lut   -  Pointer to UDP containing one lookup table, organized as a
**               single scan line.
**
**  FUNCTION VALUE:
**
*****************************************************************************/
int 	SmiPoint( srcudp, dstudp, idc, lut )
UdpPtr		srcudp;
UdpPtr		dstudp;
UdpPtr	 	idc;
UdpPtr		lut;

{
    UdpPtr   table;
    UdpPtr   cpp;

    long int status, pixels;
    long int X1, X2, Y1, Y2;		/* Extent of src/dst overlap */
    long int x1, x2, y1, y2;		/* Extent of ROI */


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
	x1 = X1; x2 = X2;
	y1 = Y1; y2 = Y2;
	}
    else
	{
	x1 = idc->UdpL_X1 > X1 ? idc->UdpL_X1 : X1;
	x2 = idc->UdpL_X2 < X2 ? idc->UdpL_X2 : X2;
	y1 = idc->UdpL_Y1 > Y1 ? idc->UdpL_Y1 : Y1;
	y2 = idc->UdpL_Y2 < Y2 ? idc->UdpL_Y2 : Y2;
	}
    cpp =  (idc != NULL && idc->UdpA_Base != NULL) ? idc : NULL;
	
    /* Convert the lookup table to an efficient form if necessary */
    table = _TempCvtLut(lut);
    if( !IsPointer_(table) ) return( (int) table );	

    /* Just copy the areas which don't need to be modified */
    status = DdxCopy_( srcudp, dstudp, X1, Y1, X1, Y1, X2-X1+1, y1-Y1 );
    if( status == Success )    
	status = DdxCopy_( srcudp, dstudp, X1, y2+1, X1, y2+1, X2-X1+1, Y2-y2 );
    if( status == Success )    
	status = DdxCopy_( srcudp, dstudp, X1, y1, X1, y1, x1-X1, y2-y1+1 );
    if( status == Success )    
	status = DdxCopy_( srcudp, dstudp, x2+1, y1, x2+1, y1, X2-x2, y2-y1+1 );
	    
    if( status == Success )    
	switch (table->UdpB_DType)
	    {
	case UdpK_DTypeBU :
	    _PointProcessByte(srcudp, dstudp, cpp, x1, y1, x2, y2, table);
	    break;

	case UdpK_DTypeWU :
	    _PointProcessWord(srcudp, dstudp, cpp, x1, y1, x2, y2, table);
	    break;

	case UdpK_DTypeLU :
	    _PointProcessLong(srcudp, dstudp, cpp, x1, y1, x2, y2, table);
	    break;
	    }

    _TempDestroyUdp(table, lut);
    
    return( status );
}					/* end SmiPoint */

/*****************************************************************************
**  _PointProcessxxxx
**
**  FUNCTIONAL DESCRIPTION:
**
**	These routines perform the actual pixel remapping for the
**	selected region of  an image.
**      The src and dst data types must be the same.
**
**  FORMAL PARAMETERS:
**
**	src     - Pointer to a UDP describing the source data.
**      dst     - Pointer to a UDP describing the destination buffer.
**      cpp     - Pointer to a UDP describing the Control Processing Plane.
**                (NULL if no CPP)
**      x1      - The x coordinate of the upper left corner of the CPP, or
**                the ROI.
**      y1      - The y coordinate of the upper left corner of the CPP, or
**                the ROI.
**      x2      - The x coordinate of the lower right corner of the CPP, or
**                the ROI.
**      y2      - The y coordinate of the lower right corner of the CPP, or
**                the ROI.
**      lut     - Pointers to a UDP which contains the lookup table for
**                this source udp.
**
*****************************************************************************/
static void  _PointProcessByte(src, dst, cpp, x1, y1, x2, y2, lut)
UdpPtr	     	src;
UdpPtr	     	dst;
UdpPtr		cpp;
long int	x1;
long int	x2;
long int	y1;
long int	y2;
UdpPtr		lut;
{
    long int		pixel;
    long int		scanline;

    unsigned long	srcmask;
    unsigned long	dstmask;

    long int		srcpxlstart;
    long int		srcpxloff;
    long int		dstpxlstart;
    long int		dstpxloff;
    unsigned long	pxlvalue;

    long int		cpppxlstart;
    long int		cpppxloff;

    srcmask = (1<<src->UdpW_PixelLength)-1;
    dstmask = (1<<dst->UdpW_PixelLength)-1;

    srcpxlstart = src->UdpL_Pos + (y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dstpxlstart = dst->UdpL_Pos + (y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;


    /* Begin massive code duplication to minimize branches in the
     * inner loops.
     */
    if( cpp == NULL )
	{
	/* The mapping is not controlled by a CPP */
	switch( DdxGetTyp_( dst ) )
	    {
	case XieK_UBAV :
	case XieK_UBAVU:		/* This currently handles LU
					 * data due to hack in DdxGetTyp_ */
	    /* Destination is aligned or unaligned bit stream format */
	    for (scanline = y1; scanline <= y2; scanline++)
		{
		srcpxloff = srcpxlstart+(x1-src->UdpL_X1)*src->UdpL_PxlStride;
		dstpxloff = dstpxlstart+(x1-dst->UdpL_X1)*dst->UdpL_PxlStride;

		for( pixel = x1; pixel <= x2; pixel++ )
		    {
		    pxlvalue = GET_VALUE_(src->UdpA_Base, srcpxloff, srcmask);
		    pxlvalue = ((unsigned char *)lut->UdpA_Base)[pxlvalue];

		    PUT_VALUE_(dst->UdpA_Base, dstpxloff, pxlvalue, dstmask);

		    srcpxloff += src->UdpL_PxlStride;
		    dstpxloff += dst->UdpL_PxlStride;
		    }
		srcpxlstart += src->UdpL_ScnStride;
		dstpxlstart += dst->UdpL_ScnStride;
		}
	    break;

	case XieK_ABU :
	    /* Destination is byte aligned */
	    for( scanline = y1; scanline <= y2; scanline++ )
		{
		srcpxloff = srcpxlstart+(x1-src->UdpL_X1)*src->UdpL_PxlStride;
		dstpxloff = dstpxlstart+(x1-dst->UdpL_X1)*dst->UdpL_PxlStride;

		for (pixel = x1; pixel <= x2; pixel++)
		    {
		    pxlvalue = GET_VALUE_(src->UdpA_Base, srcpxloff, srcmask);
		    pxlvalue = ((unsigned char *)lut->UdpA_Base)[pxlvalue];

		    *((unsigned char *)
		      (dst->UdpA_Base + (dstpxloff >> 3))) = pxlvalue;

		    srcpxloff += src->UdpL_PxlStride;
		    dstpxloff += dst->UdpL_PxlStride;
		    }
		srcpxlstart += src->UdpL_ScnStride;
		dstpxlstart += dst->UdpL_ScnStride;
		}
	    break;

	case XieK_AWU :
	    /* Destination is word (16 bit shorts) aligned */
	    for( scanline = y1; scanline <= y2; scanline++ )
		{
		srcpxloff = srcpxlstart+(x1-src->UdpL_X1)*src->UdpL_PxlStride;
		dstpxloff = dstpxlstart+(x1-dst->UdpL_X1)*dst->UdpL_PxlStride;

		for (pixel = x1; pixel <= x2; pixel++)
		    {
		    pxlvalue = GET_VALUE_(src->UdpA_Base, srcpxloff, srcmask);
		    pxlvalue = ((unsigned char *)lut->UdpA_Base)[pxlvalue];

		    *((unsigned short *)
		      (dst->UdpA_Base + (dstpxloff >> 3))) = pxlvalue;

		    srcpxloff += src->UdpL_PxlStride;
		    dstpxloff += dst->UdpL_PxlStride;
		    }
		srcpxlstart += src->UdpL_ScnStride;
		dstpxlstart += dst->UdpL_ScnStride;
		}
	    break;

	    }				/* end switch */

	}

    else
	{
	/* The mapping is controlled by a Control Processing Plane */

	cpppxlstart = cpp->UdpL_Pos + cpp->UdpL_ScnStride * (y1 - cpp->UdpL_Y1);

	switch( DdxGetTyp_( dst ) )
	    {
	case XieK_UBAV :
	case XieK_UBAVU :
	    /* Destination is aligned or unaligned bit stream format */
	    for( scanline = y1; scanline <= y2; scanline++ )
		{
		srcpxloff = srcpxlstart+(x1-src->UdpL_X1)*src->UdpL_PxlStride;
		dstpxloff = dstpxlstart+(x1-dst->UdpL_X1)*dst->UdpL_PxlStride;
		cpppxloff = cpppxlstart;

		for( pixel = x1; pixel <= x2; pixel++ )
		    {
		    pxlvalue = GET_VALUE_(src->UdpA_Base, srcpxloff, srcmask);
		    if (GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1))
			pxlvalue = ((unsigned char *)lut->UdpA_Base)[pxlvalue];

		    PUT_VALUE_(dst->UdpA_Base, dstpxloff, pxlvalue, dstmask);

		    srcpxloff += src->UdpL_PxlStride;
		    dstpxloff += dst->UdpL_PxlStride;
		    cpppxloff += cpp->UdpL_PxlStride;
		    }
		srcpxlstart += src->UdpL_ScnStride;
		dstpxlstart += dst->UdpL_ScnStride;
		cpppxlstart += cpp->UdpL_ScnStride;
		}
	    break;

	case XieK_ABU :
	    /* Destination is byte aligned */
	    for( scanline = y1; scanline <= y2; scanline++ )
		{
		srcpxloff = srcpxlstart+(x1-src->UdpL_X1)*src->UdpL_PxlStride;
		dstpxloff = dstpxlstart+(x1-dst->UdpL_X1)*dst->UdpL_PxlStride;
		cpppxloff = cpppxlstart;

		for( pixel = x1; pixel <= x2; pixel++ )
		    {
		    pxlvalue = GET_VALUE_(src->UdpA_Base, srcpxloff, srcmask);
		    if (GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1) )
			pxlvalue = ((unsigned char *)lut->UdpA_Base)[pxlvalue];

		    cpppxloff += cpp->UdpL_PxlStride;

		    (*(unsigned char *)
		     (dst->UdpA_Base + (dstpxloff >> 3))) = pxlvalue;

		    srcpxloff+=src->UdpL_PxlStride;
		    dstpxloff+=dst->UdpL_PxlStride;
		    }
		srcpxlstart += src->UdpL_ScnStride;
		dstpxlstart += dst->UdpL_ScnStride;
		cpppxlstart += cpp->UdpL_ScnStride;
		}
	    break;

	case XieK_AWU:
	    /* Destination is word (16 bit short) aligned */
	    for( scanline = y1; scanline <= y2; scanline++ )
		{
		srcpxloff = srcpxlstart+(x1-src->UdpL_X1)*src->UdpL_PxlStride;
		dstpxloff = dstpxlstart+(x1-dst->UdpL_X1)*dst->UdpL_PxlStride;
		cpppxloff = cpppxlstart;

		for( pixel = x1; pixel <= x2; pixel++ )
		    {
		    pxlvalue = GET_VALUE_(src->UdpA_Base, srcpxloff, srcmask);
		    if (GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1) )
			pxlvalue = ((unsigned char *)lut->UdpA_Base)[pxlvalue];

		    cpppxloff += cpp->UdpL_PxlStride;

		    (*(unsigned short *)
		     (dst->UdpA_Base + (dstpxloff >> 3))) = pxlvalue;

		    srcpxloff+=src->UdpL_PxlStride;
		    dstpxloff+=dst->UdpL_PxlStride;
		    }
		srcpxlstart += src->UdpL_ScnStride;
		dstpxlstart += dst->UdpL_ScnStride;
		cpppxlstart += cpp->UdpL_ScnStride;
		}
	    break;
	    }				/* end switch */
	}
}					/* end _PointProcessByte */

static void  _PointProcessWord(src, dst, cpp, x1, y1, x2, y2, lut)
UdpPtr	     	src;
UdpPtr	     	dst;
UdpPtr		cpp;
long int	x1;
long int	x2;
long int	y1;
long int	y2;
UdpPtr		lut;
{
    long int		pixel;
    long int		scanline;

    unsigned long	srcmask;
    unsigned long	dstmask;

    long int		srcpxlstart;
    long int		srcpxloff;
    long int		dstpxlstart;
    long int		dstpxloff;
    unsigned long	pxlvalue;

    long int		cpppxlstart;
    long int		cpppxloff;

    srcmask = (1<<src->UdpW_PixelLength)-1;
    dstmask = (1<<dst->UdpW_PixelLength)-1;

    srcpxlstart = src->UdpL_Pos + (y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dstpxlstart = dst->UdpL_Pos + (y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;


    /* Begin massive code duplication to minimize branches in the
     * inner loops.
     */
    if( cpp == NULL )
	{
	/* The mapping is not controlled by a CPP */
	switch( DdxGetTyp_( dst ) )
	    {
	case XieK_UBAV :
	case XieK_UBAVU:		/* This currently handles  LU
					 * data due to hack in DdxGetTyp_ */
	    /* Destination is aligned or unaligned bit stream format */
	    for (scanline = y1; scanline <= y2; scanline++)
		{
		srcpxloff = srcpxlstart+(x1-src->UdpL_X1)*src->UdpL_PxlStride;
		dstpxloff = dstpxlstart+(x1-dst->UdpL_X1)*dst->UdpL_PxlStride;

		for( pixel = x1; pixel <= x2; pixel++ )
		    {
		    pxlvalue = GET_VALUE_(src->UdpA_Base, srcpxloff, srcmask);
		    pxlvalue = ((unsigned short *)lut->UdpA_Base)[pxlvalue];

		    PUT_VALUE_(dst->UdpA_Base, dstpxloff, pxlvalue, dstmask);

		    srcpxloff += src->UdpL_PxlStride;
		    dstpxloff += dst->UdpL_PxlStride;
		    }
		srcpxlstart += src->UdpL_ScnStride;
		dstpxlstart += dst->UdpL_ScnStride;
		}
	    break;

	case XieK_ABU :
	    /* Destination is byte aligned */
	    for( scanline = y1; scanline <= y2; scanline++ )
		{
		srcpxloff = srcpxlstart+(x1-src->UdpL_X1)*src->UdpL_PxlStride;
		dstpxloff = dstpxlstart+(x1-dst->UdpL_X1)*dst->UdpL_PxlStride;

		for (pixel = x1; pixel <= x2; pixel++)
		    {
		    pxlvalue = GET_VALUE_(src->UdpA_Base, srcpxloff, srcmask);
		    pxlvalue = ((unsigned short *)lut->UdpA_Base)[pxlvalue];

		    *((unsigned char *)
		      (dst->UdpA_Base + (dstpxloff >> 3))) = pxlvalue;

		    srcpxloff += src->UdpL_PxlStride;
		    dstpxloff += dst->UdpL_PxlStride;
		    }
		srcpxlstart += src->UdpL_ScnStride;
		dstpxlstart += dst->UdpL_ScnStride;
		}
	    break;

	case XieK_AWU :
	    /* Destination is word (16 bit short) aligned */
	    for( scanline = y1; scanline <= y2; scanline++ )
		{
		srcpxloff = srcpxlstart+(x1-src->UdpL_X1)*src->UdpL_PxlStride;
		dstpxloff = dstpxlstart+(x1-dst->UdpL_X1)*dst->UdpL_PxlStride;

		for (pixel = x1; pixel <= x2; pixel++)
		    {
		    pxlvalue = GET_VALUE_(src->UdpA_Base, srcpxloff, srcmask);
		    pxlvalue = ((unsigned short *)lut->UdpA_Base)[pxlvalue];

		    *((unsigned short *)
		      (dst->UdpA_Base + (dstpxloff >> 3))) = pxlvalue;

		    srcpxloff += src->UdpL_PxlStride;
		    dstpxloff += dst->UdpL_PxlStride;
		    }
		srcpxlstart += src->UdpL_ScnStride;
		dstpxlstart += dst->UdpL_ScnStride;
		}
	    break;

	    }				/* end switch */
	}

    else
	{
	/* The mapping is controlled by a Control Processing Plane */

	cpppxlstart = cpp->UdpL_Pos + cpp->UdpL_ScnStride * (y1 - cpp->UdpL_Y1);

	switch( DdxGetTyp_( dst ) )
	    {
	case XieK_UBAV :
	case XieK_UBAVU :
	    /* Destination is aligned or unaligned bit stream format */
	    for( scanline = y1; scanline <= y2; scanline++ )
		{
		srcpxloff = srcpxlstart+(x1-src->UdpL_X1)*src->UdpL_PxlStride;
		dstpxloff = dstpxlstart+(x1-dst->UdpL_X1)*dst->UdpL_PxlStride;
		cpppxloff = cpppxlstart;

		for( pixel = x1; pixel <= x2; pixel++ )
		    {
		    pxlvalue = GET_VALUE_(src->UdpA_Base, srcpxloff, srcmask);
		    if (GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1))
  		      pxlvalue = ((unsigned short *)lut->UdpA_Base)[pxlvalue];
		    PUT_VALUE_(dst->UdpA_Base, dstpxloff, pxlvalue, dstmask);

		    srcpxloff += src->UdpL_PxlStride;
		    dstpxloff += dst->UdpL_PxlStride;
		    cpppxloff += cpp->UdpL_PxlStride;
		    }
		srcpxlstart += src->UdpL_ScnStride;
		dstpxlstart += dst->UdpL_ScnStride;
		cpppxlstart += cpp->UdpL_ScnStride;
		}
	    break;

	case XieK_ABU :
	    /* Destination is byte aligned */
	    for( scanline = y1; scanline <= y2; scanline++ )
		{
		srcpxloff = srcpxlstart+(x1-src->UdpL_X1)*src->UdpL_PxlStride;
		dstpxloff = dstpxlstart+(x1-dst->UdpL_X1)*dst->UdpL_PxlStride;
		cpppxloff = cpppxlstart;

		for( pixel = x1; pixel <= x2; pixel++ )
		    {
		    pxlvalue = GET_VALUE_(src->UdpA_Base, srcpxloff, srcmask);

		    if (GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1) )
		       pxlvalue = ((unsigned short *)lut->UdpA_Base)[pxlvalue];

		    (*(unsigned char *)
		     (dst->UdpA_Base + (dstpxloff >> 3))) = pxlvalue;

		    srcpxloff+=src->UdpL_PxlStride;
		    dstpxloff+=dst->UdpL_PxlStride;
		    cpppxloff += cpp->UdpL_PxlStride;
		    }
		srcpxlstart += src->UdpL_ScnStride;
		dstpxlstart += dst->UdpL_ScnStride;
		cpppxlstart += cpp->UdpL_ScnStride;
		}
	    break;

	case XieK_AWU :
	    /* Destination is word (16 bit short) aligned */
	    for( scanline = y1; scanline <= y2; scanline++ )
		{
		srcpxloff = srcpxlstart+(x1-src->UdpL_X1)*src->UdpL_PxlStride;
		dstpxloff = dstpxlstart+(x1-dst->UdpL_X1)*dst->UdpL_PxlStride;
		cpppxloff = cpppxlstart;

		for( pixel = x1; pixel <= x2; pixel++ )
		    {
		    pxlvalue = GET_VALUE_(src->UdpA_Base, srcpxloff, srcmask);

		    if (GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1) )
		       pxlvalue = ((unsigned short *)lut->UdpA_Base)[pxlvalue];

		    (*(unsigned short *)
		     (dst->UdpA_Base + (dstpxloff >> 3))) = pxlvalue;

		    srcpxloff+=src->UdpL_PxlStride;
		    dstpxloff+=dst->UdpL_PxlStride;
		    cpppxloff += cpp->UdpL_PxlStride;
		    }
		srcpxlstart += src->UdpL_ScnStride;
		dstpxlstart += dst->UdpL_ScnStride;
		cpppxlstart += cpp->UdpL_ScnStride;
		}
	    break;

	    }				/* end switch */
	}
}					/* end _PointProcessWord */

static void  _PointProcessLong(src, dst, cpp, x1, y1, x2, y2, lut)
UdpPtr	     	src;
UdpPtr	     	dst;
UdpPtr		cpp;
long int	x1;
long int	x2;
long int	y1;
long int	y2;
UdpPtr		lut;
{
    long int		pixel;
    long int		scanline;

    unsigned long	srcmask;
    unsigned long	dstmask;

    long int		srcpxlstart;
    long int		srcpxloff;
    long int		dstpxlstart;
    long int		dstpxloff;
    unsigned long	pxlvalue;

    long int		cpppxlstart;
    long int		cpppxloff;

    srcmask = (1<<src->UdpW_PixelLength)-1;
    dstmask = (1<<dst->UdpW_PixelLength)-1;

    srcpxlstart = src->UdpL_Pos + (y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dstpxlstart = dst->UdpL_Pos + (y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;


    /* Begin massive code duplication to minimize branches in the
     * inner loops.
     */
    if( cpp == NULL )
	{
	/* The mapping is not controlled by a CPP */
	switch( DdxGetTyp_( dst ) )
	    {
	case XieK_UBAV :
	case XieK_UBAVU:		/* This currently handles  LU
					 * data due to hack in DdxGetTyp_ */
	    /* Destination is aligned or unaligned bit stream format */
	    for (scanline = y1; scanline <= y2; scanline++)
		{
		srcpxloff = srcpxlstart+(x1-src->UdpL_X1)*src->UdpL_PxlStride;
		dstpxloff = dstpxlstart+(x1-dst->UdpL_X1)*dst->UdpL_PxlStride;

		for( pixel = x1; pixel <= x2; pixel++ )
		    {
		    pxlvalue = GET_VALUE_(src->UdpA_Base, srcpxloff, srcmask);
		    pxlvalue = ((unsigned long *)lut->UdpA_Base)[pxlvalue];

		    PUT_VALUE_(dst->UdpA_Base, dstpxloff, pxlvalue, dstmask);

		    srcpxloff += src->UdpL_PxlStride;
		    dstpxloff += dst->UdpL_PxlStride;
		    }
		srcpxlstart += src->UdpL_ScnStride;
		dstpxlstart += dst->UdpL_ScnStride;
		}
	    break;

	case XieK_ABU :
	    /* Destination is byte aligned */
	    for( scanline = y1; scanline <= y2; scanline++ )
		{
		srcpxloff = srcpxlstart+(x1-src->UdpL_X1)*src->UdpL_PxlStride;
		dstpxloff = dstpxlstart+(x1-dst->UdpL_X1)*dst->UdpL_PxlStride;

		for (pixel = x1; pixel <= x2; pixel++)
		    {
		    pxlvalue = GET_VALUE_(src->UdpA_Base, srcpxloff, srcmask);
		    pxlvalue = ((unsigned long *)lut->UdpA_Base)[pxlvalue];

		    *((unsigned char *)
		      (dst->UdpA_Base + (dstpxloff >> 3))) = pxlvalue;

		    srcpxloff += src->UdpL_PxlStride;
		    dstpxloff += dst->UdpL_PxlStride;
		    }
		srcpxlstart += src->UdpL_ScnStride;
		dstpxlstart += dst->UdpL_ScnStride;
		}
	    break;

	case XieK_AWU :
	    /* Destination is word (16 bit short) aligned */
	    for( scanline = y1; scanline <= y2; scanline++ )
		{
		srcpxloff = srcpxlstart+(x1-src->UdpL_X1)*src->UdpL_PxlStride;
		dstpxloff = dstpxlstart+(x1-dst->UdpL_X1)*dst->UdpL_PxlStride;

		for (pixel = x1; pixel <= x2; pixel++)
		    {
		    pxlvalue = GET_VALUE_(src->UdpA_Base, srcpxloff, srcmask);
		    pxlvalue = ((unsigned long *)lut->UdpA_Base)[pxlvalue];

		    *((unsigned short *)
		      (dst->UdpA_Base + (dstpxloff >> 3))) = pxlvalue;

		    srcpxloff += src->UdpL_PxlStride;
		    dstpxloff += dst->UdpL_PxlStride;
		    }
		srcpxlstart += src->UdpL_ScnStride;
		dstpxlstart += dst->UdpL_ScnStride;
		}
	    break;
	    }				/* end switch */
	}

    else
	{
	/* The mapping is controlled by a Control Processing Plane */

	cpppxlstart = cpp->UdpL_Pos + cpp->UdpL_ScnStride * (y1 - cpp->UdpL_Y1);

	switch( DdxGetTyp_( dst ) )
	    {
	case XieK_UBAV :
	case XieK_UBAVU :
	    /* Destination is aligned or unaligned bit stream format */
	    for( scanline = y1; scanline <= y2; scanline++ )
		{
		srcpxloff = srcpxlstart+(x1-src->UdpL_X1)*src->UdpL_PxlStride;
		dstpxloff = dstpxlstart+(x1-dst->UdpL_X1)*dst->UdpL_PxlStride;
		cpppxloff = cpppxlstart;

		for( pixel = x1; pixel <= x2; pixel++ )
		    {
		    pxlvalue = GET_VALUE_(src->UdpA_Base, srcpxloff, srcmask);
		    if (GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1))
			pxlvalue = ((unsigned long *)lut->UdpA_Base)[pxlvalue];

		    PUT_VALUE_(dst->UdpA_Base, dstpxloff, pxlvalue, dstmask);

		    srcpxloff += src->UdpL_PxlStride;
		    dstpxloff += dst->UdpL_PxlStride;
		    cpppxloff += cpp->UdpL_PxlStride;
		    }
		srcpxlstart += src->UdpL_ScnStride;
		dstpxlstart += dst->UdpL_ScnStride;
		cpppxlstart += cpp->UdpL_ScnStride;
		}
	    break;

	case XieK_ABU :
	    /* Destination is byte aligned */
	    for( scanline = y1; scanline <= y2; scanline++ )
		{
		srcpxloff = srcpxlstart+(x1-src->UdpL_X1)*src->UdpL_PxlStride;
		dstpxloff = dstpxlstart+(x1-dst->UdpL_X1)*dst->UdpL_PxlStride;
		cpppxloff = cpppxlstart;

		for( pixel = x1; pixel <= x2; pixel++ )
		    {
		    pxlvalue = GET_VALUE_(src->UdpA_Base, srcpxloff, srcmask);

		    if (GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1) )
			pxlvalue = ((unsigned long *)lut->UdpA_Base)[pxlvalue];

		    (*(unsigned char *)
		     (dst->UdpA_Base + (dstpxloff >> 3))) = pxlvalue;

		    srcpxloff+=src->UdpL_PxlStride;
		    dstpxloff+=dst->UdpL_PxlStride;
		    cpppxloff += cpp->UdpL_PxlStride;
		    }
		srcpxlstart += src->UdpL_ScnStride;
		dstpxlstart += dst->UdpL_ScnStride;
		cpppxlstart += cpp->UdpL_ScnStride;
		}
	    break;

	case XieK_AWU :
	    /* Destination is word (16 bit short) aligned */
	    for( scanline = y1; scanline <= y2; scanline++ )
		{
		srcpxloff = srcpxlstart+(x1-src->UdpL_X1)*src->UdpL_PxlStride;
		dstpxloff = dstpxlstart+(x1-dst->UdpL_X1)*dst->UdpL_PxlStride;
		cpppxloff = cpppxlstart;

		for( pixel = x1; pixel <= x2; pixel++ )
		    {
		    pxlvalue = GET_VALUE_(src->UdpA_Base, srcpxloff, srcmask);

		    if (GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1) )
			pxlvalue = ((unsigned long *)lut->UdpA_Base)[pxlvalue];

		    (*(unsigned short *)
		     (dst->UdpA_Base + (dstpxloff >> 3))) = pxlvalue;

		    srcpxloff+=src->UdpL_PxlStride;
		    dstpxloff+=dst->UdpL_PxlStride;
		    cpppxloff += cpp->UdpL_PxlStride;
		    }
		srcpxlstart += src->UdpL_ScnStride;
		dstpxlstart += dst->UdpL_ScnStride;
		cpppxlstart += cpp->UdpL_ScnStride;
		}
	    break;

	    }				/* end switch */
	}
}					/* end _PointProcessLong */

/*****************************************************************************
**  _TempCvtLut
**
**  FUNCTIONAL DESCRIPTION:
**
**	Convert data, described by src UDP, into the nearest atomic array
**      datatype which will hold it.  The result is created with no
**      pad, so it can be indexed efficiently.
**      If the formats is already suitable, just return the original UDP.
**
**
**  FORMAL PARAMETERS:
**
**	src   - Pointer to input UDP
**
**  FUNCTION VALUE:
**
**      Returns a pointer to a UDP describing the data.
**
*****************************************************************************/
static UdpPtr _TempCvtLut(src)
    UdpPtr src;
{
    UdpPtr out;
    int pixel_length, dtype, status;

    if((src->UdpB_DType != UdpK_DTypeBU &&
	src->UdpB_DType != UdpK_DTypeWU &&
	src->UdpB_DType != UdpK_DTypeLU) ||
	(src->UdpL_PxlStride != src->UdpW_PixelLength ||
	 src->UdpL_ScnStride != (src->UdpL_PxlPerScn * src->UdpL_PxlStride) ||
	 src->UdpL_Pos != 0))
	{
	if( src->UdpL_Levels <= 256 )
	    {
	    pixel_length = 8 * sizeof(char);
	    dtype = UdpK_DTypeBU;
	    }
	else if( src->UdpL_Levels <= 65535 )
	    {
	    pixel_length = 8 * sizeof(short);
	    dtype = UdpK_DTypeWU;
	    }
	else
	    {
	    pixel_length = 8 * sizeof(long);
	    dtype = UdpK_DTypeLU;
	    }
	out = (UdpPtr) DdxMalloc_(sizeof(UdpRec));
	if( out == NULL ) return( (UdpPtr) BadAlloc );

	out->UdpW_PixelLength = pixel_length;
	out->UdpB_DType	      = dtype;
	out->UdpB_Class	      = UdpK_ClassA;
	out->UdpL_X1	      = src->UdpL_X1;
	out->UdpL_X2	      = src->UdpL_X2;
	out->UdpL_Y1	      = src->UdpL_Y1;
	out->UdpL_Y2	      = src->UdpL_Y2;
	out->UdpL_PxlPerScn   = src->UdpL_PxlPerScn;
	out->UdpL_ScnCnt      = src->UdpL_ScnCnt;
	out->UdpL_PxlStride   = out->UdpW_PixelLength;
	out->UdpL_ScnStride   = out->UdpL_PxlStride * out->UdpL_PxlPerScn;
	out->UdpL_ArSize      = out->UdpL_ScnStride * out->UdpL_ScnCnt;
	out->UdpA_Base	      = NULL;
	out->UdpL_Pos	      = 0;
	out->UdpL_CompIdx     = src->UdpL_CompIdx;
	out->UdpL_Levels      = src->UdpL_Levels;

	status = DdxConvert_( src, out, XieK_MoveMode );
	if( status != Success ) return( DdxFree_(out), (UdpPtr) status );
	}
    else
	out = src;

    return( out );
}					/* end _TempCvtLut */

/*****************************************************************************
**  _TempDestroyUdp
**
**  FUNCTIONAL DESCRIPTION:
**
**	Free a temporary UDPs and its data, if it is not just a
*       reference to another UDP.
**
**  FORMAL PARAMETERS:
**
**	in      - Pointer to input UDP
**      realudp - Point to a UDP which might be the same as the input UDP.
**
*****************************************************************************/
static void _TempDestroyUdp(in, realudp)
    UdpPtr in;
    UdpPtr realudp;
{
    if( IsPointer_(in) && in != realudp )
	{
	if( IsPointer_(in->UdpA_Base) )
	   DdxFreeBits_(in->UdpA_Base);
	DdxFree_(in);
	}
}					/* end _TempDestroyUdp */
