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

/************************************************************************
**
**  FACILITY:
**
**      X Imaging Extension
**	Sample Machine Independant DDX
**
**  ABSTRACT:
**
**      This module contains pipeline schedule management routines.
**
**  ENVIRONMENT:
**
**      VAX/VMS V5.0
**	VAX ULTRIX V3.0
**	RISC ULTRIX V3.0
**
**  AUTHOR(S):
**
**      Michael O'Connor( Creator )
**	Richard Hennessy( Optimized pipe mods )
**	
**
**  CREATION DATE:
**
**      March 29, 1989
**
**  MODIFICATION HISTORY:
**
************************************************************************/

/*
**  Include files
*/
#include <stdio.h>

#include "SmiPipe.h"

/*
**  Table of contents
*/
PipeElementCtxPtr   SchedProcessComplete();
PipeElementCtxPtr   SchedProcessNoInput();
PipeElementCtxPtr   SchedProcessReady();
int		    SchedOkToRun();

/*
**  MACRO definitions
*/

/*
**  Equated Symbols
*/

/*
**  External References
*/

/*
**	Local Storage
*/

/************************************************************************
**  SchedProcessComplete
**
**  FUNCTIONAL DESCRIPTION:
**
**      [@tbs@]
**
**  FORMAL PARAMETERS:
**
**      [@description_or_none@]
**
**  FUNCTION VALUE:
**
**      [@description_or_none@]
**
************************************************************************/
PipeElementCtxPtr  SchedProcessComplete(ctx)
 PipeElementCtxPtr ctx;
{
    CtxStatus_(ctx) = StatusComplete;
}
  
/************************************************************************
**  SchedProcessNoInput
**
**  FUNCTIONAL DESCRIPTION:
**
**      Return the next pipeline context which is runnable by virtue
**      of having its NoInput flag set.
**
**	The scheduling is done from the current ctx to front.
**
**  FORMAL PARAMETERS:
**
**      pipe  ==  Pointer to a pipeline header struct
**		  (may be the base Smi pipeline or an optimized pipe)
**      ctx   ==  Pointer to the context to begin with.
**
**  FUNCTION VALUE:
**
**      NULL  ==  no element ready to run
**	ctx   ==  pointer to the context of the element to run
**
************************************************************************/
PipeElementCtxPtr SchedProcessNoInput( pipe, ctx )
 Pipe		  pipe;
 PipeElementCtxPtr ctx;
{
    /*
    **  Search for an element that is "ready" AND requires no input.
    */
    while( CtxTyp_(ctx) != TypePipe )
	{
	if( CtxStatus_(ctx) == StatusReady && VecNoInpt_(CtxVecPtr_(ctx)) )
	    /*
	    **  Return runnable element's context.
	    */
	    return( ctx );

	ctx = CtxPrv_(ctx);
	}
    return( NULL );
}					/* end of SchedProcessNoInput */
  
/************************************************************************
**  SchedProcessReady
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function selects the next pipeline element that is ready
**	 for processing.
**
**	The scheduling is done from back to front. This allows elements
**	that are stalled for input to be stepped over until an element
**	closer to the front of the pipe is found that can run. 
**
**  FORMAL PARAMETERS:
**
**      pipe  ==  Pointer to a pipeline header struct
**		  (may be the base Smi pipeline or an optimized pipe)
**
**  FUNCTION VALUE:
**
**      NULL  ==  no element ready to run
**	ctx   ==  pointer to the context of the element to run
**
************************************************************************/
PipeElementCtxPtr SchedProcessReady(pipe)
 Pipe		  pipe;
{
    PipeElementCtxPtr ctx = PipeCtxRun_(pipe);

    if( ctx != NULL )
	{   /*
	    **	Run this element -- it's already got the green light.
	    */
	PipeCtxRun_(pipe) = NULL;
	return( ctx );
	}

    /*
    **  Search for a runnable element -- it must be "ready" AND either
    **				  requires no input, OR it has quantum.
    */
    for( ctx = PipeCtxPrv_(pipe); CtxTyp_(ctx) != TypePipe; ctx = CtxPrv_(ctx) )
	if( CtxStatus_(ctx) == StatusReady
	 && VecNoInpt_(CtxVecPtr_(ctx))
	 || DdxRmHasQuantum_( CtxInp_(ctx) ) < XieK_MaxComponents )
	    /*
	    **  Return runnable element's context.
	    */
	    return( ctx );

    return( NULL );
}
  
/************************************************************************
**  SchedOkToRun
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function walks the pipe element list from "ctx" to back.
**
**  FORMAL PARAMETERS:
**
**      ctx	- current pipe element context
**
**  FUNCTION VALUE:
**
**       TRUE  ==  runnable -- no element closer to the end is runnable.
**	FALSE  ==  NOT ok to run -- another element has higher priority.
**
************************************************************************/
int   SchedOkToRun(ctx)
 PipeElementCtxPtr ctx;
{
    PipeElementCtxPtr    nxt_ctx;

    if( CtxStatus_(ctx) != StatusReady )
	return( FALSE );

    for( nxt_ctx = CtxNxt_(ctx);
	    CtxTyp_(nxt_ctx) != TypePipe;
		nxt_ctx = CtxNxt_(nxt_ctx) )
	{   /*
	    **	If a downline element is able to run (ie. doesn't need input,
	    **	or has quantum) then our caller is ineligable to run (ie. the
	    **	downline element should run next).
	    */
	if( CtxStatus_(nxt_ctx) == StatusReady &&
	  ( VecNoInpt_(CtxVecPtr_(nxt_ctx)) ||
	    DdxRmHasQuantum_( CtxInp_(nxt_ctx) ) < XieK_MaxComponents ) )
	    {
	    PipeCtxRun_(CtxHead_(ctx)) = nxt_ctx;
	    return( FALSE );
	    }
	}
    return( TRUE );
}
