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
**      _IpsSeparatePlane separates a single plane of data into multiple
**	planes.
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
**      May 22, 1990
**
************************************************************************/

/*
**  Table of contents
*/
#ifdef NODAS_PROTO
long  _IpsSeparatePlane();
#endif

/*
** Include files
*/
#include <IpsDef.h>			    /* IPS definitions		    */
#include <IpsStatusCodes.h>		    /* IPS Status Codes		    */
#include <IpsMemoryTable.h>		    /* IPS Memory Vector Table      */
#ifndef NODAS_PROTO
#include <ipsprot.h>				    /* Ips prototypes */
#endif
/*
**  External References:
*/

#ifdef NODAS_PROTO
long _IpsVerifyNotInPlace();		    /* from Ips__udp_utils.c	    */
#endif

/******************************************************************************
**
**  FUNCTIONAL DESCRIPTION:
**
**	This function separates udps into multiple udps by shifting
**	leftmost pixels down last.  The last udp will consist of the
**	leftmost bits of the source pixel.  The bits per plane applies
**	to the rightmost bits first.
**
**         src               dst1  dst2  dst3
**        8bits              3bits 3bits 2bits  
**     +----------+  To 3    +---+ +---+ +--+
**     |10|110|101| ------>  |101| |110| |10| 
**     +----------+ Planes   +---+ +---+ +--+
**
**
**  FORMAL PARAMETERS:
**
**	src_udp		    source udp
**	dst_udp_list	    list of destination udps
**	num_of_planes	    number of destination udps desired
**	bits_per_plane	    address of the desired number of bits/plane
**	list_of_data_types  returned list of dtypes for of the resultant udps
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	list of data types for the resultant dst udps
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

long _IpsSeparatePlane (src_udp, udp_ptr, num_of_planes, 
				bits_per_plane, dtypes)
struct UDP	*src_udp;		    /* source udp		    */
struct UDP	*udp_ptr[];		    /* list of dst udps		    */
unsigned long	num_of_planes;		    /* number of dst udps	    */
unsigned long	*bits_per_plane;	    /* desired bits per plane	    */
unsigned long	*dtypes;		    /* returned data types	    */
{
    struct UDP		*dst_udp;	    /* dst udp pointer		    */
    unsigned long	line_cnt;	    /* number of scanlines	    */
    unsigned long	line_idx;	    /* scanline loop index	    */
    unsigned long	pixel_cnt;	    /* number of pixels/scanline    */
    unsigned long	pixel_idx;	    /* pixel loop index		    */
    unsigned long	ix,iy;		    /* loop entities		    */

    unsigned long	src_line_pad;
    unsigned long	dst_line_pad;
    unsigned char	*src_byte_ptr, *dst_byte_ptr;	
    unsigned short	*src_word_ptr, *dst_word_ptr;
    unsigned long 	*src_long_ptr, *dst_long_ptr;
    unsigned long	total_bits, num_of_bits;
    unsigned long	shift[MAX_NUMBER_OF_COMPONENTS]; /* right shift value */
    unsigned long	mask[MAX_NUMBER_OF_COMPONENTS];  /* mask value        */
    long		status;

    if (num_of_planes < 2) return (IpsX_INVDARG);

    /* 
    ** make sure in place is not requested (rely on the fact that no src udp 
    ** have 0 base addr)
    */
    for (ix = 0; ix < num_of_planes; ix++)
	{
	dst_udp = udp_ptr[ix];
	if (src_udp->UdpA_Base == dst_udp->UdpA_Base) return (IpsX_NOINPLACE);
	}

    for (total_bits = 0, ix = 0; ix < num_of_planes; ix++) 
	total_bits += *(bits_per_plane+ix);
    if (total_bits > src_udp->UdpW_PixelLength) return (IpsX_INCNSARG);
	
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
		break;
		case UdpK_DTypeF:
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

    /* Determine dst data types */
    /* Fill in dst udp before copying */

    total_bits = 0;
    for (ix = 0; ix < num_of_planes; ix++)
	{
	dst_udp = udp_ptr[ix];
	num_of_bits = *(bits_per_plane + ix);
	if (num_of_bits <= 8) 
	    {
	    dst_udp->UdpW_PixelLength = 8;
	    dst_udp->UdpB_DType = UdpK_DTypeBU;
	    dst_udp->UdpL_PxlStride = 8;
	    *(dtypes + ix) = IpsK_DTypeBU;
	    }
	else if (num_of_bits <= 16) 
	    {
	    dst_udp->UdpW_PixelLength = 16;
	    dst_udp->UdpB_DType = UdpK_DTypeWU;
	    dst_udp->UdpL_PxlStride = 16;
	    *(dtypes + ix) = IpsK_DTypeWU;
	    }
	else 
	    {
	    dst_udp->UdpW_PixelLength = 32;
	    dst_udp->UdpB_DType = UdpK_DTypeLU;
	    dst_udp->UdpL_PxlStride = 32;
	    *(dtypes + ix) = IpsK_DTypeLU;
	    }

	dst_udp->UdpB_Class = src_udp->UdpB_Class;
	dst_udp->UdpL_X1 = 0;
	dst_udp->UdpL_Y1 = 0;
	dst_udp->UdpL_X2 = src_udp->UdpL_PxlPerScn - 1;
	dst_udp->UdpL_Y2 = src_udp->UdpL_ScnCnt - 1;
	dst_udp->UdpL_PxlPerScn = src_udp->UdpL_PxlPerScn;
	dst_udp->UdpL_ScnStride = dst_udp->UdpL_PxlPerScn * 
	    dst_udp->UdpL_PxlStride;
	dst_udp->UdpL_ScnCnt = src_udp->UdpL_ScnCnt;
        dst_udp->UdpL_Levels = 1 << num_of_bits;
        dst_udp->UdpL_CompIdx = src_udp->UdpL_CompIdx;
	
        if (dst_udp->UdpA_Base != 0)
            {
	    /* No in place allowed */
	    status = _IpsVerifyNotInPlace (src_udp, dst_udp);
	    if (status != IpsX_SUCCESS) return (status);
            if (dst_udp->UdpL_ArSize - dst_udp->UdpL_Pos < 
		(dst_udp->UdpL_ScnStride * dst_udp->UdpL_ScnCnt))
		    return (IpsX_INSVIRMEM);
            }
        else
            {
            dst_udp->UdpA_Base = (unsigned char *)
                (*IpsA_MemoryTable[IpsK_AllocateDataPlane])
                    (((dst_udp->UdpL_ScnStride * dst_udp->UdpL_ScnCnt+7)/8),
			IpsM_InitMem,0);
            if (!dst_udp->UdpA_Base) 
                return (IpsX_INSVIRMEM);
            dst_udp->UdpL_ArSize = dst_udp->UdpL_ScnStride * 
		dst_udp->UdpL_ScnCnt;
	    dst_udp->UdpL_Pos = 0;
            }

	shift[ix] = total_bits;
	mask[ix] = (1 << num_of_bits) - 1;
	total_bits += num_of_bits;
	}/* end planes loop */
    /*
    ** separate it 
    */
    line_cnt = dst_udp->UdpL_ScnCnt;
    pixel_cnt = dst_udp->UdpL_PxlPerScn;

    switch (src_udp->UdpB_DType)	
	{
	case UdpK_DTypeBU:		    /* byte src */
	    src_line_pad = 
		(src_udp->UdpL_ScnStride>>3) - src_udp->UdpL_PxlPerScn;

	    for (ix = 0; ix < num_of_planes; ix++)
		{
		dst_udp = udp_ptr[ix];

		src_byte_ptr = (unsigned char *)
		    src_udp->UdpA_Base + (src_udp->UdpL_Pos>>3) +
			(src_udp->UdpL_ScnStride>>3) * src_udp->UdpL_Y1 +
			    src_udp->UdpL_X1;
		switch (*(dtypes+ix))
		    {
		    case IpsK_DTypeBU:
			dst_byte_ptr = dst_udp->UdpA_Base + 
			    (dst_udp->UdpL_Pos>>3);
			dst_line_pad = (dst_udp->UdpL_ScnStride>>3) - 
			    dst_udp->UdpL_PxlPerScn;
			for (line_idx = 0; line_idx < line_cnt; line_idx++)
			    {
			    for (pixel_idx=0;pixel_idx < pixel_cnt; pixel_idx++)
				*dst_byte_ptr++ = 
				    ((*src_byte_ptr++ >> shift[ix]) & mask[ix]);
			    src_byte_ptr += src_line_pad;  /* skip over pad */
			    dst_byte_ptr += dst_line_pad;
			    }/* end scanline loop */
		    break;

		    case UdpK_DTypeWU:		    /* byte src, dst word */
		    case IpsK_DTypeLU:
		    default:
			return (IpsX_INCNSARG);
		    break;
		    } /* end of dst switch */
		} /* end of planes loop */
	break;

	case UdpK_DTypeWU:		    /* word src */
	    
	    src_line_pad = (src_udp->UdpL_ScnStride>>4) - 
		src_udp->UdpL_PxlPerScn;

	    for (ix = 0; ix < num_of_planes; ix++)
		{
		dst_udp = udp_ptr[ix];
		src_word_ptr = (unsigned short*) src_udp->UdpA_Base + 
		    (src_udp->UdpL_Pos>>4) + (src_udp->UdpL_ScnStride>>4) * 
			src_udp->UdpL_Y1 + src_udp->UdpL_X1;
		switch (*(dtypes+ix))
		    {
		    case IpsK_DTypeBU:
			dst_byte_ptr = dst_udp->UdpA_Base + 
			    (dst_udp->UdpL_Pos>>3);
			dst_line_pad = (dst_udp->UdpL_ScnStride>>3) - 
			    dst_udp->UdpL_PxlPerScn;
			for (line_idx = 0; line_idx < line_cnt; line_idx++)
			    {
			    for (pixel_idx=0;pixel_idx < pixel_cnt; pixel_idx++)
				*dst_byte_ptr++ = 
				    ((*src_word_ptr++ >> shift[ix])) & mask[ix];
			    src_word_ptr += src_line_pad;  /* skip over pad */
			    dst_byte_ptr += dst_line_pad;
			    }/* end scanline loop */
		    break;

		    case UdpK_DTypeWU:		    /* word src, dst word */
			dst_word_ptr = (unsigned short *)dst_udp->UdpA_Base + 
			    (dst_udp->UdpL_Pos>>4);
			dst_line_pad = (dst_udp->UdpL_ScnStride>>4) - 
			    dst_udp->UdpL_PxlPerScn;
			for (line_idx = 0; line_idx < line_cnt; line_idx++)
			    {
			    for (pixel_idx=0;pixel_idx < pixel_cnt; pixel_idx++)
				*dst_word_ptr++ =
				    ((*src_word_ptr++ >> shift[ix])) & mask[ix];
			    src_word_ptr += src_line_pad;
			    dst_word_ptr += dst_line_pad;
			    }/* end scanline loop */
		    break;

		    case IpsK_DTypeLU:

		    default:
			return (IpsX_INCNSARG);
		    break;
		    } /* end of dst switch */
		} /* end of planes loop */
	break;

	case UdpK_DTypeLU:		    /* long src */
	    src_line_pad = 
		(src_udp->UdpL_ScnStride>>5) - src_udp->UdpL_PxlPerScn;

	    for (ix = 0; ix < num_of_planes; ix++)
		{
		dst_udp = udp_ptr[ix];
		src_long_ptr = (unsigned long *) src_udp->UdpA_Base + 
		    (src_udp->UdpL_Pos>>5) + (src_udp->UdpL_ScnStride>>5) * 
			src_udp->UdpL_Y1 + src_udp->UdpL_X1;
		switch (*(dtypes+ix))
		    {
		    case IpsK_DTypeBU:
			dst_byte_ptr = dst_udp->UdpA_Base + 
			    (dst_udp->UdpL_Pos>>3);
			dst_line_pad = (dst_udp->UdpL_ScnStride>>3) - 
			    dst_udp->UdpL_PxlPerScn;
			for (line_idx = 0; line_idx < line_cnt; line_idx++)
			    {
			    for (pixel_idx=0;pixel_idx < pixel_cnt; pixel_idx++)
				*dst_byte_ptr++ = 
				    ((*src_long_ptr++ >> shift[ix])) & mask[ix];
			    src_long_ptr += src_line_pad;  /* skip over pad */
			    dst_byte_ptr += dst_line_pad;
			    }/* end scanline loop */
		    break;

		    case UdpK_DTypeWU:		    /* long src, dst word */
			dst_word_ptr = (unsigned short *)dst_udp->UdpA_Base + 
			    (dst_udp->UdpL_Pos>>4);
			dst_line_pad = (dst_udp->UdpL_ScnStride>>4) - 
			    dst_udp->UdpL_PxlPerScn;
			for (line_idx = 0; line_idx < line_cnt; line_idx++)
			    {
			    for (pixel_idx=0;pixel_idx < pixel_cnt; pixel_idx++)
				*dst_word_ptr++ =
				    ((*src_long_ptr++ >> shift[ix])) & mask[ix];
			    src_long_ptr += src_line_pad;
			    dst_word_ptr += dst_line_pad;
			    }/* end scanline loop */
		    break;

		    case IpsK_DTypeLU:
			dst_long_ptr = (unsigned long *) dst_udp->UdpA_Base + 
			    (dst_udp->UdpL_Pos>>5);
			dst_line_pad = (dst_udp->UdpL_ScnStride>>5) - 
			    dst_udp->UdpL_PxlPerScn;
			for (line_idx = 0; line_idx < line_cnt; line_idx++)
			    {
			    for (pixel_idx=0;pixel_idx < pixel_cnt; pixel_idx++)
				*dst_long_ptr++ =
				    ((*src_long_ptr++ >> shift[ix])) & mask[ix];
			    src_long_ptr += src_line_pad;
			    dst_long_ptr += dst_line_pad;
			    }/* end scanline loop */
		    break;

		    default:
			return (IpsX_INCNSARG);
		    break;
		    } /* end of dst switch */
		} /* end of planes loop */
	break;
	} /* end of src switch */
			    
    return (IpsX_SUCCESS);
    } /* end of _IpsSeparatePlane*/

