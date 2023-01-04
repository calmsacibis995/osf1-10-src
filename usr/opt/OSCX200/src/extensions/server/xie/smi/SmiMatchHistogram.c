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
**      X Image Extension
**	Sample Machine Independant DDX
**
**  ABSTRACT:
**
**      This module contains sample code which remaps an image such that
**      the resulting image has the specified distribution of pixel
**      values.
**
**  ENVIRONMENT:
**
**	VAX/VMS V5.3
**	ULTRIX  V3.1
**
**  AUTHOR(S):
**
**      Gary L. Grebus
**      Richard Piccolo
**
**  CREATION DATE:
**
**      Thu Jun 28 16:35:52 1990
**
**  MODIFICATION HISTORY:
**
*******************************************************************************/

/*
**  Include files 
*/
#include <stdio.h>
#include <math.h>                           /* C math definitions           */

#include "SmiMatchHistogram.h"
/*
**  Table of contents
*/
int	SmiMatchHistogram();
int     SmiCreateMatchHistogram();

static int  _MatchHistogramInitialize();
static int  _MatchHistogramActivate();
static int  _MatchHistogramDestroy();

static UdpPtr	_ChkUdp();
static int	_CreateMHLut();
static UdpPtr	_FreeUdp();
static int	_GenerateCdf();
/*
**  Equated Symbols
*/
# define K1                 .39894228               /* 1 / sqrt (2pi) */
/*
**  MACRO definitions
*/
/*
**  External References
*/
/*
 * Local Storage
 */
    /*
    ** MatchHistogram Pipeline Element Vector
    */
static PipeElementVector MatchHistogramPipeElement = {
    NULL,				/* FLINK			    */
    NULL,				/* BLINK			    */
    sizeof(PipeElementVector),		/* Size				    */
    TypePipeElementVector,		/* Structure type		    */
    StypeMatchHistogramElement,		/* Structure subtype		    */
    sizeof(MatchHistogramPipeCtx),	/* Context block size		    */
    0,					/* No input flag		    */
    0,					/* Reserved flags		    */
    _MatchHistogramInitialize,		/* Initialize entry		    */
    _MatchHistogramActivate,		/* Activate entry		    */
    NULL,				/* Flush entry			    */
    _MatchHistogramDestroy,		/* Destroy entry		    */
    NULL				/* Abort entry			    */
    };

/*****************************************************************************
**  SmiCreateMatchHistogram
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function fills in a newly created MATCHHISTOGRAM pipeline element.
**
**  FORMAL PARAMETERS:
**
**      ctx - MatchHistogram pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
int 	SmiCreateMatchHistogram( pipe, src, dst, idc, hshape, param1, param2 )
Pipe		     pipe;
PipeSinkPtr	     src;
PipeSinkPtr	     dst;
UdpPtr               idc;
int		     hshape;		/* Distribution shape and parameters */
float                param1;
float                param2;
{
    MatchHistogramPipeCtxPtr ctx = (MatchHistogramPipeCtxPtr)
		DdxCreatePipeCtx_( pipe, &MatchHistogramPipeElement, FALSE );
    if( !IsPointer_(ctx) ) return( (int) ctx );
    
    /* 
    **	Save the parameter values 
    */
    MhSrcSnk_(ctx) = src;
    MhDstSnk_(ctx) = dst;
    MhIdc_(ctx)	   = idc;
    MhHShape_(ctx) = hshape;
    MhParam1_(ctx) = param1;
    MhParam2_(ctx) = param2;

    /* 
    **	Setup one drain from source sink, receiving all components 
    */
    MhSrcDrn_(ctx) = DdxRmCreateDrain_(MhSrcSnk_(ctx),-1);
    if( !IsPointer_(MhSrcDrn_(ctx)) ) return( (int) MhSrcDrn_(ctx) );

    /* 
    **	The datatypes handled by this module and those handled by DdxPoint_ 
    **	and DdxHistogram_ must be in synch
    **	Bytes & words are ok. 
    */
    DdxRmSetDType_( MhSrcDrn_(ctx), UdpK_DTypeUndefined, DtM_BU | DtM_WU );
    DdxRmSetDType_( MhDstSnk_(ctx), UdpK_DTypeUndefined, DtM_BU | DtM_WU );

    /* 
    **	Set quantums to process the entire image 
    */
    DdxRmSetQuantum_( MhSrcDrn_(ctx), SnkHeight_(MhSrcSnk_(ctx),0) );
    DdxRmSetQuantum_( MhDstSnk_(ctx), SnkHeight_(MhDstSnk_(ctx),0) );

    return( Success );
}					/* end SmiCreateMatchHistogram */

/*****************************************************************************
**  _MatchHistogramInitialize
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function initializes the MATCHHISTOGRAM pipeline element prior to
**	an activation.
**
**  FORMAL PARAMETERS:
**
**      ctx - MatchHistogram pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	_MatchHistogramInitialize(ctx)
MatchHistogramPipeCtxPtr	ctx;
{

    PipeDataPtr data;
    int status  = DdxRmInitializePort_( CtxHead_(ctx), MhSrcDrn_(ctx) );
    if( status == Success )
	status  = DdxRmInitializePort_( CtxHead_(ctx), MhDstSnk_(ctx) );

    /* Currently processing from first (and only) source drain */
    CtxInp_(ctx) = MhSrcDrn_(ctx);

    return( Success );
}

/*****************************************************************************
**  _MatchHistogramActivate
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine reads data from the pipeline.  Since the histogram
**      must be calculated across the entire image, we can only accumulate
**      the scanlines until _MatchHistogramFlush is called.
**
**  FORMAL PARAMETERS:
**
**      ctx - MatchHistogram pipeline element context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _MatchHistogramActivate(ctx)
MatchHistogramPipeCtxPtr    ctx;
{
    PipeDataPtr	 src, dst = NULL;
    int status;

    for( status = Success; status == Success; src = DdxRmDeallocData_(src),
					      dst = DdxRmDeallocData_(dst) )
	{
	src = DdxRmGetData_( ctx, MhSrcDrn_(ctx) );
	if( !IsPointer_(src) ) return( (int) src );

	dst = DdxRmAllocData_( MhDstSnk_(ctx), DatCmpIdx_(src),
			       DatY1_(src), DatHeight_(src) );
	status  = (int) dst;
	if( !IsPointer_(dst) ) continue;

	status = DdxMatchHistogram_( DatUdpPtr_(src),
				     DatUdpPtr_(dst),
				     MhIdc_(ctx),
				     MhHShape_(ctx),
				     MhParam1_(ctx),
				     MhParam2_(ctx) );
	if( status != Success ) continue;

	status = DdxRmPutData_( MhDstSnk_(ctx), dst );
	dst    = NULL;
	}
    return( status );
}

/*****************************************************************************
**  _MatchHistogramDestroy
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine deallocates all resources allocated by the MATCHHISTOGRAM
**	pipeline element.
**
**  FORMAL PARAMETERS:
**
**      ctx - dither pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int 	    _MatchHistogramDestroy(ctx)
MatchHistogramPipeCtxPtr    ctx;
{
    MhSrcDrn_(ctx) = DdxRmDestroyDrain_(MhSrcDrn_(ctx));
    return( Success );
}

/******************************************************************************
**  SmiMatchHistogram
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine performs a non-pipelined transformation on the source
**      image such that its new distribution has the specified shape and
**      parameters.
**
**  FORMAL PARAMETERS:
**
**	in	- Pointer to source udp (initialized)
**	out	- Pointer to destination udp (initialized, maybe without array)
**      idc     - Pointer to a UDP describing the Region of Interest,
**                or Control Processing Plane.  (NULL if no IDC)
**      hshape  - Shape of the resulting histogram (distribution)
**                  XieK_Flat
**                  XieK_Gaussian
**                  XieK_Hyperbolic
**      param1  - First parameter of the distribution (hshape dependent)
**      param2  - Second parameter of the distribution (hshape dependent)
**     
**
******************************************************************************/
int  SmiMatchHistogram( in, out, idc, hshape, param1, param2)
UdpPtr		in;
UdpPtr		out;
UdpPtr          idc;
int             hshape;
float           param1;
float           param2;
{
UdpPtr	src = NULL;
UdpPtr  dst = NULL;
UdpPtr  lut = NULL;
int	status;


/*
**  Allocate the destination image array
*/
if( out->UdpA_Base == NULL )
{
    out->UdpA_Base = DdxMallocBits_( out->UdpL_ArSize );
    if( out->UdpA_Base == NULL ) return( BadAlloc );
}

/*
**  Convert src and destination to byte data.
*/
src = _ChkUdp( in, TRUE );
if( !IsPointer_(src) )
    {
    status = (int) src;
    goto clean_up;
    }
dst = _ChkUdp( out, FALSE );
if( !IsPointer_(dst) )
    {
    status = (int) dst;
    goto clean_up;
    }

/* Create a lookup table which implements the desired transformation */
status = _CreateMHLut( src, idc, hshape, param1, param2, &lut );

/* And remap the image to have the new distribution */
if( status == Success )
    status  = DdxPoint_( src, dst, idc, lut );

clean_up :

if( IsPointer_(lut) )
    lut = _FreeUdp( lut );			/* Free the lookup table    */

if( IsPointer_(src) && src != in )
    src  = _FreeUdp(src);			/* Free the temporary copy  */

if( IsPointer_(dst) && dst != out )
    {
    /* Convert the output data if necessary */
    status = DdxConvert_( dst, out, XieK_MoveMode );
    dst = _FreeUdp(dst);
    }
    
return( status );
}				    /* end SmiMatchHistogram */


/******************************************************************************
**  _ChkUdp
**
**  FUNCTIONAL DESCRIPTION:
**
**	Check udp and convert to a supported datatype if necessary.
**
**  FORMAL PARAMETERS:
**
**	src	- Pointer to udp
**	cvt	- boolean: TRUE if data should be converted
**
******************************************************************************/
static UdpPtr _ChkUdp( src, cvt )
    UdpPtr	src;
    int		cvt;
{
    int	stype, status = Success;
    UdpPtr	dst = src;

    stype = DdxGetTyp_( src );

    /*
    **	If the src is bits or floats it will be converted to bytes which 
    **	can be processed properly by DdxPoint_ and DdxHistogram_.
    */	 
    if( stype != XieK_ABU )
    {
	if( stype != XieK_AWU )
	{   /*	 
	    **  Convert a bit or float array to bytes
	    */
	    dst = (UdpPtr) DdxMalloc_(sizeof(UdpRec));
	    if( dst == NULL ) return( (UdpPtr) BadAlloc );

	    *dst = *src;
	    dst->UdpB_DType	  = UdpK_DTypeBU;
	    dst->UdpB_Class	  = UdpK_ClassA;
	    dst->UdpW_PixelLength = 8;
	    dst->UdpL_PxlStride   = dst->UdpW_PixelLength;
	    dst->UdpL_ScnStride   = dst->UdpL_PxlStride*src->UdpL_PxlPerScn;
	    dst->UdpL_ArSize      = dst->UdpL_ScnStride*src->UdpL_ScnCnt;
	    dst->UdpA_Base	  = cvt ? NULL : DdxMallocBits_(dst->UdpL_ArSize);
	    if( cvt )
		status = DdxConvert_( src, dst, XieK_MoveMode );

	    if( status != Success || dst->UdpA_Base == NULL )
	    {
		_FreeUdp(dst);
		dst = (UdpPtr) status;
	    }
	}
    }

    return( dst );
}				    /* end _ChkUdp */


/******************************************************************************
**  _FreeUdp
**
**  FUNCTIONAL DESCRIPTION:
**
**	Free a UDP and the associated bits.
**
**  FORMAL PARAMETERS:
**
**	udp	- Pointer to udp
**
******************************************************************************/
static UdpPtr _FreeUdp(udp)
    UdpPtr udp;
{
    if( IsPointer_(udp) )
	{
	if( IsPointer_(udp->UdpA_Base) )
	   DdxFreeBits_(udp->UdpA_Base);
	DdxFree_(udp);
	}
    return( NULL );
}

/******************************************************************************
**
**  FUNCTIONAL DESCRIPTION:
**
**      Takes a UDP, creates a lookup table by matching its histogram to the 
**	specified shape.
**
**  FORMAL PARAMETERS:
**
**	src_udp	-  pointer to source udp
**	idc     -  CPP or ROI selecting region to be processed.
**	hshape  -  indicator for distribution shape
**	param1  -  general argument (depending on the density type)
**	param2  -  general argument (depending on the density type)
**      lut_ptr -  place to receive pointer to lookup table UDP.
**
**  FUNCTION VALUE:
**
*******************************************************************************/
static int _CreateMHLut( src_udp, idc, hshape, param1, param2, lut_ptr )
UdpPtr src_udp;
UdpPtr idc;
int hshape;
float param1;
float param2;
UdpPtr *lut_ptr;
{
    unsigned status;
    unsigned i,j,k;			/* Indices for loops		    */
    unsigned long *hist_ptr, *h_ptr;	/* Histogram Pointers		    */
    unsigned long hist_cnt;		/* Number elements in histogram     */
    double fmax, prev_err, err;		/* working float values		    */
    unsigned long total_pixels;		/* number of pixels in src udp	    */

    float *src_pd_base, *src_pd;	/* Src Probability Density	    */
    float *src_cdf_base, *src_cdf;	/* Src Cumulative Disribution	    */
    float *src_map_base, *src_map;	/* Src Transfer Function for Eq.    */

    float *dst_cdf_base, *dst_cdf;	/* Target Cumulative Disribution    */

    unsigned long *remap_base, *remap;	/* Lookup Table For Match Histogram */
    UdpPtr lut;

    hist_cnt = src_udp->UdpL_Levels;

    hist_ptr = (unsigned long *) DdxMalloc_(hist_cnt * sizeof(long));
    if( hist_ptr == NULL ) return( BadAlloc );

    status = DdxHistogram_( src_udp, idc, hist_ptr );
    if( status != Success )
	{
	DdxFree_(hist_ptr);
	return( status );
	}
    /*
    **  Allocate memory for the source probability density and cumulative
    **  functions 
    */
    src_pd_base  = (float *) DdxMalloc_(2 * hist_cnt * sizeof(float));
    if( src_pd_base == NULL )
	{
	DdxFree_(hist_ptr);
	return( BadAlloc );
	}
    src_cdf_base = (float *)(src_pd_base + hist_cnt);   

    /*
    **  Allocate memory for the lookup table
    */
    remap_base = (unsigned long *) DdxCallocBits_(hist_cnt * sizeof(long) * 8);
    if( remap_base == NULL )
	{
	DdxFree_(hist_ptr);
	DdxFree_(src_pd_base);
	return( BadAlloc );
	}
    /*
    **  Obtain Normalized, Equalized, Histogram
    */
    total_pixels = src_udp->UdpL_PxlPerScn * src_udp->UdpL_ScnCnt;
    h_ptr = hist_ptr;

    /* 
    ** Calculate probability density function of source UDP
    */
    src_pd = (float *)src_pd_base;
    for( i = 0; i < hist_cnt; i++ )
	*src_pd++ = (float)(*h_ptr++) / (float)total_pixels;

    /*
    ** Calculate Cumulative Distribution Function 
    ** (i.e. histogram equalization transfer function)
    */
    src_pd  = (float *)src_pd_base;
    src_cdf = (float *)src_cdf_base;
    *src_cdf++ = *src_pd++;
    for( i = 1; i < hist_cnt; i++, src_cdf++ )
	*src_cdf = *(src_cdf-1) + *src_pd++;

    /* 
    ** Generate src_pd -> src_cdf mapping (i.e. make sure src_cdf values are 
    ** real levels of the source) 
    ** Take advantage of the fact that src_cdf is a monotonically increasing 
    ** function (use of k)
    */
    src_cdf = (float *)src_cdf_base;
    fmax = (float) (hist_cnt - 1);		/* max value */

    switch( hshape )
	{
    case XieK_Flat:

	/* flat - histogram equalization */
	remap = remap_base;

	for( k = 0, i= 0; i < hist_cnt; i++, src_cdf++ )
	    {
	    for( prev_err = 1.0,j = k; j < hist_cnt; j++ )
		{
		/* calc abs diff  */
		err = fabs ( (double)(((float)j / fmax) - *src_cdf));
		if( err == 0.0 )
		    {
		    j++;
		    break;
		    }
		else if( err > prev_err )
		    break;
		else
		    prev_err = err;
		}
	    *remap++ = j - 1;
	    k = j - 1;				    /* search next from k    */
	    }
	break;				/* end of histogram equalization     */

    default:			    /* all other density types - erroneous
				    ** density types will be flagged in 
				    ** the generate_cdf subroutine
				    */

	/* Histogram matching */
	/*
	** Allocate memory for mapping array (from src to an equalized 
	** histogram) and dst cumulative function
	*/
	src_map_base =  (float*) DdxMalloc_(2 * hist_cnt * sizeof(float));
	if( src_map_base == NULL )
	    {
	    DdxFree_(hist_ptr);
	    DdxFree_(src_pd_base);
	    DdxFreeBits_(remap_base);
	    return( BadAlloc );
	    }
	dst_cdf_base = (float *)(src_map_base + hist_cnt);

	/*
	** Map cdf elements to actual levels
	*/
	src_map = (float *)src_map_base;
	for( k = 0, i= 0; i < hist_cnt; i++, src_cdf++ )
	    {
	    for( prev_err = 1.0,j = k; j < hist_cnt; j++ )
		{
		/* calc abs diff  */
		err = fabs ( (double)(((float)j / fmax) - *src_cdf));
		if( err == 0.0 )
		    {
		    j++;
		    break;
		    }
		else if( err > prev_err )
		    break;
		else
		    prev_err = err;
		}
	    *src_map++ = (float)(j - 1) / fmax;	     /* set to norm val       */
	    k = j - 1;				     /* search next from k    */
	    }

	/*
	** Call general cumulative distribution subroutine
	*/
	status = _GenerateCdf( hshape, param1, param2, dst_cdf_base, hist_cnt );
	if( status != Success )
	    {
	    DdxFree_(hist_ptr);
	    DdxFree_(src_pd_base);
	    DdxFree_(src_map_base);
	    DdxFreeBits_(remap_base);
	    return( status );
	    }
	/* 
	** Match closest CDF value to each src_map value to obtain 
	** src -> dst mapping (remap)
	*/
	src_map = (float *)src_map_base;
	dst_cdf = (float *)dst_cdf_base;
	remap = remap_base;
	for( k = 0, i= 0; i < hist_cnt; i++, src_map++ )
	    {
	    dst_cdf = (float *)(dst_cdf_base + k);
	    for( prev_err = 1.0, j = k; j < hist_cnt; j++ )
		{
		err = fabs ((double)(*dst_cdf++ - *src_map));
		if( err == 0.0 )
		    {
		    j++;
		    break;
		    }
		else if( err > prev_err )
		    break;
		else
		    prev_err = err;
		}
	    *remap++ = j - 1;
	    k = j - 1; 
	    }
	DdxFree_(src_map_base);
	break;				/* end of matching density function */
	}
	   
/* debug */
/*
**{
**    unsigned long *ptr;
**    ptr = remap_base;
**    for (i=0; i < 256; i++)    
**	printf ("%d %d\n",i, *ptr++);
**}
*/
/* end of debug */

    DdxFree_(hist_ptr);
    DdxFree_(src_pd_base);

/* Build a one-scanline UDP to describe the LUT */    
    lut = (UdpPtr) DdxMalloc_(sizeof(UdpRec));
    if( lut == NULL )
	{
	DdxFreeBits_(remap_base);
	return( BadAlloc );
	}
    lut->UdpW_PixelLength = sizeof(long) * 8;
    lut->UdpB_DType = UdpK_DTypeLU;
    lut->UdpB_Class = UdpK_ClassA;
    lut->UdpA_Base = (unsigned char *)remap_base;
    lut->UdpL_ArSize = hist_cnt * lut->UdpW_PixelLength;
    lut->UdpL_PxlStride = lut->UdpW_PixelLength;
    lut->UdpL_ScnStride = lut->UdpL_ArSize;
    lut->UdpL_X1 = 0;
    lut->UdpL_Y1 = 0;
    lut->UdpL_X2 = hist_cnt - 1;
    lut->UdpL_Y2 = 0;
    lut->UdpL_PxlPerScn = hist_cnt;
    lut->UdpL_ScnCnt = 1;
    lut->UdpL_Pos = 0;
    lut->UdpL_CompIdx = 0;
    lut->UdpL_Levels = hist_cnt;

    *lut_ptr = lut;

    return( status );
}

/*
**  This subroutine generates a cumulative distribution function for the
**  specified density type (hshape).  The param arguments depend on the
**  density.
*/
static int _GenerateCdf (hshape, param1, param2,  out_ptr, levels)
unsigned long hshape;		        /* density type			    */
double param1, param2;			/* density parameters		    */
float *out_ptr;				/* ptr to the output distr. func    */
unsigned long levels;			/* number of levels (array elements */
{
    unsigned long i,j;
    float *target;			/* working pointer		    */
    float fmax, sfactor, sum;
    double z;

    switch (hshape)
	{
    case (XieK_Gaussian):
	/* 
	** Generate Gaussian Probability Density Function Around specified 
	** mean level (param1) with the specified variance (param2)
	*/
	if( param1 == 0.0 ) 
	    param1 = levels >> 1;
	if( param2 == 0.0 )
	    param2 = param1 / 4;

	target = (float *)out_ptr;
	for(sum = 0, fmax = 0.0, i = 0; i < levels; i++ )
	    {
	    z  = (float)i - (float)param1;
	    z  = -1.0 * z * z;
	    z /= (2 * param2 * param2);
	    z  = exp(z);
	    *target = (float)((z * K1) / param2);
	    if( *target >= fmax ) 
		fmax = *target;
	    sum += *target++;
	    }
	/*
	** Normalize and Calculate Cumulative Distribution Function For the 
	** Gaussian probability density (i.e. histogram equalization 
	** transfer function)
	*/
	sfactor    = 1 / sum;
	target     = (float *)out_ptr;
	*target++ *= sfactor;
	for( i = 1; i < levels; i++, target++ )
	    *target = *(target- 1) + (*target * sfactor);
	break;

    case (XieK_Hyperbolic):
	/* 
	** Generate Hyperbolic Probability Density Function with specified 
	** constant (param1)
	*/
	if( param1 == 0.0 )			/* default is 1/4 th */
	    param1 = levels >> 2;

	target = (float *)out_ptr;
	param2 = 1.0 / param1;		/* use mult rather than div */

	if( param2 != 1.0 )			/* default is neg exp */
	    {
	    for( sum = 0, fmax = 0.0, i = 0; i < levels; i++ )
		{
		z = log (1.0 + param2);
		*target = (float)(1.0 / (((double)i + param1) * z));
		if( *target >= fmax )
		    fmax = *target;
		sum += *target++;
		}
	    }
	else
	    {
	    j = levels - 1;
	    for( sum = 0, fmax = 0.0, i = 0; i < levels; i++, j-- )
		{
		z = log (1.0 + param2);
		*target = (float)(1.0 / (((double)j + param1) * z));
		if( *target >= fmax )
		    fmax = *target;
		sum += *target++;
		}
	    }
	/*
	** Normalize and Calculate Cumulative Distribution Function For the 
	** hyperbolic probability density (i.e. histogram equalization 
	** transfer function)
	*/
	sfactor    = 1 / sum;
	target     = (float *)out_ptr;
	*target++ *= sfactor;
	for( i = 1; i < levels; i++, target++ )
	    *target = *(target- 1) + (*target * sfactor);
	break;

    default :
	return( BadValue );
	}		    /* end of switch */
    return( Success );
}	
/* end module SmiMatchHistogram.c */
