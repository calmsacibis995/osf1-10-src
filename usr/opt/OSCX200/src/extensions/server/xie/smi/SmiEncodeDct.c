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
Copyright 1990 by Digital Equipment Corporation, Maynard, Massachusetts,
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
**      This module contains the interface to the DCT compressesion routines.
**
**  ENVIRONMENT:
**
**	VAX/VMS V5.3
**	ULTRIX  V3.1
**
**  AUTHORS:
**
**	John Nadai
**	John Weber
**
**  CREATION DATE:
**
**	April 5, 1990
**
*******************************************************************************/

/*
**  INCLUDE FILES
*/
#include <stdio.h>
#include <string.h>

#if	defined(XIESMI)
#include "SmiEncodeDct.h"
#include "SmiDctMacros.h"
#elif	defined(IPS)
#include <math.h>
#include <IpsDef.h>
#include <IpsStatusCodes.h>
#include <IpsDctDef.h>
#include <IpsDctMacros.h>
#include <IpsEncodeDct.h>
#endif

/*
**  Table of Contents
*/
#if	defined(XIESMI)	
unsigned long		    SmiEncodeDct();	    /* External XIE entries */
#elif	defined(IPS)
long			   _IpsEncodeDct();	    /* External IPS entries */
#endif

#if defined(XIESMI)
int 			    SmiCreateEncodeDct();
static int 		   _EncodeDctInitialize();
static int 		   _EncodeDctActivate();
static int 		   _EncodeDctFlush();
static int 		   _EncodeDctAbort();
static int 		   _EncodeDctDestroy();
static int 		   _EncodeDctFreeData();
#endif

static unsigned long	    DceEncodeSegment();
static EncodeDctStatePtr    DceInitializeState();
static EncodeDctStatePtr    DceDestroyState();

static int		    DceNextToken();
static int		    DceEncodeData();

static int		    DceLenDat();
static int		    DceLenRSC();
static int		    DceLenSOI();
static int		    DceLenSOF();
static int		    DceLenSOS();
static int		    DceLenEOI();

static int		    DceActDat();
static int		    DceActRSC();
static int		    DceActSOI();
static int		    DceActSOF();
static int		    DceActSOS();
static int		    DceActEOI();

static void		    DcePadData();
static void		    Dce8x8_8U_16S() ;

static int		    DctCreateScaledQuant();
static int		    DctRstEnable();
static int		    DctEncode();
static int		    DctPutBits();

/*
**  Local MACROS
*/
    /*
    **	Put a 16 bit value into the code stream, LSB first.
    */
#define	PutShort_(ptr,value) \
    *(ptr)++ =  (value) & 0xFF; \
    *(ptr)++ = ((value) >> 8) &0xFF;

/*  ZIGZAG_QUANT MACRO

       Inputs:  > 16-bit signed, UNquantized DCT coefficients
                > organized as sequence of blocks (within blocks, the
                    coefficients stored by rows)

       Outputs: > 16-bit signed, quantized DCT coefficients 
                > organized as sequence of blocks (within blocks, the
                    coefficients stored in zigzag order)

*/

#define ZQ_VARIABLES \
\
double d_dtemp ;\
short int *d_in_ptr,*d_out_ptr,*d_z_ptr;\
unsigned char *d_q_ptr ;\
int d_i ;

#define ZIGZAG_QUANT(IN_PT,OUT_PT,Z_PT,Q_PT) \
            d_in_ptr  = (short int *)(IN_PT) ;\
            d_out_ptr = (short int *)(OUT_PT) ;\
            d_q_ptr   = (unsigned char *)(Q_PT) ;\
            d_z_ptr   = (short int *)(Z_PT) ;\
	    for( d_i=0 ; d_i<=63 ; d_i++ )\
	    {\
	      d_dtemp = \
               (double)(*(d_in_ptr+(*d_z_ptr++)))/(double)(*d_q_ptr++);\
    	      *d_out_ptr++ = \
               (d_dtemp<0.) ? (int)(d_dtemp-.5) : (int)(d_dtemp+.5) ;\
	    }

/*
**  Static storage
*/
    /*
    **  EncodeDct Element Vector
    */
#if defined(XIESMI)
static PipeElementVector EncodeDctPipeElement = {
    NULL,				/* FLINK			    */
    NULL,				/* BLINK			    */
    sizeof(PipeElementVector),		/* Size				    */
    TypePipeElementVector,		/* Structure type		    */
    StypeEncodeDctElement,		/* Structure subtype		    */
    sizeof(EncodeDctPipeCtx),		/* Context block size		    */
    0,					/* No input flag		    */
    0,					/* Reserved flags		    */
    _EncodeDctInitialize,		/* Initialize entry		    */
    _EncodeDctActivate,			/* Activate entry		    */
    _EncodeDctFlush,			/* Flush entry			    */
    _EncodeDctDestroy,			/* Destroy entry		    */
    _EncodeDctAbort			/* Abort entry			    */
    };
#endif

    /*
    **	Token length table.
    */
static int (*token_length[DctK_TokenMax])() = {
    NULL,					    /* None */
    DceLenSOI,					    /* SOI */
    DceLenSOF,					    /* SOF */
    DceLenSOS,					    /* SOS */
    DceLenDat,					    /* Data */
    DceLenRSC,					    /* RSC */
    DceLenEOI,					    /* EOI */
    NULL					    /* Error */
};
static int (*token_action[DctK_TokenMax])() = {
    NULL,					    /* None */
    DceActSOI,					    /* SOI */
    DceActSOF,					    /* SOF */
    DceActSOS,					    /* SOS */
    DceActDat,					    /* Data */
    DceActRSC,					    /* RSC */
    DceActEOI,					    /* EOI */
    NULL					    /* Error */
};

/*****************************************************************************
**  SmiEncodeDct
**
**  FUNCTIONAL DESCRIPTION:
**
**      Image data is encoded using the DCT data compression scheme.
**
**  FORMAL PARAMETERS:
**
**      udplst     - pointer to image data to be compressed.
**	udpcnt	   - number of components in udplst.
**      cdp	   - pointer to resulting compressed image data plane.
**	factor	   - compression factor to be used to encode image.
**	resync	   - enable generation of resync markers.
**	size_ret   - where to return size of compressed data
**
**  FUNCTION VALUE:
**
**	status
**
*****************************************************************************/
#if	defined(XIESMI)
unsigned long	 SmiEncodeDct(udplst,udpcnt,cdp,factor,resync,size_ret)
#elif	defined(IPS)
long		_IpsEncodeDct(udplst,udpcnt,cdp,factor,resync,size_ret)
#endif
UdpPtr		 udplst[];
int		 udpcnt;
UdpPtr		 cdp;
unsigned long	 factor;
unsigned long	 resync;
int		*size_ret;
{
    long    status;
    long    bytcnt;
    long    output_bytcnt;
    long    i,j,k;
    long    width;
    long    height;
    long    mem_alloc = FALSE;
    EncodeDctStatePtr	state;

    *size_ret = 0;
    /*
    **	Check DCT compression factor.
    */
    if( factor < 5 || factor > 200 )
	return( DctX_BadFactor );
    /*
    **	Check input UDP fields
    **	Calculate worst case compressed data buffer size.
    */
    for( output_bytcnt = 0, i = 0; i < udpcnt; i++ )
	{
	if( udplst[i]->UdpB_Class != UdpK_ClassA ||
	    udplst[i]->UdpB_DType != UdpK_DTypeBU )
	    return( DctX_UnsOption );
    	width  = (udplst[i]->UdpL_PxlPerScn + 7) & ~7;
    	height = (udplst[i]->UdpL_ScnCnt    + 7) & ~7;
	output_bytcnt += width * height * (20.0 / (float)factor) + 3200;
	}
#if defined(IPS)
    if( cdp->UdpA_Base != NULL && cdp->UdpL_ArSize / 8 < output_bytcnt )
	return( DctX_OutBufSiz );
#endif
    /*
    **	If error during initialization, return to caller.
    */
    state = DceInitializeState( udplst, udpcnt );
    if( state == NULL ) return( DctX_BadAlloc );
    /*
    **	Check CDP...if buffer not allocated, allocate a worst case size.
    */
    if( cdp->UdpA_Base == NULL )
    {
#if defined(IPS)
       *cdp = *udplst[0];
	cdp->UdpL_Pos = 0;
#endif
    	cdp->UdpL_ArSize = output_bytcnt * 8;
	cdp->UdpA_Base   = (unsigned char *) DctMallocBits_(output_bytcnt*8);
	if( cdp->UdpA_Base == NULL )
	    {
	    DceDestroyState(state);
	    return DctX_BadAlloc;
	    }
	mem_alloc = TRUE;
    }
#if defined(IPS)
    else
    {
	i = (long)cdp->UdpL_ArSize;
	j = (long)cdp->UdpA_Base;
	k = (long)cdp->UdpL_Pos;
	*cdp = *udplst[0];
	/* Adjust fields */
	cdp->UdpL_ArSize = (unsigned long)i;
	cdp->UdpA_Base   = (unsigned char *)j;
	cdp->UdpL_Pos    = (unsigned long)k;
    }
    cdp->UdpB_Class = UdpK_ClassUBS;
    cdp->UdpB_DType = UdpK_DTypeVU;
    cdp->UdpL_X1    = 0;
    cdp->UdpL_Y1    = 0;
    cdp->UdpL_X2    = cdp->UdpL_PxlPerScn - 1;
    cdp->UdpL_Y2    = cdp->UdpL_ScnCnt - 1;
#endif
    /*
    **	Setup custom quantization tables based on default quantization tables
    **	scaled by "factor".
    */
    if( (status = DctCreateScaledQuant(state,0,0,factor)) != DctX_Success )
	{
	DceDestroyState(state);
	return status;
	}
    /*
    **	Enable resync codes if requested.
    */
    if( resync ) 
	DctRstEnable(state);
    /*
    **	Setup destination buffer.
    */
    DceCurPtr_(state) = cdp->UdpA_Base;
    DceCurLen_(state) = cdp->UdpL_ArSize / 8;
    DceCurRem_(state) = DceCurLen_(state);
    /*
    **	Encode the data.
    */
    status = DceEncodeSegment( state, udplst, udpcnt );
    bytcnt = DceCurLen_(state) - DceCurRem_(state);

    DceDestroyState(state);

    if( status != DctK_StatusDone )
	{
#if	defined(XIESMI)
	if( status > DctK_StatusDone )
	    return( status );
	else if( status == DctK_StatusOutput )
	    return( DctX_Success );
	else
#endif
	return( DctX_EncodeFail );
	}
    /* 
    **  reallocate memory if allocated from inside...
    */
    if( mem_alloc )	
	{
	cdp->UdpL_ArSize = bytcnt * 8;
	cdp->UdpA_Base   = DctReallocBits_(cdp->UdpA_Base, cdp->UdpL_ArSize);
	}
    *size_ret = bytcnt * 8;

    return DctX_Success;
}

/*****************************************************************************
**  SmiCreateEncodeDct
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function creates the ENCODE DCT pipeline element
**
**  FORMAL PARAMETERS:
**
**      pipe	    - pipeline pointer
**      srcsnk      - the sink from which to read PCM data
**	cmpcnt	    - number of components in sink.
**	dstudp      - pointer to destination Udps for DCT data
**	factor	    - compression factor
**	final	    - pointer to flag to set when finished
**
*****************************************************************************/
#if defined(XIESMI)
int SmiCreateEncodeDct(pipe,srcsnk,cmpmsk,dstudp,factor,final)
 Pipe		 pipe;
 PipeSinkPtr	 srcsnk;
 int		 cmpmsk;
 UdpPtr		 dstudp;
 int		 factor;
 unsigned int	*final;
{
    int i;
    /*
    **	Create pipe element context block.
    */
    EncodeDctPipeCtxPtr ctx = (EncodeDctPipeCtxPtr)
			DdxCreatePipeCtx_( pipe, &EncodeDctPipeElement, FALSE );
    if( !IsPointer_(ctx) ) return( (int) ctx );
    /*
    **  Build our EncodeDct Pipe Ctx from the arguments passed in.
    */
    DceSrcSnk_(ctx) = srcsnk;
    DceDstUdp_(ctx) = dstudp;
    DceFinal_(ctx)  = final;
    DceCmpMsk_(ctx) = cmpmsk;
    DceCmpFct_(ctx) = factor;
    *final &= ~(cmpmsk);
    /*
    **  Create drains.
    */
    for (i = 0;  i < XieK_MaxComponents ;  i++)
	if( 1<<i & cmpmsk )
	    {
	    DceSrcDrn_(ctx,i) = DdxRmCreateDrain_( srcsnk, 1 << i );
	    if( !IsPointer_(DceSrcDrn_(ctx,i)) )
		return( (int) DceSrcDrn_(ctx,i) );
	    DdxRmSetDType_( DceSrcDrn_(ctx,i), UdpK_DTypeBU, DtM_BU );
	    DdxRmSetQuantum_( DceSrcDrn_(ctx,i), DctK_BlockSize );
	    }
    return( DctX_Success );
}                               /* end SmiCreateEncodeDct */
#endif

/*****************************************************************************
**  _EncodeDctInitialize
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function initializes the ENCODE DCT pipeline element
**
**  FORMAL PARAMETERS:
**
**      ctx - encode DCT pipeline context
**
*****************************************************************************/
#if defined(XIESMI)
static int _EncodeDctInitialize(ctx)
 EncodeDctPipeCtxPtr  ctx;
{
    int  i, status, udpcnt, width, height, bytcnt;
    UdpPtr  udplst[XieK_MaxComponents];

   *DceFinal_(ctx) &= ~(DceCmpMsk_(ctx));

    width  = SnkWidth_(DceSrcSnk_(ctx),0)  + 7 & ~7;
    height = SnkHeight_(DceSrcSnk_(ctx),0) + 7 & ~7;
    bytcnt = 0;
    for( CtxInp_(ctx) = NULL, i = 0, udpcnt = 0; i < XieK_MaxComponents; i++ )
	if( DceSrcDrn_(ctx,i) != NULL )
	    {
	    status = DdxRmInitializePort_( CtxHead_(ctx), DceSrcDrn_(ctx,i) );
	    if( status != DctX_Success ) return( status );
	    if (CtxInp_(ctx) == NULL)
		CtxInp_(ctx) = DceSrcDrn_(ctx,i);
	    udplst[udpcnt++] = SnkUdpPtr_(DceSrcSnk_(ctx),i);
	    bytcnt += width * height * (20.0 / DceCmpFct_(ctx)) + 3200.0;
	    }
    /*
    **	Allocate DCT state block
    */
    DceSteBlk_(ctx) = DceInitializeState( udplst, udpcnt );
    if( DceSteBlk_(ctx) == NULL ) return( DctX_BadAlloc );
    /*
    **	Setup custom quantization tables based on default quantization tables
    **	scaled by "factor".
    */
    status = DctCreateScaledQuant( DceSteBlk_(ctx), 0, 0, DceCmpFct_(ctx) );
    if( status != DctX_Success )  return( status );
    /*
    **	Enable resync codes.
    */
    DctRstEnable( DceSteBlk_(ctx) );
    /*
    **	Initialize buffer pointers and lengths
    */
    DceDstUdp_(ctx)->UdpL_ArSize = 0;
    DceCurLen_(DceSteBlk_(ctx))  = bytcnt - DceDstUdp_(ctx)->UdpL_Pos / 8;
    DceCurRem_(DceSteBlk_(ctx))  = DceCurLen_(DceSteBlk_(ctx));
    DceCurPtr_(DceSteBlk_(ctx))  = DceDstUdp_(ctx)->UdpA_Base
				 + DceDstUdp_(ctx)->UdpL_Pos / 8;

    return( DctX_Success );
}                               /* end _EncodeDctInitialize */
#endif

/*****************************************************************************
**  _EncodeDctActivate
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine performs DCT compression on a segment of the image.
**
**  FORMAL PARAMETERS:
**
**      ctx - encode DCT pipeline element context
**
*****************************************************************************/
#if defined(XIESMI)
static int _EncodeDctActivate(ctx)
 EncodeDctPipeCtxPtr  ctx;
{
    UdpPtr  dst = DceDstUdp_(ctx);
    UdpPtr  udplst[XieK_MaxComponents];
    int	    udpcnt;
    int	    i;
    int	    status;
    int	    bits;

    if( DceDstUdp_(ctx)->UdpA_Base == NULL )
	{
	bits = DceDstUdp_(ctx)->UdpL_Pos + DceCurRem_(DceSteBlk_(ctx)) * 8;
    	DceDstUdp_(ctx)->UdpA_Base = DctMallocBits_(bits);
	if( DceDstUdp_(ctx)->UdpA_Base == NULL ) return( DctX_BadAlloc );
	DceCurPtr_(DceSteBlk_(ctx)) = DceDstUdp_(ctx)->UdpA_Base
				    + DceDstUdp_(ctx)->UdpL_Pos / 8;
	}

    for(status = DctX_Success; status == DctX_Success; _EncodeDctFreeData(ctx))
	{
	for( i = 0, udpcnt = 0;  i < XieK_MaxComponents;  i++ )
	    /*
	    **  Read a quantum worth of data on each drain.
	    */
	    if( 1<<i & DceCmpMsk_(ctx) )
		{
		if( DceSrcDat_(ctx,i) == NULL )
		    {
		    DceSrcDat_(ctx,i) = DdxRmGetData_( ctx, DceSrcDrn_(ctx,i) );
		    if( !IsPointer_(DceSrcDat_(ctx,i)) )
			return( (int) DceSrcDat_(ctx,i) );
		    }
		udplst[udpcnt++] = DatUdpPtr_(DceSrcDat_(ctx,i));
		}
	/*
	**  If the final flags have not been set, continue to decompress data.
	*/
	if( !(*DceFinal_(ctx) & DceCmpMsk_(ctx)) )
	    {	/*
		**  Encode the data.
		*/
	    status = DceEncodeSegment( DceSteBlk_(ctx), udplst, udpcnt );
	    /*
	    **  Dispatch based on results of encoding.
	    */
	    switch( status )
		{   /*
		    **	If done status, set done flags.  In either case
		    **	update size of output buffer.
		    */
	    case DctK_StatusDone:
		*DceFinal_(ctx) |= DceCmpMsk_(ctx);

	    case DctK_StatusInput:
		upArSize_(dst) += (DceCurLen_(DceSteBlk_(ctx)) - 
				   DceCurRem_(DceSteBlk_(ctx))) * 8;
		DceCurLen_(DceSteBlk_(ctx)) = DceCurRem_(DceSteBlk_(ctx));
		status = DctX_Success;
		break;

	    case DctK_StatusOutput:
		status = DctX_Success;
	    case DctK_StatusError:
	    default:	/*
			**  Data couldn't fit in allocated output buffer or
			**  error...return image didn't compress.
			*/
		upArSize_(dst)	= 0;
		upPos_(dst)	= 0;
	       *DceFinal_(ctx) |= DceCmpMsk_(ctx);
		if( status != DctX_Success )
		    status  = DctX_EncodeFail;
		}
	    }
	}
    return( status );
}                               /* end _EncodeDctActivate */
#endif

/*****************************************************************************
**  _EncodeDctFlush
**
**  FUNCTIONAL DESCRIPTION:
**
**      Process any data remaining in the pipeline.
**
**  FORMAL PARAMETERS:
**
**      ctx - encode DCT pipeline context
**
*****************************************************************************/
#if defined(XIESMI)
static int _EncodeDctFlush(ctx)
 EncodeDctPipeCtxPtr  ctx;
{
    int	i, status;
    /*
    **  Set all drains to quantum "any" to release any data less than a
    **	full DCT block at the end of the image.
    */
    for( i = 0;  i < XieK_MaxComponents ;  i++ )
	if ( 1<<i & DceCmpMsk_(ctx))
	    DdxRmSetQuantum_( DceSrcDrn_(ctx,i), 0 );

    /*
    **	Call activate to process it.
    */
    status = _EncodeDctActivate(ctx);
    _EncodeDctAbort(ctx);

    return( status );
}                               /* end _EncodeDctFlush */
#endif

/*****************************************************************************
**  _EncodeDctAbort
**
**  FUNCTIONAL DESCRIPTION:
**
**      Deallocate resources acquired during initialize or activate.
**
**  FORMAL PARAMETERS:
**
**      ctx - encode DCT pipeline context
**
*****************************************************************************/
#if defined(XIESMI)
static int _EncodeDctAbort(ctx)
 EncodeDctPipeCtxPtr  ctx;
{
    DceSteBlk_(ctx) = DceDestroyState( DceSteBlk_(ctx) );

    return( _EncodeDctFreeData(ctx) );
}                               /* end _EncodeDctAbort */
#endif

/*****************************************************************************
**  _EncodeDctDestroy
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine deallocates all resources allocated by the encode DCT
**	pipeline element.
**
**  FORMAL PARAMETERS:
**
**      ctx - encode DCT pipeline context
**
*****************************************************************************/
#if defined(XIESMI)
static int _EncodeDctDestroy(ctx)
 EncodeDctPipeCtxPtr  ctx;
{
    int	i;

    for( i = 0;  i < XieK_MaxComponents;  i++ )
	if( 1<<i & DceCmpMsk_(ctx) )
	    DceSrcDrn_(ctx,i) = DdxRmDestroyDrain_( DceSrcDrn_(ctx,i) );

    return( DctX_Success );
}                               /* end _EncodeDctDestroy */
#endif

/*****************************************************************************
**  _EncodeDctFreeData
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine deallocates any data we own.
**
**  FORMAL PARAMETERS:
**
**      ctx - encode DCT pipeline context
**
*****************************************************************************/
#if defined(XIESMI)
static int _EncodeDctFreeData(ctx)
 EncodeDctPipeCtxPtr  ctx;
{
    int	i;
    /*
    **  Deallocate all the pipe data for this segment.
    */
    for( i = 0;  i < XieK_MaxComponents;  i++ )
	if( IsPointer_(DceSrcDat_(ctx,i)) )
	    DceSrcDat_(ctx,i) = DdxRmDeallocData_( DceSrcDat_(ctx,i) );

    return( DctX_Success );
}                               /* end _EncodeDctFreeData */
#endif

/*****************************************************************************
**  DceEncodeSegment
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static unsigned long DceEncodeSegment( state, udplst, udpcnt )
EncodeDctStatePtr   state;
UdpPtr		    udplst[];
int		    udpcnt;
{
    int	    status;
    int	    nxttok;
    int	    reqlen;

    while (TRUE)
    {
	/*
	**  Determine which token will be output next and maximum length it
	**  will require.
	*/
        nxttok = DceNextToken(state);
	reqlen = (*token_length[nxttok])(state);
	/*
	**  If remaining buffer length is too short, buffer temporarily...
	**  otherwise, write directly into supplied buffer.
	**
	**  NOTE : For now, signal output full if buffer too short.
	*/
        if (reqlen > DceCurRem_(state))
	    return DctK_StatusOutput;
	/*
	**  Call action routine....if good return, then update next token
	**  else return with current status.
	*/	
	status = (*token_action[nxttok])(state,udplst,udpcnt);
	if (status == DctK_StatusContinue)
	    DceCurTok_(state) = nxttok;
	else
	    return status;
    }
}

/*****************************************************************************
**  DceInitializeState
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
**	Address of state block or
**	NULL on error (one of):
**	    BadAlloc
**
*****************************************************************************/
static EncodeDctStatePtr DceInitializeState( udplst, udpcnt )
UdpPtr		    udplst[];
int		    udpcnt;
{
    int	    i;
    int	    c;
    int	    xmin;		/* Minimum width */
    int	    xmax;		/* Maximum width */
    int	    ymin;		/* Minimum height */
    int	    ymax;		/* Maximum height */
    char    iscd;		/* Boolean, is a common denominator */
    int	    gcd;		/* Greatest common denominator */

    EncodeDctStatePtr state = 
		(EncodeDctStatePtr) DctCalloc_(1,sizeof(EncodeDctStateRec));
    /*
    **	If allocation error, return now.
    */
    if( state == NULL ) return NULL;

    DceCmpCnt_(state) = udpcnt;
    /*
    **	Setup default tables.
    */
    DceHacTbl_(state,0) = &DctR_AcEncodeTable[0];
    DceHacTbl_(state,1) = &DctR_AcEncodeTable[1];
    DceHdcTbl_(state,0) = &DctR_DcEncodeTable[0];
    DceHdcTbl_(state,1) = &DctR_DcEncodeTable[1];

    DceQntTbl_(state,0) = &DctR_QuantDefault[0];
    DceQntDef_(state,0) = TRUE;
    DceQntTbl_(state,1) = &DctR_QuantDefault[1];
    DceQntDef_(state,1) = TRUE;
    /*
    **	Set frame dimensions to match udp[0]
    */
    DceFrmWid_(state) = udplst[0]->UdpL_PxlPerScn;
    DceFrmHgt_(state) = udplst[0]->UdpL_ScnCnt;
    /*
    **	Initialize "per-component" information.
    **	NOTE : Default tables are 0, so no explicit initialization required.
    */
    for( i = 0;  i < DceCmpCnt_(state);  i++ )
        DceSrcUdp_(state,i) = *udplst[i];
    /*
    **	For multiple components, calculate relative subsampling within scans...
    */
    if( DceCmpCnt_(state) > 1 )
	{
        xmin = DceCmpWid_(state,0);
	ymin = DceCmpHgt_(state,0);
	xmax = DceCmpWid_(state,0);
	ymax = DceCmpHgt_(state,0);

        for( i = 1;  i < DceCmpCnt_(state);  i++ )
	    {
            if( xmin > DceCmpWid_(state,i) )
		xmin = DceCmpWid_(state,i);
            if( xmax < DceCmpWid_(state,i) )
		xmax = DceCmpWid_(state,i);
            if( ymin > DceCmpHgt_(state,i) )
		ymin = DceCmpWid_(state,i);
            if( ymin < DceCmpHgt_(state,i) )
		ymax = DceCmpWid_(state,i);
	    }
	/*
	**  Find greatest common denominator for horizontal dimensions.
	*/
        for( i = 1, gcd = 1;  i <= xmin;  i++ )
            for( c = 0, iscd = TRUE;  c < DceCmpCnt_(state);  c++ )
		{
                if( DceCmpWid_(state,c) % i != 0 )
		    {
                    iscd = FALSE;
		    break;
		    }
                if( iscd )
		    gcd = i;
		}
	/*
	**  Determine the horizontal sampling rates based on GCD.
	*/
        for( c = 0;  c < DceCmpCnt_(state);  c++ )
            DceCmpHrz_(state,c) = DceCmpWid_(state,c) / gcd;
	/*
	**  Find greatest common denominator for vertical dimensions.
	*/
        for( i = 1, gcd = 1;  i <= ymin;  i++ )
            for( c = 0, iscd = TRUE;  c < DceCmpCnt_(state);  c++ )
		{
                if( DceCmpHgt_(state,c) % i != 0 )
		    {
                    iscd = FALSE;
		    break;
		    }
                if( iscd )
		    gcd = i;
		}
	/*
	**  Determine the vertical sampling rates based on GCD.
	*/
        for( c = 0;  c < DceCmpCnt_(state);  c++ )
            DceCmpVrt_(state,c) = DceCmpHgt_(state,c) / gcd;
	}
    else
	{
        DceCmpVrt_(state,0) = 1;
	DceCmpHrz_(state,0) = 1;
	}
    /*
    **	Assuming restarts are disabled, calculate restart interval to be
    **	entire image.
    */
    DceRstInt_(state) = 
      ((DceCmpHgt_(state,0)+(8*DceCmpVrt_(state,0)-1))/(8*DceCmpVrt_(state,0)))*
      ((DceCmpWid_(state,0)+(8*DceCmpHrz_(state,0)-1))/(8*DceCmpHrz_(state,0)));

    return( state );
}

/*****************************************************************************
**  DceDestroyState
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static EncodeDctStatePtr DceDestroyState( state )
 EncodeDctStatePtr   state;
{
    int	i;

#if defined(XIESMI) 
    if( !IsPointer_(state) ) return( NULL );
#endif
    /*
    **	Free any "non-default" quantization tables.
    */
    for (i = 0;  i < DctK_MaxQuantTables;  i++)
#if	defined(XIESMI) 
        if ((!DceQntDef_(state,i)) && (IsPointer_(DceQntTbl_(state,i))))
#elif	defined(IPS) 
        if ((!DceQntDef_(state,i)) && (DceQntTbl_(state,i) != NULL))
#endif
            DctFree_(DceQntTbl_(state,i));

    DctFree_(state);

    return( NULL );
}

/*****************************************************************************
**  DceNextToken
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int	    DceNextToken(state)
EncodeDctStatePtr   state;
{
    int	    next_token;
    UdpPtr  udp;

    switch (DceCurTok_(state))
    {
        case DctK_TokenNone:
            next_token = DctK_TokenSOI;
	    break;

        case DctK_TokenSOI:
            next_token = DctK_TokenSOF;
	    break;

        case DctK_TokenSOF:
            next_token = DctK_TokenSOS;
	    break;

        case DctK_TokenSOS:
            if (DceRstEnb_(state))
                next_token = DctK_TokenRSC;
            else
                next_token = DctK_TokenData;
	    break;

        case DctK_TokenData:
	    udp = &DceSrcUdp_(state,0);
	    
            if (DceCurY_(state,0) > DceSrcUdp_(state,0).UdpL_Y2)
                next_token = DctK_TokenEOI;
            else if (DceEndFrm_(state))
                next_token = DctK_TokenSOF;
            else if (DceRstEnb_(state))
                next_token = DctK_TokenRSC;
            else
                next_token = DctK_TokenData;
	    break;

        case DctK_TokenRSC:
            next_token = DctK_TokenData;
	    break;

        default:
            next_token = DctK_TokenError;
    }
    return  next_token;
}

/*****************************************************************************
**  DceLenDat
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int	    DceLenDat(state)
EncodeDctStatePtr   state;
{
    /* WORST CASE COMPRESSED DATA LENGTH ?? */
    return  0;
}

/*****************************************************************************
**  DceLenRSC
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int	    DceLenRSC(state)
EncodeDctStatePtr   state;
{
    return  DctK_LenFixedRSC;
}

/*****************************************************************************
**  DceLenSOI
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int	    DceLenSOI(state)
EncodeDctStatePtr   state;
{
    return  DctK_LenFixedSOI;
}

/*****************************************************************************
**  DceLenSOF
**
**  FUNCTIONAL DESCRIPTION:
**
**	Returns the size of the Start of Frame token. The size is the sum of:
**
**	1) The marker code and fixed size header information.
**	2) "per component" information, one block per component.
**	3) Quantization table assignments. One byte for every two components.
**	4) Quantization tables, 1 byte for table specifier plus table data
**	   for each table, followed by a one byte terminator (0x80 value);
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int	    DceLenSOF(state)
EncodeDctStatePtr   state;
{
    int	    quant_cnt = 0;
    int	    i;

    for (i = 0;  i < DctK_MaxQuantTables;  i++)
	if (DceQntTbl_(state,i) != NULL && !DceQntDef_(state,i))
	    quant_cnt++;

    return  DctK_LenFixedSOF +
	    DceCmpCnt_(state) * DctK_LenPerCmpSOF +
	    (DceCmpCnt_(state) + 1) / 2 +
	    quant_cnt * (sizeof(QuantTable) + 1) + 1;
}

/*****************************************************************************
**  DceLenSOS
**
**  FUNCTIONAL DESCRIPTION:
**
**	Returns the length of the Start of Scan marker. Value includes the sum
**	of :
**
**	1) Marker and fixed size header information.
**	2) Per component huffman specifiers.
**	3) User defined huffman tables (not supported), so only need room for
**	   one byte terminator (0x80).
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int	    DceLenSOS(state)
EncodeDctStatePtr   state;
{
    return  DctK_LenFixedSOS +
	    DceCmpCnt_(state) * DctK_LenPerCmpSOS +
/*  
**  Private huffman tables not supported
** 	    DceHufCnt_(state) * (sizeof(HuffTable) + 1) + 1;
*/
	    1;
}

/*****************************************************************************
**  DceLenEOI
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int	    DceLenEOI(state)
EncodeDctStatePtr   state;
{
    return  DctK_LenFixedEOI;
}

/*****************************************************************************
**  DceActDat
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int	    DceActDat(state,udplst,udpcnt)
EncodeDctStatePtr   state;
UdpPtr		    udplst[];
int		    udpcnt;
{
    int	    length;
    int	    mducnt;
    int	    scncnt;
    int	    lstscn;
    /*
    **	Make sure there's enough data to encode the next bunch-o-MDUs.
    **
    **	Start by calculating how many MDU will remain after the current scan.
    */
    mducnt = DceRstInt_(state) -
	     ((DceCmpWid_(state,0) - DceCurX_(state,0)) +
	      (DctK_BlockSize * DceCmpHrz_(state,0) - 1)) /
	      (DctK_BlockSize * DceCmpHrz_(state,0));
    /*
    **	Now, compute how many total scanlines will be required.
    */
    scncnt = mducnt <= 0 ? 
		DctK_BlockSize :
		mducnt / 
		    ((DceCmpWid_(state,0) + 
			(DctK_BlockSize * DceCmpHrz_(state,0) - 1)) / 
		        (DctK_BlockSize * DceCmpHrz_(state,0)));
    /*
    **	Figure last "legal" input scan as Y2 or Y2 rounded to next block if
    **	padding at end of image.
    */
    lstscn = (DceCurY_(state,0) + scncnt) > DceCmpHgt_(state,0) ?
		DceSrcUdp_(state,0).UdpL_Y2 :
		DceCurY_(state,0) + scncnt - 1;
    /*
    **	If current segment doesn't have data we need, return for more.
    */
    if ((udplst[0]->UdpL_Y1 > DceCurY_(state,0)) || (udplst[0]->UdpL_Y2 < lstscn))
	return DctK_StatusInput;
    /*
    **	Encode data buffers.
    */
    length = DceEncodeData(state,
			   udplst,
			   udpcnt,
			   DceCurPtr_(state),
			   DceCurRem_(state));
    /*
    **	Update output pointers...
    */
    DceCurPtr_(state) += length;
    DceCurRem_(state) -= length;
    /*
    **	Return goodness...
    */
    return  DctK_StatusContinue;
}

/*****************************************************************************
**  DceActRSC
**
**  FUNCTIONAL DESCRIPTION:
**
**  Write out a resync marker code. There are eight possible values...we 
**  increment the low three bits of the marker code as we go.
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int	    DceActRSC(state,udplst,udpcnt)
EncodeDctStatePtr   state;
UdpPtr		    udplst[];
int		    udpcnt;
{
    int	i;

    *(DceCurPtr_(state))++ = DctK_Marker;
    *(DceCurPtr_(state))++ = DctK_MarkerRSC + (DceRstCnt_(state)++ & 0x7);

    DceCurRem_(state) -= DctK_LenFixedRSC;
    /*
    **	Reset DC predictor
    */
    for (i = 0;  i < DceCmpCnt_(state);  i++)
    {
	DceCmpPdc_(state,i) = 0;
    }

    return  DctK_StatusContinue;
}

/*****************************************************************************
**  DceActSOI
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int	    DceActSOI(state,udplst,udpcnt)
EncodeDctStatePtr   state;
UdpPtr		    udplst[];
int		    udpcnt;
{
    *(DceCurPtr_(state))++ = DctK_Marker;
    *(DceCurPtr_(state))++ = DctK_MarkerSOI;

    DceCurRem_(state) -= DctK_LenFixedSOI;

    return DctK_StatusContinue;
}

/*****************************************************************************
**  DceActSOF
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int	    DceActSOF(state,udplst,udpcnt)
EncodeDctStatePtr   state;
UdpPtr		    udplst[];
int		    udpcnt;
{
    unsigned char   *ptr = DceCurPtr_(state);
    unsigned char   *lenptr;
    int		     i;
    short	     length;
    /*
    **	Write marker code and save room to write data length.
    */
    *ptr++ = DctK_Marker;
    *ptr++ = DctK_MarkerSOF;
    lenptr = ptr; ptr += 2;
    /*
    **	Frame information...
    */
    *ptr++ = DctK_Mode;
    *ptr++ = DctK_Precision;
     PutShort_(ptr,DceFrmHgt_(state));
     PutShort_(ptr,DceFrmWid_(state));
     PutShort_(ptr,DceRstInt_(state));
    /*
    **	Per component information...
    */
    *ptr++ = DceCmpCnt_(state);
    for (i = 0;  i < DceCmpCnt_(state);  i++)
    {
        *ptr++ = DceCmpIdx_(state,i);
	*ptr++ = (DceCmpHrz_(state,i) << 4) + DceCmpVrt_(state,i);
    }
    /*
    **	Per component quantization table assignments.
    */
    *ptr = '\0';	    /* Note : even though an extra byte may be	    */
    *(ptr + 1) = '\0';	    /*  zeroed, it shouldn't matter since the	    */
			    /*  terminating 0x80 must be written anyway.    */
    switch (DceCmpCnt_(state))
    {
        case 4:	*(ptr + 1)  = DceCmpQnt_(state,3) << 4;
        case 3:	*(ptr + 1) |= DceCmpQnt_(state,2) & 0xF;
        case 2:	*ptr        = DceCmpQnt_(state,1) << 4;
        case 1:	*ptr       |= DceCmpQnt_(state,0) & 0xF;
		break;
        default:
            return DctK_StatusError;
    }
    ptr += ((DceCmpCnt_(state) + 1) / 2);
    /*
    **	Non-default quantization tables.
    */
    for (i = 0;  i < DctK_MaxQuantTables; i++)
	if (DceQntTbl_(state,i) != NULL && !DceQntDef_(state,i))
	{
	    *ptr++ = i;
	    memcpy(ptr,DceQntTbl_(state,i),sizeof(QuantTable));
	    ptr += sizeof(QuantTable);
	}
    *ptr++ = 0x80;

    length = ptr - lenptr;
    PutShort_(lenptr,length);

    DceCurRem_(state) -= ptr - DceCurPtr_(state);
    DceCurPtr_(state)  = ptr;

    return  DctK_StatusContinue;
}

/*****************************************************************************
**  DceActSOS
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int	    DceActSOS(state,udplst,udpcnt)
EncodeDctStatePtr   state;
UdpPtr		    udplst[];
int		    udpcnt;
{
    unsigned char   *ptr = DceCurPtr_(state);
    unsigned char   *lenptr;
    int		     i;
    short	     length;
    /*
    **	Write marker code, save room for length.
    */
    *ptr++ = DctK_Marker;
    *ptr++ = DctK_MarkerSOS;
    lenptr = ptr; ptr += 2;

    *ptr++ = 0;		    /* Component ID, always 0...if data wasn't	    */
			    /* interleaved, this would be the component	    */
			    /* index in this scan.			    */
    /*
    **	Huffman table assignments.
    */
    for (i = 0;  i < DceCmpCnt_(state);  i++)
    {
        *ptr++ = (DceCmpHac_(state,i) << 4) + DceCmpHdc_(state,i);
    }
    /*
    **	Download custom huffman tables...not supported.
    */
    *ptr++ = 0x80;
    /*
    **	Store length of data in field.
    */
    length = ptr - lenptr;
    PutShort_(lenptr,length);
    /*
    **	Update pointers.
    */
    DceCurRem_(state) -= ptr - DceCurPtr_(state);
    DceCurPtr_(state)  = ptr;

    return  DctK_StatusContinue;
}

/*****************************************************************************
**  DceActEOI
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int	    DceActEOI(state,udplst,udpcnt)
EncodeDctStatePtr   state;
UdpPtr		    udplst[];
int		    udpcnt;
{
    *(DceCurPtr_(state))++ = DctK_Marker;
    *(DceCurPtr_(state))++ = DctK_MarkerEOI;
      DceCurRem_(state)  -= DctK_LenFixedEOI;

    return DctK_StatusDone;
}

/*****************************************************************************
**  DceEncodeData
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine encodes a segment of data equivalent to the number of
**	MDUs in a restart interval.
**
**	The data must be padded out to modulo 8 pixels per scanline and
**	scanline count.
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int	     DceEncodeData(state,udplst,udpcnt,outptr,outlen)
EncodeDctStatePtr    state;
UdpPtr		     udplst[];
int		     udpcnt;
unsigned char	    *outptr;
int		     outlen;
{
    ZQ_VARIABLES

    unsigned char   *scnptr;
    unsigned char   *pxlptr;
    UdpPtr	     udp;

    int		     i;
    int		     c;
    int		     h;
    int		     v;
    int		     x;
    int		     y;

    unsigned char    tmp[64];
    short int	     work1[64];
    short int	     work2[64];

    int		     bufoff = 0;

    unsigned char   *base[DctK_MaxComponents];
    int		     pix_stride[DctK_MaxComponents];
    int		     scn_stride[DctK_MaxComponents];
    /*
    **	Do per-component set-up.
    */
    for (i = 0;  i < DceCmpCnt_(state);  i++)
    {
	udp = udplst[i];

        scn_stride[i] = udp->UdpL_ScnStride / 8;
	pix_stride[i] = udp->UdpL_PxlStride / 8;
#if	defined(XIESMI)
	base[i] = udp->UdpA_Base + udp->UdpL_Pos / 8;
#elif	defined(IPS)
	base[i] = (unsigned char *)udp->UdpA_Base + (((udp->UdpL_Pos + 
	    (udp->UdpL_Y1 * udp->UdpL_ScnStride))>>3) + udp->UdpL_X1);
#endif
    }
    /*
    **	For each MDU in this segment.
    */
    for (i = 0;  i < DceRstInt_(state);  i++)
    {
        /*
	**  For each component in the MDU
	*/
        for (c = 0;  c < DceCmpCnt_(state);  c++)
        {
	    udp = udplst[c];

	    x = DceCurX_(state,c);
	    y = DceCurY_(state,c);

	    scnptr = base[c] + 
		     (y - udp->UdpL_Y1) * scn_stride[c] + 
		     (x - udp->UdpL_X1) * pix_stride[c];
	    /*
	    **	For each vertical sample...
	    */
            for (v = 0;  v < DceCmpVrt_(state,c);  v++)
            {
		pxlptr = scnptr;
		/*
		**  For each horizontal sample.
		*/
                for (h = 0;  h < DceCmpHrz_(state,c);  h++)
                {
		    /*
		    **	If there isn't enough source data for this block, pad
		    **	it out...
		    */
                    if (((x + DctK_BlockSize - 1) > udp->UdpL_X2) || 
			((y + DctK_BlockSize - 1) > udp->UdpL_Y2))
                    {
			/*
			**  Calculate remaining rows and columns in image. If
			**  greater than 8, copy no more than that.
			*/
			int srcrow = udp->UdpL_Y2 - y + 1;
			int srccol = udp->UdpL_X2 - x + 1;

			if (srcrow > 8) srcrow = 8;
			if (srccol > 8) srccol = 8;
			/*
			**  Pad data into temporary buffer.
			*/
			DcePadData (pxlptr,		    /* Source pointer */
				    srcrow,		    /* Source rows    */
				    srccol,		    /* Source columns */
				    scn_stride[c],	    /* Scan stride    */
				    pix_stride[c],	    /* Pixel stride   */
				    tmp,		    /* Dest pointer   */
				    8,			    /* Dest rows      */
				    8);			    /* Dest columns   */
			/*
			**  Do forward DCT from temporary buffer.
			*/
			Dce8x8_8U_16S(tmp,		    /* Data pointer   */
				      work1,		    /* Dest pointer   */
				      8,		    /* Data scan str  */
				      1);		    /* Data pixel str */
                    }
                    else
                    {
			/*
			**  Do forward DCT directly from source data.
			*/
			Dce8x8_8U_16S(pxlptr,
				      work1,
				      scn_stride[c],
				      pix_stride[c]);
                    }
		    /*
		    **	Quantize and zigzag order coefficients.
		    */
#ifdef __alpha
            d_in_ptr  = (short int *)(work1) ;
            d_out_ptr = (short int *)(work2) ;
            d_q_ptr   = (unsigned char *)(DceQntTbl_(state,DceCmpQnt_(state,c))) ;
            d_z_ptr   = (short int *)(DctAW_ZigZag) ;
	    for( d_i=0 ; d_i<=63 ; d_i++ )
	    {
	      d_dtemp = 
               (double)(*(d_in_ptr+(*d_z_ptr++)))/(double)(*d_q_ptr++);
    	      *d_out_ptr++ = 
               (d_dtemp<0.) ? (int)(d_dtemp-.5) : (int)(d_dtemp+.5) ;
	    }
#else
		    ZIGZAG_QUANT(work1,
				 work2,
				 DctAW_ZigZag,
				 DceQntTbl_(state,DceCmpQnt_(state,c)));
#endif
		    /*
		    **	Do huffman encoding on the result.
		    */
		    bufoff += DctEncode(work2,
					DceHdcTbl_(state,DceCmpHdc_(state,c)),
					DceHacTbl_(state,DceCmpHac_(state,c)),
					&DceCmpPdc_(state,c),
					outptr,
					outlen,
					bufoff);
		    /*
		    **	Update pointer to next horizontal sample.
		    */
		    pxlptr += (8 * pix_stride[c]);
		    x += 8;
                }
		/*
		**  Update pointers to next vertical sample;
		*/
		scnptr += (8 * scn_stride[c]);
		y += 8;
            }
	    /*
	    **	Update current X,Y location pointers. If at end of scan, reset
	    **	X pointer and increment Y pointer.
	    */
	    DceCurX_(state,c) += (8 * DceCmpHrz_(state,c));
	    if (DceCurX_(state,c) >= DceCmpWid_(state,c))
	    {
	    	DceCurX_(state,c) = 0;
		DceCurY_(state,c) += (8 * DceCmpVrt_(state,c));
	    }
        }
	if (DceCurY_(state,0) >= DceCmpHgt_(state,0))
	    break;
    }
    return  (bufoff + 7) / 8;
}


/*****************************************************************************
**  DcePadData
**
**  FUNCTIONAL DESCRIPTION:
**
**  Copies a partial interleave of pixels into a work area and completes a
**  whole interleave there by replicating right and bottom pixels.
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
*****************************************************************************/

static void DcePadData (srcptr,srcrow,srccol,srcsst,srcpst,dstptr,dstrow,dstcol)
unsigned char	*srcptr;	/* Pointer to source data		    */
int		 srcrow;	/* Number of rows of source data	    */
int		 srccol;	/* Number of columns of source data	    */
int		 srcsst;	/* Source data scanline stride		    */
int		 srcpst;	/* Source data pixel stride		    */
unsigned char	*dstptr;	/* Destination data pointer		    */
int		 dstrow;	/* Destination rows			    */
int		 dstcol;	/* Destination columns			    */
{
    int i, j;
    unsigned char *srcscn = srcptr;
    unsigned char *src;
    unsigned char *dst = dstptr;
    /*
    **	Copy to destination rows
    */
    for (i = 0; i < dstrow; i++)
    {
	/*
	**  Update source pointer to beginning of current row.
	*/
	src = srcscn;
	/*
	**  If source row exists, copy from source, otherwise, copy previous
	**  destination row.
	*/
        if (i < srcrow)
        {
	    /*
	    **	Copy from source...
	    */
            for (j = 0;  j < dstcol;  j++)
            {
		/*
		**  If source column exists, copy from source, otherwise, copy
		**  previous destination pixel.
		*/
                if (j < srccol)
                {
		    *dst++ = *src;	/* Copy from source */
		     src += srcpst;	/* Update source pointer */
                }
                else
                {
		    *dst++ = *(src-srcpst); /* Copy previous source pixel */
                }
            }
        }
        else
        {
	    /*
	    **	Copy previous destination row.
	    */
	    memcpy(dst,dst-dstcol,dstcol);
	    dst += dstcol;
        }
	/*
	**  Update source scan pointer to next scanline
	*/
	srcscn += srcsst;
    }
} 

/*****************************************************************************
**  DctEncode
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
*****************************************************************************/

/* ENCODE: Huffman encodes one block of quantized, zigzag scanned DCT coeffs */

static int	     DctEncode(z_coef,dc_tbl,ac_tbl,dc_prev,bufptr,buflen,
				bitoff)  
short int	    *z_coef; 
DcEncodeTablePtr     dc_tbl;
AcEncodeTablePtr     ac_tbl;
short int           *dc_prev;
unsigned char	    *bufptr;
int		     buflen;
int		     bitoff;
{
    unsigned char   *dc_size_tbl ;
    unsigned char   *ac_size_tbl ;
    short int       *dc_code_tbl ; /* was int  */
    short int       *ac_code_tbl ; /* was int  */

    int		    coef_index;
    int		    temp;
    unsigned char   exp;
    unsigned char   run_length ;

    int		    curoff = bitoff;
        
    short	    coef;
    unsigned char   coef_lo;
    unsigned char   coef_hi;

    static unsigned char exp_tbl_hi[256] = { 
    0,9,10,10,11,11,11,11,12,12,12,12,12,12,12,12,
    13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,
    14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,
    14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,
    15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
    14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,
    14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,
    13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,
    12,12,12,12,12,12,12,12,11,11,11,11,10,10,9,8  } ; 

    static unsigned char exp_tbl_lo0[256] = { 
    0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8  } ;

    static unsigned char exp_tbl_lo1[256] = { 
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,4,4,4,4,4,4,4,4,3,3,3,3,2,2,1,16  } ; 

    /* 
    **	Assign pointers inside huff structures  
    */
    dc_size_tbl = dc_tbl->huff_size; 
    ac_size_tbl = ac_tbl->huff_size;
    dc_code_tbl = dc_tbl->huff_code;
    ac_code_tbl = ac_tbl->huff_code;
    /* 
    **	DC encoding
    */
     coef = (*z_coef) - (*dc_prev);
    *dc_prev = *z_coef++;

    if (coef < 0) coef = (coef - 1) | 0x8000 ;

    coef_lo = coef & 0xFF;
    coef_hi = coef >> 8;
    
    if (!((exp = exp_tbl_hi[coef_hi]) & 7))/* if hi byte is not just sign*/
	if (exp) 
	    exp = exp_tbl_lo1[coef_lo]; /* if hi byte is all 1's */
	else
	    exp = exp_tbl_lo0[coef_lo]; /* if hi byte is all 0's */

    curoff += DctPutBits(dc_code_tbl[exp],(int)dc_size_tbl[exp],bufptr,curoff);
    /*
    **	Write out mantissa bits if exp != 0 or 16
    */
    if (exp & 15)
	curoff += DctPutBits((int)(coef),(int)(exp),bufptr,curoff);
    /*
    **	AC encoding
    */
    run_length = 0 ;
    for (coef_index = 1; coef_index <= 63; coef_index++)
    {
	/*
	**  Accumulate runs of zero coefficients
	*/
        if (*z_coef == 0)
	{
	    z_coef++;
            run_length++;
        }
  	else
    	{
	    /*
	    ** Generate MAX-RUN-LENGTH if run >= 16 and non-zero coef found
	    */
	    while (run_length >= 16)
	    {
		run_length = run_length - 16 ;
                curoff += DctPutBits((int)ac_code_tbl[240],
				     (int)ac_size_tbl[240],
				     bufptr,
				     curoff) ;
    	    }
	    /*	 
	    **	Normal case: encode exponent, mantissa, and preceding 0
	    **	run length, for non-zero coefficient
	    **
	    **	Calculate exponent from table (Note: the value 0x8000 is not
	    **	allowed as input -- will crash) (Assumes clipping on the
	    **	quantizer output)
	    */
     	    coef = *z_coef++ ;
            if (coef < 0)  coef -= 1;

	    coef_lo = coef & 0xFF;
	    coef_hi = coef >> 8;

    	    if (!((exp=exp_tbl_hi[coef_hi])&7))   /* if hi byte is not just sign*/
		if (exp) exp = exp_tbl_lo1[coef_lo]; /* if hi byte is all 1's */
                else     exp = exp_tbl_lo0[coef_lo]; /* if hi byte is all 0's */
	    /*
	    **	Store fixed codes
	    */
            temp = exp + 16 * run_length ;
            curoff += DctPutBits((int)ac_code_tbl[temp], 
				 (int)ac_size_tbl[temp],
				 bufptr,
				 curoff);
            curoff += DctPutBits((int)(coef),(int)(exp),bufptr,curoff);
            run_length = 0 ;
    	}
    }
    /*
    ** Generate END-OF-BLOCK if last coef is a zero
    */
    if (run_length != 0)
	curoff += DctPutBits((int)ac_code_tbl[0],
			     (int)ac_size_tbl[0],
			     bufptr,
			     curoff);
    /*
    **	Return number of bits written.
    */
    return  curoff - bitoff;
}

/*******************************************************************
  DctPadByte

  Pads the BITS_AVAILABLE msb's of the current byte in the output bitstream 
  STREAM_OUT with 1 bits.  If the result is FF, adds a pad byte of 00 to the 
  stream.
********************************************************************/

static int	 DctPadByte(bufptr,bitoff)
unsigned char	*bufptr;
int		 bitoff;
{ 
    if (bitoff & 0x7)
	return DctPutBits(0xFF,8,bufptr,bitoff);
    else
	return 0;
}

/*******************************************************************
  DctPutBits 

  Places NUMBER_OF_BITS lsb's of DATA_IN into a bit stream (array of 
  unsigned char) currently in position STREAM_OUT.  The byte pointed to
  by STREAM_OUT has BITS_AVAILABLE of unfilled msb bits upon entry.
  Both STREAM_OUT and BITS_AVAILABLE may be changed by PUTBITS.
********************************************************************/
static int	 DctPutBits(data_in,number_of_bits,bufptr,bitoff)
int		 data_in;
int		 number_of_bits;
unsigned char	*bufptr;
int		 bitoff;
{
    int		     bits_available = 8 - (bitoff & 0x07);
    unsigned char   *stream_out = bufptr + bitoff / 8;
    int		     bits_written = number_of_bits;

    if (bits_available == 8)
	*stream_out = 0;

    if (number_of_bits >= bits_available)
    {
	(*stream_out) |= 
		  ((data_in & ((1 << bits_available) - 1))<<(8-bits_available));
	data_in >>= bits_available ;
	number_of_bits -= bits_available ;
	if(*stream_out == 0xff)
	    (*(++stream_out) = 0, bits_written += 8);
	*(++stream_out) = 0;
	bits_available = 8;
	if(number_of_bits >= 8)		/* 8 being the current bits_available */
	{
	    *stream_out = data_in & 255;
	    data_in >>= 8 ;
	    number_of_bits -= 8;
	    if (*stream_out == 0xff) 
		(*(++stream_out) = 0, bits_written += 8);
	    *(++stream_out) = 0;
	}
    }

    (*stream_out) |= 
	    ((data_in & ((1 << number_of_bits) - 1))<<(8-bits_available)) ;

    return bits_written;
}

/*****************************************************************************
**  DctCreateScaledQuant
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
**	status - one of:
**	    Success, all is well
**	    BadAlloc, memory allocation failure
**
*****************************************************************************/
static int	    DctCreateScaledQuant(state,defidx,outidx,factor)
EncodeDctStatePtr   state;
int		    defidx;
int		    outidx;
int		    factor;
{
    int	temp;
    int i;
    /*
    **	Allocate space for new quantization table.
    */
    QuantTablePtr   outtab = (QuantTablePtr)DctMalloc_(sizeof(QuantTable));
    /*
    **	Return error on bad allocation
    */
    if (outtab == NULL)	return DctX_BadAlloc;
    /*
    **	Fill in data from specified default table.
    */
    for (i = 0;  i < DctK_QuantTableSize;  i++)
    {
        temp = 
	    (int)(0.5+((float)factor)*DctR_QuantDefault[defidx].quant[i]/50.0);
	outtab->quant[i] = temp < 1 ? 1 : temp > 255 ? 255 : temp;
    }
    /*
    **	If current table slot already contains a non-default table, deallocate
    **	it.
    */
    if ((!DceQntDef_(state,outidx)) && (DceQntTbl_(state,outidx) != NULL)) 
	DctFree_(DceQntTbl_(state,outidx));
    /*
    **	Set new quantization table in state block.
    */
    DceQntTbl_(state,outidx) = outtab;
    DceQntDef_(state,outidx) = FALSE;

    return  DctX_Success;
}

/*****************************************************************************
**  DctCreateScaledQuant
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int	    DctRstEnable(state)
EncodeDctStatePtr   state;
{
    /*
    **	Calculate restart interval such that we end up with whole scanlines.
    */
    DceRstEnb_(state) = TRUE;
    DceRstInt_(state) =
	(DceCmpWid_(state,0)+(8*DceCmpHrz_(state,0)-1))/(8*DceCmpHrz_(state,0));
    /*
    DceRstInt_(state) = 19;
    */
}

static void Dce8x8_8U_16S(PIXELS,COEFS,SCAN_WIDTH,PEL_WIDTH) 

unsigned char *PIXELS ; 
short int     *COEFS ; 
int           SCAN_WIDTH, PEL_WIDTH ;

{
DCT_VARIABLES

    d_puc1 = PIXELS                                              ;
    d_puc2 = d_puc1 + 7 * PEL_WIDTH                              ;
    d_ps1 = COEFS                                                ;
    d_pi1 = d_work                                               ;
    for(d_k=0 ; d_k<8; d_k++)
    {
      d_ap = d_a                                                 ;
      *d_ap++ = ((*d_puc1) + (*d_puc2) - 256)<<4                 ;
      d_puc1 += PEL_WIDTH; d_puc2 -= PEL_WIDTH ;                  
      *d_ap++ = ((*d_puc1) + (*d_puc2) - 256)<<4                 ;
      d_puc1 += PEL_WIDTH; d_puc2 -= PEL_WIDTH ;                  
      *d_ap++ = ((*d_puc1) + (*d_puc2) - 256)<<4                 ;
      d_puc1 += PEL_WIDTH; d_puc2 -= PEL_WIDTH ;                  
      *d_ap++ = ((*d_puc1)   + (*d_puc2)   - 256)<<4             ;
      *d_ap++ = ((*d_puc1) - (*d_puc2)      )<<4                 ;
      d_puc1 -= PEL_WIDTH; d_puc2 += PEL_WIDTH ;                  
      *d_ap++ = ((*d_puc1) - (*d_puc2)      )<<4                 ;
      d_puc1 -= PEL_WIDTH; d_puc2 += PEL_WIDTH ;                  
      *d_ap++ = ((*d_puc1) - (*d_puc2)      )<<4                 ;
      d_puc1 -= PEL_WIDTH; d_puc2 += PEL_WIDTH ;                  
      *d_ap   = ((*d_puc1)   - (*d_puc2)        )<<4             ;
      d_puc1 += SCAN_WIDTH                                       ;
      d_puc2 += SCAN_WIDTH                                       ;

      d_bp = d_b                                                 ;
      *d_bp++ = d_a[0] + d_a[3]                                  ;
      *d_bp++ = d_a[1] + d_a[2]                                  ;
      *d_bp++ = d_a[1] - d_a[2]                                  ;
      *d_bp++ = d_a[0] - d_a[3]                                  ;
      *d_bp++ = d_a[4]                                           ;
      *d_bp++ = (d_cpi4*(d_a[6]-d_a[5])           + 32768)>>16   ;
      *d_bp++ = (d_cpi4*(d_a[6]+d_a[5])           + 32768)>>16   ;
      *d_bp   = d_a[7]                                           ;

      d_cp = d_c                                                 ;
      *d_cp++ = (d_cpi4*(d_b[0]+d_b[1])           + 32768)>>16   ;
      *d_cp++ = (d_cpi4*(d_b[0]-d_b[1])           + 32768)>>16   ;
      *d_cp++ = (d_spi8*d_b[2] + d_cpi8*d_b[3]      + 32768)>>16 ;
      *d_cp++ = (d_c3pi8*d_b[3] - d_s3pi8*d_b[2]    + 32768)>>16 ;
      *d_cp++ = d_b[4] + d_b[5]                                  ;
      *d_cp++ = d_b[4] - d_b[5]                                  ;
      *d_cp++ = d_b[7] - d_b[6]                                  ;
      *d_cp   = d_b[7] + d_b[6]                                  ;

      *d_pi1      = (d_c[0]                            +    1)>>1      ;
      *(d_pi1+8)  = (d_spi16*d_c[4]  + d_cpi16*d_c[7]  +65536)>>17     ;
      *(d_pi1+16) = (d_c[2]                            +    1)>>1      ;
      *(d_pi1+24) = (d_c3pi16*d_c[6] - d_s3pi16*d_c[5] +65536)>>17     ;
      *(d_pi1+32) = (d_c[1]                            +    1)>>1      ;
      *(d_pi1+40) = (d_s5pi16*d_c[5] + d_c5pi16*d_c[6] +65536)>>17     ;
      *(d_pi1+48) = (d_c[3]                            +    1)>>1      ;
      *(d_pi1+56) = (d_c7pi16*d_c[7] - d_s7pi16*d_c[4] +65536)>>17     ;
      d_pi1++                                                          ;
    }
    d_pi2 = d_work                                               ;
    d_pi3 = d_work + 7                                           ;
    for(d_k=0 ; d_k <8; d_k++)
    {
      d_ap = d_a                                                 ;
      *d_ap++ = (*d_pi2++) + (*d_pi3--)                          ;
      *d_ap++ = (*d_pi2++) + (*d_pi3--)                          ;
      *d_ap++ = (*d_pi2++) + (*d_pi3--)                          ;
      *d_ap++ = (*d_pi2)   + (*d_pi3)                            ;
      *d_ap++ = (*d_pi2--) - (*d_pi3++)                          ;
      *d_ap++ = (*d_pi2--) - (*d_pi3++)                          ;
      *d_ap++ = (*d_pi2--) - (*d_pi3++)                          ;
      *d_ap   = (*d_pi2)   - (*d_pi3)                            ;
      d_pi2 += 8                                                 ;
      d_pi3 += 8                                                 ;

      d_bp = d_b                                                   ;
      *d_bp++ = d_a[0] + d_a[3]                                  ;
      *d_bp++ = d_a[1] + d_a[2]                                  ;
      *d_bp++ = d_a[1] - d_a[2]                                  ;
      *d_bp++ = d_a[0] - d_a[3]                                  ;
      *d_bp++ = d_a[4]                                           ;
      *d_bp++ = (d_cpi4*(d_a[6]-d_a[5])           + 32768)>>16   ;
      *d_bp++ = (d_cpi4*(d_a[6]+d_a[5])           + 32768)>>16   ;
      *d_bp   = d_a[7]                                           ;

      d_cp = d_c                                                 ;
      *d_cp++ = (d_cpi4*(d_b[0]+d_b[1])           + 32768)>>16   ;
      *d_cp++ = (d_cpi4*(d_b[0]-d_b[1])           + 32768)>>16   ;
      *d_cp++ = (d_spi8*d_b[2] + d_cpi8*d_b[3]    + 32768)>>16   ;
      *d_cp++ = (d_c3pi8*d_b[3] - d_s3pi8*d_b[2]  + 32768)>>16   ;
      *d_cp++ = d_b[4] + d_b[5]                                  ;
      *d_cp++ = d_b[4] - d_b[5]                                  ;
      *d_cp++ = d_b[7] - d_b[6]                                  ;
      *d_cp   = d_b[7] + d_b[6]                                  ;

      *d_ps1      = (d_c[0]                            + 16     )>>5      ;
      *(d_ps1+8)  = (d_spi16*d_c[4]  + d_cpi16*d_c[7]  + (1<<20))>>21     ;
      *(d_ps1+16) = (d_c[2]                            + 16     )>>5      ;
      *(d_ps1+24) = (d_c3pi16*d_c[6] - d_s3pi16*d_c[5] + (1<<20))>>21     ;
      *(d_ps1+32) = (d_c[1]                            + 16     )>>5      ;
      *(d_ps1+40) = (d_s5pi16*d_c[5] + d_c5pi16*d_c[6] + (1<<20))>>21     ;
      *(d_ps1+48) = (d_c[3]                            + 16     )>>5      ;
      *(d_ps1+56) = (d_c7pi16*d_c[7] - d_s7pi16*d_c[4] + (1<<20))>>21     ;
      d_ps1++ ;
    }

    return ;
}
