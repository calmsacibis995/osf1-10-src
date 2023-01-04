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
**      This module does a Floyd Stienberg dither, same as blue noise
**	only no random, no serpentine stuff.
**
**  ENVIRONMENT:
**
**	VAX/VMS V5.3
**	ULTRIX  V3.1
**
**  AUTHORS:
**
**	John Weber
**
**  CREATION DATE:     
**
**	28-April-1989
**
*******************************************************************************/

#include <stdio.h>

#include "SmiDither.h"

/*
**  Table of contents
*/
int 		SmiDither();
int             SmiCreateDither();

static int 	_DitherInitialize();
static int 	_DitherActivate();
static int 	_DitherAbort();
static int 	_DitherDestroy();

static void	DitherProcess();

/*
**  Equated Symbols
*/

#define F1_32	0.03125
#define F1_8	0.125
#define F7_16	0.4375
#define F1_16	0.0625
#define F5_16	0.3125
#define F3_16	0.1875

/*
**  External References
*/

/*
**  Local storage
*/
    /*
    **  Dither Element Vector
    */
static PipeElementVector DitherPipeElement = {
    NULL,				/* FLINK			    */
    NULL,				/* BLINK			    */
    sizeof(PipeElementVector),		/* Size				    */
    TypePipeElementVector,		/* Structure type		    */
    StypeDitherElement,			/* Structure subtype		    */
    sizeof(DitherPipeCtx),		/* Context block size		    */
    0,					/* No input flag		    */
    0,					/* Reserved flags		    */
    _DitherInitialize,			/* Initialize entry		    */
    _DitherActivate,			/* Activate entry		    */
    _DitherAbort,			/* Flush entry			    */
    _DitherDestroy,			/* Destroy entry		    */
    _DitherAbort			/* Abort entry point		    */
    };

/*****************************************************************************
**  SmiCreateDither
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function initializes the DITHER pipeline element
**
**  FORMAL PARAMETERS:
**
**      ctx - dither pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
int 	    SmiCreateDither(pipe,srcsnk,dstsnk)
Pipe		     pipe;
PipeSinkPtr	     srcsnk;
PipeSinkPtr	     dstsnk;
{
    int i, dtype;
    DitherPipeCtxPtr ctx = (DitherPipeCtxPtr)
			    DdxCreatePipeCtx_(pipe, &DitherPipeElement, FALSE);
    if( !IsPointer_(ctx) ) return( (int) ctx );
    /*
    **	Copy parameters
    */
    DtrSrcSnk_(ctx) = srcsnk;
    DtrDstSnk_(ctx) = dstsnk;

    DtrSrcDrn_(ctx) = DdxRmCreateDrain_( srcsnk, -1 );
    if( !IsPointer_(DtrSrcDrn_(ctx)) ) return( (int) DtrSrcDrn_(ctx) );

    DdxRmSetDType_( DtrSrcDrn_(ctx), UdpK_DTypeUndefined, DtM_BU | DtM_WU );
    DdxRmSetQuantum_( DtrSrcDrn_(ctx), 1 );

    for(i = 0; i < XieK_MaxComponents && IsPointer_(SnkUdpPtr_(dstsnk,i)); i++)
	if( SnkLvl_(dstsnk,i) <= 256 )
	    dtype = UdpK_DTypeBU;
	else
	    {
	    dtype = UdpK_DTypeWU;
	    break;
	    }
    DdxRmSetDType_( dstsnk, dtype, 1<<dtype );
    DdxRmSetQuantum_( dstsnk, 1 );

    return( Success );
}					/* end of SmiCreateDither */

/*****************************************************************************
**  _DitherInitialize
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function initializes the DITHER pipeline element
**
**  FORMAL PARAMETERS:
**
**      ctx - dither pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	_DitherInitialize(ctx)
DitherPipeCtxPtr	ctx;
{
    int i;
    int status  = DdxRmInitializePort_( CtxHead_(ctx), DtrSrcDrn_(ctx) );

    if( status == Success )
	status  = DdxRmInitializePort_( CtxHead_(ctx), DtrDstSnk_(ctx) );

    CtxInp_(ctx) = DtrSrcDrn_(ctx);

    for( i = 0;  i < XieK_MaxComponents && status == Success;  i++ )
	{
	if (SnkUdpPtr_(DtrSrcSnk_(ctx),i) == NULL)
	    break;
	DtrErrBuf_(ctx,i) = (double *)
		    DdxCalloc_( SnkWidth_(DtrSrcSnk_(ctx),i)+2, sizeof(double));
	if( DtrErrBuf_(ctx,i) == NULL )
	    status = BadAlloc;
	}
    if( status == Success )
	DtrNxtErr_(ctx) = (double *)
		    DdxCalloc_(SnkWidth_(DtrSrcSnk_(ctx),0)+2, sizeof(double));
    if( DtrNxtErr_(ctx) == NULL )
	status = BadAlloc;

    return( status );
}					/* end of _DitherInitialize */

/*****************************************************************************
**  _DitherActivate
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine controls data scaling
**
**  FORMAL PARAMETERS:
**
**      ctx - dither pipeline element context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _DitherActivate(ctx)
DitherPipeCtxPtr    ctx;
{
    PipeDataPtr	 src;
    PipeDataPtr	 dst;
    int  status, error_index;
    double	*tmp;

    for( status = Success; status == Success; src = DdxRmDeallocData_(src) )
	{
	src = DdxRmGetData_( ctx, DtrSrcDrn_(ctx) );
	if( !IsPointer_(src) ) return( (int) src );

	dst = DdxRmAllocData_( DtrDstSnk_(ctx), DatCmpIdx_(src), DatY1_(src),
							     DatHeight_(src) );
	status = (int) dst;
	if( !IsPointer_(dst) ) continue;

	if( DatLvl_(dst) < 2 )
	    {
	    status = DdxFillRegion_( DatUdpPtr_(dst),
				     DatX1_(dst), DatY1_(dst),
				     DatWidth_(dst), DatHeight_(dst), 0 );
	    if( status == Success )
		status = DdxRmPutData_( DtrDstSnk_(ctx), dst );
	    else
		DdxRmDeallocData_(dst);
	    }
	else
	    {
	    error_index = 0;
	    DitherProcess( DatUdpPtr_(src),
			   DatUdpPtr_(dst),
			   DtrNxtErr_(ctx),
			   DtrErrBuf_(ctx,DatCmpIdx_(src)),
			  &error_index );

	    if( error_index != 0 )
		{
		tmp = DtrNxtErr_(ctx);
		DtrNxtErr_(ctx) = DtrErrBuf_(ctx,DatCmpIdx_(src));
		DtrErrBuf_(ctx,DatCmpIdx_(src)) = tmp;
		}
	    status = DdxRmPutData_( DtrDstSnk_(ctx), dst );
	    }
	}
    return( status );
}					/* end of _DitherActivate */

/*****************************************************************************
**  _DitherAbort
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**      ctx - dither pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _DitherAbort(ctx)
DitherPipeCtxPtr    ctx;
{
    int i;

    for( i = 0;  i < XieK_MaxComponents;  i++ )
	if( IsPointer_(DtrErrBuf_(ctx,i)) )
	    DtrErrBuf_(ctx,i) = (double *) DdxCfree_(DtrErrBuf_(ctx,i));

    if( IsPointer_(DtrNxtErr_(ctx)) )
	DtrNxtErr_(ctx) = (double *) DdxCfree_(DtrNxtErr_(ctx));

    return( Success );
}					/* end of _DitherAbort */

/*****************************************************************************
**  _DitherDestroy
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine deallocates all resources allocated by the dither
**	pipeline element.
**
**  FORMAL PARAMETERS:
**
**      ctx - dither pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _DitherDestroy(ctx)
DitherPipeCtxPtr    ctx;
{
    DtrSrcDrn_(ctx) = DdxRmDestroyDrain_( DtrSrcDrn_(ctx) );

    return( Success );
}					/* end of _DitherDestroy */

/*****************************************************************************
**  SmiDither
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
int 	SmiDither( srcudp, dstudp )
UdpPtr	srcudp;
UdpPtr	dstudp;
{
    UdpPtr   src = NULL, dst = NULL;
    int	     status = Success, curerr = 0;
    double  *errbuf[2];
    errbuf[0] = NULL;
    errbuf[1] = NULL;

    if( dstudp->UdpA_Base == NULL )
	{   /*
	    **  Allocate the destination image array
	    */
        dstudp->UdpA_Base = DdxMallocBits_( dstudp->UdpL_ArSize );
        if( dstudp->UdpA_Base == NULL ) return( BadAlloc );
	}
    if( dstudp->UdpL_Levels < 2 )
	    /*
	    **	Forget the image -- just zero-fill the destination scanlines.
	    */
	return( DdxFillRegion_( dstudp,
				dstudp->UdpL_X1, dstudp->UdpL_Y1,
				dstudp->UdpL_PxlPerScn,
				dstudp->UdpL_ScnCnt, 0 ) );

    if(srcudp->UdpB_DType == UdpK_DTypeBU || srcudp->UdpB_DType == UdpK_DTypeWU)
	src = srcudp;
    else
	{   /*
	    **	Convert input data buffer to our "prefered" format.
	    */
	src = (UdpPtr) DdxMalloc_(sizeof(UdpRec));
	if( src == NULL ) return( BadAlloc );

	src->UdpW_PixelLength	= srcudp->UdpL_Levels > 256 ? 8 * sizeof(short)
							    : 8 * sizeof(char);
	src->UdpB_DType		= srcudp->UdpL_Levels > 256 ? UdpK_DTypeWU
							    : UdpK_DTypeBU;
	src->UdpB_Class		= UdpK_ClassA;
	src->UdpL_X1		= srcudp->UdpL_X1;
	src->UdpL_X2		= srcudp->UdpL_X2;
	src->UdpL_Y1		= srcudp->UdpL_Y1;
	src->UdpL_Y2		= srcudp->UdpL_Y2;
	src->UdpL_PxlPerScn	= srcudp->UdpL_PxlPerScn;
	src->UdpL_ScnCnt	= srcudp->UdpL_ScnCnt;
	src->UdpL_PxlStride	= src->UdpW_PixelLength;
	src->UdpL_ScnStride	= src->UdpL_PxlStride * src->UdpL_PxlPerScn;
	src->UdpL_ArSize	= src->UdpL_ScnStride * src->UdpL_ScnCnt;
	src->UdpA_Base		= NULL;
	src->UdpL_Pos		= 0;
	src->UdpL_CompIdx	= srcudp->UdpL_CompIdx;
	src->UdpL_Levels	= srcudp->UdpL_Levels;
	status = DdxConvert_( srcudp, src, XieK_MoveMode );
	if( status != Success ) goto clean_up;
	}

    if(dstudp->UdpB_DType == UdpK_DTypeBU || dstudp->UdpB_DType == UdpK_DTypeWU)
	dst = dstudp;
    else
	{   /*
	    **	Convert output data buffer to our "prefered" format.
	    */
	dst = (UdpPtr) DdxMalloc_(sizeof(UdpRec));
	if( dst == NULL )
	    {
	    status = BadAlloc;
	    goto clean_up;
	    }
	dst->UdpW_PixelLength	= dstudp->UdpL_Levels > 256 ? 8 * sizeof(short)
							    : 8 * sizeof(char);
	dst->UdpB_DType		= dstudp->UdpL_Levels > 256 ? UdpK_DTypeWU
							    : UdpK_DTypeBU;
	dst->UdpB_Class		= UdpK_ClassA;
	dst->UdpL_X1		= dstudp->UdpL_X1;
	dst->UdpL_X2		= dstudp->UdpL_X2;
	dst->UdpL_Y1		= dstudp->UdpL_Y1;
	dst->UdpL_Y2		= dstudp->UdpL_Y2;
	dst->UdpL_PxlPerScn	= dstudp->UdpL_PxlPerScn;
	dst->UdpL_ScnCnt	= dstudp->UdpL_ScnCnt;
	dst->UdpL_PxlStride	= dst->UdpW_PixelLength;
	dst->UdpL_ScnStride	= dst->UdpL_PxlStride * dst->UdpL_PxlPerScn;
	dst->UdpL_ArSize	= dst->UdpL_ScnStride * dst->UdpL_ScnCnt;
	dst->UdpA_Base		= DdxMallocBits_( dst->UdpL_ArSize );
	dst->UdpL_Pos		= 0;
	dst->UdpL_CompIdx	= dstudp->UdpL_CompIdx;
	dst->UdpL_Levels	= dstudp->UdpL_Levels;
	if( dst->UdpA_Base == NULL )
	    {
	    status = BadAlloc;
	    goto clean_up;
	    }
	}
    /*
    **	Allocate error buffers
    */
    errbuf[0] = (double *) DdxCalloc_(dst->UdpL_PxlPerScn+2,sizeof(double));
    errbuf[1] = (double *) DdxCalloc_(dst->UdpL_PxlPerScn+2,sizeof(double));

    if( errbuf[0] == NULL || errbuf[1] == NULL )
	status = BadAlloc;
    else
	DitherProcess( src, dst, errbuf[0], errbuf[1], &curerr );

clean_up :
    if( IsPointer_(errbuf[0]) )
	  DdxFree_(errbuf[0]);
    if( IsPointer_(errbuf[1]) )
	  DdxFree_(errbuf[1]);

    if( IsPointer_(src) && src != srcudp )
	{
	if( IsPointer_(src->UdpA_Base) )
	  DdxFreeBits_(src->UdpA_Base);
	DdxFree_(src);
	}
    if( IsPointer_(dst) && dst != dstudp )
	{
	if( status == Success )
	    status  = DdxConvert_( dst, dstudp, XieK_MoveMode );
	if( IsPointer_(dst->UdpA_Base) )
	  DdxFreeBits_(dst->UdpA_Base);
	DdxFree_(dst);
	}
    return( status );
}					/* end of SmiDither */

/*****************************************************************************
**  DitherProcess
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**
*****************************************************************************/
static void  DitherProcess(src, dst, err0, err1, erridx)
UdpPtr	     src;
UdpPtr	     dst;
double	    *err0;
double	    *err1;
int	    *erridx;
{
    int		     pix, scn, dtypes;
    int		     src_bytes_per_scn, src_bytes_per_pxl;
    int		     dst_bytes_per_scn, dst_bytes_per_pxl;
    double	    *errbuf[2], *curerr, *prverr, nompxlrng, modpxlval;
    unsigned char    BUpxlval, *dstptr, *srcptr;
    unsigned short   WUpxlval;

    errbuf[0] = err0;
    errbuf[1] = err1;

    nompxlrng = (double)(src->UdpL_Levels - 1) / (double)(dst->UdpL_Levels - 1);

    src_bytes_per_scn = src->UdpL_ScnStride / 8;
    dst_bytes_per_scn = dst->UdpL_ScnStride / 8;
    src_bytes_per_pxl = src->UdpL_PxlStride / 8;
    dst_bytes_per_pxl = dst->UdpL_PxlStride / 8;
    /*
    **	Figure out what Dtype combination we have.
    */
    dtypes = src->UdpB_DType == dst->UdpB_DType
	   ? src->UdpB_DType == UdpK_DTypeBU ? 0 /* BytByt */ : 1 /* WrdWrd */
	   : src->UdpB_DType == UdpK_DTypeBU ? 2 /* BytWrd */ : 3 /* WrdByt */;

    for( scn = 0; scn < dst->UdpL_ScnCnt; scn++ )
        {
	curerr = errbuf[*erridx] + 1;
	prverr = errbuf[(*erridx + 1) & 1] + 1;

	srcptr = src->UdpA_Base + src_bytes_per_scn * scn + src->UdpL_Pos / 8;
	dstptr = dst->UdpA_Base + dst_bytes_per_scn * scn + dst->UdpL_Pos / 8;

	switch( dtypes )
	    {					    /* dither a scanline    */
	case 0 : /* BytByt */
	    for( pix = 0;  pix < dst->UdpL_PxlPerScn;  pix++ )
		{
		modpxlval = *((unsigned char *)srcptr) + F7_16 * (*(curerr-1))
						       + F1_16 * (*(prverr-1))
		    				       + F5_16 * (*(prverr))
		    				       + F3_16 * (*(prverr+1));
		BUpxlval  = (modpxlval + nompxlrng/2)/nompxlrng;
		*curerr   =  modpxlval -  (BUpxlval * nompxlrng);
		*((unsigned char *)dstptr) = BUpxlval;
		srcptr   +=  src_bytes_per_pxl;
		dstptr   +=  dst_bytes_per_pxl;
		curerr++;
		prverr++;
		}
	    break;

	case 1 : /* WrdWrd */
	    for( pix = 0;  pix < dst->UdpL_PxlPerScn;  pix++ )
		{
		modpxlval = *((unsigned short *)srcptr) + F7_16 * (*(curerr-1))
						        + F1_16 * (*(prverr-1))
		    				        + F5_16 * (*(prverr))
		    				        + F3_16 * (*(prverr+1));
		WUpxlval  = (modpxlval + nompxlrng/2)/nompxlrng;
		*curerr   =  modpxlval -  (WUpxlval * nompxlrng);
		*((unsigned short *)dstptr) = WUpxlval;
		srcptr   +=  src_bytes_per_pxl;
		dstptr   +=  dst_bytes_per_pxl;
		curerr++;
		prverr++;
		}
	    break;

	case 2 : /* BytWrd */
	    for( pix = 0;  pix < dst->UdpL_PxlPerScn;  pix++ )
		{
		modpxlval = *((unsigned char *)srcptr) + F7_16 * (*(curerr-1))
						       + F1_16 * (*(prverr-1))
		    				       + F5_16 * (*(prverr))
		    				       + F3_16 * (*(prverr+1));
		WUpxlval  = (modpxlval + nompxlrng/2)/nompxlrng;
		*curerr   =  modpxlval -  (WUpxlval * nompxlrng);
		*((unsigned short *)dstptr) = WUpxlval;
		srcptr   +=  src_bytes_per_pxl;
		dstptr   +=  dst_bytes_per_pxl;
		curerr++;
		prverr++;
		}
	    break;

	case 3 : /* WrdByt */
	    for( pix = 0;  pix < dst->UdpL_PxlPerScn;  pix++ )
		{
		modpxlval = *((unsigned short *)srcptr) + F7_16 * (*(curerr-1))
						        + F1_16 * (*(prverr-1))
		    				        + F5_16 * (*(prverr))
		    				        + F3_16 * (*(prverr+1));
		BUpxlval  = (modpxlval + nompxlrng/2)/nompxlrng;
		*curerr   =  modpxlval -  (BUpxlval * nompxlrng);
		*((unsigned char *)dstptr) = BUpxlval;
		srcptr   +=  src_bytes_per_pxl;
		dstptr   +=  dst_bytes_per_pxl;
		curerr++;
		prverr++;
		}
	    }
	*erridx = (*erridx + 1) & 1;
	}
}					/* end of DitherProcess */
/* end of module SmiDither.c */
