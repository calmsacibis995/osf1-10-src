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
Copyright 1989, 1990 by Digital Equipment Corporation, Maynard, Massachusetts,
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
**      This module implements decompression of G4 Facsimile encoded data.
**
**  ENVIRONMENT:
**
**	VAX/VMS V5.3
**	ULTRIX  V3.1
**
**  AUTHORS:
**
**	Derived from code originally developed by Ken MacDonald
**	John Weber
**	Robert NC Shelley
**
**  CREATION DATE:     
**
**	February 27, 1990
**
*******************************************************************************/

#include <stdio.h>

#include "SmiDecodeG4.h"

/*
**  Table of contents
*/
int			 SmiDecodeG4();

int			 SmiCreateDecodeG4();
static int		_DecodeG4Initialize();
static int		_DecodeG4Activate();
static int		_DecodeG4Abort();
static int		_DecodeG4Destroy();

static int		_DecodeG4Segment();
static DecodeG4StatePtr _InitializeG4State();
static DecodeG4StatePtr _DestroyG4State();
static int		_DecodeG4Src();
static int		_DecodeG4Process();

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
**  SmiDecodeG4
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
int	SmiDecodeG4( src, dst )
 UdpPtr	src;
 UdpPtr	dst;
{
    DecodeG4StatePtr state;
    int              status;
    /*
    **	Initialize state record fields.
    */
    state = _InitializeG4State( dst );
    if( !IsPointer_(state) ) return( (int) state );

    state->srcpos = upPos_(src);
    /*
    **	Decode the entire buffer as a single segment.
    */
    status = _DecodeG4Segment( src, dst, state );
    _DestroyG4State( state );

    return(status == G4K_StsDstFul || status==G4K_StsEoi ? Success : BadValue);
}

/*****************************************************************************
**  SmiCreateDecodeG4
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
*****************************************************************************/
int	    SmiCreateDecodeG4( pipe, src, dst )
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
    **  Process any amount of encoded data, output bits.
    */
    DdxRmSetQuantum_( DG4SrcDrn_(ctx), 0 );
    DdxRmSetDType_( DG4SrcDrn_(ctx), UdpK_DTypeV, DtM_V );
    DdxRmSetQuantum_( dst, 0 );
    DdxRmSetDType_( dst, UdpK_DTypeVU, DtM_VU );

    return( Success );
}				    /* end of SmiCreateDecodeG4 */

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
    **	Create a destination descriptor.
    */
    DG4DstDat_(ctx) = DdxRmAllocData_( DG4DstSnk_(ctx), 0, 0,
				       DdxRmGetMaxQuantum_(DG4DstSnk_(ctx)) );
    if( !IsPointer_(DG4DstDat_(ctx)) )
	return((int)DG4DstDat_(ctx));
    /*
    **	Create a decode state.
    */
    DG4SteBlk_(ctx) = _InitializeG4State(SnkUdpPtr_(DG4DstSnk_(ctx),0));
    if( !IsPointer_(DG4SteBlk_(ctx))) return((int) DG4SteBlk_(ctx) );

    return( status );
}				    /* end of _DecodeG4Initialize */

/*****************************************************************************
**  _DecodeG4Activate
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int  _DecodeG4Activate( ctx )
 DecodeG4PipeCtxPtr  ctx;
{
    PipeDataPtr    src;
    unsigned char *tmp;
    int   pos, h, y, s;

    if( VecNoInpt_(CtxVecPtr_(ctx)) && DdxPipeHasYielded_(CtxHead_(ctx)) )
	{   /*
	    **	We already have source data but aren't allowed to process it.
	    */
	if( !DatOwned_(DG4SrcDat_(ctx)) )
	    {	/*
		**  Move the unprocessed data to a buffer of our own.
		*/
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
	return( Success );
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
	**  Call the decoder (if we've already finished just discard the data).
	*/
	s = !IsPointer_(DG4DstDat_(ctx)) ? G4K_StsSrcExh
	  :  _DecodeG4Segment( DatUdpPtr_(DG4SrcDat_(ctx)),
			       DatUdpPtr_(DG4DstDat_(ctx)),
			       DG4SteBlk_(ctx) );
	switch( s )
	    {
	case G4K_StsDstFul :
	    /*
	    **	Pass the PCM data along.
	    */
	    y = DatY2_(DG4DstDat_(ctx));
	    s = DdxRmPutData_( DG4DstSnk_(ctx), DG4DstDat_(ctx) );

	    if( s == Success && y++ < SnkY2_(DG4DstSnk_(ctx),0) )
		{   /*
		    **  Allocate space for another quantum of decoded data.
		    */
		h = DdxRmGetMaxQuantum_( DG4DstSnk_(ctx) );
		if( h > SnkHeight_(DG4DstSnk_(ctx),0) - y )
		    h = SnkHeight_(DG4DstSnk_(ctx),0) - y;
		DG4DstDat_(ctx) = DdxRmAllocData_( DG4DstSnk_(ctx), 0, y, h );
		if( !IsPointer_(DG4DstDat_(ctx)) )
		    s = (int) DG4DstDat_(ctx);
		}
	    else
		DG4DstDat_(ctx) = NULL;

	    VecNoInpt_(CtxVecPtr_(ctx)) = IsPointer_(DG4DstDat_(ctx));
	    return( s );

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
    DG4SteBlk_(ctx) = _DestroyG4State( DG4SteBlk_(ctx) );
    /*
    **	Turn off the no-input flag.
    */
    VecNoInpt_(CtxVecPtr_(ctx)) = FALSE;
    /*
    **	Deallocate source and destination data descriptors.
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
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int _DecodeG4Segment( src, dst, state )
 UdpPtr		    src;
 UdpPtr		    dst;
 DecodeG4StatePtr   state;
{
    UdpRec	  tmp;
    unsigned char rem[4];
    int		  value;

    /*
    **	Allocate destination buffer if required.
    */
    if( upBase_(dst) == NULL )
	{
        upBase_(dst) = DdxMallocBits_( upArSize_(dst) );
	if( upBase_(dst) == NULL ) return( BadAlloc );
	}
    /*
    **  If there are source bits remaining, concatenate them in a buffer with
    **  some new source bits.
    */
    if( state->remlen != 0 )
	{
	urArSize_(tmp) = state->remlen + G4K_MinSrcLen;
	urBase_(tmp)   = rem;
	urPos_(tmp)    = 0;
	state->srcpos  = 0;
	PUT_VALUE_( urBase_(tmp), 0, state->remdat, (1<<state->remlen)-1 );
	value  = GET_VALUE_( upBase_(src), upPos_(src), (1<<G4K_MinSrcLen)-1 );
	PUT_VALUE_( urBase_(tmp), state->remlen, value, (1<<G4K_MinSrcLen)-1 );

	switch( _DecodeG4Src( &tmp, dst, state ) )
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
		    state->remlen   = abs( state->srcpos );
	            }
	        else
		    state->remlen = 0;
	        return( G4K_StsDstFul );

	    case G4K_StsDcdErr:
	        return( G4K_StsDcdErr );
	    }
        }
    return( _DecodeG4Src( src, dst, state ) );
}

/*****************************************************************************
**  _InitializeG4State
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
**      none
**
*****************************************************************************/
static DecodeG4StatePtr  _InitializeG4State(dst)
 UdpPtr	    dst;
{
    DecodeG4StatePtr state = (DecodeG4StatePtr)
			      DdxCalloc_(1,sizeof(DecodeG4StateRec));
    if( state == NULL ) return( (DecodeG4StatePtr) BadAlloc );

    state->ppl	  = upWidth_(dst);
    state->nxtscn = upY1_(dst);
    state->lstscn = dst->UdpL_Y2;
    state->ccl	  = (int *) DdxMalloc_((state->ppl + 4) * sizeof(int));
    state->rcl	  = (int *) DdxMalloc_((state->ppl + 4) * sizeof(int));

    state->rcl[0] = 1;
    state->rcl[1] = state->ppl;
    state->rcl[2] = state->ppl;
    state->rcl[3] = state->ppl;
    state->rcl[4] = state->ppl;

    state->mode	  = G4K_ModeNewScan;

    return( state );
}

/*****************************************************************************
**  _DestroyG4State
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
**      none
**
*****************************************************************************/
static DecodeG4StatePtr _DestroyG4State(state)
 DecodeG4StatePtr   state;
{
    if( IsPointer_(state) )
	{
        if( IsPointer_(state->ccl) )
	      DdxFree_(state->ccl);
        if( IsPointer_(state->rcl) )
	      DdxFree_(state->rcl);
	DdxFree_(state);
	}
    return( NULL );
}

/*****************************************************************************
**  _DecodeG4Src
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
static	int _DecodeG4Src( src, dst, state )
 UdpPtr	    	     src;
 UdpPtr		     dst;
 DecodeG4StatePtr    state;
{
    int	 status;
    int	*tmp;
    long scanofs;

    if( state->nxtscn < upY1_(dst) )
	status = G4K_StsDstFul;
    else
	while( TRUE )
	    {
	    if( state->nxtscn > upY2_(dst) )
		{
		status = G4K_StsDstFul;
		break;
		}
	    status = _DecodeG4Process( src, state );
	    if( status == G4K_StsEol )
		{
		state->ccl[state->ccl[0]+1] = state->ppl;
		state->ccl[state->ccl[0]+2] = state->ppl;
		state->ccl[state->ccl[0]+3] = state->ppl;

		scanofs =  upPos_(dst) + upScnStr_(dst)
			* (state->nxtscn++ - upY1_(dst));
		memset( upBase_(dst)+(scanofs+7>>3), 0, upScnStr_(dst)+7>>3 );
		MiPutRuns( upBase_(dst), scanofs, state->ccl );
		/*
		**  Swap current and reference scanlines.
		*/
		tmp	   = state->rcl;
		state->rcl = state->ccl;
		state->ccl = tmp;
		}
	    else	    
		break;
	    }

    switch (status)
    {
        case G4K_StsEoi:
            /*
	    ** EOI before the expected last scanline.  For now,
	    ** consider this an error.
	    */
            if (state->nxtscn <= state->lstscn)
		return G4K_StsDcdErr;
   
        case G4K_StsEol:
            if (state->nxtscn > state->lstscn)
                return	G4K_StsEoi;
            else
		return  G4K_StsDstFul;
        default:
            return  status;
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
static int   _DecodeG4Process(src, state)
 UdpPtr		    src;
 DecodeG4StatePtr   state;
{
    int bits, value, type;
    
    if( state->mode == G4K_ModeNewScan )
	{
	state->cidx	= 0;
	state->ridx	= 1;
	state->a0	= -1;
	state->a0prime	= -1;
	state->a1	= 0;
	state->b1	= 0;
	state->mode	= G4K_ModeCodeword;
	state->ccl[0]	= 0;
	}
    while( TRUE )
	{
	if( upArSize_(src) - state->srcpos > G4K_MinSrcLen )
	    {
	    switch( state->mode )
		{   /*
		    **	Read the next codeword and dispatch.
		    */		
	    case G4K_ModeCodeword :
		bits = GET_VALUE_( upBase_(src), state->srcpos, G4M_Codeword );
		state->srcpos += MiArFax2DDecodeTable[bits].length;

		switch( MiArFax2DDecodeTable[bits].type )
		    {	/*
			**  State mode is codeword, codeword type is TERMINATOR,
			**  dispatch from here based on codeword value.
			*/
		case G4K_CodetypeTerminator :
		    switch( MiArFax2DDecodeTable[bits].value )
			{   /*
			    **	State mode is codeword, codeword type is
			    **	TERMINATOR, codeword value is PASS.
			    */
		    case G4K_CodevaluePass :
			while( state->rcl[state->ridx] <= state->a0prime ||
			     ((state->ridx & 1) == (state->cidx & 1)))
			    state->ridx++;
			state->b1 = state->rcl[state->ridx];
			state->a0prime = state->rcl[state->ridx + 1];
			if( state->ridx > 1
			 && state->rcl[state->ridx - 1] > state->a0prime )
			    state->ridx--;
			if( state->ccl[state->cidx] >= state->ppl )
			    {
			    state->mode = G4K_ModeNewScan;
			    return( G4K_StsEol );
			    }
			break;
		    /*
		    **	State mode is Codeword, codeword type is TERMINATOR,
		    **	codeword value is HORIZONTAL.
		    */
		    case G4K_CodevalueHoriz :
			if( state->a0 < 0 )
			    {
			    state->a0 = 0;
			    if( state->a0prime < 0 )
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
		    case G4K_CodevalueV0  :
		    case G4K_CodevalueVR1 :
		    case G4K_CodevalueVL1 :
		    case G4K_CodevalueVR2 :
		    case G4K_CodevalueVL2 :
		    case G4K_CodevalueVR3 :
		    case G4K_CodevalueVL3 :
			while( state->rcl[state->ridx] <= state->a0prime ||
			     ((state->ridx & 1) == (state->cidx & 1)))
			    state->ridx++;
			state->b1 = state->rcl[state->ridx];

			if( state->ridx > 1
			 && state->rcl[state->ridx - 1] > state->a0prime )
			    state->ridx--;
			state->a0 = state->b1+MiArFax2DDecodeTable[bits].value;
			state->a0prime = state->a0;
			state->ccl[++(state->cidx)] = state->a0;
			state->ccl[0]++;

			if( state->ccl[state->cidx] >= state->ppl )
			    {
			    state->mode = G4K_ModeNewScan;
			    return( G4K_StsEol );
			    }
			break;
		    /*
		    **	Here on invalid codeword values.
		    */
		    default: return( G4K_StsDcdErr );
		    }
		break;	/* End of switch (codeword type) */

		/*
		**  State mode is Codeword, codeword type is EOL.
		*/
		case G4K_CodetypeEol :
		    bits = GET_VALUE_(upBase_(src),state->srcpos,G4M_Codeword);
                    if( MiArFax2DDecodeTable[bits].type == G4K_CodetypeEol )
			{
			state->srcpos += MiArFax2DDecodeTable[bits].length;
                        return( G4K_StsEoi );
			}
                    else
			{
			state->mode = G4K_ModeNewScan;
			return( G4K_StsEol );
			}
		/*
		**  Here on invalid codeword types.
		*/
		default: return( G4K_StsDcdErr );
		}
	    break;  /* End of switch (codeword value)	*/

	case G4K_ModeFirstRun :
	    if( state->cidx & 1 == G4K_WhiteRun )
		{
		bits  = GET_VALUE_(upBase_(src), state->srcpos, G4M_WhiteRun);
		value = MiArFax1DDecodeWhite[bits].value;
		type  = MiArFax1DDecodeWhite[bits].type;
		state->srcpos += MiArFax1DDecodeWhite[bits].length;
		}
	    else
		{
		bits  = GET_VALUE_(upBase_(src), state->srcpos, G4M_BlackRun);
		value = MiArFax1DDecodeBlack[bits].value;
		type  = MiArFax1DDecodeBlack[bits].type;
		state->srcpos += MiArFax1DDecodeBlack[bits].length;
		}
	    state->rlen += value;

	    if( type != G4K_CodetypeMakeup )
		{
		state->a1 = state->rlen + state->a0prime;
		state->ccl[state->cidx++] = state->a1;
		state->ccl[0]++;
		state->rlen = 0;
		state->mode = G4K_ModeSecondRun;
		}
	    break;

	case G4K_ModeSecondRun :
	    if( state->cidx & 1 == G4K_WhiteRun )
		{
		bits  = GET_VALUE_(upBase_(src), state->srcpos, G4M_WhiteRun);
		value = MiArFax1DDecodeWhite[bits].value;
		type  = MiArFax1DDecodeWhite[bits].type;
		state->srcpos += MiArFax1DDecodeWhite[bits].length;
		}
	    else
		{
		bits  = GET_VALUE_(upBase_(src), state->srcpos, G4M_BlackRun);
		value = MiArFax1DDecodeBlack[bits].value;
		type  = MiArFax1DDecodeBlack[bits].type;
		state->srcpos += MiArFax1DDecodeBlack[bits].length;
		}
	    state->rlen += value;

	    if( type != G4K_CodetypeMakeup )
		{
		state->a0 = state->a1 + state->rlen;
		state->a0prime = state->a0;
		state->ccl[state->cidx] = state->a0;
		state->ccl[0]++;
		state->mode = G4K_ModeCodeword;

		if( state->ccl[state->cidx] >= state->ppl )
		    {
		    state->mode = G4K_ModeNewScan;
		    return( G4K_StsEol );
		    }
		}
	    break;
	/*
	**  Here on invalid state mode.
	*/
	default: return( G4K_StsDcdErr );
	    }   /* End of switch (state->mode) */

	    }   /* End of if (enough source data remaining) */
	else
	    {	/*
		**  Not enough source data, return source exhausted.
		*/
	    state->remlen = upArSize_(src) - state->srcpos;
	    state->remdat = GET_VALUE_( upBase_(src),
					state->srcpos, (1<<state->remlen)-1 );
	    return( G4K_StsSrcExh );
	    }
	}
}
