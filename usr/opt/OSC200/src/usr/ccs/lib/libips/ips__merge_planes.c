/*  DEC/CMS REPLACEMENT HISTORY, Element IPS__MERGE_PLANES.C */
/*  *4     9-AUG-1990 15:18:20 PICCOLO "POS" */
/*  *3    14-JUN-1990 09:33:03 PICCOLO "code review, bug fix" */
/*  *2     6-JUN-1990 10:50:19 PICCOLO "make routines long; not unsigned long" */
/*  *1     1-JUN-1990 16:20:13 PICCOLO "IPS Base Level" */
/*  DEC/CMS REPLACEMENT HISTORY, Element IPS__MERGE_PLANES.C */
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
**      _IpsMergePlanes merges multiple data planes into a single data plane
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
long  _IpsMergePlanes();
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
#include <math.h>			    /* C math functions		    */

/*
** External References
*/
#ifdef NODAS_PROTO
long _IpsVerifyNotInPlace();		    /* from ips__udp_utils	    */
#endif

/******************************************************************************
**
**  FUNCTIONAL DESCRIPTION:
**
**	This function merges multiple udps into one udp
**
**      src1  src2  src3
**      3bits 3bits 2bits  
**      +---+ +---+ +--+   default   +----------+
**      |101| |110| |10|   ------>   |10|110|101|
**      +---+ +---+ +--+             +----------+
**
**      src1  src2  src3
**      3bits 3bits 2bits  
**      +---+ +---+ +--+   reverse   +----------+
**      |101| |110| |10|   ------>   |101|110|10|
**      +---+ +---+ +--+             +----------+
**
**  FORMAL PARAMETERS:
**
**	udp_list	list of udps
**	dst_udp		resultant merged udp
**	num_of_planes	number of source udps
**	flags		options to read udp list in reverse order and/or
**			to use bits per pixel or quantization levels
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

long _IpsMergePlanes (udp_ptr, dst_udp, num_of_planes, flags)
struct UDP	*udp_ptr[];
struct UDP	*dst_udp;
unsigned long	num_of_planes;
unsigned long	flags;
{
    struct UDP		*src_udp;	    /* udp pointer		    */
    struct UDP		*tmp_udp;	    /* scratch udp pointer	    */
    unsigned long	line_cnt;	    /* number of scanlines	    */
    unsigned long	line_idx;	    /* scanline loop index	    */
    unsigned long	line_pad;	    /* number of pad	            */
    unsigned long	pixel_cnt;	    /* number of pixels/scanline    */
    unsigned long	pixel_idx;	    /* pixel loop index		    */
    unsigned long	ix,iy;		    /* loop entities		    */

    unsigned long	src_line_pad;
    unsigned long	dst_line_pad;
    unsigned char	*src_byte_ptr, *dst_byte_ptr;
    unsigned short	*src_word_ptr, *dst_word_ptr;
    unsigned int 	*src_long_ptr, *dst_long_ptr;
    unsigned long	total_bits;
    unsigned long	shift[MAX_NUMBER_OF_COMPONENTS];
    long		status;

    /* Preliminary checks */

    if (num_of_planes < 2) 
	return (IpsX_INVDARG);

    /* 
    ** make sure all udps have same spatial extent and make sure in place 
    ** is not requested (rely on the fact that no src udp have 0 base addr)
    */

    src_udp = udp_ptr[0];
    for (ix = 1; ix < num_of_planes; ix++)
	{
	tmp_udp = udp_ptr[ix];
	if (tmp_udp->UdpL_PxlPerScn != src_udp->UdpL_PxlPerScn ||
	    tmp_udp->UdpL_ScnCnt != src_udp->UdpL_ScnCnt) 
		return (IpsX_INCNSARG);
	}

    /* Determine dst data type */
    total_bits = 0;
    for (ix = 0, iy = num_of_planes - 1; ix < num_of_planes; ix++, iy--)
	{
	if (flags & IpsM_ReverseUdpList) src_udp = udp_ptr[iy];
	else src_udp = udp_ptr[ix];
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
	if (flags & IpsM_SrcBitsPerPix)
	    {
	    shift[ix] = total_bits;
	    total_bits += src_udp->UdpW_PixelLength;
	    }
	else 
	    {
	    shift[ix] = total_bits;
	    total_bits += ceil(log10((double)src_udp->UdpL_Levels)/log10(2.0));
	    }
	}/* end total_bits loop */

	src_udp = udp_ptr[0];		/* use as spacial template */

	if (total_bits <= 8) 
	    {
	    dst_udp->UdpW_PixelLength = 8;
	    dst_udp->UdpB_DType = UdpK_DTypeBU;
	    dst_udp->UdpL_PxlStride = 8;
	    }
	else if (total_bits <= 16) 
	    {
	    dst_udp->UdpW_PixelLength = 16;
	    dst_udp->UdpB_DType = UdpK_DTypeWU;
	    dst_udp->UdpL_PxlStride = 16;
	    }
	else if (total_bits <= 32) 
	    {
	    dst_udp->UdpW_PixelLength = 32;
	    dst_udp->UdpB_DType = UdpK_DTypeLU;
	    dst_udp->UdpL_PxlStride = 32;
	    }
	else 
	    return (IpsX_DLENGTR32);

	/* build dst udp - no retain allowed for this routine */

	dst_udp->UdpB_Class = src_udp->UdpB_Class;
	dst_udp->UdpL_X1 = 0;
	dst_udp->UdpL_Y1 = 0;
	dst_udp->UdpL_X2 = src_udp->UdpL_PxlPerScn - 1;
	dst_udp->UdpL_Y2 = src_udp->UdpL_ScnCnt - 1;
	dst_udp->UdpL_PxlPerScn = src_udp->UdpL_PxlPerScn;
	dst_udp->UdpL_ScnStride = dst_udp->UdpL_PxlPerScn * 
	    dst_udp->UdpL_PxlStride;
	dst_udp->UdpL_ScnCnt = src_udp->UdpL_ScnCnt;
        dst_udp->UdpL_Levels = 1 << total_bits;
	
        if (dst_udp->UdpA_Base != 0)
            {
	    /* 
	    ** No in place allowed - verify 
	    */
	    for (ix = 0; ix < num_of_planes; ix++)
		{
		src_udp = udp_ptr[ix];
		status = _IpsVerifyNotInPlace (src_udp, dst_udp);
		    if (status != IpsX_SUCCESS) return (status);
		}
            if ((dst_udp->UdpL_ArSize - dst_udp->UdpL_Pos) < 
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
        dst_udp->UdpL_CompIdx = src_udp->UdpL_CompIdx;

    /*
    ** merge it 
    */
    line_cnt = dst_udp->UdpL_ScnCnt;
    pixel_cnt = dst_udp->UdpL_PxlPerScn;

    switch (dst_udp->UdpB_DType)
	{
	/* 
	** dst type byte 
	*/
	case UdpK_DTypeBU:
	    for (ix = 0, iy = num_of_planes - 1; ix < num_of_planes; ix++, iy--)
		{
		/* no retain allowed thus no Y1 and X1 increment */
		dst_byte_ptr = dst_udp->UdpA_Base + (dst_udp->UdpL_Pos>>3);
		dst_line_pad = (dst_udp->UdpL_ScnStride>>3) - 
		    dst_udp->UdpL_PxlPerScn;
		if (flags & IpsM_ReverseUdpList) src_udp = udp_ptr[iy];
		else src_udp = udp_ptr[ix];
	
		switch (src_udp->UdpB_DType)	
		    {
		    case UdpK_DTypeBU:		    /* byte dst, byte src */
		    src_byte_ptr = (unsigned char *) src_udp->UdpA_Base + 
			(src_udp->UdpL_Pos>>3) + (src_udp->UdpL_ScnStride>>3) * 
			    src_udp->UdpL_Y1 + src_udp->UdpL_X1;
		    src_line_pad = 
			(src_udp->UdpL_ScnStride>>3) - src_udp->UdpL_PxlPerScn;
		
		    for (line_idx = 0; line_idx < line_cnt; line_idx++)
		    	{
			for (pixel_idx = 0; pixel_idx < pixel_cnt; pixel_idx++)
			    {
			    *dst_byte_ptr |= (*src_byte_ptr++ << shift[ix]);
			    dst_byte_ptr++;
 		    	    }/* end pixel loop *
		    	src_byte_ptr += src_line_pad;  /* skip over any pad */
			dst_byte_ptr += dst_line_pad;
		    	}/* end scanline loop */
		    break;

		    case UdpK_DTypeWU:		    /* byte dst, word src */
		    src_word_ptr = (unsigned short *) src_udp->UdpA_Base + 
			(src_udp->UdpL_Pos>>4) + (src_udp->UdpL_ScnStride>>4) *
			     src_udp->UdpL_Y1 +	src_udp->UdpL_X1;
		    src_line_pad = 
			(src_udp->UdpL_ScnStride>>4) - src_udp->UdpL_PxlPerScn;
		
		    for (line_idx = 0; line_idx < line_cnt; line_idx++)
		    	{
			for (pixel_idx = 0; pixel_idx < pixel_cnt; pixel_idx++)
			    {
			    *dst_byte_ptr |= (*src_word_ptr++ << shift[ix]);
			    dst_byte_ptr++;
 		    	    }/* end pixel loop */
		    	src_word_ptr += src_line_pad;
			dst_byte_ptr += dst_line_pad;
		    	}/* end scanline loop */
		    break;

		    case UdpK_DTypeLU:		    /* byte dst, long src */
		    src_long_ptr = (unsigned int *)src_udp->UdpA_Base + 
			(src_udp->UdpL_Pos>>5) + (src_udp->UdpL_ScnStride>>5) *
			     src_udp->UdpL_Y1 +	src_udp->UdpL_X1;
		    src_line_pad = 
			(src_udp->UdpL_ScnStride>>5) - src_udp->UdpL_PxlPerScn;
		
		    for (line_idx = 0; line_idx < line_cnt; line_idx++)
		    	{
			for (pixel_idx = 0; pixel_idx < pixel_cnt; pixel_idx++)
			    {
			    *dst_byte_ptr |= (*src_long_ptr++ << shift[ix]);
			    dst_byte_ptr++;
 		    	    }/* end pixel loop */
		    	src_long_ptr += src_line_pad;
			dst_byte_ptr += dst_line_pad;
		    	}/* end scanline loop */
		    break;
		} /* end of src switch */
	    }	/* end of number of planes loop */
	break;  /* end of dst byte */

	/* 
	** dst type word
	*/
	case UdpK_DTypeWU:
	    for (ix = 0, iy = num_of_planes - 1; ix < num_of_planes; ix++, iy--)
		{
		/* no retain allowed thus no Y1 and X1 increment */
		dst_word_ptr = (unsigned short *) dst_udp->UdpA_Base + 
		    (dst_udp->UdpL_Pos>>4);
		dst_line_pad = (dst_udp->UdpL_ScnStride>>4) - 
		    dst_udp->UdpL_PxlPerScn;
		if (flags & IpsM_ReverseUdpList) src_udp = udp_ptr[iy];
		else src_udp = udp_ptr[ix];
	
		switch (src_udp->UdpB_DType)
		    {
		    case UdpK_DTypeBU:		    /* word dst, byte src */
		    src_byte_ptr = (unsigned char *) src_udp->UdpA_Base + 
			(src_udp->UdpL_Pos>>3) + (src_udp->UdpL_ScnStride>>3) * 
			    src_udp->UdpL_Y1 + src_udp->UdpL_X1;
		    src_line_pad = 
			(src_udp->UdpL_ScnStride>>3) - src_udp->UdpL_PxlPerScn;

		    for (line_idx = 0; line_idx < line_cnt; line_idx++)
		    	{
			for (pixel_idx = 0; pixel_idx < pixel_cnt; pixel_idx++)
			    {
			    *dst_word_ptr |= (*src_byte_ptr++ << shift[ix]);
			    dst_word_ptr++;
 		    	    }/* end pixel loop */
		    	src_byte_ptr += src_line_pad;
			dst_word_ptr += dst_line_pad;
		    	}/* end scanline loop */
		    break;

		    case UdpK_DTypeWU:		    /* word dst, word src */
		    src_word_ptr = (unsigned short *) src_udp->UdpA_Base + 
			(src_udp->UdpL_Pos>>4) + (src_udp->UdpL_ScnStride>>4) *
			     src_udp->UdpL_Y1 +	src_udp->UdpL_X1;
		    src_line_pad = 
			(src_udp->UdpL_ScnStride>>4) - src_udp->UdpL_PxlPerScn;
		
		    for (line_idx = 0; line_idx < line_cnt; line_idx++)
		    	{
			for (pixel_idx = 0; pixel_idx < pixel_cnt; pixel_idx++)
			    {
			    *dst_word_ptr |= (*src_word_ptr++ << shift[ix]);
			    dst_word_ptr++;
 		    	    }/* end pixel loop */
		    	src_word_ptr += src_line_pad;
			dst_word_ptr += dst_line_pad;
		    	}/* end scanline loop */
		    break;

		    case UdpK_DTypeLU:		    /* word dst, long src */
		    src_long_ptr = (unsigned int *) src_udp->UdpA_Base + 
			(src_udp->UdpL_Pos>>5) + (src_udp->UdpL_ScnStride>>5) *
			     src_udp->UdpL_Y1 +	src_udp->UdpL_X1;
		    src_line_pad = 
			(src_udp->UdpL_ScnStride>>5) - src_udp->UdpL_PxlPerScn;

		    for (line_idx = 0; line_idx < line_cnt; line_idx++)
		    	{
			for (pixel_idx = 0; pixel_idx < pixel_cnt; pixel_idx++)
			    {
			    *dst_word_ptr |= (*src_long_ptr++ << shift[ix]);
			    dst_word_ptr++;
 		    	    }/* end pixel loop */
		    	src_long_ptr += src_line_pad;
			dst_word_ptr += dst_line_pad;
		    	}/* end scanline loop */
		    break;
		} /* end of switch */
	    }	/* end of number of planes loop */
	break;

	/* 
	** dst type long
	*/
	case UdpK_DTypeLU:
	    for (ix = 0, iy = num_of_planes - 1; ix < num_of_planes; ix++, iy--)
		{
		/* no retain allowed thus no Y1 and X1 increment */
		dst_long_ptr = (unsigned int *) dst_udp->UdpA_Base + 
		    (dst_udp->UdpL_Pos>>5);
		dst_line_pad = (dst_udp->UdpL_ScnStride>>5) - 
		    dst_udp->UdpL_PxlPerScn;
		if (flags & IpsM_ReverseUdpList) src_udp = udp_ptr[iy];
		else src_udp = udp_ptr[ix];
	
		switch (src_udp->UdpB_DType)
		    {
		    case UdpK_DTypeBU:		    /* long dst, byte src */
		    src_byte_ptr = (unsigned char *) src_udp->UdpA_Base + 
			(src_udp->UdpL_Pos>>3) + (src_udp->UdpL_ScnStride>>3) * 
			    src_udp->UdpL_Y1 + src_udp->UdpL_X1;
		    src_line_pad = 
			(src_udp->UdpL_ScnStride>>3) - src_udp->UdpL_PxlPerScn;
		
		    for (line_idx = 0; line_idx < line_cnt; line_idx++)
		    	{
			for (pixel_idx = 0; pixel_idx < pixel_cnt; pixel_idx++)
			    {
			    *dst_long_ptr |= (*src_byte_ptr++ << shift[ix]);
			    dst_long_ptr++;
 		    	    }/* end pixel loop */
		    	src_byte_ptr += src_line_pad;
			dst_long_ptr += dst_line_pad;
		    	}/* end scanline loop */
		    break;

		    case UdpK_DTypeWU:		    /* long dst, word src */
		    src_word_ptr = (unsigned short *) src_udp->UdpA_Base + 
			(src_udp->UdpL_Pos>>4) + (src_udp->UdpL_ScnStride>>4) *
			     src_udp->UdpL_Y1 +	src_udp->UdpL_X1;
		    src_line_pad = 
			(src_udp->UdpL_ScnStride>>4) - src_udp->UdpL_PxlPerScn;
		
		    for (line_idx = 0; line_idx < line_cnt; line_idx++)
		    	{
			for (pixel_idx = 0; pixel_idx < pixel_cnt; pixel_idx++)
			    {
			    *dst_long_ptr |= (*src_word_ptr++ << shift[ix]);
			    dst_long_ptr++;
 		    	    }/* end pixel loop */
		    	src_word_ptr += src_line_pad;
			dst_long_ptr += dst_line_pad;
		    	}/* end scanline loop */
		    break;

		    case UdpK_DTypeLU:		    /* long dst, long src */
		    src_long_ptr = (unsigned int *) src_udp->UdpA_Base + 
			(src_udp->UdpL_Pos>>5) + (src_udp->UdpL_ScnStride>>5) *
			     src_udp->UdpL_Y1 +	src_udp->UdpL_X1;
		    src_line_pad = 
			(src_udp->UdpL_ScnStride>>5) - src_udp->UdpL_PxlPerScn;
		
		    for (line_idx = 0; line_idx < line_cnt; line_idx++)
		    	{
			for (pixel_idx = 0; pixel_idx < pixel_cnt; pixel_idx++)
			    {
			    *dst_long_ptr |= (*src_long_ptr++ << shift[ix]);
			    dst_long_ptr++;
 		    	    }/* end pixel loop */
		    	src_long_ptr += src_line_pad;
			dst_long_ptr += dst_line_pad;
		    	}/* end scanline loop */
		    break;
		} /* end of src switch */
	    }	/* end of number of planes loop */
	break;
	}	/* end of dst data type switch */
			    
    return (IpsX_SUCCESS);
    } /* end of _IpsMergePlanes*/
