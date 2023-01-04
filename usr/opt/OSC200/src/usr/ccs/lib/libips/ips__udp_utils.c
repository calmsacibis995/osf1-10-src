/*  DEC/CMS REPLACEMENT HISTORY, Element IPS__UDP_UTILS.C */
/*  *10   11-SEP-1990 15:08:36 SORNSON "modifying CopyUdp routine for case when src and dst arrays are line and pixel packed, but */
/* the src and dst pixel counts are different" */
/*  *9    14-AUG-1990 11:29:46 PICCOLO "reverse error checking in BuildDstUdp to provide a more meaningful error message" */
/*  *8    14-AUG-1990 11:23:29 PICCOLO "add comments" */
/*  *7    10-AUG-1990 16:47:50 PICCOLO "improve verifynotinplace" */
/*  *6     9-AUG-1990 15:22:59 PICCOLO "pos change - new verifynotinplace" */
/*  *5     3-AUG-1990 16:16:57 PICCOLO "new min rect function and new pos logic" */
/*  *4    29-JUN-1990 11:37:32 RODWELL "add memory init flag" */
/*  *3    25-JUN-1990 16:37:00 PICCOLO "take out references to byte, word etc." */
/*  *2     6-JUN-1990 13:26:15 PICCOLO "add createudp routine and changed unsigned long to long" */
/*  *1     1-JUN-1990 16:22:03 PICCOLO "IPS Base Level" */
/*  DEC/CMS REPLACEMENT HISTORY, Element IPS__UDP_UTILS.C */
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
**  IPS__UDP_UTILS
**
**  FACILITY:
**
**	Image Processing Services (IPS)
**
**  ABSTRACT:
**
**	This module contains low level utility functions that operate
**	on attributes and image data that are described using the
**	UDP descriptor.
**
**  ENVIRONMENT:
**
**	VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
**	Mark Sornson
**	Richard Piccolo (latter routines)
**
**  CREATION DATE:
**
**	4-OCT-1989
**
************************************************************************/

/*
**  Table of contents:
**
**	Global routines
*/
#ifdef NODAS_PROTO
void	_IpsSetUdpClassAndDType();
long	_IpsBuildDstUdp();		/* fill dst udp based on arguments    */
long	_IpsCreateUdp();		/* create udp based on arguments      */
long	_IpsMinRectUdp();		/* find smallest ROI for non-zero val */
long	_IpsVerifyNotInPlace();		/* verify not in place		      */
long	_IpsGetStartAddress();		/* determine start address	      */
#endif
		    

/*
**  Include files:
*/
#include    <IpsDef.h>
#include    <IpsDefP.h>
#include    <IpsStatusCodes.h>
#include    <IpsMemoryTable.h>
#ifndef NODAS_PROTO
#include <ipsprot.h>				    /* Ips prototypes */
#endif


/******************************************************************************
**  _IpsSetUdpClassAndDtype
**
**  FUNCTIONAL DESCRIPTION:
**
**	Set the class and dtype fields in the udp according to the data
**	type passed in.
**
**  FORMAL PARAMETERS:
**
**	udp	    UDP to set up.  Passed by reference.
**
**	data_type   Data type of data.  Unsigned longword, by value.
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
**	void (none)
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
void _IpsSetUdpClassAndDType( udp, data_type )
struct UDP  *udp;
long	     data_type;
{

switch ( data_type )
    {
    case IpsK_DTypeIntBit:
    case IpsK_DTypeIntBits:
    case IpsK_DTypeIntBitsU:
	/*
	** Note that the distinction between aligned and unaligned bitfields
	** is not made
	*/
	udp->UdpB_Class = UdpK_ClassUBA;
	udp->UdpB_DType = UdpK_DTypeVU;
	break;
    case IpsK_DTypeIntByte:
	udp->UdpB_Class = UdpK_ClassA;
	udp->UdpB_DType = UdpK_DTypeBU;	/* NOTE: should be signed */
	break;
    case IpsK_DTypeBU:
    case IpsK_DTypeIntByteU:
	udp->UdpB_Class = UdpK_ClassA;
	udp->UdpB_DType = UdpK_DTypeBU;
	break;
    case IpsK_DTypeIntWord:
	udp->UdpB_Class = UdpK_ClassA;
	udp->UdpB_DType = UdpK_DTypeWU;	/* NOTE: should be signed */
	break;
    case IpsK_DTypeWU:
    case IpsK_DTypeIntWordU:
	udp->UdpB_Class = UdpK_ClassA;
	udp->UdpB_DType = UdpK_DTypeWU;
	break;
    case IpsK_DTypeIntLongword:
	udp->UdpB_Class = UdpK_ClassA;
	udp->UdpB_DType = UdpK_DTypeLU;	/* NOTE: should be signed */
	break;
    case IpsK_DTypeLU:
    case IpsK_DTypeIntLongwordU:
	udp->UdpB_Class = UdpK_ClassA;
	udp->UdpB_DType = UdpK_DTypeLU;
	break;
    case IpsK_DTypeBitstream:
    default:
	udp->UdpB_Class = UdpK_ClassUBS;
	udp->UdpB_DType = UdpK_DTypeVU;
	break;
    } /* end switch */

return;
} /* end of _IpsSetUdpClassAndDType */

/******************************************************************************
**  _IpsBuildDstUdp()
**
**  FUNCTIONAL DESCRIPTION:
**
**  Defines dst udp fields based on src udp and flags
**
**  FORMAL PARAMETERS:
**
**	srcudp		Source buffer UDP.  Passed by reference.
**	dstudp		Destination buffer UDP.  Passed by reference.
**	mem flag	indicating whether to init memory to 0 (IpsM_InintMem)
**	retain flag	indicating whether to retain source outer dimensions
**	in-place flag	indicating whether an in-place operation is allowed
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	dst udp
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
long _IpsBuildDstUdp (src, dst, init_flag, retain_flag, in_place)
struct UDP *src;			/* source udp			      */
struct UDP *dst;			/* destination udp		      */
unsigned long init_flag;		/* 0 or IpsM_MemInit		      */
unsigned long retain_flag;		/* 0 or IpsM_RetainSrcDim	      */
unsigned long in_place;			/* 0 or IpsK_InPlaceAllowed	      */
{
    long status;
    unsigned long size;
    dst->UdpW_PixelLength = src->UdpW_PixelLength;
    dst->UdpB_DType = src->UdpB_DType;
    dst->UdpB_Class = src->UdpB_Class;
    dst->UdpL_PxlPerScn = src->UdpL_PxlPerScn;
    dst->UdpL_ScnCnt = src->UdpL_ScnCnt;
    dst->UdpL_PxlStride = src->UdpL_PxlStride;
    dst->UdpL_Levels = src->UdpL_Levels;
    dst->UdpL_CompIdx = src->UdpL_CompIdx;

    if (retain_flag & IpsM_RetainSrcDim)
	/* if retain, keep the same as the source */
	{
	dst->UdpL_X1 = src->UdpL_X1;
	dst->UdpL_Y1 = src->UdpL_Y1;
	dst->UdpL_X2 = src->UdpL_X2;
	dst->UdpL_Y2 = src->UdpL_Y2;
	dst->UdpL_ScnStride = src->UdpL_ScnStride;
	size = src->UdpL_ArSize - src->UdpL_Pos;
	if (dst->UdpA_Base != 0)
	    {
	    if (in_place != IpsK_InPlaceAllowed) 
		{
		status = _IpsVerifyNotInPlace (src, dst);
		if (status != IpsX_SUCCESS) return (status);
		}
	    if ((dst->UdpL_ArSize - dst->UdpL_Pos) < size) 
		return (IpsX_INSVIRMEM);
	    }
	else
	    {
	    /* zero the allocation for retain regardless of the mem flag */
	    dst->UdpA_Base = (unsigned char *)
		(*IpsA_MemoryTable[IpsK_AllocateDataPlane])
		    (((size + 7)>>3), IpsM_InitMem, 0);
	    if (!dst->UdpA_Base) return (IpsX_INSVIRMEM);
	    dst->UdpL_ArSize = size;
	    dst->UdpL_Pos = 0;
	    }
	}
    else
	/* don't retain source outer dimensions - define new region */
	{
	dst->UdpL_X1 = 0;
	dst->UdpL_Y1 = 0;
	dst->UdpL_X2 = src->UdpL_PxlPerScn - 1;
	dst->UdpL_Y2 = src->UdpL_ScnCnt - 1;
	dst->UdpL_ScnStride = dst->UdpL_PxlPerScn * dst->UdpL_PxlStride;
	/*
	** Adjust to byte alignment if bitonal 
	*/
	if (dst->UdpB_DType == UdpK_DTypeVU) 
            dst->UdpL_ScnStride += (8 - (dst->UdpL_ScnStride % 8)) % 8;

	size = dst->UdpL_ScnCnt * dst->UdpL_ScnStride;
	if (dst->UdpA_Base != 0)
	    {
            /*
            **  After size verification, use already allocated
            **  destination memory.
            */
	    if (in_place != IpsK_InPlaceAllowed) 
		{
		status = _IpsVerifyNotInPlace (src, dst);
		if (status != IpsX_SUCCESS) return (status);
		}
	    if ((dst->UdpL_ArSize - dst->UdpL_Pos) < size)
		return (IpsX_INSVIRMEM);
	    }
	else
	    {
	    /*
	    **	Allocate destination memory.
	    **
	    **	NOTE:	if src data plane size is less than the
	    **		sourc ArSize, the source is probably
	    **		compressed, which means that the destination
	    **		size must be likewise reduced (from full PCM
	    **		size).
	    */
	    unsigned long   srcSizeBits;
	    unsigned long   srcSizeBytes;
	    unsigned long   *srcSizePtr;

	    /* get actual src size in bytes from bytecount prefix in data plane */
	    srcSizePtr = src->UdpA_Base - sizeof(srcSizeBytes);
	    srcSizeBytes = *srcSizePtr;

	    /* figure out PCM size in bits from attributes  */
	    srcSizeBits = src->UdpL_Pos + (src->UdpL_ScnCnt * src->UdpL_ScnStride);
	
	    if ( (srcSizeBytes * 8) < srcSizeBits )
		size = srcSizeBytes * 8;

	    dst->UdpA_Base = (unsigned char *)
		(*IpsA_MemoryTable[IpsK_AllocateDataPlane])
		    (((size + 7) >> 3),init_flag,0);
	    if (!dst->UdpA_Base) return (IpsX_INSVIRMEM);
	    dst->UdpL_ArSize = size;
	    dst->UdpL_Pos = 0;
	    }
	}
return (IpsX_SUCCESS);
}

/******************************************************************************
**  _IpsCreateUdp()
**
**  FUNCTIONAL DESCRIPTION:
**
**  Creates udp and defines the udp fields based on the arguments
**
**  FORMAL PARAMETERS:
**
**	x_size	    Outer x size
**	y_size	    Outer y size
**	x_sub_size  Inner x size
**	y_sub_size  Inner y size
**	x_offset    X offset into inner rectangle (zero based from upper left)
**	y_offset    y offset into inner rectangle (zero based from upper left)
**	dtype	    data type of udp (IpsK_DType*)
**	dst_udp	    the address of the resultant udp pointer
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	dstudp	address of the pointer to the destination UDP.  
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
long _IpsCreateUdp (x_size, y_size, x_sub_size, y_sub_size, x_offset,
			   y_offset, dtype, dst_udp)
unsigned long x_size;			/* parent x size		     */
unsigned long y_size;			/* parent y size		     */
unsigned long x_sub_size;		/* sub rectangle x size		     */
unsigned long y_sub_size;		/* sub rectangle y size		     */
unsigned long x_offset;			/* x offset of sub rectangle	     */
unsigned long y_offset;			/* y offset of sub rectangle	     */
unsigned long dtype;			/* desired dst udp datat type	     */
struct   UDP **dst_udp;			/* address of the udp pointer	     */
{
    struct   UDP *dst;			
    if (x_size == 0 || y_size == 0) return (IpsX_INVDARG);

    /* if no sub region (i.e. either sub size set to 0) set accordingly */
    if (x_sub_size == 0 || y_sub_size == 0)
	{
	x_sub_size = x_size;
	x_offset = 0;
	y_sub_size = y_size;
	y_offset = 0;
	}

    if (x_sub_size + x_offset > x_size || y_sub_size + y_offset > y_size)
	return (IpsX_INCNSARG);

    dst = (struct UDP *) (IpsA_MemoryTable[IpsK_Alloc]) 
	(sizeof(struct UDP),0,0);
    if (!dst) return (IpsX_INSVIRMEM);
    *dst_udp = dst;			/* set caller's ptr to allocated udp */

    switch (dtype)    
	{
	case IpsK_DTypeVU:
	    dst->UdpW_PixelLength = 1;
	    dst->UdpB_DType = UdpK_DTypeVU;
	    dst->UdpB_Class = UdpK_ClassUBA;
	    dst->UdpL_Levels = 2;
	break;

	case IpsK_DTypeBU:
	    dst->UdpW_PixelLength = 8;
	    dst->UdpB_DType = UdpK_DTypeBU;
	    dst->UdpB_Class = UdpK_ClassA;
	    dst->UdpL_Levels = 256;
	break;

	case IpsK_DTypeWU:
	    dst->UdpW_PixelLength = 16;
	    dst->UdpB_DType = UdpK_DTypeWU;
	    dst->UdpB_Class = UdpK_ClassA;
	    dst->UdpL_Levels = 65536;
	break;

	case IpsK_DTypeLU:
	    dst->UdpW_PixelLength = 32;
	    dst->UdpB_DType = UdpK_DTypeLU;
	    dst->UdpB_Class = UdpK_ClassA;
	    dst->UdpL_Levels = 0xFFFFFFFF; /* max allowed; defined as signed */
	break;

	case IpsK_DTypeF:
	    dst->UdpW_PixelLength = 32;
	    dst->UdpB_DType = UdpK_DTypeF;
	    dst->UdpB_Class = UdpK_ClassA;
	    dst->UdpL_Levels = 0;
	break;

	case IpsK_DTypeC:
	    dst->UdpW_PixelLength = 64;
	    dst->UdpB_DType = UdpK_DTypeC;
	    dst->UdpB_Class = UdpK_ClassA;
	    dst->UdpL_Levels = 0;
	break;
	
	default:
	    (*IpsA_MemoryTable[IpsK_Dealloc])(dst);
	    return (IpsX_INVDTYPE);
	break;
    }

    dst->UdpL_PxlPerScn = x_sub_size;
    dst->UdpL_ScnCnt = y_sub_size;
    dst->UdpL_PxlStride = dst->UdpW_PixelLength;
    dst->UdpL_CompIdx = 0;
    dst->UdpL_X1 = x_offset;
    dst->UdpL_Y1 = y_offset;
    dst->UdpL_X2 = dst->UdpL_X1 + dst->UdpL_PxlPerScn - 1;
    dst->UdpL_Y2 = dst->UdpL_Y1 + dst->UdpL_ScnCnt - 1;
    dst->UdpL_ScnStride = x_size * dst->UdpL_PxlStride;
    dst->UdpL_Pos = 0;		/* don't allow preamble */
    dst->UdpL_ArSize = y_size * dst->UdpL_ScnStride;
    dst->UdpA_Base = (unsigned char *)
	(*IpsA_MemoryTable[IpsK_AllocateDataPlane]) 
	    (((dst->UdpL_ArSize + 7)>>3), IpsM_InitMem, 0);
    if (!dst->UdpA_Base)
	{
	(*IpsA_MemoryTable[IpsK_Dealloc])(dst);
	return (IpsX_INSVIRMEM);
	}
    else
	return (IpsX_SUCCESS);
}

/******************************************************************************
**  _IpsMinRectUdp()
**
**  FUNCTIONAL DESCRIPTION:
**
**  Adjusts src udp fields to reflect the smallest rectangle to enclose
**  non zero values
**
**  FORMAL PARAMETERS:
**
**	srcudp	Source buffer UDP.  Passed by reference.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	dst udp
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
long _IpsMinRectUdp (src)
struct UDP *src;			/* source udp			      */
{
    unsigned long i, j ,k;
    unsigned long xmin, xmax, ymin, ymax;
    unsigned char   *byte_ptr;
    unsigned short  *word_ptr;
    unsigned long   *long_ptr;
    float	    *float_ptr;
    COMPLEX	    *complex_ptr;
    unsigned long offset;
    unsigned long test;

    xmin = src->UdpL_X2;
    xmax = 0;
    ymin = src->UdpL_Y2;
    ymax = 0;

    switch (src->UdpB_DType)    
	{
	case IpsK_DTypeVU:
	    byte_ptr = (unsigned char *)src->UdpA_Base + (src->UdpL_Pos >> 3);
	    /*
	    ** Get to beginning of line of 1st line in ROI
	    */
	    offset = src->UdpL_ScnStride * src->UdpL_Y1;
	    for (j = src->UdpL_Y1; j <= src->UdpL_Y2; j++)
		{
		for (i = src->UdpL_X1; i <= src->UdpL_X2; i++)
		    {
		    if (((*(byte_ptr + ((offset + i) >> 3))) & 
			(1 << (i & 7))) != 0)
			{
			if (i < xmin) xmin = i;
			if (i > xmax) xmax = i;
			if (j < ymin) ymin = j;
			if (j > ymax) ymax = j;
			}
		    }
		offset += src->UdpL_ScnStride;
		}
	break;

	case IpsK_DTypeBU:
	    byte_ptr = (unsigned char *)src->UdpA_Base + (src->UdpL_Pos >> 3);
	    byte_ptr += ((src->UdpL_ScnStride >>3) * src->UdpL_Y1 + 
		src->UdpL_X1); 
	    for (j = src->UdpL_Y1; j <= src->UdpL_Y2; j++)
		{
		for (i = 0; i < src->UdpL_PxlPerScn; i++)
		    {
		    if (*(byte_ptr + i) != 0)
			{
			if (i < xmin) xmin = i;
			if (i > xmax) xmax = i;
			if (j < ymin) ymin = j;
			if (j > ymax) ymax = j;
			}
		    }
		byte_ptr += (src->UdpL_ScnStride>>3);
		}
	xmin += src->UdpL_X1;
	xmax += src->UdpL_X1;
	break;

	case IpsK_DTypeWU:
	    word_ptr = (unsigned short *)src->UdpA_Base + (src->UdpL_Pos >> 4);
	    word_ptr += ((src->UdpL_ScnStride >>4) * src->UdpL_Y1 + 
		src->UdpL_X1); 
	    for (j = src->UdpL_Y1; j <= src->UdpL_Y2; j++)
		{
		for (i = 0; i < src->UdpL_PxlPerScn; i++)
		    {
		    if (*(word_ptr + i) != 0)
			{
			if (i < xmin) xmin = i;
			if (i > xmax) xmax = i;
			if (j < ymin) ymin = j;
			if (j > ymax) ymax = j;
			}
		    }
		word_ptr += (src->UdpL_ScnStride>>3);
		}
	xmin += src->UdpL_X1;
	xmax += src->UdpL_X1;
	break;

	case IpsK_DTypeLU:
	    long_ptr = (unsigned long *)src->UdpA_Base + (src->UdpL_Pos >> 5);
	    long_ptr += ((src->UdpL_ScnStride >>5) * src->UdpL_Y1 + 
		src->UdpL_X1); 
	    for (j = src->UdpL_Y1; j <= src->UdpL_Y2; j++)
		{
		for (i = 0; i < src->UdpL_PxlPerScn; i++)
		    {
		    if (*(long_ptr + i) != 0)
			{
			if (i < xmin) xmin = i;
			if (i > xmax) xmax = i;
			if (j < ymin) ymin = j;
			if (j > ymax) ymax = j;
			}
		    }
		long_ptr += (src->UdpL_ScnStride>>5);
		}
	xmin += src->UdpL_X1;
	xmax += src->UdpL_X1;
	break;

	case IpsK_DTypeF:
	    float_ptr = (float *)src->UdpA_Base + (src->UdpL_Pos >> 5);
	    float_ptr += ((src->UdpL_ScnStride >>5) * src->UdpL_Y1 + 
		src->UdpL_X1); 
	    for (j = src->UdpL_Y1; j <= src->UdpL_Y2; j++)
		{
		for (i = 0; i < src->UdpL_PxlPerScn; i++)
		    {
		    if (*(float_ptr + i) != 0)
			{
			if (i < xmin) xmin = i;
			if (i > xmax) xmax = i;
			if (j < ymin) ymin = j;
			if (j > ymax) ymax = j;
			}
		    }
		float_ptr += (src->UdpL_ScnStride>>5);
		}
	xmin += src->UdpL_X1;
	xmax += src->UdpL_X1;
	break;

	case IpsK_DTypeC:
	    complex_ptr = (COMPLEX *)src->UdpA_Base + (src->UdpL_Pos >> 6);
	    complex_ptr += ((src->UdpL_ScnStride >>6) * src->UdpL_Y1 + 
		src->UdpL_X1); 
	    for (j = src->UdpL_Y1; j <= src->UdpL_Y2; j++)
		{
		for (i = 0; i < src->UdpL_PxlPerScn; i++)
		    {
		    if (((complex_ptr + i)->real != 0.0) &&
			((complex_ptr + i)->imag != 0.0))
			{
			if (i < xmin) xmin = i;
			if (i > xmax) xmax = i;
			if (j < ymin) ymin = j;
			if (j > ymax) ymax = j;
			}
		    }
		complex_ptr += (src->UdpL_ScnStride>>6);
		}
	xmin += src->UdpL_X1;
	xmax += src->UdpL_X1;
	break;
	
	default:
	    return (IpsX_INVDTYPE);
	break;
    }

    /* adjust src fields */
	   
    src->UdpL_X2 = xmax;
    src->UdpL_X1 = xmin;
    src->UdpL_Y2 = ymax;
    src->UdpL_Y1 = ymin;
    src->UdpL_PxlPerScn = src->UdpL_X2 - src->UdpL_X1 + 1;
    src->UdpL_ScnCnt = src->UdpL_Y2 - src->UdpL_Y1 + 1;

    return (IpsX_SUCCESS);
}


/******************************************************************************
**  _IpsVerifyNotInPlace ()
**
**  FUNCTIONAL DESCRIPTION:
**
**  Checks to make sure that the src starting address does not equal dst
**  starting address.  If it does, return (IpsX_NOINPLACE)
**
**  Note : src and dst udp must be completely filled in 
**
**  FORMAL PARAMETERS:
**
**	srcudp		Source buffer UDP.  Passed by reference.
**	dstudp		Destination buffer UDP.  Passed by reference.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	dst udp
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
long _IpsVerifyNotInPlace (src, dst)
struct UDP *src;			/* source udp			      */
struct UDP *dst;			/* destination udp		      */
{
unsigned long src_addr;
unsigned long dst_addr;
long	      status;

status = _IpsGetStartAddress (src, &src_addr);
if (status != IpsX_SUCCESS) return (status);
status = _IpsGetStartAddress (dst, &dst_addr);
if (status != IpsX_SUCCESS) return (status);

if (src_addr == dst_addr)
    return (IpsX_NOINPLACE);
return (IpsX_SUCCESS);
}

/******************************************************************************
**  _IpsGetStartAddress ()
**
**  FUNCTIONAL DESCRIPTION:
**
**  Determines starting address of data based on udp
**
**  Note: udp must be completely filled in 
**
**  FORMAL PARAMETERS:
**
**	srcudp		Source buffer UDP.  Passed by reference.
**	start_addr	Returned starting address stored as a longword
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	dst udp
**
**  FUNCTION VALUE:
**
**	status 
**
******************************************************************************/
long _IpsGetStartAddress (udp, start_addr)
struct UDP *udp;			/* source udp			      */
unsigned long *start_addr;
    {
    switch (udp->UdpB_DType)
	{
	case UdpK_DTypeVU:		    
	    /* nearest byte boundary */
	    *start_addr = (unsigned long) (udp->UdpA_Base + ((udp->UdpL_Pos + 
		udp->UdpL_Y1 * udp->UdpL_ScnStride + udp->UdpL_X1) >> 3));
	break;

	case UdpK_DTypeBU:	    
	    *start_addr = (unsigned long) (udp->UdpA_Base + 
		(udp->UdpL_Pos >> 3) + 	udp->UdpL_Y1 * 
		    (udp->UdpL_ScnStride >> 3) + udp->UdpL_X1);
	break;

	case UdpK_DTypeWU:
	    *start_addr = (unsigned long) ((unsigned short *)udp->UdpA_Base + 
		(udp->UdpL_Pos >> 4) + 	udp->UdpL_Y1 * 
		    (udp->UdpL_ScnStride >> 4) + udp->UdpL_X1);
	break;

	case UdpK_DTypeLU:	    
	case UdpK_DTypeF:
	    *start_addr = (unsigned long)((unsigned long *) udp->UdpA_Base + 
		(udp->UdpL_Pos >> 5) + udp->UdpL_Y1 * 
		    (udp->UdpL_ScnStride >> 5) + udp->UdpL_X1);
	break;

	case UdpK_DTypeC:
	    *start_addr = (unsigned long)((COMPLEX *) udp->UdpA_Base + 
		(udp->UdpL_Pos >> 6) + udp->UdpL_Y1 * 
		    (udp->UdpL_ScnStride >> 6) + udp->UdpL_X1);
	break;

	default:
	    return (IpsX_INVDTYPE);
	break;
    }   /* end of switch */

    return (IpsX_SUCCESS);
}
