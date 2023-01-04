/******************************************************************************
**
**  Copyright (c) Digital Equipment Corporation, 1990-1991 All Rights Reserved. 
**  Unpublished rights reserved under the copyright laws of the United States.
**  The software contained on this media is proprietary to and embodies the 
**  confidential technology of Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and media is authorized only 
**  pursuant to a valid written license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the U.S. 
**  Government is subject to restrictions as set forth in Subparagraph 
**  (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
**
*****************************************************************************/

/******************************************************************************
**
**  FACILITY:
**
**      Image Processing Services (IPS)
**
**  ABSTRACT:
**
**      ips__freq_filter routines for the creation and manipulation of 
**	frequency domain filtering
**
**  ENVIRONMENT:
**
**      VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
**      Richard Piccolo, Digital Equipment Corp.
**
**  CREATION DATE:
**
**      Ocober 2, 1990
**
*******************************************************************************/

/*
**  Table of contents:
*/
#ifdef NODAS_PROTO
long _IpsCreateFilter();		/* create a float udp freq filter */
long _IpsFilter();			/* utilize a filter		  */
#endif

/*
**  Include files:
*/
#include    <IpsDef.h>			/* IPS Definitions		     */
#include    <IpsDefP.h>			/* IPS Private definitions	     */
#include    <IpsStatusCodes.h>		/* IPS Status Codes		     */
#include    <IpsMemoryTable.h>		/* IPS Memory Vector Table	     */
#ifndef NODAS_PROTO
#include    <ipsprot.h>			/* Ips prototypes */
#endif
#include    <math.h>

/*
**  Equated Symbols:
*/

/*
**  External References:
*/
#ifdef NODAS_PROTO
long _IpsVerifySizeFFT();		/* check size - from freq_utils	      */
long _IpsCreateUdp();			/* create and build udp from udp_utils*/
long _IpsBuildDstUdp();			/* fill dst udp	- from udp_utils      */
double pow();
double sqrt();
#endif

/*
**  Local Storage:
*/

/*
**  Macro Definitions
*/

/******************************************************************************
**
**  _IpsCreateFilter
**
**  FUNCTIONAL DESCRIPTION:
**
**      Creates a float udp used for filtering via _IpsFilter.  
**
**	NOTE: this filter routine will define the DC term as the middle of 
**	udp plane.  This complies with the FFT implementation using the
**	processing flag IpsM_FFTCenterDC
**
**  FORMAL PARAMETERS:
**
**	filter type --> IpsK_Ideal_LP		 Ideal Low Pass
**			IpsK_Ideal_HP		 Ideal High Pass *
**			IpsK_Trapezoidal_LP	 Trapezoidal Low Pass *
**			IpsK_Trapezoidal_HP	 Trapezoidal High Pass *
**			IpsK_Butterworth_LP	 Butterworth Low Pass
**			IpsK_Butterworth_HP	 Butterworth High Pass
**			* = not yet implemented
**
**	size	    --> dimensions of resultant udp (square dimensions)
**	f_pass	    --> radius of passed frequencies (used for ideal and trap)
**	f_cutoff    --> radius of total attentuation (used for all except ideal)
**	udp_ptr	    --> pointer to address of a udp
**
**

**	Example of ideal high pass filter:
**
**  1.0	|-----------------+
**      |                 |
**      |                 |
**      |                 |
**      |                 |
**      |                 |
**      |                 |
**	+-----------------|--------------->
**	DC	        f_pass
**
**	Example of trapezoidal high pass filter:
**
**  1.0	---------+
**	|         \	  
**	|          \	  
**      |           \	  
**	|            \	  
**	|             \	  
**	|              \	  
**	|               \	  
**	|                \	  
**	+---------|-------|------------>
**	DC      f_pass  f_cutoff
**
**	Example of butterworth high pass filter:
**
**  1.0	------------+
**	|            \	  
**	|             \	  
**  0.5 -              *	  
**	|               \	  
**	|                \	  
**	+--------------|-------------->
**	DC          f_cutoff
**
**	For Butterworth only f_cutoff is defined and represents a 50% 
**	attentuation.  The rate of total frequency attentuation is determined 
**	by the order of the filter; the higher the order the steeper the 
**	filter.
**
**	High pass filters are conceptually the same as low pass filters but 
**	allow the higher frequencies to pass instead.  Slopes are the negative 
**	of those found in the low pass filters.
**
**	Example of butterworth low pass filter:
**
**  1.0	-       +-------
**	|      /	  
**	|     /	  
**	|    /	  
**  0.5 -   *	  
**	|  /	  
**	| /	  
**	|/
**      +---|-------------------->
**	DC  f_cutoff
**
**  NOTE:  there are potentially other filters and variations that can be
**	   implemented.  This first code entry represents filters to illustrate
**	   frequency filter usage.
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      long status
**
*******************************************************************************/

long _IpsCreateFilter (filter_type, size, f_pass, f_cutoff, order, udp)
unsigned long	filter_type;		/* any supported filter type	      */
unsigned long	size;			/* filter dimensions - square 	      */
unsigned long	f_pass;			/* total attenuated f_pass 	      */
unsigned long	f_cutoff;		/* filter cutoff f_cutoff;	      */
unsigned long	order;			/* filter order for butterworth	      */
struct	 UDP	**udp;			/* returned UDP descriptor	      */
    {
    double	    xc, yc;			/* center		    */
    double	    z1, f2;			/* working variables	    */
    double	    x_diff, y_diff;		/* x and y differences      */
    double	    x_diff2, y_diff2;		/* x and y differences      */
    double	    dist;			/* distance from curr to DC */
    double	    dist2;			/* dist**2 from curr to DC  */
    double	    r2;				/* radius squared	    */
    unsigned long   i,j,k;			/* indices		    */
    long	    ix, iy;			/* coordinate indices	    */
    struct UDP	    *filter;			/* working udp		    */
    float	    *src_base;			/* data pointer		    */
    double	    slope,y_int;		/* trapezoid variables	    */
    double	    delta_x, delta_y;		/* trapezoid variables	    */
    long	    nu;
    long	    status;
    double	    freq, exponent;
	    
    /*
    ** check to make sure size is power of two
    */
    nu = _IpsVerifySizeFFT (size);
    if (nu == -1)
	return (IpsX_UNSIMGDIM);
	    
    /* 
    ** check filter argument 
    */
    switch (filter_type)
	{
	case IpsK_Ideal_LP:
	    if (f_pass > (size >> 1))
		return (IpsX_INCNSARG);
	break;

	case IpsK_Trapezoidal_LP:
	    if (f_pass > f_cutoff || f_pass > (size >> 1))
		return (IpsX_INCNSARG);
	break;

	case IpsK_Butterworth_HP:
	case IpsK_Butterworth_LP:
	    if (f_cutoff > (size >> 1))
		return (IpsX_INCNSARG);
	break;

	default:
	    return (IpsX_UNSOPTION);
	break;
	}

    /*
    ** create udp and plane of type float
    */
    status = _IpsCreateUdp (size, size, 0, 0, 0, 0, IpsK_DTypeF, &filter);
    if (status != IpsX_SUCCESS)
	return (status);
    
    /*
    ** generate filter
    */
    switch (filter_type)
	{
	case IpsK_Ideal_LP:
	    r2 = f_pass * f_pass;
	    xc = yc = size>>1;
	    src_base = (float *)filter->UdpA_Base + (filter->UdpL_Pos>>5);

	    y_diff = -1 * yc;				/* starting y - yc  */
	    for (iy = 0; iy < filter->UdpL_ScnCnt; iy++)
		{
		x_diff = -1 * xc;			/* starting x - xc  */
		y_diff2 = y_diff * y_diff;		/* y diff	    */
		for (ix = 0; ix < filter->UdpL_PxlPerScn; ix++)
		    {
		    x_diff2 = x_diff * x_diff;		/* x diff	    */
		    dist2 = x_diff2 + y_diff2;		/* distance from DC */
		    if (dist2 <= r2)
			*src_base++ = 1.0;
		    else
			*src_base++ = 0.0;
		    x_diff++;
		    }
		y_diff++;
		}
	break;

	case IpsK_Trapezoidal_LP:
	    r2 = f_pass * f_pass;
	    f2 = f_cutoff * f_cutoff;
	    xc = yc = size>>1;
	    src_base = (float *)filter->UdpA_Base + (filter->UdpL_Pos>>5);

	    /* 
	    ** use y = slope * x + y_int 
	    */
	    delta_x = (double) (f_cutoff - f_pass);	
	    delta_y = -1.0;
	    slope = delta_y / delta_x;
	    y_int = -1 * (slope * (double)f_cutoff);

	    y_diff = -1 * yc;				/* starting y - yc  */
	    for (iy = 0; iy < filter->UdpL_ScnCnt; iy++)
		{
		x_diff = -1 * xc;			/* starting x - xc  */
		y_diff2 = y_diff * y_diff;		/* y diff	    */
		for (ix = 0; ix < filter->UdpL_PxlPerScn; ix++)
		    {
		    x_diff2 = x_diff * x_diff;		/* x diff	    */
		    dist2 = x_diff2 + y_diff2;		/* distance from DC */
		    if (dist2 <= r2)
			*src_base++ = 1.0;
		    else if (dist2 <= f2)
			/* 
			** extrapolate linearly from f_cutoff to f_pass 
			*/
			*src_base++ = (slope * sqrt(dist2)) + y_int;
		    else
			*src_base++ = 0.0;
		    x_diff++;
		    }
		y_diff++;
		}
	break;

	case IpsK_Butterworth_HP:
	    xc = yc = size>>1;
	    src_base = (float *)filter->UdpA_Base + (filter->UdpL_Pos>>5);
	    freq = (double)f_cutoff;
	    exponent = order * 2;

	    y_diff = -1 * yc;				/* starting y - yc  */
	    for (iy = 0; iy < filter->UdpL_ScnCnt; iy++)
		{
		x_diff = -1 * xc;			/* starting x - xc  */
		y_diff2 = y_diff * y_diff;		/* y diff	    */
		for (ix = 0; ix < filter->UdpL_PxlPerScn; ix++)
		    {
		    x_diff2 = x_diff * x_diff;		/* x diff	    */
		    dist = sqrt(x_diff2 + y_diff2);	/* distance from dc */
		    if (dist == 0.0)
			{
			*src_base++ = 0.0;
			}
		    else
			{
			z1 = freq / dist;
			z1 = pow (z1, exponent);
			*src_base++ = 1.0 / (1.0 + z1);
			}
		    x_diff++;
		    }
		y_diff++;
		}
	break;

	case IpsK_Butterworth_LP:
	    xc = yc = size>>1;
	    src_base = (float *)filter->UdpA_Base + (filter->UdpL_Pos>>5);
	    freq = (double)f_cutoff;
	    exponent = order * 2;

	    y_diff = -1 * yc;				/* starting y - yc  */
	    for (iy = 0; iy < filter->UdpL_ScnCnt; iy++)
		{
		x_diff = -1 * xc;			/* starting x - xc  */
		y_diff2 = y_diff * y_diff;		/* y diff	    */
		for (ix = 0; ix < filter->UdpL_PxlPerScn; ix++)
		    {
		    x_diff2 = x_diff * x_diff;		/* x diff	    */
		    dist = sqrt(x_diff2 + y_diff2);	/* distance from dc */
		    z1 = dist / freq;
		    z1 = pow (z1, exponent);
		    *src_base++ = 1.0 / (1.0 + z1);
		    x_diff++;
		    }
		y_diff++;
		}
	break;

	default:
	    return (IpsX_UNSOPTION);
	break;

	}
		    
    *udp = filter;
    return (IpsX_SUCCESS);
    }

/******************************************************************************
**
**  _IpsFilter
**
**  FUNCTIONAL DESCRIPTION:
**
**      Implement filtering operation using previously created filter udp
**	via _IpsCreateFilter
**
**  FORMAL PARAMETERS:
**
**	src_udp	    --> src udp (complex - output from _IpsFFT)
**	filter_udp  --> filter udp (float) (from createfilterudp)
**	dst_udp	    --> dst udp (complex - input to inverse FFT)	
**	flags	    --> none
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      long status
**
**  RESTRICTIONS:
**
**	Note: pxl per scanline and scanline count must be powers of two
**	ROIs not supported for frequency domain routines
**
*******************************************************************************/

long _IpsFilter (src_udp, filter_udp, dst_udp, flags)
struct UDP  *src_udp;		    /* complex fourier image		    */
struct UDP  *filter_udp;	    /* float udp representing filter	    */
struct UDP  *dst_udp;		    /* dst udp				    */
unsigned long	flags;		    /* processing flags (none implemented   */
    {
    long	    status;
    unsigned long   mem_alloc= 0;
    unsigned long   i,j;
    float	    *filter_ptr;
    COMPLEX	    *src_ptr;
    COMPLEX	    *dst_ptr;
    long	    nu;

    /*
    ** check if src_udp is complex
    */
    if (src_udp->UdpB_DType != UdpK_DTypeC)
	return (IpsX_INVDTYPE);

    /*	
    ** check sizes for power of two and equality 
    */
    nu = _IpsVerifySizeFFT (src_udp->UdpL_ScnCnt);
    if (nu == -1)
	return (IpsX_UNSIMGDIM);

    if ((src_udp->UdpL_PxlPerScn != src_udp->UdpL_ScnCnt) ||
	(src_udp->UdpL_PxlPerScn != filter_udp->UdpL_PxlPerScn) ||
	    (filter_udp->UdpL_PxlPerScn != filter_udp->UdpL_ScnCnt))
		return (IpsX_INCNSARG);

    /* 
    ** if dst base is not defined indicate so using internal flag
    */
    if (dst_udp->UdpA_Base != 0) 
	mem_alloc = 1;

    /* 
    ** build dst udp (in place operation is allowed)
    */
    status = _IpsBuildDstUdp (src_udp, dst_udp, 0, 0, IpsK_InPlaceAllowed);
    if (status != IpsX_SUCCESS)
	return (status);

    filter_ptr =   (float *)filter_udp->UdpA_Base + (filter_udp->UdpL_Pos>>5);
    src_ptr = (COMPLEX *)src_udp->UdpA_Base + (src_udp->UdpL_Pos>>6); 
    dst_ptr = (COMPLEX *)dst_udp->UdpA_Base + (dst_udp->UdpL_Pos>>6); 
    for (j=0; j < dst_udp->UdpL_ScnCnt; j++) 
	{
	for (i=0; i< dst_udp->UdpL_PxlPerScn; i++) 
	    {
	    dst_ptr[i].real = src_ptr[i].real * *filter_ptr;
	    dst_ptr[i].imag = src_ptr[i].imag * *filter_ptr;
	    filter_ptr++;
	    }
	src_ptr += src_udp->UdpL_PxlPerScn;
	dst_ptr += dst_udp->UdpL_PxlPerScn;
    	}

    return (IpsX_SUCCESS);
    }
