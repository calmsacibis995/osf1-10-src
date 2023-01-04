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
**      This module contains support for the pipelined Chrominance Combine
**      operator.  The non-pipelined operator is handled entirely in the DIX.
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
**      Wed Jul 11 10:43:28 1990
**
**  MODIFICATION HISTORY:
**
*******************************************************************************/

#include <stdio.h>

#include "SmiChromeCom.h"

/*
**  Table of contents
*/
int             SmiCreateChromeCom();

static int 	_ChromeComInitialize();
static int 	_ChromeComActivate();
static int 	_ChromeComDestroy();

/*
**  External References
*/

/*
**  Local storage
*/
    /*
    **  ChromeCom Element Vector
    */
static PipeElementVector ChromeComPipeElement = {
    NULL,				/* FLINK			    */
    NULL,				/* BLINK			    */
    sizeof(PipeElementVector),		/* Size				    */
    TypePipeElementVector,		/* Structure type		    */
    StypeChromeComElement,		/* Structure subtype		    */
    sizeof(ChromeComPipeCtx),		/* Context block size		    */
    0,					/* No input flag		    */
    0,					/* Reserved flags		    */
    _ChromeComInitialize,		/* Initialize entry		    */
    _ChromeComActivate,			/* Activate entry		    */
    NULL,				/* Flush entry			    */
    _ChromeComDestroy,			/* Destroy entry		    */
    NULL				/* Abort entry point		    */
    };

/*****************************************************************************
**  SmiCreateChromeCom
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function initializes the ChromeCom pipeline element
**
**  FORMAL PARAMETERS:
**
**      ctx - ChromeCom pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
int 	    SmiCreateChromeCom( pipe, srcsnk1, srcsnk2, srcsnk3, dstsnk )
Pipe		     pipe;
PipeSinkPtr	     srcsnk1;
PipeSinkPtr	     srcsnk2;
PipeSinkPtr	     srcsnk3;
PipeSinkPtr	     dstsnk;
{
    int i;
    ChromeComPipeCtxPtr ctx = (ChromeComPipeCtxPtr)
		       DdxCreatePipeCtx_( pipe, &ChromeComPipeElement, FALSE );
    if( !IsPointer_(ctx) ) return( (int) ctx );

    /*
    **	Copy parameters into context struct
    */
    ChcSrcSnk_(ctx,0) = srcsnk1;
    ChcSrcSnk_(ctx,1) = srcsnk2;
    ChcSrcSnk_(ctx,2) = srcsnk3;
    ChcDstSnk_(ctx) = dstsnk;

    /* Any input/output data types.  Quantum 1 */
    for( i = 0; i < XieK_MaxComponents; i++ )
	{
	ChcDrn_(ctx,i) = DdxRmCreateDrain_( ChcSrcSnk_(ctx,i), 1 );
	if( !IsPointer_(ChcDrn_(ctx,i)) ) return( (int) ChcDrn_(ctx,i) );
	DdxRmSetDType_( ChcDrn_(ctx,i), UdpK_DTypeUndefined, DtM_Any );
	DdxRmSetQuantum_( ChcDrn_(ctx,i), 1 );
	}
    DdxRmSetDType_( ChcDstSnk_(ctx), UdpK_DTypeUndefined, DtM_Any );
    DdxRmSetQuantum_( ChcDstSnk_(ctx), 1 );

    return( Success ); 
}					/* end SmiCreateChromeCom */

/*****************************************************************************
**  _ChromeComInitialize
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function initializes the ChromeCom pipeline element
**
**  FORMAL PARAMETERS:
**
**      ctx - ChromeCom pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 		_ChromeComInitialize(ctx)
ChromeComPipeCtxPtr	ctx;
{
    int i, dtype, status;

    CtxInp_(ctx) = ChcDrn_(ctx,0);

    for( i = 0; i<XieK_MaxComponents; i++ )
	{
	status  = DdxRmInitializePort_( CtxHead_(ctx), ChcDrn_(ctx,i) );
	if( status != Success ) return( status );

	if( i == 0 )
	    dtype = DdxRmGetDType_( ChcDrn_(ctx,i) );

	else if( dtype != DdxRmGetDType_(ChcDrn_(ctx,i)) )
	    return( BadMatch );
	}
    /*
    **	Set our destination sink DType to match our drains and initialize it.
    */
    DdxRmSetDType_( ChcDstSnk_(ctx), dtype, 1<<dtype );
    status = DdxRmInitializePort_( CtxHead_(ctx), ChcDstSnk_(ctx) );

    return( status ); 
}

/*****************************************************************************
**  _ChromeComActivate
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine performs the Chrominance Combine operation.
**
**  FORMAL PARAMETERS:
**
**      ctx - ChromeCom pipeline element context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _ChromeComActivate(ctx)
ChromeComPipeCtxPtr ctx;
{
    PipeDataPtr	src, dst;
    int status, i;

    for( status = Success, i=0;
	i < XieK_MaxComponents && status == Success; i++ ) {

	while( status == Success ) {

	    src = DdxRmGetData_( ctx, ChcDrn_(ctx,i) );
	    if( !IsPointer_(src) ) {
		status = (int) src;
		break;
	    }

	    if (DatY1_(src) <= SnkY2_(ChcDstSnk_(ctx),i)) {

		dst = DdxRmAllocData_( ChcDstSnk_(ctx), i,
				       DatY1_(src), DatHeight_(src) );
		if( !IsPointer_(dst) ) {
		    DdxRmDeallocData_(src);
		    status = (int) dst;
		    break;
		}

		status = DdxConvert_( DatUdpPtr_(src),
				      DatUdpPtr_(dst), XieK_MoveMode );
		if( status == Success )
		    status  = DdxRmPutData_( ChcDstSnk_(ctx), dst );
		else
		    DdxRmDeallocData_(dst);
	    }

	    DdxRmDeallocData_(src);
	}
    }

    return( status ); 
}

/*****************************************************************************
**  _ChromeComDestroy
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine deallocates all resources allocated by the ChromeCom
**	pipeline element.
**
**  FORMAL PARAMETERS:
**
**      ctx - ChromeCom pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _ChromeComDestroy(ctx)
ChromeComPipeCtxPtr    ctx;
{
    int i;

    for( i = 0; i < XieK_MaxComponents; i++ )
	ChcDrn_(ctx,i) = DdxRmDestroyDrain_( ChcDrn_(ctx,i) );

    return( Success ); 
}
