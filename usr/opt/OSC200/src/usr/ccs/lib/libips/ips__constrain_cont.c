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
**      _IpsConstrainCont creates dst udp with the specified data type
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
**      Oct 1, 1990
**
************************************************************************/

/*
**  Table of contents
*/
#ifdef NODAS_PROTO
long	    _IpsConstrainCont();
static long  IpsScaleData();
#endif

/*
** Include files
*/
#include <IpsDef.h>			    /* IPS definitions		    */
#include <IpsStatusCodes.h>		    /* IPS Status Codes		    */
#include <IpsMemoryTable.h>		    /* IPS Memory Vector Table      */
#ifndef NODAS_PROTO
#include <ipsprot.h>			    /* Ips prototypes */
#endif

/*
** External References
*/
#ifdef NODAS_PROTO
long _IpsVerifyNotInPlace();		    /* from Ips__udp_utils	     */
long _IpsPointStatistics();		    /* from ips__point_statisitcs.c  */
long _IpsConstantArithmetic();		    /* from ips__constant_arithmetic */
#endif

/******************************************************************************
**
**  FUNCTIONAL DESCRIPTION:
**
**	This function promotes a continuous tone udp to the specified data type
**
**
**  FORMAL PARAMETERS:
**
**	src_udp		src_udp
**	dst_udp		resultant udp
**	dst_dtype	desired data type
**	flags		options to define how to constrain
**			IpsM_ScaleIfNecessary	-> scale data 
**			IpsM_ClipToFit		-> clip to fit data
**			IpsM_ScaleToFit		-> always scale
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	status
**
******************************************************************************/

long _IpsConstrainCont (src_udp, dst_udp, dst_dtype, flags)
struct UDP	*src_udp;
struct UDP	*dst_udp;
unsigned long	dst_dtype;
unsigned long	flags;
{
    unsigned char	*src_byte_ptr, *dst_byte_ptr;
    unsigned short	*src_word_ptr, *dst_word_ptr;
    unsigned int	*src_long_ptr, *dst_long_ptr;
    float		*src_float_ptr,*dst_float_ptr;
    unsigned long	src_line_pad;
    unsigned long	size;
    unsigned char	mem_alloc = 0;
    unsigned long	line_cnt;	    /* number of scanlines	    */
    unsigned long	pixel_cnt;	    /* number of pixels/scanline    */
    unsigned long	ix,iy;		    /* pixel loop index		    */
    long		status;
    struct UDP		tmp_udp;
    long i;

    switch (src_udp->UdpB_Class)
	{
	case UdpK_ClassA:
	    /*
	    ** Atomic array
	    */
	    switch (src_udp->UdpB_DType)
		{
	    	case UdpK_DTypeWU:
		case UdpK_DTypeLU:
		case UdpK_DTypeF:
		break;
		case UdpK_DTypeBU:
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

    /* Check dtype to make sure we are really constraining */
    /* Note: data types are ordered */

    if (dst_dtype >= src_udp->UdpB_DType)
	return (IpsX_INVDTYPE);

    /* 
    ** build dst udp (no retain capability) and copy data 
    */
    dst_udp->UdpB_DType = dst_dtype;
    dst_udp->UdpB_Class = src_udp->UdpB_Class;
    dst_udp->UdpL_PxlPerScn = src_udp->UdpL_PxlPerScn;
    dst_udp->UdpL_ScnCnt = src_udp->UdpL_ScnCnt;
    dst_udp->UdpL_X1 = 0;
    dst_udp->UdpL_Y1 = 0;
    dst_udp->UdpL_X2 = dst_udp->UdpL_PxlPerScn - 1;
    dst_udp->UdpL_Y2 = dst_udp->UdpL_ScnCnt - 1;
    dst_udp->UdpL_CompIdx = src_udp->UdpL_CompIdx;

    switch (dst_dtype)
	{
	case IpsK_DTypeBU:
	    dst_udp->UdpW_PixelLength = 8;
	    dst_udp->UdpL_Levels = 256;
	break;
	case IpsK_DTypeWU:
	    dst_udp->UdpW_PixelLength = 16;
	    dst_udp->UdpL_Levels = 65536;
	break;
	case IpsK_DTypeLU:
	    dst_udp->UdpW_PixelLength = 32;
	    dst_udp->UdpL_Levels = 0xFFFFFFFF;
	break;
	default:
	    return (IpsX_INVDTYPE);
	break;
	}

    dst_udp->UdpL_PxlStride = dst_udp->UdpW_PixelLength;
    dst_udp->UdpL_ScnStride = dst_udp->UdpL_PxlPerScn * dst_udp->UdpL_PxlStride;
    size = dst_udp->UdpL_ScnStride * dst_udp->UdpL_ScnCnt;

    if (dst_udp->UdpA_Base != 0)
	{
	/* 
	** buffer declared by user - in place is not allowed - 
	** make sure its not in place 
	*/
	status = _IpsVerifyNotInPlace (src_udp, dst_udp);
	    if (status != IpsX_SUCCESS) return (status);
	/* check if big enough */
	if ((dst_udp->UdpL_ArSize - dst_udp->UdpL_Pos) < 
	    (dst_udp->UdpL_ScnStride * dst_udp->UdpL_ScnCnt))
		return (IpsX_INSVIRMEM);
	}
    else
	{
	dst_udp->UdpA_Base = (unsigned char *)
	    (*IpsA_MemoryTable[IpsK_AllocateDataPlane])
		(((size + 7) >> 3),0,0);
	if (!dst_udp->UdpA_Base) return (IpsX_INSVIRMEM);
	dst_udp->UdpL_ArSize = size;
	dst_udp->UdpL_Pos = 0;
	mem_alloc = 1;
	}

	line_cnt = dst_udp->UdpL_ScnCnt;
	pixel_cnt = dst_udp->UdpL_PxlPerScn;

/* src switch */
	switch (src_udp->UdpB_DType)	    
	    {
	    case UdpK_DTypeF:		    /* float src */
		src_float_ptr = (float *)src_udp->UdpA_Base + 
		    (src_udp->UdpL_Pos>>5) + (src_udp->UdpL_ScnStride>>5) *
			src_udp->UdpL_Y1 + src_udp->UdpL_X1;
		src_line_pad = (src_udp->UdpL_ScnStride>>5) - 
		    src_udp->UdpL_PxlPerScn;
		
		switch (dst_udp->UdpB_DType)
		    {
		    case UdpK_DTypeLU:

    /* src = float, dst = long */
				    
		    /* long output */
		    dst_long_ptr = (unsigned int *)dst_udp->UdpA_Base;
		    if (flags & IpsM_ClipToFit)
			{
			for (iy = 0; iy < line_cnt; iy++)
			    {
			    for (ix = 0; ix < pixel_cnt; ix++)
				{
				if (*src_float_ptr < 0.0) 
				    *dst_long_ptr++ = 0;
				else if (*src_float_ptr >= dst_udp->UdpL_Levels)
				    *dst_long_ptr++ = 0xffffffff;
				else 
				    *dst_long_ptr++ = (unsigned int)
					    *src_float_ptr;
				src_float_ptr++;
				}
			    src_float_ptr += src_line_pad; 
			    }/* end scanline loop */
			}
		    else 
			{
			tmp_udp.UdpA_Base = 0;
			status = IpsScaleData 
			    (src_udp, &tmp_udp, dst_udp->UdpL_Levels, flags);
			if (status != IpsX_SUCCESS)
			    {
			    if (mem_alloc == 1)
				(*IpsA_MemoryTable[IpsK_FreeDataPlane]) 
				    (dst_udp->UdpA_Base);
			    if (tmp_udp.UdpA_Base != 0)
				(*IpsA_MemoryTable[IpsK_FreeDataPlane]) 
				    (tmp_udp.UdpA_Base);
			    return (status);
			    }

			/* set to tmp if scaledata did anything */
			if (tmp_udp.UdpA_Base != 0)
			    {
			    src_float_ptr = (float *) (tmp_udp.UdpA_Base);
			    src_line_pad = 0;
			    }

			for (iy = 0; iy < line_cnt; iy++)
			    {
			    for (ix = 0; ix < pixel_cnt; ix++)
			     *dst_long_ptr++ = (unsigned int)*src_float_ptr++;
			    src_float_ptr += src_line_pad;  
			    }/* end scanline loop */
			if (tmp_udp.UdpA_Base != 0)
			    (*IpsA_MemoryTable[IpsK_FreeDataPlane]) 
			        (tmp_udp.UdpA_Base);
  			}
		    break;	    /* float src - long dst */

		    case UdpK_DTypeWU:

    /* src = float, dst = word */
		    /* word output */
		    dst_word_ptr = (unsigned short*)dst_udp->UdpA_Base + 
			(dst_udp->UdpL_Pos>>5);
		    if (flags & IpsM_ClipToFit)
			{
			for (iy = 0; iy < line_cnt; iy++)
			    {
			    for (ix = 0; ix < pixel_cnt; ix++)
				{
				if (*src_float_ptr < 0.0)
				    *dst_word_ptr++ = 0;
				else if 
				    (*src_float_ptr >= dst_udp->UdpL_Levels)
					*dst_word_ptr++ = 0xffff;
				else 
				    *dst_word_ptr++ = (unsigned short)
					*src_float_ptr;
				src_float_ptr++;
				}
			    src_float_ptr += src_line_pad;  
			    }/* end scanline loop */
			}
		    else 
			{
			tmp_udp.UdpA_Base = 0;
			status = IpsScaleData 
			    (src_udp, &tmp_udp, dst_udp->UdpL_Levels, flags);
			if (status != IpsX_SUCCESS)
			    {
			    if (mem_alloc == 1)
				(*IpsA_MemoryTable[IpsK_FreeDataPlane]) 
				    (dst_udp->UdpA_Base);
			    if (tmp_udp.UdpA_Base != 0)
				(*IpsA_MemoryTable[IpsK_FreeDataPlane]) 
				    (tmp_udp.UdpA_Base);
			    return (status);
			    }

			/* set to tmp if scaledata did anything */
			if (tmp_udp.UdpA_Base != 0)
			    {
			    src_float_ptr = (float *) (tmp_udp.UdpA_Base);
			    src_line_pad = 0;
			    }
			for (iy = 0; iy < line_cnt; iy++)
			    {
			    for (ix = 0; ix < pixel_cnt; ix++)
				*dst_word_ptr++ = (unsigned short ) 
				    *src_float_ptr++;
			    src_float_ptr += src_line_pad;  
			    }/* end scanline loop */
			if (tmp_udp.UdpA_Base != 0)
			    (*IpsA_MemoryTable[IpsK_FreeDataPlane]) 
			    (tmp_udp.UdpA_Base);
  			}
		    break;	    /* float src - word dst */

		    case UdpK_DTypeBU:

    /* src = float, dst = byte */
		    /* byte output */
		    dst_byte_ptr = (unsigned char *)dst_udp->UdpA_Base + 
			(dst_udp->UdpL_Pos>>3);
		    if (flags & IpsM_ClipToFit)
			{
			for (iy = 0; iy < line_cnt; iy++)
			    {
			    for (ix = 0; ix < pixel_cnt; ix++)
				{
				if (*src_float_ptr < 0.0)
				    *dst_byte_ptr++ = 0;
				else if 
				    (*src_float_ptr >= dst_udp->UdpL_Levels)
					*dst_byte_ptr++ = 0xff;
				else 
				    *dst_byte_ptr++ = (unsigned char)
					*src_float_ptr;
				src_float_ptr++;
				}
			    src_float_ptr += src_line_pad;  
			    }/* end scanline loop */
			}
		    else 
			{
			tmp_udp.UdpA_Base = 0;
			status = IpsScaleData 
			    (src_udp, &tmp_udp, dst_udp->UdpL_Levels, flags);
			if (status != IpsX_SUCCESS)
			    {
			    if (mem_alloc == 1)
				(*IpsA_MemoryTable[IpsK_FreeDataPlane]) 
				    (dst_udp->UdpA_Base);
			    if (tmp_udp.UdpA_Base != 0)
				(*IpsA_MemoryTable[IpsK_FreeDataPlane]) 
				    (tmp_udp.UdpA_Base);
			    return (status);
			    }

			/* set to tmp if scaledata did anything */
			if (tmp_udp.UdpA_Base != 0)
			    {
			    src_float_ptr = (float *) (tmp_udp.UdpA_Base);
			    src_line_pad = 0;
			    }
			for (iy = 0; iy < line_cnt; iy++)
			    {
			    for (ix = 0; ix < pixel_cnt; ix++)
				*dst_byte_ptr++ = (unsigned char)
				    *src_float_ptr++;
			    src_float_ptr += src_line_pad;  
			    }/* end scanline loop */
			if (tmp_udp.UdpA_Base != 0)
			    (*IpsA_MemoryTable[IpsK_FreeDataPlane]) 
			    (tmp_udp.UdpA_Base);
  			}
		    break;	    /* float src - byte dst */
		    }		    /* end of dst switch    */
		break;	/* end of float src */

	    case UdpK_DTypeLU:		    /* long src */
		src_long_ptr = (unsigned int *)src_udp->UdpA_Base + 
		    (src_udp->UdpL_Pos>>5) + (src_udp->UdpL_ScnStride>>5) *
			src_udp->UdpL_Y1 + src_udp->UdpL_X1;
		src_line_pad = (src_udp->UdpL_ScnStride>>5) - 
		    src_udp->UdpL_PxlPerScn;
		
		switch (dst_udp->UdpB_DType)
		    {

    /* src = long , dst = short */
		    case UdpK_DTypeWU:
		    /* word output */
		    dst_word_ptr = (unsigned short*)dst_udp->UdpA_Base + 
			(dst_udp->UdpL_Pos>>4);

		    if (src_udp->UdpL_Levels <= dst_udp->UdpL_Levels)
		        {
			/* 
			** set dst quant levels to src's quant levels 
			*/
			dst_udp->UdpL_Levels = src_udp->UdpL_Levels;
			for (iy = 0; iy < line_cnt; iy++)
			    {
			    for (ix = 0; ix < pixel_cnt; ix++)
				    *dst_word_ptr++ = (unsigned short)
					*src_long_ptr++;
			    src_long_ptr += src_line_pad;  
			    }/* end scanline loop */
			}
		    else if (flags & IpsM_ClipToFit)
		        {
			for (iy = 0; iy < line_cnt; iy++)
			    {
			    for (ix = 0; ix < pixel_cnt; ix++)
				{
				if (*src_long_ptr >= dst_udp->UdpL_Levels)
				    *dst_word_ptr++ = 0xffff;
				else 
				    *dst_word_ptr++ = (unsigned short)
					*src_long_ptr;
				src_long_ptr++;
				}
			    src_long_ptr += src_line_pad;  
			    }/* end scanline loop */
			}
		    else 
			{
			tmp_udp.UdpA_Base = 0;
			status = IpsScaleData 
			    (src_udp, &tmp_udp, dst_udp->UdpL_Levels, flags);
			if (status != IpsX_SUCCESS)
			    {
			    if (mem_alloc == 1)
				(*IpsA_MemoryTable[IpsK_FreeDataPlane]) 
				    (dst_udp->UdpA_Base);
			    if (tmp_udp.UdpA_Base != 0)
				(*IpsA_MemoryTable[IpsK_FreeDataPlane]) 
				    (tmp_udp.UdpA_Base);
			    return (status);
			    }

			/* set to tmp if scaledata did anything */
			if (tmp_udp.UdpA_Base != 0)
			    {
			    src_long_ptr = (unsigned int *) (tmp_udp.UdpA_Base);
			    src_line_pad = 0;
			    }
			for (iy = 0; iy < line_cnt; iy++)
			    {
			    for (ix = 0; ix < pixel_cnt; ix++)
				*dst_word_ptr++ = (unsigned short)
				    *src_long_ptr++;
			    src_long_ptr += src_line_pad;  
			    }/* end scanline loop */
			if (tmp_udp.UdpA_Base != 0)
			    (*IpsA_MemoryTable[IpsK_FreeDataPlane]) 
			    (tmp_udp.UdpA_Base);
  			}
		    break;	    /* long src - word dst */

		    case UdpK_DTypeBU:

    /* src = long , dst = byte */
		    /* byte output */
		    dst_byte_ptr = (unsigned char *)dst_udp->UdpA_Base + 
			(dst_udp->UdpL_Pos>>3);
		    if (src_udp->UdpL_Levels <= dst_udp->UdpL_Levels)
		        {
			/* 
			** set dst quant levels to src's quant levels 
			*/
			dst_udp->UdpL_Levels = src_udp->UdpL_Levels;
			for (iy = 0; iy < line_cnt; iy++)
			    {
			    for (ix = 0; ix < pixel_cnt; ix++)
				    *dst_byte_ptr++ = (unsigned char)
					*src_long_ptr++;
			    src_long_ptr += src_line_pad;  
			    }/* end scanline loop */
			}
		    else if (flags & IpsM_ClipToFit)
			{
			for (iy = 0; iy < line_cnt; iy++)
			    {
			    for (ix = 0; ix < pixel_cnt; ix++)
				{
				if (*src_long_ptr >= dst_udp->UdpL_Levels)
					*dst_byte_ptr++ = 0xffffffff;
				else 
				    *dst_byte_ptr++ = (unsigned char)
					*src_long_ptr;
				src_long_ptr++;
				}
			    src_long_ptr += src_line_pad;  
			    }/* end scanline loop */
			}
		    else 
			{
			tmp_udp.UdpA_Base = 0;
			status = IpsScaleData 
			    (src_udp, &tmp_udp, dst_udp->UdpL_Levels, flags);
			if (status != IpsX_SUCCESS)
			    {
			    if (mem_alloc == 1)
				(*IpsA_MemoryTable[IpsK_FreeDataPlane]) 
				    (dst_udp->UdpA_Base);
			    if (tmp_udp.UdpA_Base != 0)
				(*IpsA_MemoryTable[IpsK_FreeDataPlane]) 
				    (tmp_udp.UdpA_Base);
			     return (status);
			    }

			if (tmp_udp.UdpA_Base != 0)
			    {
			    src_long_ptr = (unsigned int *) (tmp_udp.UdpA_Base);
			    src_line_pad = 0;
			    }
			for (iy = 0; iy < line_cnt; iy++)
			    {
			    for (ix = 0; ix < pixel_cnt; ix++)
				*dst_byte_ptr++ = (unsigned short)
				    *src_long_ptr++;
			    src_long_ptr += src_line_pad;  
			    }/* end scanline loop */
			if (tmp_udp.UdpA_Base != 0)
			    (*IpsA_MemoryTable[IpsK_FreeDataPlane]) 
			    (tmp_udp.UdpA_Base);
  			}
		    break;	    /* long src - byte dst */
		    }		    /* end of dst switch    */
		break;	/* end of long src */

	    case UdpK_DTypeWU:		    /* word src */
		src_word_ptr = (unsigned short *)src_udp->UdpA_Base + 
		    (src_udp->UdpL_Pos>>4) + (src_udp->UdpL_ScnStride>>4) *
			src_udp->UdpL_Y1 + src_udp->UdpL_X1;
		src_line_pad = (src_udp->UdpL_ScnStride>>4) - 
		    src_udp->UdpL_PxlPerScn;
		
		switch (dst_udp->UdpB_DType)
		    {
		    case UdpK_DTypeBU:

    /* src = short , dst = byte */
				
		    /* byte output */
		    dst_byte_ptr = (unsigned char *)dst_udp->UdpA_Base + 
			(dst_udp->UdpL_Pos>>3);
		    if (src_udp->UdpL_Levels <= dst_udp->UdpL_Levels)
		        {
			/* 
			** set dst quant levels to src's quant levels 
			*/
			dst_udp->UdpL_Levels = src_udp->UdpL_Levels;
			for (iy = 0; iy < line_cnt; iy++)
			    {
			    for (ix = 0; ix < pixel_cnt; ix++)
				    *dst_byte_ptr++ = (unsigned char)
					*src_word_ptr++;
			    src_word_ptr += src_line_pad;  
			    }/* end scanline loop */
			}
		    else if (flags & IpsM_ClipToFit)
			{
			for (iy = 0; iy < line_cnt; iy++)
			    {
			    for (ix = 0; ix < pixel_cnt; ix++)
				{
				if (*src_word_ptr >= dst_udp->UdpL_Levels)
					*dst_byte_ptr++ = 0xffffffff;
				else 
				    *dst_byte_ptr++ = (unsigned char)
					*src_word_ptr;
				src_word_ptr++;
				}
			    src_word_ptr += src_line_pad;  
			    }/* end scanline loop */
			}
		    else 
			{
			tmp_udp.UdpA_Base = 0;
			status = IpsScaleData 
			    (src_udp, &tmp_udp, dst_udp->UdpL_Levels, flags);
			if (status != IpsX_SUCCESS)
			    {
			    if (mem_alloc == 1)
				(*IpsA_MemoryTable[IpsK_FreeDataPlane]) 
				    (dst_udp->UdpA_Base);
			    if (tmp_udp.UdpA_Base != 0)
				(*IpsA_MemoryTable[IpsK_FreeDataPlane]) 
				    (tmp_udp.UdpA_Base);
			    return (status);
			    }

			if (tmp_udp.UdpA_Base != 0)
			    {
			    src_word_ptr = (unsigned short *) 
				(tmp_udp.UdpA_Base);
			    src_line_pad = 0;
			    }
			for (iy = 0; iy < line_cnt; iy++)
			    {
			    for (ix = 0; ix < pixel_cnt; ix++)
				*dst_byte_ptr++ = (unsigned char)
				    *src_word_ptr++;
			    src_word_ptr += src_line_pad;  
			    }/* end scanline loop */
			if (tmp_udp.UdpA_Base != 0)
			    (*IpsA_MemoryTable[IpsK_FreeDataPlane]) 
			    (tmp_udp.UdpA_Base);
  			}
		    break;	    /* word src - byte dst */
		    }		    /* end of dst switch    */
		break;	/* end of long src */
	    } /* end of src switch */

    return (IpsX_SUCCESS);
}

static long IpsScaleData (src_udp, dst_udp, levels, flags)
struct UDP	*src_udp;
struct UDP	*dst_udp;
unsigned long	levels;
unsigned long	flags;
    {
    long status;
    double maximum, minimum, dummy, dummy1, dummy2, dummy3;
    unsigned long long_dummy = 0;
    double scale_factor;


    maximum = minimum = dummy = dummy1 = dummy2 = dummy3 = 0.0;

    /* 
    ** find minimum 
    */
    status = _IpsPointStatistics (src_udp, 0, &dummy, &dummy1, 
	&minimum, &maximum, &dummy2, &dummy3);
    if (minimum < 0)
	{
	/* make positive if less than zero */
	maximum -= minimum;
	status = _IpsConstantArithmetic (src_udp, dst_udp, 0,
	    (-1.0 * minimum), IpsK_Addition, 0, 0, &long_dummy);
	/* 
	** scale data - multiply by scale factor 
	*/
	if ((maximum >= levels) || (flags & IpsM_ScaleToFit))
	    {
	    scale_factor = (double)(levels - 1) / maximum;
	    status = _IpsConstantArithmetic (dst_udp, dst_udp, 0,
		scale_factor, IpsK_Multiplication, 0, 0, &long_dummy);
	    }
	}
    else
	{
	/* 
	** scale data - multiply "src" by scale factor 
	*/
	if ((maximum >= levels) || (flags & IpsM_ScaleToFit))
	    {
	    if (maximum != 0.0)
		{
		scale_factor = (double)(levels - 1) / maximum;
		status = _IpsConstantArithmetic (src_udp, dst_udp, 0,
		    scale_factor, IpsK_Multiplication, 0, 0, &long_dummy);
		}
	    }
	}
    dst_udp->UdpL_Levels = levels;
    return status;
    }

