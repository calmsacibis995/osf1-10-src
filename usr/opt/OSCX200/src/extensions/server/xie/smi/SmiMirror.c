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
/*  DEC/CMS REPLACEMENT HISTORY, Element SMIROTATE.C */
/*  *11   13-MAR-1990 10:16:31 GREBUS "Fixed bug. Temp buffers must match drain datatype." */
/*  *10    9-MAR-1990 14:48:18 GREBUS "Fixed rotate when dest sink bigger than source." */
/*  *9     8-MAR-1990 14:54:02 GREBUS "ADD PIPELINE SUPPORT." */
/*  *8     3-JAN-1990 13:18:39 TAG "break up macro too complex for Pmax" */
/*  *7     2-NOV-1989 19:13:42 SHELLEY "move degrees to radians conversion factor to XieDdx.h" */
/*  *6     1-NOV-1989 19:55:10 TAG "copyright" */
/*  *5    30-OCT-1989 13:47:48 SHELLEY "duplicate code to improve execution speed" */
/*  *4    27-OCT-1989 14:46:36 SHELLEY "replace with all purpose version" */
/*  *3     2-OCT-1989 10:07:31 TAG "change angle from float to double" */
/*  *2    25-SEP-1989 15:14:31 GARIKAPATI " added support for bitonal rotate" */
/*  *1    14-SEP-1989 11:54:57 GARIKAPATI " rotate functionality" */
/*  DEC/CMS REPLACEMENT HISTORY, Element SMIROTATE.C */

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
**      Tue Mar 13 15:38:35 1990
**
**  MODIFICATION HISTORY:
**
*******************************************************************************/

/*
**  Include files 
*/
#include <stdio.h>

#include "SmiMirror.h"

/*
**  Table of contents
*/
int	SmiMirror();
int     SmiCreateMirror();

static int  _MirrorInitialize();
static int  _MirrorActivate();
static int  _MirrorFlush();
static int  _MirrorAbort();
static int  _MirrorDestroy();

static void _MirrorProcess();
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

/* Mirror Pipeline Element Vector */
static PipeElementVector MirrorPipeElement = {
    NULL,				/* FLINK			    */
    NULL,				/* BLINK			    */
    sizeof(PipeElementVector),		/* Size				    */
    TypePipeElementVector,		/* Structure type		    */
    StypeMirrorElement,			/* Structure subtype		    */
    sizeof(MirrorPipeCtx),		/* Context block size		    */
    0,					/* No input flag		    */
    0,					/* Reserved flags		    */
    _MirrorInitialize,			/* Initialize entry		    */
    _MirrorActivate,			/* Activate entry		    */
    NULL,				/* Flush entry			    */
    _MirrorDestroy,			/* Destroy entry		    */
    NULL				/* Abort entry			    */
    };

/*****************************************************************************
**  SmiCreateMirror
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function fills in a newly created MIRROR pipeline element.
**
**  FORMAL PARAMETERS:
**
**      ctx - mirror pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
int SmiCreateMirror( pipe, srcsnk, dstsnk, xflip, yflip )
 Pipe		     pipe;
 PipeSinkPtr	     srcsnk;
 PipeSinkPtr	     dstsnk;
 int		     xflip;
 int		     yflip;
{
    MirrorPipeCtxPtr ctx = (MirrorPipeCtxPtr)
			    DdxCreatePipeCtx_(pipe, &MirrorPipeElement, FALSE);
    if( !IsPointer_(ctx) ) return( (int) ctx );  

    /*
    **	Copy the parameters to the context block.
    */
    MirSrcSnk_(ctx) = srcsnk;
    MirDstSnk_(ctx) = dstsnk;
    MirXFlip_(ctx) = xflip;
    MirYFlip_(ctx) = yflip;
    /*
    **	Setup one drain from source sink, receiving all components
    */
    MirSrcDrn_(ctx) = DdxRmCreateDrain_( srcsnk, -1 );
    if( !IsPointer_(MirSrcDrn_(ctx)) ) return( (int) MirSrcDrn_(ctx) );
    /*
    **	Decide on data types we'll allow (we're not too fussy).
    */
    DdxRmSetDType_( MirSrcDrn_(ctx), UdpK_DTypeUndefined, DtM_Any );
    DdxRmSetDType_( MirDstSnk_(ctx), UdpK_DTypeUndefined, DtM_Any );
    /*
    **	Handle any quantum or wait for full image depending on X flip.
    */
    DdxRmSetQuantum_( MirSrcDrn_(ctx), xflip ? SnkHeight_(srcsnk,0) : 0 );
    DdxRmSetQuantum_( MirDstSnk_(ctx), xflip ? SnkHeight_(srcsnk,0) : 0 );

    return( Success ); 
}					/* end SmiCreateMirror */

/*****************************************************************************
**  _MirrorInitialize
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function initializes the MIRROR pipeline element prior to
**	an activation.
**
**  FORMAL PARAMETERS:
**
**      ctx - mirror pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	_MirrorInitialize(ctx)
MirrorPipeCtxPtr	ctx;
{
    int dtype, status;

    /*
    **	Initialize the source drain.
    */
    CtxInp_(ctx) = MirSrcDrn_(ctx);
    status = DdxRmInitializePort_( CtxHead_(ctx), MirSrcDrn_(ctx) );
    if( status != Success ) return( status );
    /*
    **	Set our destination sink DType to match our source and initialize it.
    */
    dtype = DdxRmGetDType_( MirSrcDrn_(ctx) );
    DdxRmSetDType_( MirDstSnk_(ctx), dtype, 1<<dtype );
    status = DdxRmInitializePort_( CtxHead_(ctx), MirDstSnk_(ctx) );

    return( status ); 
}

/*****************************************************************************
**  _MirrorActivate
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine reads data from the pipeline and flips it as requested.
**
**  FORMAL PARAMETERS:
**
**      ctx - mirror pipeline element context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _MirrorActivate(ctx)
MirrorPipeCtxPtr    ctx;
{
    PipeDataPtr	 src, dst;
    int status;
    
    for( status = Success; status == Success; DdxRmDeallocData_(src) )
	{
	src = DdxRmGetData_( ctx, MirSrcDrn_(ctx) );
	if( !IsPointer_(src) ) return( (int) src );

	if( MirXFlip_(ctx) || MirYFlip_(ctx) )
	    {
	    /* Allocate an output buffer */
	    dst = DdxRmAllocData_( MirDstSnk_(ctx), DatCmpIdx_(src),
				   DatY1_(src), DatHeight_(src) );
	    status = (int) dst;
	    if( !IsPointer_(dst) ) continue;
    
	    /* flip this quantum and pass it along */
	    _MirrorProcess( DatUdpPtr_(src), DatUdpPtr_(dst),
			    MirXFlip_(ctx),  MirYFlip_(ctx));

	    status = DdxRmPutData_( MirDstSnk_(ctx), dst );
	    }
	else 
	    {
	    /* If a no-op don't even move the data */
	    status = DdxRmPutData_( MirDstSnk_(ctx), src );
	    src = NULL;
	    }
	}
    return( status ); 
}

/*****************************************************************************
**  _MirrorDestroy
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine deallocates all resources allocated by the MIRROR
**	pipeline element.
**
**  FORMAL PARAMETERS:
**
**      ctx - dither pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _MirrorDestroy(ctx)
MirrorPipeCtxPtr    ctx;
{
    MirSrcDrn_(ctx) = DdxRmDestroyDrain_( MirSrcDrn_(ctx) );

    return( Success ); 
}

/******************************************************************************
**  SmiMirror
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine performs a non-pipelined mirror about the x, y
**      or x and y axes.  It handles the necessary data type setup
**      and invokes _MirrorProcess() to do the actual work.
**
**  FORMAL PARAMETERS:
**
**	in	- Pointer to source udp (initialized)
**	dst	- Pointer to destination udp (initialized, maybe without array)
**      xflip   - Mirror about X axis (Boolean)
**      yflip	- Mirror about Y axis (Boolean)
**
******************************************************************************/
int  SmiMirror( in, dst, xflip, yflip )
UdpPtr		in;
UdpPtr		dst;
int		xflip;
int		yflip;
{
int status = Success;
UdpPtr		src;			/* src upd pointer		    */

/*
**  Convert src to match dst data type if necessary.
*/
src = _CvtSrc( in, dst );
if( !IsPointer_(src) ) return( (int) src );

/*
**  Allocate memory for destination buffer (with a longword of pad added).
*/
if( dst->UdpA_Base == NULL )
    dst->UdpA_Base = DdxMallocBits_( dst->UdpL_ArSize );
if( dst->UdpA_Base == NULL )
    status = BadAlloc;
else
    /* Do the actual work */
    _MirrorProcess(src, dst, xflip, yflip);

if( src != in )
    {					    /* free converted input data    */
    DdxFreeBits_( src->UdpA_Base );
    DdxFree_( src );
    }
return( status ); 
}				    /* end SmiMirror */

/******************************************************************************
**  _MirrorProcess
**
**  FUNCTIONAL DESCRIPTION:
**
**	Mirror an image about the X or Y, or X and Y axes.
**
**  FORMAL PARAMETERS:
**
**	in	- Pointer to source udp (initialized)
**	dst	- Pointer to destination udp (initialized, maybe without array)
**      xflip   - Mirror about the X axis (Boolean)
**      yflip   - Mirror about the Y axis (Boolean)
**
******************************************************************************/
static void  _MirrorProcess( src, dst, xflip, yflip )
UdpPtr		src;
UdpPtr		dst;
int		xflip;
int		yflip;
{
unsigned long	pxlind, scnind;	    	/* Pixel and scanline indices	    */
unsigned long	srcpxloff, dstpxloff;	/* Offsets to current pixels */

unsigned long   srcscnoff,dstscnoff;	/* Offsets to start of current scan
					 * lines */
unsigned long	flippxloff;	        /* Offsets to first pixel in scanline
					 * to process. */

int	dstpxlinc, dstscninc;		/* Increment to successor
					 * pixel/scanline in dest */

unsigned long   pxlval;			/* Temporary pixel value */

unsigned long	smask,  dmask;		/* Src and dst pixel value masks    */

/*
 * Setup the offsets and increments.
 */
srcscnoff = src->UdpL_Pos;

if (xflip){
    /* Storing to last scan line and working backward */
    dstscninc = -dst->UdpL_ScnStride;
    dstscnoff = (dst->UdpL_ScnCnt -1) * dst->UdpL_ScnStride +  dst->UdpL_Pos;
}else {
    /* Storing to first scan and working forward */
    dstscninc = dst->UdpL_ScnStride;
    dstscnoff = dst->UdpL_Pos;
}
if (yflip) {
    /* Storing to last pixel and working backward */
    dstpxlinc  = -dst->UdpL_PxlStride;
    flippxloff = (dst->UdpL_PxlPerScn -1) * dst->UdpL_PxlStride;
}else {
    /* Storing to first pixel and working forward */
    dstpxlinc  = dst->UdpL_PxlStride;
    flippxloff = 0;
}
/*
**  Mirror the image data (code duplication traded for execution efficiency).
*/
switch( DdxGetTyp_( dst ) )
    {
case XieK_UBAV  :
case XieK_UBAVU :
    /*
    **	Mirror aligned or unaligned bit stream image data.
    */
    smask = (1<<src->UdpW_PixelLength)-1;   /* mask for isolating src pixel */
    dmask = (1<<dst->UdpW_PixelLength)-1;   /* mask for isolating dst pixel */

    for( scnind = 0;  scnind < src->UdpL_ScnCnt;  scnind++ ) {
	srcpxloff = srcscnoff;		    /* initial pixel bit offset */
	dstpxloff = dstscnoff + flippxloff;;

	for( pxlind = 0;  pxlind < dst->UdpL_PxlPerScn;  pxlind++ )
	    {	/*
		**  Copy the pixel.
		*/
	    pxlval = GET_VALUE_( src->UdpA_Base,  srcpxloff, smask );
	    PUT_VALUE_( dst->UdpA_Base, dstpxloff, pxlval, dmask );

	    srcpxloff += src->UdpL_PxlStride; /* Position to next pixel */
	    dstpxloff += dstpxlinc;
	    }
	srcscnoff += src->UdpL_ScnStride;    /* Position to next scanline  */
	dstscnoff += dstscninc;
	}
    break;		/* end unaligned bit stream image   */

case XieK_ABU :
    /*
    **	Mirror byte aligned byte size image data.
    */
    for( scnind = 0;  scnind < src->UdpL_ScnCnt;  scnind++ )
	{
	srcpxloff = srcscnoff;		    /* initial pixel bit offset */
	dstpxloff = dstscnoff + flippxloff;;

	for( pxlind = 0;  pxlind < dst->UdpL_PxlPerScn;  pxlind++ )
	    {	/*
		**  Copy the pixel.
		*/
	    *((unsigned char *)(dst->UdpA_Base + (dstpxloff >> 3))) =
		*((unsigned char *)(src->UdpA_Base + (srcpxloff >> 3)));

	    srcpxloff += src->UdpL_PxlStride; /* Position to next pixel */
	    dstpxloff += dstpxlinc;
	    }
	srcscnoff += src->UdpL_ScnStride;    /* Position to next scanline  */
	dstscnoff += dstscninc;
	}
    break;		/* end aligned byte image   */

case XieK_AWU :
    /*
    **	Mirror byte aligned word (16 bit short) size image data.
    */
    for( scnind = 0;  scnind < src->UdpL_ScnCnt;  scnind++ )
	{
	srcpxloff = srcscnoff;		    /* initial pixel bit offset */
	dstpxloff = dstscnoff + flippxloff;;

	for( pxlind = 0;  pxlind < dst->UdpL_PxlPerScn;  pxlind++ )
	    {	/*
		**  Copy the pixel.
		*/
	    *((unsigned short *)(dst->UdpA_Base + (dstpxloff >> 3))) =
		*((unsigned short *)(src->UdpA_Base + (srcpxloff >> 3)));

	    srcpxloff += src->UdpL_PxlStride; /* Position to next pixel */
	    dstpxloff += dstpxlinc;
	    }
	srcscnoff += src->UdpL_ScnStride;    /* Position to next scanline  */
	dstscnoff += dstscninc;
	}
    break;		/* end aligned byte image   */

case XieK_AF :
    /*
    **	Mirror aligned float image data.
    */
    smask = (1<<src->UdpW_PixelLength)-1;   /* mask for isolating src pixel */
    dmask = (1<<dst->UdpW_PixelLength)-1;   /* mask for isolating dst pixel */

    for( scnind = 0;  scnind < src->UdpL_ScnCnt;  scnind++ )
	{
	srcpxloff = srcscnoff;		    /* initial pixel bit offset */
	dstpxloff = dstscnoff + flippxloff;;

	for( pxlind = 0;  pxlind < dst->UdpL_PxlPerScn;  pxlind++ )
	    {	/*
		**  Copy the pixel.
		*/
	    *((float *)(dst->UdpA_Base + (dstpxloff >> 3))) =
		*((float *)(src->UdpA_Base + (srcpxloff >> 3)));

	    srcpxloff += src->UdpL_PxlStride; /* Position to next pixel */
	    dstpxloff += dstpxlinc;
	    }
	srcscnoff += src->UdpL_ScnStride;    /* Position to next scanline  */
	dstscnoff += dstscninc;
	}
    break;		/* end aligned float image  */
    }
}				    /* end _MirrorProcess */

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
/* end module SmiMirror.c */
