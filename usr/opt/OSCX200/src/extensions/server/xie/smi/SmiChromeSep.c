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
**      This module contains support for the pipelined Chrominance Separate
**      operator.  Non-pipelined ChromeSep is handled entirely in the DIX.
**
**  ENVIRONMENT:
**
**	VAX/VMS V5.3
**	ULTRIX  V3.1
**
**  AUTHOR(S):
**
**      Gary Grebus
**
**  DATE:
**
**      Wed Jul 11 12:38:21 1990
**
**  MODIFICATION HISTORY:
**
*******************************************************************************/

#include <stdio.h>

#include "SmiChromeSep.h"

/*
**  Table of contents
*/
int             SmiCreateChromeSep();

static int 	_ChromeSepInitialize();
static int 	_ChromeSepActivate();
static int 	_ChromeSepDestroy();

/*
**  External References
*/

/*
**  Local storage
*/
    /*
    **  ChromeSep Element Vector
    */
static PipeElementVector ChromeSepPipeElement = {
    NULL,				/* FLINK			    */
    NULL,				/* BLINK			    */
    sizeof(PipeElementVector),		/* Size				    */
    TypePipeElementVector,		/* Structure type		    */
    StypeChromeSepElement,		/* Structure subtype		    */
    sizeof(ChromeSepPipeCtx),		/* Context block size		    */
    0,					/* No input flag		    */
    0,					/* Reserved flags		    */
    _ChromeSepInitialize,		/* Initialize entry		    */
    _ChromeSepActivate,			/* Activate entry		    */
    NULL,				/* Flush entry			    */
    _ChromeSepDestroy,			/* Destroy entry		    */
    NULL				/* Abort entry point		    */
    };

/*****************************************************************************
**  SmiCreateChromeSep
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function initializes the CHROMESEP pipeline element
**
**  FORMAL PARAMETERS:
**
**      ctx - chromeSep pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
int 	    SmiCreateChromeSep( pipe, srcsnk, dstsnk, component )
Pipe		     pipe;
PipeSinkPtr	     srcsnk;
PipeSinkPtr	     dstsnk;
long int             component;
{
    ChromeSepPipeCtxPtr ctx = (ChromeSepPipeCtxPtr)
		       DdxCreatePipeCtx_( pipe, &ChromeSepPipeElement, FALSE );
    if( !IsPointer_(ctx) ) return( (int) ctx );

    /*
    **	Copy parameters into context struct
    */
    ChsSrcSnk_(ctx) = srcsnk;
    ChsDstSnk_(ctx) = dstsnk;
    ChsComp_(ctx)   = component;		/* The component to select */

    /* Handle any data type and quantum */
    ChsSrcDrn_(ctx) = DdxRmCreateDrain_( ChsSrcSnk_(ctx), 1 << component );
    if( !IsPointer_(ChsSrcDrn_(ctx)) ) return( (int) ChsSrcDrn_(ctx) );
    DdxRmSetDType_( ChsSrcDrn_(ctx), UdpK_DTypeUndefined, DtM_Any );
    DdxRmSetQuantum_( ChsSrcDrn_(ctx), 0 );

    DdxRmSetDType_( ChsDstSnk_(ctx), UdpK_DTypeUndefined, DtM_Any );
    DdxRmSetQuantum_( ChsDstSnk_(ctx), 0 );

    return( Success ); 
}					/* end SmiCreateChromeSep */

/*****************************************************************************
**  _ChromeSepInitialize
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function initializes the CHROMESEP pipeline element
**
**  FORMAL PARAMETERS:
**
**      ctx - chromeSep pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 		_ChromeSepInitialize(ctx)
ChromeSepPipeCtxPtr	ctx;
{
    int dtype, status;

    /*
    **	Initialize the source drain.
    */
    CtxInp_(ctx) = ChsSrcDrn_(ctx);
    status = DdxRmInitializePort_( CtxHead_(ctx), ChsSrcDrn_(ctx) );
    if( status != Success ) return( status );

    /*
    **	Set our destination sink DType to match our drain and initialize it.
    */
    dtype = DdxRmGetDType_( ChsSrcDrn_(ctx) );
    DdxRmSetDType_( ChsDstSnk_(ctx), dtype, 1<<dtype );
    status = DdxRmInitializePort_( CtxHead_(ctx), ChsDstSnk_(ctx) );

    return( status ); 
}

/*****************************************************************************
**  _ChromeSepActivate
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine controls performs the pipeline Chrominance Separation
**
**  FORMAL PARAMETERS:
**
**      ctx - chromeSep pipeline element context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _ChromeSepActivate(ctx)
ChromeSepPipeCtxPtr ctx;
{
    int status;
    PipeDataPtr	 src, dst;
    
    for( status = Success; status == Success; src = DdxRmDeallocData_(src) )
	{
	src = DdxRmGetData_(ctx, ChsSrcDrn_(ctx));
	if( !IsPointer_(src) ) return( (int) src );

	dst = DdxRmAllocData_(ChsDstSnk_(ctx), 0, DatY1_(src), DatHeight_(src));
	status = (int) dst;
	if( !IsPointer_(dst) ) continue;

	status = DdxConvert_( DatUdpPtr_(src), DatUdpPtr_(dst), XieK_MoveMode );

	if( status == Success )
	    status  = DdxRmPutData_( ChsDstSnk_(ctx), dst );
	else
	    DdxRmDeallocData_(dst);
	}

    return( status ); 
}

/*****************************************************************************
**  _ChromeSepDestroy
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine deallocates all resources allocated by the chromeSep
**	pipeline element.
**
**  FORMAL PARAMETERS:
**
**      ctx - chromeSep pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _ChromeSepDestroy(ctx)
ChromeSepPipeCtxPtr    ctx;
{
    ChsSrcDrn_(ctx) = DdxRmDestroyDrain_( ChsSrcDrn_(ctx) );

    return( Success ); 
}
