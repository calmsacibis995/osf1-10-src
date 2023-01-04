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
**      This code represents the IPS entries for remapping
**	greyscale or color images.  
**
**  ENVIRONMENT:
**
**      VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
**      Karen Rodwell, Digital Equipment Corp.
**	Richard Piccolo - Revised for new IPS Interface
**
**  CREATION DATE:
**
**      August, 1989
**
*******************************************************************************/

/*
**  Table of contents:
*/
#ifdef NODAS_PROTO
long _IpsRemapUdp();     /* remap image plane */
#endif

/*
**  Internal routines                                                         
*/
#ifdef NODAS_PROTO
long  _remap_byte_udp();     
#endif

/*
**  Include files:
*/
#include <math.h>	      /* math libraries			    */
#include <IpsDef.h>           /* IPS definitions		    */
#include <IpsStatusCodes.h>   /* status codes			    */
#include <IpsMemoryTable.h>   /* IPS Memory Mgt. Functions          */
#include <IpsMacros.h>	      /* IPS Macros			    */
#ifndef NODAS_PROTO
#include <ipsprot.h>				    /* Ips prototypes */
#endif  
/*
**  External References:
*/

#ifdef NODAS_PROTO
long	_IpsBuildDstUdp();	                   /* from IPS__UDP_UTILS.C */
void	_IpsMovtcLong();                           /* from    UIPS__MOVC5.C */
#endif

/******************************************************************************
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**	src_udp dst_udp and cpp
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
**      status return
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**      none
**
**  RESTRICTIONS:                                                              
**
**	operates on data plane for BYTE_IMAGES only, therefore a pixel stride 
**	of one byte is assumed.
**		      
**                                                                            
*******************************************************************************/

/***************************************************************************
**
** _IpsRemapUdp()
**
** Internal routine which performs remapping of pixels using a lookup table
** specified as a descriptor by the user.
**
****************************************************************************/
long _IpsRemapUdp (src_udp, dst_udp, cpp, tab_ptr, tab_cnt, tab_data_type, flags)
struct UDP *src_udp;		/* Working src UDP descriptor  */
struct UDP *dst_udp;		/* Working dst UDP descriptor  */
struct UDP *cpp;		/* Control Processing Plane  */
unsigned long *tab_ptr;		/* pointer to user supplied lut*/
unsigned long tab_cnt;		/* number of elements in table */
unsigned long tab_data_type;	/* data type of table	   */
unsigned flags;			/* processing flag	   */
    {
    long           size;
    unsigned long  status;
    unsigned long  mem_alloc = 0;

    if (tab_cnt != src_udp->UdpL_Levels) return (IpsX_INCNSARG);
    switch (src_udp->UdpB_Class)
        {
        case UdpK_ClassA:
            switch (src_udp->UdpB_DType)
                {
                case UdpK_DTypeBU: break;
                case UdpK_DTypeWU:
                case UdpK_DTypeLU:
                case UdpK_DTypeF:
                case UdpK_DTypeV:
                case UdpK_DTypeVU:
                default: return(IpsX_UNSOPTION); break;
                }; break;
        case UdpK_ClassUBA:
        case UdpK_ClassUBS:
        case UdpK_ClassCL:
	default: 
	    return(IpsX_UNSOPTION); 
	break;
        };

    if (dst_udp->UdpA_Base == 0)
	mem_alloc = 1;
    status = _IpsBuildDstUdp (src_udp, dst_udp, IpsM_InitMem, flags, 
			    IpsK_InPlaceAllowed);
    if (status != IpsX_SUCCESS) return (status);

    /* Dispatch to layer 2a */

    switch (src_udp->UdpB_DType)
	{
	case UdpK_DTypeBU:
	    status = _remap_byte_udp (src_udp,dst_udp,tab_ptr,tab_cnt,
		tab_data_type);
	break;
	case UdpK_DTypeWU:	      
	case UdpK_DTypeLU:	      
	case UdpK_DTypeF:	      
	case UdpK_DTypeVU:	      
	case UdpK_DTypeV:
	default: 
	    return (IpsX_UNSOPTION); 
	break;
	}; /* end switch on DType */

    if ((status != IpsX_SUCCESS) && (mem_alloc == 1))
	(*IpsA_MemoryTable[IpsK_FreeDataPlane]) (dst_udp->UdpA_Base);
    return (status);
    }/* end _IpsPixelRemapLUT */

/***************************************************************************
**
** _remap_byte_udp()
**
** Internal routine which handles BU data.
**
****************************************************************************/
long _remap_byte_udp (src_udp, dst_udp, tab_ptr, tab_cnt, tab_data_type)
struct UDP *src_udp;		/* working src UDP descriptors */
struct UDP *dst_udp;		/* working dst UDP descriptors */
unsigned long *tab_ptr;		/* pointer to user supplied lut*/
unsigned long tab_cnt;		/* number of elements in table */
unsigned long tab_data_type;	/* data type of table	   */
{
unsigned long  ix,iy;		/* loop counters 	       */
unsigned char  orig_value;
unsigned long  src_pad;
unsigned long  dst_pad;
unsigned char  *src_byte;
unsigned char  *dst_byte;
unsigned char  *b_ptr;
unsigned short *w_ptr;

src_byte = (unsigned char *) src_udp->UdpA_Base + (src_udp->UdpL_Pos>>3) +
    ((src_udp->UdpL_ScnStride>>3) * src_udp->UdpL_Y1) + src_udp->UdpL_X1;
src_pad = (src_udp->UdpL_ScnStride >> 3) - src_udp->UdpL_PxlPerScn;
dst_byte = (unsigned char *) dst_udp->UdpA_Base + (dst_udp->UdpL_Pos>>3) +
    ((dst_udp->UdpL_ScnStride>>3) * dst_udp->UdpL_Y1) + dst_udp->UdpL_X1;
dst_pad = (dst_udp->UdpL_ScnStride >> 3) - dst_udp->UdpL_PxlPerScn;

switch (tab_data_type)
    {
    case IpsK_DTypeBU:

    b_ptr = (unsigned char *) tab_ptr;

#ifndef VMS
	for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
	    {
	    for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		{
		orig_value = *src_byte++;
		*dst_byte++ = (*(b_ptr + orig_value));
		} /* end inner for */
	    src_byte += src_pad;
	    dst_byte += dst_pad;
	    } /* end outer "for" */
    break;
#else

	if (src_pad == 0 && dst_pad == 0)	/* no padding use 1 call */
	    _IpsMovtcLong (src_udp->UdpL_PxlPerScn * src_udp->UdpL_ScnCnt, 
			    src_byte, 0, tab_ptr, 
				dst_udp->UdpL_PxlPerScn * dst_udp->UdpL_ScnCnt, 
				    dst_byte);
	else
	    {
	    src_pad = src_udp->UdpL_ScnStride>>3;
	    dst_pad = dst_udp->UdpL_ScnStride>>3;
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{           
		_IpsMovtcLong (dst_udp->UdpL_PxlPerScn, src_byte, 0, b_ptr, 
		    dst_udp->UdpL_PxlPerScn, dst_byte);
		src_byte += src_pad;
		dst_byte += dst_pad;
		} /* end outer "for" */
	    }
    break;
#endif

    case IpsK_DTypeWU:
	{
	w_ptr = (unsigned short *)tab_ptr;
        for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
            {           
            for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
                {
                orig_value = *src_byte++;
                *dst_byte++ = *(w_ptr + orig_value);
                } /* end inner for */
            src_byte += src_pad;
            dst_byte += dst_pad;
            } /* end outer "for" */
	break;
	}
    case IpsK_DTypeLU:
	{
        for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
            {           
            for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
                {
                orig_value = *src_byte++;
                *dst_byte++ = (*(tab_ptr + orig_value));
                } /* end inner for */
            src_byte += src_pad;
            dst_byte += dst_pad;
            } /* end outer "for" */
	break;
	}
    default:
    return (IpsX_INVDARG);
    break;
    }/* end switch on lut length */
return (IpsX_SUCCESS);
}
