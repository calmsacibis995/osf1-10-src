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
/************************************************************************
**
**  FACILITY:
**
**      Image Processing Services (IPS)
**
**  ABSTRACT:
**
**      Peforms a convolution with user specified kernel.
**
**  ENVIRONMENT:
**
**      VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
**	Rich Piccolo,  Digital Equipment Corp.
**
**  CREATION DATE:     July 16, 1990
**
**  RESTRICTIONS:
**
**	Executes on byte and float images only
**
************************************************************************/
/*
**  Table of contents:
*/
#ifdef NODAS_PROTO
long	    _IpsConvolve();		    /* Convolution routine	    */
#endif
/*
**  Include files:
*/
#include <IpsDef.h>			    /* IPS Definitions              */
#include <IpsStatusCodes.h>		    /* Status codes		    */
#include <IpsMemoryTable.h>		    /* IPS Memory Mgt. Functions    */
#include <IpsMacros.h>			    /* IPS Macros		    */
#ifndef NODAS_PROTO
#include <ipsprot.h>			    /* Ips prototypes */
#endif
#include <math.h>			    /* math routine definitions     */

/*
**  MACRO definitions:
*/

/*
**  External References:
*/
#ifdef NODAS_PROTO
long _IpsBuildDstUdp();			    /* from Ips__udp_utils.c	    */
#endif

/*
**  Local Storage:
*/

/*
** Kernel Table 
*/

typedef struct 
    {
    float         coef;		/* coefficient				     */
    unsigned long num;		/* no. of times this coef. appears in kernel */
    long *offset;		/* where in src unit offsets they occur      */
    } KERNEL_TABLE;

/*
** Local routines
*/
#ifdef NODAS_PROTO
static long	_IpsBuildKernelTable();	    /* kernel table create          */
static long     _IpsConvolveByte();
static long     _IpsConvolveFloat();
#else
PROTO(static long _IpsConvolveByte, (struct UDP */*src_udp*/, struct UDP */*dst_udp*/, unsigned long /*m*/, unsigned long /*n*/, float */*coef*/, KERNEL_TABLE */*k_tab*/, unsigned char /*k_num*/));
PROTO(static long _IpsConvolveFloat, (struct UDP */*src_udp*/, struct UDP */*dst_udp*/, unsigned long /*m*/, unsigned long /*n*/, float */*coef*/, KERNEL_TABLE */*k_tab*/, unsigned char /*k_num*/));
PROTO(static long _IpsBuildKernelTable, (float */*coef*/, unsigned long /*m*/, unsigned long /*n*/, unsigned long /*src_units*/, KERNEL_TABLE */*k_tab*/, unsigned char */*k_num*/, long */*offsets*/));
#endif

/************************************************************************
**
**  _IpsConvolve()
**
**  FUNCTIONAL DESCRIPTION:
**
**      Performs convolution of a user specified kernel with a src udp. 
**
**  FORMAL PARAMETERS:
**
**	src_udp	         - source UDP
**  	dst_udp	         - destination UDP
**      kernel		 - filter to use 
**	m	         - kernel width
**      n                - kernel height
**      kernel_data_type - datatype of kernel
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
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
************************************************************************/

/***************************************************************************/
/* Restrictions:                                                           */
/*    current restrictions:                                                */
/*                                                                         */
/*    byte image input only						   */
/*                                                                         */
/***************************************************************************/

long _IpsConvolve(src_udp, dst_udp, kernel, m, n, kernel_data_type)
struct UDP *src_udp;                    /* Working src UDP descriptor	    */
struct UDP *dst_udp;                    /* Working dst UDP descriptor	    */
unsigned long *kernel;			/* pointer to kernel		    */
unsigned long m;			/* kernel width			    */
unsigned long n;			/* kernel height		    */
long kernel_data_type;                  /* datatype of kernel contents	    */
{
    KERNEL_TABLE	*k_tab;		/* pointer to kernel decrip tab	    */
    unsigned char	k_num;		/* no of entries in kernel tab	    */
    long		status;		/* status			    */
    unsigned long	num_of_coef;	/* no. of coefficients in ker	    */
    unsigned long	i,j,k;		/* indicies			    */
    unsigned long	src_width_units;/* number of bytes, wrds etc/line   */
    long		*offsets;	/* src offsets for kernel table     */
    unsigned long	mem_alloc = 0;

    /* 
    ** pointers to coefficients and the various kernel data types 
    */

    float		*coef_tmp, *coef;
    unsigned char	*byte_ptr;
    unsigned short  	*word_ptr;
    unsigned long	*long_ptr;
    float		*f_ptr;

/*
** Make sure kernel is odd 
*/
    if (((m & 1) == 0) ||
	((n & 1) == 0))
	    return (IpsX_INVDARG);
/* 
** Check kernel dimensions 
*/
    if ((src_udp->UdpL_PxlPerScn < m) ||
        (src_udp->UdpL_ScnCnt < n)) 
	    return (IpsX_INVDARG);
/*
** Validate source and type and set source units for kernel descriptor table 
*/
    switch (src_udp->UdpB_Class)
        {
        case UdpK_ClassA:
	    /*
	    ** Atomic array
	    */
	    switch (src_udp->UdpB_DType)
		{
		case UdpK_DTypeBU:
		    src_width_units = src_udp->UdpL_ScnStride>>3;
		break;

		case UdpK_DTypeF:
		    src_width_units = src_udp->UdpL_ScnStride>>5;
		break;

		case UdpK_DTypeWU:
		case UdpK_DTypeLU:
		    return(IpsX_UNSOPTION);
		break;

		case UdpK_DTypeVU:
		case UdpK_DTypeV:
		default:
		    return(IpsX_INVDTYPE);
		break;
		}/* end switch on DType */
	break;

	case UdpK_ClassUBA:				
	case UdpK_ClassCL:
	case UdpK_ClassUBS:
	default:
	    return(IpsX_UNSOPTION);
	break;
	}/* end switch on class */

/* 
** allocate 2 working copies of the kernel 
*/
    num_of_coef = m * n;
    coef = (float *) (*IpsA_MemoryTable[IpsK_Alloc])
	(2 * sizeof(float) * num_of_coef, 0, 0);
    if (!coef)
	return (IpsX_INSVIRMEM);

    coef_tmp = coef + num_of_coef;

/*
** Convert kernel to float if not already while copying to working buffer coef
*/
    switch (kernel_data_type)
	{
	case IpsK_DTypeBU:
	    byte_ptr = (unsigned char *) kernel;
	    for (i = 0; i < num_of_coef; i++) *(coef+i) = (float) (*byte_ptr++);
	break;

	case IpsK_DTypeWU:
	    word_ptr = (unsigned short *) kernel;
	    for (i = 0; i < num_of_coef; i++) *(coef+i) = (float) (*word_ptr++);
	break;

	case IpsK_DTypeLU:
	    long_ptr = (unsigned long *) kernel;
	    for (i = 0; i < num_of_coef; i++) *(coef+i) = (float) (*long_ptr++);
	break;

	case IpsK_DTypeF:
	    f_ptr = (float *) kernel;
	    for (i = 0; i < num_of_coef; i++) *(coef+i) = (float) *f_ptr++;
	break;

	default:
	    (*IpsA_MemoryTable[IpsK_Dealloc])(coef);
	    return (IpsX_INVDARG);
	break;
	}
/* 
** another copy of kernel to coef_tmp to be used by BuildKernelTable
*/
    for (i = 0; i < num_of_coef; i++) *(coef_tmp + i) = *(coef + i);

/* 
** allocate a kernel table entry per coefficient 
*/
    k_tab = (KERNEL_TABLE *) (*IpsA_MemoryTable[IpsK_Alloc])(
	    (sizeof (KERNEL_TABLE) * num_of_coef), 0, 0);
    if (!k_tab)
	{
	(*IpsA_MemoryTable[IpsK_Dealloc])(coef);
	return (IpsX_INSVIRMEM);
	}

/* 
** allocate enough offsets within the table 
*/
    offsets = (long *) (*IpsA_MemoryTable[IpsK_Alloc])(
    	(sizeof (long) * num_of_coef), 0, 0);
    if (!offsets)
	{
	(*IpsA_MemoryTable[IpsK_Dealloc])(coef);
	(*IpsA_MemoryTable[IpsK_Dealloc])(k_tab);
	return (IpsX_INSVIRMEM);
	}

/* 
** build kernel table descriptor 
*/
    status = _IpsBuildKernelTable (coef_tmp, m, n, src_width_units,
	k_tab, &k_num, offsets);
    if (status != IpsX_SUCCESS)
	{
        (*IpsA_MemoryTable[IpsK_Dealloc])(offsets);
        (*IpsA_MemoryTable[IpsK_Dealloc])(k_tab);
        (*IpsA_MemoryTable[IpsK_Dealloc])(coef);
        return (status );
	}
/* 
** create dst udp 
*/
    if (dst_udp->UdpA_Base == 0)
	mem_alloc = 1;
    status = _IpsBuildDstUdp (src_udp, dst_udp, 0, 0, 0); 
    if (status != IpsX_SUCCESS) 
	{
        (*IpsA_MemoryTable[IpsK_Dealloc])(offsets);
        (*IpsA_MemoryTable[IpsK_Dealloc])(k_tab);
        (*IpsA_MemoryTable[IpsK_Dealloc])(coef);
	return (status);
	}

    switch (src_udp->UdpB_DType)
	{
	case UdpK_DTypeBU:
	    status = _IpsConvolveByte 
		(src_udp, dst_udp, m, n, coef, k_tab, k_num);
	break;

	case UdpK_DTypeF:
	    status = _IpsConvolveFloat
		(src_udp, dst_udp, m, n, coef, k_tab, k_num);
	break;

	default:
	    return(IpsX_INCNSARG);
	break;
	}/* end switch on DType */

    (*IpsA_MemoryTable[IpsK_Dealloc])(offsets);
    (*IpsA_MemoryTable[IpsK_Dealloc])(k_tab);
    (*IpsA_MemoryTable[IpsK_Dealloc])(coef);
    if ((status != IpsX_SUCCESS) && (mem_alloc == 1))
	(*IpsA_MemoryTable[IpsK_FreeDataPlane]) (dst_udp->UdpA_Base);
    return (status);
}

static long _IpsConvolveByte (
    struct UDP *    src_udp,            /* Working src UDP descriptor	     */
    struct UDP *    dst_udp,            /* Working dst UDP descriptor	     */
    unsigned long   m,			/* kernel width			     */
    unsigned long   n,			/* kernel height		     */
    float *	    coef,		/* coefficients			     */
    KERNEL_TABLE *  k_tab,		/* kernel descriptor file	     */
    unsigned char   k_num		/* no of entries in kernel table     */
    )
    {
    unsigned long	ix, iy, ker, k;		/* loop indices		     */
    unsigned char	*src_byte_ptr;		
    unsigned char	*dst_byte_ptr;
    unsigned long	src_pad, dst_pad;
    unsigned char	mid_x, mid_y;		/* half of kernel size	     */
    unsigned long       src_stride_in_bytes;
    float		add_val;
    float		result;
    long		new_x, new_y;
    long		kw, kh;
/* 
** convolve src to dst - remember to keep away from edges, will special
** case later
*/
/* 
** mid_x is used to indicate number of columns on each side of kernel center 
** and mid_y is used to indicate number of rows on each side of kernel center
*/
    mid_x = m / 2;
    mid_y = n / 2;

/* 
** Get to the begininng of the ROI
** and move over mid_x and mid_y 
*/
    src_byte_ptr = (unsigned char *) 
	src_udp->UdpA_Base + (src_udp->UdpL_Pos>>3) + (src_udp->UdpL_Y1 *
	    (src_udp->UdpL_ScnStride>>3)) + src_udp->UdpL_X1;
    src_byte_ptr += ((src_udp->UdpL_ScnStride >> 3) * mid_y) + mid_x;

    dst_byte_ptr = (unsigned char *) dst_udp->UdpA_Base + 
	(dst_udp->UdpL_Pos>>3) + ((dst_udp->UdpL_ScnStride >> 3) * mid_y) + 
	    mid_x;

    src_pad = (src_udp->UdpL_ScnStride >> 3) - src_udp->UdpL_PxlPerScn + 
	(2 * mid_x);
    dst_pad = 2 * mid_x;

    for (iy = mid_y ; iy < src_udp->UdpL_ScnCnt - mid_y; iy++)
	{
	for (ix = mid_x; ix < src_udp->UdpL_PxlPerScn - mid_x; ix++)
	    {
	    for (result = 0.0, ker = 0; ker < k_num; ker++)
		{
		for (add_val = 0, k = 0; k < k_tab[ker].num; k++)
		    add_val += (float)(*(src_byte_ptr + k_tab[ker].offset[k]));
		result += (float) (add_val * k_tab[ker].coef);
		}
	    src_byte_ptr++;
	    if (result >= 255.5)
		*dst_byte_ptr++ = 255;
	    else if (result < 0.0) 
		*dst_byte_ptr++ = 0;
	    else
		*dst_byte_ptr++ = (unsigned char) (result + 0.5);
	    }
	src_byte_ptr += src_pad;
	dst_byte_ptr += dst_pad;
	}

/* do edges for byte images */

    src_stride_in_bytes = src_udp->UdpL_ScnStride>>3;

/* 
** top leftover lines 
*/
    dst_byte_ptr = (unsigned char *) dst_udp->UdpA_Base + 
	(dst_udp->UdpL_Pos>>3);

    for (iy = src_udp->UdpL_Y1; iy < src_udp->UdpL_Y1 + mid_y; iy++)
	{
	for (ix = src_udp->UdpL_X1; ix <= src_udp->UdpL_X2; ix++)
	    {
	    result = 0;		    
	    for (kh = 0; kh < n; kh++)
		for (kw = 0; kw < m; kw++)
		    {
		    new_x = ix + kw - mid_x;
		    new_y = iy + kh - mid_y;
		    if ((new_x >= src_udp->UdpL_X1) &&
		        (new_x <= src_udp->UdpL_X2) &&				
		        (new_y >= src_udp->UdpL_Y1))				
			    result += (float)((*(coef + ((kh * m) + kw))) * 
				(float) (*(src_udp->UdpA_Base + 
				    (src_udp->UdpL_Pos>>3) + (new_y * 
					src_stride_in_bytes) + new_x)));
		    }	/* end of kernel */
		if (result >= 255.5)
		    *dst_byte_ptr++ = 255;
		else if (result < 0.0) 
		    *dst_byte_ptr++ = 0;
		else
		    *dst_byte_ptr++ = (unsigned char) (result + 0.5);
	    } /* end of ix loop */
	}     /* end of iy loop */

/* 
** bottom leftover lines 
*/
    dst_byte_ptr = (unsigned char *) (dst_udp->UdpA_Base + 
	(dst_udp->UdpL_Pos>>3) + (dst_udp->UdpL_PxlPerScn * 
	    (dst_udp->UdpL_ScnCnt - mid_y)));

    for (iy = src_udp->UdpL_Y2 - mid_y + 1; iy <= src_udp->UdpL_Y2; iy++)
	{
	for (ix = src_udp->UdpL_X1; ix <= src_udp->UdpL_X2; ix++)
	    {
	    result = 0;		    
	    for (kh = 0; kh < n; kh++)
		for (kw = 0; kw < m; kw++)
		    {
		    new_x = ix + kw - mid_x;
		    new_y = iy + kh - mid_y;
		    if ((new_x >= src_udp->UdpL_X1) &&
		        (new_x <= src_udp->UdpL_X2) &&				
		        (new_y <= src_udp->UdpL_Y2))
			    result += (float)((*(coef + ((kh * m) + kw))) * 
				(float) (*(src_udp->UdpA_Base + 
				    (src_udp->UdpL_Pos>>3) + (new_y * 
					src_stride_in_bytes) + new_x)));
		    }	/* end of kernel */
		if (result >= 255.5)
		    *dst_byte_ptr++ = 255;
		else if (result < 0.0) 
		    *dst_byte_ptr++ = 0;
		else
		    *dst_byte_ptr++ = (unsigned char) (result + 0.5);
	    } /* end of ix loop */
	}     /* end of iy loop */

/* 
** leftmost and rightmost pixels 
*/
    for (iy = mid_y + src_udp->UdpL_Y1; iy <= src_udp->UdpL_Y2 - mid_y; iy++)
	{
	dst_byte_ptr = (unsigned char *) (dst_udp->UdpA_Base + 
	    (dst_udp->UdpL_Pos>>3) + ((iy - src_udp->UdpL_Y1) * 
		dst_udp->UdpL_PxlPerScn));
	/* leftmost */
	for (ix = src_udp->UdpL_X1; ix < src_udp->UdpL_X1 + mid_x; ix++) 
	    {
	    result = 0;		    
	    for (kh = 0; kh < n; kh++)
		for (kw = 0; kw < m; kw++)
		    {
		    new_x = ix + kw - mid_x;
		    new_y = iy + kh - mid_y;
		    if (new_x >= src_udp->UdpL_X1)
			    result += (float)((*(coef + ((kh * m) + kw))) * 
				(float) (*(src_udp->UdpA_Base + 
				    (src_udp->UdpL_Pos>>3) + (new_y * 
					src_stride_in_bytes) + new_x)));
		    }	/* end of kernel */
		if (result >= 255.5)
		    *dst_byte_ptr++ = 255;
		else if (result < 0.0) 
		    *dst_byte_ptr++ = 0;
		else
		    *dst_byte_ptr++ = (unsigned char) (result + 0.5);
	    } /* end of leftmost ix loop */

	dst_byte_ptr = (unsigned char *) (dst_udp->UdpA_Base + 
	    (dst_udp->UdpL_Pos>>3) + ((iy - src_udp->UdpL_Y1) * 
		dst_udp->UdpL_PxlPerScn) + (dst_udp->UdpL_PxlPerScn - mid_x));

	/* rightmost */
	for (ix = src_udp->UdpL_X2 - mid_x + 1; ix <= src_udp->UdpL_X2; ix++)
	    {
	    result = 0;		    
	    for (kh = 0; kh < n; kh++)
		for (kw = 0; kw < m; kw++)
		    {
		    new_x = ix + kw - mid_x;
		    new_y = iy + kh - mid_y;
		    if ((new_x <= src_udp->UdpL_X2))
			    result += (float)((*(coef + ((kh * m) + kw))) * 
				(float) (*(src_udp->UdpA_Base + 
				    (src_udp->UdpL_Pos>>3) + (new_y * 
					src_stride_in_bytes) + new_x)));
		    }	/* end of kernel */
		if (result >= 255.5)
		    *dst_byte_ptr++ = 255;
		else if (result < 0.0) 
		    *dst_byte_ptr++ = 0;
		else
		    *dst_byte_ptr++ = (unsigned char) (result + 0.5);
	    } /* end of rightmost ix loop */
	}     /* end of iy loop */

    return (IpsX_SUCCESS);

}/* end _IpsConvolvByte*/


static long _IpsConvolveFloat (
    struct UDP *    src_udp,            /* Working src UDP descriptor	     */
    struct UDP *    dst_udp,            /* Working dst UDP descriptor	     */
    unsigned long   m,			/* kernel width			     */
    unsigned long   n,			/* kernel height		     */
    float *	    coef,		/* coefficients			     */
    KERNEL_TABLE *  k_tab,		/* kernel descriptor table	     */
    unsigned char   k_num		/* no of entries in kernel tab       */
    )
    {
    unsigned long	ix, iy, ker, k;
    float		*src_float_ptr;
    float		*dst_float_ptr;
    unsigned long	src_pad, dst_pad;
    unsigned char	mid_x, mid_y;	/* half of kernel size		     */
    unsigned long       src_stride_in_longs;
    float		add_val;
    long		new_x, new_y;
    long		kw, kh;
/* 
** convolve src to dst - remember to keep away from edges, will special
** case later
*/
/* 
** mid_x is used to indicate number of columns on each side of kernel center 
** and mid_y is used to indicate number of rows on each side of kernel center
*/
    mid_x = m / 2;
    mid_y = n / 2;

    src_float_ptr = (float *) 
	src_udp->UdpA_Base + (src_udp->UdpL_Pos>>5) + (src_udp->UdpL_Y1 * 
	    (src_udp->UdpL_ScnStride>>5)) + src_udp->UdpL_X1;
    src_float_ptr += ((src_udp->UdpL_ScnStride >> 5) * mid_y) + mid_x;

    dst_float_ptr = (float *) dst_udp->UdpA_Base + 
	(dst_udp->UdpL_Pos>>5) + ((dst_udp->UdpL_ScnStride >> 5) * mid_y) + 
	    mid_x;

    src_pad = (src_udp->UdpL_ScnStride >> 5) - src_udp->UdpL_PxlPerScn + 
	(2 * mid_x);
    dst_pad = 2 * mid_x;

    for (iy = mid_y ; iy < src_udp->UdpL_ScnCnt - mid_y; iy++)
	{
	for (ix = mid_x; ix < src_udp->UdpL_PxlPerScn - mid_x; ix++)
	    {
	    for (*dst_float_ptr = 0.0, ker = 0; ker < k_num; ker++)
		{
		for (add_val = 0.0, k = 0; k < k_tab[ker].num; k++)
		    add_val += *(src_float_ptr + k_tab[ker].offset[k]);	    
		*dst_float_ptr += add_val * k_tab[ker].coef;
		}
	    src_float_ptr++;
	    dst_float_ptr++;
	    }
	src_float_ptr += src_pad;
	dst_float_ptr += dst_pad;
	}

/* do edges for float */

    src_stride_in_longs = src_udp->UdpL_ScnStride>>5;

/* 
** top leftover lines 
*/
    dst_float_ptr = (float *) dst_udp->UdpA_Base + (dst_udp->UdpL_Pos>>5);

    for (iy = src_udp->UdpL_Y1; iy < src_udp->UdpL_Y1 + mid_y; iy++)
	{
	for (ix = src_udp->UdpL_X1; ix <= src_udp->UdpL_X2; ix++)
	    {
	    *dst_float_ptr = 0;		    
	    for (kh = 0; kh < n; kh++)
		for (kw = 0; kw < m; kw++)
		    {
		    new_x = ix + kw - mid_x;
		    new_y = iy + kh - mid_y;
		    if ((new_x >= src_udp->UdpL_X1) &&
		        (new_x <= src_udp->UdpL_X2) &&				
		        (new_y >= src_udp->UdpL_Y1))				
			    *dst_float_ptr += (*(coef + ((kh * m) + kw)) * 
				*((float *)src_udp->UdpA_Base + 
				    (src_udp->UdpL_Pos>>5) + (new_y * 
					src_stride_in_longs) + new_x));
		    }	/* end of kernel */
	    dst_float_ptr++;
	    } /* end of ix loop */
	}     /* end of iy loop */

/* 
** bottom leftover lines 
*/
    dst_float_ptr = (float *) dst_udp->UdpA_Base + 
	(dst_udp->UdpL_Pos>>5) + (dst_udp->UdpL_PxlPerScn * 
	    (dst_udp->UdpL_ScnCnt - mid_y));

    for (iy = src_udp->UdpL_Y2 - mid_y + 1; iy <= src_udp->UdpL_Y2; iy++)
	{
	for (ix = src_udp->UdpL_X1; ix <= src_udp->UdpL_X2; ix++)
	    {
	    *dst_float_ptr = 0.0;		    
	    for (kh = 0; kh < n; kh++)
		for (kw = 0; kw < m; kw++)
		    {
		    new_x = ix + kw - mid_x;
		    new_y = iy + kh - mid_y;
		    if ((new_x >= src_udp->UdpL_X1) &&
		        (new_x <= src_udp->UdpL_X2) &&				
		        (new_y <= src_udp->UdpL_Y2))
			    *dst_float_ptr += (*(coef + ((kh * m) + kw)) * 
				*((float *)src_udp->UdpA_Base + 
				    (src_udp->UdpL_Pos>>5) + (new_y * 
					src_stride_in_longs) + new_x));
		    }	/* end of kernel */
	    dst_float_ptr++;
	    } /* end of ix loop */
	}     /* end of iy loop */

/* 
** leftmost and rightmost pixels 
*/
    for (iy = mid_y + src_udp->UdpL_Y1; iy <= src_udp->UdpL_Y2 - mid_y; iy++)
	{
	dst_float_ptr = (float *) dst_udp->UdpA_Base + 
	    (dst_udp->UdpL_Pos>>5) + ((iy - src_udp->UdpL_Y1) * 
		dst_udp->UdpL_PxlPerScn);
	/* leftmost */
	for (ix = src_udp->UdpL_X1; ix < src_udp->UdpL_X1 + mid_x; ix++) 
	    {
	    *dst_float_ptr = 0.0;		    
	    for (kh = 0; kh < n; kh++)
		for (kw = 0; kw < m; kw++)
		    {
		    new_x = ix + kw - mid_x;
		    new_y = iy + kh - mid_y;
		    if (new_x >= src_udp->UdpL_X1)
			    *dst_float_ptr += (*(coef + ((kh * m) + kw)) * 
				*((float *)src_udp->UdpA_Base + 
				    (src_udp->UdpL_Pos>>5) + (new_y * 
					src_stride_in_longs) + new_x));
		    }	/* end of kernel */
	    dst_float_ptr++;
	    } /* end of leftmost ix loop */

	dst_float_ptr = (float *) dst_udp->UdpA_Base + 
	    (dst_udp->UdpL_Pos>>5) + ((iy - src_udp->UdpL_Y1) * 
		dst_udp->UdpL_PxlPerScn) + (dst_udp->UdpL_PxlPerScn - mid_x);

	/* rightmost */
	for (ix = src_udp->UdpL_X2 - mid_x + 1; ix <= src_udp->UdpL_X2; ix++)
	    {
	    *dst_float_ptr = 0.0;		    
	    for (kh = 0; kh < n; kh++)
		for (kw = 0; kw < m; kw++)
		    {
		    new_x = ix + kw - mid_x;
		    new_y = iy + kh - mid_y;
		    if ((new_x <= src_udp->UdpL_X2))
			    *dst_float_ptr += (*(coef + ((kh * m) + kw)) * 
				*((float *)src_udp->UdpA_Base + 
				    (src_udp->UdpL_Pos>>5) + (new_y * 
					src_stride_in_longs) + new_x));
		    }	/* end of kernel */
	    dst_float_ptr++;
	    } /* end of rightmost ix loop */
	}     /* end of iy loop */

    return (IpsX_SUCCESS);

}/* end _IpsConvolvFloat*/


/************************************************************************
**
**  DESCRIPTION:
**
**  This module defines the entries for the kernel descriptor table.
**
**  The structure of the table is:
**
**  - kernel coefficient
**  - number of times the kernel coefficient appears in the kernel
**  - the offsets in src_udp units relative to the center of the kernel
**    Note: allocated area of offsets is passed in via "offsets"
**
**  Example:
**
**  Assume 10 x 10 byte image as source.  Assume 3 x 3 kernel as follows:
**
**	    0 -1  0
**	   -1  9 -1
**	    0 -1  0
**
**  The table would be the following:
**
**  k_tab[0].coef =	 -1.0	 kernel coefficient 
**  k_tab[0].num  =	  4	 number of times this coefficient appears 
**  k_tab[0].offset[0] = -10	 1st offset (in bytes) from calc pix 
**  k_tab[0].offset[1] =  -1	 2nd offset (in bytes) from calc pix 
**  k_tab[0].offset[2] =   1	 3rd offset (in bytes) from calc pix 
**  k_tab[0].offset[3] =  10	 4th offset (in bytes) from calc pix 
**  k_tab[1].coef =	  9.0	 next kernel coefficient 
**  k_tab[0].num  =	  1	 number of times this coefficient appears 
**  k_tab[0].offset[0] =  0	 offset (in bytes) from calc pix 
**
**  k_num = # of entries in table = 2
**
*****************************************************************************/

static long _IpsBuildKernelTable (coef, m, n, src_units, k_tab, k_num, offsets)
float		    *coef;		    /* ptr to kernel		      */
unsigned long	    m,n;		    /* Kernel dimensions	      */
unsigned long	    src_units;		    /* number of src units offsets    */
KERNEL_TABLE	    *k_tab;
unsigned char	    *k_num;		    /* no of entries in kernel tab    */
long		    *offsets;		    /* offset table		      */
    {
    unsigned long	i,j,k,num;
    unsigned long	x,y;		    /* x,y kernel position u.l. based */
    unsigned long	mid_x, mid_y;	    /* middle location of kernel      */
    unsigned long	num_of_coef;
    float		z;
    unsigned char k_cnt;		/* no of entries in kernel tab  */

    mid_x = m / 2;
    mid_y = n / 2;
    num_of_coef = m * n;

    k_cnt = 0;
    k_tab[k_cnt].num = 0;
    k_tab[k_cnt].offset = offsets;		/* point to top of offset tab */

    for (i = 0; i < num_of_coef; i++)
	{
	if ((z = *(coef+i)) != 0.0)	       /* if zero, throw out	    */
	    {
	    num = 0;			       /* initialize coef count	    */
	    k_tab[k_cnt].coef = z;	       /* set coefficient in table  */
    
	    /* 
	    ** get x, y position in kernel relative to upper left being (0,0) 
	    ** (i.e. udp based coordinates)
	    */
	    x = i % m;
	    y = (i - x) / m;

	    k_tab[k_cnt].offset[k_tab[k_cnt].num] = 
		((y-mid_y) * src_units) + (x - mid_x);
	    offsets++;		    /* bump offset table */
	    k_tab[k_cnt].num++;	    /* incr num of times coef appears */

	    for (j = i+1; j < num_of_coef; j++)
		{
		if (*(coef+j) == z)
		    {
		    *(coef+j) = 0.0;	    /* zero this to signal its use    */
		    x = j % m;
		    y = (j-x) / m;
		    k_tab[k_cnt].offset[k_tab[k_cnt].num] = 
			((y-mid_y) * src_units) + (x - mid_x);
		    offsets++;		    /* bump offset table */
		    k_tab[k_cnt].num++;	    /* incr num of times coef appears */
		    }
		}
	    k_cnt++;			    /* incr no. of entries	      */
	    k_tab[k_cnt].num = 0;
	    k_tab[k_cnt].offset = offsets;
	    }				    /* end if */
	}

    *k_num = k_cnt;
    return (IpsX_SUCCESS);	/* end of BuildKernelTable */
}
