
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
/******************************************************************************
**
**  FACILITY:
**
**      Image Processing Services (IPS)
**
**  ABSTRACT:
**
**      Core routine to perform a flip horizontally.
**
**  ENVIRONMENT:
**
**      VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
**      Richard Piccolo, Digital Equipment Corp.
**	Modified for V3.0 by John Poltrack
**
**  CREATION DATE:
**
**      SEPT, 1988
**
*******************************************************************************/

/*
**  Table of contents:
*/
#ifdef NODAS_PROTO
long _IpsFlipHorizontal();		    /* flip horizontal		    */
long _IpsFlipHorizontalByte();		    /* flip horizontal bytes	    */
long _IpsFlipHByte();
#endif

/*
**  Include files:
*/
#include    <IpsDef.h>			/* IPS Definitions		     */
#include    <IpsStatusCodes.h>		/* IPS Status Codes		     */
#include    <IpsMemoryTable.h>		/* IPS Memory Mgt. Functions         */
#ifndef NODAS_PROTO
#include    <ipsprot.h>		        /* Ips prototypes */
#endif

/*
**  Equated Symbols:
*/
#define BLANK 1

/*
**  External References:
*/
#ifdef NODAS_PROTO
long _IpsBuildDstUdp();			/* from udp_utils */
#endif

/*
**  Local Storage:
*/

/******************************************************************************
**
**  _IpsFlipHorizontal
**
**  FUNCTIONAL DESCRIPTION:
**
**      Flips a frame horizontally
**
**  FORMAL PARAMETERS:
**
**	src_udp --> pointer to source	    udp (initialized)
**	dst_udp --> pointer to destination  udp (uninitialized)
**	flags   --> retain outer dimension option
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
**	none
**
**  SIDE EFFECTS:
**
**      none
**
*******************************************************************************/

/******************************************************************************/
/* Restrictions:                                                              */
/*    current restrictions:						      */
/*                                                                            */
/*    multiple data planes, content elements, etc. not handled                */
/*    decompression not performed                                             */
/*                                                                            */
/******************************************************************************/

long _IpsFlipHorizontal(src_udp, dst_udp, flags)
struct	UDP *src_udp;			/* source      UDP descriptor	*/
struct	UDP *dst_udp;			/* destination UDP descriptor	*/
unsigned long flags;			/* retain option	        */
{
unsigned long size;
long status;
unsigned long mem_alloc=0;

/*
** Validate source and type
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
	        break;
	    case UdpK_DTypeWU:
	    case UdpK_DTypeLU:
	    case UdpK_DTypeF:
	    case UdpK_DTypeVU:
	    case UdpK_DTypeV:
	    default:
		return(IpsX_UNSOPTION);
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
** Initialize destination udp fixed fields , calculate size in bytes of
** new data buffer and allocate memory
*/
if (dst_udp->UdpA_Base == 0)
    mem_alloc = 1;
status = _IpsBuildDstUdp (src_udp, dst_udp, 0, flags, 0);
if (status != IpsX_SUCCESS) return (status);


/*****************************************************************************
**	    Dispatch to layer 2a                                            **
*****************************************************************************/

switch (src_udp->UdpB_DType)
    {
    case UdpK_DTypeBU:
	status = _IpsFlipHByte(src_udp,dst_udp);
    break;
    case UdpK_DTypeWU:
    case UdpK_DTypeLU:
    case UdpK_DTypeF:
    default:
    	return(IpsX_UNSOPTION);
    break;
    }/* end switch on DType */

if ((status != IpsX_SUCCESS) && (mem_alloc == 1))
    (*IpsA_MemoryTable[IpsK_FreeDataPlane]) (dst_udp->UdpA_Base);
return (status);
 
}/* end _IpsFlipHorizontal */

/******************************************************************************
**
**  _IpsFlipHByte
**
**  FUNCTIONAL DESCRIPTION:
**
**      Flips a non-bitonal frame horizontally
**
**  FORMAL PARAMETERS:
**
**	src_udp --> pointer to source	    udp (initialized)
**	dst_udp --> pointer to destination  udp (initialized)
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
**	none
**
**  SIDE EFFECTS:
**
**      none
**
*******************************************************************************/

/******************************************************************************/
/* Restrictions:                                                              */
/*    current restrictions:						      */
/*                                                                            */
/*    multiple data planes, content elements, etc. not handled                */
/*    decompression not performed                                             */
/*                                                                            */
/******************************************************************************/

long _IpsFlipHByte(src_udp, dst_udp)
struct	UDP *src_udp;			/* source      UDP descriptor	    */
struct	UDP *dst_udp;			/* destination UDP descriptor	    */
    {
    unsigned char *src_line_ptr;	    /* pointers for src		    */
    unsigned char *dst_line_ptr;	    /* pointers for dst		    */
    long dst_bytes_per_scanline_roi;	    /* bytes / scanline (dst - roi) */
    long ix,iy;				    /* loop indices		    */
    long src_incr, dst_incr;		    /* loop increments		    */

    /*
    ** Point to first pixel of source and last pixel of destination (1st scanline)
    */
    src_line_ptr = (unsigned char *)src_udp->UdpA_Base + (src_udp->UdpL_Pos >> 3) + 
        ((src_udp->UdpL_ScnStride>>3) * src_udp->UdpL_Y1) + src_udp->UdpL_X1;

    dst_bytes_per_scanline_roi = dst_udp->UdpL_PxlPerScn;
    dst_line_ptr = (unsigned char *)dst_udp->UdpA_Base + 
	(dst_udp->UdpL_Pos >> 3) + ((dst_udp->UdpL_ScnStride>>3) * 
	    dst_udp->UdpL_Y1) + dst_udp->UdpL_X1 + dst_bytes_per_scanline_roi;

    src_incr = (src_udp->UdpL_ScnStride >> 3) - src_udp->UdpL_PxlPerScn;
    dst_incr = (dst_udp->UdpL_ScnStride >> 3) + dst_bytes_per_scanline_roi;

    /*
    ** Grand Loop exchanges relative position of source and destination byte
    */
    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
        {
        for (ix = 0;  ix < dst_udp->UdpL_PxlPerScn;  ix++)
	    *--dst_line_ptr = *src_line_ptr++;
        src_line_ptr += src_incr;
        dst_line_ptr += dst_incr;
        }

    return(IpsX_SUCCESS);
    }/* end _IpsFlipHByte */

