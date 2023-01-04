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
**      This module implements the arithmetic operator which performs
**	arithmetic functions between two photomaps.
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
**	Fri Apr 20 15:55:27 1990
**
*******************************************************************************/

#include <stdio.h>
#include <signal.h>
#include <setjmp.h>

#include "SmiArithmetic.h"

/*
**  Table of contents
*/
int 		SmiArithmetic();
int		SmiCreateArithmetic();

static int 	_ArithmeticInitialize();
static int 	_ArithmeticActivate();
static int 	_ArithmeticFlush();
static int 	_ArithmeticAbort();
static int 	_ArithmeticDestroy();

static void	ArithmeticException();
static UdpPtr   ArithmeticGetTempUdp();
static void     ArithmeticDestroyTempUdp();
static int	ArithmeticConvertScanline();
static int	ArithmeticStoreScanline();

static int	ArithmeticAddII();
static int	ArithmeticAddIC();
static int	ArithmeticSubII();
static int	ArithmeticSubIC();
static int	ArithmeticSubCI();
static int	ArithmeticMulII();
static int	ArithmeticMulIC();
static int	ArithmeticDivII();
static int	ArithmeticDivIC();
static int	ArithmeticDivCI();
static int	ArithmeticMinII();
static int	ArithmeticMinIC();
static int	ArithmeticMaxII();
static int	ArithmeticMaxIC();

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
    **  Arithmetic Element Vector
    */
static PipeElementVector ArithmeticPipeElement = {
    NULL,				/* FLINK			    */
    NULL,				/* BLINK			    */
    sizeof(PipeElementVector),		/* Size				    */
    TypePipeElementVector,		/* Structure type		    */
    StypeArithmeticElement,		/* Structure subtype		    */
    sizeof(ArithmeticPipeCtx),		/* Context block size		    */
    0,					/* No input flag		    */
    0,					/* Reserved flags		    */
    _ArithmeticInitialize,		/* Initialize entry		    */
    _ArithmeticActivate,		/* Activate entry		    */
    _ArithmeticFlush,			/* Flush entry			    */
    _ArithmeticDestroy,			/* Destroy entry		    */
    _ArithmeticAbort			/* Abort entry			    */
    };

/* Dispatch table to select the correct operation routine */
/* This must match the ordering of the XieK_xxxx constants in XieAppl.h */
static int (*ArithmeticOpTable[])() = {
    NULL,				/* Constants start at 1 */
    ArithmeticAddII,
    ArithmeticAddIC,
    ArithmeticSubII,
    ArithmeticSubIC,
    ArithmeticSubCI,
    ArithmeticMulII,
    ArithmeticMulIC,
    ArithmeticDivII,
    ArithmeticDivIC,
    ArithmeticDivCI,
    ArithmeticMinII,
    ArithmeticMinIC,
    ArithmeticMaxII,
    ArithmeticMaxIC
    };

/*****************************************************************************
**  SmiCreateArithmetic
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function fills in a newly created context for a ARITHMETIC
**      pipeline element.
**
**  FORMAL PARAMETERS:
**
**      pipe    - Descriptor for pipe being built..
**      srcsnk1	- Pointer to first source sink
**      srcsnk2 - Pointer to second source sink (NULL if no second source)
**      constant- Array of "constant image" values (used when srcsnk2==NULL)
**      dstsnk  - Pointer to destination sink
**      idc     - Pointer to UDP describing the ROI or CPP (NULL if no IDC)
**      operator- Arithmetic operation function code.
**      
**
**  FUNCTION VALUE:
**
*****************************************************************************/
int 	    SmiCreateArithmetic(pipe,srcsnk1,srcsnk2,constant,
				     dstsnk, idc, operator)
Pipe		     pipe;
PipeSinkPtr	     srcsnk1;
PipeSinkPtr	     srcsnk2;
unsigned int    constant[XieK_MaxComponents];
PipeSinkPtr	     dstsnk;
UdpPtr		     idc;
int	     operator;

{
    int i;
    ArithmeticPipeCtxPtr ctx = (ArithmeticPipeCtxPtr)
		    DdxCreatePipeCtx_(pipe, &ArithmeticPipeElement, FALSE );
    if( !IsPointer_(ctx) ) return( (int) ctx );
    /*
    **	Copy parameters
    */
    AriSrcSnk1_(ctx) = srcsnk1;
    AriSrcSnk2_(ctx) = srcsnk2;
    AriDstSnk_(ctx)  = dstsnk;
    AriFunc_(ctx)    = ArithmeticOpTable[operator];

    if (srcsnk2 == NULL) {		/* Using a constant "image" */
	for (i = 0; i < XieK_MaxComponents; i++){
	    if (SnkUdpPtr_(srcsnk1,i) == NULL)
		break;
	    AriFConst_(ctx,i) = (float)constant[i];
	}
    }

    AriCpp_(ctx) =  (idc != NULL && idc->UdpA_Base != NULL) ?
	              idc : /* It's a CPP */
		      NULL;

    /* Calculate the bounds of the region that will actually be processed */
    if (srcsnk2 == NULL) {
	AriRoiX1_(ctx) = SnkX1_(srcsnk1,0);
	AriRoiX2_(ctx) = SnkX2_(srcsnk1,0);
	AriRoiY1_(ctx) = SnkY1_(srcsnk1,0);
	AriRoiY2_(ctx) = SnkY2_(srcsnk1,0);
    } else {
	/* Intersection of two source operands */
	AriRoiX1_(ctx) = SnkX1_(srcsnk2,0) > SnkX1_(srcsnk1,0) ?
	                        SnkX1_(srcsnk2,0) : SnkX1_(srcsnk1,0);
	AriRoiX2_(ctx) = SnkX2_(srcsnk2,0) < SnkX2_(srcsnk1,0) ?
	                        SnkX2_(srcsnk2,0) : SnkX2_(srcsnk1,0);
	AriRoiY1_(ctx) = SnkY1_(srcsnk2,0) > SnkY1_(srcsnk1,0) ?
	                        SnkY1_(srcsnk2,0) : SnkY1_(srcsnk1,0);
	AriRoiY2_(ctx) = SnkY2_(srcsnk2,0) < SnkY2_(srcsnk1,0) ?
	                        SnkY2_(srcsnk2,0) : SnkY2_(srcsnk1,0);
    }

    if (idc != NULL) {
	/* Further restrict the area to be processed with the CPP/ROI  */
	AriRoiX1_(ctx) = idc->UdpL_X1 > AriRoiX1_(ctx) ? idc->UdpL_X1
	                                                : AriRoiX1_(ctx);
	AriRoiX2_(ctx) = idc->UdpL_X2 < AriRoiX2_(ctx) ? idc->UdpL_X2
	                                                : AriRoiX2_(ctx);
	AriRoiY1_(ctx) = idc->UdpL_Y1 > AriRoiY1_(ctx) ? idc->UdpL_Y1
	                                                : AriRoiY1_(ctx);
	AriRoiY2_(ctx) = idc->UdpL_Y2 < AriRoiY2_(ctx) ? idc->UdpL_Y2
	                                                : AriRoiY2_(ctx);
    }

    AriSrcData1_(ctx) = NULL;
    AriSrcData2_(ctx) = NULL;

    /* Create drains on the sink(s).
     * Handle all the data as floats.
     * Process a quantum of 1 scanline
     */
    AriSrcDrn1_(ctx) = DdxRmCreateDrain_( AriSrcSnk1_(ctx), -1 );
    if( !IsPointer_(AriSrcDrn1_(ctx)) ) return( (int) AriSrcDrn1_(ctx) );
    DdxRmSetDType_( AriSrcDrn1_(ctx), UdpK_DTypeF, DtM_F );
    DdxRmSetQuantum_( AriSrcDrn1_(ctx), 1 );

    if (srcsnk2 != NULL) {
	AriSrcDrn2_(ctx) = DdxRmCreateDrain_( AriSrcSnk2_(ctx), -1 );
	if( !IsPointer_(AriSrcDrn2_(ctx)) ) return( (int) AriSrcDrn2_(ctx) );
	DdxRmSetDType_( AriSrcDrn2_(ctx), UdpK_DTypeF, DtM_F );
	DdxRmSetQuantum_( AriSrcDrn2_(ctx), 1 );
    }

    DdxRmSetDType_( AriDstSnk_(ctx), UdpK_DTypeF, DtM_F );
    DdxRmSetQuantum_( AriDstSnk_(ctx), 1 );

    return( Success ); 
}					/* end SmiCreateArithmetic */

/*****************************************************************************
**  _ArithmeticInitialize
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function initializes the ARITHMETIC pipeline element prior to
**	activation.
**
**  FORMAL PARAMETERS:
**
**      ctx - arithmetic pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	_ArithmeticInitialize(ctx)
ArithmeticPipeCtxPtr	ctx;
{
    int status  = DdxRmInitializePort_( CtxHead_(ctx), AriSrcDrn1_(ctx) );

    if( status == Success && AriSrcSnk2_(ctx) != NULL )
	status  = DdxRmInitializePort_( CtxHead_(ctx), AriSrcDrn2_(ctx) );

    if( status == Success )
	status  = DdxRmInitializePort_( CtxHead_(ctx), AriDstSnk_(ctx) );

    /* Start reading from the first drain */
    CtxInp_(ctx) = AriSrcDrn1_(ctx);

    return( status ); 
}

/*****************************************************************************
**  _ArithmeticActivate
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine reads the data from the source drain(s) and
**      performs the arithmetic operations.
**
**  FORMAL PARAMETERS:
**
**      ctx - arithmetic pipeline element context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _ArithmeticActivate(ctx)
ArithmeticPipeCtxPtr    ctx;
{

    PipeDataPtr	 src1 = NULL, src2 = NULL, dst = NULL;
    long int  status, dstpos;

#if defined(XIE_USE_EXCEPTION_HANDLING)
    struct sigvec tmpvec, oldvec;


    /*
     * Establish error handling to catch possible division by zero, etc.
     */
    HandleFPE_(tmpvec, ArithmeticException, oldvec);
    if (setjmp(exception_buf) != 0) {

	/* On an exception, clean up locally held resources */
	UnhandleFPE_( oldvec );
	if ( src1 != NULL ) DdxRmDeallocData_(src1);
	if ( src2 != NULL ) DdxRmDeallocData_(src2);
	if ( dst != NULL )  DdxRmDeallocData_(dst);
	return( BadValue );
    }
#endif


    if( AriSrcSnk2_(ctx) == NULL )
	{
	/* Operating between one image and a constant */
	for(status = Success; status == Success; src1 = DdxRmDeallocData_(src1),
						  dst = DdxRmDeallocData_(dst))
	    {
	    src1 = DdxRmGetData_( ctx, AriSrcDrn1_(ctx) );
	    if( !IsPointer_(src1) ) return( (int) src1 );

	    if( DatY1_(src1) < AriRoiY1_(ctx) || DatY1_(src1) > AriRoiY2_(ctx) )
		{
		/* Scanline is outside the ROI - just pass it through */
		status = DdxRmPutData_( AriDstSnk_(ctx), src1 );
		src1   = NULL;
		dst    = NULL;
		continue;
		}
	    dst = DdxRmAllocData_( AriDstSnk_(ctx), DatCmpIdx_(src1),
				   DatY1_(src1), DatHeight_(src1) );
	    status = (int) dst;
	    if( !IsPointer_(dst) ) continue;

	    /* Copy the portions of this line outside the ROI */
	    status = DdxCopy_( DatUdpPtr_(src1),		/* Left  */
			       DatUdpPtr_(dst),
			       DatX1_(src1), 0,
			       DatX1_(dst), DatY1_(dst),
			       AriRoiX1_(ctx) - DatX1_(src1), 1 );
	    if( status != Success ) continue;

	    status = DdxCopy_( DatUdpPtr_(src1),		/* Right */
			       DatUdpPtr_(dst),
			       AriRoiX2_(ctx)+1, 0,
			       AriRoiX2_(ctx)+1, DatY1_(dst),
			       DatX2_(src1) - AriRoiX2_(ctx), 1 );
	    if( status != Success ) continue;
	    
	    dstpos = DatPos_(dst);
	    DatPos_(src1) += (AriRoiX1_(ctx) - DatX1_(src1)) * DatPxlStr_(src1);
	    DatPos_(dst)  += (AriRoiX1_(ctx) - DatX1_(dst))  * DatPxlStr_(dst);

	    status = (*AriFunc_(ctx))(DatUdpPtr_(src1),
				      AriFConst_(ctx,DatCmpIdx_(src1)),
				      DatUdpPtr_(dst),
				      AriCpp_(ctx),
				      AriRoiY1_(ctx),
				      AriRoiX1_(ctx),
				      AriRoiX2_(ctx));
	    if( status != Success ) continue;

	    DatPos_(dst) = dstpos;
	    status = DdxRmPutData_( AriDstSnk_(ctx), dst );
	    dst    = NULL;
	    }
	}

    else
	{
	/* Operating between two images */
	for( status = Success; status == Success;
				    AriSrcData1_(ctx) = DdxRmDeallocData_(src1),
				    AriSrcData2_(ctx) = DdxRmDeallocData_(src2),
						  dst = DdxRmDeallocData_(dst) )
	    {
	    if( AriSrcData1_(ctx) == NULL )  
		{
		AriSrcData1_(ctx) = DdxRmGetData_( ctx, AriSrcDrn1_(ctx) );
		if( !IsPointer_(AriSrcData1_(ctx)) )
		    return( (int) AriSrcData1_(ctx) );
		}
	    src1 = AriSrcData1_(ctx);

	    if( DatY1_(src1) <= AriRoiY2_(ctx) && AriSrcData2_(ctx) == NULL )
		/* Keep the two streams synchronized as long as we still
		 * need src2 data */
		{
		AriSrcData2_(ctx) = DdxRmGetData_( ctx, AriSrcDrn2_(ctx) );
		if( !IsPointer_(AriSrcData2_(ctx)) )
		    return( (int) AriSrcData2_(ctx) );
		}
	    src2 = AriSrcData2_(ctx);

	    if( DatY1_(src1) < AriRoiY1_(ctx) || DatY1_(src1) > AriRoiY2_(ctx) )
		{
		/* Scanline is outside the ROI - just pass it through */
		status = DdxRmPutData_( AriDstSnk_(ctx), src1 );
		src1   = NULL;
		dst    = NULL;
		continue;
		}
	    dst = DdxRmAllocData_( AriDstSnk_(ctx), DatCmpIdx_(src1),
				   DatY1_(src1), DatHeight_(src1) );
	    status = (int) dst;
	    if( !IsPointer_(dst) ) continue;
	
	    /* Copy the portions of this line outside the ROI */
	    status = DdxCopy_( DatUdpPtr_(src1),		/* Left  */
			       DatUdpPtr_(dst),
			       DatX1_(src1), 0,
			       DatX1_(dst), DatY1_(dst),
			       AriRoiX1_(ctx) - DatX1_(src1), 1 );
	    if( status != Success ) continue;
	    
	    status = DdxCopy_( DatUdpPtr_(src1),		/* Right */
			       DatUdpPtr_(dst),
			       AriRoiX2_(ctx)+1, 0,
			       AriRoiX2_(ctx)+1, DatY1_(dst),
			       DatX2_(src1) - AriRoiX2_(ctx), 1 );
	    if( status != Success ) continue;
	
	    dstpos = DatPos_(dst);
	    DatPos_(src1) += (AriRoiX1_(ctx) - DatX1_(src1)) * DatPxlStr_(src1);
	    DatPos_(src2) += (AriRoiX1_(ctx) - DatX1_(src2)) * DatPxlStr_(src2);

	    status = (*AriFunc_(ctx))(DatUdpPtr_(src1), DatUdpPtr_(src2),
				      DatUdpPtr_(dst),
				      AriCpp_(ctx),
				      AriRoiY1_(ctx),
				      AriRoiX1_(ctx),
				      AriRoiX2_(ctx));
	    if( status != Success ) continue;

	    DatPos_(dst) = dstpos;
	    status = DdxRmPutData_( AriDstSnk_(ctx), dst );
	    dst = NULL;
	    }
	}

#if defined(XIE_USE_EXCEPTION_HANDLING)
    UnhandleFPE_( oldvec );
#endif

    return( status ); 
}

/*****************************************************************************
**  _ArithmeticFlush
**
**  FUNCTIONAL DESCRIPTION:
**
**  	This routine is called when all preceding pipeline stages have
**      completed. 
**
**
**  FORMAL PARAMETERS:
**
**      ctx - arithmetic pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _ArithmeticFlush(ctx)
ArithmeticPipeCtxPtr    ctx;
{
    PipeDataPtr src2;

    _ArithmeticAbort(ctx);

    /* If the image on drain 2 has more scanlines that the image on drain 1
     * we will have unprocessed scanlines on drain 2.  Flush them.
     */
    if( IsPointer_(AriSrcSnk2_(ctx)) )
	while( ( src2 = DdxRmGetData_( ctx, AriSrcDrn2_(ctx) ) ) != NULL )
	    {
	    if( !IsPointer_(src2) ) return( (int) src2 );
	    DdxRmDeallocData_(src2);
	    }

    return( Success ); 
}

/*****************************************************************************
**  _ArithmeticAbort
**
**  FUNCTIONAL DESCRIPTION:
**
**  	Called when pipeline is aborted or finished.
**
**  FORMAL PARAMETERS:
**
**      ctx - arithmetic pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _ArithmeticAbort(ctx)
ArithmeticPipeCtxPtr    ctx;
{
    if( IsPointer_(AriSrcData1_(ctx)) )
	AriSrcData1_(ctx) = DdxRmDeallocData_( AriSrcData1_(ctx) );
    if( IsPointer_(AriSrcData2_(ctx)) )
	AriSrcData2_(ctx) = DdxRmDeallocData_( AriSrcData2_(ctx) );

    return( Success ); 
}

/*****************************************************************************
**  _ArithmeticDestroy
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine deallocates all resources allocated by the ARITHMETIC
**	pipeline element.
**
**  FORMAL PARAMETERS:
**
**      ctx - arithmetic pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _ArithmeticDestroy(ctx)
ArithmeticPipeCtxPtr    ctx;
{
    AriSrcDrn1_(ctx) = DdxRmDestroyDrain_( AriSrcDrn1_(ctx) );

    if( AriSrcSnk2_(ctx) != NULL )
	AriSrcDrn2_(ctx) = DdxRmDestroyDrain_( AriSrcDrn2_(ctx) );

    return( Success ); 
}

/*****************************************************************************
**  SmiArithmetic
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine implements a non-pipelined arithmetic operator, which
**      performs an arithmetic operation between two photomaps, or between
**      a photomap and a constant.
** 
**      Note that the destination photomap must be of type float or
**      the data will be transformed from unconstrained to a constrained
**      type in an undefined manner.
**
**  FORMAL PARAMETERS:
**
**	srcudp1 - Pointer to a UDP describing the first source operand.
**	srcudp2 - Pointer to a UDP describing the second source operand.
**      constant- Array of constants supplied if srcudp2 is NULL.
**      dstudp  - Pointer to a UDP describing the destination buffer.
**      idc     - Pointer to a UDP describing the Region of Interest,
**                or Control Processing Plane.
**                (NULL if no IDC)
**      operator-Code for the operation being performed.
**
**  FUNCTION VALUE:
**
*****************************************************************************/
int 	SmiArithmetic( srcudp1, srcudp2, constant, dstudp, idc, operator )
UdpPtr			srcudp1;
UdpPtr		  	srcudp2;
unsigned int	constant[];
UdpPtr			dstudp;
UdpPtr	 		idc;
int		operator;
{
    UdpPtr   cpp, src1 = NULL, src2 = NULL, dst = NULL;
    float fconstant;
    long int pixels, scanline, status;
    long int X1, X2, Y1, Y2;		/* src/destination common bounds     */
    long int x1, x2, y1, y2;		/* src/destination/ROI common bounds */

#if defined(XIE_USE_EXCEPTION_HANDLING)
    struct sigvec tmpvec, oldvec;

    /*
    **	Establish error handling to catch possible division by zero, etc.
    */
    HandleFPE_( tmpvec, ArithmeticException, oldvec );
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
     * Determine maximum common dimensions between source1 and destination,
     * which is the defined region of output.
     */
    X1 = srcudp1->UdpL_X1 > dstudp->UdpL_X1 ? srcudp1->UdpL_X1
	                                    : dstudp->UdpL_X1;
    X2 = srcudp1->UdpL_X2 < dstudp->UdpL_X2 ? srcudp1->UdpL_X2
	                                    : dstudp->UdpL_X2;
    Y1 = srcudp1->UdpL_Y1 > dstudp->UdpL_Y1 ? srcudp1->UdpL_Y1
	                                    : dstudp->UdpL_Y1;
    Y2 = srcudp1->UdpL_Y2 < dstudp->UdpL_Y2 ? srcudp1->UdpL_Y2
					    : dstudp->UdpL_Y2;

    /*
     * Restrict the area to be processed by finding the intersection of
     * the region of output and the second source operand (if used).
     */
    if (srcudp2 == NULL) {
	x1 = X1; x2 = X2;
	y1 = Y1; y2 = Y2;
    } else {
	x1 = srcudp2->UdpL_X1 > X1 ? srcudp2->UdpL_X1 : X1;
	x2 = srcudp2->UdpL_X2 < X2 ? srcudp2->UdpL_X2 : X2;
	y1 = srcudp2->UdpL_Y1 > Y1 ? srcudp2->UdpL_Y1 : Y1;
	y2 = srcudp2->UdpL_Y2 < Y2 ? srcudp2->UdpL_Y2 : Y2;
    }
	
    /* 
     * Further restrict the area to be processed by finding the intersection
     * with the IDC (ROI or CPP).
     */
    if (idc != NULL) {
	x1 = idc->UdpL_X1 > x1 ? idc->UdpL_X1 : x1;
	x2 = idc->UdpL_X2 < x2 ? idc->UdpL_X2 : x2;
	y1 = idc->UdpL_Y1 > y1 ? idc->UdpL_Y1 : y1;
	y2 = idc->UdpL_Y2 < y2 ? idc->UdpL_Y2 : y2;
    }
	
    /* Allocate and init temporary UDP's to hold one scanline of float data */
    status = BadAlloc;
    src1 = ArithmeticGetTempUdp(srcudp1, y1, x1, x2);
    if( !IsPointer_(src1) ) goto clean_up;

    if( srcudp2 != NULL )
	{
	src2 = ArithmeticGetTempUdp(srcudp2, y1, x1, x2);
	if( !IsPointer_(src2) ) goto clean_up;
	}
    dst = ArithmeticGetTempUdp(dstudp, y1, x1, x2);
    if( !IsPointer_(dst) ) goto clean_up;

    fconstant = (float)constant[srcudp1->UdpL_CompIdx];
    cpp = (idc != NULL && idc->UdpA_Base != NULL) ? idc : NULL;

    /* Just copy the scanlines outside of the ROI */
    status = DdxCopy_( srcudp1, dstudp,		/* Top */
		       X1, Y1, X1, Y1, X2 - X1 + 1, y1 - Y1 );
    if( status != Success ) goto clean_up;

    status = DdxCopy_( srcudp1, dstudp,		/* Bottom */
		       X1, y2 + 1, X1, y2 + 1, X2 - X1 + 1, Y2 - y2 );
    if( status != Success ) goto clean_up;

    status = DdxCopy_( srcudp1, dstudp,		/* Left */
		       X1, y1, X1, y1, x1 - X1, y2 - y1 + 1 );
    if( status != Success ) goto clean_up;
    
    status = DdxCopy_( srcudp1, dstudp,		/* Right */
		       x2 + 1, y1, x2 + 1, y1, X2 - x2, y2 - y1 + 1 );
    if( status != Success ) goto clean_up;

    /* If the temporary UDP's are pointing to the original data, position
     * to the correct offset.
     */
    if (src1->UdpA_Base == srcudp1->UdpA_Base)
	src1->UdpL_Pos = srcudp1->UdpL_Pos + y1 * srcudp1->UdpL_ScnStride +
	    (x1 - srcudp1->UdpL_X1) * srcudp1->UdpL_PxlStride;

    if (srcudp2 != NULL && src2->UdpA_Base == srcudp2->UdpA_Base)
	src2->UdpL_Pos = srcudp2->UdpL_Pos + y1 * srcudp2->UdpL_ScnStride +
	    (x1 - srcudp2->UdpL_X1) * srcudp2->UdpL_PxlStride;

    dst->UdpL_Y1 = dst->UdpL_Y2 = y1;
    if (dst->UdpA_Base == dstudp->UdpA_Base)
	dst->UdpL_Pos = dstudp->UdpL_Pos + y1 * dstudp->UdpL_ScnStride +
	    (x1 - dstudp->UdpL_X1) * dstudp->UdpL_PxlStride;

    /* Now perform the arithmetic operation over the region-of-interest */

    if (srcudp2 != NULL )
	{
	/* Image-to-image operation */
	for( scanline = y1; scanline <= y2; scanline++)
	    {
	    /* Get a UDP pointing to this scanline */
	    status = ArithmeticConvertScanline(srcudp1, src1);
	    if( status != Success ) goto clean_up;
	    status = ArithmeticConvertScanline(srcudp2, src2);
	    if( status != Success ) goto clean_up;
	    status = (*ArithmeticOpTable[operator])(src1, src2, dst, cpp,
						    y1, x1, x2);
	    if( status != Success ) goto clean_up;
	    status = ArithmeticStoreScanline(dst, dstudp);
	    if( status != Success ) goto clean_up;
	    }
	}
    else
	{
	/* Image-to-constant or Constant-to-Image operation */
	for( scanline = y1; scanline <= y2; scanline++)
	    {
	    /* Get a UDP pointing to this scanline */
	    status = ArithmeticConvertScanline(srcudp1, src1);
	    if( status != Success ) goto clean_up;
	    
	    status = (*ArithmeticOpTable[operator])(src1, fconstant, dst, cpp, 
						    y1, x1, x2);
	    if( status != Success ) goto clean_up;
	    status = ArithmeticStoreScanline(dst, dstudp);
	    if( status != Success ) goto clean_up;
	    }
	}
    /* Disable error handling and cleanup the temporary descriptors */
clean_up :

#if defined(XIE_USE_EXCEPTION_HANDLING)
    UnhandleFPE_( oldvec );
#endif


    if( IsPointer_(src1) )
	ArithmeticDestroyTempUdp(src1, srcudp1);
    if( IsPointer_(src2) )
	ArithmeticDestroyTempUdp(src2, srcudp2);
    if( IsPointer_(dst) )
	ArithmeticDestroyTempUdp(dst, dstudp);

    return( status ); 
}					/* end SmiArithmetic */

/*****************************************************************************
**  Arithmetic processing routines
**
**  FUNCTIONAL DESCRIPTION:
**
**	These routines the actual arithmetic operation over selected
**      portion of a scanline.  A separate routine is used for each
**      type of operation.
**
**  FORMAL PARAMETERS:
**
**	src1    - Pointer to a UDP describing the first source operand.
**	src2    - Pointer to a UDP describing the second source operand.
**      fconst  - Constant "image" value if image-to-constant.
**      dst     - Pointer to a UDP describing the destination buffer.
**      cpp     - Pointer to a UDP describing the Control Processing Plane.
**                (NULL if no CPP)
**      y1      - The y corrdinate of the beginning of the region to
*                 process.
**      x1      - The x coordinate of the left endpoint of the region to
**                process.
**      x2      - The x coordinate of the right endpoint of the region to
**                process.
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int ArithmeticAddII(src1, src2, dst, cpp,  y1, x1, x2)
UdpPtr	     	src1;
UdpPtr	     	src2;
UdpPtr	     	dst;
UdpPtr		cpp;
long int        y1;
long int	x1;
long int	x2;
{
    int			pixel;

    unsigned char 	*src1ptr;
    unsigned char 	*src2ptr;
    unsigned char       *dstptr;
    long int		cpppxloff;
    int			cppbit;

    int			src1bytes = src1->UdpL_PxlStride >> 3;
    int                 src2bytes = src2->UdpL_PxlStride >> 3;
    int                 dstbytes  = dst->UdpL_PxlStride >> 3;
    

    src1ptr = src1->UdpA_Base + (src1->UdpL_Pos >> 3);
    src2ptr = src2->UdpA_Base + (src2->UdpL_Pos >> 3);
    dstptr = dst->UdpA_Base + (dst->UdpL_Pos >> 3);

    if (cpp != NULL)
	cpppxloff = cpp->UdpL_Pos +
	    cpp->UdpL_ScnStride * (src1->UdpL_Y1 - y1);
    else
	cppbit = 1;			/* Fake all 1's in CPP */


    for (pixel = x1; pixel <= x2; pixel++){
	
	if (cpp != NULL) {
	    cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
	    cpppxloff+=cpp->UdpL_PxlStride;
	}	  

	if (cppbit)
	    *(float *)dstptr = *(float *)src1ptr + *(float *)src2ptr;
	else
	    *(float *)dstptr = *(float *)src1ptr; /* Not selected by CPP */

	src1ptr+=src1bytes;
	src2ptr+=src2bytes;
	dstptr+=dstbytes;

    }
    return( Success );
}					/* end ArithmeticAddII */

static int   ArithmeticSubII(src1, src2, dst, cpp,  y1, x1, x2)
UdpPtr	     	src1;
UdpPtr	     	src2;
UdpPtr	     	dst;
UdpPtr		cpp;
long int        y1;
long int	x1;
long int	x2;
{
    int			pixel;

    unsigned char 	*src1ptr;
    unsigned char 	*src2ptr;
    unsigned char       *dstptr;
    long int		cpppxloff;
    int			cppbit;

    int			src1bytes = src1->UdpL_PxlStride >> 3;
    int                 src2bytes = src2->UdpL_PxlStride >> 3;
    int                 dstbytes  = dst->UdpL_PxlStride >> 3;
    

    src1ptr = src1->UdpA_Base + (src1->UdpL_Pos >> 3);
    src2ptr = src2->UdpA_Base + (src2->UdpL_Pos >> 3);
    dstptr = dst->UdpA_Base + (dst->UdpL_Pos >> 3);

    if (cpp != NULL)
	cpppxloff = cpp->UdpL_Pos +
	    cpp->UdpL_ScnStride * (src1->UdpL_Y1 - y1);
    else
	cppbit = 1;			/* Fake all 1's in CPP */


    for (pixel = x1; pixel <= x2; pixel++){
	
	if (cpp != NULL) {
	    cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
	    cpppxloff+=cpp->UdpL_PxlStride;
	}	  
	
	if (cppbit)
	    *(float *)dstptr = *(float *)src1ptr - *(float *)src2ptr;
	else
	    *(float *)dstptr = *(float *)src1ptr; /* Not selected by CPP */
	
	src1ptr+=src1bytes;
	src2ptr+=src2bytes;
	dstptr+=dstbytes;

    }
    return( Success );
}					/* end ArithmeticSubII */

static int   ArithmeticMulII(src1, src2, dst, cpp,  y1, x1, x2)
UdpPtr	     	src1;
UdpPtr	     	src2;
UdpPtr	     	dst;
UdpPtr		cpp;
long int        y1;
long int	x1;
long int	x2;
{
    int			pixel;

    unsigned char 	*src1ptr;
    unsigned char 	*src2ptr;
    unsigned char       *dstptr;
    long int		cpppxloff;
    int			cppbit;

    int			src1bytes = src1->UdpL_PxlStride >> 3;
    int                 src2bytes = src2->UdpL_PxlStride >> 3;
    int                 dstbytes  = dst->UdpL_PxlStride >> 3;
    

    src1ptr = src1->UdpA_Base + (src1->UdpL_Pos >> 3);
    src2ptr = src2->UdpA_Base + (src2->UdpL_Pos >> 3);
    dstptr = dst->UdpA_Base + (dst->UdpL_Pos >> 3);

    if (cpp != NULL)
	cpppxloff = cpp->UdpL_Pos +
	    cpp->UdpL_ScnStride * (src1->UdpL_Y1 - y1);
    else
	cppbit = 1;			/* Fake all 1's in CPP */


    for (pixel = x1; pixel <= x2; pixel++){
	
	if (cpp != NULL) {
	    cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
	    cpppxloff+=cpp->UdpL_PxlStride;
	}	  
	
	if (cppbit)
	    *(float *)dstptr = *(float *)src1ptr * *(float *)src2ptr;
	else
	    *(float *)dstptr = *(float *)src1ptr; /* Not selected by CPP */
	
	src1ptr+=src1bytes;
	src2ptr+=src2bytes;
	dstptr+=dstbytes;

    }
    return( Success );
}					/* end ArithmeticMulII */

static int   ArithmeticDivII(src1, src2, dst, cpp,  y1, x1, x2)
UdpPtr	     	src1;
UdpPtr	     	src2;
UdpPtr	     	dst;
UdpPtr		cpp;
long int        y1;
long int	x1;
long int	x2;
{
    int			pixel;

    unsigned char 	*src1ptr;
    unsigned char 	*src2ptr;
    unsigned char       *dstptr;
    long int		cpppxloff;
    int			cppbit;

    int			src1bytes = src1->UdpL_PxlStride >> 3;
    int                 src2bytes = src2->UdpL_PxlStride >> 3;
    int                 dstbytes  = dst->UdpL_PxlStride >> 3;
    

    src1ptr = src1->UdpA_Base + (src1->UdpL_Pos >> 3);
    src2ptr = src2->UdpA_Base + (src2->UdpL_Pos >> 3);
    dstptr = dst->UdpA_Base + (dst->UdpL_Pos >> 3);

    if (cpp != NULL)
	cpppxloff = cpp->UdpL_Pos +
	    cpp->UdpL_ScnStride * (src1->UdpL_Y1 - y1);
    else
	cppbit = 1;			/* Fake all 1's in CPP */


    for (pixel = x1; pixel <= x2; pixel++){
	
	if (cpp != NULL) {
	    cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
	    cpppxloff+=cpp->UdpL_PxlStride;
	}	  
	
	if (cppbit) {
#if !defined(XIE_USE_EXCEPTION_HANDLING)
	    if ( *(float *)src2ptr == 0.0 ) return( BadValue );
#endif
	    *(float *)dstptr = *(float *)src1ptr / *(float *)src2ptr;
	    }
	else
	    *(float *)dstptr = *(float *)src1ptr; /* Not selected by CPP */
	
	
	src1ptr+=src1bytes;
	src2ptr+=src2bytes;
	dstptr+=dstbytes;

    }
    return( Success );
}					/* end ArithmeticDivII */

static int   ArithmeticMinII(src1, src2, dst, cpp,  y1, x1, x2)
UdpPtr	     	src1;
UdpPtr	     	src2;
UdpPtr	     	dst;
UdpPtr		cpp;
long int        y1;
long int	x1;
long int	x2;
{
    int			pixel;

    unsigned char 	*src1ptr;
    unsigned char 	*src2ptr;
    unsigned char       *dstptr;
    long int		cpppxloff;
    int			cppbit;

    int			src1bytes = src1->UdpL_PxlStride >> 3;
    int                 src2bytes = src2->UdpL_PxlStride >> 3;
    int                 dstbytes  = dst->UdpL_PxlStride >> 3;
    

    src1ptr = src1->UdpA_Base + (src1->UdpL_Pos >> 3);
    src2ptr = src2->UdpA_Base + (src2->UdpL_Pos >> 3);
    dstptr = dst->UdpA_Base + (dst->UdpL_Pos >> 3);

    if (cpp != NULL)
	cpppxloff = cpp->UdpL_Pos +
	    cpp->UdpL_ScnStride * (src1->UdpL_Y1 - y1);
    else
	cppbit = 1;			/* Fake all 1's in CPP */


    for (pixel = x1; pixel <= x2; pixel++){
	
	if (cpp != NULL) {
	    cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
	    cpppxloff+=cpp->UdpL_PxlStride;
	}	  
	
	if (cppbit)
	    *(float *)dstptr = Min_(*(float *)src1ptr, *(float *)src2ptr);
	else
	    *(float *)dstptr = *(float *)src1ptr; /* Not selected by CPP */
	
	src1ptr+=src1bytes;
	src2ptr+=src2bytes;
	dstptr+=dstbytes;

    }
    return( Success );
}					/* end ArithmeticMinII */

static int   ArithmeticMaxII(src1, src2, dst, cpp,  y1, x1, x2)
UdpPtr	     	src1;
UdpPtr	     	src2;
UdpPtr	     	dst;
UdpPtr		cpp;
long int        y1;
long int	x1;
long int	x2;
{
    int			pixel;

    unsigned char 	*src1ptr;
    unsigned char 	*src2ptr;
    unsigned char       *dstptr;
    long int		cpppxloff;
    int			cppbit;

    int			src1bytes = src1->UdpL_PxlStride >> 3;
    int                 src2bytes = src2->UdpL_PxlStride >> 3;
    int                 dstbytes  = dst->UdpL_PxlStride >> 3;
    

    src1ptr = src1->UdpA_Base + (src1->UdpL_Pos >> 3);
    src2ptr = src2->UdpA_Base + (src2->UdpL_Pos >> 3);
    dstptr = dst->UdpA_Base + (dst->UdpL_Pos >> 3);

    if (cpp != NULL)
	cpppxloff = cpp->UdpL_Pos +
	    cpp->UdpL_ScnStride * (src1->UdpL_Y1 - y1);
    else
	cppbit = 1;			/* Fake all 1's in CPP */


    for (pixel = x1; pixel <= x2; pixel++){
	
	if (cpp != NULL) {
	    cppbit = GET_VALUE_(cpp->UdpA_Base, cpppxloff, 1);
	    cpppxloff+=cpp->UdpL_PxlStride;
	}	  
	
	if (cppbit)
	    *(float *)dstptr = Max_(*(float *)src1ptr, *(float *)src2ptr);
	else
	    *(float *)dstptr = *(float *)src1ptr; /* Not selected by CPP */
	
	
	src1ptr+=src1bytes;
	src2ptr+=src2bytes;
	dstptr+=dstbytes;
	
    }
    return( Success );
}					/* end ArithmeticMaxII */

static int   ArithmeticAddIC(src, fconst, dst, cpp,  y1, x1, x2)
    UdpPtr	src;
    float	fconst;
    UdpPtr	dst;
    UdpPtr	cpp;
    long int    y1;
    long int	x1;
    long int	x2;
{
    int			pixel;

    unsigned char 	*srcptr;
    unsigned char       *dstptr;
    long int		cpppxloff;
    int			cppbit;

    int			srcbytes = src->UdpL_PxlStride >> 3;
    int                 dstbytes = dst->UdpL_PxlStride >> 3;
    

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
	    *(float *)dstptr = *(float *)srcptr + fconst;
	else
	    *(float *)dstptr = *(float *)srcptr; /* Not selected by CPP */
	
	srcptr+=srcbytes;
	dstptr+=dstbytes;

    }
    return( Success );
}					/* end ArithmeticAddIC */

static int   ArithmeticSubIC(src, fconst, dst, cpp,  y1, x1, x2)
    UdpPtr	src;
    float	fconst;
    UdpPtr	dst;
    UdpPtr	cpp;
    long int    y1;
    long int	x1;
    long int	x2;
{
    int			pixel;

    unsigned char 	*srcptr;
    unsigned char       *dstptr;
    long int		cpppxloff;
    int			cppbit;

    int			srcbytes = src->UdpL_PxlStride >> 3;
    int                 dstbytes = dst->UdpL_PxlStride >> 3;
    

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
	    *(float *)dstptr = *(float *)srcptr - fconst;
	else
	    *(float *)dstptr = *(float *)srcptr; /* Not selected by CPP */
	
	srcptr+=srcbytes;
	dstptr+=dstbytes;

    }
    return( Success );
}					/* end ArithmeticSubIC */

static int   ArithmeticMulIC(src, fconst, dst, cpp,  y1, x1, x2)
    UdpPtr	src;
    float	fconst;
    UdpPtr	dst;
    UdpPtr	cpp;
    long int    y1;
    long int	x1;
    long int	x2;
{
    int			pixel;

    unsigned char 	*srcptr;
    unsigned char       *dstptr;
    long int		cpppxloff;
    int			cppbit;

    int			srcbytes = src->UdpL_PxlStride >> 3;
    int                 dstbytes = dst->UdpL_PxlStride >> 3;
    

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
	    *(float *)dstptr = *(float *)srcptr * fconst;
	else
	    *(float *)dstptr = *(float *)srcptr; /* Not selected by CPP */
	
	srcptr+=srcbytes;
	dstptr+=dstbytes;

    }
    return( Success );
}					/* end ArithmeticMulIC */

static int   ArithmeticDivIC(src, fconst, dst, cpp,  y1, x1, x2)
    UdpPtr	src;
    float	fconst;
    UdpPtr	dst;
    UdpPtr	cpp;
    long int    y1;
    long int	x1;
    long int	x2;
{
    int			pixel;

    unsigned char 	*srcptr;
    unsigned char       *dstptr;
    long int		cpppxloff;
    int			cppbit;

    int			srcbytes = src->UdpL_PxlStride >> 3;
    int                 dstbytes = dst->UdpL_PxlStride >> 3;
    
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
	    if (fconst == 0.0)
		return( BadValue );
#endif
	    *(float *)dstptr = *(float *)srcptr / fconst;
	} else
	    *(float *)dstptr = *(float *)srcptr; /* Not selected by CPP */
	
	srcptr+=srcbytes;
	dstptr+=dstbytes;

    }
    return( Success );
}					/* end ArithmeticDivIC */

static int   ArithmeticMinIC(src, fconst, dst, cpp,  y1, x1, x2)
    UdpPtr	src;
    float	fconst;
    UdpPtr	dst;
    UdpPtr	cpp;
    long int    y1;
    long int	x1;
    long int	x2;
{
    int			pixel;

    unsigned char 	*srcptr;
    unsigned char       *dstptr;
    long int		cpppxloff;
    int			cppbit;

    int			srcbytes = src->UdpL_PxlStride >> 3;
    int                 dstbytes = dst->UdpL_PxlStride >> 3;
    

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
	    *(float *)dstptr = Min_(*(float *)srcptr, fconst);
	else
	    *(float *)dstptr = *(float *)srcptr; /* Not selected by CPP */
	
	srcptr+=srcbytes;
	dstptr+=dstbytes;

    }
    return( Success );
}					/* end ArithmeticMinIC */

static int   ArithmeticMaxIC(src, fconst, dst, cpp,  y1, x1, x2)
    UdpPtr	src;
    float	fconst;
    UdpPtr	dst;
    UdpPtr	cpp;
    long int    y1;
    long int	x1;
    long int	x2;
{
    int			pixel;

    unsigned char 	*srcptr;
    unsigned char       *dstptr;
    long int		cpppxloff;
    int			cppbit;

    int			srcbytes = src->UdpL_PxlStride >> 3;
    int                 dstbytes = dst->UdpL_PxlStride >> 3;
    

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
	    *(float *)dstptr = Max_(*(float *)srcptr, fconst);
	else
	    *(float *)dstptr = *(float *)srcptr; /* Not selected by CPP */
	
	srcptr+=srcbytes;
	dstptr+=dstbytes;

    }
    return( Success );
}					/* end ArithmeticMaxIC */

static int   ArithmeticSubCI(src, fconst, dst, cpp,  y1, x1, x2)
    UdpPtr	src;
    float	fconst;
    UdpPtr	dst;
    UdpPtr	cpp;
    long int    y1;
    long int	x1;
    long int	x2;
{
    int			pixel;

    unsigned char 	*srcptr;
    unsigned char       *dstptr;
    long int		cpppxloff;
    int			cppbit;

    int			srcbytes = src->UdpL_PxlStride >> 3;
    int                 dstbytes = dst->UdpL_PxlStride >> 3;
    

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
	    *(float *)dstptr = fconst - *(float *)srcptr;
	else
	    *(float *)dstptr = *(float *)srcptr; /* Not selected by CPP */
	
	srcptr+=srcbytes;
	dstptr+=dstbytes;

    }
    return( Success );
}					/* end ArithmeticSubCI */

static int ArithmeticDivCI(src, fconst, dst, cpp,  y1, x1, x2)
    UdpPtr	src;
    float	fconst;
    UdpPtr	dst;
    UdpPtr	cpp;
    long int    y1;
    long int	x1;
    long int	x2;
{
    int			pixel;

    unsigned char 	*srcptr;
    unsigned char       *dstptr;
    long int		cpppxloff;
    int			cppbit;

    int			srcbytes = src->UdpL_PxlStride >> 3;
    int                 dstbytes = dst->UdpL_PxlStride >> 3;
    

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
	if ( *(float *)srcptr == 0.0 )
	    return( BadValue );
#endif
	    *(float *)dstptr = fconst / *(float *)srcptr;

	} else
	    *(float *)dstptr = *(float *)srcptr; /* Not selected by CPP */
	
	srcptr+=srcbytes;
	dstptr+=dstbytes;
	
    }
    return( Success );
}					/* end ArithmeticDivCI */

/*****************************************************************************
**  ArithmeticException
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
static void ArithmeticException( )
{
#if defined(XIE_USE_EXCEPTION_HANDLING)
    longjmp( exception_buf, 1 );
#endif
}

/*****************************************************************************
**  ArithmeticGetTempUdp
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
static UdpPtr ArithmeticGetTempUdp(src, y1, x1, x2)
    UdpPtr src;
    long int y1;
    long int x1, x2;
{
    UdpPtr dst;

    dst = (UdpPtr) DdxMalloc_(sizeof(UdpRec));
    if( dst == NULL ) return( (UdpPtr) BadAlloc );

    if (src->UdpB_DType != UdpK_DTypeF)
	{
	dst->UdpW_PixelLength = sizeof(float) * 8;
	dst->UdpB_DType	      = UdpK_DTypeF;
	dst->UdpB_Class       = UdpK_ClassA;
	dst->UdpL_PxlStride   = dst->UdpW_PixelLength;
	dst->UdpL_PxlPerScn   = x2 - x1 + 1;
	dst->UdpL_ScnStride   = dst->UdpL_PxlPerScn * dst->UdpL_PxlStride;
	dst->UdpL_ArSize      = dst->UdpL_PxlPerScn * sizeof(float) << 3;
	dst->UdpA_Base	      = DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL )
	    return( DdxFree_(dst), (UdpPtr) BadAlloc );
	dst->UdpL_Pos	      = 0;
	dst->UdpL_CompIdx     = src->UdpL_CompIdx;
	dst->UdpL_Levels      = src->UdpL_Levels;
	}
    else 
	{
	*dst = *src;
	dst->UdpL_PxlPerScn = x2 - x1 + 1;
	dst->UdpL_ScnStride = dst->UdpL_PxlPerScn * dst->UdpL_PxlStride;
	}
    dst->UdpL_X1	  = x1;
    dst->UdpL_X2	  = x2;
    dst->UdpL_Y1	  = y1;
    dst->UdpL_Y2	  = y1; /* One scanline UDP */
    dst->UdpL_ScnCnt	  = 1;

    return( dst );
}					/* end ArithmeticGetTempUdp */

/*****************************************************************************
**  ArithmeticDestroyTempUdp
**
**  FUNCTIONAL DESCRIPTION:
**
**	Free a UDP created by ArithmeticGetTempUdp.
**
**  FORMAL PARAMETERS:
**
**	udp     - Pointer to the udp to free.
**      realudp - Pointer to a udp which might actually own the storage
**                pointed to by udp.
**
*****************************************************************************/
static void ArithmeticDestroyTempUdp( udp, realudp )
UdpPtr udp;
UdpPtr realudp;

{
    if (udp->UdpA_Base != NULL && udp->UdpA_Base != realudp->UdpA_Base)
	DdxFreeBits_( udp->UdpA_Base );
    DdxFree_( udp );
}					/* end ArithmeticDestroyTempUdp */

/*****************************************************************************
**  ArithmeticConvertScanline
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
static int ArithmeticConvertScanline(src,  dst)
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
}					/* end ArithmeticConvertScanline */

/*****************************************************************************
**  ArithmeticStoreScanline
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
static int ArithmeticStoreScanline(dst, realdst)
    UdpPtr dst;
    UdpPtr realdst;
{
    int status = Success;

    if (dst->UdpA_Base != realdst->UdpA_Base) {
	status = DdxConvert_( dst, realdst, XieK_MoveMode );
    } else {
	dst->UdpL_Pos +=  realdst->UdpL_ScnStride;
    }
    dst->UdpL_Y1++;
    dst->UdpL_Y2++;

    return( status );
}					/* end ArithmeticStoreScanline */
