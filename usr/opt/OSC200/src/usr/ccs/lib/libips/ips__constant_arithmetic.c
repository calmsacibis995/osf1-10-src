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
**	for arithmetic operations using a constant.
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
**      14-MAR-1990
**
************************************************************************/

/*
**  Table of contents
*/
#ifdef NODAS_PROTO
long _IpsConstantArithmetic();	    /* main entry routine, tests dtype */
long _IpsConstantArithmeticByt();   /* arith operation on byte dtype source */
long _IpsConstantArithmeticW ();    /* arith operation on word dtype source */
long _IpsConstantArithmeticFlt();   /* arith operation on float dtype source*/

long _IpsConstantArithmeticBytCpp();/* arith operation on byte data w/cpp */
long _IpsConstantArithmeticWCpp();  /* arith operation on word data w/cpp */
long _IpsConstantArithmeticFltCpp();/* arith opertaion on float data w/cpp*/
#endif

/*
**  Include files
*/
#include <IpsMacros.h>                      /* IPS Macro Definitions         */
#include <IpsMemoryTable.h>                 /* IPS Memory Mgt Functions      */
#include <IpsDef.h>			    /* IPS Definitions		     */
#include <IpsStatusCodes.h>                 /* IPS Status Codes              */
#ifndef NODAS_PROTO
#include <ipsprot.h>			    /* Ips prototypes */
#endif

/*
**  External References:
*/
#ifdef NODAS_PROTO
long _IpsBuildDstUdp();			    /* from udp_utils */
#endif

/*****************************************************************************
**  _IpsConstantArithmetic
**
**  FUNCTIONAL DESCRIPTION:
**
**      Performs arithmetic operations on corresponding values from the
**      source image and the constant to produce results for the destination 
**      image on a point by point basis.
**
**  FORMAL PARAMETERS:
**
**      src_udp		-- Pointer to first source udp
**      dst_udp		-- Pointer to second source udp 
**      cpp		-- Optional Pointer to Control Processing Plane
**	constant	-- Constant value to use as operand
**      operator	-- Arithmetic operation to be performed
**	control		-- Options on overflow/underflow handling
**      flags		-- Flags for options on output results
**	result_flag	-- Flags specifying the nature of the results
**
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   none
**  SIGNAL CODES:     none
**  SIDE EFFECTS:     none
**
************************************************************************/
long _IpsConstantArithmetic(src_udp,dst_udp,cpp,constant,operator,control,flags,result_flag)
struct UDP *src_udp;       /* source udp			  */
struct UDP *dst_udp;       /* destination udp			  */
struct UDP *cpp;           /* control processing plane		  */
double constant;           /* constant to be used in the arithmetic operation */
unsigned long operator;    /* arithmetic operator		  */
unsigned long control;     /* overflow/underflow control option   */
unsigned long flags;       /* options on destination dimensions   */
unsigned long *result_flag;/* indicates the nature of the results */ 
{
unsigned long size;
long status;
long mem_alloc = 0;

if  (src_udp->UdpB_Class != UdpK_ClassA)
    return (IpsX_UNSOPTION);
 
if ((src_udp->UdpB_DType == UdpK_DTypeVU) ||
    (src_udp->UdpB_DType == UdpK_DTypeV))
    return (IpsX_INVDTYPE);

if (cpp != 0)
    VALIDATE_CPP_(cpp, src_udp);

if (dst_udp->UdpA_Base == 0) mem_alloc = 1;
status = _IpsBuildDstUdp (src_udp, dst_udp, 0, flags, IpsK_InPlaceAllowed);
if (status != IpsX_SUCCESS) return (status);

*result_flag = 0;  

switch (src_udp->UdpB_DType)
    {
    case UdpK_DTypeBU:
	if (cpp == 0)
	    status = _IpsConstantArithmeticByt
		(src_udp, dst_udp, constant, operator, control, result_flag);
        else
	    status = _IpsConstantArithmeticBytCpp
		(src_udp, dst_udp, cpp, constant, operator, 
			    control, result_flag);
    break;

    case UdpK_DTypeWU:
	if (cpp == 0)
	    status =  _IpsConstantArithmeticW
		(src_udp, dst_udp, constant, operator, control, result_flag);
	else
	    status = _IpsConstantArithmeticWCpp
	     (src_udp, dst_udp, cpp, constant, operator, control, result_flag);
    break;

    case UdpK_DTypeF:
	if (cpp == 0)
	    status =  _IpsConstantArithmeticFlt
		    (src_udp, dst_udp, constant, operator);
	else
	    status = _IpsConstantArithmeticFltCpp
		    (src_udp, dst_udp, cpp, constant, operator);
    break;

    case UdpK_DTypeLU:
    default:
	status = IpsX_UNSOPTION;
    break;
    };  /* end switch on DType */

if ((status != IpsX_SUCCESS) && (mem_alloc == 1))
    (*IpsA_MemoryTable[IpsK_FreeDataPlane]) (dst_udp->UdpA_Base);

return (status);
}

/*****************************************************************************
**  _IpsConstantArithmeticByt- 
**
**  FUNCTIONAL DESCRIPTION:
**  
**      Performs arithmetic operations on corresponding values from the
**      unsigned byte source image and the constant to produce results 
**      for the unsigned byte data type destination image on a point by 
**      point basis.
**
**  FORMAL PARAMETERS:
**
**      src_udp     -- Pointer to source udp.
**      dst_udp     -- Pointer to destination udp.
**	constant    -- Constant operand.
**      operator    -- Arithmetic operation to be performed.
**      control     -- Overflow/underflow control option.
**      result_flag -- Indicates the nature of the results.
**
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   none
**  SIGNAL CODES:     none
**  SIDE EFFECTS:     none
**
************************************************************************/
long _IpsConstantArithmeticByt(src_udp,dst_udp,constant,operator,control,result_flag)
struct UDP *src_udp;      /* source byte data type data plane     */
struct UDP *dst_udp;      /* destintion byte data type data plane */
double constant;          /* constant to be used as operand in arith operation*/
unsigned long operator;   /* arithmetic operation to be performed */
unsigned long control;    /* overflow/underflow control option    */
unsigned long *result_flag;/* indicates the nature of the results  */
{
unsigned long  size;                  /* buffer size                    */
unsigned char  *src_plane_data_base;  /* pointer to source data base    */
unsigned char  *dst_byte_ptr;         /* used for dst image             */
unsigned char  *row_ptr;	      /* dynamc row pointer             */
unsigned long  ix,iy;                 /* loop counters                  */
unsigned long  src_pad;               /* src pad                        */
unsigned long  dst_pad;               /* dst pad                        */  
unsigned long  src_stride;            /* src stride in bytes            */
unsigned long  dst_stride;            /* dst stride in bytes            */
long  result;			      /* true result, before clip|wrap  */

src_plane_data_base = (unsigned char *)src_udp->UdpA_Base +
                    ((src_udp->UdpL_Pos +
                    (src_udp->UdpL_ScnStride * src_udp->UdpL_Y1) +
                    (src_udp->UdpL_X1 * src_udp->UdpL_PxlStride) )>>3);

dst_byte_ptr = (unsigned char *)dst_udp->UdpA_Base + 
                    ((dst_udp->UdpL_Pos +
                    (dst_udp->UdpL_ScnStride * dst_udp->UdpL_Y1) +
                    (dst_udp->UdpL_X1 * dst_udp->UdpL_PxlStride) )>>3);

row_ptr = (unsigned char *)src_plane_data_base;
src_stride = (src_udp->UdpL_ScnStride >> 3);
dst_stride = (dst_udp->UdpL_ScnStride >> 3);
src_pad = src_stride  - src_udp->UdpL_PxlPerScn;
dst_pad = dst_stride  - dst_udp->UdpL_PxlPerScn;

/*
** 1) Test for the overflow control option.
** 2) Perform the specified arithmetic operation.
** 3) If overflow or underflow, the result (based on the control option)
**      can either be clipped to fit within the range of levels OR,
**      by default, wrapped.
*/
 
switch (operator)
    {
    case IpsK_Addition:
        {
        switch (control)
            {
            case IpsK_Clip:
                {
		*result_flag = IpsK_SUCCESS_NOCLIP;
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                        {
                        result = ((*row_ptr++) + (constant));
                        if ((result > 255) || (result < 0))
			    {
			    if (result < 0)
			        *dst_byte_ptr++ = 0;
			    else
                                *dst_byte_ptr++ = 255;
			    *result_flag = IpsK_SUCCESS_CLIP;
			    }
                        else
                            *dst_byte_ptr++ = result;
                        }/* end ix loop */
                    dst_byte_ptr +=  dst_pad;
                    row_ptr  += src_pad;
                    } /*end iy loop */
                break;
                }
            case IpsK_Wrap:
            default:
                {
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
		        {
                        *dst_byte_ptr++ = ((*row_ptr++) + (constant)); 
			} /* end ix loop */
                    dst_byte_ptr +=  dst_pad;
                    row_ptr += src_pad;
                    } /*end iy loop */
                break;
                }
            }/* end switch on control */
	break;
	}
    case IpsK_SubtractionByConstant:
        {
        switch (control)
            {
            case IpsK_Clip:
                {
		*result_flag = IpsK_SUCCESS_NOCLIP;
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                        {
                        result = ((*row_ptr++) - (constant));
                        if ((result > 255) || (result < 0))
			    {
			    if (result < 0)
			        *dst_byte_ptr++ = 0;
			    else
                                *dst_byte_ptr++ = 255;
			    *result_flag = IpsK_SUCCESS_CLIP;
			    }
                        else
                            *dst_byte_ptr++ = result;
                        }/* end ix loop */
                    dst_byte_ptr +=  dst_pad;
                    row_ptr  += src_pad;
                    } /*end iy loop */
                break;
                }
            case IpsK_Wrap:
            default:
                {
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
		        {
                        *dst_byte_ptr++ = ((*row_ptr++) - (constant)); 
			} /* end ix loop */
                    dst_byte_ptr +=  dst_pad;
                    row_ptr  += src_pad;
                    } /*end iy loop */
                break;
                }
            }/* end switch on control */
	break;
	}
    case IpsK_SubtractionFromConstant:
        {
        switch (control)
            {
            case IpsK_Clip:
                {
		*result_flag = IpsK_SUCCESS_NOCLIP;
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                        {
                        result = ((constant) - (*row_ptr++));
                        if ((result > 255) || (result < 0))
			    {
			    if (result < 0)
			        *dst_byte_ptr++ = 0;
			    else
                                *dst_byte_ptr++ = 255;
			    *result_flag = IpsK_SUCCESS_CLIP;
			    }
                        else
                            *dst_byte_ptr++ = result;
                        }/* end ix loop */
                    dst_byte_ptr +=  dst_pad;
                    row_ptr  += src_pad;
                    } /*end iy loop */
                break;
                }
            case IpsK_Wrap:
            default:
                {
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
			{
                        *dst_byte_ptr++ = ((constant) - (*row_ptr++));
			} /* end ix loop */
                    dst_byte_ptr +=  dst_pad;
                    row_ptr  += src_pad;
                    } /*end iy loop */
                break;
                }
            }/* end switch on control */
	break;
	}
    case IpsK_Multiplication:
        {
        switch (control)
            {
            case IpsK_Clip:
                {
		*result_flag = IpsK_SUCCESS_NOCLIP;
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                        {
                        result = ((*row_ptr++) * (constant));
                        if ((result > 255) || (result < 0))
			    {
			    if (result < 0)
			        *dst_byte_ptr++ = 0;
			    else
                                *dst_byte_ptr++ = 255;
			    *result_flag = IpsK_SUCCESS_CLIP;
			    }
                        else
                            *dst_byte_ptr++ = result;
                        }/* end ix loop */
                    dst_byte_ptr +=  dst_pad;
                    row_ptr += src_pad;
                    } /*end iy loop */
                break;
                }
            case IpsK_Wrap:
            default:
                {
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
			{
                        *dst_byte_ptr++ = ((*row_ptr++) * (constant)); 
			} /* end ix loop */
                    dst_byte_ptr +=  dst_pad;
                    row_ptr += src_pad;
                    } /*end iy loop */
                break;
                }
            }/* end switch on control */
	break;
	}
    case IpsK_DivisionByConstant:
        {
	if (constant == 0)
	    return (IpsX_INVDARG);
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
		{
                *dst_byte_ptr++ = ((*row_ptr++) / (constant)); 
		} /* end ix loop */
            dst_byte_ptr +=  dst_pad;
            row_ptr += src_pad;
            } /*end iy loop */
        break;
        }
    case IpsK_DivisionFromConstant:
        {
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
		if (*row_ptr != 0)
                    *dst_byte_ptr++ = ((constant) / (*row_ptr++));
		else
		    {
		    *dst_byte_ptr++ = constant;
		    row_ptr++;
		    };
                }/* end ix loop */
            dst_byte_ptr +=  dst_pad;
            row_ptr += src_pad;
            } /*end iy loop */
	break;
	}
    case IpsK_Maximum:
	{
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                *dst_byte_ptr++ = MAX_((*row_ptr), constant);
		row_ptr++;
                }		/* end of ix loop      */
            dst_byte_ptr +=  dst_pad;
            row_ptr += src_pad;
            }             /* end of iy loop      */
	break;
	}

    case IpsK_Minimum:
        {
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                *dst_byte_ptr++ = MIN_((*row_ptr), constant);
		row_ptr++;
                }		/* end of ix loop      */
            dst_byte_ptr +=  dst_pad;
            row_ptr += src_pad;
            }             /* end of iy loop      */
	break;
	}
    case IpsK_Modulo:
        {
	if (constant == 0)
	    return (IpsX_INVDARG);	    
            
	for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                *dst_byte_ptr++ = *row_ptr++ % (long)constant;
                }		/* end of ix loop      */
            dst_byte_ptr +=  dst_pad;
            row_ptr += src_pad;
            }             /* end of iy loop      */
	break;
	}
    case IpsK_SetToConstant:
        {
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                *dst_byte_ptr++ = (unsigned char)constant;
                }		/* end of ix loop      */
            dst_byte_ptr +=  dst_pad;
            row_ptr += src_pad;
            }             /* end of iy loop      */
	break;
	}
    default:
        return (IpsX_UNSOPTION);
        break;
    }
return (IpsX_SUCCESS);
}

/*****************************************************************************
**  _IpsConstantArithmeticBytCpp- 
**
**  FUNCTIONAL DESCRIPTION:
**  
**      Performs arithmetic operations on corresponding values from the
**      unsigned byte source image and the constant to produce results 
**      for the unsigned byte data type destination image on a point by 
**      point basis.
**   
**      This routine tests the bits in the CPP plane corresponding to
**      the pixels being operated on.
**
**          If the CPP bit is set, it performs the operation and
**          places the result in the corresponding destination plane pixel
**          location.
**
**          If the CPP bit is not set, the pixel value of the sourc1
**          plane is placed in the corresponding destination plane pixel
**          location.
**
**  FORMAL PARAMETERS:
**
**      src_udp
**      dst_udp
**	cpp
**	constant
**      operator
**      control
**      result_flag
**
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   none
**  SIGNAL CODES:     none
**  SIDE EFFECTS:     none
**
************************************************************************/
long _IpsConstantArithmeticBytCpp(src_udp,dst_udp,cpp,constant,operator,control,result_flag)
struct UDP *src_udp;       /* source byte data type data plane            */
struct UDP *dst_udp;       /* destination data plane                      */
struct UDP *cpp;           /* control processing plane                    */
double constant;           /* constant to be used in arithmetic operation */
unsigned long operator;    /* arithmetic operation to be performed        */
unsigned long control;     /* overflow/underflow control option           */
unsigned long *result_flag; /* indicates the nature of the results         */
{
unsigned long  size;                  /* buffer size                    */
unsigned char  *src_plane_data_base;  /* pointer to source data base    */
unsigned char  *dst_byte_ptr;         /* used for dst image             */
unsigned char  *row_ptr;	      /* dynamc row pointer             */
unsigned long  ix,iy;                 /* loop counters                  */
unsigned long  src_pad;               /* src pad                        */
unsigned long  dst_pad;               /* dst pad                        */  
unsigned char  *cpp_ptr;              /* control processing plane base  */
unsigned long  cpp_stride;	      /* bitplane stride		*/
unsigned long  src_stride;            /* src stride                     */
unsigned long  dst_stride;            /* dst stride                     */
long           result;		      /* signed var for true result     */

src_plane_data_base = (unsigned char *)src_udp->UdpA_Base +
                    ((src_udp->UdpL_Pos +
                    (src_udp->UdpL_ScnStride * src_udp->UdpL_Y1) +
                    (src_udp->UdpL_X1 * src_udp->UdpL_PxlStride) )>>3);

dst_byte_ptr = (unsigned char *)dst_udp->UdpA_Base +
                    ((dst_udp->UdpL_Pos +
                    (dst_udp->UdpL_ScnStride * dst_udp->UdpL_Y1) +
                    (dst_udp->UdpL_X1 * dst_udp->UdpL_PxlStride) )>>3);

cpp_stride = (cpp->UdpL_ScnStride >> 3);
row_ptr = (unsigned char *)src_plane_data_base;
cpp_ptr = (unsigned char *) cpp->UdpA_Base
                +(((cpp->UdpL_Pos +
                  (cpp->UdpL_ScnStride * cpp->UdpL_Y1) +
                   cpp->UdpL_X1)+7)/8);

src_stride = (src_udp->UdpL_ScnStride >> 3);
src_pad = dst_pad = src_stride  - src_udp->UdpL_PxlPerScn;
dst_stride = (dst_udp->UdpL_ScnStride >> 3);
dst_pad = dst_stride - dst_udp->UdpL_PxlPerScn;

switch (operator)
    {
    case IpsK_Addition:
        {
        switch (control)
            {
            case IpsK_Clip:
                {
		*result_flag = IpsK_SUCCESS_NOCLIP;
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                        {
                        if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
			    {
                            result = ((*row_ptr++) + (constant));
                            if ((result > 255) || (result < 0))
				{
			        if (result < 0)
			            *dst_byte_ptr++ = 0;
			        else
                                    *dst_byte_ptr++ = 255;
			        *result_flag = IpsK_SUCCESS_CLIP;
				}
                            else
                                *dst_byte_ptr++ = result;
			    }
                        else
                            *dst_byte_ptr++ = *row_ptr++;
                        }/* end ix loop */
		    cpp_ptr += cpp_stride;
                    dst_byte_ptr +=  dst_pad;
                    row_ptr += src_pad;
                    } /*end iy loop */
                break;
                }
            case IpsK_Wrap:
            default:
                {
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
		        {
                        if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
                            *dst_byte_ptr++ = ((*row_ptr++) + (constant)); 
			else
                            *dst_byte_ptr++ = *row_ptr++;
			}/* end ix loop */
                    cpp_ptr += cpp_stride;
                    dst_byte_ptr +=  dst_pad;
                    row_ptr += src_pad;
                    } /*end iy loop */
                break;
                }
            }/* end switch on control */
	break;
	}
    case IpsK_SubtractionByConstant:
        {
        switch (control)
            {
            case IpsK_Clip:
                {
		*result_flag = IpsK_SUCCESS_NOCLIP;
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                        {
                        if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
			    {
                            result = ((*row_ptr++) - (constant));
                            if ((result > 255) || (result < 0))
			        {
			        if (result < 0)
			            *dst_byte_ptr++ = 0;
			        else
                                    *dst_byte_ptr++ = 255;
			        *result_flag = IpsK_SUCCESS_CLIP;
				}
                            else
                                *dst_byte_ptr++ = result;
			    }
			else 
			    *dst_byte_ptr++ = *row_ptr++;
                        }/* end ix loop */
                    dst_byte_ptr +=  dst_pad;
                    row_ptr += src_pad;
                    cpp_ptr += cpp_stride;
                    } /*end iy loop */
                break;
                }
            case IpsK_Wrap:
            default:
                {
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
			{
                        if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
                            *dst_byte_ptr++ = ((*row_ptr++) - (constant)); 
			else
			    *dst_byte_ptr++ = *row_ptr++;
			}/* end ix loop */
                    dst_byte_ptr +=  dst_pad;
                    row_ptr += src_pad;
                    cpp_ptr += cpp_stride;
                    } /*end iy loop */
                break;
                }
            }/* end switch on control */
	break;
	}
    case IpsK_SubtractionFromConstant:
        {
        switch (control)
            {
            case IpsK_Clip:
                {
		*result_flag = IpsK_SUCCESS_NOCLIP;
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                        {
                        if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
			    {
                            result = ((constant) - (*row_ptr++));
                            if ((result > 255) || (result < 0))
				{
			        if (result < 0)
			            *dst_byte_ptr++ = 0;
			        else
                                    *dst_byte_ptr++ = 255;
			        *result_flag = IpsK_SUCCESS_CLIP;
				}
                            else
                                *dst_byte_ptr++ = result;
			    }
			else
			    *dst_byte_ptr = *row_ptr++;
                        }/* end ix loop */
                    dst_byte_ptr +=  dst_pad;
                    row_ptr += src_pad;
                    cpp_ptr += cpp_stride;
                    } /*end iy loop */
                break;
                }
            case IpsK_Wrap:
            default:
                {
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
		        {
                        if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
			    *dst_byte_ptr++ = ((constant) - (*row_ptr++));
			else
			    *dst_byte_ptr++ = *row_ptr++;
			}/* end ix loop */
                    dst_byte_ptr +=  dst_pad;
                    row_ptr += src_pad;
                    cpp_ptr += cpp_stride;
                    } /*end iy loop */
                break;
                }
            }/* end switch on control */
	break;
	}
    case IpsK_Multiplication:
        {
        switch (control)
            {
            case IpsK_Clip:
                {
		*result_flag = IpsK_SUCCESS_NOCLIP;
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                        {
			if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
			    {
                            result = ((*row_ptr++) * (constant));
                            if ((result > 255) || (result < 0))
			        {
			        if (result < 0)
			            *dst_byte_ptr++ = 0;
			        else
                                    *dst_byte_ptr++ = 255;
			        *result_flag = IpsK_SUCCESS_CLIP;
				}
                            else
                                *dst_byte_ptr++ = result;
			    }
			else
			    *dst_byte_ptr++ = *row_ptr++;
                        }/* end ix loop */
                    dst_byte_ptr +=  dst_pad;
                    row_ptr += src_pad;
                    cpp_ptr += cpp_stride;
                    } /*end iy loop */
                break;
                }
            case IpsK_Wrap:
            default:
                {
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
		        {
                        if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
                            *dst_byte_ptr++ = ((*row_ptr++) * (constant)); 
			else
			    *dst_byte_ptr++ = *row_ptr++;
			}/* end ix loop */
                    dst_byte_ptr +=  dst_pad;
                    row_ptr += src_pad;
                    cpp_ptr += cpp_stride;
                    } /*end iy loop */
                break;
                }
            }/* end switch on control */
	break;
	}
    case IpsK_DivisionByConstant:
        {
        if (constant == 0)
            return (IpsX_INVDARG);

        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
		{
                if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
                    *dst_byte_ptr++ = ((*row_ptr++) / (constant)); 
		else
		    *dst_byte_ptr++ = *row_ptr++;
		}/* end ix loop */
            dst_byte_ptr +=  dst_pad;
            row_ptr += src_pad;
            cpp_ptr += cpp_stride;
            } /*end iy loop */
        break;
	}
    case IpsK_DivisionFromConstant:
        {
	for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
		    {
		    if (*row_ptr != 0)
                        *dst_byte_ptr++ = ((constant) / (*row_ptr++));
		    else
			{
			*dst_byte_ptr++ = constant;
			row_ptr++;
			};
		    }
		else
		    *dst_byte_ptr++ = *row_ptr++;
                }/* end ix loop */
            dst_byte_ptr +=  dst_pad;
            row_ptr += src_pad;
            cpp_ptr += cpp_stride;
            } /*end iy loop */
	break;
	}
    case IpsK_Maximum:
	{
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
                    {
                    *dst_byte_ptr++ = MAX_((*row_ptr), constant);
		    row_ptr++;
		    }
		else
		    *dst_byte_ptr++ = *row_ptr++;
                } /* end of ix loop      */
            dst_byte_ptr +=  dst_pad;
            row_ptr  += src_pad;
            cpp_ptr += cpp_stride;
            }/* end of iy loop      */
	break;
	}

    case IpsK_Minimum:
        {
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
		if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
		    {
                    *dst_byte_ptr++ = MIN_((*row_ptr), constant);
		    row_ptr++;
		    }
		else
		    *dst_byte_ptr++ = *row_ptr++;
                }		/* end of ix loop      */
            dst_byte_ptr +=  dst_pad;
            row_ptr += src_pad;
            cpp_ptr += cpp_stride;
            }             /* end of iy loop      */
	break;
	}

    case IpsK_Modulo:
        {
	if (constant == 0)
	    return (IpsX_INVDARG);	    
	for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
                    *dst_byte_ptr++ = *row_ptr++ % (long)constant;
	        else
		    *dst_byte_ptr++ = *row_ptr++;
                }		/* end of ix loop      */
            dst_byte_ptr +=  dst_pad;
            row_ptr += src_pad;
            cpp_ptr += cpp_stride;
            }             /* end of iy loop      */
	break;
        }
    case IpsK_SetToConstant:
        {
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)		    
                    *dst_byte_ptr++ = (unsigned char )constant;
		else
		    *dst_byte_ptr++ = *row_ptr;
		row_ptr++;
                }		/* end of ix loop      */
            dst_byte_ptr +=  dst_pad;
            row_ptr += src_pad;
            cpp_ptr += cpp_stride;
            }             /* end of iy loop      */
	break;
	}
    default:
        return (IpsX_UNSOPTION);
        break;
    }
return (IpsX_SUCCESS);
}

/*****************************************************************************
**  _IpsConstantArithmeticW- 
**
**  FUNCTIONAL DESCRIPTION:
**  
**      Performs arithmetic operations on corresponding values from the
**      unsigned word source image and the constant to produce results 
**      for the unsigned word data type destination image on a point by 
**      point basis.
**
**  FORMAL PARAMETERS:
**
**      src_udp
**      dst_udp
**	constant
**      operator
**      control
**	result_flag
** 
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   none
**  SIGNAL CODES:     none
**  SIDE EFFECTS:     none
**
************************************************************************/
long _IpsConstantArithmeticW(src_udp, dst_udp, constant, operator, control, result_flag)
struct UDP *src_udp;       /* source word data type data plane     */
struct UDP *dst_udp;       /* destintion data plane                */
double constant;           /* constant to be used as operand       */
unsigned long operator;    /* arithmetic operation to be performed */
unsigned long control;     /* overflow/underflow control option    */
unsigned long *result_flag;/* indicates the nature of the results  */
{
unsigned long      size;                 /* buffer size                    */
unsigned short int *src_plane_data_base; /* pointer to source data base    */
unsigned short int *dst_word_ptr;        /* used for dst image             */
unsigned short int *row_ptr;		 /* dynamc row pointer             */
unsigned long      ix,iy;                /* loop counters                  */
unsigned long      src_stride;           /* src stride                     */
unsigned long      dst_stride;           /* dst stride                     */
unsigned long      src_pad;              /* src pad                        */
unsigned long      dst_pad;              /* dst pad                        */
long               result;		 /* true result before clip | wrap */

src_plane_data_base = (unsigned short int *)src_udp->UdpA_Base +
                    ((src_udp->UdpL_Pos +
                    (src_udp->UdpL_ScnStride * src_udp->UdpL_Y1) +
                    (src_udp->UdpL_X1 * src_udp->UdpL_PxlStride) )>>4);

dst_word_ptr = (unsigned short int *)dst_udp->UdpA_Base +
                    ((dst_udp->UdpL_Pos +
                    (dst_udp->UdpL_ScnStride * dst_udp->UdpL_Y1) +
                    (dst_udp->UdpL_X1 * dst_udp->UdpL_PxlStride) )>>4);

row_ptr = (unsigned short int *)src_plane_data_base;
src_stride = (src_udp->UdpL_ScnStride >> 4);
src_pad = src_stride - src_udp->UdpL_PxlPerScn;
dst_stride = (dst_udp->UdpL_ScnStride >> 4);
dst_pad = dst_stride - dst_udp->UdpL_PxlPerScn;
switch (operator)
    {
    case IpsK_Addition:
        {
        switch (control)
            {
            case IpsK_Clip:
                {
		*result_flag = IpsK_SUCCESS_NOCLIP;
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                        {
                        result = ((*row_ptr++) + (constant));
                        if ((result > 65535) || (result < 0))
			    {
			    if (result < 0)
			        *dst_word_ptr++ = 0;
			    else
                                *dst_word_ptr++ = 65535;
			    *result_flag = IpsK_SUCCESS_CLIP;
			    }
                        else
                            *dst_word_ptr++ = result;
                        }/* end ix loop */
                    dst_word_ptr +=  dst_pad;
                    row_ptr  += src_pad;
                    } /*end iy loop */
                break;
                }
            case IpsK_Wrap:
            default:
                {
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
		        {
                        *dst_word_ptr++ = ((*row_ptr++) + (constant)); 
			}/* end ix loop */
                    dst_word_ptr +=  dst_pad;
                    row_ptr  += src_pad;
                    } /*end iy loop */
                break;
                }
            }/* end switch on control */
	break;
	}

    case IpsK_SubtractionByConstant:
        {
        switch (control)
            {
            case IpsK_Clip:
                {
 		*result_flag = IpsK_SUCCESS_NOCLIP;
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                        {
                        result = ((*row_ptr++) - (constant));
                        if ((result > 65535) || (result < 0))
			    {
			    if (result < 0)
			        *dst_word_ptr++ = 0;
			    else
                                *dst_word_ptr++ = 65535;
			    *result_flag = IpsK_SUCCESS_CLIP;
			    }
                        else
                            *dst_word_ptr++ = result;
                        }/* end ix loop */
                    dst_word_ptr +=  dst_pad;
                    row_ptr  += src_pad;
                    } /*end iy loop */
                break;
                }
            case IpsK_Wrap:
            default:
                {
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
		        {
                        *dst_word_ptr++ = ((*row_ptr++) - (constant)); 
			}/* end ix loop */
                    dst_word_ptr +=  dst_pad;
                    row_ptr  += src_pad;
                    } /*end iy loop */
                break;
                }
            }/* end switch on control */
	break;
	}
    case IpsK_SubtractionFromConstant:
        {
        switch (control)
            {
            case IpsK_Clip:
		{
		*result_flag = IpsK_SUCCESS_NOCLIP;
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                        {
                        result = ((constant) - (*row_ptr++));
                        if ((result > 65535) || (result < 0))
			    {
			    if (result < 0)
			        *dst_word_ptr++ = 0;
			    else
                                *dst_word_ptr++ = 65535;
			    *result_flag = IpsK_SUCCESS_CLIP;
			    }
                        else
                            *dst_word_ptr++ = result;
                        }/* end ix loop */
                    dst_word_ptr +=  dst_pad;
                    row_ptr  += src_pad;
                    } /*end iy loop */
                break;
                }
            case IpsK_Wrap:
            default:
                {
                 for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
		        {
                        *dst_word_ptr++ = ((constant) - (*row_ptr++));
			}
                    dst_word_ptr +=  dst_pad;
                    row_ptr += src_pad;
                    } /*end iy loop */
                break;
                }
            }/* end switch on control */
	break;
	}
    case IpsK_Multiplication:
        {
        switch (control)
            {
            case IpsK_Clip:
                {
  		*result_flag = IpsK_SUCCESS_NOCLIP;
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                        {
                        result = ((*row_ptr++) * (constant));
                        if ((result > 65535) || (result < 0))
			    {
			    if (result < 0)
			        *dst_word_ptr++ = 0;
			    else
                                *dst_word_ptr++ = 65535;
			    *result_flag = IpsK_SUCCESS_CLIP;
			    }
                        else
                            *dst_word_ptr++ = result;
                        }/* end ix loop */
                    dst_word_ptr +=  dst_pad;
                    row_ptr += src_pad;
                    } /*end iy loop */
                break;
                }
            case IpsK_Wrap:
            default:
                {
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
		        {
                        *dst_word_ptr++ = ((*row_ptr++) * (constant)); 
			}/* end ix loop */
                    dst_word_ptr +=  dst_pad;
                    row_ptr += src_pad;
                    } /*end iy loop */
                break;
                }
            }/* end switch on control */
	break;
	}

    case IpsK_DivisionByConstant:
        {
        if (constant == 0)
            return (IpsX_INVDARG);
                  
	for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
		{
                *dst_word_ptr++ = ((*row_ptr++) / (constant)); 
		}/* end ix loop */
            dst_word_ptr +=  dst_pad;
            row_ptr += src_pad;
            } /*end iy loop */
	break;
	}
    case IpsK_DivisionFromConstant:
        {
	for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
		if (*row_ptr != 0)
                    *dst_word_ptr++ = ((constant) / (*row_ptr++));
		else
		    {
		    *dst_word_ptr++ = constant;
		    row_ptr++;
		    };
                }/* end ix loop */
            dst_word_ptr +=  dst_pad;
            row_ptr += src_pad;
            } /*end iy loop */
	break;
	}
    case IpsK_Maximum:
	{
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                *dst_word_ptr++ = MAX_((*row_ptr), constant);
		row_ptr++;
                } /* end of ix loop      */
            dst_word_ptr +=  dst_pad;
            row_ptr  += src_pad;
            } /* end of iy loop      */
	break;
	}
    case IpsK_Minimum:
	{
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                *dst_word_ptr++ = MIN_((*row_ptr), constant);
		row_ptr++;
                } /* end of ix loop */
            dst_word_ptr +=  dst_pad;
            row_ptr += src_pad;
            } /* end of iy loop */
	break;
	}
    case IpsK_Modulo:
        {
	if (constant == 0)
	    return (IpsX_INVDARG);	    
            
	for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                *dst_word_ptr++ = *row_ptr++ % (long)constant;
                } /* end of ix loop */
            dst_word_ptr +=  dst_pad;
            row_ptr  += src_pad;
            } /* end of iy loop */
	break;
	}
    case IpsK_SetToConstant:
        {
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                *dst_word_ptr++ = (unsigned short int)constant;
                } /* end of ix loop      */
            dst_word_ptr +=  dst_pad;
            row_ptr += src_pad;
            } /* end of iy loop      */
	break;
	}
    default:
        return (IpsX_UNSOPTION);
        break;
    }
return (IpsX_SUCCESS);
}


/*****************************************************************************
**  _IpsConstantArithmeticL- 
**
**  FUNCTIONAL DESCRIPTION:
**  
**      Performs arithmetic operations on corresponding values from the
**      unsigned longword source image and the constant to produce results 
**      for the unsigned long data type destination image on a point by 
**      point basis.
**
**  FORMAL PARAMETERS:
**
**      src_udp
**      dst_udp
**	constant
**      operator
**      control
**	result_flag
** 
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   none
**  SIGNAL CODES:     none
**  SIDE EFFECTS:     none
**
************************************************************************/
long _IpsConstantArithmeticL(src_udp, dst_udp, constant, operator, control, result_flag)
struct UDP *src_udp;       /* source longword data type data plane     */
struct UDP *dst_udp;       /* destintion data plane                */
double constant;           /* constant to be used as operand       */
unsigned long operator;    /* arithmetic operation to be performed */
unsigned long control;     /* overflow/underflow control option    */
unsigned long *result_flag;/* indicates the nature of the results  */
{
unsigned long      size;                 /* buffer size                    */
unsigned long      *src_plane_data_base; /* pointer to source data base    */
unsigned long      *dst_long_ptr;        /* used for dst image             */
unsigned long      *row_ptr;		 /* dynamc row pointer             */
unsigned long      ix,iy;                /* loop counters                  */
unsigned long      src_stride;           /* src stride                     */
unsigned long      dst_stride;           /* dst stride                     */
unsigned long      src_pad;              /* src pad                        */
unsigned long      dst_pad;              /* dst pad                        */
long               result;		 /* true result before clip | wrap */

src_plane_data_base = (unsigned long *)src_udp->UdpA_Base +
                    ((src_udp->UdpL_Pos +
                    (src_udp->UdpL_ScnStride * src_udp->UdpL_Y1) +
                    (src_udp->UdpL_X1 * src_udp->UdpL_PxlStride) )>>5);

dst_long_ptr = (unsigned long *)dst_udp->UdpA_Base +
                    ((dst_udp->UdpL_Pos +
                    (dst_udp->UdpL_ScnStride * dst_udp->UdpL_Y1) +
                    (dst_udp->UdpL_X1 * dst_udp->UdpL_PxlStride) )>>5);

row_ptr = (unsigned long *)src_plane_data_base;
src_stride = (src_udp->UdpL_ScnStride >> 5);
src_pad = src_stride - src_udp->UdpL_PxlPerScn;
dst_stride = (dst_udp->UdpL_ScnStride >> 5);
dst_pad = dst_stride - dst_udp->UdpL_PxlPerScn;
switch (operator)
    {
    case IpsK_Addition:
        {
        switch (control)
            {
            case IpsK_Clip:
                {
		*result_flag = IpsK_SUCCESS_NOCLIP;
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                        {
                        result = ((*row_ptr++) + (constant));
                        if ((result > 4294967295) || (result < 0))
			    {
			    if (result < 0)
			        *dst_long_ptr++ = 0;
			    else
                                *dst_long_ptr++ = 4294967295;
			    *result_flag = IpsK_SUCCESS_CLIP;
			    }
                        else
                            *dst_long_ptr++ = result;
                        }/* end ix loop */
                    dst_long_ptr +=  dst_pad;
                    row_ptr  += src_pad;
                    } /*end iy loop */
                break;
                }
            case IpsK_Wrap:
            default:
                {
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
		        {
                        *dst_long_ptr++ = ((*row_ptr++) + (constant)); 
			}/* end ix loop */
                    dst_long_ptr +=  dst_pad;
                    row_ptr  += src_pad;
                    } /*end iy loop */
                break;
                }
            }/* end switch on control */
	break;
	}

    case IpsK_SubtractionByConstant:
        {
        switch (control)
            {
            case IpsK_Clip:
                {
 		*result_flag = IpsK_SUCCESS_NOCLIP;
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                        {
                        result = ((*row_ptr++) - (constant));
                        if ((result > 4294967295) || (result < 0))
			    {
			    if (result < 0)
			        *dst_long_ptr++ = 0;
			    else
                                *dst_long_ptr++ = 4294967295;
			    *result_flag = IpsK_SUCCESS_CLIP;
			    }
                        else
                            *dst_long_ptr++ = result;
                        }/* end ix loop */
                    dst_long_ptr +=  dst_pad;
                    row_ptr  += src_pad;
                    } /*end iy loop */
                break;
                }
            case IpsK_Wrap:
            default:
                {
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
		        {
                        *dst_long_ptr++ = ((*row_ptr++) - (constant)); 
			}/* end ix loop */
                    dst_long_ptr +=  dst_pad;
                    row_ptr  += src_pad;
                    } /*end iy loop */
                break;
                }
            }/* end switch on control */
	break;
	}
    case IpsK_SubtractionFromConstant:
        {
        switch (control)
            {
            case IpsK_Clip:
		{
		*result_flag = IpsK_SUCCESS_NOCLIP;
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                        {
                        result = ((constant) - (*row_ptr++));
                        if ((result > 4294967295) || (result < 0))
			    {
			    if (result < 0)
			        *dst_long_ptr++ = 0;
			    else
                                *dst_long_ptr++ = 4294967295;
			    *result_flag = IpsK_SUCCESS_CLIP;
			    }
                        else
                            *dst_long_ptr++ = result;
                        }/* end ix loop */
                    dst_long_ptr +=  dst_pad;
                    row_ptr  += src_pad;
                    } /*end iy loop */
                break;
                }
            case IpsK_Wrap:
            default:
                {
                 for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
		        {
                        *dst_long_ptr++ = ((constant) - (*row_ptr++));
			}
                    dst_long_ptr +=  dst_pad;
                    row_ptr += src_pad;
                    } /*end iy loop */
                break;
                }
            }/* end switch on control */
	break;
	}
    case IpsK_Multiplication:
        {
        switch (control)
            {
            case IpsK_Clip:
                {
  		*result_flag = IpsK_SUCCESS_NOCLIP;
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                        {
                        result = ((*row_ptr++) * (constant));
                        if ((result > 4294967295) || (result < 0))
			    {
			    if (result < 0)
			        *dst_long_ptr++ = 0;
			    else
                                *dst_long_ptr++ = 4294967295;
			    *result_flag = IpsK_SUCCESS_CLIP;
			    }
                        else
                            *dst_long_ptr++ = result;
                        }/* end ix loop */
                    dst_long_ptr +=  dst_pad;
                    row_ptr += src_pad;
                    } /*end iy loop */
                break;
                }
            case IpsK_Wrap:
            default:
                {
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
		        {
                        *dst_long_ptr++ = ((*row_ptr++) * (constant)); 
			}/* end ix loop */
                    dst_long_ptr +=  dst_pad;
                    row_ptr += src_pad;
                    } /*end iy loop */
                break;
                }
            }/* end switch on control */
	break;
	}

    case IpsK_DivisionByConstant:
        {
        if (constant == 0)
            return (IpsX_INVDARG);
                  
	for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
		{
                *dst_long_ptr++ = ((*row_ptr++) / (constant)); 
		}/* end ix loop */
            dst_long_ptr +=  dst_pad;
            row_ptr += src_pad;
            } /*end iy loop */
	break;
	}
    case IpsK_DivisionFromConstant:
        {
	for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
		if (*row_ptr != 0)
                    *dst_long_ptr++ = ((constant) / (*row_ptr++));
		else
		    {
		    *dst_long_ptr++ = constant;
		    row_ptr++;
		    };
                }/* end ix loop */
            dst_long_ptr +=  dst_pad;
            row_ptr += src_pad;
            } /*end iy loop */
	break;
	}
    case IpsK_Maximum:
	{
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                *dst_long_ptr++ = MAX_((*row_ptr), constant);
		row_ptr++;
                } /* end of ix loop      */
            dst_long_ptr +=  dst_pad;
            row_ptr  += src_pad;
            } /* end of iy loop      */
	break;
	}
    case IpsK_Minimum:
	{
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                *dst_long_ptr++ = MIN_((*row_ptr), constant);
		row_ptr++;
                } /* end of ix loop */
            dst_long_ptr +=  dst_pad;
            row_ptr += src_pad;
            } /* end of iy loop */
	break;
	}
    case IpsK_Modulo:
        {
	if (constant == 0)
	    return (IpsX_INVDARG);	    
            
	for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                *dst_long_ptr++ = *row_ptr++ % (long)constant;
                } /* end of ix loop */
            dst_long_ptr +=  dst_pad;
            row_ptr  += src_pad;
            } /* end of iy loop */
	break;
	}
    case IpsK_SetToConstant:
        {
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                *dst_long_ptr++ = (unsigned long)constant;
                } /* end of ix loop      */
            dst_long_ptr +=  dst_pad;
            row_ptr += src_pad;
            } /* end of iy loop      */
	break;
	}
    default:
        return (IpsX_UNSOPTION);
        break;
    }
return (IpsX_SUCCESS);
}

/*****************************************************************************
**  _IpsConstantArithmeticWCpp- 
**
**  FUNCTIONAL DESCRIPTION:
**  
**      Performs arithmetic operations on corresponding values from the
**      unsigned word source image and the constant to produce results 
**      for the unsigned word data type destination image on a point by 
**      point basis.
**
**      This routine tests the bits in the CPP plane corresponding to
**      the pixels being operated on.
**
**          If the CPP bit is set, it performs the operation and
**          places the result in the corresponding destination plane pixel
**          location.
**
**          If the CPP bit is not set, the pixel value of the sourc1
**          plane is placed in the corresponding destination plane pixel
**          location.
**
**
**  FORMAL PARAMETERS:
**
**      src_udp
**      dst_udp
**      cpp
**	constant
**      operator
**      control
**	result_flag
**
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   none
**  SIGNAL CODES:     none
**  SIDE EFFECTS:     none
**
************************************************************************/
long _IpsConstantArithmeticWCpp(src_udp, dst_udp, cpp, constant, operator, control, result_flag)
struct UDP *src_udp;       /* source byte data type data plane     */
struct UDP *dst_udp;       /* destintion byte data type data plane */
struct UDP *cpp;           /* control processing plane             */
double constant;           /* constant to be used as operand	   */
unsigned long operator;    /* arithmetic operation to be performed */
unsigned long control;     /* overflow/underflow control option    */
unsigned long *result_flag; /* indicates the nature of the results  */
{
unsigned long      size;                 /* buffer size                    */
unsigned short int *src_plane_data_base; /* pointer to source data base    */
unsigned short int *dst_word_ptr;        /* used for dst image             */
unsigned short int *row_ptr;		 /* dynamc row pointer             */
unsigned long      ix,iy;                /* loop counters                  */
unsigned char      *cpp_ptr;             /* control processing plane base  */
unsigned long	   cpp_stride;		 /* control proc stride            */
unsigned long      src_pad;              /* src pad                        */
unsigned long      dst_pad;              /* dst pad                        */
unsigned long      src_stride;           /* src stride                     */
unsigned long      dst_stride;           /* dst stride                     */
long               result;		 /* signed result		   */

src_plane_data_base = (unsigned short int *)src_udp->UdpA_Base +
                    ((src_udp->UdpL_Pos +
                    (src_udp->UdpL_ScnStride * src_udp->UdpL_Y1) +
                    (src_udp->UdpL_X1 * src_udp->UdpL_PxlStride) )>>4);

dst_word_ptr = (unsigned short int *)dst_udp->UdpA_Base +
                    ((dst_udp->UdpL_Pos +
                    (dst_udp->UdpL_ScnStride * dst_udp->UdpL_Y1) +
                    (dst_udp->UdpL_X1 * dst_udp->UdpL_PxlStride) )>>4);

row_ptr = (unsigned short int *)src_plane_data_base;

cpp_ptr = (unsigned char *) cpp->UdpA_Base
                + (cpp->UdpL_Pos +
                  (cpp->UdpL_ScnStride * cpp->UdpL_Y1) +
                   cpp->UdpL_X1);
cpp_stride = (cpp->UdpL_ScnStride >> 3);
src_stride = (src_udp->UdpL_ScnStride >> 4);
src_pad = src_stride - src_udp->UdpL_PxlPerScn;
dst_stride = (dst_udp->UdpL_ScnStride >> 4);
dst_pad = dst_stride - dst_udp->UdpL_PxlPerScn;

switch (operator)
    {
    case IpsK_Addition:
        {
        switch (control)
            {
            case IpsK_Clip:
                {
		*result_flag = IpsK_SUCCESS_NOCLIP;
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                        {
                        if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
			    {
                            result = ((*row_ptr++) + (constant));
                            if ((result > 65535) || (result < 0))
				{
			        if (result < 0)
			            *dst_word_ptr++ = 0;
			        else
                                    *dst_word_ptr++ = 65535;
			        *result_flag = IpsK_SUCCESS_CLIP;
				}
                            else
                                *dst_word_ptr++ = result;
			    }
			else
			    *dst_word_ptr++ = *row_ptr++;    
                        }/* end ix loop */
                    cpp_ptr += cpp_stride;
                    dst_word_ptr +=  dst_pad;
                    row_ptr  += src_pad;
                    } /*end iy loop */
                break;
                }
            case IpsK_Wrap:
            default:
                {
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
		        {
                        if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
		            {
                            *dst_word_ptr++ = ((*row_ptr++) + (constant)); 
			    }
			else
			    *dst_word_ptr++ = *row_ptr++;
			}/* end ix loop */
                    cpp_ptr += cpp_stride;
                    dst_word_ptr +=  dst_pad;
                    row_ptr += src_pad;
                    } /*end iy loop */
                break;
                }
            }/* end switch on control */
	break;
	}
    case IpsK_SubtractionByConstant:
        {
        switch (control)
            {
            case IpsK_Clip:
                {
		*result_flag = IpsK_SUCCESS_NOCLIP;
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                        {
                        if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
			    {
                            result = ((*row_ptr++) - (constant));
                            if ((result > 65535) || (result < 0))
				{
			        if (result < 0)
			            *dst_word_ptr++ = 0;
			        else
                                    *dst_word_ptr++ = 65535;
			        *result_flag = IpsK_SUCCESS_CLIP;
				}
                            else
                                *dst_word_ptr++ = result;
			    }
			else
			    *dst_word_ptr++ = *row_ptr++;
                        }/* end ix loop */
                    dst_word_ptr +=  dst_pad;
                    row_ptr += src_pad;
                    cpp_ptr += cpp_stride;
                    } /*end iy loop */
                break;
                }
            case IpsK_Wrap:
            default:
                {
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
		        {
                        if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
                            *dst_word_ptr++ = ((*row_ptr++) - (constant)); 
			else
			    *dst_word_ptr++ = *row_ptr;
			}/* end ix loop */
                    dst_word_ptr +=  dst_pad;
                    row_ptr  += src_pad;
                    cpp_ptr += cpp_stride;
                    } /*end iy loop */
                break;
                }
            }/* end switch on control */
	break;
	}
    case IpsK_SubtractionFromConstant:
        {
        switch (control)
            {
            case IpsK_Clip:
		{
		*result_flag = IpsK_SUCCESS_NOCLIP;
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                        {
                        if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
			    {
                            result = ((constant) - (*row_ptr++));
                            if ((result > 65535) || (result < 0))
			        {
			        if (result < 0)
			            *dst_word_ptr++ = 0;
			        else
                                    *dst_word_ptr++ = 65535;
			        *result_flag = IpsK_SUCCESS_CLIP;
				}
                            else
                                *dst_word_ptr++ = result;
			    }
			else
			    *dst_word_ptr++ = *row_ptr++;
                        }/* end ix loop */
                    dst_word_ptr +=  dst_pad;
                    row_ptr += src_pad;
                    cpp_ptr += cpp_stride;
                    } /*end iy loop */
                break;
                }
            case IpsK_Wrap:
            default:
                {
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
			{
                        if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
                            *dst_word_ptr++ = ((constant) - (*row_ptr++));
			else
			    *dst_word_ptr++ = *row_ptr++;
			}/* end ix loop */
                    dst_word_ptr +=  dst_pad;
                    row_ptr += src_pad;
                    cpp_ptr += cpp_stride;
                    } /*end iy loop */
                break;
                }
            }/* end switch on control */
	break;
	}
    case IpsK_Multiplication:
        {
        switch (control)
            {
            case IpsK_Clip:
                {
		*result_flag = IpsK_SUCCESS_NOCLIP;
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                        {
                        if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
			    {
                            result = ((*row_ptr++) * (constant));
                            if ((result > 65535) || (result < 0))
			        {
			        if (result < 0)
			            *dst_word_ptr++ = 0;
			        else
                                    *dst_word_ptr++ = 65535;
			        *result_flag = IpsK_SUCCESS_CLIP;
				}
                            else
                                *dst_word_ptr++ = result;
			    }
			else
			    *dst_word_ptr++ = *row_ptr++;
                        }/* end ix loop */
                    dst_word_ptr +=  dst_pad;
                    row_ptr += src_pad;
                    cpp_ptr += cpp_stride;
                    } /*end iy loop */
                break;
                }
            case IpsK_Wrap:
            default:
                {
                for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
                    {
                    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
			{
                        if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
                            *dst_word_ptr++ = ((*row_ptr++) * (constant)); 
			else
			    *dst_word_ptr++ = *row_ptr++;
			}
                    dst_word_ptr +=  dst_pad;
                    row_ptr += src_pad;
                    cpp_ptr += cpp_stride;
                    } /*end iy loop */
                break;
                }
            }/* end switch on control */
	break;
	}

    case IpsK_DivisionByConstant:
        {
	if (constant == 0)
	    return (IpsX_INVDARG);

        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
		{
                if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
                    *dst_word_ptr++ = ((*row_ptr++) / (constant)); 
		else
		    *dst_word_ptr++ = *row_ptr++;
		}/* end ix loop */
            dst_word_ptr +=  dst_pad;
            row_ptr += src_pad;
            cpp_ptr += cpp_stride;
            } /*end iy loop */
	break;
	}
    case IpsK_DivisionFromConstant:
        {
	for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
		    {
		    if (*row_ptr != 0)
                        *dst_word_ptr++ = ((constant) / (*row_ptr++));
		    else
			{
			*dst_word_ptr++ = constant;
			row_ptr++;
			};
		    }
		else
		    *dst_word_ptr++ = *row_ptr++;
                }/* end ix loop */
            dst_word_ptr +=  dst_pad;
            row_ptr += src_pad;
            cpp_ptr += cpp_stride;
            } /*end iy loop */
	break;
	}
    case IpsK_Maximum:
	{
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
		    {
                    *dst_word_ptr++ = MAX_((*row_ptr), constant);
		    row_ptr++;
		    }
		else
		    *dst_word_ptr++ = *row_ptr++;
                } /* end of ix loop */
            dst_word_ptr +=  dst_pad;
            row_ptr += src_pad;
            cpp_ptr += cpp_stride;
            } /* end of iy loop */
	break;
	}
    case IpsK_Minimum:
	{
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
	        {
                if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)                
		    {
                    *dst_word_ptr++ = MIN_((*row_ptr), constant);
		    row_ptr++;
		    }
		else
		    *dst_word_ptr++ = *row_ptr++;
                } /* end of ix loop      */
            dst_word_ptr +=  dst_pad;
            row_ptr += src_pad;
            cpp_ptr += cpp_stride;
            } /* end of iy loop      */
	break;
	}

    case IpsK_Modulo:
        {
	if (constant == 0)
	    return (IpsX_INVDARG);	    

        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)		    
                    *dst_word_ptr++ = *row_ptr++ % (long)constant;
	        else
	            *dst_word_ptr++ = *row_ptr++;
                } /* end of ix loop */
            dst_word_ptr +=  dst_pad;
            row_ptr += src_pad;
            cpp_ptr += cpp_stride;
            } /* end of iy loop */
	break;
	}
    case IpsK_SetToConstant:
        {
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)		    
                    *dst_word_ptr++ = (unsigned short)constant;
		else
		    *dst_word_ptr++ = *row_ptr;
		row_ptr++;
                }		/* end of ix loop      */
            dst_word_ptr +=  dst_pad;
            row_ptr += src_pad;
            cpp_ptr += cpp_stride;
            }             /* end of iy loop      */
	break;
	}
    default:
        return (IpsX_UNSOPTION);
        break;
    }
return (IpsX_SUCCESS);
}

/*****************************************************************************
**  _IpsConstantArithmeticFlt- 
**
**  FUNCTIONAL DESCRIPTION:
**  
**      Performs arithmetic operations on corresponding values from the
**      floating point data type source image and the constant to produce 
**      results for the floating point data type destination image on a 
**      point by point basis.
**  
**  FORMAL PARAMETERS:
**
**      src_udp
**      dst_udp
**	constant
**      operator
**
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   none
**  SIGNAL CODES:     none
**  SIDE EFFECTS:     none
**
************************************************************************/
long _IpsConstantArithmeticFlt(src_udp, dst_udp, constant, operator)
struct UDP *src_udp;/* source float data type data plane		*/
struct UDP *dst_udp;/* destination data plane				*/
double constant;    /* constant to be used in the arithmetic opertation */
long operator;      /* arithmetic operation to be performed		*/
{
long            size;                   /* buffer size                  */
float           *src_plane_data_base;   /* pointer to source data base  */
float           *dst_float_ptr;         /* used for dst image           */
float		*row_ptr;		/* dynamc row pointer           */
unsigned long   ix,iy;                  /* loop counters                */
unsigned long	src_stride;		/* source scanline stride       */
unsigned long	dst_stride;		/* destination scanline stride  */
unsigned long   src_pad;                /* src pad                      */
unsigned long   dst_pad;                /* dst pad                      */
	    
src_plane_data_base = (float *)src_udp->UdpA_Base +
                   ((src_udp->UdpL_Pos +
                    (src_udp->UdpL_ScnStride * src_udp->UdpL_Y1) +
                    (src_udp->UdpL_X1 * src_udp->UdpL_PxlStride) )>>5);

row_ptr = (float *)src_plane_data_base;

dst_float_ptr = (float *) dst_udp->UdpA_Base +
                    ((dst_udp->UdpL_Pos +
                    (dst_udp->UdpL_ScnStride * dst_udp->UdpL_Y1) +
                    (dst_udp->UdpL_X1 * dst_udp->UdpL_PxlStride) )>>5);

src_stride = (src_udp->UdpL_ScnStride >> 5);
src_pad = src_stride - src_udp->UdpL_PxlPerScn;
dst_stride = (dst_udp->UdpL_ScnStride >> 5);
dst_pad = dst_stride - dst_udp->UdpL_PxlPerScn;

switch (operator)
    {
    case IpsK_Addition:
        {
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
		{
                *dst_float_ptr++ = *row_ptr++ + constant;		 
		}/* end of ix loop */  
            dst_float_ptr +=  dst_pad;
            row_ptr  += src_pad;
            } /* end of iy loop */
	break;
	}
    case IpsK_SubtractionByConstant:
        {
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
		{
                *dst_float_ptr++ = *row_ptr++ - constant;
		} /* end of ix loop */
            dst_float_ptr +=  dst_pad;
            row_ptr += src_pad;
            } /* end of iy loop */
	break;
	}

    case IpsK_SubtractionFromConstant:
        {
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
		{
                *dst_float_ptr++ = constant - *row_ptr++;
		} /* end of ix loop */
            dst_float_ptr +=  dst_pad;
            row_ptr += src_pad;
            } /* end of iy loop      */
	break;
	}
    case IpsK_Multiplication:
        {
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
	        {
                *dst_float_ptr++ = *row_ptr++ * constant;
		} /* end of ix loop */
            dst_float_ptr +=  dst_pad;
            row_ptr += src_pad;
            } /* end of iy loop */
	break;
	}

    case IpsK_DivisionByConstant:
	{
	if (constant == 0)
	    return (IpsX_INVDARG);
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                *dst_float_ptr++ = *row_ptr++ / constant;
                } /* end of ix loop      */
            dst_float_ptr +=  dst_pad;
            row_ptr += src_pad;
            } /* end of iy loop      */
	break;
	}

    case IpsK_DivisionFromConstant:
	{
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
		if (*row_ptr != 0)
                    *dst_float_ptr++ = constant / (*row_ptr++);
		else
		    {
	  	    *dst_float_ptr++ = constant;
		    row_ptr++;
		    };
                } /* end of ix loop      */
            dst_float_ptr +=  dst_pad;
            row_ptr += src_pad;
            } /* end of iy loop      */
	break;
	}
    case IpsK_Maximum:
	{
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                *dst_float_ptr++ = MAX_((*row_ptr), constant);
		row_ptr++;
                } /* end of ix loop      */
            dst_float_ptr +=  dst_pad;
            row_ptr += src_pad;
            } /* end of iy loop      */
	break;
	}
    case IpsK_Minimum:
	{
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                *dst_float_ptr++ = MIN_((*row_ptr), constant);
		row_ptr++;
                } /* end of ix loop      */
            dst_float_ptr +=  dst_pad;
            row_ptr += src_pad;
            } /* end of iy loop      */
	break;
	}
    case IpsK_Modulo:
        {
	if (constant == 0)
	    return (IpsX_INVDARG);
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                *dst_float_ptr++ = (long )*row_ptr++ % (long)constant;
                } /* end of ix loop      */
            dst_float_ptr +=  dst_pad;
            row_ptr += src_pad;
            } /* end of iy loop      */
	break;
	}
    case IpsK_SetToConstant:
        {
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                *dst_float_ptr++ = constant;
                }		/* end of ix loop      */
            dst_float_ptr +=  dst_pad;
            row_ptr += src_pad;
            }             /* end of iy loop      */
	break;
	}
    default:
        return (IpsX_UNSOPTION);
        break;
    }
return (IpsX_SUCCESS);
}

/*****************************************************************************
**  _IpsConstantArithmeticFltCpp- 
**
**  FUNCTIONAL DESCRIPTION:
**  
**      Performs arithmetic operations on corresponding values from the
**      floating point data type source image and the constant to produce 
**      results for the floating point data type destination image on a 
**      point by point basis.
**  
**      This routine tests the bits in the CPP plane corresponding to
**      the pixels being operated on.
**
**          If the CPP bit is set, it performs the operation and
**          places the result in the corresponding destination plane pixel
**          location.
**
**          If the CPP bit is not set, the pixel value of the sourc1
**          plane is placed in the corresponding destination plane pixel
**          location.
**
**  FORMAL PARAMETERS:
**
**      src_udp
**      dst_udp
**	cpp
**	constant
**      operator
**
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   none
**  SIGNAL CODES:     none
**  SIDE EFFECTS:     none
**
************************************************************************/
long _IpsConstantArithmeticFltCpp(src_udp, dst_udp, cpp, constant, operator)
struct UDP *src_udp; /* source floating point dtype data plane               */
struct UDP *dst_udp; /* destination data plane                               */
struct UDP *cpp;     /* control processing plane                             */
double constant;     /* constant (second operand)in the arithmetic operation */
long operator;	     /* arithmetic operation to be performed                 */
{
unsigned long   size;                   /* buffer size                    */
float           *src_plane_data_base;   /* pointer to source data base    */
float           *dst_float_ptr;         /* used for dst image             */
float		*row_ptr;		/* dynamc row pointer             */
unsigned long   ix,iy;                  /* loop counters                  */
unsigned char   *cpp_ptr;               /* control processing plane base  */
unsigned long   cpp_stride;		/* control proc scanline stride   */
unsigned long	src_stride;		/* source scanline stride         */
unsigned long	dst_stride;		/* destination scanline stride    */
unsigned long   src_pad;		/* source pad			  */
unsigned long   dst_pad;		/* destination pad		  */

src_plane_data_base = (float *)src_udp->UdpA_Base +
                   ((src_udp->UdpL_Pos +
                    (src_udp->UdpL_ScnStride * src_udp->UdpL_Y1) +
                    (src_udp->UdpL_X1 * src_udp->UdpL_PxlStride) )>>5);

row_ptr = (float *)src_plane_data_base;

dst_float_ptr = (float *) dst_udp->UdpA_Base +
                    ((dst_udp->UdpL_Pos +
                    (dst_udp->UdpL_ScnStride * dst_udp->UdpL_Y1) +
                    (dst_udp->UdpL_X1 * dst_udp->UdpL_PxlStride) )>>5);

cpp_ptr = (unsigned char *) cpp->UdpA_Base
                + (cpp->UdpL_Pos +
                  (cpp->UdpL_ScnStride * cpp->UdpL_Y1) +
                   cpp->UdpL_X1);
cpp_stride = (cpp->UdpL_ScnStride >> 3);
src_stride = (src_udp->UdpL_ScnStride >> 5);
dst_stride = (dst_udp->UdpL_ScnStride >> 5);
src_pad = src_stride - src_udp->UdpL_PxlPerScn;
dst_pad = dst_stride - dst_udp->UdpL_PxlPerScn;
switch (operator)
    {
    case IpsK_Addition:
        {
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
	        {
                if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
                    *dst_float_ptr++ = *row_ptr++ + constant;		   
		else
		    *dst_float_ptr++ = *row_ptr++;
		} /* end of ix loop */
            cpp_ptr += cpp_stride;
            dst_float_ptr +=  dst_pad;
            row_ptr += src_pad;
            } /* end of iy loop */
	break;
	}
    case IpsK_SubtractionByConstant:
        {
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
	        {
                if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
                    *dst_float_ptr++ = *row_ptr++ - constant;
		else
		    *dst_float_ptr++ = *row_ptr++;
                } /* end of ix loop */
            cpp_ptr += cpp_stride;
            dst_float_ptr +=  dst_pad;
            row_ptr += src_pad;
            } /* end of iy loop */
	break;
	}

    case IpsK_SubtractionFromConstant:
        {
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
	        {
                if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
                    *dst_float_ptr++ = constant - *row_ptr++;
		else
		    *dst_float_ptr++ = *row_ptr++;
		} /* end of ix loop */
            cpp_ptr += cpp_stride;
            dst_float_ptr +=  dst_pad;
            row_ptr  += src_pad;
            } /* end of iy loop */
	break;
	}
    case IpsK_Multiplication:
        {
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
		{
		if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
                    *dst_float_ptr++ = *row_ptr++ * constant;
		else
		    *dst_float_ptr++ = *row_ptr++;
		} /* end of ix loop */
            cpp_ptr += cpp_stride;
            dst_float_ptr +=  dst_pad;
            row_ptr += src_pad;
            } /* end of iy loop      */
	break;
	}

    case IpsK_DivisionByConstant:
	{
	if (constant == 0)
	    return (IpsX_INVDARG);
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
                    *dst_float_ptr++ = *row_ptr++ / constant;
		else
		    *dst_float_ptr++ = *row_ptr++;
                } /* end of ix loop      */
            cpp_ptr += cpp_stride;
            dst_float_ptr +=  dst_pad;
            row_ptr += src_pad;
            } /* end of iy loop      */
	break;
	}

    case IpsK_DivisionFromConstant:
	{
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
		    {
		    if (*row_ptr != 0)
                        *dst_float_ptr++ = constant / (*row_ptr++);
		    else
			{
		        row_ptr++;
			*dst_float_ptr++ = constant;
			};
		    }
		else
		    *dst_float_ptr++ = *row_ptr++;
                } /* end of ix loop      */
            cpp_ptr += cpp_stride;
            dst_float_ptr +=  dst_pad;
            row_ptr += src_pad;
            } /* end of iy loop      */
	break;
	}
    case IpsK_Maximum:
	{
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
		    {
                    *dst_float_ptr++ = MAX_((*row_ptr), constant);
		    row_ptr++;
		    }
		else
		    *dst_float_ptr++ = *row_ptr++;
                } /* end of ix loop      */
            cpp_ptr += cpp_stride;
            dst_float_ptr +=  dst_pad;
            row_ptr += src_pad;
            }/* end of iy loop      */
	break;
	}
    case IpsK_Minimum:
	{
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
		    {
                    *dst_float_ptr++ = MIN_((*row_ptr), constant);
		    row_ptr++;
		    }
		else
		    *dst_float_ptr++ = *row_ptr++;
                } /* end of ix loop      */
            cpp_ptr += cpp_stride;
            dst_float_ptr +=  dst_pad;
            row_ptr += src_pad;
            } /* end of iy loop      */
	break;
	}
    case IpsK_Modulo:
        {
	if (constant == 0)
	    return (IpsX_INVDARG);
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
                    *dst_float_ptr++ = (long )*row_ptr++ % (long)constant;
		else
		    *dst_float_ptr++ = *row_ptr++;
                } /* end of ix loop      */
            cpp_ptr += cpp_stride;
            dst_float_ptr +=  dst_pad;
            row_ptr += src_pad;
            } /* end of iy loop      */
	break;
	}
    case IpsK_SetToConstant:
        {
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)		    
                    *dst_float_ptr++ = constant;
		else
		    *dst_float_ptr++ = *row_ptr;
		row_ptr++;
                }		/* end of ix loop      */
            cpp_ptr += cpp_stride;
            dst_float_ptr +=  dst_pad;
            row_ptr += src_pad;
            }             /* end of iy loop      */
	break;
	}
    default:
        return (IpsX_UNSOPTION);
        break;
    }
return (IpsX_SUCCESS);
}
