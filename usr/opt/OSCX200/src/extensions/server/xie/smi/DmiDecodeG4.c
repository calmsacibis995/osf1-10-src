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
**  Copyright (c) Digital Equipment Corporation, 1990 All Rights Reserved.
**  Unpublished rights reserved under the copyright laws of the United States.
**  The software contained on this media is proprietary to and embodies the
**  confidential technology of Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and media is authorized only
**  pursuant to a valid written license from Digital Equipment Corporation.

**  RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the U.S.
**  Government is subject to restrictions as set forth in Subparagraph
**  (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
**
**  Derived from work bearing the following copyright and permissions:
**
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

/******************************************************************************
**++
**  FACILITY:
**
**      X Imaging Extension
**      Digital Machine Independant DDX
**
**  ABSTRACT:
**
**      This module implements decompression of G4 Facsimile encoded data.
**
**  ENVIRONMENT:
**
**	VAX/VMS V5.3
**	ULTRIX  V3.1
**
**  AUTHORS:
**
**      Gary Grebus (cloned from SmiDecodeG4.c)
**	John Weber
**	Derived from code originally developed by Ken MacDonald
**
**  CREATION DATE:     
**
**	Fri Oct  5 16:01:43 1990
**
*******************************************************************************/

#include <stdio.h>

#include <X.h>
#include <XieDdx.h>
#include <XieUdpDef.h>

#include "DmiDecodeG4.h"
#include <SmiMemMgt.h>

/*
**  Table of contents
*/
int		     	DmiDecodeG4();

int			DmiCreateDecodeG4();
static int		_DecodeG4Initialize();
static int		_DecodeG4Activate();
static int		_DecodeG4Abort();
static int		_DecodeG4Destroy();

static int	     	_DecodeG4Segment();
static DecodeG4StatePtr	_InitializeG4State();
static void		_DestroyG4State();
static int	    	_DecodeG4ToBits();
static int	    	_DecodeG4ToCl();

#if defined(VMS)
extern int	    	_DecodeG4Process();
#else
static int		_DecodeG4Process();
#endif
static int              _PadG4Segment();
static int		_SaveSourceData();

/*
**  Equated Symbols
*/
#define Min_(a,b) ((a<b) ? (a) : (b))
#define Max_(a,b) ((a>b) ? (a) : (b))

/*
**  External References
*/

/*
**  Local storage
*/
    /*
    **  DecodeG4 Element Vector
    */
static PipeElementVector DecodeG4PipeElement = {
    NULL,				/* FLINK			    */
    NULL,				/* BLINK			    */
    sizeof(PipeElementVector),		/* Size				    */
    TypePipeElementVector,		/* Structure type		    */
    StypeDecodeG4Element,		/* Structure subtype		    */
    sizeof(DecodeG4PipeCtx),		/* Context block size		    */
    0,					/* No input flag		    */
    0,					/* Reserved flags		    */
    _DecodeG4Initialize,		/* Initialize entry		    */
    _DecodeG4Activate,			/* Activate entry		    */
    _DecodeG4Abort,			/* Flush entry			    */
    _DecodeG4Destroy,			/* Destroy entry		    */
    _DecodeG4Abort			/* Abort entry			    */
    };

/*****************************************************************************
**  DmiDecodeG4
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
int	DmiDecodeG4( src, dst )
UdpPtr	src;
UdpPtr	dst;
{
    DecodeG4StatePtr  state;
    int		 status;

    /*
    **	Initialize state record fields.
    */
    state = _InitializeG4State(dst);
    if (!IsPointer_(state) ) return( (int) state);

    state->srcpos = upPos_(src);
    /*
    **	Decode the entire buffer as a single segment.
    */
    status = _DecodeG4Segment( src, dst, state, TRUE );
    _DestroyG4State( state );

    return(status==G4K_StsDstFul || status==G4K_StsEoi ? Success : BadValue);
}					/* end DmiDecodeG4 */

/*****************************************************************************
**  DmiCreateDecodeG4
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
*****************************************************************************/
int	    DmiCreateDecodeG4( pipe, src, dst )
 Pipe		pipe;
 PipeSinkPtr	src;
 PipeSinkPtr	dst;
{
    DecodeG4PipeCtxPtr ctx = (DecodeG4PipeCtxPtr)
		    DdxCreatePipeCtx_( pipe, &DecodeG4PipeElement, TRUE );
    if( !IsPointer_(ctx) ) return( (int) ctx );
    /*
    **  Build our DecodeG4 Pipe Ctx from the arguments passed in.
    */
    DG4SrcSnk_(ctx) = src;
    DG4DstSnk_(ctx) = dst;
    /*
    **  Create a drain which reads component 0.
    */
    DG4SrcDrn_(ctx) = DdxRmCreateDrain_( src, 1 );
    if( !IsPointer_(DG4SrcDrn_(ctx)) ) return((int) DG4SrcDrn_(ctx) );
    /*
    **  Process any amount of encoded data, output bits or changelists.
    */
    DdxRmSetQuantum_( DG4SrcDrn_(ctx), 0 );
    DdxRmSetDType_( DG4SrcDrn_(ctx), UdpK_DTypeV, DtM_V );

    DdxRmSetQuantum_( dst, 0 );
    DdxRmSetDType_( dst, UdpK_DTypeUndefined, DtM_VU|DtM_CL );

    return( Success );
}				    /* end of DmiCreateDecodeG4 */

/*****************************************************************************
**  _DecodeG4Initialize
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int  _DecodeG4Initialize( ctx )
 DecodeG4PipeCtxPtr  ctx;
{
    int status;
    int bytes_per_scan, quantum;

    /*
    **	Initialize the source Drain.
    */
    status = DdxRmInitializePort_( CtxHead_(ctx), DG4SrcDrn_(ctx) );
    if( status != Success ) return( status );
    CtxInp_(ctx) = DG4SrcDrn_(ctx);
    VecNoInpt_(CtxVecPtr_(ctx)) = FALSE;
    /*
    **	Initialize the destination Sink.
    */
    status = DdxRmInitializePort_( CtxHead_(ctx), DG4DstSnk_(ctx) );
    if( status != Success ) return( status );

    /*
    ** If the next element in the pipeline has a zero input quantum,
    ** we should set a sink quantum to avoid decompressing the image in
    ** one lump.  This is a necessity for changelists, since they require
    ** such overallocation.
    ** Attempt to bound the quantum to 64KB chunks.
    */
    bytes_per_scan = (SnkDtyp_(DG4DstSnk_(ctx)) == UdpK_DTypeCL) ?
			(SnkWidth_(DG4DstSnk_(ctx),0) + 2) * sizeof(int) :
			((SnkScnStr_(DG4DstSnk_(ctx),0)+7)&~7) >> 3;
    quantum = Min_(SnkHeight_(DG4DstSnk_(ctx),0),
	      Max_(64 * 1024 / bytes_per_scan,1));
    DdxRmSetQuantum_( DG4DstSnk_(ctx), quantum );

    /*
    **	Create a destination descriptor.
    */
    DG4DstDat_(ctx) = DdxRmAllocData_( DG4DstSnk_(ctx), 0, 0,
				       DdxRmGetMaxQuantum_(DG4DstSnk_(ctx)) );
    if( !IsPointer_(DG4DstDat_(ctx)) )
	return((int)DG4DstDat_(ctx));
    DG4NewDst_(ctx) = TRUE;
    /*
    **	Create a decode state.
    */
    DG4SteBlk_(ctx) = _InitializeG4State(SnkUdpPtr_(DG4DstSnk_(ctx),0),
					 DatDatTyp_(DG4DstDat_(ctx)));
    if( !IsPointer_(DG4SteBlk_(ctx))) return((int) DG4SteBlk_(ctx) );

    return( status );
}				    /* end of _DecodeG4Initialize */


/*****************************************************************************
**  _DecodeG4Activate
**
**  FUNCTIONAL DESCRIPTION:
**
**   Decode some data.  If we already have a source segment in progress,
**   decode from it, otherwise get another source segment.  We'll decode until
**   we run out of source segments, create one quantum of
**   destination data, or hit a decoding error.
**
**   In the case of changelists, the destination data segments must be
**   large enough to hold a worst-case scanline (change at every pixel).
**   Since this usually results in massive overallocation, we apply the
**   "changelist packing" optimization. This can actually produce more than
**   a quantum's worth of scanlines.
**
**   If the compressed data stream ends before the expected last line of
**   the image, we will pad the image with zero filled scanlines (or
**   equivalent changelists).  Multiple quantums of scanlines may be
**   produced in this case.
**
**  FORMAL PARAMETERS:
**       ctx - DecodeG4 pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int  _DecodeG4Activate( ctx )
 DecodeG4PipeCtxPtr  ctx;
{
    PipeDataPtr	 src;
    int h, y, s;
    int oneCLbits;
    int scnrem;
    int curpos;
    int status;

    if( VecNoInpt_(CtxVecPtr_(ctx)) && DdxPipeHasYielded_(CtxHead_(ctx)) )
	{   /*
	    **	We already have source data but aren't allowed to process it.
	    */
	if( !DatOwned_(DG4SrcDat_(ctx)) )
	    _SaveSourceData(ctx);
	return( Success );
	}

    if( IsPointer_(DG4DstDat_(ctx)) &&
       DatDatTyp_(DG4DstDat_(ctx)) == UdpK_DTypeCL)
	{
	oneCLbits =
	    (SnkWidth_(DG4SrcSnk_(ctx),0) + 2 ) * sizeof(unsigned int) * 8;
	scnrem = DatHeight_(DG4DstDat_(ctx));
	curpos = DatPos_(DG4DstDat_(ctx));
	}


    do	{   /*
	    **	Decode data until source exhausted, destination full, or error.
	    */
	if( !VecNoInpt_(CtxVecPtr_(ctx)) )
	    {	/*
		**  Get a chunk of encoded data.
		*/
	    DG4SrcDat_(ctx) = DdxRmGetData_( ctx, DG4SrcDrn_(ctx) );
	    if( !IsPointer_(DG4SrcDat_(ctx)) ) return( (int) DG4SrcDat_(ctx) );
	    DG4SteBlk_(ctx)->srcpos = DatPos_(DG4SrcDat_(ctx));
	    VecNoInpt_(CtxVecPtr_(ctx)) = TRUE;
	    }

	/*
	** No destination data segment means we have already returned the
	** the last scanline of the image, or there is a bug somewhere.
	** Tolerate the client being confused or having a slightly bogus
	** G4 stream by discarding any data and returning success.
	*/
	if( !IsPointer_(DG4DstDat_(ctx)) )
	    {
	    DG4SrcDat_(ctx) = DdxRmDeallocData_(DG4SrcDat_(ctx));
	    VecNoInpt_(CtxVecPtr_(ctx)) = FALSE;
	    break;
	    }

	/*
	**  Call the decoder.
	*/
	if ( !DG4SteBlk_(ctx)->pad_flag )
   	    s = _DecodeG4Segment( DatUdpPtr_(DG4SrcDat_(ctx)),
			       DatUdpPtr_(DG4DstDat_(ctx)),
			       DG4SteBlk_(ctx), DG4NewDst_(ctx) );
	else
	    s = _PadG4Segment( DatUdpPtr_(DG4DstDat_(ctx)),
			       DatY1_(DG4DstDat_(ctx)) );

	DG4NewDst_(ctx) = FALSE;

	switch( s )
	    {
	case G4K_StsDstFul :

	    if (DatDatTyp_(DG4DstDat_(ctx)) == UdpK_DTypeCL )
		{
		/*
		** For changelists, the data buffer will usually be massively
		** overallocated.  Therefore, as long as there is space for
		** at least one worst-case changelist left in the buffer
		** we'll keep advancing the Y2 value of the data segment and
		** decoding more data.
		*/
		curpos = DdxPosCl_(DatBase_(DG4DstDat_(ctx)), curpos, scnrem);
		scnrem = (DatArSize_(DG4DstDat_(ctx)) - curpos) / oneCLbits;

		if (DatY2_(DG4DstDat_(ctx)) + scnrem
		        > SnkY2_(DG4SrcSnk_(ctx),0))
		    /* Gotta stop at the end of the image */
		    scnrem = SnkY2_(DG4SrcSnk_(ctx),0) - DatY2_(DG4DstDat_(ctx));
				   
		if (scnrem > 0 )
		    {
		    DatY2_(DG4DstDat_(ctx)) += scnrem;
		    DatHeight_(DG4DstDat_(ctx)) += scnrem;
		    s = G4K_StsSrcExh;	/* dest isn't full anymore! */
		    continue;   /* the do-while loop */
		    }
		}
					/* Drop through */
	case G4K_StsEoi    :
			       
	    /*
	    **	Pass along all the PCM data we've decompressed
	    */
	    status = Success;
	    y = DatY2_(DG4DstDat_(ctx));
	    status = DdxRmPutData_( DG4DstSnk_(ctx), DG4DstDat_(ctx) );

	    if( status == Success && y++ < SnkY2_(DG4DstSnk_(ctx),0) )
		{	/*
		    **  Allocate space for another quantum of decoded data.
		    */
		h = DdxRmGetMaxQuantum_( DG4DstSnk_(ctx) );
		if( h > SnkHeight_(DG4DstSnk_(ctx),0) - y )
		    h = SnkHeight_(DG4DstSnk_(ctx),0) - y;
		DG4DstDat_(ctx) = DdxRmAllocData_( DG4DstSnk_(ctx), 0, y, h );
		if( !IsPointer_(DG4DstDat_(ctx)) )
		    status = (int) DG4DstDat_(ctx);
		DG4NewDst_(ctx) = TRUE;

		if( s == G4K_StsEoi )
		    /*
		    ** Eoi before last scanline.  Generate padding on future
		    ** activations.
		    */
		    DG4SteBlk_(ctx)->pad_flag = TRUE;
		}
	    else
		DG4DstDat_(ctx) = NULL;

	    VecNoInpt_(CtxVecPtr_(ctx)) = IsPointer_(DG4DstDat_(ctx));
	    return( status );

	case G4K_StsSrcExh :
	    /*
	    **	We're done with that chunk -- prepare for more.
	    */
	    DG4SrcDat_(ctx) = DdxRmDeallocData_(DG4SrcDat_(ctx));
	    VecNoInpt_(CtxVecPtr_(ctx)) = FALSE;
	    break;

	default : return( BadValue );
	    }
	}
    while( s == G4K_StsSrcExh );

    return( Success );
}				    /* end of _DecodeG4Activate */

/*****************************************************************************
**  _DecodeG4Abort
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int  _DecodeG4Abort( ctx )
 DecodeG4PipeCtxPtr  ctx;
{
    /*
    **	Destroy decode state.
    */
    _DestroyG4State( DG4SteBlk_(ctx) );
    DG4SteBlk_(ctx) = NULL;

    /*
    **	Turn off the no-input flag.
    */
    VecNoInpt_(CtxVecPtr_(ctx)) = FALSE;
    /*
    **	Destroy destination data descriptor.
    */
    DG4SrcDat_(ctx) = DdxRmDeallocData_( DG4SrcDat_(ctx) );
    DG4DstDat_(ctx) = DdxRmDeallocData_( DG4DstDat_(ctx) );

    return( Success );
}				    /* end of _DecodeG4Abort */

/*****************************************************************************
**  _DecodeG4Destroy
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int  _DecodeG4Destroy( ctx )
 DecodeG4PipeCtxPtr  ctx;
{
    DG4SrcDrn_(ctx) = DdxRmDestroyDrain_( DG4SrcDrn_(ctx) );

    return( Success );
}				    /* end of _DecodeG4Destroy */

/*****************************************************************************
**  _DecodeG4Segment
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**     dst - pointer to the UDP to receive the decompressed segment.
**           If dst is not of type changelist its associated data
**           buffer *MUST* have been zeroed, since only the non-zero
**           pixels are written.
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int   _DecodeG4Segment( src, dst, state, newdst )
UdpPtr	    		src;
UdpPtr	    		dst;
DecodeG4StatePtr  	state;
int         		newdst;
{
    UdpRec  		tmp;
    unsigned char 	rem[4];
    int 		value;
    int			status;

    /*
    **	Allocate destination buffer if required.
    */
    if( upBase_(dst) == NULL )
	{
        upBase_(dst) = DdxMallocBits_( upArSize_(dst) );
	if( upBase_(dst) == NULL ) return( BadAlloc );
	}

    /*
    **  If there are source bits remaining, append them in a buffer with the
    **  new source bits.
    */
    if (state->remlen != 0)
	{
	urArSize_(tmp) = state->remlen + G4K_MinSrcLen;
	urBase_(tmp)   = rem;
	urPos_(tmp)    = 0;
	state->srcpos  = 0;
	PUT_VALUE_( urBase_(tmp), 0, state->remdat, (1<<state->remlen)-1 );
	value  = GET_VALUE_( upBase_(src), upPos_(src), (1<<G4K_MinSrcLen)-1 );
	PUT_VALUE_( urBase_(tmp), state->remlen, value, (1<<G4K_MinSrcLen)-1 );

	status = dst->UdpB_Class == UdpK_ClassCL
			? _DecodeG4ToCl(&tmp, dst, state, newdst)
			: _DecodeG4ToBits(&tmp, dst, state);

	switch (status)
	    {
	    case G4K_StsSrcExh:
		state->srcpos = upPos_(src) + G4K_MinSrcLen - state->remlen;
		state->remlen = 0;
		break;

	    case G4K_StsDstFul:
		/*
		**  Destination full, but src not exhausted.
		**  Restore state so things can be resumed.
		*/
		state->srcpos -= state->remlen;
		if( state->srcpos < 0 )
		    {
		    state->remdat >>= state->remlen + state->srcpos;
		    state->remlen = abs(state->srcpos);
		    }
		else
		    state->remlen = 0;
		return( G4K_StsDstFul );

	    default:
		return status;
	    }
        }
    return (dst->UdpB_Class == UdpK_ClassCL
		? _DecodeG4ToCl(src, dst, state, newdst)
		: _DecodeG4ToBits(src, dst, state));

}					/* end _DecodeG4Segment */

/*****************************************************************************
**  _InitializeG4State
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine creates a new G4 decode state record. This record is
**	required to keep track of decode state variables between decoded data
**	segments.
**
**  FORMAL PARAMETERS:
**
**	dst - Destination UDP pointer. Describes parameters of the image to
**	      be decompressed.
**
**	dtype - Data type of target buffer.
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
**      On success, pointer to created state block
**	On failure, status, one of the following :
**		    BadAlloc : Insufficient resources to allocate dynamic memory
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
static DecodeG4StatePtr  _InitializeG4State(dst,dtype)
UdpPtr	    dst;
int	    dtype;
{
    DecodeG4StatePtr state = (DecodeG4StatePtr)
				DdxCalloc_( 1, sizeof(DecodeG4StateRec) );
    if( state == NULL ) return( (DecodeG4StatePtr) BadAlloc );

    state->ppl	  = upWidth_(dst);
    state->nxtscn = upY1_(dst);
    state->lstscn = dst->UdpL_Y2;
    state->pad_flag = FALSE;

    if (dtype == UdpK_DTypeCL)
    {
	/*
	**  Allocate a reference change list to hold the reference line
	**  between output segments. Initialize it to the imaginary white
	**  reference line. 
	*/
	state->rcl_desc = (int *) DdxMalloc_((state->ppl + 4) * sizeof(int));
	state->rcl = state->rcl_desc;
        if (state->rcl_desc == NULL)
        {
            DdxFree_(state);
	    return (DecodeG4StatePtr)BadAlloc;
        }
    }
    else
    {
	/*
	**  Allocate space for change lists...
	*/
	state->ccl = (int *) DdxMalloc_((state->ppl + 4) * sizeof(int));
        if (state->ccl == NULL)
        {
            DdxFree_(state);
	    return (DecodeG4StatePtr)BadAlloc;
        }

	state->rcl = (int *) DdxMalloc_((state->ppl + 4) * sizeof(int));
        if (state->rcl == NULL)
        {
            DdxFree_(state);
            DdxFree_(state->ccl);
	    return (DecodeG4StatePtr)BadAlloc;
        }
    }

    state->rcl[0] = 1;
    state->rcl[1] = state->ppl;
    state->rcl[2] = state->ppl;
    state->rcl[3] = state->ppl;
    state->rcl[4] = state->ppl;

    state->mode	  = G4K_ModeNewScan;

    return( state );
}					/* end _InitializeG4State */

/*****************************************************************************
**  _DestroyG4State
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**
*****************************************************************************/
static void  _DestroyG4State(state)
DecodeG4StatePtr   state;
{
    if (IsPointer_(state) )
    {
	if (IsPointer_(state->rcl_desc))
	{
	    DdxFree_(state->rcl_desc);
	}
	else
	{
	    if (IsPointer_(state->ccl))
		DdxFree_(state->ccl);
	    if (IsPointer_(state->rcl))
		DdxFree_(state->rcl);
	}
	DdxFree_(state);
    }
}

/*****************************************************************************
**  _DecodeG4ToBits
**
**  FUNCTIONAL DESCRIPTION:
**
**	This function decodes a G4 data segment into image data.
**
**  FORMAL PARAMETERS:
**
**	src	Address of UDP describing compressed data buffer
**	dst	Address of UDP describing destination image data buffer
**	state	Address of G4 state block
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
**      status, one of the following:
**	        G4K_StsDstFul, Destination buffer full
**	        G4K_StsSrcExh, Source buffer exhausted
**	        G4K_StsDcdErr, Decode error (probably bad compressed data)
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
static	int _DecodeG4ToBits(src,dst,state)
UdpPtr	     src;
UdpPtr	     dst;
DecodeG4StatePtr   state;
{
    int	 status = G4K_StsEol;
    int	*tmp;
    long scanofs;
    /*
    **	If we're already past this scanline, something's wrong...
    */
    if (state->nxtscn < dst->UdpL_Y1)
    {
	return  G4K_StsDstFul;
    }
    /*
    **	While within this output segment...
    */
    while (state->nxtscn <= dst->UdpL_Y2)
    {
	status = _DecodeG4Process(src, state);
	if (status == G4K_StsEol)
	{
	    state->ccl[state->ccl[0]+1] = state->ppl;
	    state->ccl[state->ccl[0]+2] = state->ppl;
	    state->ccl[state->ccl[0]+3] = state->ppl;

	    scanofs = dst->UdpL_Pos + dst->UdpL_ScnStride
			* (state->nxtscn++ - dst->UdpL_Y1);
	    memset(dst->UdpA_Base + ((scanofs + 7) >> 3),
		   0, 
		   ((dst->UdpL_ScnStride +7) >> 3));
	    DdxPutRuns_(dst->UdpA_Base, scanofs, state->ccl);
	    /*
	    **  Swap current and reference scanlines.
	    */
	    tmp = state->rcl;
	    state->rcl = state->ccl;
	    state->ccl = tmp;
	}
	else	    
	    break;
    }

    switch( status )
	{
        case G4K_StsEol:
	    return G4K_StsDstFul;
	
        case G4K_StsEoi:
	    _PadG4Segment( dst, state->nxtscn );
	    return G4K_StsEoi;

        default:
	    return status;
	}
}

/*****************************************************************************
**  _DecodeG4ToCl
**
**  FUNCTIONAL DESCRIPTION:
**
**	This function decodes a G4 data segment into a change list.
**
**  FORMAL PARAMETERS:
**
**	src	Address of UDP describing compressed data buffer
**	dst	Address of UDP describing destination image data buffer
**	state	Address of G4 state block
**	newdst	Boolean, indicates caller has provided a new destination buffer
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
**      status, one of the following:
**
**	        G4K_StsDstFul, Destination buffer full
**	        G4K_StsSrcExh, Source buffer exhausted
**		G4K_StsEoi,    End of encoded image encountered
**	        G4K_StsDcdErr, Decode error (probably bad compressed data)
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
static	int _DecodeG4ToCl(src,dst,state,newdst)
UdpPtr	     src;
UdpPtr	     dst;
DecodeG4StatePtr   state;
int	     newdst;
{
    int	    status = G4K_StsEol;
    int     *clsrc,*cldst;
    int	    i;
    /*
    **	If we're already past this scanline, something's wrong...
    */
    if (state->nxtscn < dst->UdpL_Y1)
    {
	return  G4K_StsDstFul;
    }
    /*
    **	If new destination buffer, get pointer to starting point in output
    **	changelist.
    */
    if (newdst)
    {
	state->ccl = 
	    (int *)(dst->UdpA_Base + 
		(DdxPosCl_(dst->UdpA_Base,
			   dst->UdpL_Pos,
			   state->nxtscn-dst->UdpL_Y1) >> 3));
	state->rcl = state->rcl_desc;
    }
    /*
    **	While within this output segment...
    */
    while (state->nxtscn <= dst->UdpL_Y2)
    {
	status = _DecodeG4Process(src, state);

	if (status == G4K_StsEol)
	{
	    state->ccl[state->ccl[0]+1] = state->ppl;
	    state->ccl[state->ccl[0]+2] = state->ppl;
	    state->ccl[state->ccl[0]+3] = state->ppl;
	    state->ccl[0] += 3; 
	    /*
	    **  Update change list pointers.
	    */
	    state->rcl = state->ccl;
	    state->ccl = state->ccl + state->ccl[0] + 2;
	    /*
	    **	Proceed to next scanline...
	    */
	    state->nxtscn++;
	}
	else	    
	    break;
    }
    /*
    ** Before returning full destination buffer, copy the current
    ** change list so it can be used as the reference change list with
    ** compunction during the next segment.
    **
    ** NOTE :
    **	    We wouldn't have to do this copy if we had a handle on the
    **	    pipe data descriptor down at this level. That would allow
    **	    us to make a reference descriptor on the data to insure that
    **	    it wouldn't be freed until we were done with it.
    */
    clsrc = state->rcl;
    cldst = state->rcl_desc;
    for (i = 0;  i <= *state->rcl;  i++)
    {
	*cldst++ = *clsrc++;
    }    

    switch( status )
	{
        case G4K_StsEol:
	    return G4K_StsDstFul;
	
        case G4K_StsEoi:
	    _PadG4Segment( dst, state->nxtscn );
	    return G4K_StsEoi;

        default:
	    return status;
	}

}

/*****************************************************************************
**  _DecodeG4Process
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
#if !defined(VMS)
static int   _DecodeG4Process(src, state)
UdpPtr	     src;
DecodeG4StatePtr   state;
{
    int	    bits;
    int	    value;
    int	    type;
    
    if( state->mode == G4K_ModeNewScan )
	{
	state->cidx = 0;
	state->ridx = 1;
	state->a0 = -1;
	state->a0prime = -1;
	state->a1 = 0;
	state->b1 = 0;
	state->mode = G4K_ModeCodeword;
	state->ccl[0] = 0;
	}
    while( TRUE )
	{
	if( src->UdpL_ArSize - state->srcpos > G4K_MinSrcLen )
	    {
	    switch( state->mode )
		{

	    /*
	    **	State mode is codeword. Read the next codeword and dispatch
	    **	based on the
	    */		
	    case G4K_ModeCodeword:
		bits = GET_VALUE_(src->UdpA_Base,state->srcpos,G4M_Codeword);
		state->srcpos += MiArFax2DDecodeTable[bits].length;
		switch (MiArFax2DDecodeTable[bits].type)
		    {
		/*
		**  State mode is codeword, codeword type is TERMINATOR,
		**  dispatch from here based on codeword value.
		*/
		case G4K_CodetypeTerminator:
		    switch (MiArFax2DDecodeTable[bits].value)
			{

		    /*
		    **	State mode is codeword, codeword type is TERMINATOR,
		    **	codeword value is PASS.
		    */
		    case G4K_CodevaluePass:
			while (state->rcl[state->ridx] <= state->a0prime ||
			       ((state->ridx & 1) == (state->cidx & 1)))
			    state->ridx++;

			state->b1 = state->rcl[state->ridx];
			state->a0prime = state->rcl[state->ridx + 1];
			if ((state->ridx > 1) &&
			    (state->rcl[state->ridx - 1] > state->a0prime))
			    state->ridx--;

			if (state->ccl[state->cidx] >= state->ppl)
			    {
			    state->mode = G4K_ModeNewScan;
			    return  G4K_StsEol;
			    }
			break;

		    /*
		    **	State mode is Codeword, codeword type is TERMINATOR,
		    **	codeword value is HORIZONTAL.
		    */
		    case G4K_CodevalueHoriz:
			if (state->a0 < 0)
			    {
			    state->a0 = 0;
			    if (state->a0prime < 0)
				state->a0prime = 0;
			    }
			state->cidx++;
			state->rlen = 0;
			state->mode = G4K_ModeFirstRun;
			break;

		    /*
		    **	State mode is codeword, codeword type is TERMINATOR,
		    **	codeword value is VERTICAL.
		    */
		    case G4K_CodevalueV0:
		    case G4K_CodevalueVR1:
		    case G4K_CodevalueVL1:
		    case G4K_CodevalueVR2:
		    case G4K_CodevalueVL2:
		    case G4K_CodevalueVR3:
		    case G4K_CodevalueVL3:
			while (state->rcl[state->ridx] <= state->a0prime ||
			       ((state->ridx & 1) == (state->cidx & 1)))
			    state->ridx++;

			state->b1 = state->rcl[state->ridx];

			if ((state->ridx > 1) &&
			    (state->rcl[state->ridx - 1] > state->a0prime))
			    state->ridx--;

			state->a0 = state->b1+MiArFax2DDecodeTable[bits].value;
			state->a0prime = state->a0;
			state->ccl[++(state->cidx)] = state->a0;
			state->ccl[0]++;

			if (state->ccl[state->cidx] >= state->ppl)
			    {
			    state->mode = G4K_ModeNewScan;
			    return  G4K_StsEol;
			    }
			break;

		    /*
		    **	Here on invalid codeword values.
		    */
		    default:
			return	G4K_StsDcdErr;
		    }
		break;	/* End of switch (codeword type) */

		/*
		**  State mode is Codeword, codeword type is EOL.
		*/
		case G4K_CodetypeEol:
		    bits = GET_VALUE_(src->UdpA_Base,
				      state->srcpos,
				      G4M_Codeword);

                    if (MiArFax2DDecodeTable[bits].type == G4K_CodetypeEol)
                    {
			state->srcpos += MiArFax2DDecodeTable[bits].length;
                        return G4K_StsEoi;
                    }
                    else
                    {
			state->mode = G4K_ModeNewScan;
			return  G4K_StsEol;
                    }

		/*
		**  Here on invalid codeword types.
		*/
		default:
		    return  G4K_StsDcdErr;
		}
	    break;  /* End of switch (codeword value)	*/

	case G4K_ModeFirstRun:
	    if (state->cidx & 1 == G4K_WhiteRun)
		{
		bits = GET_VALUE_(src->UdpA_Base,state->srcpos,G4M_WhiteRun);
		state->srcpos += MiArFax1DDecodeWhite[bits].length;
		value = MiArFax1DDecodeWhite[bits].value;
		type = MiArFax1DDecodeWhite[bits].type;
		}
	    else
		{
		bits = GET_VALUE_(src->UdpA_Base,state->srcpos,G4M_BlackRun);
		state->srcpos += MiArFax1DDecodeBlack[bits].length;
		value = MiArFax1DDecodeBlack[bits].value;
		type = MiArFax1DDecodeBlack[bits].type;
		}
	    state->rlen += value;

	    if (type != G4K_CodetypeMakeup)
		{
		state->a1 = state->rlen + state->a0prime;
		state->ccl[state->cidx++] = state->a1;
		state->ccl[0]++;
		state->rlen = 0;
		state->mode = G4K_ModeSecondRun;
		}
	    break;

	case G4K_ModeSecondRun:
	    if (state->cidx & 1 == G4K_WhiteRun)
		{
		bits = GET_VALUE_(src->UdpA_Base,state->srcpos,G4M_WhiteRun);
		state->srcpos += MiArFax1DDecodeWhite[bits].length;
		value = MiArFax1DDecodeWhite[bits].value;
		type = MiArFax1DDecodeWhite[bits].type;
		}
	    else
		{
		bits = GET_VALUE_(src->UdpA_Base,state->srcpos,G4M_BlackRun);
		state->srcpos += MiArFax1DDecodeBlack[bits].length;
		value = MiArFax1DDecodeBlack[bits].value;
		type = MiArFax1DDecodeBlack[bits].type;
		}
	    state->rlen += value;

	    if (type != G4K_CodetypeMakeup)
		{
		state->a0 = state->a1 + state->rlen;
		state->a0prime = state->a0;
		state->ccl[state->cidx] = state->a0;
		state->ccl[0]++;
		state->mode = G4K_ModeCodeword;

		if (state->ccl[state->cidx] >= state->ppl)
		    {
		    state->mode = G4K_ModeNewScan;
		    return  G4K_StsEol;
		    }
		}
	    break;

	/*
	**  Here on invalid state mode.
	*/
	default:
	    return G4K_StsDcdErr;
	    }   /* End of switch (state->mode) */

	    }   /* End of if (enough source data remaining) */
	else
	    /*
	    **  If not enough source data, return source exhausted.
	    */
	    {
	    state->remlen = src->UdpL_ArSize - state->srcpos;
	    state->remdat = GET_VALUE_(src->UdpA_Base,
					      state->srcpos, 
					      (1 << state->remlen) - 1);
	    return  G4K_StsSrcExh;
	    }
	}
}
#endif

/*****************************************************************************
**  _PadG4Segment
**
**  FUNCTIONAL DESCRIPTION:
**
**   Fill the specified udp with the equivalent of zero image data, starting
**   at the specified scanline.
**
**  FORMAL PARAMETERS:
**    dst - Pointer to udp to be filled with padding
**    start - Starting scanline
**
**  FUNCTION VALUE:
**    G4K_StsEoi
**
*****************************************************************************/
static int  _PadG4Segment( dst, start )
    UdpPtr dst;
    int start;
{
    int i;
    int startpos;
    unsigned int *cl;

    if( dst->UdpB_DType != UdpK_DTypeCL )
	DdxFillRegion_( dst, dst->UdpL_X1, start, 
		        dst->UdpL_PxlPerScn, dst->UdpL_Y2- start + 1, 0);
    else
	{
	startpos = DdxPosCl_(dst->UdpA_Base, dst->UdpL_Pos,
			   start - dst->UdpL_Y1);
	cl = (unsigned int *)(dst->UdpA_Base + ( startpos / 8 ));

	for( i = start; i <= dst->UdpL_Y2; i++)
	    {
	    cl[0] = 1;
	    cl[1] = dst->UdpL_PxlPerScn;
	    cl[2] = 0;
	    cl += 3;
	    }
	}
    return( G4K_StsEoi );
}					/* end _PadG4Segment */

/*****************************************************************************
**  _SaveSourceData
**
**  FUNCTIONAL DESCRIPTION:
**
**   Save the unprocessed portion of the data currently in the
**   decompression state record.  
**
**
**  FORMAL PARAMETERS:
**    ctx - pipe context pointer
**
**
**  FUNCTION VALUE:
**    Succes or BadAlloc
**
*****************************************************************************/
static int  _SaveSourceData( ctx )
    DecodeG4PipeCtxPtr	ctx;
{
    int pos;
    unsigned char *tmp;
    PipeDataPtr src;

    src = DG4SrcDat_(ctx);
    tmp = DatBase_(src);
    pos = DG4SteBlk_(ctx)->srcpos & ~0x7;
    DG4SteBlk_(ctx)->srcpos       &= 0x7;
    DatArSize_(src)		  -= pos;
    DatOwned_(src)		   = TRUE;
    DatPos_(src)		   = 0;
    DatBase_(src)		   = DdxMallocBits_( DatArSize_(src) );
    if( DatBase_(src) == NULL ) return( BadAlloc );
    memcpy( DatBase_(src), tmp+(pos>>3), DatArSize_(src)>>3 );
    }


static int	_DecodePrintState(state)
DecodeG4StatePtr	state;
{
    int i;

    printf("Scanline = %d\n",state->nxtscn);

    printf("Current change list @ %X: \n",state->ccl);
    for (i = 1; i <= state->ccl[0]; i++)
	printf("%d\n",state->ccl[i]);

    printf("Reference change list @ %X: \n", state->rcl);
    for (i = 1; i <= state->rcl[0]; i++)
	printf("%d\n",state->rcl[i]);
}
