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
**      This module implements the mathematical operator which performs
**	mathematical functions between two photomaps.
**
**  ENVIRONMENT:
**
**	VAX/VMS V5.3
**	ULTRIX  V3.1
**
**  AUTHORS:
**
**	Gary L. Grebus
**
**  CREATION DATE:     
**
**	Thu Jul  5 13:57:43 1990
**
*******************************************************************************/

#include <stdio.h>
#include <signal.h>
#include <setjmp.h>

#include "SmiMath.h"

/*
**  Table of contents
*/
int 		SmiMath();
int             SmiCreateMath();

static int 	_MathInitialize();
static int 	_MathActivate();
static int 	_MathDestroy();

static void	MathException();
static UdpPtr   MathGetTempUdp();
static void     MathDestroyTempUdp();
static int	MathConvertScanline();
static int	MathStoreScanline();

static int 	MathExp();
static int 	MathLn();
static int 	MathLog2();
static int 	MathLog10();
static int 	MathSquare();
static int 	MathSqrt();

/*
**  Equated Symbols
*/
/*
** Alpha/VMS and VXT don't currently support sigvec/setjmp/longjmp,
** and require explicit testing of every pixel value.  Otherwise,
** use exception handling to catch any bad values.
*/
#if (!defined(ALPHA) && !defined(_ALPHA) && !defined(VXT) || !(defined(VMS)))
#define XIE_USE_EXCEPTION_HANDLING
#endif

/*
**  External References
*/

/*
 * Macros
 */
#define Min_(a,b) ((a) <= (b) ? (a) : (b))
#define Max_(a,b) ((a) >  (b) ? (a) : (b))

#if defined(XIE_USE_EXCEPTION_HANDLING)
#define HandleFPE_( tvec, rtn, ovec ) \
	( tvec.sv_handler = rtn, tvec.sv_mask=0, tvec.sv_onstack=0, \
	  sigvec( SIGFPE, &tvec, &ovec) )

#define UnhandleFPE_( ovec ) \
	( sigvec( SIGFPE, &ovec, NULL ) )

/*
**  Local storage
*/
static jmp_buf exception_buf;

#endif

    /*
    **  Math Element Vector
    */
static PipeElementVector MathPipeElement = {
    NULL,				/* FLINK			    */
    NULL,				/* BLINK			    */
    sizeof(PipeElementVector),		/* Size				    */
    TypePipeElementVector,		/* Structure type		    */
    StypeMathElement,		        /* Structure subtype		    */
    sizeof(MathPipeCtx),		/* Context block size		    */
    0,					/* No input flag		    */
    0,					/* Reserved flags		    */
    _MathInitialize,		        /* Initialize entry		    */
    _MathActivate,		        /* Activate entry		    */
    NULL,			        /* Flush entry			    */
    _MathDestroy,			/* Destroy entry		    */
    NULL				/* Abort entry			    */
    };

/* Dispatch table to select the correct operation routine */
/* This must match the ordering of the XieK_xxxx constants in XieAppl.h */
static int (*MathOpTable[])() = {
    NULL,				/* Constants start at 1 */
    MathExp,
    MathLn,
    MathLog2,
    MathLog10,
    MathSquare,
    MathSqrt
    };

/*****************************************************************************
**  SmiCreateMath
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function fills in a newly created context for a MATH
**      pipeline element.
**
**  FORMAL PARAMETERS:
**
**      pipe    - Descriptor for pipe being built..
**      srcsnk	- Pointer to source sink
**      dstsnk  - Pointer to destination sink
**      idc     - Pointer to UDP describing the ROI or CPP (NULL if no IDC)
**      operator- Math operation function code.
**      
**
**  FUNCTION VALUE:
**
*****************************************************************************/
int 	    SmiCreateMath(pipe,srcsnk, dstsnk, idc, operator)
Pipe		     pipe;
PipeSinkPtr	     srcsnk;
PipeSinkPtr	     dstsnk;
UdpPtr		     idc;
int	     operator;

{
    MathPipeCtxPtr ctx = (MathPipeCtxPtr)
			  DdxCreatePipeCtx_( pipe, &MathPipeElement, FALSE );
    if( !IsPointer_(ctx) ) return( (int) ctx );
    /*
    **	Copy parameters
    */
    MthSrcSnk_(ctx) = srcsnk;
    MthDstSnk_(ctx) = dstsnk;
    MthFunc_(ctx)   = MathOpTable[operator];
    MthCpp_(ctx)    = idc != NULL && idc->UdpA_Base != NULL ? idc : NULL;

    /* Calculate the bounds of the region that will actually be processed */
    if( idc == NULL )
	{
	MthRoiX1_(ctx) = SnkX1_(srcsnk,0);
	MthRoiX2_(ctx) = SnkX2_(srcsnk,0);
	MthRoiY1_(ctx) = SnkY1_(srcsnk,0);
	MthRoiY2_(ctx) = SnkY2_(srcsnk,0);
	}
    else
	{
	/* Further restrict the area to be processed with the CPP/ROI  */
	MthRoiX1_(ctx) = idc->UdpL_X1 > SnkX1_(srcsnk,0) ?
			 idc->UdpL_X1 : SnkX1_(srcsnk,0);
	MthRoiX2_(ctx) = idc->UdpL_X2 < SnkX2_(srcsnk,0) ?
			 idc->UdpL_X2 : SnkX2_(srcsnk,0);
	MthRoiY1_(ctx) = idc->UdpL_Y1 > SnkY1_(srcsnk,0) ?
			 idc->UdpL_Y1 : SnkY1_(srcsnk,0);
	MthRoiY2_(ctx) = idc->UdpL_Y2 < SnkY2_(srcsnk,0) ?
			 idc->UdpL_Y2 : SnkY2_(srcsnk,0);
	}

    /* Create drains on the sink(s).
     * Handle all the data as floats.
     * Process a quantum of 1 scanline
     */
    MthSrcDrn_(ctx) = DdxRmCreateDrain_( MthSrcSnk_(ctx), -1 );
    if( !IsPointer_(MthSrcDrn_(ctx)) ) return( (int) MthSrcDrn_(ctx));
    DdxRmSetDType_( MthSrcDrn_(ctx), UdpK_DTypeF, DtM_F );
    DdxRmSetQuantum_( MthSrcDrn_(ctx), 1 );

    DdxRmSetDType_( MthDstSnk_(ctx), UdpK_DTypeF, DtM_F );
    DdxRmSetQuantum_( MthDstSnk_(ctx), 1 );

    return( Success );
}					/* end SmiCreateMath */

/*****************************************************************************
**  _MathInitialize
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function initializes the MATH pipeline element prior to
**	activation.
**
**  FORMAL PARAMETERS:
**
**      ctx - math pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	_MathInitialize(ctx)
MathPipeCtxPtr	ctx;
{
    int status  = DdxRmInitializePort_( CtxHead_(ctx), MthSrcDrn_(ctx) );
    if( status == Success )
	status  = DdxRmInitializePort_( CtxHead_(ctx), MthDstSnk_(ctx) );

    /* Start reading from the first drain */
    CtxInp_(ctx) = MthSrcDrn_(ctx);

    return( status );
}

/*****************************************************************************
**  _MathActivate
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine reads the data from the source drain and
**      performs the math operations.
**
**  FORMAL PARAMETERS:
**
**      ctx - math pipeline element context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _MathActivate(ctx)
MathPipeCtxPtr    ctx;
{
    PipeDataPtr	src = NULL, dst = NULL;
    int status, dstpos;

#if defined(XIE_USE_EXCEPTION_HANDLING)
    struct sigvec tmpvec, oldvec;

    /*
     * Establish error handling to catch possible floating point errors.
     */
    HandleFPE_(tmpvec, MathException, oldvec);
    if (setjmp(exception_buf) != 0) {

	/* On an exception, clean up locally held resources */
	UnhandleFPE_( oldvec );
	if ( src != NULL ) DdxRmDeallocData_(src);
	if ( dst != NULL ) DdxRmDeallocData_(dst);
	return( BadValue );
    }
#endif

    for( status = Success; status == Success; src = DdxRmDeallocData_(src),
					      dst = DdxRmDeallocData_(dst) )
	{
	src = DdxRmGetData_( ctx, MthSrcDrn_(ctx) );
	if( !IsPointer_(src) ) return( (int) src );

	if( DatY1_(src) < MthRoiY1_(ctx) || DatY1_(src) > MthRoiY2_(ctx) )
	    {
	    /* Scanline is outside the ROI - just pass it through */
	    status = DdxRmPutData_( MthDstSnk_(ctx), src );
	    src    = NULL;
	    }
	else
	    {
	    dst = DdxRmAllocData_( MthDstSnk_(ctx), DatCmpIdx_(src),
				   DatY1_(src), DatHeight_(src) );
	    status = (int) dst;
	    if( !IsPointer_(dst) ) continue;
 
	    /* Copy the portions of this line outside the ROI */
	    /* Left  */
	    status = DdxCopy_( DatUdpPtr_(src), DatUdpPtr_(dst),
			       DatX1_(src), 0,
			       DatX1_(dst), DatY1_(dst),
			       MthRoiX1_(ctx) - DatX1_(src), 1 );
	    if( status != Success ) continue;
	    /* Right */	    
	    status = DdxCopy_( DatUdpPtr_(src), DatUdpPtr_(dst),
			       MthRoiX2_(ctx)+1, 0,
			       MthRoiX2_(ctx)+1, DatY1_(dst),
			       DatX2_(src) - MthRoiX2_(ctx), 1 );
	    if( status != Success ) continue;	    

	    DatPos_(src) += (MthRoiX1_(ctx) - DatX1_(src)) * DatPxlStr_(src);
	    dstpos	  = DatPos_(dst);
	    DatPos_(dst) += (MthRoiX1_(ctx) - DatX1_(dst)) * DatPxlStr_(dst);;

	    status = (*MthFunc_(ctx))(DatUdpPtr_(src),
				      DatUdpPtr_(dst),
				      MthCpp_(ctx),
				      MthRoiY1_(ctx),
				      MthRoiX1_(ctx),
				      MthRoiX2_(ctx));
	    if( status != Success ) continue;

	    DatPos_(dst) = dstpos;
	    status = DdxRmPutData_( MthDstSnk_(ctx), dst );
	    dst    = NULL;
	    }
	}
#if defined(XIE_USE_EXCEPTION_HANDLING)
    UnhandleFPE_( oldvec );
#endif
    return( status );
}

/*****************************************************************************
**  _MathDestroy
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine deallocates all resources allocated by the MATH
**	pipeline element.
**
**  FORMAL PARAMETERS:
**
**      ctx - math pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _MathDestroy(ctx)
MathPipeCtxPtr    ctx;
{
    MthSrcDrn_(ctx) = DdxRmDestroyDrain_( MthSrcDrn_(ctx) );

    return( Success );
}

/*****************************************************************************
**  SmiMath
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine performs a non-pipelined math operator, which
**      evaluates a mathematical function for each pixel in the photomap.
** 
**      Note that the destination photomap must be of type float or
**      the data will be transformed from unconstrained to a constrained
**      type in an undefined manner.
**
**  FORMAL PARAMETERS:
**
**	srcudp - Pointer to a UDP describing the source operand.
**      dstudp  - Pointer to a UDP describing the destination buffer.
**      idc     - Pointer to a UDP describing the Region of Interest,
**                or Control Processing Plane.
**                (NULL if no IDC)
**      operator-Code for the operation being performed.
**
**  FUNCTION VALUE:
**
*****************************************************************************/
int 	SmiMath( srcudp, dstudp, idc, operator )
UdpPtr			srcudp;
UdpPtr			dstudp;
UdpPtr	 		idc;
int		operator;
{
    UdpPtr   cpp, src = NULL, dst = NULL;
    long int pixels, scanline, status;
    long int X1, X2, Y1, Y2;		/* src/destination common bounds     */
    long int x1, x2, y1, y2;		/* src/destination/ROI common bounds */

#if defined(XIE_USE_EXCEPTION_HANDLING)
    struct sigvec tmpvec, oldvec;
     /*
     * Establish error handling to catch possible floating point errors.
     */
    HandleFPE_( tmpvec, MathException, oldvec );
    if( setjmp( exception_buf ) != 0 )
	{
	status = BadValue;
	goto clean_up;
	}
#endif
    /*
    **  Allocate the destination image array
    */
    if( dstudp->UdpA_Base == NULL )
	{
        dstudp->UdpA_Base = DdxMallocBits_( dstudp->UdpL_ArSize );
        if( dstudp->UdpA_Base == NULL ) return( BadAlloc );
	}
    /*
     * Determine maximum common dimensions between source and destination,
     * which is the defined region of output.
     */
    X1 = srcudp->UdpL_X1 > dstudp->UdpL_X1 ? srcudp->UdpL_X1 : dstudp->UdpL_X1;
    X2 = srcudp->UdpL_X2 < dstudp->UdpL_X2 ? srcudp->UdpL_X2 : dstudp->UdpL_X2;
    Y1 = srcudp->UdpL_Y1 > dstudp->UdpL_Y1 ? srcudp->UdpL_Y1 : dstudp->UdpL_Y1;
    Y2 = srcudp->UdpL_Y2 < dstudp->UdpL_Y2 ? srcudp->UdpL_Y2 : dstudp->UdpL_Y2;
    /* 
     * Further restrict the area to be processed by finding the maximum
     * common dimensions between the source region, and the IDC
     * (ROI or CPP).
     */
    if( idc == NULL )
	{
	x1 = X1;
	x2 = X2;
	y1 = Y1;
	y2 = Y2;
	}
    else
	{
	x1 = idc->UdpL_X1 > X1 ? idc->UdpL_X1 : X1;
	x2 = idc->UdpL_X2 < X2 ? idc->UdpL_X2 : X2;
	y1 = idc->UdpL_Y1 > Y1 ? idc->UdpL_Y1 : Y1;
	y2 = idc->UdpL_Y2 < Y2 ? idc->UdpL_Y2 : Y2;
	}
    cpp = idc != NULL && idc->UdpA_Base != NULL ? idc : NULL;
	
    /* Allocate temporary UDP's to hold one scanline of float data */
    src = MathGetTempUdp( srcudp, y1, x1, x2 );
    if( !IsPointer_(src) )
        {
        status = (int) src;
        goto clean_up;
        }
    dst = MathGetTempUdp( dstudp, y1, x1, x2 );
    if( !IsPointer_(dst) )
        {
        status = (int) dst;
        goto clean_up;
        }

    /* Just copy the scanlines outside of the ROI */
    /* Top */
    status = DdxCopy_( srcudp, dstudp, X1, Y1, X1, Y1, X2-X1+1, y1-Y1 );
    if( status != Success ) goto clean_up;
    /* Bottom */
    status = DdxCopy_( srcudp, dstudp, X1, y2+1, X1, y2+1, X2-X1+1, Y2-y2 );
    if( status != Success ) goto clean_up;
    /* Left */
    status = DdxCopy_( srcudp, dstudp, X1, y1, X1, y1, x1-X1, y2-y1+1 );
    if( status != Success ) goto clean_up;
    /* Right */    
    status = DdxCopy_( srcudp, dstudp, x2+1, y1, x2+1, y1, X2-x2, y2-y1+1 );
    if( status != Success ) goto clean_up;

    /* If the temporary UDP's are pointing to the original data, position
     * to the correct offset.
     */
    if( src->UdpA_Base == srcudp->UdpA_Base )
	src->UdpL_Pos   = srcudp->UdpL_Pos + srcudp->UdpL_ScnStride * y1
					   + srcudp->UdpL_PxlStride *
					     (x1 - srcudp->UdpL_X1);
    dst->UdpL_Y1 = dst->UdpL_Y2 = y1;
    if( dst->UdpA_Base == dstudp->UdpA_Base )
	dst->UdpL_Pos   = dstudp->UdpL_Pos + y1 * dstudp->UdpL_ScnStride +
			 (x1 - dstudp->UdpL_X1) * dstudp->UdpL_PxlStride;

    /* Now perform the math operation over the region-of-interest */

    for( scanline = y1; scanline <= y2; scanline++ )
	{
	/* Get a UDP pointing to this scanline */
	status = MathConvertScanline( srcudp, src );
	if( status != Success ) break;
	    
	status = (*MathOpTable[operator])(src, dst, cpp, y1, x1, x2);
	if( status != Success ) break;
	
	status = MathStoreScanline(dst, dstudp);
	if( status != Success ) break;
	}

    /* Cleanup the temporary descriptors */
clean_up :

#if defined(XIE_USE_EXCEPTION_HANDLING)
    UnhandleFPE_( oldvec );
#endif

    if( IsPointer_(src) )
	MathDestroyTempUdp( src, srcudp );
    if( IsPointer_(dst) )
	MathDestroyTempUdp( dst, dstudp );

    return( status );
}					/* end SmiMath */

/*****************************************************************************
**  Math processing routines
**
**  FUNCTIONAL DESCRIPTION:
**
**	These routines the actual math operation over selected
**      portion of a scanline.  A separate routine is used for each
**      type of operation.
**
**      The explicit tests for bad pixel values are highly implementation
**      dependent and probably incomplete.
**
**  FORMAL PARAMETERS:
**
**	src     - Pointer to a UDP describing the source operand.
**      dst     - Pointer to a UDP describing the destination buffer.
**      cpp     - Pointer to a UDP describing the Control Processing Plane.
**                (NULL if no CPP)
**      y1      - The y coordinate of the beginning of the region to
*                 process.
**      x1      - The x coordinate of the left endpoint of the region to
**                process.
**      x2      - The x coordinate of the right endpoint of the region to
**                process.
**
*****************************************************************************/
static int  MathExp(src, dst, cpp,  y1, x1, x2)
UdpPtr	     	src;
UdpPtr	     	dst;
UdpPtr		cpp;
long int        y1;
long int	x1;
long int	x2;
{
    int			pixel;

    unsigned char 	*srcptr;
    unsigned char       *dstptr;
    long int		cpppxloff;
    int			cppbit;

    int			srcbytes = src->UdpL_PxlStride >> 3;
    int                 dstbytes  = dst->UdpL_PxlStride >> 3;
    
    srcptr = src->UdpA_Base + (src->UdpL_Pos >> 3);
    dstptr = dst->UdpA_Base + (dst->UdpL_Pos >> 3);

    if (cpp != NULL)
	cpppxloff = cpp->UdpL_Pos +
	    cpp->UdpL_ScnStride * (src->UdpL_Y1 - y1);
    else
	cppbit = 1;			/* Fake all 1's in CPP */

    for (pixel = x1; pixel <= x2; pixel++){
	
	if (cpp != NULL) {
	    cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
	    cpppxloff+=cpp->UdpL_PxlStride;
	}	  
	
	if (cppbit) {
#if !defined(XIE_USE_EXCEPTION_HANDLING)
	    if (*(float *)srcptr > 88.029 )  /* VAX MTH$DEXP */
	        return( BadValue );
#endif
	    *(float *)dstptr = exp(*(float *)srcptr);
	} else
	    *(float *)dstptr = *(float *)srcptr; /* Not selected by CPP */
	
	srcptr+=srcbytes;
	dstptr+=dstbytes;

    }
    return( Success );
}					/* end MathExp */

static int  MathLn(src, dst, cpp,  y1, x1, x2)
UdpPtr	     	src;
UdpPtr	     	dst;
UdpPtr		cpp;
long int        y1;
long int	x1;
long int	x2;
{
    int			pixel;

    unsigned char 	*srcptr;
    unsigned char       *dstptr;
    long int		cpppxloff;
    int			cppbit;

    int			srcbytes = src->UdpL_PxlStride >> 3;
    int                 dstbytes  = dst->UdpL_PxlStride >> 3;
    
    srcptr = src->UdpA_Base + (src->UdpL_Pos >> 3);
    dstptr = dst->UdpA_Base + (dst->UdpL_Pos >> 3);

    if (cpp != NULL)
	cpppxloff = cpp->UdpL_Pos +
	    cpp->UdpL_ScnStride * (src->UdpL_Y1 - y1);
    else
	cppbit = 1;			/* Fake all 1's in CPP */

    for (pixel = x1; pixel <= x2; pixel++){
	
	if (cpp != NULL) {
	    cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
	    cpppxloff+=cpp->UdpL_PxlStride;
	}	  
	
	if (cppbit) {
#if !defined(XIE_USE_EXCEPTION_HANDLING)
	    if (*(float *)srcptr <= 0.0)
	        return( BadValue );
#endif
	    *(float *)dstptr = log(*(float *)srcptr);
	} else
	    *(float *)dstptr = *(float *)srcptr; /* Not selected by CPP */
	
	srcptr+=srcbytes;
	dstptr+=dstbytes;

    }
    return( Success );
}					/* end MathLn */

static int  MathLog2(src, dst, cpp,  y1, x1, x2)
UdpPtr	     	src;
UdpPtr	     	dst;
UdpPtr		cpp;
long int        y1;
long int	x1;
long int	x2;
{
    int			pixel;

    unsigned char 	*srcptr;
    unsigned char       *dstptr;
    long int		cpppxloff;
    int			cppbit;

    int			srcbytes = src->UdpL_PxlStride >> 3;
    int                 dstbytes  = dst->UdpL_PxlStride >> 3;
    double              lntwo = log(2.0);
    
    srcptr = src->UdpA_Base + (src->UdpL_Pos >> 3);
    dstptr = dst->UdpA_Base + (dst->UdpL_Pos >> 3);

    if (cpp != NULL)
	cpppxloff = cpp->UdpL_Pos +
	    cpp->UdpL_ScnStride * (src->UdpL_Y1 - y1);
    else
	cppbit = 1;			/* Fake all 1's in CPP */

    for (pixel = x1; pixel <= x2; pixel++){
	
	if (cpp != NULL) {
	    cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
	    cpppxloff+=cpp->UdpL_PxlStride;
	}	  
	
	if (cppbit) {
#if !defined(XIE_USE_EXCEPTION_HANDLING)
	    if (*(float *)srcptr <= 0.0)
	        return( BadValue );
#endif
	    *(float *)dstptr = log((*(float *)srcptr))/lntwo;
	} else
	    *(float *)dstptr = *(float *)srcptr; /* Not selected by CPP */
	
	srcptr+=srcbytes;
	dstptr+=dstbytes;

    }
    return( Success );
}					/* end MathLog2 */

static int  MathLog10(src, dst, cpp,  y1, x1, x2)
UdpPtr	     	src;
UdpPtr	     	dst;
UdpPtr		cpp;
long int        y1;
long int	x1;
long int	x2;
{
    int			pixel;

    unsigned char 	*srcptr;
    unsigned char       *dstptr;
    long int		cpppxloff;
    int			cppbit;

    int			srcbytes = src->UdpL_PxlStride >> 3;
    int                 dstbytes  = dst->UdpL_PxlStride >> 3;
    
    srcptr = src->UdpA_Base + (src->UdpL_Pos >> 3);
    dstptr = dst->UdpA_Base + (dst->UdpL_Pos >> 3);

    if (cpp != NULL)
	cpppxloff = cpp->UdpL_Pos +
	    cpp->UdpL_ScnStride * (src->UdpL_Y1 - y1);
    else
	cppbit = 1;			/* Fake all 1's in CPP */

    for (pixel = x1; pixel <= x2; pixel++){
	
	if (cpp != NULL) {
	    cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
	    cpppxloff+=cpp->UdpL_PxlStride;
	}	  
	
	if (cppbit) {
#if !defined(XIE_USE_EXCEPTION_HANDLING)
	    if (*(float *)srcptr <= 0.0)
	        return( BadValue );
#endif
	    *(float *)dstptr = log10(*(float *)srcptr);
	} else
	    *(float *)dstptr = *(float *)srcptr; /* Not selected by CPP */
	
	srcptr+=srcbytes;
	dstptr+=dstbytes;

    }
    return( Success );
}					/* end MathLog10 */

static int  MathSquare(src, dst, cpp,  y1, x1, x2)
UdpPtr	     	src;
UdpPtr	     	dst;
UdpPtr		cpp;
long int        y1;
long int	x1;
long int	x2;
{
    int			pixel;

    unsigned char 	*srcptr;
    unsigned char       *dstptr;
    long int		cpppxloff;
    int			cppbit;

    int			srcbytes = src->UdpL_PxlStride >> 3;
    int                 dstbytes  = dst->UdpL_PxlStride >> 3;
    
    srcptr = src->UdpA_Base + (src->UdpL_Pos >> 3);
    dstptr = dst->UdpA_Base + (dst->UdpL_Pos >> 3);

    if (cpp != NULL)
	cpppxloff = cpp->UdpL_Pos +
	    cpp->UdpL_ScnStride * (src->UdpL_Y1 - y1);
    else
	cppbit = 1;			/* Fake all 1's in CPP */

    for (pixel = x1; pixel <= x2; pixel++){
	
	if (cpp != NULL) {
	    cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
	    cpppxloff+=cpp->UdpL_PxlStride;
	}	  
	
	if (cppbit)
	    *(float *)dstptr = *(float *)srcptr * *(float *)srcptr;
	else
	    *(float *)dstptr = *(float *)srcptr; /* Not selected by CPP */
	
	srcptr+=srcbytes;
	dstptr+=dstbytes;

    }
    return( Success );
}					/* end MathSquare */

static int  MathSqrt(src, dst, cpp,  y1, x1, x2)
UdpPtr	     	src;
UdpPtr	     	dst;
UdpPtr		cpp;
long int        y1;
long int	x1;
long int	x2;
{
    int			pixel;

    unsigned char 	*srcptr;
    unsigned char       *dstptr;
    long int		cpppxloff;
    int			cppbit;

    int			srcbytes = src->UdpL_PxlStride >> 3;
    int                 dstbytes  = dst->UdpL_PxlStride >> 3;
    
    srcptr = src->UdpA_Base + (src->UdpL_Pos >> 3);
    dstptr = dst->UdpA_Base + (dst->UdpL_Pos >> 3);

    if (cpp != NULL)
	cpppxloff = cpp->UdpL_Pos +
	    cpp->UdpL_ScnStride * (src->UdpL_Y1 - y1);
    else
	cppbit = 1;			/* Fake all 1's in CPP */

    for (pixel = x1; pixel <= x2; pixel++){
	
	if (cpp != NULL) {
	    cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
	    cpppxloff+=cpp->UdpL_PxlStride;
	}	  
	
	if (cppbit) {
#if !defined(XIE_USE_EXCEPTION_HANDLING)
	    if (*(float *)srcptr < 0.0)
	        return( BadValue );
#endif
	    *(float *)dstptr = sqrt(*(float *)srcptr);
	} else
	    *(float *)dstptr = *(float *)srcptr; /* Not selected by CPP */
	
	srcptr+=srcbytes;
	dstptr+=dstbytes;

    }
    return( Success );
}					/* end MathSqrt */

/*****************************************************************************
**  MathException
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine is our signal handler for a floating point exception.
**
**  FORMAL PARAMETERS:
**
**	none
**
**  FUNCTION VALUE:
**
**      Returns a 1 through the setjmp. 
**
*****************************************************************************/
static void MathException( )
{
#if defined(XIE_USE_EXCEPTION_HANDLING)
    longjmp( exception_buf, 1 );
#endif
}

/*****************************************************************************
**  MathGetTempUdp
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create a UDP which can be used to hold one scanline of the 
**      source UDP, but using float data.  If the source UDP is already
*       in float format, just point to its data.
**
**  FORMAL PARAMETERS:
**
**	inudp - Pointer to source UDP
**      y1    - First scanline we need
**      x1    - Leftmost pixel we need
**      x2    - Rightmost pixel we need
**
**  FUNCTION VALUE:
**
**      Returns a pointer to a UDP describing floating point data.
**
*****************************************************************************/
static UdpPtr MathGetTempUdp(src, y1, x1, x2)
    UdpPtr src;
    long int y1;
    long int x1, x2;
{
    UdpPtr dst;

    dst = (UdpPtr) DdxMalloc_(sizeof(UdpRec));
    if( dst == NULL ) return( (UdpPtr) BadAlloc );

    if( src->UdpB_DType != UdpK_DTypeF )
	{
	dst->UdpW_PixelLength	= sizeof(float) * 8;
	dst->UdpB_DType		= UdpK_DTypeF;
	dst->UdpB_Class		= UdpK_ClassA;
	dst->UdpL_PxlStride	= dst->UdpW_PixelLength;
	dst->UdpL_PxlPerScn	= x2 - x1 + 1;
	dst->UdpL_ScnStride	= dst->UdpL_PxlPerScn * dst->UdpL_PxlStride;
	dst->UdpL_ArSize	= dst->UdpL_PxlPerScn * sizeof(float) << 3;
	dst->UdpA_Base		= DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL )
	    {
	    DdxFree_(dst);
	    return( (UdpPtr) BadAlloc );
	    }
	dst->UdpL_Pos		= 0;
	dst->UdpL_CompIdx	= src->UdpL_CompIdx;
	dst->UdpL_Levels	= src->UdpL_Levels;
	}
    else 
	{
	*dst = *src;
	dst->UdpL_PxlPerScn	= x2 - x1 + 1;
	dst->UdpL_ScnStride	= dst->UdpL_PxlPerScn * dst->UdpL_PxlStride;
	}
    /* One scanline UDP */
    dst->UdpL_X1     = x1;
    dst->UdpL_X2     = x2;
    dst->UdpL_Y1     = y1;
    dst->UdpL_Y2     = y1;
    dst->UdpL_ScnCnt =  1;

    return( dst );
}					/* end MathGetTempUdp */

/*****************************************************************************
**  MathDestroyTempUdp
**
**  FUNCTIONAL DESCRIPTION:
**
**	Free a UDP created by MathGetTempUdp.
**
**  FORMAL PARAMETERS:
**
**	udp     - Pointer to the udp to free.
**      realudp - Pointer to a udp which might actually own the storage
**                pointed to by udp.
**
*****************************************************************************/
static void MathDestroyTempUdp( udp, realudp )
UdpPtr udp;
UdpPtr realudp;

{
    if( IsPointer_(udp) )
	{
	if( IsPointer_(udp->UdpA_Base) && udp->UdpA_Base != realudp->UdpA_Base )
	    DdxFreeBits_( udp->UdpA_Base );
	DdxFree_( udp );
	}
}					/* end MathDestroyTempUdp */

/*****************************************************************************
**  MathConvertScanline
**
**  FUNCTIONAL DESCRIPTION:
**
**	Adjust a UDP to point to the specified scanline of the source
**	UDP.  If the source UDP is not of type float, convert the
**      data and leave it associated with the adjusted UDP.
**
**  FORMAL PARAMETERS:
**
**	src      - Pointer to the source UDP.
**      dst      - Pointer to the UDP to be adjusted.
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int MathConvertScanline(src,  dst)
    UdpPtr src;
    UdpPtr dst;
{
    int status = Success;

    dst->UdpL_Y1++;
    dst->UdpL_Y2++;

    if (src->UdpB_DType != UdpK_DTypeF)
	status = DdxConvert_( src, dst, XieK_MoveMode );
    else
	dst->UdpL_Pos += src->UdpL_ScnStride;

    return( status );
}					/* end MathConvertScanline */

/*****************************************************************************
**  MathStoreScanline
**
**  FUNCTIONAL DESCRIPTION:
**
**	Store a scanline of data into the destination UDP.
**      If the scanline UDP already points into the destination UDP,
**      just adjust the scanline UDP to point past the scanline already
**      stored.  If not, copy (and possibly convert) the data into the
**      destination.
**
**  FORMAL PARAMETERS:
**
**	dst        - Pointer to scanline UDP
**      realdst    - Pointer to the destination UDP
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int  MathStoreScanline(dst, realdst)
    UdpPtr dst;
    UdpPtr realdst;
{
    int status = Success;

    if (dst->UdpA_Base != realdst->UdpA_Base)
	status = DdxConvert_( dst, realdst, XieK_MoveMode );
    else 
	dst->UdpL_Pos += realdst->UdpL_ScnStride;

    dst->UdpL_Y1++;
    dst->UdpL_Y2++;

    return( status );
}					/* end MathStoreScanline */
