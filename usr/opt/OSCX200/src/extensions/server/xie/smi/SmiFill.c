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
**      This module contains support for the  operation.
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
**	Wed Jul 11 10:12:21 1990
**
*******************************************************************************/

#include <stdio.h>

#include "SmiFill.h"

/*
**  Table of contents
*/
int             SmiCreateFill();
int             SmiFill();
int             SmiFillRegion();

static int  	_CvtUnsOpr();
static int  	_FillBs();
static int  	_FillByt();
static int  	_FillFlt();
static int  	_FillWrd();

static int  	_FillBsCpp();
static int  	_FillBytCpp();
static int  	_FillFltCpp();
static int  	_FillWrdCpp();

static int 	_FillInitialize();
static int 	_FillActivate();
static int 	_FillDestroy();

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
    **  Fill Element Vector
    */
static PipeElementVector FillPipeElement = {
    NULL,				/* FLINK			    */
    NULL,				/* BLINK			    */
    sizeof(PipeElementVector),		/* Size				    */
    TypePipeElementVector,		/* Structure type		    */
    StypeFillElement,			/* Structure subtype		    */
    sizeof(FillPipeCtx),		/* Context block size		    */
    0,					/* No input flag		    */
    0,					/* Reserved flags		    */
    _FillInitialize,			/* Initialize entry		    */
    _FillActivate,			/* Activate entry		    */
    NULL,				/* Flush entry			    */
    _FillDestroy,			/* Destroy entry		    */
    NULL				/* Abort entry point		    */
    };

/*
**  This table describes functions to be called when filling a descriptor type.
*/
static int  (*fill_table[XieK_DATA_TYPE_COUNT])() =
 {_FillBs ,_FillBs, _CvtUnsOpr, _CvtUnsOpr, _CvtUnsOpr, _FillByt, _FillFlt, _FillWrd};

static int  (*fill_table_cpp[XieK_DATA_TYPE_COUNT])() =
 {_FillBsCpp ,_FillBsCpp, _CvtUnsOpr, _CvtUnsOpr, _CvtUnsOpr,
  _FillBytCpp, _FillFltCpp, _FillWrdCpp};


/*****************************************************************************
**  SmiCreateFill
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function fills in a newly created FILL pipeline element.
**
**  FORMAL PARAMETERS:
**
**      ctx - fill pipeline context
**
**  FUNCTION VALUE:
**
**      none
**
*****************************************************************************/
int 	    SmiCreateFill(pipe,src,dst,idcudp,constant)
Pipe		     pipe;
PipeSinkPtr	     src;
PipeSinkPtr	     dst;
UdpPtr               idcudp;
unsigned long int    constant[XieK_MaxComponents];
{
    int i;
    FillPipeCtxPtr ctx = (FillPipeCtxPtr)
			  DdxCreatePipeCtx_( pipe, &FillPipeElement, FALSE );
    if( !IsPointer_(ctx) ) return( (int) ctx );

    /*
    **	Copy parameters into context block.
    */
    FilSrcSnk_(ctx) = src;
    FilDstSnk_(ctx) = dst;
    if( idcudp == NULL )
	{
	/* Roi the entire source */
	FilRoiX1_(ctx) = SnkX1_(src,0);
	FilRoiX2_(ctx) = SnkX2_(src,0);
	FilRoiY1_(ctx) = SnkY1_(src,0);
	FilRoiY2_(ctx) = SnkY2_(src,0);
	FilCpp_(ctx)   = NULL;
	}
    else
	{
	FilRoiX1_(ctx) = idcudp->UdpL_X1;
	FilRoiX2_(ctx) = idcudp->UdpL_X2;
	FilRoiY1_(ctx) = idcudp->UdpL_Y1;
	FilRoiY2_(ctx) = idcudp->UdpL_Y2;
	if( idcudp->UdpA_Base == NULL )
	    FilCpp_(ctx) = NULL;
	else
	    FilCpp_(ctx) = idcudp;
	}	

    for( i = 0; i < XieK_MaxComponents && IsPointer_(SnkUdpPtr_(dst,i)); i++ )
	FilConst_(ctx,i) = constant[i] < SnkLvl_(dst,i)
			 ? constant[i] : SnkLvl_(dst,i) - 1;

    /*
    **	Setup one drain from source sink, receiving all components.
    */
    FilSrcDrn_(ctx) = DdxRmCreateDrain_( FilSrcSnk_(ctx), -1 );
    if( !IsPointer_(FilSrcDrn_(ctx)) ) return( (int) FilSrcDrn_(ctx) );

    /*
    **	Handle any data type.
    */
    DdxRmSetDType_( FilSrcDrn_(ctx), UdpK_DTypeUndefined, DtM_Any );
    DdxRmSetDType_( FilDstSnk_(ctx), UdpK_DTypeUndefined, DtM_Any );

    return( Success );
}					/* end SmiCreateFill */

/*****************************************************************************
**  _FillInitialize
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function initializes the FILL pipeline element prior to
**	an activation.
**
**  FORMAL PARAMETERS:
**
**      ctx - fill pipeline context
**
**  FUNCTION VALUE:
**
**      none
**
*****************************************************************************/
static int 	_FillInitialize(ctx)
FillPipeCtxPtr	ctx;
{
    int dtype, status;

    /*
    **	Initialize the source drain.
    */
    CtxInp_(ctx) = FilSrcDrn_(ctx);
    status = DdxRmInitializePort_( CtxHead_(ctx), FilSrcDrn_(ctx) );
    if( status != Success ) return( status );

    /*
    **	Set our destination sink DType to match our drain and initialize it.
    */
    dtype = DdxRmGetDType_( FilSrcDrn_(ctx) );
    DdxRmSetDType_( FilDstSnk_(ctx), dtype, 1<<dtype );
    status = DdxRmInitializePort_( CtxHead_(ctx), FilDstSnk_(ctx) );

    return( status );
}

/*****************************************************************************
**  _FillActivate
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine reads data from the pipeline and fills each quantum,
**	as necessary. 
**
**  FORMAL PARAMETERS:
**
**      ctx - fill pipeline element context
**
**  FUNCTION VALUE:
**
**      none
**
*****************************************************************************/
static int 	    _FillActivate(ctx)
FillPipeCtxPtr    ctx;
{
    PipeDataPtr	 src = NULL, dst = NULL;
    PipeSinkPtr	 dstsnk;
    UdpRec       udp, cppudp;
    int		 status, typ, X1, X2, Y1, Y2;

    for( status = Success; status == Success; src = DdxRmDeallocData_(src),
					      dst = DdxRmDeallocData_(dst) )
	{
	src = DdxRmGetData_( ctx, FilSrcDrn_(ctx) );
	if( !IsPointer_(src) ) return( (int) src );

	if( DatY2_(src) < FilRoiY1_(ctx) || DatY1_(src) > FilRoiY2_(ctx) )
	    {	/*
		**  Entire quantum is outside the ROI
		*/
	    status = DdxRmPutData_( FilDstSnk_(ctx), src );
	    src    = NULL;
	    }
	else
	    {
	    /* The quantum intersects the ROI.  Copy the regions outside
	     * the ROI and fill the region within */
	    dst = DdxRmAllocData_( FilDstSnk_(ctx), DatCmpIdx_(src),
				   DatY1_(src), DatHeight_(src) );
	    status = (int) dst;
	    if( !IsPointer_(dst) ) continue;

	    X1 = DatX1_(src) > FilRoiX1_(ctx) ? DatX1_(src) : FilRoiX1_(ctx);
	    X2 = DatX2_(src) < FilRoiX2_(ctx) ? DatX2_(src) : FilRoiX2_(ctx);
	    Y1 = DatY1_(src) > FilRoiY1_(ctx) ? DatY1_(src) : FilRoiY1_(ctx);
	    Y2 = DatY2_(src) < FilRoiY2_(ctx) ? DatY2_(src) : FilRoiY2_(ctx);
	    
	    if( DatY1_(src) < Y1 )
		{   /* Copy the part of this quantum above the ROI */
		status = DdxCopy_( DatUdpPtr_(src), DatUdpPtr_(dst), 0, 0,
				   DatX1_(dst), DatY1_(dst),
				   DatWidth_(src), Y1 - DatY1_(src) );
		if( status != Success ) continue;
		}
	    if( DatX1_(src) < X1 )
		{   /* Copy the part of this quantum left of the ROI */
		status = DdxCopy_( DatUdpPtr_(src), DatUdpPtr_(dst),
				   0, Y1 - DatY1_(src),
				   DatX1_(dst), Y1,
				   X1 - DatX1_(src), Y2 - Y1 + 1);
		if( status != Success ) continue;
		}
	    if( DatX2_(src) > X2 )
		{   /* Copy the part of this quantum right of the ROI */
		status = DdxCopy_( DatUdpPtr_(src), DatUdpPtr_(dst),
				   X2 - DatX1_(src)+1, Y1 - DatY1_(src),
				   X2+1, Y1,
				   DatX2_(src) - X2, Y2 - Y1 + 1);
		if( status != Success ) continue;
		}
	    if( DatY2_(src) > Y2 )
		{   /* Copy the part of this quantum below the ROI */
		status = DdxCopy_( DatUdpPtr_(src), DatUdpPtr_(dst),
				   0, Y2 - DatY1_(src) + 1,
				   DatX1_(dst), Y2 + 1,
				   DatWidth_(src), DatY2_(src) - Y2);
		if( status != Success ) continue;
		}
	    /* Fill the intersection of the IDC with this quantum */
	    udp = DatUdp_(dst);
	    udp.UdpL_X1 = X1;
	    udp.UdpL_X2 = X2;
	    udp.UdpL_Y1 = Y1;
	    udp.UdpL_Y2 = Y2;
	    udp.UdpL_PxlPerScn = udp.UdpL_X2 - udp.UdpL_X1 + 1;
	    udp.UdpL_ScnCnt    = udp.UdpL_Y2 - udp.UdpL_Y1 + 1;

	    udp.UdpL_Pos += (udp.UdpL_Y1 - DatY1_(dst)) * udp.UdpL_ScnStride
		          + (udp.UdpL_X1 - DatX1_(dst)) * udp.UdpL_PxlStride;

	    typ = DdxGetTyp_(&udp);

	    if( FilCpp_(ctx) == NULL )
		status = (*fill_table[typ])
			    (&udp, &FilConst_(ctx,DatCmpIdx_(src)));
	    else
		{
		/* Copy the source data to the destination, then fill
		 * the pixels selected by the CPP.
		 */
		status = DdxConvert_( DatUdpPtr_(src), &udp, XieK_MoveMode );
		if( status != Success ) continue;

	        cppudp = *FilCpp_(ctx);
		cppudp.UdpL_Pos +=
		  (udp.UdpL_Y1 - cppudp.UdpL_Y1) * cppudp.UdpL_ScnStride +
		  (udp.UdpL_X1 - cppudp.UdpL_X1) * cppudp.UdpL_PxlStride;

		status = (*fill_table_cpp[typ])
			    (&udp, &cppudp, &FilConst_(ctx,DatCmpIdx_(src)));
		}
	    if( status != Success ) continue;

	    status = DdxRmPutData_( FilDstSnk_(ctx), dst );
	    dst    = NULL;
	    }
	}
    return( status );
}

/*****************************************************************************
**  _FillDestroy
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine deallocates all resources allocated by the FILL
**	pipeline element.
**
**  FORMAL PARAMETERS:
**
**      ctx - fill pipeline context
**
**  FUNCTION VALUE:
**
**      none
**
*****************************************************************************/
static int 	    _FillDestroy(ctx)
FillPipeCtxPtr    ctx;
{
    FilSrcDrn_(ctx) = DdxRmDestroyDrain_( FilSrcDrn_(ctx) );

    return( Success );
}

/*****************************************************************************
**  SmiFill
**
**  FUNCTIONAL DESCRIPTION:
**
**      Fill pixels in a UDP as specified by an IDC.
**
**  FORMAL PARAMETERS:
**
**      src     - source data descriptor
**	dst	- destination data descriptor
**      idcudp  - destination IDC
**	value	- pel fill pattern
**
**  FUNCTION VALUE:
**
**      none
**
*****************************************************************************/
int 	    SmiFill( src, dst, idcudp, value )
UdpPtr		src, dst;
UdpPtr          idcudp;
unsigned int	value;
{
    UdpRec	    udp;
    int		    typ, status;
    unsigned long   val;

    /*
    **  Allocate the destination image array
    */
    if( dst->UdpA_Base == NULL )
	{
        dst->UdpA_Base = DdxMallocBits_( dst->UdpL_ArSize );
        if( dst->UdpA_Base == NULL ) return( BadAlloc );
	}

    /*
     * Copy the image.
     */
    status = DdxConvert_( src, dst, XieK_MoveMode );
    if( status != Success ) return( status );
    
    /*
    **	Validate and get type of destination
    */
    typ = DdxGetTyp_(dst);
    if( typ >= XieK_DATA_TYPE_COUNT ) return( BadImplementation );

    /*
    **	Make a copy of the destination UDP that we can modify.
    */
    udp = *dst;
    /*
    **	Modify the UDP so that is describes only the ROI to receive the fill.
    */
    if( idcudp != NULL )
	{
	/* If the IDC was supplied, find its intersection with the image */
	udp.UdpL_X1 = idcudp->UdpL_X1 > udp.UdpL_X1 ?
		      idcudp->UdpL_X1 : udp.UdpL_X1;
	udp.UdpL_X2 = idcudp->UdpL_X2 < udp.UdpL_X2 ?
		      idcudp->UdpL_X2 : udp.UdpL_X2;
	udp.UdpL_Y1 = idcudp->UdpL_Y1 > udp.UdpL_Y1 ?
		      idcudp->UdpL_Y1 : udp.UdpL_Y1;
	udp.UdpL_Y2 = idcudp->UdpL_Y2 < udp.UdpL_Y2 ?
		      idcudp->UdpL_Y2 : udp.UdpL_Y2;
	}
    udp.UdpL_PxlPerScn = udp.UdpL_X2 - udp.UdpL_X1 + 1;
    udp.UdpL_ScnCnt    = udp.UdpL_Y2 - udp.UdpL_Y1 + 1;

    udp.UdpL_Pos += (udp.UdpL_Y1 - dst->UdpL_Y1) * udp.UdpL_ScnStride
		  + (udp.UdpL_X1 - dst->UdpL_X1) * udp.UdpL_PxlStride;

    val = value < udp.UdpL_Levels ? value : udp.UdpL_Levels - 1;
    /*
    **	Call specific fill routine.
    */
    if( idcudp == NULL || idcudp->UdpA_Base == NULL )
	status = (*fill_table[typ])(&udp, &val);
    else
	status = (*fill_table_cpp[typ])(&udp, idcudp, &val);
	
    return( status );
}				    /* end SmiFill */

/*****************************************************************************
**  SmiFillRegion
**
**  FUNCTIONAL DESCRIPTION:
**
**      This is utility style interface to SmiFill.
**
**  FORMAL PARAMETERS:
**
**	dst	- destination data descriptor
**	dst_x	- Destination X coordinate
**	dst_y	- Destination Y coordinate
**      width	- Width of ROI in pels
**	height	- Height of ROI in pels
**	value	- pel fill pattern
**
**  FUNCTION VALUE:
**
**      Standard X status codes.
**
*****************************************************************************/
int 	    SmiFillRegion( dst, x, y, width, height, value )
UdpPtr		dst;
int	        x;
int	        y;
unsigned int	width;
unsigned int	height;
unsigned int	value;
{
    UdpRec	    udp;
    int		    typ;
    unsigned long   val;

    /*
     * Handle the no-op case.  
     */
    if (width == 0 || height == 0)
	return( Success );
    /*
    **	Validate and get type of descriptors
    */
    typ = DdxGetTyp_(dst);
    if( typ >= XieK_DATA_TYPE_COUNT ) return( BadImplementation );
    /*
    **	Make a copy of the destination UDP that we can modify.
    */
    udp = *dst;
    /*
    **	Modify the UDP so that is describes only the ROI to receive the value.
    */
    udp.UdpL_X1 = x		 > udp.UdpL_X1 ? x		: udp.UdpL_X1;
    udp.UdpL_X2 = x + width  - 1 < udp.UdpL_X2 ? x + width  - 1 : udp.UdpL_X2;
    udp.UdpL_Y1 = y		 > udp.UdpL_Y1 ? y		: udp.UdpL_Y1;
    udp.UdpL_Y2 = y + height - 1 < udp.UdpL_Y2 ? y + height - 1 : udp.UdpL_Y2;

    udp.UdpL_PxlPerScn = udp.UdpL_X2 - udp.UdpL_X1 + 1;
    udp.UdpL_ScnCnt    = udp.UdpL_Y2 - udp.UdpL_Y1 + 1;

    udp.UdpL_Pos += (udp.UdpL_Y1 - dst->UdpL_Y1) * udp.UdpL_ScnStride
		  + (udp.UdpL_X1 - dst->UdpL_X1) * udp.UdpL_PxlStride;

    val = value < udp.UdpL_Levels ? value : udp.UdpL_Levels - 1;
    /*
    **	Call specific fill routine.
    */
    return( (*fill_table[typ])(&udp, &val) );
}				    /* end SmiFillRegion */

/*****************************************************************************
**  _CvtUnsOpr
**
**  FUNCTIONAL DESCRIPTION:
**
**      This is an action routine called when conversion from source descriptor
**	type to destination descriptor type is not supported. Simply signal an
**	error at this point.
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int  _CvtUnsOpr(src, dst)
UdpPtr  src;
UdpPtr  dst;
{
    return( BadImplementation );
}

/*****************************************************************************
**  _FillBs
**
**  FUNCTIONAL DESCRIPTION:
**
**      Fill a region of a bit array with an integer value
**
**  FORMAL PARAMETERS:
**
**	dst - destination data descriptor
**	val - value
**
*****************************************************************************/
static int _FillBs( dst, val )
UdpPtr  dst;
unsigned long *val;
{
    unsigned long value = *val;
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int dst_mask;
    int dst_scan_offset;
    int dst_offset;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
        {
        dst->UdpA_Base = DdxMallocBits_( dst->UdpL_ArSize );
        if( dst->UdpA_Base == NULL ) return( BadAlloc );
        }
/*
**  Get destination dimensions.
*/
    X1 = dst->UdpL_X1;
    X2 = dst->UdpL_X2;
    Y1 = dst->UdpL_Y1;
    Y2 = dst->UdpL_Y2;

    dst_mask = (1 << dst->UdpW_PixelLength) - 1;

    dst_scan_offset = dst->UdpL_Pos;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	dst_offset = dst_scan_offset;

	for( pixel = X1;  pixel <= X2;  pixel++ )
	    {
	    PUT_VALUE_(dst->UdpA_Base, dst_offset, value, dst_mask);
	    dst_offset += dst->UdpL_PxlStride;
	    }

	dst_scan_offset += dst->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _FillByt
**
**  FUNCTIONAL DESCRIPTION:
**
**      Fill a region of a byte array with an integer value
**
**  FORMAL PARAMETERS:
**
**	dst - destination data descriptor
**	val - value
**
*****************************************************************************/
static int _FillByt( dst, val )
UdpPtr  dst;
unsigned long *val;
{
    unsigned char value = *val;
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int dst_scan_offset;
    int dst_offset;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
        {
        dst->UdpA_Base = DdxMallocBits_( dst->UdpL_ArSize );
        if( dst->UdpA_Base == NULL ) return( BadAlloc );
        }
/*
**  Get destination dimensions.
*/
    X1 = dst->UdpL_X1;
    X2 = dst->UdpL_X2;
    Y1 = dst->UdpL_Y1;
    Y2 = dst->UdpL_Y2;

    dst_scan_offset = dst->UdpL_Pos;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	dst_offset = dst_scan_offset;

	for( pixel = X1;  pixel <= X2;  pixel++ )
	    {
	    *((unsigned char *)(dst->UdpA_Base + (dst_offset >> 3))) = value;
	    dst_offset += dst->UdpL_PxlStride;
	    }

	dst_scan_offset += dst->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _FillFlt
**
**  FUNCTIONAL DESCRIPTION:
**
**      Fill a region of a floating array with a floating value
**
**  FORMAL PARAMETERS:
**
**	dst - destination data descriptor
**	val - floating value
**
*****************************************************************************/
static int _FillFlt( dst, val )
UdpPtr  dst;
float  *val;
{
    float value = *val;
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int dst_scan_offset;
    int dst_offset;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
        {
        dst->UdpA_Base = DdxMallocBits_( dst->UdpL_ArSize );
        if( dst->UdpA_Base == NULL ) return( BadAlloc );
        }
/*
**  Get destination dimensions.
*/
    X1 = dst->UdpL_X1;
    X2 = dst->UdpL_X2;
    Y1 = dst->UdpL_Y1;
    Y2 = dst->UdpL_Y2;

    dst_scan_offset = dst->UdpL_Pos;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	dst_offset = dst_scan_offset;

	for( pixel = X1;  pixel <= X2;  pixel++ )
	    {
	    *((float *)(dst->UdpA_Base + (dst_offset >> 3))) = value;
	    dst_offset += dst->UdpL_PxlStride;
	    }

	dst_scan_offset += dst->UdpL_ScnStride;
	}
    return( Success );
}


/*****************************************************************************
**  _FillWrd
**
**  FUNCTIONAL DESCRIPTION:
**
**      Fill a region of a word array with an integer value
**
**  FORMAL PARAMETERS:
**
**	dst - destination data descriptor
**	val - value
**
*****************************************************************************/
static int _FillWrd( dst, val )
UdpPtr  dst;
unsigned long *val;
{
    unsigned short value = *val;
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int dst_scan_offset;
    int dst_offset;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
        {
        dst->UdpA_Base = DdxMallocBits_( dst->UdpL_ArSize );
        if( dst->UdpA_Base == NULL ) return( BadAlloc );
        }
/*
**  Get destination dimensions.
*/
    X1 = dst->UdpL_X1;
    X2 = dst->UdpL_X2;
    Y1 = dst->UdpL_Y1;
    Y2 = dst->UdpL_Y2;

    dst_scan_offset = dst->UdpL_Pos;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	dst_offset = dst_scan_offset;

	for( pixel = X1;  pixel <= X2;  pixel++ )
	    {
	    *((unsigned short *)(dst->UdpA_Base + (dst_offset >> 3))) = value;
	    dst_offset += dst->UdpL_PxlStride;
	    }

	dst_scan_offset += dst->UdpL_ScnStride;
	}
    return( Success );
}


/*****************************************************************************
**  _FillBsCpp
**
**  FUNCTIONAL DESCRIPTION:
**
**      Fill a region of a bit array with an integer value, under the
**      control of a CPP.
**
**  FORMAL PARAMETERS:
**
**	dst - destination data descriptor
**      cpp - pointer to CPP
**	val - value
**
*****************************************************************************/
static int _FillBsCpp( dst, cpp, val )
UdpPtr  dst;
UdpPtr  cpp;
unsigned long *val;
{
    unsigned long value = *val;
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int dst_mask;
    int dst_scan_offset;
    int dst_offset;
    int cpp_scan_offset;
    int cpp_offset;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
        {
        dst->UdpA_Base = DdxMallocBits_( dst->UdpL_ArSize );
        if( dst->UdpA_Base == NULL ) return( BadAlloc );
        }
/*
**  Get destination dimensions.
*/
    X1 = dst->UdpL_X1;
    X2 = dst->UdpL_X2;
    Y1 = dst->UdpL_Y1;
    Y2 = dst->UdpL_Y2;

    dst_mask = (1 << dst->UdpW_PixelLength) - 1;

    dst_scan_offset = dst->UdpL_Pos;
    cpp_scan_offset = cpp->UdpL_Pos;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	dst_offset = dst_scan_offset;
	cpp_offset = cpp_scan_offset;

	for( pixel = X1;  pixel <= X2;  pixel++ )
	    {
	    if (GET_VALUE_(cpp->UdpA_Base, cpp_offset, 1))
	        PUT_VALUE_(dst->UdpA_Base, dst_offset, value, dst_mask);
	    dst_offset += dst->UdpL_PxlStride;
	    cpp_offset += cpp->UdpL_PxlStride;
	    }

	dst_scan_offset += dst->UdpL_ScnStride;
	cpp_scan_offset += cpp->UdpL_ScnStride;
	}
    return( Success );
}					/* end _FillBsCpp */


/*****************************************************************************
**  _FillBytCpp
**
**  FUNCTIONAL DESCRIPTION:
**
**      Fill a region of a byte array with an integer value, under the
**      control of a CPP.
**
**  FORMAL PARAMETERS:
**
**	dst - destination data descriptor
**      cpp - pointer to CPP
**	val - value
**
*****************************************************************************/
static int _FillBytCpp( dst, cpp, val )
UdpPtr  dst;
UdpPtr  cpp;
unsigned long *val;
{
    unsigned char value = *val;
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int dst_scan_offset;
    int dst_offset;
    int cpp_scan_offset;
    int cpp_offset;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
        {
        dst->UdpA_Base = DdxMallocBits_( dst->UdpL_ArSize );
        if( dst->UdpA_Base == NULL ) return( BadAlloc );
        }
/*
**  Get destination dimensions.
*/
    X1 = dst->UdpL_X1;
    X2 = dst->UdpL_X2;
    Y1 = dst->UdpL_Y1;
    Y2 = dst->UdpL_Y2;

    dst_scan_offset = dst->UdpL_Pos;
    cpp_scan_offset = cpp->UdpL_Pos;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	dst_offset = dst_scan_offset;
	cpp_offset = cpp_scan_offset;

	for( pixel = X1;  pixel <= X2;  pixel++ )
	    {
	    if (GET_VALUE_(cpp->UdpA_Base, cpp_offset, 1)) 
		*((unsigned char *)(dst->UdpA_Base + (dst_offset >> 3))) =
		                                                       value;
	    dst_offset += dst->UdpL_PxlStride;
	    cpp_offset += cpp->UdpL_PxlStride;
	    }

	dst_scan_offset += dst->UdpL_ScnStride;
	cpp_scan_offset += cpp->UdpL_ScnStride;
	}
    return( Success );
}					/* end _FillBytCpp */


/*****************************************************************************
**  _FillFltCpp
**
**  FUNCTIONAL DESCRIPTION:
**
**      Fill a region of a floating array with a floating value,
**      under control of a CPP.
**
**  FORMAL PARAMETERS:
**
**	dst - destination data descriptor
**      cpp - pointer to CPP
**	val - floating value
**
*****************************************************************************/
static int _FillFltCpp( dst, cpp, val )
UdpPtr  dst;
UdpPtr  cpp;
float  *val;
{
    float value = *val;
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int dst_scan_offset;
    int dst_offset;
    int cpp_scan_offset;
    int cpp_offset;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
        {
        dst->UdpA_Base = DdxMallocBits_( dst->UdpL_ArSize );
        if( dst->UdpA_Base == NULL ) return( BadAlloc );
        }
/*
**  Get destination dimensions.
*/
    X1 = dst->UdpL_X1;
    X2 = dst->UdpL_X2;
    Y1 = dst->UdpL_Y1;
    Y2 = dst->UdpL_Y2;

    dst_scan_offset = dst->UdpL_Pos;
    cpp_scan_offset = cpp->UdpL_Pos;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	dst_offset = dst_scan_offset;
	cpp_offset = cpp_scan_offset;

	for( pixel = X1;  pixel <= X2;  pixel++ )
	    {
	    if (GET_VALUE_(cpp->UdpA_Base, cpp_offset, 1))
		*((float *)(dst->UdpA_Base + (dst_offset >> 3))) = value;

	    dst_offset += dst->UdpL_PxlStride;
	    cpp_offset += cpp->UdpL_PxlStride;
	    }

	dst_scan_offset += dst->UdpL_ScnStride;
	cpp_scan_offset += cpp->UdpL_ScnStride;
	}
    return( Success );
}					/* end _FillFltCpp */


/*****************************************************************************
**  _FillWrdCpp
**
**  FUNCTIONAL DESCRIPTION:
**
**      Fill a region of a word array with an integer value, under the
**      control of a CPP.
**
**  FORMAL PARAMETERS:
**
**	dst - destination data descriptor
**      cpp - pointer to CPP
**	val - value
**
*****************************************************************************/
static int _FillWrdCpp( dst, cpp, val )
UdpPtr  dst;
UdpPtr  cpp;
unsigned long *val;
{
    unsigned short value = *val;
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int dst_scan_offset;
    int dst_offset;
    int cpp_scan_offset;
    int cpp_offset;

/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
        {
        dst->UdpA_Base = DdxMallocBits_( dst->UdpL_ArSize );
        if( dst->UdpA_Base == NULL ) return( BadAlloc );
        }

/*
**  Get destination dimensions.
*/
    X1 = dst->UdpL_X1;
    X2 = dst->UdpL_X2;
    Y1 = dst->UdpL_Y1;
    Y2 = dst->UdpL_Y2;

    dst_scan_offset = dst->UdpL_Pos;
    cpp_scan_offset = cpp->UdpL_Pos;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	dst_offset = dst_scan_offset;
	cpp_offset = cpp_scan_offset;

	for( pixel = X1;  pixel <= X2;  pixel++ )
	    {
	    if (GET_VALUE_(cpp->UdpA_Base, cpp_offset, 1)) 
		*((unsigned short *)(dst->UdpA_Base + (dst_offset >> 3))) =
		                                                       value;
	    dst_offset += dst->UdpL_PxlStride;
	    cpp_offset += cpp->UdpL_PxlStride;
	    }

	dst_scan_offset += dst->UdpL_ScnStride;
	cpp_scan_offset += cpp->UdpL_ScnStride;
	}
    return( Success );
}					/* end _FillWrdCpp */

/*  E N D  of  S m i F i l l . c   */
