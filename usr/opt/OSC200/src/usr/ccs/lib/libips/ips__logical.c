/************************************************************************
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
**      Image Processing Services
**
**  ABSTRACT:
**
**      This module contains the user level service and support routines
**	for logical operations on two input images.
**      It generates a destination image of the same data type as the source.
**
**  ENVIRONMENT:
**
**      VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
**      Karen Rodwell
**
**  CREATION DATE:
**
**      25-MAY-1990
**
************************************************************************/

/*
**  Table of contents
*/
#ifdef NODAS_PROTO
long _IpsLogical();	    /* main entry routine */
#endif

/*
**  Include files
*/
#include <IpsDef.h>			    /* Ips Image Definitions	     */
#include <IpsMacros.h>			    /* Ips Macro Definitions	     */
#include <IpsMemoryTable.h>                 /* Ips Memory Mgt Functions      */
#ifndef NODAS_PROTO
#include <ipsprot.h>			    /* IPS Prototypes		    */
#endif
#include <IpsStatusCodes.h>		    /* Ips Status Codes		     */

/*
**  External references
*/
long _IpsBuildDstUdp();                            /* from ips__udp_utils.c */

/*
**  Local routines
*/
#ifdef NODAS_PROTO
long _BufferCombineByte();
long _BufferCombineWord();
long _BufferCombineLong();
#endif

/*****************************************************************************
**  _IpsLogical - 
**
**  FUNCTIONAL DESCRIPTION:
**	Performs logic operations on corresponding values from the 
**      two source images to produce results for the destination image on 
**      a point by point basis.  
**
**  FORMAL PARAMETERS:
**
**      src1_udp    -- Pointer to first source udp
**	src2_udp    -- Pointer to second source udp
**	dst_udp     -- Pointer to destination udp (uninitialized)
**      cpp	    -- Control Processing Plane
**	operator    -- Logical operation to be performed on two planes
**
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   none
**  SIGNAL CODES:     none
**  SIDE EFFECTS:     none
**
************************************************************************/
long _IpsLogical(src1_udp, src2_udp, dst_udp, cpp, operator)
struct UDP *src1_udp;      /* First Source UDP for the logical operation  */
struct UDP *src2_udp;      /* Second Source UDP for the logical operation */
struct UDP *dst_udp;       /* Destination UDP generated by logic op       */
struct UDP *cpp;           /* Control Processsing plane for logic op      */
unsigned long operator;	   /* Function operator flag                      */
{
long bit_count;
long dst_offset;
long scanline;
long scanline_count;
long src1_offset;
long src2_offset;

unsigned long mem_allocated;   /* flag to indicate memory was allocated here*/
long status;

if ((operator < IpsK_LogicalMin) || (operator > IpsK_LogicalMax))
    return (IpsX_INVDARG);
 
if  (src1_udp->UdpB_Class != UdpK_ClassA)
    return (IpsX_UNSOPTION);

/* return mismatch error if the dimensions are mismatched */

if ((src1_udp->UdpL_ScnCnt != src2_udp->UdpL_ScnCnt) ||
    (src1_udp->UdpL_PxlPerScn != src2_udp->UdpL_PxlPerScn) ||
    (src1_udp->UdpB_DType != src2_udp->UdpB_DType))
    return (IpsX_NOMATCH); 

if ((src1_udp->UdpB_DType == UdpK_DTypeVU) || 
    (src2_udp->UdpB_DType == UdpK_DTypeVU) ||
    (src1_udp->UdpB_DType == UdpK_DTypeV) ||
    (src2_udp->UdpB_DType == UdpK_DTypeV))
    return (IpsX_INVDTYPE);

if (dst_udp->UdpA_Base == 0)
    mem_allocated = TRUE;
status = _IpsBuildDstUdp
	    (src1_udp, dst_udp, 0, IpsM_RetainSrcDim, IpsK_InPlaceAllowed);
if (status != IpsX_SUCCESS) 
    return (status);
 
/*
** Calculate offsets and number of scanlines to process
*/
scanline_count = src1_udp->UdpL_ScnCnt;
src1_offset = src1_udp->UdpL_Pos + (src1_udp->UdpL_X1 * 
    src1_udp->UdpL_PxlStride) + (src1_udp->UdpL_ScnStride * src1_udp->UdpL_Y1);
src2_offset = src2_udp->UdpL_Pos + (src2_udp->UdpL_X1 * 
    src2_udp->UdpL_PxlStride) + (src2_udp->UdpL_ScnStride * src2_udp->UdpL_Y1);
dst_offset = dst_udp->UdpL_Pos + (dst_udp->UdpL_X1 * 
    dst_udp->UdpL_PxlStride) + (dst_udp->UdpL_ScnStride * dst_udp->UdpL_Y1);
bit_count = src1_udp->UdpL_PxlPerScn * src1_udp->UdpL_PxlStride;

switch (src1_udp->UdpB_DType)
    {
    case UdpK_DTypeBU:
	for (scanline = 0;  scanline < scanline_count; scanline++)
	    {
	    _BufferCombineByte(
	           bit_count,				  /* Bit count      */
	           src1_offset,				  /* src1 bit offset*/
	           src1_udp->UdpA_Base,                   /* buffer address */
		   src2_offset,				  /* bit offset     */
	           src2_udp->UdpA_Base,                   /* buffer address */
	           dst_offset,				  /* bit offset     */
	           dst_udp->UdpA_Base,		          /* buffer adr     */
	           operator);
	    src1_offset += src1_udp->UdpL_ScnStride;
	    src2_offset += src2_udp->UdpL_ScnStride;
	    dst_offset += dst_udp->UdpL_ScnStride;
	    }/* end scanline loop */
        break;
    case UdpK_DTypeWU:
	for (scanline = 0;  scanline < scanline_count; scanline++)
	    {
	    _BufferCombineWord(
	           bit_count,				  /* Bit count      */
	           src1_offset,				  /* src1 bit offset*/
	           src1_udp->UdpA_Base,                   /* buffer address */
		   src2_offset,				  /* bit offset     */
	           src2_udp->UdpA_Base,                   /* buffer address */
	           dst_offset,				  /* bit offset     */
	           dst_udp->UdpA_Base,		          /* buffer adr     */
	           operator);
	    src1_offset += src1_udp->UdpL_ScnStride;
	    src2_offset += src2_udp->UdpL_ScnStride;
	    dst_offset += dst_udp->UdpL_ScnStride;
	    }/* end scanline loop */
        break;
    case UdpK_DTypeLU: 
	for (scanline = 0;  scanline < scanline_count; scanline++)
	    {
	    _BufferCombineLong(
	           bit_count,				  /* Bit count      */
	           src1_offset,				  /* src1 bit offset*/
	           src1_udp->UdpA_Base,                   /* buffer address */
		   src2_offset,				  /* bit offset     */
	           src2_udp->UdpA_Base,                   /* buffer address */
	           dst_offset,				  /* bit offset     */
	           dst_udp->UdpA_Base,		          /* buffer adr     */
	           operator);
	    src1_offset += src1_udp->UdpL_ScnStride;
	    src2_offset += src2_udp->UdpL_ScnStride;
	    dst_offset += dst_udp->UdpL_ScnStride;
	    }/* end scanline loop */
        break;
    case UdpK_DTypeF:
        if (mem_allocated == TRUE)
            (*IpsA_MemoryTable[IpsK_FreeDataPlane])
			    (dst_udp->UdpA_Base);
	return (IpsX_UNSOPTION);
	break;
    case UdpK_DTypeVU:
    default:
        if (mem_allocated == TRUE)
            (*IpsA_MemoryTable[IpsK_FreeDataPlane])
			    (dst_udp->UdpA_Base);
        return (IpsX_INVDTYPE);
        break;
    };  /* end switch on DType */
return (IpsX_SUCCESS);
}

/************************************************************************
**
**  _BufferCombineByte 
**
**  FUNCTIONAL DESCRIPTION:
**
**  Performs a logical point combination on two non-bitonal input images.
**
**  FORMAL PARAMETERS:
**
    long		size;		* number of bits to combine	     *
    long		src1_addr;	* base address of source buf         *
    long		src1_pos;	* starting offset in source buf      *
    long		src2_addr;	* base address of source buf         *
    long		src2_pos;	* starting offset in source buf      *
    long		dst_addr;	* base address of destination buf    *
    long		dst_pos;	* starting offset in destination buf *
    long		rule;		* combination rule                   *
**
**  IMPLICIT INPUTS:  None
**  IMPLICIT OUTPUTS: None
**  FUNCTION VALUE:   None
**  SIGNAL CODES:     None
**  SIDE EFFECTS:     None
**
************************************************************************/

long _BufferCombineByte(size,src1_pos,src1_addr,src2_pos,src2_addr,dst_pos,dst_addr, rule)
long		size;		/* size of bits to be combined         */
long		src1_pos;	/* starting offset in source1 buf      */
unsigned char	*src1_addr;	/* base address of source1 buf         */
long		src2_pos;	/* starting offset in source2 buf      */
unsigned char	*src2_addr;	/* base address of source2 buf         */
long		dst_pos;	/* starting offset in destination buf  */
unsigned char	*dst_addr;	/* base address of destination buf     */
long		rule;		/* combination rule                    */
{
unsigned char 	*src1;		/* current source1 buf word            */
unsigned char 	*src2;		/* current source2 buf word            */
unsigned char 	*dst;		/* current destination buf word        */
long		nbits;		/* number of bits to copy this time    */
unsigned long 	src1_word;	/* the aligned source1 word            */
unsigned long 	src2_word;	/* the aligned source2 word            */

/*
* obtain pointers to the source and destination buffers
*/

dst = (unsigned char *) dst_addr + dst_pos / BYTE_SIZE;
src1 = (unsigned char *) src1_addr + src1_pos / BYTE_SIZE;
src2 = (unsigned char *) src2_addr + src2_pos / BYTE_SIZE;
src1_pos = src1_pos % BYTE_SIZE;
src2_pos = src2_pos % BYTE_SIZE;
dst_pos = dst_pos % BYTE_SIZE;

/*
** Combine the largest portion of the longword possible until
** the entire bit string is combined
*/

/*
** apply the rule
*/

switch (rule)
     {
     case IpsK_Src1AndSrc2:
	    for(;size > 0; src1++, dst++, src2++, size -= BYTE_SIZE)
		*dst =  *src1 & *src2;
     break;
    
     case IpsK_Src1AndNotSrc2:
	    for(;size > 0; src1++, dst++, src2++, size -= BYTE_SIZE)
		*dst = (*src1) & (~*src2);
     break;
    
     case IpsK_NotSrc1AndSrc2:
	    for(;size > 0; src1++, dst++, src2++, size -= BYTE_SIZE)
		*dst = (~*src1) & *src2;
     break;
    
     case IpsK_Src1XorSrc2:
	    for(;size > 0; src1++, dst++, src2++, size -= BYTE_SIZE)
		*dst = (*src1) ^ (*src2);
     break;
    
     case IpsK_Src1OrSrc2:
	    for(;size > 0; src1++, dst++, src2++, size -= BYTE_SIZE)
		*dst = *src1 | *src2;
     break;
    
     case IpsK_NotSrc1AndNotSrc2:
	    for(;size > 0; src1++, dst++, src2++, size -= BYTE_SIZE)
		*dst = (~*src1) & (~*src2);
     break;
    
     case IpsK_NotSrc1XorSrc2:
	    for(;size > 0; src1++, dst++, src2++, size -= BYTE_SIZE)
		*dst = (~*src1) ^ (*src2);
     break;
    
     case IpsK_Src1OrNotSrc2:
	    for(;size > 0; src1++, dst++, src2++, size -= BYTE_SIZE)
		*dst = (*src1) | (~*src2);
     break;
    
     case IpsK_NotSrc1OrSrc2:
	    for(;size > 0; src1++, dst++, src2++, size -= BYTE_SIZE)
		*dst = (~*src1) | (*src2);
     break;
    
     case IpsK_NotSrc1OrNotSrc2:
	    for(;size > 0; src1++, dst++, src2++, size -= BYTE_SIZE)
		*dst = (~*src1) | (~*src2);
     break;
    
     case IpsK_SetToSrc2:
	    for(;size > 0; src1++, dst++, src2++, size -= BYTE_SIZE)
		*dst = *src2;
     break;

     case IpsK_SetToNotSrc2:
	    for(;size > 0; src1++, dst++, src2++, size -= BYTE_SIZE)
		*dst = (~*src2);
     break;

     case IpsK_SetSrc1:
	    for(;size > 0; src1++, dst++, src2++, size -= BYTE_SIZE)
		*dst = *src1;
     break;

     case IpsK_NotSrc1:
	    for(;size > 0; src1++, dst++, src2++, size -= BYTE_SIZE)
		*dst = (~*src1);
     break;

     default:
     break;
     }/* end switch on rule */
}

/************************************************************************
**
**  _BufferCombineWord 
**
**  FUNCTIONAL DESCRIPTION:
**
**  Performs a logical point combination on two non-bitonal input images.
**
**  FORMAL PARAMETERS:
**
    long		size;		* number of bits to combine	     *
    long		src1_addr;	* base address of source buf         *
    long		src1_pos;	* starting offset in source buf      *
    long		src2_addr;	* base address of source buf         *
    long		src2_pos;	* starting offset in source buf      *
    long		dst_addr;	* base address of destination buf    *
    long		dst_pos;	* starting offset in destination buf *
    long		rule;		* combination rule                   *
**
**  IMPLICIT INPUTS:  None
**  IMPLICIT OUTPUTS: None
**  FUNCTION VALUE:   None
**  SIGNAL CODES:     None
**  SIDE EFFECTS:     None
**
************************************************************************/

long _BufferCombineWord(size,src1_pos,src1_addr,src2_pos,src2_addr,dst_pos,dst_addr, rule)
long		size;		/* size of bits to be combined         */
long		src1_pos;	/* starting offset in source1 buf      */
unsigned char	*src1_addr;	/* base address of source1 buf         */
long		src2_pos;	/* starting offset in source2 buf      */
unsigned char	*src2_addr;	/* base address of source2 buf         */
long		dst_pos;	/* starting offset in destination buf  */
unsigned char	*dst_addr;	/* base address of destination buf     */
long		rule;		/* combination rule                    */
{
unsigned short 	*src1;		/* current source1 buf word            */
unsigned short	*src2;		/* current source2 buf word            */
unsigned short	*dst;		/* current destination buf word        */
long		nbits;		/* number of bits to copy this time    */
unsigned long 	src1_word;	/* the aligned source1 word            */
unsigned long 	src2_word;	/* the aligned source2 word            */

/*
* obtain pointers to the source and destination buffers
*/

dst = (unsigned short *) dst_addr + dst_pos / WORD_SIZE;
src1 = (unsigned short *) src1_addr + src1_pos / WORD_SIZE;
src2 = (unsigned short *) src2_addr + src2_pos / WORD_SIZE;
src1_pos = src1_pos % WORD_SIZE;
src2_pos = src2_pos % WORD_SIZE;
dst_pos = dst_pos % WORD_SIZE;

/*
** Combine the largest portion of the longword possible until
** the entire bit string is combined
*/

/*
** apply the rule
*/

switch (rule)
     {
     case IpsK_Src1AndSrc2:
	    for(;size > 0; src1++, dst++, src2++, size -= WORD_SIZE)
		*dst =  *src1 & *src2;
     break;
    
     case IpsK_Src1AndNotSrc2:
	    for(;size > 0; src1++, dst++, src2++, size -= WORD_SIZE)
		*dst = (*src1) & (~*src2);
     break;
    
     case IpsK_NotSrc1AndSrc2:
	    for(;size > 0; src1++, dst++, src2++, size -= WORD_SIZE)
		*dst = (~*src1) & *src2;
     break;
    
     case IpsK_Src1XorSrc2:
	    for(;size > 0; src1++, dst++, src2++, size -= WORD_SIZE)
		*dst = (*src1) ^ (*src2);
     break;
    
     case IpsK_Src1OrSrc2:
	    for(;size > 0; src1++, dst++, src2++, size -= WORD_SIZE)
		*dst = *src1 | *src2;
     break;
    
     case IpsK_NotSrc1AndNotSrc2:
	    for(;size > 0; src1++, dst++, src2++, size -= WORD_SIZE)
		*dst = (~*src1) & (~*src2);
     break;
    
     case IpsK_NotSrc1XorSrc2:
	    for(;size > 0; src1++, dst++, src2++, size -= WORD_SIZE)
		*dst = (~*src1) ^ (*src2);
     break;
    
     case IpsK_Src1OrNotSrc2:
	    for(;size > 0; src1++, dst++, src2++, size -= WORD_SIZE)
		*dst = (*src1) | (~*src2);
     break;
    
     case IpsK_NotSrc1OrSrc2:
	    for(;size > 0; src1++, dst++, src2++, size -= WORD_SIZE)
		*dst = (~*src1) | (*src2);
     break;
    
     case IpsK_NotSrc1OrNotSrc2:
	    for(;size > 0; src1++, dst++, src2++, size -= WORD_SIZE)
		*dst = (~*src1) | (~*src2);
     break;
    
     case IpsK_SetToSrc2:
	    for(;size > 0; src1++, dst++, src2++, size -= WORD_SIZE)
		*dst = *src2;
     break;

     case IpsK_SetToNotSrc2:
	    for(;size > 0; src1++, dst++, src2++, size -= WORD_SIZE)
		*dst = (~*src2);
     break;

     case IpsK_SetSrc1:
	    for(;size > 0; src1++, dst++, src2++, size -= WORD_SIZE)
		*dst = *src1;
     break;

     case IpsK_NotSrc1:
	    for(;size > 0; src1++, dst++, src2++, size -= WORD_SIZE)
		*dst = (~*src1);
     break;

     default:
     break;
     }/* end switch on rule */
}

/************************************************************************
**
**  _BufferCombine 
**
**  FUNCTIONAL DESCRIPTION:
**
**  Performs a logical point combination on two non-bitonal input images.
**
**  FORMAL PARAMETERS:
**
    long		size;		* number of bits to combine	     *
    long		src1_addr;	* base address of source buf         *
    long		src1_pos;	* starting offset in source buf      *
    long		src2_addr;	* base address of source buf         *
    long		src2_pos;	* starting offset in source buf      *
    long		dst_addr;	* base address of destination buf    *
    long		dst_pos;	* starting offset in destination buf *
    long		rule;		* combination rule                   *
**
**  IMPLICIT INPUTS:  None
**  IMPLICIT OUTPUTS: None
**  FUNCTION VALUE:   None
**  SIGNAL CODES:     None
**  SIDE EFFECTS:     None
**
************************************************************************/

long _BufferCombineLong
    (size,src1_pos,src1_addr,src2_pos,src2_addr,dst_pos,dst_addr, rule)
long		size;		/* size of bits to be combined         */
long		src1_pos;	/* starting offset in source1 buf      */
unsigned char	*src1_addr;	/* base address of source1 buf         */
long		src2_pos;	/* starting offset in source2 buf      */
unsigned char	*src2_addr;	/* base address of source2 buf         */
long		dst_pos;	/* starting offset in destination buf  */
unsigned char	*dst_addr;	/* base address of destination buf     */
long		rule;		/* combination rule                    */
{
unsigned long 	*src1;		/* current source1 buf word            */
unsigned long 	*src2;		/* current source2 buf word            */
unsigned long 	*dst;		/* current destination buf word        */
long		nbits;		/* number of bits to copy this time    */
unsigned long 	src1_word;	/* the aligned source1 word            */
unsigned long 	src2_word;	/* the aligned source2 word            */

/*
* obtain pointers to the source and destination buffers
*/

dst = (unsigned long *) dst_addr + dst_pos / LONG_SIZE;
src1 = (unsigned long *) src1_addr + src1_pos / LONG_SIZE;
src2 = (unsigned long *) src2_addr + src2_pos / LONG_SIZE;
src1_pos = src1_pos % LONG_SIZE;
src2_pos = src2_pos % LONG_SIZE;
dst_pos = dst_pos % LONG_SIZE;

/*
** Combine the largest portion of the longword possible until
** the entire bit string is combined
*/

/*
** apply the rule
*/

switch (rule)
     {
     case IpsK_Src1AndSrc2:
	    for(;size > 0; src1++, dst++, src2++, size -= LONG_SIZE)
		*dst =  *src1 & *src2;
     break;
    
     case IpsK_Src1AndNotSrc2:
	    for(;size > 0; src1++, dst++, src2++, size -= LONG_SIZE)
		*dst = (*src1) & (~*src2);
     break;
    
     case IpsK_NotSrc1AndSrc2:
	    for(;size > 0; src1++, dst++, src2++, size -= LONG_SIZE)
		*dst = (~*src1) & *src2;
     break;
    
     case IpsK_Src1XorSrc2:
	    for(;size > 0; src1++, dst++, src2++, size -= LONG_SIZE)
		*dst = (*src1) ^ (*src2);
     break;
    
     case IpsK_Src1OrSrc2:
	    for(;size > 0; src1++, dst++, src2++, size -= LONG_SIZE)
		*dst = *src1 | *src2;
     break;
    
     case IpsK_NotSrc1AndNotSrc2:
	    for(;size > 0; src1++, dst++, src2++, size -= LONG_SIZE)
		*dst = (~*src1) & (~*src2);
     break;
    
     case IpsK_NotSrc1XorSrc2:
	    for(;size > 0; src1++, dst++, src2++, size -= LONG_SIZE)
		*dst = (~*src1) ^ (*src2);
     break;
    
     case IpsK_Src1OrNotSrc2:
	    for(;size > 0; src1++, dst++, src2++, size -= LONG_SIZE)
		*dst = (*src1) | (~*src2);
     break;
    
     case IpsK_NotSrc1OrSrc2:
	    for(;size > 0; src1++, dst++, src2++, size -= LONG_SIZE)
		*dst = (~*src1) | (*src2);
     break;
    
     case IpsK_NotSrc1OrNotSrc2:
	    for(;size > 0; src1++, dst++, src2++, size -= LONG_SIZE)
		*dst = (~*src1) | (~*src2);
     break;
    
     case IpsK_SetToSrc2:
	    for(;size > 0; src1++, dst++, src2++, size -= LONG_SIZE)
		*dst = *src2;
     break;

     case IpsK_SetToNotSrc2:
	    for(;size > 0; src1++, dst++, src2++, size -= LONG_SIZE)
		*dst = (~*src2);
     break;

     case IpsK_SetSrc1:
	    for(;size > 0; src1++, dst++, src2++, size -= LONG_SIZE)
		*dst = *src1;
     break;

     case IpsK_NotSrc1:
	    for(;size > 0; src1++, dst++, src2++, size -= LONG_SIZE)
		*dst = (~*src1);
     break;

     default:
     break;
     }/* end switch on rule */
}
