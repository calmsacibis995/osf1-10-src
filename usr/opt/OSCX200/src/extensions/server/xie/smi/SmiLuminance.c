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
**      X Imaging Extension
**	Sample Machine Independant DDX
**
**  ABSTRACT:
**
**      This module contains support for the luminance operator.
**
**  ENVIRONMENT:
**
**	VAX/VMS V5.3
**	ULTRIX  V3.1
**
**  AUTHOR(S):
**
**      John Weber
**
**  CREATION DATE:
**
**      May 15, 1989
**
**  MODIFICATION HISTORY:
**
*******************************************************************************/

#include <stdio.h>
#include <math.h>

#include "SmiLuminance.h"

/*
**  Table of contents
*/
int 		SmiLuminance();
int             SmiCreateLuminance();

static int 	_LuminanceInitialize();
static int 	_LuminanceActivate();
static int 	_LuminanceDestroy();

static UdpPtr	GetBytUdp();
static UdpPtr	GetFltUdp();
static int	GetFltScn();

static void	FreeUdp();

static void	LuminanceProcess();
/*
**  External References
*/

/*
**  Local storage
*/
    /*
    **  Luminance Element Vector
    */
static PipeElementVector LuminancePipeElement = {
    NULL,				/* FLINK			    */
    NULL,				/* BLINK			    */
    sizeof(PipeElementVector),		/* Size				    */
    TypePipeElementVector,		/* Structure type		    */
    StypeLuminanceElement,		/* Structure subtype		    */
    sizeof(LuminancePipeCtx),		/* Context block size		    */
    0,					/* No input flag		    */
    0,					/* Reserved flags		    */
    _LuminanceInitialize,		/* Initialize entry		    */
    _LuminanceActivate,			/* Activate entry		    */
    NULL,				/* Flush entry			    */
    _LuminanceDestroy,			/* Destroy entry		    */
    NULL				/* Abort entry point		    */
    };
/* RGB luminance weights */

#define RED_WEIGHT 0.299
#define GRN_WEIGHT 0.587
#define BLU_WEIGHT 0.114

/*****************************************************************************
**  SmiCreateLuminance
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function initializes the LUMINANCE pipeline element
**
**  FORMAL PARAMETERS:
**
**      ctx - luminance pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
int 	    SmiCreateLuminance( pipe, srcsnk, dstsnk )
Pipe		     pipe;
PipeSinkPtr	     srcsnk;
PipeSinkPtr	     dstsnk;
{
    LuminancePipeCtxPtr ctx = (LuminancePipeCtxPtr)
			DdxCreatePipeCtx_( pipe, &LuminancePipeElement, FALSE );
    if( !IsPointer_(ctx) ) return( (int) ctx );
    /*
    **	Copy parameters into context struct
    */
    LumSrcSnk_(ctx) = srcsnk;
    LumDstSnk_(ctx) = dstsnk;

    /* Let the resource manager convert the data to floats */
    LumRedDrn_(ctx) = DdxRmCreateDrain_( LumSrcSnk_(ctx), 1 << 0 );
    if( !IsPointer_(LumRedDrn_(ctx)) ) return( (int) LumRedDrn_(ctx) );
    DdxRmSetDType_( LumRedDrn_(ctx), UdpK_DTypeF, DtM_F );
    DdxRmSetQuantum_( LumRedDrn_(ctx), 1 );

    LumGrnDrn_(ctx) = DdxRmCreateDrain_( LumSrcSnk_(ctx), 1 << 1 );
    if( !IsPointer_(LumGrnDrn_(ctx)) ) return( (int) LumGrnDrn_(ctx) );
    DdxRmSetDType_( LumGrnDrn_(ctx), UdpK_DTypeF, DtM_F );
    DdxRmSetQuantum_( LumGrnDrn_(ctx), 1 );

    LumBluDrn_(ctx) = DdxRmCreateDrain_( LumSrcSnk_(ctx), 1 << 2 );
    if( !IsPointer_(LumBluDrn_(ctx)) ) return( (int) LumBluDrn_(ctx) );
    DdxRmSetDType_( LumBluDrn_(ctx), UdpK_DTypeF, DtM_F );
    DdxRmSetQuantum_( LumBluDrn_(ctx), 1 );

    /* Output bytes */
    DdxRmSetDType_( LumDstSnk_(ctx), UdpK_DTypeBU, DtM_BU );
    DdxRmSetQuantum_( LumDstSnk_(ctx), 1 );

    return( Success );
}					/* end SmiCreateLuminance */

/*****************************************************************************
**  _LuminanceInitialize
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function initializes the LUMINANCE pipeline element
**
**  FORMAL PARAMETERS:
**
**      ctx - luminance pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 		_LuminanceInitialize( ctx )
LuminancePipeCtxPtr	ctx;
{
    int status  = DdxRmInitializePort_( CtxHead_(ctx), LumRedDrn_(ctx) );
    if( status == Success )
	status  = DdxRmInitializePort_( CtxHead_(ctx), LumGrnDrn_(ctx) );
    if( status == Success )
	status  = DdxRmInitializePort_( CtxHead_(ctx), LumBluDrn_(ctx) );
    if( status == Success )
	status  = DdxRmInitializePort_( CtxHead_(ctx), LumDstSnk_(ctx) );
    if( status == Success )
	{
	/* Adjust the RGB weight factors by the relative ranges of the
	 * components. */
	LumWeight_(ctx,0) = RED_WEIGHT / (SnkLvl_(LumSrcSnk_(ctx),0) - 1);
	LumWeight_(ctx,1) = GRN_WEIGHT / (SnkLvl_(LumSrcSnk_(ctx),1) - 1);
	LumWeight_(ctx,2) = BLU_WEIGHT / (SnkLvl_(LumSrcSnk_(ctx),2) - 1);

	CtxInp_(ctx) = LumRedDrn_(ctx);
	}
    return( status );
}

/*****************************************************************************
**  _LuminanceActivate
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine controls luminance generation
**
**  FORMAL PARAMETERS:
**
**      ctx - luminance pipeline element context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _LuminanceActivate( ctx )
LuminancePipeCtxPtr ctx;
{
    PipeDataPtr	 dst = NULL;
    int	status;
    
    for( status = Success; status == Success;
			   LumRedDat_(ctx) = DdxRmDeallocData_(LumRedDat_(ctx)),
			   LumGrnDat_(ctx) = DdxRmDeallocData_(LumGrnDat_(ctx)),
			   LumBluDat_(ctx) = DdxRmDeallocData_(LumBluDat_(ctx)))
	{
	if( LumRedDat_(ctx) == NULL )
	    {
	    LumRedDat_(ctx)  = DdxRmGetData_( ctx, LumRedDrn_(ctx) );
	    if( !IsPointer_(LumRedDat_(ctx)) ) return( (int) LumRedDat_(ctx) );
	    }
	if (LumGrnDat_(ctx) == NULL) 
	    {
	    LumGrnDat_(ctx)  = DdxRmGetData_( ctx, LumGrnDrn_(ctx) );
	    if( !IsPointer_(LumGrnDat_(ctx)) ) return( (int) LumGrnDat_(ctx) );
	    }
	if (LumBluDat_(ctx) == NULL) 
	    {
	    LumBluDat_(ctx)  = DdxRmGetData_( ctx, LumBluDrn_(ctx) );
	    if( !IsPointer_(LumBluDat_(ctx)) ) return( (int) LumBluDat_(ctx) );
	    }
	dst = DdxRmAllocData_( LumDstSnk_(ctx), 0, DatY1_(LumRedDat_(ctx)),
			       DatHeight_(LumRedDat_(ctx)) );
	status = (int) dst;
	if( !IsPointer_(dst) ) continue;

	LuminanceProcess(DatUdpPtr_(LumRedDat_(ctx)),
			 DatUdpPtr_(LumGrnDat_(ctx)),
			 DatUdpPtr_(LumBluDat_(ctx)),
			 LumWeightPtr_(ctx),
			 DatUdpPtr_(dst));

	status = DdxRmPutData_( LumDstSnk_(ctx), dst );
	}

    return( status );
}

/*****************************************************************************
**  _LuminanceDestroy
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine deallocates all resources allocated by the luminance
**	pipeline element.
**
**  FORMAL PARAMETERS:
**
**      ctx - luminance pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _LuminanceDestroy( ctx )
LuminancePipeCtxPtr    ctx;
{
    LumRedDrn_(ctx) = DdxRmDestroyDrain_( LumRedDrn_(ctx) );
    LumGrnDrn_(ctx) = DdxRmDestroyDrain_( LumGrnDrn_(ctx) );
    LumBluDrn_(ctx) = DdxRmDestroyDrain_( LumBluDrn_(ctx) );

    return( Success );
}

/*****************************************************************************
**  SmiLuminance
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
int SmiLuminance( redudp, grnudp, bluudp, dstudp )
UdpPtr  redudp;
UdpPtr  grnudp;
UdpPtr  bluudp;
UdpPtr  dstudp;
{
    UdpPtr  red = NULL, grn = NULL, blu = NULL, dst = NULL;
    int status, scanline;
    float weight[XieK_MaxComponents];

    /* Adjust the RGB weight factors by the relative ranges of the
     * components.
     */
    weight[0] = RED_WEIGHT / (redudp->UdpL_Levels - 1);
    weight[1] = GRN_WEIGHT / (grnudp->UdpL_Levels - 1);
    weight[2] = BLU_WEIGHT / (bluudp->UdpL_Levels - 1);

    if( dstudp->UdpA_Base == NULL )
	{
	dstudp->UdpA_Base  = DdxMallocBits_( dstudp->UdpL_ArSize );
	if( dstudp->UdpA_Base == NULL ) return( BadAlloc );
	}
    /*
    **	Allocate temporary Udps for desired datatype
    */
    red = GetFltUdp(redudp);
    if( !IsPointer_(red) )
	{
	status = (int) red;
	goto clean_up;
	}
    grn = GetFltUdp(grnudp);
    if( !IsPointer_(grn) )
	{
	status = (int) grn;
	goto clean_up;
	}
    blu = GetFltUdp(bluudp);
    if( !IsPointer_(blu) )
	{
	status = (int) blu;
	goto clean_up;
	}
    dst = GetBytUdp(dstudp);
    if( !IsPointer_(dst) )
	{
	status = (int) dst;
	goto clean_up;
	}

    for( scanline = 0; scanline < dstudp->UdpL_ScnCnt; scanline++ )
	{
	status = GetFltScn(redudp,scanline,red);
	if( status != Success ) goto clean_up;
	status = GetFltScn(grnudp,scanline,grn);
	if( status != Success ) goto clean_up;
	status = GetFltScn(bluudp,scanline,blu);
	if( status != Success ) goto clean_up;

	dst->UdpL_Y1 = dst->UdpL_Y2 = scanline;
	if( dstudp->UdpA_Base == dst->UdpA_Base )
	  dst->UdpL_Pos = dstudp->UdpL_Pos + dstudp->UdpL_ScnStride * scanline;

	LuminanceProcess(red,grn,blu,weight,dst);
	/*
	**  If a temporary destination buffer was allocated, do the scanline
	**  conversion now and reuse the temporary buffer for the next scanline.
	**
	**  Otherwise, set the destination pointer to the next scanline in
	**  the destination UDP.
	*/
	if( dst->UdpA_Base != dstudp->UdpA_Base )
	    {
	    status = DdxConvert_( dst, dstudp, XieK_MoveMode );
	    if( status != Success ) goto clean_up;
	    dst->UdpL_Y1++;
	    dst->UdpL_Y2++;
	    }
	else
	    dst->UdpL_Pos = 
		    dstudp->UdpL_Pos + dstudp->UdpL_ScnStride * (scanline + 1);
	}
clean_up :
    FreeUdp(red, redudp);
    FreeUdp(grn, grnudp);
    FreeUdp(blu, bluudp);
    FreeUdp(dst, dstudp);

    return( status );
}

/*****************************************************************************
**  LuminanceProcess
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**
*****************************************************************************/
static void	LuminanceProcess( red, grn, blu, weight, dst )
UdpPtr		red;
UdpPtr		grn;
UdpPtr		blu;
float           weight[XieK_MaxComponents];
UdpPtr		dst;
{
    int pixel, scanline;

    unsigned char *redptr, *grnptr, *bluptr, *dstptr;
    float value, dst_levels;

    int red_bytes_per_pel = red->UdpL_PxlStride / 8;
    int grn_bytes_per_pel = grn->UdpL_PxlStride / 8;
    int blu_bytes_per_pel = blu->UdpL_PxlStride / 8;

    int dst_bytes_per_scn = dst->UdpL_ScnStride / 8;
    int dst_bytes_per_pel = dst->UdpL_PxlStride / 8;
    int dst_bytes_pos     = dst->UdpL_Pos / 8;

    dst_levels = (float)(dst->UdpL_Levels - 1);

    for (scanline = 0; scanline < dst->UdpL_ScnCnt; scanline++)
	{
	redptr = red->UdpA_Base + red->UdpL_Pos / 8;
	grnptr = grn->UdpA_Base + grn->UdpL_Pos / 8;
	bluptr = blu->UdpA_Base + blu->UdpL_Pos / 8;
	dstptr = dst->UdpA_Base + dst->UdpL_Pos / 8;

	for( pixel = 0;  pixel < dst->UdpL_PxlPerScn;  pixel++ )
	    {
	    value = *(float *)redptr * weight[0] +
		    *(float *)grnptr * weight[1] +
		    *(float *)bluptr * weight[2];

	    
	    *(char *)dstptr = value * dst_levels;

	    redptr += red_bytes_per_pel;
	    grnptr += grn_bytes_per_pel;
	    bluptr += blu_bytes_per_pel;
	    dstptr += dst_bytes_per_pel;
	    }
	}
}

static UdpPtr GetFltUdp( src )
UdpPtr src;
{
    UdpPtr dst = (UdpPtr) DdxMalloc_(sizeof(UdpRec));
    if( dst == NULL ) return( (UdpPtr) BadAlloc );

    if (src->UdpB_DType != UdpK_DTypeF)
	{
	dst->UdpW_PixelLength	= sizeof(float) * 8;
	dst->UdpB_DType		= UdpK_DTypeF;
	dst->UdpB_Class		= UdpK_ClassA;
	dst->UdpL_X1		= src->UdpL_X1;
	dst->UdpL_X2		= src->UdpL_X2;
	dst->UdpL_Y1		= src->UdpL_Y1;
	dst->UdpL_Y2		= src->UdpL_Y1;
	dst->UdpL_PxlPerScn	= src->UdpL_PxlPerScn;
	dst->UdpL_ScnCnt	= 1;
	dst->UdpL_PxlStride	= dst->UdpW_PixelLength;
	dst->UdpL_ScnStride	= dst->UdpL_PxlStride * dst->UdpL_PxlPerScn;
	dst->UdpL_ArSize	= dst->UdpL_ScnStride;
	dst->UdpA_Base		= DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL ) return( DdxFree_(dst), (UdpPtr) BadAlloc );
	dst->UdpL_Pos		= 0;
	dst->UdpL_CompIdx	= src->UdpL_CompIdx;
	dst->UdpL_Levels	= src->UdpL_Levels;
	}
    else
	{
	*dst = *src;
	dst->UdpL_Y2	 = src->UdpL_Y1;
	dst->UdpL_ScnCnt = 1;
	}
    return( dst );
}

static UdpPtr GetBytUdp( src )
UdpPtr src;
{
    UdpPtr dst = (UdpPtr) DdxMalloc_(sizeof(UdpRec));
    if( dst == NULL ) return( (UdpPtr) BadAlloc );

    if (src->UdpB_DType != UdpK_DTypeBU)
	{
	dst->UdpW_PixelLength	= sizeof(char) * 8;
	dst->UdpB_DType		= UdpK_DTypeBU;
	dst->UdpB_Class		= UdpK_ClassA;
	dst->UdpL_X1		= src->UdpL_X1;
	dst->UdpL_X2		= src->UdpL_X2;
	dst->UdpL_Y1		= src->UdpL_Y1;
	dst->UdpL_Y2		= src->UdpL_Y1;
	dst->UdpL_PxlPerScn	= src->UdpL_PxlPerScn;
	dst->UdpL_ScnCnt	= 1;
	dst->UdpL_PxlStride	= dst->UdpW_PixelLength;
	dst->UdpL_ScnStride	= dst->UdpL_PxlStride * dst->UdpL_PxlPerScn;
	dst->UdpL_ArSize	= dst->UdpL_ScnStride;
	dst->UdpA_Base		= DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL ) return( DdxFree_(dst), (UdpPtr) BadAlloc );
	dst->UdpL_Pos		= 0;
	dst->UdpL_CompIdx	= src->UdpL_CompIdx;
	dst->UdpL_Levels	= src->UdpL_Levels;
	}
    else
	{
	*dst = *src;
	dst->UdpL_Y2	 = src->UdpL_Y1;
	dst->UdpL_ScnCnt = 1;
	}

    return( dst );
}

static void FreeUdp( udp, realudp )
UdpPtr udp, realudp;
{
    if( IsPointer_(udp) )
	{
	if( udp->UdpA_Base != NULL  &&  udp->UdpA_Base != realudp->UdpA_Base )
	    DdxFreeBits_( udp->UdpA_Base );

	DdxFree_( udp );
	}
}

/*
**  This routine will setup a UDP of one scanline of floating point data for
**  a given scanline.
*/
static int GetFltScn( src, scn, dst )
UdpPtr	src;
int	scn;
UdpPtr	dst;
{
    int status = Success;
    UdpPtr tmp;

    dst->UdpL_Y1 = dst->UdpL_Y2 = scn;

    if( src->UdpB_DType != UdpK_DTypeF )
	status = DdxConvert_( src, dst, XieK_MoveMode );
    else
	dst->UdpL_Pos =	src->UdpL_Pos + src->UdpL_ScnStride * scn;

    return( status );
}
