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
**  IpsRotateCont
**
**  FACILITY:
**
**	Image Processing Services (IPS)
**
**  ABSTRACT:
**
**	This module contains the ICS entries for rotating greyscale
** 	or color images. This code will rotate an image plane using
** 	the bilinear interpolation or nearest neighbor algorithm unless
** 	the angle can be normalized to (+ or -) 90, 180, 270 in which 
**	case it will use a special case algorithm.
**
**  ENVIRONMENT:
**
**	VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
**  	Karen Rodwell, Digital Equipment Corp.
**	Richard Piccolo , Digital Equipment Corp.
**
**  CREATION DATE:
**
**	October 2, 1989
**
**  MODIFICATION DATE:
**
**	July 1990
**
**	ROI Fixes, Speed Improvements and Documentation
**
************************************************************************/

/*
**  Table of contents:
**
*/
    /* Layer 1 Entries (Data Class & Type Switch) */

#ifdef NODAS_PROTO
long	    _IpsRotateOrthogonal();    	    /* Rotate 90, 180 or 270	    */
long	    _IpsRotateInterpolation();      /* Rotate using bilinear interp */
long	    _IpsRotateNearestNeighbor();    /* Rotate using sampling	    */

   /* Data Class & Type Sensitive Entries */

long	    _IpsRotateInterpolationByte();  /* Rotate bytes using bilin int */
long	    _IpsRotateNearestNeighborByte();/* Rotate bytes using sampling  */
long	    _IpsRotateCont180Byte();	    /* Rotate 180 or -180 degrees   */
long	    _IpsRotateCont270Byte();	    /* Rotate 270 or -90 degrees    */
long	    _IpsRotateCont90Byte();	    /* Rotate 90 or -270 degrees    */

/*
**  Internal routines
*/
void	    _IpsGetRotatedUdp();	    /* Subroutine to load new Udp   */
#endif


/*
**  Include files:
**
*/
#include <IpsMemoryTable.h>		    /* IPS Memory Mgt. Functions    */
#include <IpsDef.h>			    /* IPS Image Definitions	    */
#include <IpsStatusCodes.h>		    /* IPS Status Codes 	    */
#ifndef NODAS_PROTO
#include <ipsprot.h>				    /* Ips prototypes */
#endif
#include <math.h>			    /* Math Library		    */

#define RADIANS_PER_DEGREE 0.0174532925199432959

/*
**  External References:
*/

#ifdef NODAS_PROTO
long _IpsVerifyNotInPlace();		    /* from Ips__udp_utils.c	    */
#endif

/******************************************************************************
**  _IpsRotateOrthogonal
**
**  FUNCTIONAL DESCRIPTION:
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
**      angle   --> Degrees of rotation (Only supports: +-(90,180,270)
**		    All other angles are unsupported. Call Nearest Neighbor
**		    or Interpolation for all other angles.
**      flags   --> Fill mask
**
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   return status
**  SIGNAL CODES:     none
**  SIDE EFFECTS:     none
**
**  RESTRICTIONS:
**
**  - RotateOrthogonal ONLY operates on angles that are orthogonal.
**    It returns UNSUPPORTED if the angle is not +- 90,180, 270.
**    Zero and 360 degrees are unsupported in this layer II routine
**    as well.
**  
**  - Operates on the ISL native format (input is band interleaved by plane)
**    for byte images only, therefore a pixel stride of one byte is assumed.
**
**  OPERATION (non-orthogonal rotates):
**
**  For non-orthogonal rotates, this function finds the corresponding source
**  pixel for each dst pixel.  The steps include:
**
**	- finds new dst dimensions
**	- establishes x and y translations so that the inverse rotate
**	  of the dst back to the source is aligned correctly 
**	- adjust x and y translations so that ROIs are handled correctly and
**	  that rounding occurs.
**
**  NOTE: The images are worked on in normal cartesian coordinates where
**	  the images are in the 1st quadrant.
**
******************************************************************************/
long _IpsRotateOrthogonal(src_udp, dst_udp, angle, flags)
struct UDP *src_udp; 			/* Working src UDP descriptor */
struct UDP *dst_udp;			/* Working dst UDP descriptor */
long	   *angle;			/* Degrees of rotation        */
long 	   flags;			/* Flag to indicate edge fill */

{
unsigned long mem_alloc = 0;
long    size;				/* buffer size   	      */
long	local_angle = *angle;		/* integer angle of rotation  */
long	status;

local_angle *= -1;
switch (src_udp->UdpB_Class)
    {
    case UdpK_ClassA:
    switch (src_udp->UdpB_DType)
        {
        case UdpK_DTypeBU:
	    {
	    switch (local_angle)
		{
	    	case 90:
		case -270:
		    {
		    dst_udp->UdpL_ScnCnt = src_udp->UdpL_PxlPerScn;
		    dst_udp->UdpL_PxlPerScn = src_udp->UdpL_ScnCnt;
		    dst_udp->UdpL_ScnStride = src_udp->UdpL_PxlStride * 
					dst_udp->UdpL_PxlPerScn;
		    dst_udp->UdpL_X1 = 0;
		    dst_udp->UdpL_Y1 = 0;
		    dst_udp->UdpL_X2 = dst_udp->UdpL_PxlPerScn - 1;
		    dst_udp->UdpL_Y2 = dst_udp->UdpL_ScnCnt - 1;
		    dst_udp->UdpW_PixelLength = src_udp->UdpW_PixelLength;
		    dst_udp->UdpL_PxlStride = src_udp->UdpL_PxlStride;
		    dst_udp->UdpB_DType = src_udp->UdpB_DType;
		    dst_udp->UdpB_Class = src_udp->UdpB_Class;
		    dst_udp->UdpL_Levels = src_udp->UdpL_Levels;
		    size = dst_udp->UdpL_ScnStride * dst_udp->UdpL_ScnCnt;
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
			dst_udp->UdpL_ArSize = size;
			dst_udp->UdpL_Pos = 0;
			mem_alloc = 1;
			}
	            status = _IpsRotateCont90Byte(src_udp, dst_udp);
		    break;
		    }
		break;

	        case 180:
		case -180:
		    {
		    dst_udp->UdpL_ScnCnt = src_udp->UdpL_ScnCnt;
		    dst_udp->UdpL_PxlPerScn = src_udp->UdpL_PxlPerScn;
		    dst_udp->UdpL_ScnStride = 
			src_udp->UdpL_PxlStride * dst_udp->UdpL_PxlPerScn;
		    dst_udp->UdpL_X1 = 0;
		    dst_udp->UdpL_Y1 = 0;
		    dst_udp->UdpL_X2 = dst_udp->UdpL_PxlPerScn - 1;
		    dst_udp->UdpL_Y2 = dst_udp->UdpL_ScnCnt - 1;
		    dst_udp->UdpW_PixelLength = src_udp->UdpW_PixelLength;
		    dst_udp->UdpL_PxlStride = src_udp->UdpL_PxlStride;
		    dst_udp->UdpB_DType = src_udp->UdpB_DType;
		    dst_udp->UdpB_Class = src_udp->UdpB_Class;
		    dst_udp->UdpL_Levels = src_udp->UdpL_Levels;
		    size = dst_udp->UdpL_ScnStride * dst_udp->UdpL_ScnCnt;
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
			dst_udp->UdpL_ArSize = size;
			dst_udp->UdpL_Pos = 0;
			mem_alloc = 1;
			}
	            status = _IpsRotateCont180Byte (src_udp, dst_udp);
		    break;
		    }
		break;

	        case 270:
		case -90:
		    {
		    dst_udp->UdpL_ScnCnt = src_udp->UdpL_PxlPerScn;
		    dst_udp->UdpL_PxlPerScn = src_udp->UdpL_ScnCnt;
		    dst_udp->UdpL_ScnStride = src_udp->UdpL_PxlStride * 
				dst_udp->UdpL_PxlPerScn;
		    dst_udp->UdpL_X1 = 0;
		    dst_udp->UdpL_Y1 = 0;
		    dst_udp->UdpL_X2 = dst_udp->UdpL_PxlPerScn - 1;
		    dst_udp->UdpL_Y2 = dst_udp->UdpL_ScnCnt - 1;
		    dst_udp->UdpW_PixelLength = src_udp->UdpW_PixelLength;
		    dst_udp->UdpL_PxlStride = src_udp->UdpL_PxlStride;
		    dst_udp->UdpB_DType = src_udp->UdpB_DType;
		    dst_udp->UdpB_Class = src_udp->UdpB_Class;
		    dst_udp->UdpL_Levels = src_udp->UdpL_Levels;
		    size = dst_udp->UdpL_ScnStride * dst_udp->UdpL_ScnCnt;
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
			dst_udp->UdpL_ArSize = size;
			dst_udp->UdpL_Pos = 0;
			mem_alloc = 1;
			}
	            status = _IpsRotateCont270Byte(src_udp, dst_udp);
		    break;
		    }
		break;

		default:
		    {
		    return (IpsX_UNSOPTION);
		    break;
		    }
	        }
	    if ((status != IpsX_SUCCESS) && mem_alloc == 1)
		(*IpsA_MemoryTable[IpsK_FreeDataPlane])(dst_udp->UdpA_Base);
	    return (status);
            break;
	    }
        case UdpK_DTypeWU:
        case UdpK_DTypeLU:
        case UdpK_DTypeF:
        case UdpK_DTypeVU:
        case UdpK_DTypeV:
        default:
	    /* angle is not orthogonal */
            return (IpsX_UNSOPTION);
            break;
        };  /* end switch on DType */
    default:
        return (IpsX_UNSOPTION);
        break;

    };/* end switch on class */

} /* end of _IpsRotateOrthogonal */

/******************************************************************************
**  _IpsRotateCont90Byte()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Rotate an image by exactly 90.0 degrees.
**
**  FORMAL PARAMETERS:
**	src_udp	--> Pointer to source udp (initialized)
**	dst_udp --> Pointer to destination udp (uninitialized)
**
**  IMPLICIT INPUTS: none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE: return status
**  SIGNAL CODES: none.
**  SIDE EFFECTS: none
**
******************************************************************************/
long _IpsRotateCont90Byte( src_udp, dst_udp)
struct UDP *src_udp; 			/* Working src UDP descriptor */
struct UDP *dst_udp;			/* Working dst UDP descriptor */
{
unsigned char	*dstdata;		/* Destination data pointer    */
unsigned char	*srcdata;		/* Source data pointer	       */
unsigned char	*src_plane_data_base;	/* Source plane data base      */
long     dst_idx;			/* Destination index counter   */
long     dst_offset;			/* Destination data offset     */
long     dst_pixels_per_line_less1;	/* Destination pix per line -1 */
long     src_idx;			/* Source index counter        */
long     src_line_counter;		/* Source line counter	       */
long	 src_offset;			/* Source data offset	       */
long     src_pixel_stride;		/* Source pixel stride	       */
long	 src_stride_bytes;		/* Source pixel stride in bytes*/
long	 dst_stride_bytes;		/* Destination pix stride bytes*/
long	 dst_startpos;

/*
** Rotate the image.  
*/
src_plane_data_base = (unsigned char *)src_udp->UdpA_Base;
src_plane_data_base += (src_udp->UdpL_Pos>>3) + (src_udp->UdpL_ScnStride>>3) *
    src_udp->UdpL_Y1 + src_udp->UdpL_X1;
src_stride_bytes = src_udp->UdpL_ScnStride >> 3;
dst_stride_bytes = dst_udp->UdpL_ScnStride >> 3;
dst_pixels_per_line_less1 = dst_udp->UdpL_PxlPerScn - 1;

dst_startpos = ((long) dst_udp->UdpA_Base) + (dst_udp->UdpL_Pos>>3) +
			    (dst_udp->UdpL_ScnCnt-1) * dst_stride_bytes;

src_offset = -src_stride_bytes;
src_line_counter = 0;
for( dst_idx = 0;
    src_line_counter < src_udp->UdpL_ScnCnt;
    	++src_line_counter, ++dst_idx )
    for( src_offset += src_stride_bytes,
	srcdata = (unsigned char *)src_plane_data_base + src_offset,
	src_idx = 0,
	dstdata = (unsigned char *)dst_startpos;
	    src_idx < src_udp->UdpL_PxlPerScn;
		++src_idx, dstdata -= dst_stride_bytes )
	{
	dstdata[ dst_idx ] = srcdata[ src_idx ];
	}
return (IpsX_SUCCESS);
} /* end of _IpsRotateCont90Byte */

/******************************************************************************
**  _IpsRotateCont180Byte()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Rotate an image by exactly 180.0 degrees.
**
**  FORMAL PARAMETERS:
**	src_udp	--> Pointer to source udp (initialized)
**	dst_udp --> Pointer to destination udp (uninitialized)
**
**  IMPLICIT INPUTS: none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE: return status
**  SIGNAL CODES: none.
**  SIDE EFFECTS: none
**
******************************************************************************/
long _IpsRotateCont180Byte( src_udp, dst_udp)
struct UDP *src_udp; 			/* Working src UDP descriptor */
struct UDP *dst_udp;			/* Working dst UDP descriptor */
{
unsigned char	*dstdata;		/* destination data buffer ptr */
unsigned char	*srcdata;		/* source data buffer pointer  */
unsigned char	*src_plane_data_base;   /* pointer to source data base */
long     dst_idx;			/* destination index counter   */
long     dst_offset;			/* offset into destination data*/
long     dst_pixels_per_line_less1;	/* dest pixels per line - 1    */
long     src_idx;			/* source index counter	       */
long     src_line_counter;		/* source line counter 	       */
long     src_offset;			/* offset into source data     */
long     src_pixel_stride;		/* source pixel stride	       */
long	 src_stride_bytes;   		/* source stride in bytes      */
long	 dst_stride_bytes;   		/* destination stride in bytes */
/*
** Rotate the image.  
*/
src_plane_data_base = (unsigned char *)src_udp->UdpA_Base;
src_plane_data_base += (src_udp->UdpL_Pos>>3) + (src_udp->UdpL_ScnStride>>3) *
    src_udp->UdpL_Y1 + src_udp->UdpL_X1;
dst_pixels_per_line_less1 = dst_udp->UdpL_PxlPerScn - 1;
dst_stride_bytes = (dst_udp->UdpL_ScnStride >> 3);
src_stride_bytes = (src_udp->UdpL_ScnStride >> 3);

for( src_line_counter = 0,
     src_offset = -src_stride_bytes,
     dst_offset = dst_stride_bytes * dst_udp->UdpL_ScnCnt;
    	src_line_counter < src_udp->UdpL_ScnCnt;
    	    ++src_line_counter )
    for( 
	src_offset += src_stride_bytes, 
	srcdata = src_plane_data_base + src_offset,
	src_idx = 0,
	dst_offset -= dst_stride_bytes,
	dstdata = (unsigned char *)dst_udp->UdpA_Base + 
	    (dst_udp->UdpL_Pos>>3) + dst_offset,
	dst_idx = dst_pixels_per_line_less1;
	    src_idx < src_udp->UdpL_PxlPerScn;
		++src_idx, --dst_idx )
    {
    dstdata[ dst_idx ] = srcdata[ src_idx ];
    }
return (IpsX_SUCCESS);
} /* end of _IpsRotateCont180Byte */

/******************************************************************************
**  _IpsRotateCont270Byte
**
**  FUNCTIONAL DESCRIPTION:
**
**	Rotate an image by exactly 270.0 degrees.
**
**  FORMAL PARAMETERS:
**	src_udp	--> Pointer to source udp (initialized)
**	dst_udp --> Pointer to destination udp (uninitialized)
**
**  IMPLICIT INPUTS: none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE: return status
**  SIGNAL CODES: none.
**  SIDE EFFECTS: none
**
******************************************************************************/
long _IpsRotateCont270Byte( src_udp, dst_udp)
struct UDP *src_udp; 			/* Working src UDP descriptor */
struct UDP *dst_udp;			/* Working dst UDP descriptor */
{
unsigned char	*dstdata;		/* Destination data pointer    */
unsigned char	*srcdata;		/* Source data pointer	       */
unsigned char	*src_plane_data_base;	/* Source plane data base      */
long     dst_idx;			/* Destination index counter   */
long     dst_offset;			/* Destination data offset     */
long     dst_pixels_per_line_less1;	/* Destination pix per line -1 */
long     src_idx;			/* Source index counter        */
long     src_line_counter;		/* Source line counter	       */
long	 src_offset;			/* Source data offset	       */
long     src_pixel_stride;		/* Source pixel stride	       */
long	 src_stride_bytes;		/* Source pixel stride in bytes*/
long	 dst_stride_bytes;		/* Destination pix stride bytes*/

/*
** Rotate the image.  
*/
src_plane_data_base = (unsigned char *)src_udp->UdpA_Base;
src_plane_data_base += (src_udp->UdpL_Pos>>3) + (src_udp->UdpL_ScnStride>>3) *
    src_udp->UdpL_Y1 + src_udp->UdpL_X1;
src_stride_bytes = src_udp->UdpL_ScnStride >> 3;
dst_pixels_per_line_less1 = dst_udp->UdpL_PxlPerScn - 1;
dst_stride_bytes = dst_udp->UdpL_ScnStride >> 3;

for( src_line_counter = 0,
     src_offset = -src_stride_bytes;
    	src_line_counter < src_udp->UdpL_ScnCnt;
    	    ++src_line_counter )
    for( 
	src_offset += src_stride_bytes,
	srcdata = src_plane_data_base + src_offset,
	src_idx = 0,
	dstdata = (unsigned char *)dst_udp->UdpA_Base + (dst_udp->UdpL_Pos>>3),
	dst_idx = dst_pixels_per_line_less1 - src_line_counter;
	    src_idx < src_udp->UdpL_PxlPerScn;
		++src_idx, dstdata += dst_stride_bytes )
	    {
	    dstdata[ dst_idx ] = srcdata[ src_idx ];
	    } /* end for */
return (IpsX_SUCCESS);
} /* end of _IpsRotateCont270Byte */


/******************************************************************************
**  _IpsRotateInterpolation
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
**      angle   --> Degrees of  rotation 
**      flags   --> Fill mask
**
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   return status
**  SIGNAL CODES:     none
**  SIDE EFFECTS:     none
**
**  RESTRICTIONS:
**  Operates on the ISL native format (input is band interleaved by plane)
**  for byte images only, therefore a pixel stride of one byte is assumed.
******************************************************************************/
long _IpsRotateInterpolation(src_udp, dst_udp, angle, flags)
struct UDP *src_udp; 			/* Working src UDP descriptor */
struct UDP *dst_udp;			/* Working dst UDP descriptor */
float	   *angle;			/* Degrees of rotation        */
long 	   flags;			/* Flag to indicate edge fill */
{
float	local_angle = *angle * -1;		/* local degrees of rotation     */
long    int_angle = *angle * -1;
float fraction = 0.0;
long norm_angle = 0;
            
fraction = *angle + int_angle;
int_angle %= 360;

/* Negate the angle so that positive input angles are
** rotated clockwise */
norm_angle = int_angle % 90;

switch (src_udp->UdpB_Class)
    {
    case UdpK_ClassA:
    switch (src_udp->UdpB_DType)
        {
        case UdpK_DTypeBU:
	    {
	    if ((norm_angle == 0) && (fraction == 0.0))
		    {
		    /* negate the angle back to it's original value */
		    int_angle *= -1;
	            return (_IpsRotateOrthogonal(src_udp, dst_udp,
					&int_angle, flags));
		    }
            else
		{
	        if ((src_udp->UdpL_PxlPerScn == 1) || 
		    (src_udp->UdpL_ScnCnt == 1))
		    return (_IpsRotateNearestNeighborByte
				(src_udp, dst_udp, &local_angle, flags));
		else
		    return (_IpsRotateInterpolationByte
			        (src_udp, dst_udp, &local_angle, flags));
	        }
	    }
        case UdpK_DTypeWU:
        case UdpK_DTypeLU:
        case UdpK_DTypeF:
        case UdpK_DTypeVU:
        case UdpK_DTypeV:
        default:
            return (IpsX_UNSOPTION);
            break;
        }; /* end switch on DType */
    default:
        return (IpsX_UNSOPTION);
        break;

    };/* end switch on class */

} /* end of _IpsRotateInterpolation*/

/******************************************************************************
**  _IpsRotateInterpolationByte()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Rotate an image through any number of degrees using 
**	bilinear interpolation to interpolate the value of pixels
**	located at non-integral coordinates in the source.
**
**  FORMAL PARAMETERS:
**
**	src_udp	--> Pointer to source udp (initialized)
**	dst_udp --> Pointer to destination udp (uninitialized)
**      angle   --> Degrees of  rotation 
**      flags   --> Fill mask
**
**  IMPLICIT INPUTS: none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE: return status
**  SIGNAL CODES: none.
**  SIDE EFFECTS: none
**  RESTRICTIONS: width and height must be greater than 1
**		  otherwise use nearest neighbor
**
******************************************************************************/
long _IpsRotateInterpolationByte(src_udp, dst_udp, angle_ptr, flags )
struct UDP *src_udp;                    /* Working src UDP descriptor	      */
struct UDP *dst_udp;                    /* Working dst UDP descriptor	      */
float	*angle_ptr;			/* Angle of rotation in degrees	      */
long	 flags;				/* Flag indicates edge fill desired   */
{
float	 	angle = *angle_ptr;	/* Angle as integer		      */
long	 	sbyte_stride;		/* Source byte stride		      */
long	 	dbyte_stride;		/* Destination byte stride	      */
long	 	ix, iy;			/* Scratch counter variables	      */
long	 	size;			/* Size of buffer for destination     */
long		ul_x;                   /* upper left x			      */
long		ul_y;                   /* upper left y			      */
long     	offset;      		/* offset to current pixel in src     */
float		sinna, cosna;		/* Sin & Cos of neg radian angle      */
float   	xsinna, xcosna;		/* x sin and cos neg angle	      */
float		ysinna, ycosna;		/* y sin and cos neg angle	      */
float   	x_translation;		/* x correction to src vals	      */
float		y_translation;		/* y correction to src vals	      */
float		xmin,ymin;		/* low coordinates of dst UDP 	      */ 
float		srcx, srcy;		/* coordinates of src that map to dst */
double	 	angle_in_radians;	/* angle in degrees * radians/deg     */
unsigned char	*dstdata;		/* destination data pointer           */
unsigned char	*srcdata;		/* source data pointer		      */
unsigned char	fill_mask;		/* edge fill flag -- zero or one      */
float    	talpha,beta;		/* source - lower left or upper right */
float    	one_minus_talpha;	/* 1 minus talpha		      */
float		one_minus_beta;		/* 1 minus beta			      */
float    	ll_pixel;               /* lower left pixel		      */
float    	lr_pixel;               /* lower right pixel		      */
float    	ul_pixel;               /* upper left pixel		      */
float    	ur_pixel;               /* upper right pixel		      */
long		status;
float		lower_bound_x, lower_bound_y;
float		x_start, y_start;

/*
** Preprocess set up for process flags.
*/
if ( flags & IpsM_ReverseEdgeFill )
    fill_mask = 0;			/* reverse is all pixel bits = 0    */
else
    fill_mask = 0xFF;			/* "normal" is all pixel bits = 1   */

/* 
** copy fixed fields of src udp into the dst udp 
*/
dst_udp->UdpB_DType = src_udp->UdpB_DType;
dst_udp->UdpB_Class = src_udp->UdpB_Class;
dst_udp->UdpW_PixelLength = src_udp->UdpW_PixelLength;
dst_udp->UdpL_Levels = src_udp->UdpL_Levels;
dst_udp->UdpL_PxlStride = src_udp->UdpL_PxlStride;
dst_udp->UdpL_CompIdx = src_udp->UdpL_CompIdx;

/* 
** Figure rest of the fields based on angle 
*/
_IpsGetRotatedUdp (src_udp, dst_udp, angle, &xmin, &ymin);
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
    dst_udp->UdpA_Base = (unsigned char *)
	(*IpsA_MemoryTable[IpsK_AllocateDataPlane])
	    (((size + 7) >> 3), 0, 0);
    if (!dst_udp->UdpA_Base) return (IpsX_INSVIRMEM);
    dst_udp->UdpL_ArSize = size;
    dst_udp->UdpL_Pos = 0;
    }

angle_in_radians = angle * RADIANS_PER_DEGREE;
srcdata = (unsigned char *) src_udp->UdpA_Base + (src_udp->UdpL_Pos>>3);
dstdata = (unsigned char *) dst_udp->UdpA_Base + (dst_udp->UdpL_Pos>>3);

sbyte_stride = (src_udp->UdpL_ScnStride >> 3);
dbyte_stride = (dst_udp->UdpL_ScnStride >> 3);

cosna = cos(-angle_in_radians);
sinna = sin(-angle_in_radians);

/* 
** must correct the mapping to negative values with x and y translations 
** and ROI!! Remember the image is 1st quadrant positioned.  The coordinates
** are Udp based coordinates (i.e. upper left is 0,0) - that's why the y2
** variable in the y_translation calculation
*/
x_translation = (xmin * cosna - ymin * sinna) + src_udp->UdpL_X1;
y_translation = src_udp->UdpL_Y2 - (xmin * sinna + ymin * cosna);

/*
** Map each pixel in the dst udp to the src udp , going from top to bottom
*/
ycosna = (dst_udp->UdpL_ScnCnt-dst_udp->UdpL_Y1-1) * cosna;/* top-dst y*cosna */
ysinna = (dst_udp->UdpL_ScnCnt-dst_udp->UdpL_Y1-1) * sinna;/* top_dst y*sinna */

x_start = x_translation - ysinna;
y_start = y_translation - ycosna;

/*
** convert these to float outside loop for loop src boundary test
*/
lower_bound_x = (float)src_udp->UdpL_X1;
lower_bound_y = (float)src_udp->UdpL_Y1;

for (iy=0; iy < dst_udp->UdpL_ScnCnt; iy++, 
    x_start += sinna,	    /* really minus -sinna  - working down the image  */
    y_start += cosna)	    /* really minus -cosna  - working down the image  */
    {
    srcx = x_start;	
    srcy = y_start;     

    for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++, 
	srcx += cosna,	/* srcx = (ix * cosna) - (iy * sinna) + x_translation */
	srcy -= sinna)  /* srcy = y_translation - (ix * sinna) - (iy * cosna) */
        {
	/*
	** Truncate to integer
	*/
	ul_x = srcx;
	ul_y = srcy;

	/*
	** Boundary condition - ignore bottom and rightmost source pixel
	** to avoid an additional check.  Check lower bounds using
	** srcx, srcy - has the same affect as ul_* = floor (src*) (remember
	** that srcx, srcy can be negative and if so ul_x and ul_y can 
	** erroneously fall within range when truncating)
	*/
	if ((ul_x >= src_udp->UdpL_X2) || (ul_y >= src_udp->UdpL_Y2)
	    || (srcx < lower_bound_x) || (srcy < lower_bound_y))
		*dstdata++ = fill_mask;
	else
	    /* 
	    ** Bilinear interpolate (srcx,srcy) with its neighbors 
	    */
	    {
	    offset = (ul_y * sbyte_stride) + ul_x;
	    ul_pixel = (float)srcdata[ offset ];	    /* ul_x, ul_y */
	    ur_pixel = (float)srcdata[ offset + 1 ];	    /* ul_x, ur_y */

	    offset += sbyte_stride;			    /* next line below*/
	    ll_pixel = (float)srcdata[ offset ];	    /* ll_x, ll_y */
	    lr_pixel = (float)srcdata[ offset + 1 ];	    /* lr_x, lr_y */

	    talpha = (float)srcx - (float)ul_x;		    /* bilinear inter*/
	    one_minus_talpha = 1.0 - talpha;
	    beta = (float)srcy - (float)ul_y;
	    one_minus_beta = 1.0 - beta;
	    *dstdata++ = 
		(one_minus_talpha * ((one_minus_beta * ul_pixel) +
				   (beta * ll_pixel)) +
		talpha * ((one_minus_beta * ur_pixel) +
			 (beta * lr_pixel))) + 0.5;
	    } /* end of bilinear interpolation */
        }
    } 
return (IpsX_SUCCESS);
}/* end _IpsRotateInterpolationByte */

/******************************************************************************
**  _IpsRotateNearestNeighbor
**
**  FUNCTIONAL DESCRIPTION:
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
**      angle   --> Degrees of  rotation 
**      flags   --> Fill mask
**
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   return status
**  SIGNAL CODES:     none
**  SIDE EFFECTS:     none
**
**  RESTRICTIONS:
**  Operates on the ISL native format (input is band interleaved by plane)
**  for byte images only, therefore a pixel stride of one byte is assumed.
******************************************************************************/
long _IpsRotateNearestNeighbor(src_udp, dst_udp, angle, flags)
struct UDP *src_udp; 			/* Working src UDP descriptor */
struct UDP *dst_udp;			/* Working dst UDP descriptor */
float	   *angle;			/* Degrees of rotation        */
long 	   flags;			/* Flag to indicate edge fill */
{
float	local_angle = *angle * -1;		/* local degrees of rotation     */
long    int_angle = *angle * -1;
float fraction = 0.0;
long norm_angle = 0;
            
fraction = *angle + int_angle;
int_angle %= 360;
norm_angle = int_angle % 90;            

switch (src_udp->UdpB_Class)
    {
    case UdpK_ClassA:
    switch (src_udp->UdpB_DType)
        {
        case UdpK_DTypeBU:
	    {
	    if ((norm_angle == 0) && (fraction == 0.0))
		    {
		    int_angle *= -1;
	            return (_IpsRotateOrthogonal(src_udp, dst_udp,
					&int_angle, flags));
		    }
            else
		{
	        return (_IpsRotateNearestNeighborByte
			(src_udp, dst_udp, &local_angle, flags));
		break;
		}
            }
        case UdpK_DTypeWU:
        case UdpK_DTypeLU:
        case UdpK_DTypeF:
        case UdpK_DTypeVU:
        case UdpK_DTypeV:
        default:
            return (IpsX_UNSOPTION);
            break;
        }; /* end switch on DType */
    default:
        return (IpsX_UNSOPTION);
        break;
    };/* end switch on class */
} /* end of _IpsRotateNearestNeighbor*/

/******************************************************************************
**  _IpsRotateNearestNeighborByte()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Rotate an image through any number of degrees using the
**	nearest neighbor algorithm to interpolate the value of pixels
**	located at non-integral coordinates in the source.
**
**  FORMAL PARAMETERS:
**
**	src_udp	--> Pointer to source udp (initialized)
**	dst_udp --> Pointer to destination udp (uninitialized)
**      angle   --> Degrees of  rotation 
**      flags   --> Fill mask
**
**  IMPLICIT INPUTS: none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE: return status
**  SIGNAL CODES: none.
**  SIDE EFFECTS: none
**
******************************************************************************/
long _IpsRotateNearestNeighborByte( src_udp, dst_udp, angle_ptr, flags )
struct UDP *src_udp;                    /* Working src UDP descriptor */
struct UDP *dst_udp;                    /* Working dst UDP descriptor */
float	*angle_ptr;
long	 flags;
{
float	 	angle = *angle_ptr;	/* angle of rotation		      */
float		sinna, cosna;		/* sin and cos of neg angle	      */
float		xmin,ymin;		/* min x and y of destination	      */
float   	xsinna, xcosna;		/* x sin and cos neg angle	      */
float		ysinna, ycosna;		/* y sin and cos neg angle	      */
float   	x_translation;		/* x correction to src vals	      */
float		y_translation;		/* y correction to src vals	      */
double	 	angle_in_radians;	/* angle * radians per degree	      */
long		sbyte_stride;		/* source stride in bytes	      */
long		dbyte_stride;		/* destination stride in bytes	      */
long		size;			/* size of destination buffer	      */
long	 	ix, iy;			/* scratch variables for loop	      */
long		nearest_x;		/* nearest neighbor x variable	      */
long		nearest_y;		/* nearest neighbor y variable	      */
unsigned char	*dstdata;		/* destination data pointer	      */
unsigned char	*srcdata;		/* source data pointer		      */
unsigned char	fill_mask;		/* edge fill value 0 or 1	      */
long		status;
float		x_start, y_start;
float		srcx, srcy;

/*
** Preprocess set up for process flags.
*/

if ( flags & IpsM_ReverseEdgeFill )
    fill_mask = 0;			/* reverse is all pixel bits = 0    */
else
    fill_mask = 0xFF;			/* "normal" is all pixel bits = 1   */

/* 
** copy fixed fields of src udp into the dst udp 
*/
dst_udp->UdpB_DType = src_udp->UdpB_DType;
dst_udp->UdpB_Class = src_udp->UdpB_Class;
dst_udp->UdpW_PixelLength = src_udp->UdpW_PixelLength;
dst_udp->UdpL_Levels = src_udp->UdpL_Levels;
dst_udp->UdpL_PxlStride = src_udp->UdpL_PxlStride;
dst_udp->UdpL_CompIdx = src_udp->UdpL_CompIdx;

/* 
** Figure rest of the fields based on angle 
*/
_IpsGetRotatedUdp (src_udp, dst_udp, angle, &xmin, &ymin);
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
    dst_udp->UdpA_Base = (unsigned char *)
	(*IpsA_MemoryTable[IpsK_AllocateDataPlane])
	    (((size + 7) >> 3), 0, 0);
    if (!dst_udp->UdpA_Base) return (IpsX_INSVIRMEM);
    dst_udp->UdpL_ArSize = size;
    dst_udp->UdpL_Pos = 0;
    }

angle_in_radians = angle * RADIANS_PER_DEGREE;

sbyte_stride = (src_udp->UdpL_ScnStride >> 3);
dbyte_stride = (dst_udp->UdpL_ScnStride >> 3);

/* 
** make source base be origin (i.e. get to bottom/left of the src parent image) 
*/
srcdata = (unsigned char *) src_udp->UdpA_Base + (src_udp->UdpL_Pos>>3);
dstdata = (unsigned char *) dst_udp->UdpA_Base + (dst_udp->UdpL_Pos>>3);

cosna = cos(-angle_in_radians);
sinna = sin(-angle_in_radians);

/* 
** must correct the inverse mapping to negative values with x and y 
** translations and ROI!! We are also converting to udp coordinate space.
** Also add rounding factor here instead of in the loop 
*/
x_translation = (xmin * cosna - ymin * sinna) + src_udp->UdpL_X1 + 0.5;
y_translation = xmin * sinna + ymin * cosna + 0.5;

ycosna = (dst_udp->UdpL_ScnCnt-dst_udp->UdpL_Y1-1) * cosna;/* top-dst y*cosna */
ysinna = (dst_udp->UdpL_ScnCnt-dst_udp->UdpL_Y1-1) * sinna;/* top_dst y*sinna */

x_start = x_translation - ysinna;
y_start = (float)src_udp->UdpL_Y2 - (y_translation + ycosna);

for (iy=0; iy < dst_udp->UdpL_ScnCnt; iy++, 
    x_start += sinna,	    /* really minus -sinna  - working down the image  */
    y_start += cosna)	    /* really minus -cosna  - working down the image  */
    {
    srcx = x_start;
    srcy = y_start;

    for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++, 
	srcx += cosna,	/* srcx = (ix * cosna) - (iy * sinna) + x_translation */
	srcy -= sinna)  /* srcy = y_translation - (ix * sinna) - (iy * cosna) */
        {
        nearest_x = srcx + 0.5;
        nearest_y = srcy + 0.5;

	if ((nearest_x > src_udp->UdpL_X2 )||(nearest_y < src_udp->UdpL_Y1)
	    ||(nearest_x < src_udp->UdpL_X1)||(nearest_y > src_udp->UdpL_Y2))
		*dstdata++ = fill_mask;
	else
    	    *dstdata++ = *(srcdata + (nearest_y * sbyte_stride) + nearest_x);
        }
    } 
return (IpsX_SUCCESS);
}/* end _IpsRotateNearestNeighborByte */

/******************************************************************************
**  _IpsGetRotatedUdp()
**
**  FUNCTIONAL DESCRIPTION:
**	Get the new size and the x and y translations for Nearest Neighbor
**      and Interpolation rotations.
**
**  FORMAL PARAMETERS:
**
**	src_udp	    source UDP
**	dst_udp	    destination UDP
**	angle_ptr   pointer to angle of rotation
**
**  IMPLICIT INPUTS: none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE: return status
**  SIGNAL CODES: none.
**  SIDE EFFECTS: none
**
**
**  This function determines the resultant dst udp dimensions by mapping
**  the source vertices to the new dst vertices.
**
**
**  (0,num_of_scanlines-1) +-------+ (num_of_pxl-1/line,num_of_scanlines-1)
**			   |       |
**			   |       |
**			   |       |
**		     (0,0) +-------+ (num_of_pxl/line-1,0)
**
**  Mapping converts these vertices to:
**
**	new_x = xcos(u) - ysin(u)
**	new_y = xsin(u) + ycos(u)
**
**  New dimensions are the largest difference in these vertices (+1).
**  This routine also returns the xmin and ymin values which is used
**  adjust the obtained src coordinates so that it maps into the correct 
**  source pixel.
**
**
******************************************************************************/

void _IpsGetRotatedUdp(src_udp, dst_udp, angle,xmin,ymin)
struct UDP *src_udp;                    /* Working src UDP descriptor */
struct UDP *dst_udp;                    /* Working dst UDP descriptor */
float  	 angle;				/* Degrees of rotation	      */
float    *xmin,*ymin;			/* Min x and y values of dst  */
{
double   	sina, cosa;		/* sin and cos of the angle    */
float   	xmax, ymax;		/* max x  and y values of dst  */
double 		angle_in_radians;	/* angle in degrees * rad/deg  */
double		new_x_size, new_y_size; /* new destination x and y size*/
float		xvert[4],yvert[4];	/* array for vertices x and y  */
unsigned short	index;			/* scratch index counter       */

angle_in_radians = angle * RADIANS_PER_DEGREE;
sina = sin( angle_in_radians);
cosa = cos( angle_in_radians);

/* first, map source vertices to dst (x-> x', y-> y') */
/* (x1,y1) */
xvert[0] = 0.0;
yvert[0] = 0.0;
/* (x1,y2) */
xvert[1] = ((src_udp->UdpL_ScnCnt - 1) * sina) * -1.0;
yvert[1] = ((src_udp->UdpL_ScnCnt - 1) * cosa);
/* (x2,y1) */
xvert[2] = ((src_udp->UdpL_PxlPerScn - 1) * cosa);
yvert[2] = ((src_udp->UdpL_PxlPerScn - 1) * sina);
/* (x2,y2) */
xvert[3] = (((src_udp->UdpL_PxlPerScn - 1)* cosa) - 
    ((src_udp->UdpL_ScnCnt - 1) * sina));
yvert[3] = (((src_udp->UdpL_PxlPerScn - 1) * sina) + 
    ((src_udp->UdpL_ScnCnt - 1) * cosa));

/* calculate dst x and y sizes based on new x and y coordinates */
xmax = *xmin = xvert[0];
ymax = *ymin = yvert[0];

for (index = 1; index < 4; index++)
    {
    if (xvert[index] > xmax) xmax = xvert[index];
    if (xvert[index] < *xmin) *xmin = xvert[index];
    if (yvert[index] > ymax) ymax = yvert[index];
    if (yvert[index] < *ymin) *ymin  = yvert[index];
    }
new_x_size = ceil (xmax - *xmin + 1);
new_y_size = ceil (ymax - *ymin + 1);
dst_udp->UdpL_X1 = 0;
dst_udp->UdpL_Y1 = 0;
dst_udp->UdpL_X2 = new_x_size - 1;
dst_udp->UdpL_Y2 = new_y_size - 1;
dst_udp->UdpL_ScnCnt = new_y_size ;
dst_udp->UdpL_PxlPerScn = new_x_size;
dst_udp->UdpL_ScnStride = dst_udp->UdpL_PxlPerScn * dst_udp->UdpL_PxlStride;

} /* end _IpsGetRotatedUdp */
