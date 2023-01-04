/******************************************************************************
**
**  Copyright (c) Digital Equipment Corporation, 1990-1991 All Rights Reserved.
**  Unpublished rights reserved under the copyright laws of the United States.
**  The software contained on this media is proprietary to and embodies the 
**  confidential technology of Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and media is authorized only 
**  pursuant to a valid written license from Digital Equipment Corporation.

**  RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the U.S. 
**  Government is subject to restrictions as set forth in Subparagraph 
**  (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
**/

/*****************************************************************************
**
**  FACILITY:
**
**      Image Processing Services
**
**  ABSTRACT:
**
**      _IpsCreateLut_MH creates a lookup table such that the source's
**	histogram will match specified shape.
**
**  ENVIRONMENT:
**
**      VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
**      Richard Piccolo
**
**  CREATION DATE:
**
**      April 3, 1990
**
************************************************************************/

/*
**  Table of contents
*/
#ifdef NODAS_PROTO
long  generate_cdf();
#endif

/*
**  Include files
*/
#include <IpsDef.h>		    /* IPS Image Definitions	    */
#include <IpsStatusCodes.h>	    /* IPS Status Codes		    */
#include <IpsMemoryTable.h>	    /* IPS Memory Mgt. Functions    */
#ifndef NODAS_PROTO
#include <ipsprot.h>		    /* Ips prototypes */
#endif
#include <math.h>		    /* Math Runtime Library	    */
/*
**  MACRO definitions
*/

/*
**  Equated Symbols
*/
# define K1                 .39894228               /* 1 / sqrt (2pi) */

/*
**  External References
*/
#ifdef NODAS_PROTO
long _IpsHistogram();
#endif

/******************************************************************************
**
**  FUNCTIONAL DESCRIPTION:
**
**      Takes a UDP, creates a lookup table by matching its histogram to the 
**	specified shape.
**
**  FORMAL PARAMETERS:
**
**	src_udp --> pointer to source      udp (initialized)
**	cpp --> control processing plane
**	density_type --> indicator for Gaussian, Gamma, Exponential, Flat
**	param1 -->  general argument (depending on the density type)
**	param2 -->  general argument (depending on the density type)
**	user_pd --> user density function (optional)
**	lut     --> returned LUT pointer 
**
**  IMPLICIT INPUTS:	none
**  IMPLICIT OUTPUTS:	Lookup Table Structure
**  FUNCTION VALUE:	status return
**  SIGNAL CODES:	none
**  SIDE EFFECTS:	none
**                                                                            
*******************************************************************************/

long _IpsCreateLut_MH (src_udp, cpp, density_type, param1, param2,
				user_pd, lut_ptr)
struct UDP *src_udp;
struct UDP *cpp;
unsigned long density_type;
double param1;
double param2;
float *user_pd;
unsigned long **lut_ptr;
{
    unsigned i,j,k;			/* Indices for loops		    */
    unsigned long *hist_ptr, *h_ptr;	/* Histogram Pointers		    */
    unsigned long hist_cnt;		/* Number elements in histogram     */
    unsigned long size;			/* Table Byte Size (no. of levels)  */
    double fmax, prev_err, err;		/* working float values		    */
    unsigned long total_pixels;		/* number of pixels in src udp	    */

    float *src_pd_base, *src_pd;	/* Src Probability Density	    */
    float *src_cdf_base, *src_cdf;	/* Src Cumulative Disribution	    */
    float *src_map_base, *src_map;	/* Src Transfer Function for Eq.    */

    float *dst_cdf_base, *dst_cdf;	/* Target Cumulative Disribution    */

    unsigned long *remap_base, *remap;	/* Lookup Table For Match Histogram */
    long status;

    status = _IpsHistogram (src_udp, cpp, &hist_ptr);
    if (status != IpsX_SUCCESS)
	return (status);

    hist_cnt = src_udp->UdpL_Levels;
    size = sizeof(long) * hist_cnt;

    /*
    **  Allocate memory for the source probability density and cumulative
    **  functions 
    */
    src_pd_base =  (float*) (*IpsA_MemoryTable[IpsK_Alloc]) ((2 * size),0,0);
    if (!src_pd_base) 
	return (IpsX_INSVIRMEM);
    src_cdf_base = (float *)(src_pd_base  + hist_cnt);   

    /*
    **  Allocate memory for the lookup table
    */
    remap_base =  (unsigned long *) (*IpsA_MemoryTable[IpsK_Alloc]) 
	(size,IpsM_InitMem,0);
    if (!remap_base)
	{
	(*IpsA_MemoryTable[IpsK_Dealloc])(hist_ptr);
	(*IpsA_MemoryTable[IpsK_Dealloc])(src_pd_base);
	 return (IpsX_INSVIRMEM);
	}

    /*
    **  Obtain Normalized, Equalized  Histogram
    */
    total_pixels = src_udp->UdpL_PxlPerScn * src_udp->UdpL_ScnCnt;
    h_ptr = hist_ptr;

    /* 
    ** Calculate probability density function of source UDP
    */
    src_pd = (float *)src_pd_base;
    for (i = 0; i < hist_cnt; i++)
	*src_pd++ = (float)(*h_ptr++) / (float)total_pixels;

    /*
    ** Calculate Cumulative Distribution Function 
    ** (i.e. histogram equalization transfer function)
    */
    src_pd = (float *)src_pd_base;
    src_cdf = (float *)src_cdf_base;
    *src_cdf++ = *src_pd++;
    for (i = 1; i < hist_cnt; i++, src_cdf++)
	*src_cdf = *(src_cdf-1) + *src_pd++;

    /* 
    ** Generate src_pd -> src_cdf mapping (i.e. make sure src_cdf values are 
    ** real levels of the source) 
    ** Take advantage of the fact that src_cdf is a monotonically increasing 
    ** function (use of k)
    */
    src_cdf = (float *)src_cdf_base;
    fmax = (float) (hist_cnt - 1);		/* max value */

    switch (density_type)
	{
	case IpsK_Flat:

	/* flat - histogram equalization */
	remap = (unsigned long *)remap_base;
	/*
	** Note:
	**
	** There exist an alternate way to match cdf values
	** to actual levels, however this is more susceptible to round
	** off errors thus at the (possible!) expense of execution time (which 
	** isn't much anyway) we match closest values.  Alternatively:
	**
	** for (i = 0; i < hist_cnt; i++)
        **  {
        **  *remap++ = (fmax * ((*src_cdf++ - min_level)/max_level)) + 0.5;
        **  *src_map++ = (float)k / fmax;
        **  }
	** where min_level = 1.0 / (float)hist_cnt
	** and
	**	 max_level = fmax / (float)hist_cnt
	*/

	for (k = 0, i= 0; i < hist_cnt; i++, src_cdf++)
	    {
	    for (prev_err = 1.0,j = k; j < hist_cnt; j++) 
		{
		/* calc abs diff  */
		err = fabs ( (double)(((float)j / fmax) - *src_cdf));
		if (err == 0.0)
		    {
		    j++;
		    break;
		    }
		else if (err > prev_err)
		    break;
		else
		    prev_err = err;
		}
	    *remap++ = j - 1;
	    k = j - 1;				    /* search next from k    */
	    }
	break;				/* end of histogram equalization     */

	default:			/* all other density types - erroneous
					** density types will be flagged in 
					** the generate_cdf subroutine
					*/

	/* Histogram matching */
	/*
	** Allocate memory for mapping array (from src to an equalized 
	** histogram) and dst cumulative function
	*/
	src_map_base =  (float*) (*IpsA_MemoryTable[IpsK_Alloc]) ((2*size),0,0);
	if (!src_map_base)
	    {
	    (*IpsA_MemoryTable[IpsK_Dealloc])(hist_ptr);
	    (*IpsA_MemoryTable[IpsK_Dealloc])(src_pd_base);
	    (*IpsA_MemoryTable[IpsK_Dealloc])(remap_base);
	    return (IpsX_INSVIRMEM);
	    }
        dst_cdf_base = (float *)(src_map_base + hist_cnt);

        /*
        ** Map cdf elements to actual levels (see comment above for flat)
        */
	src_map = (float *)src_map_base;
	for (k = 0, i= 0; i < hist_cnt; i++, src_cdf++)
	    {
	    for (prev_err = 1.0,j = k; j < hist_cnt; j++) 
		{
		/* calc abs diff  */
		err = fabs ( (double)(((float)j / fmax) - *src_cdf));
		if (err == 0.0)
		    {
		    j++;
		    break;
		    }
		else if (err > prev_err)
		    break;
		else
		    prev_err = err;
		}
	    *src_map++ = (float)(j - 1) / fmax;	     /* set to norm val       */
	    k = j - 1;				     /* search next from k    */
	    }

	/*
	** call general cumulative distribution subroutine
	*/
	status = generate_cdf (density_type, param1, param2, dst_cdf_base, 
	    hist_cnt, user_pd);
	if (status != IpsX_SUCCESS)
	    {
	    (*IpsA_MemoryTable[IpsK_Dealloc]) (hist_ptr);
	    (*IpsA_MemoryTable[IpsK_Dealloc]) (src_pd_base);
	    (*IpsA_MemoryTable[IpsK_Dealloc]) (src_map_base);
	    (*IpsA_MemoryTable[IpsK_Dealloc]) (remap_base);
	    return (status);
	    }

	/* 
	** Match closest CDF value to each src_map value to obtain 
	** src -> dst mapping (remap)
	*/
	src_map = (float *)src_map_base;
	dst_cdf = (float *)dst_cdf_base;
	remap = (unsigned long *)remap_base;
	for (k = 0, i= 0; i < hist_cnt; i++, src_map++)
	    {
	    dst_cdf = (float *)(dst_cdf_base + k);
	    for (prev_err = 1.0, j = k; j < hist_cnt; j++)
		{
		err = fabs ((double)(*dst_cdf++ - *src_map));
		if (err == 0.0)
		    {
		    j++;
		    break;
		    }
		else if (err > prev_err)
		    break;
		else
		    prev_err = err;
		}
	    *remap++ = j - 1;
	    k = j - 1; 
	    }
	(*IpsA_MemoryTable[IpsK_Dealloc]) (src_map_base);
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

    (*IpsA_MemoryTable[IpsK_Dealloc]) (hist_ptr);
    (*IpsA_MemoryTable[IpsK_Dealloc]) (src_pd_base);
    *lut_ptr = remap_base;
    return (status);
}

/*
**  This subroutine generates a cumulative distribution function for the
**  specified density type (h_type).  The param arguments depend on the
**  density.
*/
long generate_cdf (density_type, param1, param2, 
			    out_ptr, levels, user_pd)
unsigned long density_type;		/* density type			    */
double param1, param2;			/* density parameters		    */
float *out_ptr;				/* ptr to the output distr. func    */
unsigned long levels;			/* number of levels (array elements */
float *user_pd;				/* user density			    */
{
    unsigned long i,j;
    float *target;			/* working pointer		    */
    float *user_buf;
    float fmax, sfactor, sum;
    double z, z2;

    switch (density_type)
	{
	case (IpsK_Gaussian):
	    /* 
	    ** Generate Gaussian Probability Density Function Around specified 
	    ** mean level (param1) with the specified variance (param2)
	    */
	    if (param1 == 0.0) 
		param1 = levels >> 1;
	    if (param2 == 0.0)
		param2 = param1 / 4;
    
	    target = (float *)out_ptr;
	    for (sum = 0, fmax = 0.0, i = 0; i < levels; i++)
		{
		z = (float)i - (float)param1;
		z = -1.0 * z * z;
		z /= (2 * param2 * param2);
		z = exp(z);
		*target = (float)((z * K1) / param2);
		if (*target >= fmax) 
		    fmax = *target;
		    sum += *target++;
		}
	    /*
	    ** Normalize and Calculate Cumulative Distribution Function For the 
	    ** Gaussian probability density (i.e. histogram equalization 
	    ** transfer function)
	    */
	    sfactor = 1 / sum;
	    target = (float *)out_ptr;
	    *target++ *= sfactor;
	    for (i = 1; i < levels; i++, target++)
		*target = *(target- 1) + (*target * sfactor);

	break;

	case (IpsK_UserDensity):

	    if (user_pd == 0)
		return (IpsX_INVDARG);

	    user_buf = (float *)user_pd;
	    for (sum = 0, fmax = 0.0, i = 0; i < levels; i++)
		{
		if (*user_buf >= fmax) 
		    fmax = *user_buf;
		sum += *user_buf++;
		}

	    /*
	    ** Normalize and Calculate Cumulative Distribution Function For the 
	    ** user probability density (i.e. histogram equalization transfer 
	    ** function)
	    */
	    sfactor = 1 / sum;
	    user_buf = (float *)user_pd;
	    target = (float *)out_ptr;
	    *target++ = *user_buf++ * sfactor;
	    for (i = 1; i < levels; i++, target++)
		*target = *(target- 1) + (*user_buf++ * sfactor);

	break;

	case (IpsK_Hyperbolic):
	    /* 
	    ** Generate Hyperbolic Probability Density Function with specified 
	    ** constant (param1)
	    */
	    if (param1 == 0.0)			/* default is 1/4 th */
		param1 = levels >> 2;

	    target = (float *)out_ptr;
	    z2 = 1.0 / param1;		/* use mult rather than div */

	    if (param2 != 1.0)			/* default is neg exp */
		{
		for (sum = 0, fmax = 0.0, i = 0; i < levels; i++)
		    {
		    z = log (1.0 + z2);
		    *target = (float)(1.0 / (((double)i + param1) * z));
		    if (*target >= fmax) 
			fmax = *target;
		    sum += *target++;
		    }
		}
	    else
		{
		j = levels - 1;
		for (sum = 0, fmax = 0.0, i = 0; i < levels; i++, j--)
		    {
		    z = log (1.0 + z2);
		    *target = (float)(1.0 / (((double)j + param1) * z));
		    if (*target >= fmax) 
			fmax = *target;
		    sum += *target++;
		    }
		}
	    /*
	    ** Normalize and Calculate Cumulative Distribution Function For the 
	    ** hyperbolic probability density (i.e. histogram equalization 
	    ** transfer function)
	    */
	    sfactor = 1 / sum;
	    target = (float *)out_ptr;
	    *target++ *= sfactor;
	    for (i = 1; i < levels; i++, target++)
		*target = *(target- 1) + (*target * sfactor);

	break;

	default:
	    return (IpsX_INVDARG);
	break;
	}		    /* end of switch */


    return (IpsX_SUCCESS);
}	


