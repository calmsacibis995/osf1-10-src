/*  DEC/CMS REPLACEMENT HISTORY, Element IPS__HISTOGRAM.C */
/*  *4    14-NOV-1990 15:01:27 DINTINO "added cpp support " */
/*  *3    23-AUG-1990 09:28:47 PICCOLO "speedup and added short, long capabilities" */
/*  *2     9-AUG-1990 15:15:36 PICCOLO "pos change" */
/*  *1     1-JUN-1990 16:18:53 PICCOLO "IPS Base Level" */
/*  DEC/CMS REPLACEMENT HISTORY, Element IPS__HISTOGRAM.C */
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
**  _IpsHistogram
**
**  FACILITY:
**
**	Image Processing Services (IPS)
**
**  FUNCTIONAL DESCRIPTION:
**
**	This function effects the collection and storage of the histogram
**	data for implicitly indexed histogram tables.
**
**  FORMAL PARAMETERS:
**
**	udp	Universal Data Plane descriptor that points to the
**		data plane from which the histogram will be derived.
**		Passed by reference.
**
**	tab_ptr pointer to a longword pointer to histogram table
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	tab_ptr Longword pointer to histogram results (number of elements
**		is always equal to the source udp quantization levels.
**	cpp     Control Processing Plane
**
**  FUNCTION VALUE:
**
**	status
**
**  ENVIRONMENT:
**
**      VAX/VMS, VAX/ULTRIX RISC/ULTRIX
**
**  AUTHOR(S):
**
**	John Poltrack
**	Revised for V3.0 by John Poltrack 14-NOV-89
**	Revised for Ips Interface by Richard Piccolo 24-APR-90
**	Revised for speed, shorts and longs
**
**
************************************************************************/

/*
** Include files
*/
#include <IpsDef.h>			    /* Image definitions	    */
#include <IpsStatusCodes.h>
#include <IpsMacros.h>                      /* IPS Macro Definitions         */
#ifndef NODAS_PROTO
#include <ipsprot.h>			    /* IPS Prototypes		    */
#endif
#include <IpsMemoryTable.h>
#include <math.h>			    /* C math functions		    */


long _IpsHistogram(udp, cpp, tab_ptr)
struct UDP	*udp;
struct UDP      *cpp;
unsigned long	**tab_ptr;
{
unsigned long	*table;
unsigned long	line_pad;		    /* number of pad	    */
unsigned char	*src_byte_ptr;		    /* byte pointer to udp	    */
unsigned short	*src_word_ptr;		    /* word pointer to udp	    */
unsigned long	*src_long_ptr;		    /* long pointer to udp	    */
unsigned long	ix,iy;			    /* loop entities		    */
unsigned long   size;
unsigned char   *cpp_ptr;   /* control proc plane base     */
unsigned long   cpp_stride; /* control proc plane stride   */
 

/*
** validate udp
*/
switch (udp->UdpB_Class)
    {
    case UdpK_ClassA:
	/*
	** Atomic array
	*/
	switch (udp->UdpB_DType)
	    {
	    case UdpK_DTypeBU:
	    case UdpK_DTypeWU:
	    case UdpK_DTypeLU:
	    break;
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

    size = sizeof(long) * udp->UdpL_Levels;
    *tab_ptr = (unsigned long*)(*IpsA_MemoryTable[IpsK_Alloc])
	(size,IpsM_InitMem,0);
    if (!*tab_ptr) 
        return (IpsX_INSVIRMEM);
    table = *tab_ptr;

    if (cpp != 0)
        VALIDATE_CPP_(cpp, udp);
                                    
    switch (udp->UdpB_DType)
        {
	case UdpK_DTypeBU:
	    line_pad = (udp->UdpL_ScnStride>>3) - udp->UdpL_PxlPerScn;
	    src_byte_ptr = (unsigned char *)udp->UdpA_Base + 
		(udp->UdpL_Pos>>3) + ((udp->UdpL_ScnStride>>3) * udp->UdpL_Y1)
		     + udp->UdpL_X1;
	    /*
	    ** Get the data value and increment its frequency count.
	    */
	    if (cpp != 0)
		{
		cpp_ptr = (unsigned char *) cpp->UdpA_Base
                	+ (((cpp->UdpL_Pos +
	                  (cpp->UdpL_ScnStride * cpp->UdpL_Y1) +
        	           cpp->UdpL_X1)+7)>>3);

		cpp_stride = (cpp->UdpL_ScnStride + 7)>>3;
 
	        for (iy = 0; iy < udp->UdpL_ScnCnt ;iy++, 
					src_byte_ptr+=line_pad )
	            {
		    for (ix = 0; ix < udp->UdpL_PxlPerScn; ix++)
		        {
	                if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)	
		            table[*src_byte_ptr] += 1;
                        src_byte_ptr++;
		        }
		    cpp_ptr += cpp_stride;
		    }
		}
	    else
		{
	        for (iy = 0; iy < udp->UdpL_ScnCnt ;iy++, 
					src_byte_ptr+=line_pad )
	            for (ix = 0; ix < udp->UdpL_PxlPerScn; ix++)
		        table[*src_byte_ptr++] += 1;
		}
	break;

	case UdpK_DTypeWU:
	    line_pad = (udp->UdpL_ScnStride>>4) - udp->UdpL_PxlPerScn;
	    src_word_ptr = (unsigned short *)udp->UdpA_Base + 
		(udp->UdpL_Pos>>4) + ((udp->UdpL_ScnStride>>4) * udp->UdpL_Y1)
		    + udp->UdpL_X1;
	    /*
	    ** Get the data value and increment its frequency count.
	    */
	    if (cpp != 0)
		{
		cpp_ptr = (unsigned char *) cpp->UdpA_Base
                	+ (((cpp->UdpL_Pos +
	                  (cpp->UdpL_ScnStride * cpp->UdpL_Y1) +
        	           cpp->UdpL_X1)+7)>>3);

		cpp_stride = (cpp->UdpL_ScnStride + 7)>>3;
	        for (iy = 0; iy < udp->UdpL_ScnCnt ;iy++, 
					src_word_ptr+=line_pad )
		    {
	            for (ix = 0; ix < udp->UdpL_PxlPerScn; ix++) 
		        {
	                if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)	
		            table[*src_word_ptr] += 1;
                        src_word_ptr++;
		        }
		    cpp_ptr += cpp_stride;
		    }
		}
	    else
		{
	        for (iy = 0; iy < udp->UdpL_ScnCnt ;iy++, 
					src_word_ptr+=line_pad )
	            for (ix = 0; ix < udp->UdpL_PxlPerScn; ix++)
		        table[*src_word_ptr++] += 1;
		}
	break;

	case UdpK_DTypeLU:
	    line_pad = (udp->UdpL_ScnStride>>5) - udp->UdpL_PxlPerScn;
	    src_long_ptr = (unsigned long *)udp->UdpA_Base + 
		(udp->UdpL_Pos>>5) + ((udp->UdpL_ScnStride>>5) * udp->UdpL_Y1)
		    + udp->UdpL_X1;
	    /*
	    ** Get the data value and increment its frequency count.
	    */
	    if (cpp != 0)
		{
		cpp_ptr = (unsigned char *) cpp->UdpA_Base
                	+ (((cpp->UdpL_Pos +
	                  (cpp->UdpL_ScnStride * cpp->UdpL_Y1) +
        	           cpp->UdpL_X1)+7)>>3);

		cpp_stride = (cpp->UdpL_ScnStride + 7)>>3;
	        for (iy = 0; iy < udp->UdpL_ScnCnt ;iy++, 
					src_long_ptr+=line_pad )
		    {
	            for (ix = 0; ix < udp->UdpL_PxlPerScn; ix++) 
		        {
	                if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)	
		            table[*src_long_ptr] += 1;
                        src_long_ptr++;
		        }
		    cpp_ptr += cpp_stride;
		    }
		}
	    else
		{
	        for (iy = 0; iy < udp->UdpL_ScnCnt ;iy++, 
					src_long_ptr+=line_pad )
	            for (ix = 0; ix < udp->UdpL_PxlPerScn; ix++)
		        table[*src_long_ptr++] += 1;
		}
	break;

	default:
	    return(IpsX_UNSOPTION);
	break;
	}/* end switch on DType */

return (IpsX_SUCCESS);

} /* end of _IpsHistogram */
