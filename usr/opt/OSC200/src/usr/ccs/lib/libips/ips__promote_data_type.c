/*  DEC/CMS REPLACEMENT HISTORY, Element IPS__PROMOTE_DATA_TYPE.C */
/*  *4    14-AUG-1990 11:39:49 PICCOLO "reverse error checking for in_place and size to provide a more meaningful error message" */
/*  *3     9-AUG-1990 15:18:48 PICCOLO "pos change" */
/*  *2    14-JUN-1990 09:34:35 PICCOLO "incorporated code review comments" */
/*  *1     8-JUN-1990 11:28:35 PICCOLO "promote data type routine" */
/*  DEC/CMS REPLACEMENT HISTORY, Element IPS__PROMOTE_DATA_TYPE.C */
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
**      _IpsPromoteDataType creates dst udp with the specified data type
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
**      June 6, 1990
**
************************************************************************/

/*
**  Table of contents
*/
#ifdef NODAS_PROTO
long  _IpsPromoteDataType();
#endif

/*
** Include files
*/
#include <IpsDef.h>			    /* IPS definitions		    */
#include <IpsDefP.h>			    /* IPS Private definitions	    */
#include <IpsStatusCodes.h>		    /* IPS Status Codes		    */
#include <IpsMemoryTable.h>		    /* IPS Memory Vector Table      */
#ifndef NODAS_PROTO
#include <ipsprot.h>				    /* Ips prototypes */
#endif

/*
** External References
*/
#ifdef NODAS_PROTO
long _IpsCopyData();			    /* from ips__udp_utils	    */
long _IpsVerifyNotInPlace();		    /* from Ips__udp_utils.c	    */
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
**	dst_udp		resultant merged udp
**	dst_dtype	desired data type
**	flags		options to define dst quant levels the same as the
**			src udp or to reflect the new data type
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
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/

long _IpsPromoteDataType (src_udp, dst_udp, dst_dtype, flags)
struct UDP	*src_udp;
struct UDP	*dst_udp;
unsigned long	dst_dtype;
unsigned long	flags;
{
    COMPLEX		*dst_complex_ptr;
    float		*dst_float_ptr;
    unsigned char	*src_byte_ptr;
    unsigned short	*src_word_ptr;
    unsigned long	*src_long_ptr;
    float		*src_float_ptr;		
    unsigned long	src_line_pad;
    unsigned long	size;
    unsigned char	mem_alloc = 0;
    unsigned long	line_cnt;	    /* number of scanlines	    */
    unsigned long	line_idx;	    /* scanline loop index	    */
    unsigned long	pixel_cnt;	    /* number of pixels/scanline    */
    unsigned long	pixel_idx;	    /* pixel loop index		    */
    long		status;

    switch (src_udp->UdpB_Class)
	{
	case UdpK_ClassA:
	    /*
	    ** Atomic array
	    */
	    switch (src_udp->UdpB_DType)
		{
		case UdpK_DTypeBU:
	    	case UdpK_DTypeWU:
		case UdpK_DTypeLU:
		case UdpK_DTypeF:
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

    /* Check dtype to make sure we are really promoting */
    /* Note: data types are ordered */

    if (dst_dtype <= src_udp->UdpB_DType)
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
    dst_udp->UdpL_Levels = src_udp->UdpL_Levels;

    switch (dst_dtype)
	{
	/* 
	** dst type word
	*/
	case IpsK_DTypeWU:
	    dst_udp->UdpW_PixelLength = 16;
	    if (flags & IpsM_UpdateQuantLevels)
		dst_udp->UdpL_Levels = 0x10000;
	break;

	case IpsK_DTypeLU:
	    dst_udp->UdpW_PixelLength = 32;
	    if (flags & IpsM_UpdateQuantLevels)
		dst_udp->UdpL_Levels = 0xFFFFFFFF;
	break;

	case IpsK_DTypeF:
	    dst_udp->UdpW_PixelLength = 32;
	break;
	    
	case IpsK_DTypeC:
	    dst_udp->UdpW_PixelLength = 64;
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

    if ((dst_dtype != IpsK_DTypeF) && (dst_dtype != IpsK_DTypeC))
	{
	/* integers only */		
	status = _IpsCopyData (src_udp, dst_udp);
	if (status != IpsX_SUCCESS)
	    if (mem_alloc == 1) 
		(*IpsA_MemoryTable[IpsK_Dealloc])(dst_udp->UdpA_Base);
	}
    else
	{
	line_cnt = dst_udp->UdpL_ScnCnt;
	pixel_cnt = dst_udp->UdpL_PxlPerScn;
	if (dst_dtype == IpsK_DTypeF)
	    {
	    /* float output */
	    dst_float_ptr = (float *)dst_udp->UdpA_Base + 
		(dst_udp->UdpL_Pos>>5);
	    switch (src_udp->UdpB_DType)	
		{
		case UdpK_DTypeBU:		    /* float dst, byte src */
		    src_byte_ptr = (unsigned char *)src_udp->UdpA_Base + 
			(src_udp->UdpL_Pos>>3) + (src_udp->UdpL_ScnStride>>3) *
			    src_udp->UdpL_Y1 + src_udp->UdpL_X1;
		    src_line_pad = 
			(src_udp->UdpL_ScnStride>>3) - src_udp->UdpL_PxlPerScn;
		
		    for (line_idx = 0; line_idx < line_cnt; line_idx++)
			{
			for (pixel_idx = 0; pixel_idx < pixel_cnt; pixel_idx++)
			    *dst_float_ptr++ = (float)*src_byte_ptr++;
			src_byte_ptr += src_line_pad;  /* skip over any pad */
			}/* end scanline loop */

		break;

		case UdpK_DTypeWU:		    /* float dst, word src */
		    src_word_ptr = (unsigned short*)src_udp->UdpA_Base + 
			(src_udp->UdpL_Pos>>4) + (src_udp->UdpL_ScnStride>>4) *
			    src_udp->UdpL_Y1 + src_udp->UdpL_X1;
		    src_line_pad = 
			(src_udp->UdpL_ScnStride>>4) - src_udp->UdpL_PxlPerScn;
		
		    for (line_idx = 0; line_idx < line_cnt; line_idx++)
			{
			for (pixel_idx = 0; pixel_idx < pixel_cnt; pixel_idx++)
			    *dst_float_ptr++ = (float)*src_word_ptr++;
			src_word_ptr += src_line_pad;  /* skip over any pad */
			}/* end scanline loop */
		break;

		case UdpK_DTypeLU:		    /* float dst, long src */
		    src_long_ptr = (unsigned long *)src_udp->UdpA_Base + 
			(src_udp->UdpL_Pos>>5) + (src_udp->UdpL_ScnStride>>5) *
			    src_udp->UdpL_Y1 + src_udp->UdpL_X1;
		    src_line_pad = 
			(src_udp->UdpL_ScnStride>>5) - src_udp->UdpL_PxlPerScn;
		
		    for (line_idx = 0; line_idx < line_cnt; line_idx++)
			{
			for (pixel_idx = 0; pixel_idx < pixel_cnt; pixel_idx++)
			    *dst_float_ptr++ = (float)*src_long_ptr++;
			src_long_ptr += src_line_pad;  /* skip over any pad */
			}/* end scanline loop */
		break;
		} /* end of src switch */
	    }
	 else
	    {
	    /* to complex data */
	    dst_complex_ptr = (COMPLEX *)dst_udp->UdpA_Base + 
		(dst_udp->UdpL_Pos>>6);
	    switch (src_udp->UdpB_DType)	
		{
		case UdpK_DTypeBU:		    /* complex dst, byte src */
		    src_byte_ptr = (unsigned char *)src_udp->UdpA_Base + 
			(src_udp->UdpL_Pos>>3) + (src_udp->UdpL_ScnStride>>3) *
			    src_udp->UdpL_Y1 + src_udp->UdpL_X1;
		    src_line_pad = 
			(src_udp->UdpL_ScnStride>>3) - src_udp->UdpL_PxlPerScn;
		
		    for (line_idx = 0; line_idx < line_cnt; line_idx++)
			{
			for (pixel_idx = 0; pixel_idx < pixel_cnt; pixel_idx++)
			    {
			    dst_complex_ptr->real = (float)*src_byte_ptr++;
			    dst_complex_ptr->imag = 0.0;
			    dst_complex_ptr++;
			    }
			src_byte_ptr += src_line_pad;  /* skip over any pad */
			}/* end scanline loop */

		break;

		case UdpK_DTypeWU:		    /* complex dst, word src */
		    src_word_ptr = (unsigned short*)src_udp->UdpA_Base + 
			(src_udp->UdpL_Pos>>4) + (src_udp->UdpL_ScnStride>>4) *
			    src_udp->UdpL_Y1 + src_udp->UdpL_X1;
		    src_line_pad = 
			(src_udp->UdpL_ScnStride>>4) - src_udp->UdpL_PxlPerScn;
		
		    for (line_idx = 0; line_idx < line_cnt; line_idx++)
			{
			for (pixel_idx = 0; pixel_idx < pixel_cnt; pixel_idx++)
			    {
			    dst_complex_ptr->real = (float)*src_word_ptr++;
			    dst_complex_ptr->imag = 0.0;
			    dst_complex_ptr++;
			    }
			src_word_ptr += src_line_pad;  /* skip over any pad */
			}/* end scanline loop */
		break;

		case UdpK_DTypeLU:		    /* complex dst, long src */
		    src_long_ptr = (unsigned long *)src_udp->UdpA_Base + 
			(src_udp->UdpL_Pos>>5) + (src_udp->UdpL_ScnStride>>5) *
			    src_udp->UdpL_Y1 + src_udp->UdpL_X1;
		    src_line_pad = 
			(src_udp->UdpL_ScnStride>>5) - src_udp->UdpL_PxlPerScn;
		
		    for (line_idx = 0; line_idx < line_cnt; line_idx++)
			{
			for (pixel_idx = 0; pixel_idx < pixel_cnt; pixel_idx++)
			    {
			    dst_complex_ptr->real = (float)*src_long_ptr++;
			    dst_complex_ptr->imag = 0.0;
			    dst_complex_ptr++;
			    }
			src_long_ptr += src_line_pad;  /* skip over any pad */
			}/* end scanline loop */
		break;

		case UdpK_DTypeF:		    /* complex dst, float src */
		    src_float_ptr = (float *)src_udp->UdpA_Base + 
			(src_udp->UdpL_Pos>>5) + (src_udp->UdpL_ScnStride>>5) *
			    src_udp->UdpL_Y1 + src_udp->UdpL_X1;
		    src_line_pad = 
			(src_udp->UdpL_ScnStride>>5) - src_udp->UdpL_PxlPerScn;
		
		    for (line_idx = 0; line_idx < line_cnt; line_idx++)
			{
			for (pixel_idx = 0; pixel_idx < pixel_cnt; pixel_idx++)
			    {
			    dst_complex_ptr->real = (float)*src_float_ptr++;
			    dst_complex_ptr->imag = 0.0;
			    dst_complex_ptr++;
			    }
			src_float_ptr += src_line_pad;  /* skip over any pad */
			}/* end scanline loop */
		break;
		} /* end of src switch */
	    } /* end of float or complex test */
	status = IpsX_SUCCESS;
	} /* end of if-else */
    return (status);
}

