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
**      Core routine to perform a flip horizontally for bitonal images.
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
long _IpsFlipHorizontalBitonal();	    /* flip horizontal bits	    */
long _IpsFlipHBitonal();
#endif
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
#define BLANK 1

/*
**  External References:
*/
#ifdef NODAS_PROTO
unsigned long *_IpsBuildChangelist();	               /* build changelists */
void 	      _IpsPutRuns();		        /* restore from changelists */
long	      _IpsBuildDstUdp();	                  /* from udp_utils */
long	      _IpsLogicalBitonal();            /* from IPS__LOGICAL_BITONAL */
#endif
/*
**  Local Storage:
*/

/******************************************************************************
**
**  _IpsFlipHorizontalBitonal
**
**  FUNCTIONAL DESCRIPTION:
**
**      Flips a bitonal frame horizontally
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

long _IpsFlipHorizontalBitonal (src_udp, dst_udp, flags)
struct	UDP *src_udp;			/* source      UDP descriptor	*/
struct	UDP *dst_udp;			/* destination UDP descriptor	*/
unsigned long flags;			/* retain option	        */
{
unsigned long size;
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
	    default:
		return(IpsX_UNSOPTION);
	    break;
	    }/* end switch on DType */
	break;
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
status = _IpsBuildDstUdp (src_udp, dst_udp, IpsM_InitMem, flags, 0);
if (status != IpsX_SUCCESS) return (status);

/* 
** zero user buffer so that put runs will work correctly (relies on
** zero buffer)
*/
if (mem_alloc == 0)
    status = _IpsLogicalBitonal (dst_udp, dst_udp, dst_udp, 0, 
	IpsK_Src1AndNotSrc2);
if (status != IpsX_SUCCESS);

/*****************************************************************************
**	    Dispatch to layer 2a                                            **
*****************************************************************************/

/*
** Bitonal UDP
*/
status = _IpsFlipHBitonal(src_udp,dst_udp);
if ((status != IpsX_SUCCESS) && (mem_alloc == 1))
    (*IpsA_MemoryTable[IpsK_FreeDataPlane]) (dst_udp->UdpA_Base);
return (status);

}/* end _IpsFlipHorizontalBitonal */

/******************************************************************************
**
**  _IpsFlipHBitonal
**
**  FUNCTIONAL DESCRIPTION:
**
**      Flips a bitonal frame horizontally
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
/* RESTRICTIONS:                                                              */
/*    current restrictions:						      */
/*                                                                            */
/*    bitonal udp's only. class is UBA                                        */
/*                                                                            */
/******************************************************************************/

long _IpsFlipHBitonal(src_udp, dst_udp)
struct	UDP *src_udp;			/* source      UDP descriptor	*/
struct	UDP *dst_udp;			/* destination UDP descriptor	*/
{
unsigned int bits_per_line;	    /* bits per scanline		*/
unsigned int chnglist_size;	    /* changelist size  		*/
unsigned int chnglist_length;	    /* changelist fixed length 		*/
unsigned int src_data_bit_offset;   /* src data offset from start	*/
unsigned int dst_data_bit_offset;   /* dst data offset from start	*/
long *src_chnglist;	    /* source      changelist pointer	*/
long *dst_chnglist;	    /* destination changelist pointer	*/
long *src_chnglist_ptr;     /* input dynamic			*/
long *dst_chnglist_ptr;     /* output dynamic			*/
unsigned int value;		    /* bitonal value (0 or 1)		*/
unsigned int ix,iy, i, j;	    /* loop counters			*/
unsigned long size;		    /* size in bytes (for allocation)	*/

src_data_bit_offset = src_udp->UdpL_Pos + (src_udp->UdpL_Y1 * 
    src_udp->UdpL_ScnStride) + src_udp->UdpL_X1;
dst_data_bit_offset = dst_udp->UdpL_Pos + (dst_udp->UdpL_Y1 * 
    dst_udp->UdpL_ScnStride) + dst_udp->UdpL_X1;

chnglist_length = src_udp->UdpL_PxlPerScn + 2;

/*
** Allocate enough room for both source and destination changlist)
*/
size = sizeof(unsigned long) *  2 * chnglist_length;

src_chnglist = 
    (long*)(*IpsA_MemoryTable[IpsK_Alloc])(size,IpsM_InitMem,0);
if (!src_chnglist)
    return (IpsX_INSVIRMEM);

dst_chnglist = src_chnglist + chnglist_length;

/*
** Create and put changelists in grand scanline loop
** This loop creates a changelist and reads it in reverse to flip
*/
for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
    {
    /*
    ** Initialize dynamic pointers
    */
    src_chnglist_ptr = src_chnglist;
    dst_chnglist_ptr = dst_chnglist;

    _IpsBuildChangelist(src_udp->UdpA_Base, src_data_bit_offset, 
                        src_udp->UdpL_PxlPerScn, src_chnglist_ptr,
                        chnglist_length);

    if (*src_chnglist_ptr != BLANK)		   /* Check for blank lines */
	{
	/*
	** flip changelist about the y axis
	*/
	*dst_chnglist_ptr = *src_chnglist_ptr;
	chnglist_size = *src_chnglist_ptr;	  /* start at end of list   */
	src_chnglist_ptr = src_chnglist_ptr + chnglist_size;
	
	if (*dst_chnglist_ptr++ & 1)		  /* ends with a 0	    */
	    for (i=1, j=0; i < chnglist_size; i++, src_chnglist_ptr--)
	        j = *dst_chnglist_ptr++= 
                    (*src_chnglist_ptr - *(src_chnglist_ptr-1)) + j;
	else
	    {
	    j= *dst_chnglist_ptr++ = 0;		           /* ends with 1 */
	    for (i=1,(*dst_chnglist)++; i<chnglist_size; i++,src_chnglist_ptr--)
	        j = *dst_chnglist_ptr++= (*src_chnglist_ptr - 
                    *(src_chnglist_ptr - 1)) + j;
	    }/* end else ends with a 1 */
	
	*dst_chnglist_ptr = *src_chnglist_ptr + j;

	_IpsPutRuns (dst_udp->UdpA_Base,dst_data_bit_offset,dst_chnglist);
	}/* end non-blank scanline */
    src_data_bit_offset += src_udp->UdpL_ScnStride;
    dst_data_bit_offset += dst_udp->UdpL_ScnStride;
    }/* end scanline loop */

/* 
** free allocated memory
*/
(*IpsA_MemoryTable[IpsK_Dealloc])(src_chnglist);
return (IpsX_SUCCESS);
}/* end _IpsFlipHBitonal */
