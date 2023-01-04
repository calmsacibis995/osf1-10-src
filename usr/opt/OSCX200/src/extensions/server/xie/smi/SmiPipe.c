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
**      This module contains user interface routines for pipelined processing
**
**  ENVIRONMENT:
**
**      VAX/VMS V5.0
**	VAX ULTRIX V3.0
**	RISC ULTRIX V3.0
**
**  AUTHOR(S):
**
**      Original: John Weber
**	Modified for optimized pipes: Richard Hennessy
**
**  CREATION DATE:
**
**      March 27, 1989
**
**  MODIFICATION HISTORY:
**	May 28, 1991 ==  Modified for optimized pipes
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
int		    SmiAbortPipeline();
PipeElementCtxPtr   SmiCreatePipeCtx();
Pipe		    SmiCreatePipeline();
int		    SmiDestroyPipeline();
int		    SmiFlushPipeline();
int		    SmiInitiatePipeline();
int		    SmiPipeHasYielded();
int		    SmiResumePipeline();

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
**  SmiAbortPipeline
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function will abort a potentially running pipe.
**	Only the active pipe, either base or optimized, will be aborted.
**
**  FORMAL PARAMETERS:
**
**      pipe = pointer to pipe hearder struct
**
**  FUNCTION VALUE:
**
**      [@description_or_none@]
**
************************************************************************/
int	SmiAbortPipeline( pipe )
 Pipe	pipe;
{
    int status, return_status = Success;
    PipeDataPtr	data;
    PipeElementCtxPtr	ctx;
    
    if( QueueEmpty_(pipe) ) return (Success);
    
    if (PipeOptPtr_(pipe) != NULL)
	/*
	**  Abort the optimized pipe.
	*/
	return( SmiAbortPipeline(PipeOptPtr_(pipe)) );

    for( ctx = CtxNxt_(pipe); CtxTyp_(ctx) != TypePipe; ctx = CtxNxt_(ctx) )
	{
	if( CtxAbort_(ctx) != NULL && CtxStatus_(ctx) != StatusComplete )
	    {
	    status = (*CtxAbort_(ctx))(ctx);
	    if( status != Success )
		return_status = status;
	    }
	CtxStatus_(ctx) = StatusComplete;
	}
    /*
    **  Free pipe data descriptor cache...
    */
    for( data = PipeDscFlk_(pipe);
	    data != PipeDsc_(pipe);
		data = PipeDscFlk_(pipe) )
	{
	RemQue_(data);
	DdxFree_(data);
	}
    return( return_status );
}				/* end of SmiAbortPipeline */

/************************************************************************
**  SmiCreatePipeCtx
**
**  FUNCTIONAL DESCRIPTION:
**
**      Create a pipe element of the speicified type and add it
**      to the current pipeline.
**
**  FORMAL PARAMETERS:
**
**      pipeline - pointer to the header of the pipe currently under
**                 construction
**      element  - pointer to the vector for the element being added
**      athead   - boolean indicating this element goes at the head
**                 of the pipe.
**
**  FUNCTION VALUE:
**
**      pointer to the newly created pipe context
**
************************************************************************/
PipeElementCtxPtr	SmiCreatePipeCtx( pipeline, element, athead )
 Pipe			pipeline;
 PipeElementVectorPtr	element;
 int                    athead;
{
    int	       count, *dst;
    QueuePtr	      pred;
    PipeElementCtxPtr ctx = (PipeElementCtxPtr)
			     DdxCalloc_(1, VecCtxSiz_(element));
    if( ctx != NULL )
	{   /*
	    **  Initialize misc context block fields.
	    */
	CtxSiz_(ctx)    = VecCtxSiz_(element);
	CtxTyp_(ctx)    = TypePipeElementCtx;
	CtxSubTyp_(ctx) = VecSubTyp_(element);
	CtxVecPtr_(ctx) = element;
	CtxHead_(ctx)   = pipeline;
	/*
	**  Insert context block on the pipe queue.
	*/
	pred = athead ? (QueuePtr) &(pipeline->bhd) :
	                (QueuePtr) pipeline->bhd.blink;
	InsQue_(ctx,pred);
	}
    else
	ctx = (PipeElementCtxPtr) BadAlloc;

    return( ctx );
}				/* end of SmiCreatePipeCtx */

/************************************************************************
**  SmiCreatePipeline
**
**  FUNCTIONAL DESCRIPTION:
**
**	This function allocates and initializes a pipeline header.
**
**  FORMAL PARAMETERS:
**
**	None.
**
**  FUNCTION VALUE:
**
**	PipelineHeadPtr
**
************************************************************************/
Pipe SmiCreatePipeline(slice)
    unsigned int slice;
{
    int	    i;
    Pipe pipe = (Pipe) DdxCalloc_(1,sizeof(PipelineHead));

    if( pipe != NULL )
	{
	pipe->bhd.flink		    = (BhdPtr) pipe;
	pipe->bhd.blink		    = (BhdPtr) pipe;
	pipe->bhd.size		    = sizeof(PipelineHead);
	pipe->bhd.type		    = TypePipe;
	PipePxlCnt_(pipe)	    = 0;
	PipeTimeSlice_(pipe)	    = slice;
	IniQue_( PipeDsc_(pipe) );
	PipeOptPtr_(pipe)	    = NULL;
	}
    else
	pipe = (Pipe) BadAlloc;

    return( pipe );
}				/* end of SmiCreatePipeline */

/************************************************************************
**  SmiDestroyPipeline
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function will destroy the pipeline.
**	If an optimized pipe then destroy it as well.
**
**  FORMAL PARAMETERS:
**
**      pipe  ==  pointer to a pipeline header block
**
**  FUNCTION VALUE:
**
************************************************************************/
int	SmiDestroyPipeline( pipe )
 Pipe pipe;
{
    int status, return_status = Success;
    PipeElementCtxPtr	ctx;
    PipeDataPtr	data;
    PipeElementCtxPtr	start;

    if( !IsPointer_(pipe) ) return( return_status );

    if( PipeOptPtr_(pipe) != NULL )
	/*
	** Destroy the optimized pipeline, which has all the resources
	** associated with it.  Any base pipe contexts which did not
	** get copied to the optimized pipe have already been called at
	** their Destroy routines by the pipeline optimizer.
	*/
	return_status = SmiDestroyPipeline( PipeOptPtr_(pipe) );

    else
	{   /*
	    ** Destroy the resources associated with the pipeline
	    */
	for( ctx = PipeCtxNxt_(pipe);
	    CtxTyp_(ctx) != TypePipe;  
	    ctx = PipeCtxNxt_(pipe) )
	    {   
	    if( !IsPointer_(ctx) )
		continue;
	    if( CtxDestroy_(ctx) != NULL )
		{
		status = (*CtxDestroy_(ctx))(ctx);
		if( status != Success )
		    return_status = status;
		}
	    RemQue_(ctx);
	    DdxCfree_(ctx);
	    }
	}
    /*
    ** Free the pipe element contexts that make up the base pipe.
    */
    for( ctx = (PipeElementCtxPtr)pipe->bhd.flink;  
	    CtxTyp_(ctx) != TypePipe;  
	    ctx = (PipeElementCtxPtr)pipe->bhd.flink )
	{   
	if( !IsPointer_(ctx) )
	    continue;
	RemQue_(ctx);
	DdxCfree_(ctx);
	}
    DdxFree_(pipe);

    return( return_status );
}					/* end of SmiDestroyPipeline */

/************************************************************************
**  SmiFlushPipeline
**
**  FUNCTIONAL DESCRIPTION:
**
**      Run the active pipe one more time and then cause each
**      processing element to perform its flush processing.
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
int	 SmiFlushPipeline( pipe )
 Pipe	pipe;
{
    PipeDataPtr	data;
    PipeElementCtxPtr	ctx;
    int status = Success;

    if( QueueEmpty_(pipe) ) return( Success );

    if ( PipeOptPtr_(pipe) != NULL )
	/*
	** Flush the optimized pipeline.
	*/
	status = SmiFlushPipeline(PipeOptPtr_(pipe) );

    else
	{   /*
	    **  Activate processes while the scheduler says they're ready.
	    */
	status = DdxResumePipeline_(pipe);
	if( DdxPipeHasYielded_(pipe) )
	    return( Success );

	/*
	**  When pipe stalls, flush contents...
	*/
	ctx =  (PipeElementCtxPtr) CtxNxt_(pipe);

	while( status == Success && CtxTyp_(ctx) != TypePipe )
	    {
	    if( CtxFlush_(ctx) != NULL && CtxStatus_(ctx) != StatusComplete )
		status = (*CtxFlush_(ctx))(ctx);

	    CtxStatus_(ctx) = StatusComplete;

	    if( status == Success )
		status  = DdxResumePipeline_( pipe );
	    if( DdxPipeHasYielded_(pipe) )
		break;

	    ctx = CtxNxt_(ctx);
	    }
	}
    /*
    **  Free pipe data descriptor cache...
    */
    for( data = PipeDscFlk_(pipe);
	    data != PipeDsc_(pipe);
		data = PipeDscFlk_(pipe) )
	{
	RemQue_(data);
	DdxFree_(data);
	}
    return( status );
}				/* end of SmiFlushPipeline */

/************************************************************************
**  SmiInitiatePipeline
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
int	SmiInitiatePipeline( pipe )
 Pipe	pipe;
{
    int  status = Success;
    PipeElementCtxPtr ctx;

    if( QueueEmpty_(pipe) ) return( Success );

    if ( PipeOptPtr_(pipe) != NULL )
	/*
	** Initialize the optimized pipeline.
	*/
	return( SmiInitiatePipeline(PipeOptPtr_(pipe)) );

    for( ctx = CtxNxt_(pipe); CtxTyp_(ctx) != TypePipe; ctx = CtxNxt_(ctx) )
	{
	if( CtxInit_(ctx) != NULL )
	    {
	    status = (*CtxInit_(ctx))(ctx);
	    if( status != Success )
		break;
	    }
	CtxStatus_(ctx) = StatusReady;
	}
    return( status );
}				/* end of SmiInitiatePipeline */

/************************************************************************
**  SmiPipeHasYielded
**
**  FUNCTIONAL DESCRIPTION:
**
**      Indicates if the specified pipeline has reached a point where
**      it wishes to yield control before completion.
**
**  FORMAL PARAMETERS:
**
**      pipe -  the DDX pipeline to test
**
**  FUNCTION VALUE:
**
**      TRUE if pipe has yielded, else FALSE;
**
************************************************************************/
int	SmiPipeHasYielded( pipe )
 Pipe	pipe;
{

    if( PipeOptPtr_( pipe ) != NULL )
	return( SmiPipeHasYielded( PipeOptPtr_( pipe ) ) );

    else
	return( PipePxlCnt_(pipe) > PipeTimeSlice_(pipe) &&
				    PipeTimeSlice_(pipe) != 0 );
	       
}					/* end of SmiPipeHasYielded */

/************************************************************************
**  SmiResumePipeline
**
**  FUNCTIONAL DESCRIPTION:
**
**      [@tbs@]
**
**  FORMAL PARAMETERS:
**
**      pipe -  the DDX pipeline to be run
**
**  FUNCTION VALUE:
**
**      [@description_or_none@]
**
************************************************************************/
int	SmiResumePipeline( pipe )
 Pipe	pipe;
{
    int    status = Success;
    PipeElementCtxPtr	ctx;

    if( QueueEmpty_(pipe) ) return( Success);
    
    if( PipeOptPtr_(pipe) != NULL )
	/*
	**  Resume the optimized pipe.
	*/
	return( SmiResumePipeline(PipeOptPtr_(pipe)) );

    /*
    **  Activate processes while the scheduler says they're ready.
    **  Keep running until the pipeline timeslice has been exceeded.
    */
    PipePxlCnt_(pipe) = 0;

    while( (ctx = DdxSchedProcessReady_(pipe)) != NULL &&
		 !DdxPipeHasYielded_(pipe) )
	{
	status = (*CtxActivate_(ctx))(ctx);
	if( status != Success )
	    break;
	}

    if( status == Success && DdxPipeHasYielded_(pipe) )
	/*
	** Reactivate any elements on the active pipe which wish to 
	** run independent of pending input.  This gives them an
	** opportunity to detect the pipeline has yielded.
	*/
	for( ctx = PipeCtxPrv_(pipe);
		(ctx = DdxSchedProcessNoInput_(pipe,ctx)) != NULL;
		    ctx = CtxPrv_(ctx) )
	    {
	    status = (*CtxActivate_(ctx))(ctx);
	    if( status != Success )
		break;
	    }
    return( status );
}				/* end of SmiResumePipeline */
/* end of module SmiPipe.c */
