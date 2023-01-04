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
**
**  _IpsGeometric
**
**  FACILITY:
**
**	Image Processing Services (IPS)
**
**  ABSTRACT:
**
**	This module contains the IPS entry for geometric transforms
**
**  ENVIRONMENT:
**
**	VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
**	Richard Piccolo , Digital Equipment Corp.
**
**  CREATION DATE:
**
**	October 25, 1990
**
************************************************************************/

/*
**  Table of contents:
**
*/
    /* Layer 1 Entries (Data Class & Type Switch) */

#ifdef NODAS_PROTO
long	    _IpsGeometric();    
#endif

/*
**  Internal routines
*/


/*
**  Include files:
**
*/
#include <IpsMemoryTable.h>		    /* IPS Memory Mgt. Functions    */
#include <IpsDef.h>			    /* IPS Image Definitions	    */
#include <IpsStatusCodes.h>		    /* IPS Status Codes 	    */
#ifndef NODAS_PROTO
#include <ipsprot.h>			    /* IPS Prototypes		    */
#endif
#include <math.h>			    /* Math Library		    */

/*
**  External References:
*/
#ifdef NODAS_PROTO
long _IpsVerifyNotInPlace();                         /* from IPS__UDP_UTILS */
#endif


/******************************************************************************
**  _IpsGeometric
**
**	This function rotates a single-plane image by any
**	arbitrary number of degrees.It is the layer 2 routine
**	which dispatches to the most efficient layer 2a routine
**	based on the angle of rotation.
**
**  FORMAL PARAMETERS:
**  
**	src_udp	--> Pointer to source udp (initialized)
**	dst_udp --> Pointer to destination udp (uninitialized)
**      x0	--> Starting x
**	y0	--> Starting y
**	dx	--> delta x
**	dy	--> delta y
**	dx2	--> quadratic delta
**	dy2	-->    "      delta
**      flags   --> none define yet
**
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   return status
**  SIGNAL CODES:     none
**  SIDE EFFECTS:     none
**
******************************************************************************/

long _IpsGeometric (src_udp, src_x, src_y, dst_udp, dst_x_size, dst_y_size, 
		    dx, dy, d_dx, d_dy, dxr, dyr, d_dxr, d_dyr, flags)
struct UDP *src_udp; 			/* Working src UDP descriptor	    */
unsigned long	src_x, src_y;		/* starting x and y in src	    */
struct UDP *dst_udp;			/* Working dst UDP descriptor	    */
unsigned long	dst_x_size;		/* result x size		    */
unsigned long	dst_y_size;		/* result y size		    */
float		dx, dy;			/* starting row pixel delta's	    */
float		d_dx, d_dy;		/* change in pixel deltas	    */
float		dxr, dyr;		/* pixel location delta per row	    */
float		d_dxr, d_dyr;		/* change per row on d_dx & d_dy    */
unsigned long	flags;			/* edge fill (on bytes only)	    */
    {
    unsigned char	*src_ptr, *dst_ptr;
    float		*src_float_ptr, *dst_float_ptr;
    long		ul_x, ul_y;
    long		offset;
    unsigned long	sbyte_stride, dbyte_stride;
    float		cur_x, cur_y;
    float		cur_dx, cur_dy;
    unsigned long	ix, iy;
    unsigned long	mem_alloc = 0;
    float		talpha,beta;	/* source - lower left or upper right */
    float		one_minus_talpha;/* 1 minus talpha		      */
    float		one_minus_beta;	/* 1 minus beta			      */
    float		ll_pixel;       /* lower left pixel		      */
    float		lr_pixel;       /* lower right pixel		      */
    float		ul_pixel;       /* upper left pixel		      */
    float		ur_pixel;       /* upper right pixel		      */
    long		size;           /* Size of buffer for destination     */
    float		x, y;
    unsigned char	fill_mask;
    long		status;

    switch (src_udp->UdpB_Class)
        {
        case UdpK_ClassA:
	    switch (src_udp->UdpB_DType)
	        {
		case UdpK_DTypeBU:
		    /*
		    ** Preprocess set up for process flags.
		    */
		    if ( flags & IpsM_ReverseEdgeFill )
			fill_mask = 0;	    
		    else
			fill_mask = 0xFF;
		break;

		case UdpK_DTypeF:
		break;

		case UdpK_DTypeWU:
		case UdpK_DTypeLU:
		case UdpK_DTypeVU:
		case UdpK_DTypeV:
		default:
		    return (IpsX_UNSOPTION);
		break;
		}; /* end switch on DType */
	break;
	default:
	    return (IpsX_UNSOPTION);
        break;
    };/* end switch on class */

    /* 
    ** copy fixed fields of src udp into the dst udp 
    */
    dst_udp->UdpB_DType = src_udp->UdpB_DType;
    dst_udp->UdpB_Class = src_udp->UdpB_Class;
    dst_udp->UdpW_PixelLength = src_udp->UdpW_PixelLength;
    dst_udp->UdpL_Levels = src_udp->UdpL_Levels;
    dst_udp->UdpL_PxlStride = src_udp->UdpL_PxlStride;
    dst_udp->UdpL_CompIdx = src_udp->UdpL_CompIdx;
    
    dst_udp->UdpL_X1 = 0;
    dst_udp->UdpL_Y1 = 0;
    dst_udp->UdpL_X2 = dst_x_size - 1;
    dst_udp->UdpL_Y2 = dst_y_size - 1;
    dst_udp->UdpL_ScnCnt = dst_y_size ;
    dst_udp->UdpL_PxlPerScn = dst_x_size;
    dst_udp->UdpL_ScnStride = dst_udp->UdpL_PxlPerScn * dst_udp->UdpL_PxlStride;

    size = dst_udp->UdpL_ScnCnt * dst_udp->UdpL_ScnStride;
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
	if (src_udp->UdpB_DType == UdpK_DTypeF)
	    dst_udp->UdpA_Base = (unsigned char *)
		(*IpsA_MemoryTable[IpsK_AllocateDataPlane])
		    (((size + 7) >> 3), IpsM_InitMem, 0);
	else
	    dst_udp->UdpA_Base = (unsigned char *)
		(*IpsA_MemoryTable[IpsK_AllocateDataPlane])
		    (((size + 7) >> 3), 0, 0);
        if (!dst_udp->UdpA_Base) 
	    return (IpsX_INSVIRMEM);
        dst_udp->UdpL_ArSize = size;
        dst_udp->UdpL_Pos = 0;
        }

    switch (src_udp->UdpB_DType)
	{
	case UdpK_DTypeBU:
	src_ptr = (unsigned char *) 
	src_udp->UdpA_Base + (src_udp->UdpL_Pos>>3);
	dst_ptr = (unsigned char *) dst_udp->UdpA_Base + (dst_udp->UdpL_Pos>>3);

	sbyte_stride = (src_udp->UdpL_ScnStride >> 3);
	dbyte_stride = (dst_udp->UdpL_ScnStride >> 3);

	x = src_x;
	y = src_y;

	for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
	    {
	    cur_x = x;
	    cur_y = y;
	    cur_dx = dx;
	    cur_dy = dy;
	    for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		{
		if ((cur_x < src_udp->UdpL_X1) || (cur_x > src_udp->UdpL_X2) ||
		    (cur_y < src_udp->UdpL_Y1) || (cur_y > src_udp->UdpL_Y2))
			*dst_ptr++ = fill_mask;
		else
		    {
		    /*
		    ** Truncate to integer
		    */
		    ul_x = floor(cur_x);
		    ul_y = floor(cur_y);
		    /* 
		    ** Bilinear interpolate (cur_x,cur_y) with its 
		    ** neighbors 
		    */
		    offset = (ul_y * sbyte_stride) + ul_x;
		    ul_pixel = (float)src_ptr[ offset ];	    /* ul_x, ul_y */
		    ur_pixel = (float)src_ptr[ offset + 1 ];    /* ul_x, ur_y */

		    offset += sbyte_stride;		/* next line below*/
		    ll_pixel = (float)src_ptr[ offset ];/* ll_x, ll_y */
		    lr_pixel = (float)src_ptr[ offset + 1 ];    /* lr_x, lr_y */

		    talpha = (float)cur_x- (float)ul_x;	    /* bilinear inter*/
		    one_minus_talpha = 1.0 - talpha;
		    beta = (float)cur_y - (float)ul_y;
		    one_minus_beta = 1.0 - beta;
		    *dst_ptr++ = 
			(one_minus_talpha * ((one_minus_beta * ul_pixel) +
		    	   (beta * ll_pixel)) + talpha * ((one_minus_beta * 
				ur_pixel) + (beta * lr_pixel))) + 0.5;
		    }
		/* 
		** update to next pixel in source if not a quadratic warp 
		** then d_dx = d_dy = 0
		*/
		cur_x += cur_dx;
		cur_y += cur_dy;
		cur_dx += d_dx;
		cur_dy += d_dy;
		}
	    x += dxr;
	    y += dyr;
	    dx += d_dxr;
	    dy += d_dyr;	
	    }
	break;

	case UdpK_DTypeF:
	src_float_ptr = (float *) src_udp->UdpA_Base + (src_udp->UdpL_Pos>>5);
	dst_float_ptr = (float *) dst_udp->UdpA_Base + (dst_udp->UdpL_Pos>>5);
	sbyte_stride = (src_udp->UdpL_ScnStride >> 5);
	dbyte_stride = (dst_udp->UdpL_ScnStride >> 5);

	x = src_x;
	y = src_y;

	for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
	    {
	    cur_x = x;
	    cur_y = y;
	    cur_dx = dx;
	    cur_dy = dy;
	    for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
		{
		if ((cur_x < src_udp->UdpL_X1) || (cur_x > src_udp->UdpL_X2) ||
		    (cur_y < src_udp->UdpL_Y1) || (cur_y > src_udp->UdpL_Y2))
			dst_float_ptr++;
		else
		    {
		    /*
		    ** Truncate to integer
		    */
		    ul_x = floor(cur_x);
		    ul_y = floor(cur_y);
		    /* 
		    ** Bilinear interpolate (cur_x,cur_y) with its 
		    ** neighbors 
		    */
		    offset = (ul_y * sbyte_stride) + ul_x;
		    ul_pixel = src_float_ptr[ offset ];	    /* ul_x, ul_y */
		    ur_pixel = src_float_ptr[ offset + 1 ];    /* ul_x, ur_y */

		    offset += sbyte_stride;		/* next line below*/
		    ll_pixel = src_float_ptr[ offset ];/* ll_x, ll_y */
		    lr_pixel = src_float_ptr[ offset + 1 ];    /* lr_x, lr_y */

		    talpha = (float)cur_x- (float)ul_x;	    /* bilinear inter*/
		    one_minus_talpha = 1.0 - talpha;
		    beta = (float)cur_y - (float)ul_y;
		    one_minus_beta = 1.0 - beta;
		    *dst_float_ptr++ = 
			(one_minus_talpha * ((one_minus_beta * ul_pixel) +
		    	   (beta * ll_pixel)) + talpha * ((one_minus_beta * 
				ur_pixel) + (beta * lr_pixel))) + 0.5;
		    }
		/* 
		** update to next pixel in source if not a quadratic warp 
		** then d_dx = d_dy = 0
		*/
		cur_x += cur_dx;
		cur_y += cur_dy;
		cur_dx += d_dx;
		cur_dy += d_dy;
		}
	    x += dxr;
	    y += dyr;
	    dx += d_dxr;
	    dy += d_dyr;	
	    }
	break;
	}
    return (IpsX_SUCCESS);
    }
