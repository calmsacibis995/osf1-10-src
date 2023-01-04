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
**      This module contains the interface to the DCT decompressesion routines.
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
**	Robert NC Shelley
**
**  CREATION DATE:
**
**	July 31, 1990
**
*******************************************************************************/

/*
**  Include Files
*/
#ifdef VMS
#include <stdlib.h>
#endif
#include <stdio.h>
#include <string.h>

#if	defined(XIESMI)
#include "SmiDecodeDct.h"
#include "SmiDctMacros.h"
#elif	defined(IPS)
#include <math.h>
#include <IpsDef.h>
#include <IpsStatusCodes.h>
#include <IpsMacros.h>
#include <IpsDctDef.h>
#include <IpsDctMacros.h>
#include <IpsDecodeDct.h>
#endif

/*
**  Table of Contents
*/
#if	defined(XIESMI)
int			    SmiDecodeDct();
#elif	defined(IPS)
long			   _IpsDecodeDct();
long			    IpsSetUdp();
#endif

#if defined(XIESMI)
int 			    SmiCreateDecodeDct();
static int 		   _DecodeDctInitialize();
static int 		   _DecodeDctActivate();
static int 		   _DecodeDctAbort();
static int 		   _DecodeDctDestroy();
#endif

static DecodeDctStatePtr    DcdInitializeState();
static DecodeDctStatePtr    DcdDestroyState();
static int		    DcdDecodeSegment();

static int		    DcdGetToken();
static int		    DcdGetData();
static unsigned char	   *DcdExtendData();
static int		    DcdSaveToken();

static int		    DcdDataAction();
static int		    DcdSosAction();
static int		    DcdSofAction();
static int		    DcdRscAction();

static void		    DcdDecode();
static short int	    DcdGetHuf();
static int		    IDCT8x8_16S_8U();

/*
**  External References
*/

/*
**  Equated symbols
*/
#define	DctK_NoComponent    0xFFFFFFFF

/*
**  Local MACROs
*/
    /*
    **	Determine if sufficient source data remains.
    */
#define	DataAvailable_(_state,_ptr,_len,_pos,_more) \
	((_len)<(_pos)+(_more)?DcdSaveToken(_state,&_ptr,&_len,_pos,_more):TRUE)


/*  DEQUANT_ZIGZAG MACRO

       Inputs:  > 16-bit signed, quantized DCT coefficients 
                > organized as sequence of blocks (within blocks, the
                    coefficients stored in zigzag order)

       Outputs: > 16-bit signed, UNquantized DCT coefficients
                > organized as sequence of blocks (within blocks, the
                    coefficients stored by rows)
*/

#define ZQ_VARIABLES \
\
double d_dtemp ;\
short int *d_in_ptr,*d_out_ptr,*d_z_ptr;\
unsigned char *d_q_ptr ;\
int d_i ;

#define DEQUANT_ZIGZAG(IN_PT,OUT_PT,Z_PT,Q_PT) \
            d_in_ptr  = (short int *)(IN_PT) ;\
            d_out_ptr = (short int *)(OUT_PT) ;\
            d_q_ptr   = (unsigned char *)(Q_PT) ;\
            d_z_ptr   = (short int *)(Z_PT) ;\
	    for( d_i=0 ; d_i<=63 ; d_i++ )\
              *(d_out_ptr+(*d_z_ptr++)) = (*d_in_ptr++) * (*d_q_ptr++) ;

/*
**  Local storage
*/
    /*
    **  DecodeDct Element Vector
    */
#if defined(XIESMI)
static PipeElementVector DecodeDctPipeElement = {
    NULL,				/* FLINK			    */
    NULL,				/* BLINK			    */
    sizeof(PipeElementVector),		/* Size				    */
    TypePipeElementVector,		/* Structure type		    */
    StypeDecodeDctElement,		/* Structure subtype		    */
    sizeof(DecodeDctPipeCtx),		/* Context block size		    */
    0,					/* No input flag		    */
    0,					/* Reserved flags		    */
    _DecodeDctInitialize,		/* Initialize entry		    */
    _DecodeDctActivate,			/* Activate entry		    */
    _DecodeDctAbort,			/* Flush entry			    */
    _DecodeDctDestroy,			/* Destroy entry		    */
    _DecodeDctAbort			/* Abort entry			    */
    };
#endif

    /*
    **	State transition table.
    */
static int next_state[DctK_StateMax][DctK_TokenMax] = {
    {DctK_StateErr, DctK_StateSOI, DctK_StateErr, DctK_StateErr, DctK_StateErr,  DctK_StateErr, DctK_StateErr, DctK_StateErr},
    {DctK_StateErr, DctK_StateErr, DctK_StateSOF, DctK_StateErr, DctK_StateErr,  DctK_StateErr, DctK_StateErr, DctK_StateErr},
    {DctK_StateErr, DctK_StateErr, DctK_StateSOF, DctK_StateSOS, DctK_StateErr,  DctK_StateErr, DctK_StateErr, DctK_StateErr},
    {DctK_StateErr, DctK_StateErr, DctK_StateErr, DctK_StateErr, DctK_StateData, DctK_StateRSC, DctK_StateErr, DctK_StateErr},
    {DctK_StateErr, DctK_StateErr, DctK_StateSOF, DctK_StateErr, DctK_StateErr,  DctK_StateRSC, DctK_StateEOI, DctK_StateErr},
    {DctK_StateErr, DctK_StateErr, DctK_StateErr, DctK_StateErr, DctK_StateData, DctK_StateErr, DctK_StateErr, DctK_StateErr},
    {DctK_StateErr, DctK_StateErr, DctK_StateErr, DctK_StateErr, DctK_StateErr,  DctK_StateErr, DctK_StateErr, DctK_StateErr},
    {DctK_StateErr, DctK_StateErr, DctK_StateErr, DctK_StateErr, DctK_StateErr,  DctK_StateErr, DctK_StateErr, DctK_StateErr},
};
    /*
    **	Transition routine table.
    */
static int (*action[DctK_StateMax][DctK_TokenMax])() = {
    {NULL, NULL, NULL,         NULL,         NULL,          NULL,         NULL, NULL},
    {NULL, NULL, DcdSofAction, NULL,         NULL,          NULL,         NULL, NULL},
    {NULL, NULL, DcdSofAction, DcdSosAction, NULL,          NULL,         NULL, NULL},
    {NULL, NULL, NULL,         NULL,         DcdDataAction, DcdRscAction, NULL, NULL},
    {NULL, NULL, DcdSofAction, NULL,         NULL,          DcdRscAction, NULL, NULL},
    {NULL, NULL, NULL,         NULL,         DcdDataAction, NULL,         NULL, NULL},
    {NULL, NULL, NULL,         NULL,         NULL,          NULL,         NULL, NULL},
    {NULL, NULL, NULL,         NULL,         NULL,          NULL,         NULL, NULL},
};

/*****************************************************************************
**  SmiDecodeDct
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      none
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
*****************************************************************************/
#if	defined(XIESMI)
int	    SmiDecodeDct(cdp,udplst,udpcnt)
#elif	defined(IPS)
long	    _IpsDecodeDct(cdp,udplst,udpcnt)
#endif
UdpPtr	    cdp;
UdpPtr	    udplst[];
int	    udpcnt;
{
    int			i;
    int			status;
    DecodeDctStatePtr	state;

#if	defined(IPS)
    /*
    **	Verify input UDPs
    */
    if ((cdp->UdpB_Class != UdpK_ClassUBS) || (cdp->UdpB_DType != UdpK_DTypeVU))
	return (DctX_UnsOption);
#endif
    /*
    **	Initialize state, return if an error.
    */
    state = DcdInitializeState();
    if (state == NULL)
	return( DctX_BadAlloc );
    /*
    **	Decode data.
    */
    status = DcdDecodeSegment( cdp, udplst, udpcnt, state );
    /*
    **	Deallocate state block.
    */
    DcdDestroyState(state);
    /*
    **	Return based on state of decode.
    */
#if	defined(XIESMI)
    if( status > DctK_StatusDone )
        return( status );
#endif
    if (status != DctK_StatusDone)
        return (DctX_DecodeFail);
    else
        return (DctX_Success);
}				    /* end of SmiDecodeDct */

/*****************************************************************************
**  SmiCreateDecodeDct
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
*****************************************************************************/
#if	defined(XIESMI)
int	    SmiCreateDecodeDct( pipe, src, dst, org )
 Pipe		pipe;
 PipeSinkPtr	src;
 PipeSinkPtr	dst;
 int		org;
{
    int c;
    DecodeDctPipeCtxPtr ctx = (DecodeDctPipeCtxPtr)
		    DdxCreatePipeCtx_( pipe, &DecodeDctPipeElement, TRUE );
    if( !IsPointer_(ctx) ) return( (int) ctx );
    /*
    **  Build our DecodeDct Pipe Ctx from the arguments passed in.
    */
    DcdSrc_(ctx) = src;
    DcdDst_(ctx) = dst;
    /*
    **	Compute number of: source planes, how many components to decode per
    **	       source plane, and the total number of destination components.
    */
    for( c = 0; c < XieK_MaxComponents && IsPointer_(SnkUdpPtr_(dst,c)); c++ );
    DcdSrcCnt_(ctx) = org == XieK_BandByPixel ? 1 : c;
    DcdDcdCnt_(ctx) = org == XieK_BandByPixel ? c : 1;
    DcdDstCnt_(ctx) = c;
    /*
    **  Create a drain which reads all the source planes.
    */
    DcdDrn_(ctx) = DdxRmCreateDrain_( src, (1 << DcdSrcCnt_(ctx)) - 1 );
    if( !IsPointer_(DcdDrn_(ctx)) ) return((int) DcdDrn_(ctx) );
    /*
    **  Process any amount of source and output bytes in DctK_BlockSize chunks.
    */
    DdxRmSetDType_( DcdDrn_(ctx), UdpK_DTypeV, DtM_V );
    DdxRmSetQuantum_( DcdDrn_(ctx), 0 );
    DdxRmSetDType_( dst, UdpK_DTypeBU, DtM_BU );
    DdxRmSetQuantum_( dst, DctK_BlockSize );

    return( DctX_Success );
}				    /* end of SmiCreateDecodeDct */
#endif

/*****************************************************************************
**  _DecodeDctInitialize
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
*****************************************************************************/
#if	defined(XIESMI)
static int  _DecodeDctInitialize( ctx )
 DecodeDctPipeCtxPtr  ctx;
{
    int h, i, status;
    /*
    **	Initialize the source Drain.
    */
    VecNoInpt_(CtxVecPtr_(ctx)) = FALSE;
    CtxInp_(ctx) = DcdDrn_(ctx);
    status = DdxRmInitializePort_( CtxHead_(ctx), DcdDrn_(ctx) );
    if( status != DctX_Success ) return( status );
    /*
    **	Initialize the destination Sink.
    */
    status = DdxRmInitializePort_( CtxHead_(ctx), DcdDst_(ctx) );
    if( status != DctX_Success ) return( status );
    /*
    **	Create a decode state per source plane.
    */
    for( i = 0; i < DcdSrcCnt_(ctx); i++ )
	{
	DcdSteBlk_(ctx,i) = DcdInitializeState();
	if( !IsPointer_(DcdSteBlk_(ctx,i))) return((int) DcdSteBlk_(ctx,i) );
	}
    /*
    **	Create a destination descriptor per component.
    */
    for( i = 0; i < DcdDstCnt_(ctx); i++ )
	{
	h = DctK_BlockSize < SnkHeight_(DcdDst_(ctx),i)
	  ? DctK_BlockSize : SnkHeight_(DcdDst_(ctx),i);
	DcdDstDat_(ctx,i) = DdxRmAllocData_( DcdDst_(ctx), i, 0, h );
	if( !IsPointer_(DcdDstDat_(ctx,i)) )
	    return((int)DcdDstDat_(ctx,i));
	DcdDstUdp_(ctx,i) = DatUdpPtr_(DcdDstDat_(ctx,i));
	}

    return( status );
}				    /* end of _DecodeDctInitialize */
#endif

/*****************************************************************************
**  _DecodeDctActivate
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
*****************************************************************************/
#if	defined(XIESMI)
static int  _DecodeDctActivate( ctx )
 DecodeDctPipeCtxPtr  ctx;
{
    PipeDataPtr		  src;
    unsigned char	 *tmp;
    int c, h, i, p, s, y, pos;
    /*
    **	If we already have source data see if we can continue to process it.
    */
    if( VecNoInpt_(CtxVecPtr_(ctx)) && DdxPipeHasYielded_(CtxHead_(ctx)) )
	{   /*
	    **	We already have source data but aren't allowed to process it.
	    */
	if( !DatOwned_(DcdSrcDat_(ctx)) )
	    {	/*
		**  Move the unprocessed data to a buffer of our own.
		*/
	    src = DcdSrcDat_(ctx);
	    tmp = DatBase_(src);
	    p   = DatCmpIdx_(src);
	    pos = DcdCurPos_(DcdSteBlk_(ctx,p));
	    DcdCurPos_(DcdSteBlk_(ctx,p)) = 0;
	    DatArSize_(src)		 -= pos << 3;
	    DatOwned_(src)		  = TRUE;
	    DatPos_(src)		  = 0;
	    DatBase_(src)		  = DdxMallocBits_( DatArSize_(src) );
	    if( DatBase_(src) == NULL )  return( DctX_BadAlloc );
	    memcpy( DatBase_(src), tmp+pos, DatArSize_(src)>>3 );
	    }
	return( DctX_Success );
	}

    do	{
	if( !VecNoInpt_(CtxVecPtr_(ctx)) )
	    {	/*
		**  Get a chunk of encoded data.
		*/
	    DcdSrcDat_(ctx) = DdxRmGetData_( ctx, DcdDrn_(ctx) );
	    if( !IsPointer_(DcdSrcDat_(ctx)) ) return( (int) DcdSrcDat_(ctx) );
	    }

	/*
	**  Call the decoder.
	*/
	p =  DatCmpIdx_(DcdSrcDat_(ctx));
	if (IsPointer_(DcdDstDat_(ctx,p)) )
	    {
	    s = DcdDecodeSegment( DatUdpPtr_(DcdSrcDat_(ctx)),
			      &DcdDstUdp_(ctx,p),
			       DcdDcdCnt_(ctx),
			       DcdSteBlk_(ctx,p) );
	    }
	else
	    {
	    /*
	    ** No destination data segment means we have already returned the
	    ** the last scanline of the image, or there is a bug somewhere.
	    ** Tolerate the client being confused or having a slightly bogus
	    ** compressed data stream.
	    */
	    DcdSrcDat_(ctx) = DdxRmDeallocData_( DcdSrcDat_(ctx) );
	    VecNoInpt_(CtxVecPtr_(ctx)) = FALSE;
	    return( DctX_Success );
	    }

	switch( s )
	    {
	case DctK_StatusDone   :
	case DctK_StatusOutput :
	    /*
	    **	Pass the PCM data along for the number of components decoded.
	    */
	    for( i = 0; i < DcdDcdCnt_(ctx); i++ )
		{
		c = p > 0 ? p : i;
		y = DatY2_(DcdDstDat_(ctx,c));
		s = DdxRmPutData_( DcdDst_(ctx), DcdDstDat_(ctx,c) );
		if( s == DctX_Success && y++ < SnkY2_(DcdDst_(ctx),c) )
		    {	/*
			**  Allocate space for another quantum of decoded data.
			*/
		    h = DctK_BlockSize < SnkHeight_(DcdDst_(ctx),i) - y
		      ? DctK_BlockSize : SnkHeight_(DcdDst_(ctx),i) - y;
		    DcdDstDat_(ctx,c) = DdxRmAllocData_(DcdDst_(ctx), c, y, h);
		    if( IsPointer_(DcdDstDat_(ctx,c)) )
			DcdDstUdp_(ctx,c) = DatUdpPtr_(DcdDstDat_(ctx,c));
		    else
			s = (int) DcdDstDat_(ctx,c);
		    }
		else
		    DcdDstDat_(ctx,c) = NULL;
		if( s != DctX_Success ) break;
		}
	    VecNoInpt_(CtxVecPtr_(ctx)) = IsPointer_(DcdDstDat_(ctx,c));
	    return( s );
	case DctK_StatusInput :
	    /*
	    **	We're done with that chunk -- prepare for more.
	    */
	    DcdSrcDat_(ctx) = DdxRmDeallocData_( DcdSrcDat_(ctx) );
	    VecNoInpt_(CtxVecPtr_(ctx)) = FALSE;
	    break;
	default : return( DctX_DecodeFail );
	    }
	}
    while( s == DctK_StatusInput );

    return( DctX_Success );
}				    /* end of _DecodeDctActivate */
#endif

/*****************************************************************************
**  _DecodeDctAbort
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
*****************************************************************************/
#if	defined(XIESMI)
static int  _DecodeDctAbort( ctx )
 DecodeDctPipeCtxPtr  ctx;
{
    int i;
    /*
    **	Destroy decode state(s).
    */
    for( i = 0; i < DcdSrcCnt_(ctx); i++ )
	DcdSteBlk_(ctx,i) = DcdDestroyState( DcdSteBlk_(ctx,i) );
    /*
    **	Turn off the no-input flag.
    */
    VecNoInpt_(CtxVecPtr_(ctx)) = FALSE;
    /*
    **	Destroy destination data descriptor(s).
    */
    for( i = 0; i < DcdDstCnt_(ctx); i++ )
	DcdDstDat_(ctx,i) = DdxRmDeallocData_( DcdDstDat_(ctx,i) );

    return( DctX_Success );
}				    /* end of _DecodeDctAbort */
#endif

/*****************************************************************************
**  _DecodeDctDestroy
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
*****************************************************************************/
#if	defined(XIESMI)
static int  _DecodeDctDestroy( ctx )
 DecodeDctPipeCtxPtr  ctx;
{
    DcdDrn_(ctx) = DdxRmDestroyDrain_( DcdDrn_(ctx) );

    return( DctX_Success );
}				    /* end of _DecodeDctDestroy */
#endif

/*****************************************************************************
**  DcdInitializeState
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static DecodeDctStatePtr DcdInitializeState()
{
    DecodeDctStatePtr state;
    int   i;
    /*
    **	Allocate new state block.
    */
    state = (DecodeDctStatePtr) DctCalloc_(1,sizeof(DecodeDctStateRec));
    /*
    **	Return on bad alloc...
    */
    if( state == NULL ) return( (DecodeDctStatePtr) DctX_BadAlloc );
    /*
    **	Initialize global locations
    */
    DcdHacTbl_(state,0) = &DctR_AcDecodeTable[0];   /* Default AC tables    */
    DcdHacTbl_(state,1) = &DctR_AcDecodeTable[1];
    DcdHdcTbl_(state,0) = &DctR_DcDecodeTable[0];   /* Default DC tables    */
    DcdHdcTbl_(state,1) = &DctR_DcDecodeTable[1];

    DcdQntTbl_(state,0) = &DctR_QuantDefault[0];    /* Default quantization */
    DcdQntDef_(state,0) = TRUE;			    /* tables.		    */
    DcdQntTbl_(state,1) = &DctR_QuantDefault[1];    /*  		    */
    DcdQntDef_(state,1) = TRUE;
    /*
    **	Allocate initial space for token data buffer.
    */
    DcdDatPtr_(state)   = (unsigned char *) DctMalloc_( DcdK_DataSize );
    if( DcdDatPtr_(state) == NULL )
	{
	DctFree_( state );
	return( (DecodeDctStatePtr) DctX_BadAlloc );
	}
    DcdDatSiz_(state)   = DcdK_DataSize;

    return( state );
}				    /* end of DcdInitializeState */

/*****************************************************************************
**  DcdDestroyState
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static DecodeDctStatePtr DcdDestroyState( state )
 DecodeDctStatePtr   state;
{
    int	    i;
    UdpPtr  udp;

#if	defined(XIESMI)
    if( !IsPointer_(state) ) return( NULL );
#endif

    if( DcdDatPtr_(state) != NULL )
	/*
	**  Free token data buffer.
	*/
	DctFree_( DcdDatPtr_(state) );

    /*
    **	Free any user defined quantization tables.
    */
    for( i = 0;  i < DctK_MaxQuantTables;  i++ )
        if( !DcdQntDef_(state,i) && DcdQntTbl_(state,i) != NULL )
            DctFree_(DcdQntTbl_(state,i));

    /*
    **	Free the state block.
    */
    DctFree_(state);

    return( NULL );
}				    /* end of DcdDestroyState */

/*****************************************************************************
**  DcdDecodeSegment
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int DcdDecodeSegment( cdp, udplst, udpcnt, state )
 UdpPtr		    cdp;
 UdpPtr		    udplst[];
 int		    udpcnt;
 DecodeDctStatePtr  state;
{
    int	 i, token;
    /*
    **	Set source in state block.
    */
    DcdCurPtr_(state) = cdp->UdpA_Base;
    DcdCurLen_(state) = cdp->UdpL_ArSize>>3;
    /*
    **	Set destination in state block.
    */
    for( i = 0;  i < udpcnt;  i++ )
	{
        DcdDstLst_(state,i) = udplst[i];
#if   defined(XIESMI)
        if( udplst[i]->UdpA_Base == NULL )
            {
            udplst[i]->UdpA_Base = DctMallocBits_( udplst[i]->UdpL_ArSize );
            if( udplst[i]->UdpA_Base == NULL ) return( DctX_BadAlloc );
            }
#elif defined(IPS)
	udplst[i]->UdpL_CompIdx = DctK_NoComponent;
#endif
	}
    if( DcdDstFul_(state) )
	{   /*
	    **  Continue decoding from where we left off.
	    */
	DcdDataAction(state);
	if( DcdDstFul_(state) )
	    return( DctK_StatusOutput );
	}
    else    /*
	    **  Assume new input.
	    */
	DcdCurPos_(state) = cdp->UdpL_Pos>>3;

    /*
    **	While tokens remain in the buffer, process...
    */
    while( TRUE )
	{   /*
	    **  Attempt to get the next token.
	    */
        token = DcdGetToken(state);

	if( DcdDatPtr_(state) == NULL )
	    return( DctX_BadAlloc );

        if( DcdInpExh_(state) )
	    /*
	    **  Input buffer exhausted.
	    */
	    return( DctK_StatusInput );
	/*
        **  Call transition action routine if specified.
	*/
        if( action[DcdState_(state)][token] != NULL  &&
          (*action[DcdState_(state)][token])(state) != DcdK_ActionSuccess )
	    return(DctK_StatusError);
	/*
	**  Transition to new state.
	*/
	DcdState_(state) = next_state[DcdState_(state)][token];

        if( DcdDstFul_(state) )
	    /*	 
	    **  No room in the inn.
	    */	 
	    return( DctK_StatusOutput );
        /*	 
        **  If at an end state, stop
        */	 
        switch( DcdState_(state) )
	    {
	case DctK_StateErr : return( DctK_StatusError );
	case DctK_StateEOI : return( DctK_StatusDone  );
	default		   : continue;
	    }
	}
}				    /* end of DcdDecodeSegment */

/*****************************************************************************
**  DcdGetToken
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
**
*****************************************************************************/
static int DcdGetToken(state)
 DecodeDctStatePtr   state;
{
    unsigned char    *dat, *src;
    int   token, len, pos,  cnt = 1;
    int   current = DcdData_(state) || !DcdInpExh_(state)
				    ||  DcdTokLen_(state) == 0;
    /*
    **	Point to current or saved token data.
    */
    src = current ? DcdCurPtr_(state) : DcdDatPtr_(state);
    len = current ? DcdCurLen_(state) : DcdTokLen_(state);
    pos = current ? DcdCurPos_(state) : 0;

    while( DataAvailable_( state, src, len, pos, cnt ) )
	{
	if( !(DcdData_(state) || DcdMarker_(state)) )
	      DcdData_(state)  = src[pos] != DctK_Marker;

	switch( DcdData_(state) ? 0 : src[pos] )
	    {
	case DctK_Marker     : pos++;  DcdMarker_(state) = TRUE;  continue;

	case DctK_MarkerSOI  : token = DctK_TokenSOI; break;

	case DctK_MarkerSOF  : cnt   = 0;
	case DctK_MarkerSOS  : token = cnt ? DctK_TokenSOS : DctK_TokenSOF;
	    /*
	    **	The next 2 bytes express the length of additional token data.
	    */
	    if( !DataAvailable_( state, src, len, pos, 3 ) )
		return( DctK_TokenNone );   /* SOF/SOS length unavailable   */
	    cnt  = src[pos+1]  + 1;
	    cnt += src[pos+2] << 8;
	    if( !DataAvailable_( state, src, len, pos, cnt ) )
		return( DctK_TokenNone );   /* SOF/SOS data unavailable	    */
	    DcdTokPtr_(state) = src;
	    DcdTokPos_(state) = pos + 3;
	    DcdTokLen_(state) = pos + cnt;
	    break;

	case DctK_MarkerRSC0 :
	case DctK_MarkerRSC1 :
	case DctK_MarkerRSC2 :
	case DctK_MarkerRSC3 :
	case DctK_MarkerRSC4 :
	case DctK_MarkerRSC5 :
	case DctK_MarkerRSC6 :
	case DctK_MarkerRSC7 : token = DctK_TokenRSC; break;

	case DctK_MarkerEOI  : token = DctK_TokenEOI; break;

	case 0 :
	    if( !DcdData_(state) )
		{   /*
		    **	Initial data was 0xFF00.
		    */
		DcdCurPos_(state) = pos;
		DcdTokLen_(state) = 1;
	       *DcdDatPtr_(state) = 0xFF;
		}
	    else if( !DcdInpExh_(state) )
		DcdTokLen_(state) = 0;
	    /*
	    **  Unstuff, copy, and count source data.
	    */
	    return( DcdGetData( state ) );

	default : return( DctK_TokenError );
	    } /* end switch */

	if( !DcdMarker_(state) )
	    token = DctK_TokenError;	 /* token not preceeded by a marker */
	DcdMarker_(state) = FALSE;
	DcdInpExh_(state) = FALSE;
	if( current )
	    DcdCurPos_(state) = pos+cnt; /* update pos to next marker/data  */
	return( token );
	} /* end while */

    return( DctK_TokenNone );			       /* no data available */
}				    /* end of DcdGetToken */

/*****************************************************************************
**  DcdGetData
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
**
*****************************************************************************/
static int DcdGetData( state )
 DecodeDctStatePtr  state;
{
    unsigned char *src = DcdCurPtr_(state), *dat = DcdDatPtr_(state);
    int pos, mark, len = DcdCurLen_(state),  cnt = DcdTokLen_(state);

    /*
    **  Unstuff, copy, and count source data.
    */
    for( mark = DcdMarker_(state), pos = DcdCurPos_(state);  pos < len;  pos++ )
	{
	if( mark )
	    if( mark = src[pos] != 0 )
		break;				    /* end of data segment  */
	    else
		continue;			    /* strip 00 from 0xFF00 */

	if( DcdDatSiz_(state) == cnt )
	    {
	    dat = DcdExtendData( state, cnt + DcdK_DataSize );
	    if( dat == NULL ) return( DctK_TokenError );
	    }
	dat[cnt] = src[pos];
	mark	 = dat[cnt++] == DctK_Marker;
	}
    /*
    **  End of data parsing -- either input exhausted or at next marker.
    */
    DcdInpExh_(state) = pos == len;
    DcdMarker_(state) = mark;
    DcdCurPos_(state) = pos;
    DcdTokLen_(state) = cnt;
    DcdTokPtr_(state) = dat;
    DcdTokPos_(state) = 0;
    DcdData_(state)   = DcdInpExh_(state);

    return( DctK_TokenData );
}				    /* end of DcdGetData */

/*****************************************************************************
**  DcdExtendData
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
**
*****************************************************************************/
static unsigned char *DcdExtendData( state, data_len )
 DecodeDctStatePtr  state;
 int		    data_len;
{
    /*
    **  Extend the token data buffer by a multiple of DcdK_DataSize.
    */
    do DcdDatSiz_(state) += DcdK_DataSize;
    while( DcdDatSiz_(state)  < data_len );

#ifdef __alpha
    DcdDatPtr_(state) = (unsigned char *) DctRealloc_( state->data_ptr,
						       state->data_siz );
#else
    DcdDatPtr_(state) = (unsigned char *) DctRealloc_( DcdDatPtr_(state),
						       DcdDatSiz_(state) );
#endif
    return( DcdDatPtr_(state) );
}				    /* end of DcdExtendData */

/*****************************************************************************
**  DcdSaveToken
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
**	Boolean:    TRUE = sufficient data remains.
**
*****************************************************************************/
static int DcdSaveToken( state, ptr, len, pos, more )
 DecodeDctStatePtr   state;
 unsigned char	   **ptr;
 int		    *len, pos, more;
{
    int need, have, copy, total;

    switch( DcdInpExh_(state) )
	{
    case FALSE :				/* save all that remains    */
	DcdInpExh_(state) = TRUE;
	DcdCurPos_(state) = pos;
	DcdTokPos_(state) = 0;
       *len		  = 0;

    case TRUE :					/* transfer what is needed  */
	total = pos + more;
	need  = total - *len;
	have  = DcdCurLen_(state) - DcdCurPos_(state);
	copy  = have < need ? have : need;

	if( copy > 0 )
	    {
	    if( DcdDatSiz_(state) < total && DcdExtendData(state,total)==NULL )
		return( FALSE );

	    memcpy( DcdDatPtr_(state) + *len,
		    DcdCurPtr_(state) +  DcdCurPos_(state), copy );
	   *len		      += copy;
	    DcdCurPos_(state) += copy;
	    }
	}
   *ptr		       =  DcdDatPtr_(state);
    DcdTokPtr_(state)  =  DcdDatPtr_(state);
    DcdTokLen_(state)  = *len;

    return( copy == need );
}				    /* end of DcdSaveToken */

/*****************************************************************************
**  DcdDataAction
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
**
*****************************************************************************/
static int DcdDataAction(state)
 DecodeDctStatePtr state;
{
    ZQ_VARIABLES
    UdpPtr   udp;
    short int work1[64], work2[64], previous_dc[DctK_MaxComponents];
    int scn_stride[DctK_MaxComponents], pix_stride[DctK_MaxComponents];
    int status, h, v, i, c, row, col, dstrow, dstcol, x, y, pos;
    unsigned char *base[DctK_MaxComponents], tmp[64];
    unsigned char *scnptr, *pxlptr, *tmpptr, *dstptr;
    /*
    **	Make sure the destination UDP can accept the output of the current
    **	segment -- if not, set destination full and return.
    */
    for( i = 0;  i < DcdCmpCnt_(state);  i++ )
	{
	udp = DcdDstLst_(state,i);
#if	defined(XIESMI)
        if( udp == NULL ) return( DcdK_ActionError ); else
#elif	defined(IPS)
	status = IpsSetUdp(state,DcdDstIdx_(state,i));
	if( status != DctX_Success ) return( status );
#endif
	DcdUdpFul_(state,DcdDstIdx_(state,i)) =
		udp->UdpL_Y2 < DcdScnSeg_(state,i) - 1 + DcdCurY_(state,i) &&
		udp->UdpL_Y2 < DcdCmpHgt_(state,i) - 1;
	}
    if( DcdDstFul_(state) )
	return( DcdK_ActionSuccess );
    /*              
    **  Assign local pointers to speed things up inside loops
    */
    for( c = 0; c < DcdCmpCnt_(state); c++ )
	{
	udp = DcdDstLst_(state,DcdDstIdx_(state,c));

	scn_stride[c] = udp->UdpL_ScnStride / 8;
	pix_stride[c] = udp->UdpL_PxlStride / 8;
	base[c] = udp->UdpA_Base + udp->UdpL_Pos / 8;
	previous_dc[c] = 0;
	}
    /*
    **	This is the main decompression loop.
    **	    -- Once around for each MDU in this code segment --
    */
    for( pos = 0, i = 0;  i < DcdRstInt_(state);  i++ )
	{   /*
	    **  For each component in the MDU
	    */
    	for( c = 0;  c < DcdCmpCnt_(state);  c++ )
	    {
	    x = DcdCurX_(state,c);
	    y = DcdCurY_(state,c);
	    udp = DcdDstLst_(state,DcdDstIdx_(state,c));
	    /*
	    **	Calculate address to start of tile.
	    */
	    scnptr = base[c] + (y - udp->UdpL_Y1) * scn_stride[c]
			     + (x - udp->UdpL_X1) * pix_stride[c];
	    /*
	    **	For each vertical sample of this component.
	    */
	    for( v = 0;  v < DcdCmpVrt_(state,c);  v++ )
		{
	        pxlptr = scnptr;
		/*
		**  For each horizontal sample in this component.
		*/
	    	for( h = 0;  h < DcdCmpHrz_(state,c);  h++ )
		    {	/*
			**  Turn huffman codes into quantized coefficients.
			*/
		    DcdDecode( DcdTokPtr_(state), &pos,
			       DcdHdcTbl_(state,DcdCmpHdc_(state,c)),
			       DcdHacTbl_(state,DcdCmpHac_(state,c)),
			      &previous_dc[c], work1 );
		    /*
		    **	Dequantize and put coefficients in raster order.
		    */
#ifdef __alpha
            d_in_ptr  = (short int *)(work1) ;
            d_out_ptr = (short int *)(work2) ;
            d_q_ptr   = (unsigned char *)(DcdQntTbl_(state,DcdCmpQnt_(state,c))) ;
            d_z_ptr   = (short int *)(DctAW_ZigZag) ;
	    for( d_i=0 ; d_i<=63 ; d_i++ )
              *(d_out_ptr+(*d_z_ptr++)) = (*d_in_ptr++) * (*d_q_ptr++) ;
#else
		    DEQUANT_ZIGZAG(work1,
				   work2,
				   DctAW_ZigZag,
				   DcdQntTbl_(state,DcdCmpQnt_(state,c)));
#endif
		    /*
		    **	If this block goes beyond the limits of the image,
		    **	buffer it in a temporary, then copy it to the dst.
		    */
                    if( x + DctK_BlockSize - 1 > udp->UdpL_X2
		     || y + DctK_BlockSize - 1 > udp->UdpL_Y2 )
			{
                        IDCT8x8_16S_8U( work2, tmp, 8, 1 );
			/*
			**  Calculate remaining rows and columns...if greater
			**  than 8, then set to maximum.
			*/
			dstrow = udp->UdpL_Y2 - y + 1;
                        if( dstrow > 8 )
			    dstrow = 8;
			dstcol = udp->UdpL_X2 - x + 1;
                        if( dstcol > 8 )
			    dstcol = 8;
			/*
			**  Copy from temp buffer into image buffer.
			*/
                        for( row = 0;  row < dstrow;  row++ )
			    {
			    tmpptr = tmp + 8 * row;
			    dstptr = pxlptr + scn_stride[c] * row;
                            for( col = 0;  col < dstcol;  col++ )
				{
                                *dstptr  = *tmpptr++;
				 dstptr += pix_stride[c];
				}
			    }
			}
                    else
			/*
			**  Do inverse DCT directly into image buffer.
			*/
			IDCT8x8_16S_8U( work2, pxlptr, scn_stride[c],
						       pix_stride[c] );
		    /*
		    **	Update pointer to next horizontal sample.
		    */
		    pxlptr += 8 * pix_stride[c];
		    x      += 8;
		    }
		/*
		**  Update pointer to next vertical sample.
		*/
		scnptr += 8 * scn_stride[c];
		y      += 8;
		}
	    /*
	    **	Update current X,Y location pointers. If at end of scan, reset
	    **	X pointer and increment Y pointer.
	    */
	    DcdCurX_(state,c) += 8 * DcdCmpHrz_(state,c);
	    if( DcdCurX_(state,c) >= DcdCmpWid_(state,c) )
		{
	    	DcdCurX_(state,c)  = 0;
		DcdCurY_(state,c) += 8 * DcdCmpVrt_(state,c);
		}
	    }
        if( DcdCurY_(state,0) >= DcdCmpHgt_(state,0) )
	    break;
	}
    return( DcdK_ActionSuccess );
}				    /* end of DcdDataAction */

/*****************************************************************************
**  DcdSosAction
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
**
*****************************************************************************/
static int DcdSosAction(state)
 DecodeDctStatePtr state;
{
    unsigned char   *ptr = DcdTokPtr_(state) + DcdTokPos_(state);
    int		     current_component;
    unsigned char    huff_specifier;
    int		     huff_idx;
    int		     huff_sel;
    int		     i;
    /*
    **	Reset resync enable for each scan.
    */
    DcdRstEnb_(state) = FALSE;
    /*
    **	First byte specifies the component to be worked on.
    */
    current_component = *ptr++;
    /*
    **	Read per component huffman table assignments. One byte per component.
    **	The DC index is in the low four bits, AC index in the high four bits.
    */
    for( i = 0;  i < DcdCmpCnt_(state);  i++ )
	{
        DcdCmpHdc_(state,i) = *ptr & 0x0F;
	DcdCmpHac_(state,i) = *ptr++ >> 4 & 0x0F;
	}
    /*
    **	Read huffman table specifiers. Since compression doesn't generate
    **	custom huffman tables, ignore that capability for now...
    */
    while((huff_specifier = *ptr++) != 0x80)
	{   /*
	    **  The huffman specifier contains the huffman table index
	    **  in the low nibble, and selector in the high nibble.
	    */
        huff_idx = huff_specifier & 0x0F;
	huff_sel = huff_specifier >> 4 & 0x0F;
        /*	 
        **  Take action based on selector value.
        */	 
        switch( huff_sel )
	    {
	case 0 : /* these cases call for loading private tables...NYI */
	case 1 : return( DcdK_ActionError );

	case 2:	/*
		**  Load default DC huffman table.
		*/
	    DcdHdcTbl_(state,huff_idx) = &DctR_DcDecodeTable[0];
	    DcdSdcTbl_(state,huff_idx) = &DctR_DcSpecTable[0];
	    break;

	case 3: /*
		**  Load default AC huffman table.
		*/
	    DcdHacTbl_(state,huff_idx) = &DctR_AcDecodeTable[0];
	    DcdSacTbl_(state,huff_idx) = &DctR_AcSpecTable[0];
	    break;

	default : return( DcdK_ActionError );
	    }
	}
    /*
    **	Reset horizontal and vertical interleave counters.
    */
    DcdHrzCnt_(state) = 0;
    DcdVrtCnt_(state) = 0;

    return( DcdK_ActionSuccess );
}				    /* end of DcdSosAction */

/*****************************************************************************
**  DcdSofAction
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
**
*****************************************************************************/
static int DcdSofAction(state)
 DecodeDctStatePtr state;
{
    unsigned char   *ptr = DcdTokPtr_(state) + DcdTokPos_(state);
    int		     vert_max = 0;
    int		     horz_max = 0;
    int		     i;
    int		     j;
    char	     qntidx;
    char	     qntprc;
    /*
    **	Parse SOF fixed header fields.
    */
    DcdMode_(state)    = *ptr++;
    DcdPrec_(state)    = *ptr++;
    DcdFrmHgt_(state)  = *ptr++;
    DcdFrmHgt_(state) += *ptr++ << 8;
    DcdFrmWid_(state)  = *ptr++;
    DcdFrmWid_(state) += *ptr++ << 8;
    DcdRstInt_(state)  = *ptr++;
    DcdRstInt_(state) += *ptr++ << 8;
    /*
    **	Get per component information.
    */
    DcdCmpCnt_(state) = *ptr++;
    for( i = 0;  i < DcdCmpCnt_(state);  i++ )
	{   /*
	    **  Get component index from DCT stream. Determine which UDP
	    **  corresponds to this component.
	    */
        DcdCmpIdx_(state,i) = *ptr++;

        for( j = 0;  j < DcdCmpCnt_(state);  j++ )
            if (DcdDstLst_(state,j)->UdpL_CompIdx == DcdCmpIdx_(state,i))
		break;
	    else if (DcdDstLst_(state,j)->UdpL_CompIdx == DctK_NoComponent)
		{
		DcdDstLst_(state,j)->UdpL_CompIdx = DcdCmpIdx_(state,i);
		break;
		}
        if( j < DcdCmpCnt_(state) )
            DcdDstIdx_(state,i) = j;
        else
            return( DcdK_ActionError );
	/*
	**  Get sampling rates.
	*/
	DcdCmpVrt_(state,i) = *ptr & 0x0F;
	DcdCmpHrz_(state,i) = *ptr++ >> 4 & 0x0F;
	
        if( vert_max < DcdCmpVrt_(state,i) )
	    vert_max = DcdCmpVrt_(state,i);
        if( horz_max < DcdCmpHrz_(state,i) )
	    horz_max = DcdCmpHrz_(state,i);
	}
    /*
    **	Calculate sizes for interleaving.
    */
    DcdIntCol_(state) = (DcdFrmWid_(state)+(8*horz_max-1)) / (8*horz_max);
    DcdIntLin_(state) = (DcdFrmHgt_(state)+(8*vert_max-1)) / (8*vert_max);
    /*
    **	Now, compute component sizes based on subsampling frequency.
    */
    for( i = 0;  i < DcdCmpCnt_(state);  i++ )
	{
        DcdCmpWid_(state,i) = DcdFrmWid_(state)*DcdCmpHrz_(state,i)/horz_max;
        DcdCmpHgt_(state,i) = DcdFrmHgt_(state)*DcdCmpVrt_(state,i)/vert_max;
	DcdCmpStr_(state,i) = DcdIntCol_(state) * 8 * DcdCmpHrz_(state,i);
	DcdCmpLen_(state,i) = DcdIntLin_(state) * 8 * DcdCmpVrt_(state,i);
	/*
	**  Initially, scans per segement is entire image (for no resync).
	*/
	DcdScnSeg_(state,i) = DcdCmpHgt_(state,i);
	}
    /*
    **	Next, extract per component quantization table assignments.
    **	These values are four bits each packed two per byte.
    */
    switch( DcdCmpCnt_(state) )
	{
    case 3  : DcdCmpQnt_(state,2) = *(ptr+1) & 0x0F;
    case 2  : DcdCmpQnt_(state,1) = (*ptr >> 4) & 0x0F;
    case 1  : DcdCmpQnt_(state,0) = *ptr & 0x0F;
	      break;
    default : return( DcdK_ActionError );
	}
    ptr += (DcdCmpCnt_(state) + 1) / 2;
    /*
    **	Now, comes quantization table specifiers. 
    */
    while( *ptr++ != 0x80 )
	{
        qntidx = *(ptr-1) & 0x0F;
	qntprc = *(ptr-1) >> 4 & 0x0F;
	/*
	**  If this slot contains a non-default quantization table,
	**  deallocate the memory.
	*/
	if( DcdQntTbl_(state,qntidx) != NULL && !DcdQntDef_(state,qntidx) )
	    DctFree_(DcdQntTbl_(state,qntidx));
	/*
	**  Read in user defined quantization table.
	*/
	DcdQntTbl_(state,qntidx) = (QuantTablePtr)
				    DctMalloc_(sizeof(QuantTable));
        if( DcdQntTbl_(state,qntidx) == NULL ) return( DctX_BadAlloc );

	DcdQntDef_(state,qntidx) = FALSE;
	memcpy( DcdQntTbl_(state,qntidx), ptr, sizeof(QuantTable) );
	ptr += sizeof(QuantTable);
	}
    /*
    **	Now, validate what we found in the JPEG stream against what's being
    **	specified by the caller.
    */

    /*
    **	Return from wence we came...
    */
    return( DcdK_ActionSuccess );
}				    /* end of DcdSofAction */

/*****************************************************************************
**  DcdRscAction
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
**
*****************************************************************************/
static int DcdRscAction(state)
 DecodeDctStatePtr state;
{
    int	i;
    
    DcdRstEnb_(state) = TRUE;

    for( i = 0;  i < DcdCmpCnt_(state);  i++ )
	/*
	**  NOTE : Sampling rate needs to be considered here...
	*/
	DcdScnSeg_(state,i) = DcdRstInt_(state)   * DctK_BlockSize
		    	    / DcdCmpWid_(state,i) * DctK_BlockSize;

    return( DcdK_ActionSuccess );
}				    /* end of DcdRscAction */

/*****************************************************************************
**  DcdDecode
**
**  FUNCTIONAL DESCRIPTION:
**	Huffman decodes one block of quantized zigzag scanned DCT coefs
**	(most bit banging is done by the macros GetBit_ and GET_VALUE_).
**
**  FORMAL PARAMETERS:
**
**
*****************************************************************************/
static void	 DcdDecode(inptr,bitoff,dc_table,ac_table,prv_dc,coef)
 unsigned char	    *inptr;
 int		    *bitoff;
 HuffDecodeTablePtr  dc_table;
 HuffDecodeTablePtr  ac_table;
 short int	    *prv_dc;
 short int	    *coef;
{
    int		 i, coef_index;
    int		 dc_exp, dc_man;
    int		 ac_exp, ac_man, ac_run;

    int		 temp;
    short int	 stemp;
    /*
    **	Decode DC coefficient of block
    */
    dc_exp = DcdGetHuf( inptr, bitoff, dc_table );

    if( dc_exp & 15 )
	{
#ifndef MSB_FIRST
	dc_man   = GET_VALUE_( inptr, *bitoff, (1<<(dc_exp-1))-1 );
	*bitoff += dc_exp-1;
	temp     = GetBit_( inptr, *bitoff );
	(*bitoff)++;
#else
	temp	 = GetBit_( inptr, *bitoff );
	(*bitoff)++;
	dc_man   = GET_VALUE_( inptr, *bitoff, (1<<(dc_exp-1))-1 );
	*bitoff += dc_exp-1;
#endif
	if( temp == 0 ) 
	    dc_man += (-1 << dc_exp)+ 1;
	else
	    dc_man +=   1 <<(dc_exp - 1);
	}
    else if( dc_exp == 0 )
	dc_man = 0 ;
    else
	dc_man = -32768 ; /* i.e. dc_exp == 16 */

    *coef = dc_man + *prv_dc;
    *prv_dc = *coef++;

    /* Decode AC coefficients of block   */

    /* 
    **  coef_index points to the NEXT coef to be processed
    */
    for( coef_index = 1; coef_index <= 63; ) 
	{
	stemp = DcdGetHuf( inptr, bitoff, ac_table );

	ac_run = stemp >> 4;
	ac_exp = stemp & 15;	    /* stemp - 16 * ac_run ; */

	if( ac_exp == 0 )	    /* The 2 special codes  */
	    {
	    if( ac_run )	    /* Max-length-run (i.e. ac_run==15)  */
		for( i = 0; i <= 15; i++,coef_index++ )
		    *coef++ = 0;
	    else		    /* End-of-block (i.e.ac_run==0)      */
		for( ;coef_index <= 63; coef_index++ )
		    *coef++ = 0;
	    }
	else		    /* Regular codes  */
	    {
#ifndef MSB_FIRST
	    ac_man   = GET_VALUE_( inptr, *bitoff, (1<<(ac_exp-1))-1 ); 
	    *bitoff += ac_exp-1;
	    temp     = GetBit_( inptr, *bitoff );
	    (*bitoff)++;
#else
	    temp     = GetBit_( inptr, *bitoff );
	    (*bitoff)++;
	    ac_man   = GET_VALUE_( inptr, *bitoff, (1<<(ac_exp-1))-1 ); 
	    *bitoff += ac_exp-1;
#endif
	    if( temp == 0 )
		ac_man += (-1<< ac_exp)+ 1;
	    else
		ac_man +=   1<<(ac_exp - 1);

	    for( i = 0; i < ac_run; i++, coef_index++ )
		*coef++ = 0;
	    *coef++ = ac_man;  
	     coef_index++;
	    }
	} /* Done with a block  */
}				    /* end of DcdDecode */

/*****************************************************************************
**  DcdGetHuf 
**
**  FUNCTIONAL DESCRIPTION:
**
**	Fetches the next Huffman code from a bit stream pointed to by
**	bufptr/bufoff, using a set of codes specified by huff_decode_ptr.
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static short int DcdGetHuf( bufptr, bufoff, huff_decode_ptr ) 
 unsigned char	    *bufptr;
 int		    *bufoff;
 HuffDecodeTable    *huff_decode_ptr;
{
    int	    temp;
    int	    index = 0;
    short   rlen;

    do	{
	temp = GetBit_( bufptr, *bufoff );
	(*bufoff)++;
	if( temp )				    /* i.e., if temp==1 */
	    index += huff_decode_ptr->half_table_size;
	rlen  = huff_decode_ptr->huff_decode_entry[index].w1;
	index = huff_decode_ptr->huff_decode_entry[index].w0;
	}
    while( index );

    return( rlen );
}				    /* end of DcdGetHuf  */

/*****************************************************************************
**  IDCT8x8_16S_8U
**
**  FUNCTIONAL DESCRIPTION:
**
**	Inverse DCT routine.
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int	 IDCT8x8_16S_8U(COEFS,PIXELS,SCAN_WIDTH,PEL_WIDTH)
 short int	*COEFS;
 unsigned char	*PIXELS;
 int		 SCAN_WIDTH;
 int		 PEL_WIDTH;
{
    DCT_VARIABLES

    d_ps1 = COEFS;
    d_pi1 = d_work;

    for(d_k=0 ; d_k<8; d_k++)
    {
	 d_ap = d_a;
	*d_ap++ = (d_ps1[0])<<2;
	*d_ap++ = (d_ps1[4*8])<<2;
	*d_ap++ = (d_ps1[2*8])<<2;
	*d_ap++ = (d_ps1[6*8])<<2;
	*d_ap++ = ((d_spi16*d_ps1[1*8]  - d_s7pi16*d_ps1[7*8]) + (1<<13))>>14;
	*d_ap++ = ((d_s5pi16*d_ps1[5*8] - d_s3pi16*d_ps1[3*8]) + (1<<13))>>14;
	*d_ap++ = ((d_c3pi16*d_ps1[3*8] + d_c5pi16*d_ps1[5*8]) + (1<<13))>>14;
	*d_ap   = ((d_cpi16*d_ps1[1*8]  + d_c7pi16*d_ps1[7*8]) + (1<<13))>>14;
	 d_ps1++;

	 d_bp = d_b;
	*d_bp++ = (d_cpi4*(d_a[0]+d_a[1])         + 32768)>>16;
	*d_bp++ = (d_cpi4*(d_a[0]-d_a[1])         + 32768)>>16;
	*d_bp++ = (d_spi8*d_a[2] - d_s3pi8*d_a[3] + 32768)>>16;
	*d_bp++ = (d_cpi8*d_a[2] + d_c3pi8*d_a[3] + 32768)>>16;
	*d_bp++ = d_a[4] + d_a[5];
	*d_bp++ = d_a[4] - d_a[5];
	*d_bp++ = d_a[7] - d_a[6];
	*d_bp   = d_a[7] + d_a[6];

	 d_cp = d_c;
	*d_cp++ = d_b[0] + d_b[3];
	*d_cp++ = d_b[1] + d_b[2];
	*d_cp++ = d_b[1] - d_b[2];
	*d_cp++ = d_b[0] - d_b[3];
	*d_cp++ = d_b[4];
	*d_cp++ = (d_cpi4*(d_b[6]-d_b[5])        + 32768)>>16;
	*d_cp++ = (d_cpi4*(d_b[6]+d_b[5])        + 32768)>>16;
	*d_cp   = d_b[7];

	 d_pi2 = d_c;
	 d_pi3 = d_c + 7;
	*d_pi1++ = ((*d_pi2++) + (*d_pi3--)   + 1) >> 1;
	*d_pi1++ = ((*d_pi2++) + (*d_pi3--)   + 1) >> 1;
	*d_pi1++ = ((*d_pi2++) + (*d_pi3--)   + 1) >> 1;
	*d_pi1++ = ((*d_pi2  ) + (*d_pi3  )   + 1) >> 1;
	*d_pi1++ = ((*d_pi2--) - (*d_pi3++)   + 1) >> 1;
	*d_pi1++ = ((*d_pi2--) - (*d_pi3++)   + 1) >> 1;
	*d_pi1++ = ((*d_pi2--) - (*d_pi3++)   + 1) >> 1;
	*d_pi1++ = ((*d_pi2  ) - (*d_pi3  )   + 1) >> 1;
    }

    d_pi2 = d_work;
    d_pi4 = d_c;
    d_pi5 = d_c + 7;
    d_puc2 = PIXELS;

    for(d_k=0 ; d_k <8; d_k++)
    {
	 d_ap = d_a;
	*d_ap++ = d_pi2[0];
	*d_ap++ = d_pi2[4*8];
	*d_ap++ = d_pi2[2*8];
	*d_ap++ = d_pi2[6*8];
	*d_ap++ = (d_spi16*d_pi2[1*8]  - d_s7pi16*d_pi2[7*8] + 32768)>>16;
	*d_ap++ = (d_s5pi16*d_pi2[5*8] - d_s3pi16*d_pi2[3*8] + 32768)>>16;
	*d_ap++ = (d_c3pi16*d_pi2[3*8] + d_c5pi16*d_pi2[5*8] + 32768)>>16;
	*d_ap   = (d_cpi16*d_pi2[1*8]  + d_c7pi16*d_pi2[7*8] + 32768)>>16;
	 d_pi2++;

	 d_bp = d_b;
	*d_bp++ = (d_cpi4*(d_a[0]+d_a[1])         + 32768)>>16;
	*d_bp++ = (d_cpi4*(d_a[0]-d_a[1])         + 32768)>>16;
	*d_bp++ = (d_spi8*d_a[2] - d_s3pi8*d_a[3] + 32768)>>16;
	*d_bp++ = (d_cpi8*d_a[2] + d_c3pi8*d_a[3] + 32768)>>16;
	*d_bp++ = d_a[4] + d_a[5];
	*d_bp++ = d_a[4] - d_a[5];
	*d_bp++ = d_a[7] - d_a[6];
	*d_bp   = d_a[7] + d_a[6];

	 d_cp = d_c;
	*d_cp++ = (d_b[0] + d_b[3]              + 4      )>>3;
	*d_cp++ = (d_b[1] + d_b[2]              + 4      )>>3;
	*d_cp++ = (d_b[1] - d_b[2]              + 4      )>>3;
	*d_cp++ = (d_b[0] - d_b[3]              + 4      )>>3;
	*d_cp++ = (d_b[4]                       + 4      )>>3;
	*d_cp++ = (d_cpi4*(d_b[6]-d_b[5])       + (1<<18))>>19;
	*d_cp++ = (d_cpi4*(d_b[6]+d_b[5])       + (1<<18))>>19;
	*d_cp   = (d_b[7]                       + 4      )>>3;

	d_puc1 = d_puc2;
	if( (d_temp = (*d_pi4++) + (*d_pi5--)) > 127) 
	    *d_puc1 = 255;
	else if(d_temp < -128)
	    *d_puc1 = 0;
	else
	    *d_puc1 = d_temp + 128;

	d_puc1 += PEL_WIDTH;                                       
	if( (d_temp = (*d_pi4++) + (*d_pi5--)) > 127)
	    *d_puc1 = 255;
	else if(d_temp < -128)
	    *d_puc1 = 0;
	else
	    *d_puc1 = d_temp + 128;

	d_puc1 += PEL_WIDTH;                                       
	if( (d_temp = (*d_pi4++) + (*d_pi5--)) > 127) 
	    *d_puc1 = 255;
	else if(d_temp < -128)
	    *d_puc1 = 0;
	else
	    *d_puc1 = d_temp + 128;

	d_puc1 += PEL_WIDTH;
	if( (d_temp = (*d_pi4)   + (*d_pi5))   > 127) 
	    *d_puc1 = 255;
	else if(d_temp < -128)
	    *d_puc1 = 0;
	else
	    *d_puc1 = d_temp + 128;

	d_puc1 += PEL_WIDTH;                                       
	if( (d_temp = (*d_pi4--) - (*d_pi5++)) > 127)
	    *d_puc1 = 255;
	else if(d_temp < -128)
	    *d_puc1 = 0;
	else
	    *d_puc1 = d_temp + 128;

	d_puc1 += PEL_WIDTH;                                       
	if( (d_temp = (*d_pi4--) - (*d_pi5++)) > 127) 
	    *d_puc1 = 255;
	else if(d_temp < -128)
	    *d_puc1 = 0;
	else
	    *d_puc1 = d_temp + 128;

	d_puc1 += PEL_WIDTH;                                       
	if( (d_temp = (*d_pi4--) - (*d_pi5++)) > 127)
	    *d_puc1 = 255;
	else if(d_temp < -128)
	    *d_puc1 = 0;
	else                     
	    *d_puc1 = d_temp + 128;

	d_puc1 += PEL_WIDTH;                                       
	if( (d_temp = (*d_pi4)   - (*d_pi5))   > 127) 
	    *d_puc1   = 255;
	else if(d_temp < -128)
	    *d_puc1   = 0;
	else
	    *d_puc1   = d_temp + 128;

	d_puc2   += SCAN_WIDTH;
    }
}				    /* end of IDCT8x8_16S_8U */

/*****************************************************************************
**  IpsSetupUdp
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
**
*****************************************************************************/
#ifdef IPS
static long	    IpsSetUdp(state,i)
 DecodeDctStatePtr  state;
 int		    i;
{
    UdpPtr  udp = DcdDstLst_(state,i);
    int	    min_arsize;
    /*
    **	Fill in UDP based on current component parameters.
    */
    udp->UdpW_PixelLength   = DcdPrec_(state);
    switch (DcdPrec_(state))
    {
        case 8:
            udp->UdpB_DType = UdpK_DTypeBU;
	    udp->UdpB_Class = UdpK_ClassA;
	    break;
        case 16:
            udp->UdpB_DType = UdpK_DTypeWU;
	    udp->UdpB_Class = UdpK_ClassA;
	    break;
        default:
            udp->UdpB_DType = UdpK_DTypeVU;
	    udp->UdpB_Class = UdpK_ClassUBA;
    }
    udp->UdpL_PxlStride	    = udp->UdpW_PixelLength;
    udp->UdpL_ScnStride	    = DcdCmpStr_(state,i) * 8;
    udp->UdpL_X1	    = 0;
    udp->UdpL_X2	    = DcdCmpWid_(state,i) - 1;
    udp->UdpL_Y1	    = 0;
    udp->UdpL_Y2	    = DcdCmpHgt_(state,i) - 1;
    udp->UdpL_PxlPerScn	    = DcdCmpWid_(state,i);
    udp->UdpL_ScnCnt	    = DcdCmpHgt_(state,i);
    udp->UdpL_CompIdx	    = DcdCmpIdx_(state,i);
    udp->UdpL_Levels	    = 1 << udp->UdpW_PixelLength;
    /*
    **	If buffer already exists, try to use it.
    */
    min_arsize = (udp->UdpL_ScnStride * udp->UdpL_ScnCnt + 7) / 8; 
    if (udp->UdpA_Base == NULL)
    {
	udp->UdpL_ArSize    = min_arsize * 8;
	udp->UdpL_Pos	    = 0;
	udp->UdpA_Base	    = 
	    (unsigned char *) DctMallocBits_(udp->UdpL_ArSize);
	if (udp->UdpA_Base == NULL)
	    return DctX_BadAlloc;
    }
    else if ((udp->UdpL_ArSize - udp->UdpL_Pos) < min_arsize)
    {
	return	DctX_OutBufSiz;
    }

    return DctX_Success;
}				    /* end of IpsSetupUdp */
#endif
