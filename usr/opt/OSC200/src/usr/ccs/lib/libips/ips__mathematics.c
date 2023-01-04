/*  DEC/CMS REPLACEMENT HISTORY, Element IPS__MATHEMATICS.C */
/*  *6    12-SEP-1990 18:36:06 DINTINO "Add cpp support" */
/*  *5    20-AUG-1990 16:43:04 PICCOLO "code review changes" */
/*  *4    14-AUG-1990 14:02:44 PICCOLO "fixed check for in place - chk src not dst" */
/*  *3     9-AUG-1990 15:17:57 PICCOLO "pos change" */
/*  *2    27-JUL-1990 16:35:58 PICCOLO "2nd generation math function for testing" */
/*  *1     6-JUL-1990 14:55:36 PICCOLO "initial version - safe keeping" */
/*  DEC/CMS REPLACEMENT HISTORY, Element IPS__MATHEMATICS.C */
/******************************************************************************
**
**  Copyright (c) Digital Equipment Corporation, 1990-1991 All Rights Reserved. 
**  Unpublished rights reserved under the copyright laws of the United States.
**  The software contained on this media is proprietary to and embodies the 
**  confidential technology of Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and media is authorized only 
**  pursuant to a valid written license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the U.S. 
**  Government is subject to restrictions as set forth in Subparagraph 
**  (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
**
*****************************************************************************/

/*****************************************************************************
**
**  FACILITY:
**
**      Image Processing Services
**
**  ABSTRACT:
**
**      _IpsMathematics performs the specified math function on each pixel
**	within the plane
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
**      June 12, 1990
**
************************************************************************/

/*
**  Table of contents
*/
#ifdef NODAS_PROTO
long  _IpsMathematics();
long  _IpsMathByte();
long  _IpsMathByteCpp();
long  _IpsMathWord();
long  _IpsMathWordCpp();
long  _IpsMathLong();
long  _IpsMathLongCpp();
long  _IpsMathFloat();
long  _IpsMathFloatCpp();
#endif

/*
** Include files
*/
#include <IpsDef.h>			    /* IPS definitions		    */
#include <IpsStatusCodes.h>		    /* IPS Status Codes		    */
#include <IpsMemoryTable.h>		    /* IPS Memory Vector Table      */
#include <IpsMacros.h>			    /* IPS Macros		    */
#ifndef NODAS_PROTO
#include <ipsprot.h>			    /* IPS Prototypes		    */
#endif
#include <math.h>			    /* Math Runtime routines        */
#include <errno.h>			    /* RTL Errors		    */

/*
**  Equated Symbols
*/
# define NORMAL	    1			    /* Normal Math Function (1 arg) */
# define POW	    2			    /* Power Math Function  (2 args)*/
# define SQUARE	    3			    /* Square Function		    */
# define NEG	    4			    /* Negate Function		    */
# define RECIPROCAL 5			    /* Reciprocal Function	    */
# define LOG_BASE2  6			    /* LOG10 Math Function / log2   */
# define CUBE	    7			    /* Cube Function		    */

/*
**  External references
*/
#ifdef NODAS_PROTO
long _IpsVerifyNotInPlace();		    /* from ips__udp_utils	    */
#endif

/******************************************************************************
**
**  FUNCTIONAL DESCRIPTION:
**
**	This function applies the specified math function on each pixel in
**	the region specified by the UDP coordinate fields.
**
**  FORMAL PARAMETERS:
**
**	src_udp		    source udp
**	dst_udp		    destination udp
**	cpp		    control processing plane udp
**	math_function	    mathematical function to be applied
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

long _IpsMathematics (src_udp, dst_udp, cpp, math_function)
struct UDP	*src_udp;		    /* source udp		    */
struct UDP	*dst_udp;		    /* dst udp			    */
struct UDP	*cpp;			    /* control processing plane     */
unsigned long	math_function;		    /* specified math function	    */
    {
    unsigned long 	mem_alloc = 0;
    unsigned long	size;
    unsigned long	func_type;
    double		(*math_func_ptr)();
    double		second_arg;
    long		status;
    
    if  (src_udp->UdpB_Class != UdpK_ClassA)
	return (IpsX_UNSOPTION);
 
    if ((src_udp->UdpB_DType == UdpK_DTypeVU) ||
        (src_udp->UdpB_DType == UdpK_DTypeV))
	    return (IpsX_INVDTYPE);

    if (cpp != 0)
        VALIDATE_CPP_(cpp, src_udp);

    /* fill in the destination UDP of type FLOAT (no retain) */

    dst_udp->UdpB_Class = src_udp->UdpB_Class;
    dst_udp->UdpB_DType = UdpK_DTypeF;
    dst_udp->UdpW_PixelLength = 32;
    dst_udp->UdpL_PxlStride = 32;
    dst_udp->UdpL_ScnCnt = src_udp->UdpL_ScnCnt;
    dst_udp->UdpL_PxlPerScn = src_udp->UdpL_PxlPerScn;
    dst_udp->UdpL_ScnStride = src_udp->UdpL_PxlPerScn * 32;
    dst_udp->UdpL_X1 = 0;
    dst_udp->UdpL_Y1 = 0;
    dst_udp->UdpL_X2 = src_udp->UdpL_PxlPerScn - 1;
    dst_udp->UdpL_Y2 = src_udp->UdpL_ScnCnt - 1;
    dst_udp->UdpL_CompIdx = 0;
	/* levels is not used for floating point */
    size = src_udp->UdpL_PxlPerScn * src_udp->UdpL_ScnCnt * 32;
    if (dst_udp->UdpA_Base == 0)
        {
        dst_udp->UdpA_Base = (unsigned char *)
	    (*IpsA_MemoryTable[IpsK_AllocateDataPlane])
		(((size + 7) / 8),0,0);
	if (!dst_udp->UdpA_Base) return (IpsX_INSVIRMEM);
	dst_udp->UdpL_ArSize = size;
	dst_udp->UdpL_Pos = 0;
	mem_alloc = 1;
	}
    else
        {
	if (src_udp->UdpB_DType != UdpK_DTypeF)
	    {
	    /* No in place allowed */
	    status = _IpsVerifyNotInPlace (src_udp, dst_udp);
	    if (status != IpsX_SUCCESS) return (status);
	    }
	if ((dst_udp->UdpL_ArSize - dst_udp->UdpL_Pos) < size)
		return (IpsX_INSVIRMEM);
        }

    /* 
    ** classified specified function as general type 
    */
    switch (math_function)
	{
	case IpsK_Sin:
	    func_type = NORMAL;
	    math_func_ptr = sin;
	break;

	case IpsK_Cos:
	    func_type = NORMAL;
	    math_func_ptr = cos;
	break;

	case IpsK_Tan:
	    func_type = NORMAL;
	    math_func_ptr = tan;
	break;

	case IpsK_ArcSin:
	    func_type = NORMAL;
	    math_func_ptr = asin;
	break;

	case IpsK_ArcCos:
	    func_type = NORMAL;
	    math_func_ptr = acos;
	break;

	case IpsK_ArcTan:
	    func_type = NORMAL;
	    math_func_ptr = atan;
	break;

	case IpsK_Sinh:
	    func_type = NORMAL;
	    math_func_ptr = sinh;
	break;

	case IpsK_Cosh:
	    func_type = NORMAL;
	    math_func_ptr = cosh;
	break;

	case IpsK_Tanh:
	    func_type = NORMAL;
	    math_func_ptr = tanh;
	break;

	case IpsK_Exp:
	    func_type = NORMAL;
	    math_func_ptr = exp;
	break;

	case IpsK_NaturalLog:
	    func_type = NORMAL;
	    math_func_ptr = log;
	break;

	case IpsK_Log2:
	    func_type = LOG_BASE2;
	    math_func_ptr = log10;
	    second_arg = log10 ((double)2.0);
	break;

	case IpsK_Log10:
	    func_type = NORMAL;
	    math_func_ptr = log10;
	break;

	case IpsK_AbsVal:
	    math_func_ptr = fabs;
	    func_type = NORMAL;
	break;

	case IpsK_Square:
	    func_type = SQUARE;
	break;

	case IpsK_Sqrt:
	    func_type = NORMAL;
	    math_func_ptr = sqrt;
	break;

	case IpsK_Cube:
	    func_type = CUBE;
	break;

	case IpsK_CubeRoot:
	    func_type = POW;
	    second_arg = 1.0 / 3.0;
	    math_func_ptr = pow;
	break;

	case IpsK_Negative:
	    func_type = NEG;
	break;

	case IpsK_Reciprocal:
	    func_type = RECIPROCAL;
	break;

	default:
	    if (mem_alloc == 1)
		(*IpsA_MemoryTable[IpsK_FreeDataPlane])(dst_udp->UdpA_Base);
	    return (IpsX_INVDARG);
	break;
	}

    switch (src_udp->UdpB_DType)
        {
        case UdpK_DTypeBU:
	    if (cpp == 0)
		status = _IpsMathByte
		    (src_udp, dst_udp, func_type, math_func_ptr, second_arg);
	    else
		status = _IpsMathByteCpp
		    (src_udp, dst_udp, cpp, func_type, math_func_ptr, 
			second_arg);
	break;

	case UdpK_DTypeWU:
	    if (cpp == 0)
		status = _IpsMathWord
		    (src_udp, dst_udp, func_type, math_func_ptr, second_arg);
	    else
		status = _IpsMathWordCpp
		    (src_udp, dst_udp, cpp, func_type, math_func_ptr, 
			second_arg);
	break;

	case UdpK_DTypeLU:
	    if (cpp == 0)
		status = _IpsMathLong
		    (src_udp, dst_udp, func_type, math_func_ptr, second_arg);
	    else
		status = _IpsMathLongCpp
		    (src_udp, dst_udp, cpp, func_type, math_func_ptr, 
			second_arg);
	break;

	case UdpK_DTypeF:
	    if (cpp == 0)
		status = _IpsMathFloat
		    (src_udp, dst_udp, func_type, math_func_ptr, second_arg);
	    else
		status = _IpsMathFloatCpp
		    (src_udp, dst_udp, cpp, func_type, math_func_ptr, 
			second_arg);
	break;

	default:
	    status = IpsX_INVDTYPE;
        break;

    };  /* end switch on DType */

    if ((status != IpsX_SUCCESS) && (mem_alloc == 1))
        (*IpsA_MemoryTable[IpsK_FreeDataPlane]) (dst_udp->UdpA_Base);

    return (status);
    }

/******************************************************************************
**
**	_IpsMathByte applies the specified math function on each byte pixel in
**	the region specified by the UDP coordinate fields.
**	The status will indicate whether an error was encountered (e.g.
**	log (0)).
**
******************************************************************************/
long _IpsMathByte (src_udp, dst_udp, function_type, math_func_ptr, second_arg)
struct UDP	*src_udp;		    /* source udp		    */
struct UDP	*dst_udp;		    /* dst udp			    */
unsigned long	function_type;		    /* specified math function type */
double		(*math_func_ptr)();	    /* specified math function addr */
double		second_arg;		    /* use as 2nd arg for POW, LOG2 */
{
    float		*dst_ptr;
    unsigned long	src_line_pad;
    unsigned char	*src_byte_ptr;	
    unsigned long	ix,iy;		    /* loop entities		    */
 
    src_byte_ptr = (unsigned char *) src_udp->UdpA_Base + 
	(src_udp->UdpL_Pos>>3) + (src_udp->UdpL_ScnStride>>3) *
	    src_udp->UdpL_Y1 + src_udp->UdpL_X1;
    src_line_pad = (src_udp->UdpL_ScnStride>>3) - src_udp->UdpL_PxlPerScn;
    dst_ptr = (float *) dst_udp->UdpA_Base + (dst_udp->UdpL_Pos>>5);

    errno = 0;
    switch (function_type)
	{
	case NORMAL:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    *dst_ptr++ = (float)(*math_func_ptr)
			((double)(*src_byte_ptr++));
		src_byte_ptr += src_line_pad;
		}
	break;

	case POW:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    *dst_ptr++ = (float)(*math_func_ptr)
			((double)(*src_byte_ptr++), (double)(second_arg));
		src_byte_ptr += src_line_pad;
		}
	break;

	case SQUARE:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    *dst_ptr++ = (float)(*src_byte_ptr * *src_byte_ptr++);
		src_byte_ptr += src_line_pad;
		}
	break;

	case NEG:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    *dst_ptr++ = (float) (-(float)*src_byte_ptr++);
		src_byte_ptr += src_line_pad;
		}
	break;

	case RECIPROCAL:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    *dst_ptr++ = (float) (1.0 / *src_byte_ptr++);
		src_byte_ptr += src_line_pad;
		}
	break;

	case LOG_BASE2:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    *dst_ptr++ = (float)((*math_func_ptr)
			((double)*src_byte_ptr++) / second_arg);
		src_byte_ptr += src_line_pad;
		}
	break;

	case CUBE:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    *dst_ptr++ = (float)((long)*src_byte_ptr * 
			(long)*src_byte_ptr * (long)*src_byte_ptr++);
		src_byte_ptr += src_line_pad;
		}
	break;

	default:
	break;
	}
    if (errno != 0) 
	return (IpsX_INVDMTHARG);
    return (IpsX_SUCCESS);
}

/******************************************************************************
**
**	_IpsMathByteCpp applies the specified math function on each byte pixel 
**	in the region specified by the UDP coordinate fields and the 
**	corresponding CPP. The status will indicate whether an error was 
**	encountered (e.g. log (0)).
**
******************************************************************************/
long _IpsMathByteCpp (src_udp, dst_udp, cpp, function_type, math_func_ptr,
    second_arg)
struct UDP	*src_udp;		    /* source udp		    */
struct UDP	*dst_udp;		    /* dst udp			    */
struct UDP	*cpp;			    /* cpp			    */
unsigned long	function_type;		    /* specified math function	    */
double		(*math_func_ptr)();	    /* specified math function addr */
double		second_arg;		    /* use as 2nd arg for POW, LOG2 */
{

    float		*dst_ptr;
    unsigned long	src_line_pad;
    unsigned char	*src_byte_ptr;	
    unsigned long       cpp_stride;         /* control proc plane stride   */
    unsigned char       *cpp_ptr;           /* control proc plane base     */
    unsigned long	ix,iy;		    /* loop entities		    */
    unsigned long       bit; 

    src_byte_ptr = (unsigned char *) src_udp->UdpA_Base + 
	(src_udp->UdpL_Pos>>3) + (src_udp->UdpL_ScnStride>>3) *
	    src_udp->UdpL_Y1 + src_udp->UdpL_X1;
    src_line_pad = (src_udp->UdpL_ScnStride>>3) - src_udp->UdpL_PxlPerScn;
    dst_ptr = (float *) dst_udp->UdpA_Base + (dst_udp->UdpL_Pos>>5);
    cpp_ptr = (unsigned char *) cpp->UdpA_Base
                + ((((cpp->UdpL_Pos + (cpp->UdpL_ScnStride * cpp->UdpL_Y1) +
                   cpp->UdpL_X1)) + 7)/8);
 
    cpp_stride = (cpp->UdpL_ScnStride >> 3);
    errno = 0;
    switch (function_type)
	{
	case NORMAL:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    {
                    if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
		        *dst_ptr++ = (float)(*math_func_ptr)
			    ((double)(*src_byte_ptr++));
		    else
			*dst_ptr++ = *src_byte_ptr++;
                    }/* end ix loop */
		cpp_ptr += cpp_stride;
		src_byte_ptr += src_line_pad;
		}
	break;

	case POW:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    {
                    if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
		        *dst_ptr++ = (float)(*math_func_ptr)
			    ((double)(*src_byte_ptr++), (double)(second_arg));
		    else
			*dst_ptr++ = *src_byte_ptr++;
                    }/* end ix loop */
		cpp_ptr += cpp_stride;
		src_byte_ptr += src_line_pad;
		}
	break;

	case SQUARE:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    {
                    if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
		        *dst_ptr++ = (float)(*src_byte_ptr * *src_byte_ptr++);
		    else
			*dst_ptr++ = *src_byte_ptr++;
                    }/* end ix loop */
		cpp_ptr += cpp_stride;
		src_byte_ptr += src_line_pad;
		}
	break;

	case NEG:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    {
                    if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
		        *dst_ptr++ = (float) (-(float)*src_byte_ptr++);
		    else
			*dst_ptr++ = *src_byte_ptr++;
                    }/* end ix loop */
		cpp_ptr += cpp_stride;
		src_byte_ptr += src_line_pad;
		}
	break;

	case RECIPROCAL:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    {
                    if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
		        *dst_ptr++ = (float) (1.0 / *src_byte_ptr++);
		    else
			*dst_ptr++ = *src_byte_ptr++;
                    }/* end ix loop */
		cpp_ptr += cpp_stride;
		src_byte_ptr += src_line_pad;
		}
	break;

	case LOG_BASE2:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    {
                    if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
		        *dst_ptr++ = (float)((*math_func_ptr)
			    ((double)*src_byte_ptr++) / second_arg);
		    else
			*dst_ptr++ = *src_byte_ptr++;
                    }/* end ix loop */
		cpp_ptr += cpp_stride;
		src_byte_ptr += src_line_pad;
		}
	break;

	case CUBE:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    {
                    if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
		        *dst_ptr++ = (float)((long)*src_byte_ptr * 
			    (long)*src_byte_ptr * (long)*src_byte_ptr++);
		    else
			*dst_ptr++ = *src_byte_ptr++;
                    }/* end ix loop */
		cpp_ptr += cpp_stride;
		src_byte_ptr += src_line_pad;
		}
	break;

	default:
	break;
	}
    if (errno != 0) 
	return (IpsX_INVDMTHARG);
    return (IpsX_SUCCESS);
}

/******************************************************************************
**
**	_IpsMathWord applies the specified math function on each word pixel in
**	the region specified by the UDP coordinate fields.
**	The status will indicate whether an error was encountered (e.g.
**	log (0)).
**
******************************************************************************/
long _IpsMathWord (src_udp, dst_udp, function_type, math_func_ptr,
    second_arg)
struct UDP	*src_udp;		    /* source udp		    */
struct UDP	*dst_udp;		    /* dst udp			    */
unsigned long	function_type;		    /* specified math function	    */
double		(*math_func_ptr)();	    /* specified math function addr */
double		second_arg;		    /* use as 2nd arg for POW, LOG2 */
{
    float		*dst_ptr;
    unsigned long	src_line_pad;
    unsigned short 	*src_word_ptr;	
    unsigned long	ix,iy;		    /* loop entities		    */
 
    src_word_ptr = (unsigned short *)src_udp->UdpA_Base + 
	(src_udp->UdpL_Pos>>4) + (src_udp->UdpL_ScnStride>>4) *
	    src_udp->UdpL_Y1 + src_udp->UdpL_X1;
    src_line_pad = (src_udp->UdpL_ScnStride>>4) - src_udp->UdpL_PxlPerScn;
    dst_ptr = (float *) dst_udp->UdpA_Base + (dst_udp->UdpL_Pos>>5);

    errno = 0;
    switch (function_type)
	{
	case NORMAL:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    *dst_ptr++ = (float)(*math_func_ptr)
			((double)(*src_word_ptr++));
		src_word_ptr += src_line_pad;
		}
	break;

	case POW:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    *dst_ptr++ = (float)(*math_func_ptr)
			((double)(*src_word_ptr++), (double)(second_arg));
		src_word_ptr += src_line_pad;
		}
	break;

	case SQUARE:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    *dst_ptr++ = (float)((long)*src_word_ptr * 
			(long)*src_word_ptr++);
		src_word_ptr += src_line_pad;
		}
	break;

	case NEG:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    *dst_ptr++ = (float) (-(float)*src_word_ptr++);
		src_word_ptr += src_line_pad;
		}
	break;

	case RECIPROCAL:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    *dst_ptr++ = (float) (1.0 / *src_word_ptr++);
		src_word_ptr += src_line_pad;
		}
	break;

	case LOG_BASE2:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    *dst_ptr++ = (float)((*math_func_ptr)
			((double)*src_word_ptr++) / second_arg);
		src_word_ptr += src_line_pad;
		}
	break;

	case CUBE:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    *dst_ptr++ = (float)((float)*src_word_ptr * 
			(float)*src_word_ptr * (float)*src_word_ptr++);
		src_word_ptr += src_line_pad;
		}
	break;

	default:
	break;
	}

    if (errno != 0) 
	return (IpsX_INVDMTHARG);
    return (IpsX_SUCCESS);
}

/******************************************************************************
**
**	_IpsMathWordCpp applies the specified math func on each word pixel in
**	the region specified by the UDP coordinate fields and the corresponding
**	CPP. The status will indicate whether an error was encountered 
**	(e.g. log (0)).
**
******************************************************************************/
long _IpsMathWordCpp (src_udp, dst_udp, cpp, function_type, math_func_ptr,
    second_arg)
struct UDP	*src_udp;		    /* source udp		    */
struct UDP	*dst_udp;		    /* dst udp			    */
struct UDP	*cpp;			    /* cpp			    */
unsigned long	function_type;		    /* specified math function	    */
double		(*math_func_ptr)();	    /* specified math function addr */
double		second_arg;		    /* use as 2nd arg for POW, LOG2 */
{
    float		*dst_ptr;
    unsigned long	src_line_pad;
    unsigned short 	*src_word_ptr;	
    unsigned long	ix,iy;		    /* loop entities		   */
    unsigned char       *cpp_ptr;           /* control proc plane base     */
    unsigned long       cpp_stride;         /* control proc plane stride   */
 
    src_word_ptr = (unsigned short *)src_udp->UdpA_Base + 
	(src_udp->UdpL_Pos>>4) + (src_udp->UdpL_ScnStride>>4) *
	    src_udp->UdpL_Y1 + src_udp->UdpL_X1;
    src_line_pad = (src_udp->UdpL_ScnStride>>4) - src_udp->UdpL_PxlPerScn;
    dst_ptr = (float *) dst_udp->UdpA_Base + (dst_udp->UdpL_Pos>>5);
    cpp_ptr = (unsigned char *) cpp->UdpA_Base
                + (cpp->UdpL_Pos +
                  (cpp->UdpL_ScnStride * cpp->UdpL_Y1) +
                   cpp->UdpL_X1);

    cpp_stride = (cpp->UdpL_ScnStride >> 3); 
    errno = 0;
    switch (function_type)
	{
	case NORMAL:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    {
                    if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
		        *dst_ptr++ = (float)(*math_func_ptr)
			    ((double)(*src_word_ptr++));
	            else
			*dst_ptr++ = *src_word_ptr++;
                    }/* end ix loop */
		cpp_ptr += cpp_stride;
		src_word_ptr += src_line_pad;
		}
	break;

	case POW:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    {
                    if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
		        *dst_ptr++ = (float)(*math_func_ptr)
			    ((double)(*src_word_ptr++), (double)(second_arg));
	            else
			*dst_ptr++ = *src_word_ptr++;
                    }/* end ix loop */
		cpp_ptr += cpp_stride;
		src_word_ptr += src_line_pad;
		}
	break;

	case SQUARE:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    {
                    if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
		        *dst_ptr++ = (float)((long)*src_word_ptr * 
			    (long)*src_word_ptr++);
	            else
			*dst_ptr++ = *src_word_ptr++;
                    }/* end ix loop */
		cpp_ptr += cpp_stride;
		src_word_ptr += src_line_pad;
		}
	break;

	case NEG:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    {
                    if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
		        *dst_ptr++ = (float) (-(float)*src_word_ptr++);
	            else
			*dst_ptr++ = *src_word_ptr++;
                    }/* end ix loop */
		cpp_ptr += cpp_stride;
		src_word_ptr += src_line_pad;
		}
	break;

	case RECIPROCAL:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    {
                    if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
		        *dst_ptr++ = (float) (1.0 / *src_word_ptr++);
	            else
			*dst_ptr++ = *src_word_ptr++;
                    }/* end ix loop */
		cpp_ptr += cpp_stride;
		src_word_ptr += src_line_pad;
		}
	break;

	case LOG_BASE2:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    {
                    if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
		        *dst_ptr++ = (float)((*math_func_ptr)
			    ((double)*src_word_ptr++) / second_arg);
	            else
			*dst_ptr++ = *src_word_ptr++;
                    }/* end ix loop */
		cpp_ptr += cpp_stride;
		src_word_ptr += src_line_pad;
		}
	break;

	case CUBE:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    {
                    if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
		        *dst_ptr++ = (float)((float)*src_word_ptr * 
			    (float)*src_word_ptr * (float)*src_word_ptr++);
	            else
			*dst_ptr++ = *src_word_ptr++;
                    }/* end ix loop */
		cpp_ptr += cpp_stride;
		src_word_ptr += src_line_pad;
		}
	break;

	default:
	break;
	}

    if (errno != 0) 
	return (IpsX_INVDMTHARG);
    return (IpsX_SUCCESS);
}

/******************************************************************************
**
**	_IpsMathLong applies the specified math function on each long pixel in
**	the region specified by the UDP coordinate fields.
**	The status will indicate whether an error was encountered (e.g.
**	log (0)).
**
******************************************************************************/
long _IpsMathLong (src_udp, dst_udp, function_type, math_func_ptr,
    second_arg)
struct UDP	*src_udp;		    /* source udp		    */
struct UDP	*dst_udp;		    /* dst udp			    */
unsigned long	function_type;		    /* specified math function	    */
double		(*math_func_ptr)();	    /* specified math function addr */
double		second_arg;		    /* use as 2nd arg for POW, LOG2 */
{
    float		*dst_ptr;
    unsigned long	src_line_pad;
    unsigned long	*src_long_ptr;	
    unsigned long	ix,iy;		    /* loop entities		    */
 
    src_long_ptr = (unsigned long *) src_udp->UdpA_Base + 
	(src_udp->UdpL_Pos>>5) + (src_udp->UdpL_ScnStride>>5) *
	    src_udp->UdpL_Y1 + src_udp->UdpL_X1;
    src_line_pad = (src_udp->UdpL_ScnStride>>5) - src_udp->UdpL_PxlPerScn;
    dst_ptr = (float *) dst_udp->UdpA_Base + (dst_udp->UdpL_Pos>>5);

    errno = 0;
    switch (function_type)
	{
	case NORMAL:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    *dst_ptr++ = (float)(*math_func_ptr)
			((double)(*src_long_ptr++));
		src_long_ptr += src_line_pad;
		}
	break;

	case POW:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    *dst_ptr++ = (float)(*math_func_ptr)
			((double)(*src_long_ptr++), (double)(second_arg));
		src_long_ptr += src_line_pad;
		}
	break;

	case SQUARE:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    *dst_ptr++ = (float)((float)*src_long_ptr * 
			(float)*src_long_ptr++);
		src_long_ptr += src_line_pad;
		}
	break;

	case NEG:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    *dst_ptr++ = (float) (-(float)*src_long_ptr++);
		src_long_ptr += src_line_pad;
		}
	break;

	case RECIPROCAL:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    *dst_ptr++ = (float) (1.0 / *src_long_ptr++);
		src_long_ptr += src_line_pad;
		}
	break;

	case LOG_BASE2:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    *dst_ptr++ = (float)((*math_func_ptr)
			((double)*src_long_ptr++) / second_arg);
		src_long_ptr += src_line_pad;
		}
	break;

	case CUBE:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    *dst_ptr++ = (float)((float)*src_long_ptr * 
			(float)*src_long_ptr * (float)*src_long_ptr++);
		src_long_ptr += src_line_pad;
		}
	break;

	default:
	break;
	}

    if (errno != 0) 
	return (IpsX_INVDMTHARG);
    return (IpsX_SUCCESS);
}

/******************************************************************************
**
**	_IpsMathLongCpp applies the specified math func on each long pixel in
**	the region specified by the UDP coordinate fields and the corresponding
**	CPP. The status will indicate whether an error was encountered 
**	(e.g. log (0)).
**
******************************************************************************/
long _IpsMathLongCpp (src_udp, dst_udp, cpp, function_type, math_func_ptr,
    second_arg)
struct UDP	*src_udp;		    /* source udp		    */
struct UDP	*dst_udp;		    /* dst udp			    */
struct UDP	*cpp;			    /* cpp			    */
unsigned long	function_type;		    /* specified math function	    */
double		(*math_func_ptr)();	    /* specified math function addr */
double		second_arg;		    /* use as 2nd arg for POW, LOG2 */
{
    float		*dst_ptr;
    unsigned long	src_line_pad;
    unsigned long	*src_long_ptr;	
    unsigned long	ix,iy;		    /* loop entities		    */
    unsigned long       cpp_stride;         /* control proc plane stride   */
    unsigned char       *cpp_ptr;           /* control proc plane base     */
 
    src_long_ptr = (unsigned long *) src_udp->UdpA_Base + 
	(src_udp->UdpL_Pos>>5) + (src_udp->UdpL_ScnStride>>5) *
	    src_udp->UdpL_Y1 + src_udp->UdpL_X1;
    src_line_pad = (src_udp->UdpL_ScnStride>>5) - src_udp->UdpL_PxlPerScn;
    dst_ptr = (float *) dst_udp->UdpA_Base + (dst_udp->UdpL_Pos>>5);

    cpp_ptr = (unsigned char *) cpp->UdpA_Base
                + (cpp->UdpL_Pos + (cpp->UdpL_ScnStride * cpp->UdpL_Y1) +
                   cpp->UdpL_X1);
    cpp_stride = (cpp->UdpL_ScnStride >> 3);
    errno = 0;

    switch (function_type)
	{
	case NORMAL:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    {
                    if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
		        *dst_ptr++ = (float)(*math_func_ptr)
			    ((double)(*src_long_ptr++));
		    else
                        *dst_ptr++ = *src_long_ptr++;
                    }/* end ix loop */
                cpp_ptr += cpp_stride;
		src_long_ptr += src_line_pad;
		}
	break;

	case POW:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    {
                    if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
		        *dst_ptr++ = (float)(*math_func_ptr)
			    ((double)(*src_long_ptr++), (double)(second_arg));
		    else
                        *dst_ptr++ = *src_long_ptr++;
                    }/* end ix loop */
                cpp_ptr += cpp_stride;			   
		src_long_ptr += src_line_pad;
		}
	break;

	case SQUARE:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    {
                    if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
		        *dst_ptr++ = (float)((float)*src_long_ptr * 
			    (float)*src_long_ptr++);
		    else
                        *dst_ptr++ = *src_long_ptr++;
                    }/* end ix loop */
                cpp_ptr += cpp_stride;
		src_long_ptr += src_line_pad;
		}
	break;

	case NEG:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    {
                    if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
		        *dst_ptr++ = (float) (-(float)*src_long_ptr++);
		    else
                        *dst_ptr++ = *src_long_ptr++;
                    }/* end ix loop */
                cpp_ptr += cpp_stride;
		src_long_ptr += src_line_pad;
		}
	break;

	case RECIPROCAL:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    {
                    if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
		        *dst_ptr++ = (float) (1.0 / *src_long_ptr++);
		    else
                        *dst_ptr++ = *src_long_ptr++;
                    }/* end ix loop */
                cpp_ptr += cpp_stride;
		src_long_ptr += src_line_pad;
		}
	break;

	case LOG_BASE2:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    {
                    if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
		        *dst_ptr++ = (float)((*math_func_ptr)
			    ((double)*src_long_ptr++) / second_arg);
		    else
                        *dst_ptr++ = *src_long_ptr++;
                    }/* end ix loop */
                cpp_ptr += cpp_stride;
		src_long_ptr += src_line_pad;
		}
	break;

	case CUBE:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    {
                    if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
		        *dst_ptr++ = (float)((float)*src_long_ptr * 
			(float)*src_long_ptr * (float)*src_long_ptr++);
		    else
                        *dst_ptr++ = *src_long_ptr++;
                    }/* end ix loop */
                cpp_ptr += cpp_stride;
		src_long_ptr += src_line_pad;
		}
	break;

	default:
	break;
	}

    if (errno != 0) 
	return (IpsX_INVDMTHARG);
    return (IpsX_SUCCESS);
}

/******************************************************************************
**
**	_IpsMathFloat applies the specified math function on each float pixel in
**	the region specified by the UDP coordinate fields.
**	The status will indicate whether an error was encountered (e.g.
**	log (0)).
**
******************************************************************************/
long _IpsMathFloat (src_udp, dst_udp, function_type, math_func_ptr, second_arg)
struct UDP	*src_udp;		    /* source udp		    */
struct UDP	*dst_udp;		    /* dst udp			    */
unsigned long	function_type;		    /* specified math function	    */
double		(*math_func_ptr)();	    /* specified math function addr */
double		second_arg;		    /* use as 2nd arg for POW, LOG2 */
{
    float		*dst_ptr;
    unsigned long	src_line_pad;
    float		*src_float_ptr;	
    unsigned long	ix,iy;		    /* loop entities		    */
 
    src_float_ptr = (float *) src_udp->UdpA_Base + 
	(src_udp->UdpL_Pos>>5) + (src_udp->UdpL_ScnStride>>5) *
	    src_udp->UdpL_Y1 + src_udp->UdpL_X1;
    src_line_pad = (src_udp->UdpL_ScnStride>>5) - src_udp->UdpL_PxlPerScn;
    dst_ptr = (float *) dst_udp->UdpA_Base + (dst_udp->UdpL_Pos>>5);

    errno = 0;
    switch (function_type)
	{
	case NORMAL:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    *dst_ptr++ = (float)(*math_func_ptr)
			((double)(*src_float_ptr++));
		src_float_ptr += src_line_pad;
		}
	break;

	case POW:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    *dst_ptr++ = (float)(*math_func_ptr)
			((double)(*src_float_ptr++), (double)(second_arg));
		src_float_ptr += src_line_pad;
		}
	break;

	case SQUARE:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    *dst_ptr++ = (double)*src_float_ptr * 
			(double)*src_float_ptr++;
		src_float_ptr += src_line_pad;
		}
	break;

	case NEG:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    *dst_ptr++ = (float) -(*src_float_ptr++);
		src_float_ptr += src_line_pad;
		}
	break;

	case RECIPROCAL:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    *dst_ptr++ = (float) ((double)1.0/(double)*src_float_ptr++);
		src_float_ptr += src_line_pad;
		}
	break;

	case LOG_BASE2:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    *dst_ptr++ = (float)((*math_func_ptr)
			((double)*src_float_ptr++) / second_arg);
		src_float_ptr += src_line_pad;
		}
	break;

	case CUBE:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    *dst_ptr++ = (double)*src_float_ptr * 
			(double)*src_float_ptr * (double)*src_float_ptr++;
		src_float_ptr += src_line_pad;
		}
	break;

	default:
	break;
	}
    if (errno != 0) 
	return (IpsX_INVDMTHARG);
    return (IpsX_SUCCESS);
}

/******************************************************************************
**
**	_IpsMathFloatCpp applies the specified math func on each float pixel in
**	the region specified by the UDP coordinate fields and the corresponding
**	CPP. The status will indicate whether an error was encountered 
**	(e.g. log (0)).
**
******************************************************************************/

long _IpsMathFloatCpp (src_udp, dst_udp, cpp, function_type, math_func_ptr,
    second_arg)
struct UDP	*src_udp;		    /* source udp		    */
struct UDP	*dst_udp;		    /* dst udp			    */
struct UDP	*cpp;			    /* cpp			    */
unsigned long	function_type;		    /* specified math function	    */
double		(*math_func_ptr)();	    /* specified math function addr */
double		second_arg;		    /* use as 2nd arg for POW, LOG2 */
{
    float		*dst_ptr;
    unsigned long	src_line_pad;
    float		*src_float_ptr;	
    unsigned long	ix,iy;		    /* loop entities		    */
    unsigned char       *cpp_ptr;           /* control proc plane base     */
    unsigned long       cpp_stride;         /* control proc plane stride   */
 
    src_float_ptr = (float *) src_udp->UdpA_Base + 
	(src_udp->UdpL_Pos>>5) + (src_udp->UdpL_ScnStride>>5) *
	    src_udp->UdpL_Y1 + src_udp->UdpL_X1;
    src_line_pad = (src_udp->UdpL_ScnStride>>5) - src_udp->UdpL_PxlPerScn;
    dst_ptr = (float *) dst_udp->UdpA_Base + (dst_udp->UdpL_Pos>>5);
    cpp_ptr = (unsigned char *) cpp->UdpA_Base
                + (cpp->UdpL_Pos +
                  (cpp->UdpL_ScnStride * cpp->UdpL_Y1) +
                   cpp->UdpL_X1);
    cpp_stride = (cpp->UdpL_ScnStride >> 3);
    errno = 0;
    switch (function_type)
	{
	case NORMAL:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
                    {
                    if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
		        *dst_ptr++ = (float)(*math_func_ptr)
			    ((double)(*src_float_ptr++));
                    else
                        *dst_ptr++ = *src_float_ptr++;
                    }/* end ix loop */
                cpp_ptr += cpp_stride;
		src_float_ptr += src_line_pad;
		}
	break;

	case POW:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    {
                    if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
		        *dst_ptr++ = (float)(*math_func_ptr)
			    ((double)(*src_float_ptr++), (double)(second_arg));
                    else
                        *dst_ptr++ = *src_float_ptr++;
                    }/* end ix loop */
                cpp_ptr += cpp_stride;
		src_float_ptr += src_line_pad;
		}
	break;

	case SQUARE:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    {
                    if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
		        *dst_ptr++ = (double)*src_float_ptr * 
			    (double)*src_float_ptr++;
                    else
                        *dst_ptr++ = *src_float_ptr++;
                    }/* end ix loop */
                cpp_ptr += cpp_stride;
		src_float_ptr += src_line_pad;
		}
	break;

	case NEG:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    {
                    if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
		        *dst_ptr++ = (float) -(*src_float_ptr++);
                    else
                        *dst_ptr++ = *src_float_ptr++;
                    }/* end ix loop */
                cpp_ptr += cpp_stride;
		src_float_ptr += src_line_pad;
		}
	break;

	case RECIPROCAL:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    {
                    if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
		        *dst_ptr++ = (float) 
			    ((double)1.0/(double)*src_float_ptr++);
                    else
                        *dst_ptr++ = *src_float_ptr++;
                    }/* end ix loop */
                cpp_ptr += cpp_stride;
		src_float_ptr += src_line_pad;
		}
	break;

	case LOG_BASE2:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    {
                    if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
		        *dst_ptr++ = (float)((*math_func_ptr)
			    ((double)*src_float_ptr++) / second_arg);
                    else
                        *dst_ptr++ = *src_float_ptr++;
                    }/* end ix loop */
                cpp_ptr += cpp_stride;
		src_float_ptr += src_line_pad;
		}
	break;

	case CUBE:
	    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
		{
		for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		    {
                    if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
		        *dst_ptr++ = (double)*src_float_ptr * 
			    (double)*src_float_ptr * (double)*src_float_ptr++;
                    else
                        *dst_ptr++ = *src_float_ptr++;
                    }/* end ix loop */
                cpp_ptr += cpp_stride;
		src_float_ptr += src_line_pad;
		}
	break;

	default:
	break;
	}
    if (errno != 0) 
	return (IpsX_INVDMTHARG);
    return (IpsX_SUCCESS);
}
