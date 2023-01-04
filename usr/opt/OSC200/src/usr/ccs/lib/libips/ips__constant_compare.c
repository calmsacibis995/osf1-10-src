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
/******************************************************************************
**
**  FACILITY:
**
**      Image Processing Services
**
**  ABSTRACT:
**
**      This module contains the user level service and support routines
**	for comparison operations on an image using a constant as the 
**	second operand.
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
**      26-MAR-1990
**
************************************************************************/

/*
**  Table of contents
*/
#ifdef NODAS_PROTO
long _IpsConstantCompare();	    /* main entry routine, tests dtype */
long _IpsConstantCompareByt();      /* compare operation on byte dtype source */
long _IpsConstantCompareW ();       /* compare operation on word dtype source */
long _IpsConstantCompareFlt();      /* compare operation on float dtype source*/
#endif


/*
**  Include files
*/
#include <IpsDef.h>			    /* IPS Definitions		     */
#include <IpsMacros.h>                      /* IPS Macro Definitions         */
#include <IpsMemoryTable.h>                 /* IPS Memory Mgt Functions      */
#include <IpsStatusCodes.h>                 /* IPS Status Codes              */
#ifndef NODAS_PROTO
#include <ipsprot.h>			    /* Ips prototypes */
#endif

/*
**  External references
*/
#ifdef NODAS_PROTO
long _IpsVerifyNotInPlace();		    /* from IPS__UDP_UTILS.C         */
#endif

/*****************************************************************************
**  _IpsConstantCompare
**
**  FUNCTIONAL DESCRIPTION:
**
**      Performs comparison operations on corresponding values from the
**      source image and the constant to produce results for the destination 
**      bitmap image on a point by point basis.
**
**  FORMAL PARAMETERS:
**
**      src_udp      -- Pointer to source udp
**      dst_udp      -- Pointer to destination udp 
**	constant     -- Constant to be compared with each source pixel
**      operator     -- Comparison operation to be performed
**      result_flag  -- Flags specifying the nature of the results
**
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   none
**  SIGNAL CODES:     none
**  SIDE EFFECTS:     none
**
************************************************************************/
long _IpsConstantCompare(src_udp,dst_udp,constant,operator,result_flag)
struct UDP *src_udp;       /* source udp				   */
struct UDP *dst_udp;       /* destination udp				   */
float constant;            /* constant to be used in the compare operation */
unsigned long operator;    /* compare operator				   */
unsigned long *result_flag;/* specifies the nature of the results	   */
{
unsigned long size; 
unsigned long mem_allocated;
long byte_boundary_padbits;
long status;

if  (src_udp->UdpB_Class != UdpK_ClassA)
    return (IpsX_UNSOPTION);

if ((src_udp->UdpB_DType == UdpK_DTypeVU) ||
    (src_udp->UdpB_DType == UdpK_DTypeV))
    return (IpsX_INVDTYPE);

dst_udp->UdpL_PxlPerScn = src_udp->UdpL_PxlPerScn;
dst_udp->UdpL_ScnCnt = src_udp->UdpL_ScnCnt;
dst_udp->UdpL_X1 = 0;
dst_udp->UdpL_Y1 = 0;
dst_udp->UdpL_X2 = src_udp->UdpL_PxlPerScn - 1;
dst_udp->UdpL_Y2 = src_udp->UdpL_ScnCnt - 1;
byte_boundary_padbits = (8 - (src_udp->UdpL_PxlPerScn % 8)) % 8;
dst_udp->UdpL_ScnStride = src_udp->UdpL_PxlPerScn + byte_boundary_padbits;
dst_udp->UdpW_PixelLength = 1;
dst_udp->UdpL_PxlStride = 1;
dst_udp->UdpB_DType = UdpK_DTypeVU;
dst_udp->UdpB_Class = UdpK_ClassUBA;
size = dst_udp->UdpL_ScnStride * dst_udp->UdpL_ScnCnt;
dst_udp->UdpL_Levels = 2;
if (dst_udp->UdpA_Base != 0)
    {
    /* No in place allowed */
    status = _IpsVerifyNotInPlace (src_udp, dst_udp);
        if (status != IpsX_SUCCESS) return (status);
    if ((dst_udp->UdpL_ArSize - dst_udp->UdpL_Pos) < size)
        return (IpsX_INSVIRMEM);
    }
else
    {
    dst_udp->UdpA_Base = (unsigned char *)
        (*IpsA_MemoryTable[IpsK_AllocateDataPlane])
                (((size + 7) >> 3),IpsM_InitMem, 0);
    if (!dst_udp->UdpA_Base) return (IpsX_INSVIRMEM);
    mem_allocated = TRUE;
    dst_udp->UdpL_ArSize = size;
    dst_udp->UdpL_Pos = 0;
    }

*result_flag = 0;
switch (src_udp->UdpB_DType)
    {
    case UdpK_DTypeBU:
	status = _IpsConstantCompareByt
		    (src_udp, dst_udp, constant, operator, result_flag);
    break;

    case UdpK_DTypeWU:
	status = _IpsConstantCompareW
		    (src_udp, dst_udp, constant, operator, result_flag);
    break;

    case UdpK_DTypeF:
	status = _IpsConstantCompareFlt
		    (src_udp, dst_udp, constant, operator, result_flag);
    break;

    case UdpK_DTypeLU:
    default:
	status = IpsX_UNSOPTION;
    break;
    };  /* end switch on DType */

if ((status != IpsX_SUCCESS) && (mem_allocated == TRUE))
    (*IpsA_MemoryTable[IpsK_FreeDataPlane]) (dst_udp->UdpA_Base);

return (status);
}

/*****************************************************************************
**  _IpsConstantCompareByt- 
**
**  FUNCTIONAL DESCRIPTION:
**  
**      Performs compare operations on corresponding values from the
**      unsigned byte source image and the constant to produce results 
**      for the bitmap destination image on a point by point basis.
**
**  FORMAL PARAMETERS:
**
**      src_udp     -- Pointer to byte datatype source udp.
**      dst_udp     -- Pointer to destination bitonal udp.
**	constant    -- Constant operand.
**      operator    -- Comparison operation to be performed.
**	result_flag -- Indicates the nature of the results.
**
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   none
**  SIGNAL CODES:     none
**  SIDE EFFECTS:     none
**
************************************************************************/
long _IpsConstantCompareByt(src_udp, dst_udp, constant, operator, result_flag)
struct UDP *src_udp;        /* source byte data type data plane      */
struct UDP *dst_udp;        /* destintion bitmap data plane          */
float constant;             /* constant operand in compare operation */
unsigned long operator;	    /* comparison operation to be performed  */
unsigned long *result_flag; /* indicates the nature of the results   */
{
long            size;                   /* buffer size                 */
unsigned char   *src_plane_data_base;   /* pointer to source data base */
unsigned char   *dst_ptr;               /* used for dst image          */
unsigned char   *row_ptr;		/* dynamc row pointer          */
unsigned long   ix,iy;                  /* loop counters               */
unsigned long   dst_stride;		/* dst scanline stride	       */
unsigned long   src_stride;		/* src scanline stride         */
unsigned long   src_pad;                /* src padding at end of line  */
unsigned long   set = 0;		/* counter for result_flag info*/

src_plane_data_base = (unsigned char *)(src_udp->UdpA_Base +
                    ((src_udp->UdpL_Pos +
                    (src_udp->UdpL_ScnStride * src_udp->UdpL_Y1) +
                    (src_udp->UdpL_X1 * src_udp->UdpL_PxlStride) )>>3));

dst_ptr = (unsigned char *)(dst_udp->UdpA_Base +
                   ((dst_udp->UdpL_Pos +
                    (dst_udp->UdpL_ScnStride * dst_udp->UdpL_Y1) +
                    (dst_udp->UdpL_X1))>>3));
 
row_ptr = (unsigned char *)src_plane_data_base;
dst_stride = (dst_udp->UdpL_ScnStride >> 3); 
src_stride = (src_udp->UdpL_ScnStride >> 3);
src_pad = src_stride - src_udp->UdpL_PxlPerScn;
 
switch (operator)
    {
    case IpsK_EQ:
        {
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                if ((*src_plane_data_base++) == (constant))
                    {
                    PUT_BIT_VALUE_(dst_ptr,ix,TRUE);
		    set++;
                    }
                }/* end ix loop */
            src_plane_data_base  += src_pad;
            dst_ptr += dst_stride;
            } /*end iy loop */
        break;
        }/*end EQ */
    case IpsK_NE:
        {
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                if ((*src_plane_data_base++) != (constant))
                    {
                    PUT_BIT_VALUE_(dst_ptr,ix,TRUE);
		    set++;
                    }
                }/* end ix loop */
            src_plane_data_base  += src_pad;
            dst_ptr += dst_stride;
            } /*end iy loop */
        break;
        }/*end NE */ 
    case IpsK_LT:
        {
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                if ((*src_plane_data_base++) < (constant))
                    {
                    PUT_BIT_VALUE_(dst_ptr,ix,TRUE);
		    set++;
                    }
                }/* end ix loop */
            src_plane_data_base  += src_pad;
            dst_ptr += dst_stride;
            } /*end iy loop */
        break;
        }/*end LT */ 
    case IpsK_LE:
        {
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                if ((*src_plane_data_base++) <= (constant))
                    {
                    PUT_BIT_VALUE_(dst_ptr,ix,TRUE);
		    set++;
                    }
                }/* end ix loop */
            src_plane_data_base  += src_pad;
            dst_ptr += dst_stride;
            } /*end iy loop */
        break;
        }/*end LE */ 
    case IpsK_GT:
        {
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                if ((*src_plane_data_base++) > (constant))
                    {
                    PUT_BIT_VALUE_(dst_ptr,ix,TRUE);
		    set++;
                    }
                }/* end ix loop */
            src_plane_data_base  += src_pad;
            dst_ptr += dst_stride;
            } /*end iy loop */
        break;
        }/*end GT */ 
    case IpsK_GE:
        {
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                if ((*src_plane_data_base++) >= (constant))
                    {
                    PUT_BIT_VALUE_(dst_ptr,ix,TRUE);
		    set++;
                    }
                }/* end ix loop */
            src_plane_data_base  += src_pad;
            dst_ptr += dst_stride;
            } /*end iy loop */
        break;
        }/*end GE */ 
    default:
        return (IpsX_UNSOPTION);
        break;
    }

if (set == 0)
    *result_flag = IpsK_SUCCESS_CLEAR;
else
    {
    if (set == (src_udp->UdpL_PxlPerScn * src_udp->UdpL_ScnCnt))
        *result_flag = IpsK_SUCCESS_SET;
    else
        *result_flag = IpsK_SUCCESS_SOME;
    }

return (IpsX_SUCCESS);
}

/*****************************************************************************
**  _IpsConstantCompareW- 
**
**  FUNCTIONAL DESCRIPTION:
**  
**      Performs compare operations on corresponding values from the
**      unsigned word source image and the constant to produce results 
**      for the bitmap destination image on a point by point basis.
**
**  FORMAL PARAMETERS:
**
**
**      src_udp     -- Pointer to word datatype source udp.
**      dst_udp     -- Pointer to destination bitonal udp.
**	constant    -- Constant operand.
**      operator    -- Comparison operation to be performed.
**	result_flag -- Indicates the nature of the results.
**
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   none
**  SIGNAL CODES:     none
**  SIDE EFFECTS:     none
**
************************************************************************/
long _IpsConstantCompareW(src_udp, dst_udp, constant, operator, result_flag)
struct UDP *src_udp;      /* source word data type data plane     */
struct UDP *dst_udp;      /* destintion data plane                */
float constant;           /* constant to be used as operand       */
unsigned long operator;	  /* comparison operation to be performed */
unsigned long *result_flag;/* indicates the nature of the results  */
{
unsigned long      size;                 /* buffer size                    */
unsigned short int *src_plane_data_base; /* pointer to source data base    */
unsigned char      *dst_ptr;             /* used for dst image             */
unsigned long      ix,iy;                /* loop counters                  */
unsigned long	   dst_stride;		 /* dst scanline stride            */
unsigned long      src_stride;           /* src scanline stride            */
unsigned long      src_pad;              /* src padding at end of line     */
unsigned long      set = 0;		 /* counter for result_flag info   */

src_plane_data_base = (unsigned short int *)src_udp->UdpA_Base +
                    ((src_udp->UdpL_Pos +
                    (src_udp->UdpL_ScnStride * src_udp->UdpL_Y1) +
                    (src_udp->UdpL_X1 * src_udp->UdpL_PxlStride) )>>4);

dst_ptr = (unsigned char *)dst_udp->UdpA_Base +
                   ((dst_udp->UdpL_Pos +
                    (dst_udp->UdpL_ScnStride * dst_udp->UdpL_Y1) +
                    (dst_udp->UdpL_X1))>>3);
 
dst_stride = (dst_udp->UdpL_ScnStride >> 3);
src_stride = (src_udp->UdpL_ScnStride >> 4);
src_pad = src_stride - src_udp->UdpL_PxlPerScn;

switch (operator)
    {
    case IpsK_EQ:
        {
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                if ((*src_plane_data_base++) == (constant))
                    {
                    PUT_BIT_VALUE_(dst_ptr,ix,TRUE);
		    set++;
                    }
                }/* end ix loop */
            src_plane_data_base  += src_pad;
            dst_ptr += dst_stride;
            } /*end iy loop */
        break;
        }/*end EQ */
    case IpsK_NE:
        {
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                if ((*src_plane_data_base++) != (constant))
                    {
                    PUT_BIT_VALUE_(dst_ptr,ix,TRUE);
		    set++;
                    }
                }/* end ix loop */
            src_plane_data_base  += src_pad;
            dst_ptr += dst_stride;
            } /*end iy loop */
        break;
        }/*end NE */ 
    case IpsK_LT:
        {
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                if ((*src_plane_data_base++) < (constant))
                    {
                    PUT_BIT_VALUE_(dst_ptr,ix,TRUE);
		    set++;
                    }
                }/* end ix loop */
            src_plane_data_base  += src_pad;
            dst_ptr += dst_stride;
            } /*end iy loop */
        break;
        }/*end LT */ 
    case IpsK_LE:
        {
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                if ((*src_plane_data_base++) <= (constant))
                    {
                    PUT_BIT_VALUE_(dst_ptr,ix,TRUE);
		    set++;
                    }
                }/* end ix loop */
            src_plane_data_base  += src_pad;
            dst_ptr += dst_stride;
            } /*end iy loop */
        break;
        }/*end LE */ 
    case IpsK_GT:
        {
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                if ((*src_plane_data_base++) > (constant))
                    {
                    PUT_BIT_VALUE_(dst_ptr,ix,TRUE);
		    set++;
                    }
                }/* end ix loop */
            src_plane_data_base  += src_pad;
            dst_ptr += dst_stride;
            } /*end iy loop */
        break;
        }/*end GT */ 
    case IpsK_GE:
        {
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                if ((*src_plane_data_base++) >= (constant))
                    {
                    PUT_BIT_VALUE_(dst_ptr,ix,TRUE);
		    set++;
                    }
                }/* end ix loop */
            src_plane_data_base  += src_pad;
            dst_ptr += dst_stride;
            } /*end iy loop */
        break;
        }/*end GE */ 
    default:
        return (IpsX_UNSOPTION);
        break;
    }
if (set == 0)
    *result_flag = IpsK_SUCCESS_CLEAR;
else
    {
    if (set == (src_udp->UdpL_PxlPerScn * src_udp->UdpL_ScnCnt))
        *result_flag = IpsK_SUCCESS_SET;
    else
        *result_flag = IpsK_SUCCESS_SOME;
    }
return (IpsX_SUCCESS);
}

/*****************************************************************************
**  _IpsConstantCompareFlt- 
**
**  FUNCTIONAL DESCRIPTION:
**  
**      Performs comparison operations on corresponding values from the
**      floating point data type source image and the constant to produce 
**      results for the bitmap destination image on a point by point basis.
**  
**  FORMAL PARAMETERS:
**
**
**      src_udp     -- Pointer to float data type source udp.
**      dst_udp     -- Pointer to destination bitonal udp.
**	constant    -- Constant operand.
**      operator    -- Comparison operation to be performed.
**	result_flag -- Indicates the nature of the results.
**
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   none
**  SIGNAL CODES:     none
**  SIDE EFFECTS:     none
**
************************************************************************/
long _IpsConstantCompareFlt(src_udp, dst_udp, constant, operator, result_flag)
struct UDP *src_udp;       /* source float data type data plane     */
struct UDP *dst_udp;       /* destination data plane                */
float constant;            /* constant to be used in the comparison */
unsigned long operator;    /* comparison operation to be performed  */
unsigned long *result_flag; /* indicates the nature of the results   */
{
long            size;                   /* buffer size                    */
float           *src_plane_data_base;   /* pointer to source data base    */
unsigned char   *dst_ptr;               /* used for dst image             */
unsigned long   ix,iy;                  /* loop counters                  */
unsigned long   dst_stride;		/* dst scanline stride		  */
unsigned long   src_stride;		/* src scanline stride            */
unsigned long   src_pad;		/* src padding at end of line     */
unsigned long   set = 0;		/* counter for result_flag info   */

src_plane_data_base = (float *)src_udp->UdpA_Base +
                    ((src_udp->UdpL_Pos +
                    (src_udp->UdpL_ScnStride * src_udp->UdpL_Y1) +
                    (src_udp->UdpL_X1 * src_udp->UdpL_PxlStride) )>>5);

dst_ptr = (unsigned char *) dst_udp->UdpA_Base + 
                   ((dst_udp->UdpL_Pos +
                    (dst_udp->UdpL_ScnStride * dst_udp->UdpL_Y1) +
                    (dst_udp->UdpL_X1))>>3);

dst_stride = (dst_udp->UdpL_ScnStride >> 3);
src_stride = (src_udp->UdpL_ScnStride >> 5);
src_pad = src_stride - src_udp->UdpL_PxlPerScn;

switch (operator)
    {
    case IpsK_EQ:
        {
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                if ((*src_plane_data_base++) == (constant))
                    {
                    PUT_BIT_VALUE_(dst_ptr,ix,TRUE);
		    set++;
                    }
                }/* end ix loop */
            src_plane_data_base  += src_pad;
            dst_ptr += dst_stride;
            } /*end iy loop */
        break;
        }/*end EQ */
    case IpsK_NE:
        {
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                if ((*src_plane_data_base++) != (constant))
                    {
                    PUT_BIT_VALUE_(dst_ptr,ix,TRUE);
		    set++;
                    }
                }/* end ix loop */
            src_plane_data_base  += src_pad;
            dst_ptr += dst_stride;
            } /*end iy loop */
        break;
        }/*end NE */ 
    case IpsK_LT:
        {
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                if ((*src_plane_data_base++) < (constant))
                    {
                    PUT_BIT_VALUE_(dst_ptr,ix,TRUE);
		    set++;
                    }
                }/* end ix loop */
            src_plane_data_base  += src_pad;
            dst_ptr += dst_stride;
            } /*end iy loop */
        break;
        }/*end LT */ 
    case IpsK_LE:
        {
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                if ((*src_plane_data_base++) <= (constant))
                    {
                    PUT_BIT_VALUE_(dst_ptr,ix,TRUE);
		    set++;
                    }
                }/* end ix loop */
            src_plane_data_base  += src_pad;
            dst_ptr += dst_stride;
            } /*end iy loop */
        break;
        }/*end LE */ 
    case IpsK_GT:
        {
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                if ((*src_plane_data_base++) > (constant))
                    {
                    PUT_BIT_VALUE_(dst_ptr,ix,TRUE);
		    set++; 
                    }
                }/* end ix loop */
            src_plane_data_base  += src_pad;
            dst_ptr += dst_stride;
            } /*end iy loop */
        break;
        }/*end GT */ 
    case IpsK_GE:
        {
        for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
            {
            for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
                {
                if ((*src_plane_data_base++) >= (constant))
                    {
                    PUT_BIT_VALUE_(dst_ptr,ix,TRUE);
		    set++;
                    }
                }/* end ix loop */
            src_plane_data_base  += src_pad;
            dst_ptr += dst_stride;
            } /*end iy loop */
        break;
        }/*end GE */ 
    default:
        return (IpsX_UNSOPTION);
        break;
    }

if (set == 0)
    *result_flag = IpsK_SUCCESS_CLEAR;
else
    {
    if (set == (src_udp->UdpL_PxlPerScn * src_udp->UdpL_ScnCnt))
        *result_flag = IpsK_SUCCESS_SET;
    else
        *result_flag = IpsK_SUCCESS_SOME;
    }
return (IpsX_SUCCESS);
}
