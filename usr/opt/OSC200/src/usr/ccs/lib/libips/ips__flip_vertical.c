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
**      Core routine to perform a flip vertically.
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
**  MODIFICATION HISTORY:
**
**
*******************************************************************************/

/*
**  Table of contents:
*/
#ifdef NODAS_PROTO
long _IpsFlipVerticalBitonal();		    /* flip Vertical (bitonal)	    */
long _IpsFlipVertical();		    /* flip Vertical		    */
long _IpsFlipVerticalAll();		    /* flip all data types	    */
#endif

/*
**  Internal routines                                                         
*/

/*
**  Include files:
*/
#include    <IpsDef.h>			/* IPS Definitions		     */
#include    <IpsStatusCodes.h>		/* IPS Status Codes		     */
#include    <IpsMemoryTable.h>		/* IPS Memory Mgt. Functions         */
#ifndef NODAS_PROTO
#include    <ipsprot.h>			/* Ips prototypes */
#endif

/*
**  Equated Symbols:
*/

/*
**  External References:
*/
#ifdef NODAS_PROTO
void          _IpsMovv5();		    /* Ips__extend_instruct	      */
long	      _IpsBuildDstUdp();	    /* from udp_utils */
#endif

/*
**  Local Storage:
*/

/******************************************************************************
**
**  _IpsFlipVertical
**
**  FUNCTIONAL DESCRIPTION:
**
**      Flips a frame Vertically
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

long _IpsFlipVerticalBitonal(src_udp, dst_udp, flags)
struct	UDP *src_udp;			/* source      UDP descriptor	*/
struct	UDP *dst_udp;			/* destination UDP descriptor	*/
unsigned long flags;			/* retain option	        */
{
return (_IpsFlipVertical(src_udp, dst_udp, flags));
}


/******************************************************************************
**
**  _IpsFlipVertical
**
**  FUNCTIONAL DESCRIPTION:
**
**      Flips a frame Vertically
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

long _IpsFlipVertical(src_udp, dst_udp, flags)
struct	UDP *src_udp;			/* source      UDP descriptor	*/
struct	UDP *dst_udp;			/* destination UDP descriptor	*/
unsigned long flags;			/* retain option	        */
{
unsigned long size;			/* size in bytes to allocate	*/
long status;
unsigned long mem_alloc = 0;

/*
** Validate source and type
*/
switch (src_udp->UdpB_Class)
    {
    case UdpK_ClassUBA:				
	/*
	** Bitonal UDP
	*/
	break;

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
	    case UdpK_DTypeVU:
	    case UdpK_DTypeV:
		break;
	    default:
		return(IpsX_UNSOPTION);
		break;
	    }/* end switch on DType */
	break;
    case UdpK_ClassUBS:
    case UdpK_ClassCL:
    default:
	return(IpsX_UNSOPTION);
	break;
    }/* end switch on class */

/*
** Initialize destination udp fixed fields , calculate size in bytes of
** new data buffer and allocate memory.
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
    case UdpK_DTypeV:
    case UdpK_DTypeVU:
    case UdpK_DTypeBU:
    case UdpK_DTypeWU:
    case UdpK_DTypeLU:
    case UdpK_DTypeF:
	return(_IpsFlipVerticalAll(src_udp,dst_udp));
    break;
    default:
	return(IpsX_UNSOPTION);
    break;
    }/* end switch on DType */
 
if ((status != IpsX_SUCCESS) && (mem_alloc == 1))
    (*IpsA_MemoryTable[IpsK_FreeDataPlane]) (dst_udp->UdpA_Base);
return (status);

}/* end _IpsFlipVertical */

/******************************************************************************
**
**  _IpsFlipVerticalAll
**
**  FUNCTIONAL DESCRIPTION:
**
**      Flips all data types Vertically
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

long _IpsFlipVerticalAll(src_udp, dst_udp)
struct	UDP *src_udp;			/* source      UDP descriptor	    */
struct	UDP *dst_udp;			/* destination UDP descriptor	    */
{
long bits_per_scanline;			/* bits per scanline		    */
long src_data_bit_offset;		/* source offset from start         */
long dst_data_bit_offset;		/* destination offset from start    */
long ix,iy;				/* loop indices			    */

/*
** Calculate source and destination bit offset values
*/
src_data_bit_offset = src_udp->UdpL_Pos + (src_udp->UdpL_ScnStride *
    src_udp->UdpL_Y1) + (src_udp->UdpL_X1 * src_udp->UdpL_PxlStride);
dst_data_bit_offset = dst_udp->UdpL_Pos + (dst_udp->UdpL_Y2 * 
    dst_udp->UdpL_ScnStride) + (dst_udp->UdpL_X1 * dst_udp->UdpL_PxlStride);

/*
** Grand Loop swaps source and destination scanlines (flip around x axis)
*/
for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
    {
    /*
    ** Move each scanline of bits from source to destination
    */
    _IpsMovv5(dst_udp->UdpL_PxlPerScn * dst_udp->UdpW_PixelLength,
		src_data_bit_offset,src_udp->UdpA_Base,
		    dst_data_bit_offset,dst_udp->UdpA_Base);
    /*
    ** Update offset values
    */
    src_data_bit_offset += src_udp->UdpL_ScnStride;
    dst_data_bit_offset -= dst_udp->UdpL_ScnStride;
    }/* end scanline loop*/
return(IpsX_SUCCESS);
}/* end _IpsFlipVerticalAll */
