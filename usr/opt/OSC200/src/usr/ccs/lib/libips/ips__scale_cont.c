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
**      This code represents the IPS entries for scaling greyscale or
**	color images.  This code will magnify or reduce an image using 
**	one of two methods: bilinear interpolation or sampling.
**
**  ENVIRONMENT:
**
**      VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
**      Richard Piccolo, Digital Equipment Corp.
**	Joe Mauro, Digital Equipment Corp.
**
**  CREATION DATE:
**
**      July, 1989
**
*******************************************************************************/

/*
**  Table of contents:
*/

#ifdef NODAS_PROTO
long	_IpsScaleInterpolation();     	  /* scale using bilinear interp.     */
long	_IpsScaleNearestNeighbor();	  /* scale using sampling             */

    /* Data Class & Type Sensitive Entries */

long	_IpsScaleInterpolationByte();     /* scale bytes using bilin interp. */
long	_IpsScaleNearestNeighborByte();	  /* scale bytes using sampling      */
#endif

/*
**  Internal routines                                                         
*/

/*
**  Include files:
*/
#include <IpsDef.h>		    /* IPS Image Definitions	    */
#include <IpsStatusCodes.h>	    /* IPS Status Codes		    */
#include <IpsMemoryTable.h>	    /* IPS Memory Mgt. Functions    */
#ifndef NODAS_PROTO
#include <ipsprot.h>				    /* Ips prototypes */
#endif

/*
**  Macros
*/

/*  This macro horizontally interpolates a "row" of data into "buf_ptr"  */

#define HORIZONTAL_INTERPOLATION_(row,buf_ptr)\
    row_ptr = (unsigned char *)(src_row_ptr+((row) * sbyte_stride));\
    for (z = 0, ix = 0; ix < dst_udp->UdpL_PxlPerScn-1; ix++) \
	{\
	px = z;\
	cx = 1.0 - (z-(float)px);\
	*buf_ptr++ = (float)(*(row_ptr+px) * cx + \
	    (float)(*(row_ptr+px+1)) * (1.0 - cx));\
	z += x_scale;\
	}\
    buf_ptr -= (dst_udp->UdpL_PxlPerScn-1); /* keep edge information */\
    *(buf_ptr+dst_udp->UdpL_PxlPerScn-1) = *(row_ptr + src_udp->UdpL_PxlPerScn-1);

/*
**  Equated Symbols:
*/

/*
**  External References:
*/

#ifdef NODAS_PROTO
long _IpsVerifyNotInPlace();		    /* from Ips__udp_utils.c	    */
#endif

/*
**  External symbol definitions
*/

/*
**  Local Storage:
*/


/******************************************************************************
**
**  FUNCTIONAL DESCRIPTION:
**
**      Dispatch to scale routines which use bilinear interpolation
**
**  FORMAL PARAMETERS:
**
**	src_udp --> pointer to source      udp (initialized)
**	dst_udp --> pointer to destination udp (uninitialized)
**
**  IMPLICIT INPUTS:	none
**  IMPLICIT OUTPUTS:	none
**  FUNCTION VALUE:	status return
**  SIGNAL CODES:	none
**  SIDE EFFECTS:	none
**                                                                            
*******************************************************************************/
long _IpsScaleInterpolation(src_udp,dst_udp,fxscale,fyscale)
struct UDP *src_udp;
struct UDP *dst_udp;
float  fxscale,fyscale;
{
long	dst_npx;		/* destination number of pixels	    */
long	dst_nsl;		/* destination number of scan lines */
long	size;			/* buffer size (bits / bytes)	    */
long	status;
long	mem_alloc = 0;

/*
** validate source class and type
*/

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

/*
** Initialize destination UDP fixed fields & data buffer
*/

dst_npx = src_udp->UdpL_PxlPerScn * fxscale;
dst_nsl = src_udp->UdpL_ScnCnt * fyscale;

dst_udp->UdpB_DType = src_udp->UdpB_DType;
dst_udp->UdpB_Class = src_udp->UdpB_Class;
dst_udp->UdpW_PixelLength = src_udp->UdpW_PixelLength;
dst_udp->UdpL_Levels = src_udp->UdpL_Levels;
dst_udp->UdpL_PxlStride = src_udp->UdpL_PxlStride;
dst_udp->UdpL_CompIdx = src_udp->UdpL_CompIdx;

dst_npx = src_udp->UdpL_PxlPerScn * fxscale;
dst_nsl = src_udp->UdpL_ScnCnt * fyscale;

dst_udp->UdpL_X1 = 0;
dst_udp->UdpL_Y1 = 0;
dst_udp->UdpL_X2 = dst_npx - 1;
dst_udp->UdpL_Y2 = dst_nsl - 1;
dst_udp->UdpL_ScnStride = dst_npx * dst_udp->UdpL_PxlStride;

dst_udp->UdpL_PxlPerScn = dst_npx;
dst_udp->UdpL_ScnCnt    = dst_nsl;
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
	    (((size + 7) >> 3), IpsM_InitMem, 0);
    if (!dst_udp->UdpA_Base) return (IpsX_INSVIRMEM);
    dst_udp->UdpL_ArSize = size;
    dst_udp->UdpL_Pos = 0;
    mem_alloc = 1;
    }

/*
** Dispatch to layer 2 interpolation scale functions
*/

switch (src_udp->UdpB_DType)
    {
    case UdpK_DTypeBU:
	 status = _IpsScaleInterpolationByte(src_udp,dst_udp); 
    break;

    case UdpK_DTypeWU:
    case UdpK_DTypeLU:
    case UdpK_DTypeF:
    case UdpK_DTypeV:
    case UdpK_DTypeVU:
    default: 
    break;
    }; 

if ((status != IpsX_SUCCESS) && (mem_alloc == 1))
    (*IpsA_MemoryTable[IpsK_FreeDataPlane]) (dst_udp->UdpA_Base);
return (status);
}

/******************************************************************************
**
**  FUNCTIONAL DESCRIPTION:
**
**      Dispatch to scale routines which use nearest neighbor
**
**  FORMAL PARAMETERS:
**
**	src_udp --> pointer to source      udp (initialized)
**	dst_udp --> pointer to destination udp (uninitialized)
**
**  IMPLICIT INPUTS:	none
**  IMPLICIT OUTPUTS:	none
**  FUNCTION VALUE:	status return
**  SIGNAL CODES:	none
**  SIDE EFFECTS:	none
**                                                                            
*******************************************************************************/
long _IpsScaleNearestNeighbor(src_udp,dst_udp,fxscale,fyscale)
struct UDP *src_udp;
struct UDP *dst_udp;
float  fxscale,fyscale;
{
long	dst_npx;		/* destination number of pixels	    */
long	dst_nsl;		/* destination number of scan lines */
long	size;			/* buffer size (bits / bytes)	    */
long	status;

/*
** validate source class and type
*/

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
    default: return(IpsX_UNSOPTION); break;
    };

/*
** Initialize destination UDP fixed fields & data buffer
*/

dst_npx = src_udp->UdpL_PxlPerScn * fxscale;
dst_nsl = src_udp->UdpL_ScnCnt * fyscale;

dst_udp->UdpB_DType = src_udp->UdpB_DType;
dst_udp->UdpB_Class = src_udp->UdpB_Class;
dst_udp->UdpW_PixelLength = src_udp->UdpW_PixelLength;
dst_udp->UdpL_Levels = src_udp->UdpL_Levels;
dst_udp->UdpL_PxlStride = src_udp->UdpL_PxlStride;
dst_udp->UdpL_CompIdx = src_udp->UdpL_CompIdx;

dst_npx = src_udp->UdpL_PxlPerScn * fxscale;
dst_nsl = src_udp->UdpL_ScnCnt * fyscale;

dst_udp->UdpL_X1 = 0;
dst_udp->UdpL_Y1 = 0;
dst_udp->UdpL_X2 = dst_npx - 1;
dst_udp->UdpL_Y2 = dst_nsl - 1;
dst_udp->UdpL_ScnStride = dst_npx * dst_udp->UdpL_PxlStride;

dst_udp->UdpL_PxlPerScn = dst_npx;
dst_udp->UdpL_ScnCnt    = dst_nsl;
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
	    (((size + 7) >> 3), IpsM_InitMem, 0);
    if (!dst_udp->UdpA_Base) return (IpsX_INSVIRMEM);
    dst_udp->UdpL_ArSize = size;
    dst_udp->UdpL_Pos = 0;
    }

/*
** Dispatch to layer 2 nearest neighbor scale functions
*/

switch (src_udp->UdpB_DType)
    {
    case UdpK_DTypeBU:
	 return(_IpsScaleNearestNeighborByte(src_udp,dst_udp)); 
    break;

    case UdpK_DTypeWU:
    case UdpK_DTypeLU:
    case UdpK_DTypeF:
    case UdpK_DTypeV:
    case UdpK_DTypeVU:
    default: 
    break;
    }; 
}

/******************************************************************************
**
**  FUNCTIONAL DESCRIPTION:
**
**      Scale a frame up/down using bilinear interpolation
**
**	  Given :
**		input x(u,v) u=0..width_in-1, v=0..height_in-1
**		scale_xfactor = (width_in - 1) / (width_out - 1)
**		scale_yfactor = (height_in - 1) / (height_out - 1)
**
**	  Calulate new output out(i,j) as follows:
**
**		For i = 0 to width_out - 1 , j = 0 to height_out - 1
**
**		out(i,j) = (out1 * cy) + (out2 * (1-cy))
**
**	  where:
**		out1 = x(px,py) * cx + x(px+1,py) * (1-cx)
**		out2 = x(px,py+1) * cx + x(px+1,py+1) * (1-cx)
**
**		px = int(i * scale_xfactor) 
**		cx = 1 - frac(i * scale_xfactor)
**		py = int(j * scale_yfactor)
**		cy = 1 - frac(i * scale_yfactor)
**
**  FORMAL PARAMETERS:
**
**	src_udp --> pointer to source      udp (initialized)
**	dst_udp --> pointer to destination udp (uninitialized)
**
**  IMPLICIT INPUTS:	none
**
**  IMPLICIT OUTPUTS:	none
**
**  FUNCTION VALUE:	status return
**
**  SIGNAL CODES:	none
**
**  SIDE EFFECTS:	none
**
**  RESTRICTIONS:                                                              
**
**	Operates on the ISL native format for BYTE_IMAGES only.
**	Operates on source images of m x n where m and n > 1
**	(UDP class == BU ; a pixel stride of one byte is assumed)
**                                                                            
*******************************************************************************/
long _IpsScaleInterpolationByte (src_udp, dst_udp)
struct UDP *src_udp;
struct UDP *dst_udp;
{
    float 	  *out_h_buf1;		  /* 1st of two ouput rows	      */
    float 	  *out_h_buf2;		  /* 2nd of two output rows	      */
    float	  *b_temp;		  /* temporary buffer pointer         */

    unsigned char *src_byte_ptr;	  /* pointers needed                  */
    unsigned char *dst_byte_ptr;
    unsigned char *row_ptr;
    unsigned char *src_row_ptr;

    unsigned long ix,iy;	  	  /* loop counters 		      */
    long 	  col_diff;		  /* diff from current to prev col    */
    long 	  px,py;		  /* row, column pointer	      */
    long	  old_py;		  /* saved col pointer		      */
    long	  sbyte_stride;		  /* src scan line stride in BYTES    */
    float	  cx,cy;		  /* horiz/vert coefficients	      */
    float	  z,zx,zy;		  /* computational scale factors      */
    double	  x_scale;		  /* x scale factor		      */
    double	  y_scale;		  /* y scale factor                   */

    if (dst_udp->UdpL_PxlPerScn == 1) 
	x_scale = 0.0;
    else 
	{
	x_scale  = (src_udp->UdpL_PxlPerScn - 1);
	x_scale /= (dst_udp->UdpL_PxlPerScn - 1);
	}
    if (dst_udp->UdpL_ScnCnt == 1)
	y_scale = 0.0;
    else 
	{
	y_scale  = (src_udp->UdpL_ScnCnt - 1);
	y_scale /= (dst_udp->UdpL_ScnCnt - 1);
	}

    sbyte_stride = (src_udp->UdpL_ScnStride >> 3);

    src_row_ptr  = (unsigned char *)src_udp->UdpA_Base + (src_udp->UdpL_Pos>>3)
	+ (src_udp->UdpL_ScnStride>>3) * src_udp->UdpL_Y1 + src_udp->UdpL_X1;
    dst_byte_ptr = (unsigned char *)dst_udp->UdpA_Base + (dst_udp->UdpL_Pos>>3);

    /*  allocate buffers for 2 output lines for horizontal interpolation      */
    /*  and one buffer for the input row 			              */

    out_h_buf1 = (float *)(*IpsA_MemoryTable[IpsK_Alloc])
			    (sizeof(float)*(2*dst_udp->UdpL_PxlPerScn),0,0);
    if (!out_h_buf1)
	return (IpsX_INSVIRMEM);
    out_h_buf2 = out_h_buf1 + dst_udp->UdpL_PxlPerScn;

    /* for each output y  */

    for (old_py = -2,zy=0,iy = 0; iy < dst_udp->UdpL_ScnCnt - 1; iy++) 
	{
	py = zy;				/* integerize fact.	      */
	cy = 1.0 - (zy-(float)py);		/* calc coefficient	      */
	col_diff = py - old_py;			/* calc col		      */
	old_py = py;				/* difference reset	      */

	if (col_diff > 1)  /* do we have to interpolate horizontally again */
	    {
	    /* interpolate 2 new rows */

	    HORIZONTAL_INTERPOLATION_(py,out_h_buf1);
	    HORIZONTAL_INTERPOLATION_(py+1,out_h_buf2);
	    }

	else if (col_diff == 1)
	    {
	    /* swap buffers and interpolate next new line */

	    b_temp = out_h_buf2;
	    out_h_buf2 = out_h_buf1;
	    out_h_buf1 = b_temp;

	    /* interpolate one new row */

	    HORIZONTAL_INTERPOLATION_(py+1,out_h_buf2);
	    }

	/* interpolate vertically */

	for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
	    *dst_byte_ptr++ = 
		(*out_h_buf1++ * cy + *out_h_buf2++ * (1.0-cy))+ 0.5;

	/* update scale factor and reset output buffer pointers */

	zy += y_scale;			      /* move scale factor	      */
	out_h_buf1 -= dst_udp->UdpL_PxlPerScn; /* back off pointers	      */
	out_h_buf2 -= dst_udp->UdpL_PxlPerScn;	 
	}				      /* end of iy loop		      */

    /* finish last row keeping edge information */

    HORIZONTAL_INTERPOLATION_(src_udp->UdpL_ScnCnt-1,out_h_buf1);
    for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
	*dst_byte_ptr++ = *out_h_buf1++; 

    out_h_buf1 -= dst_udp->UdpL_PxlPerScn;

    /* free allocated memory */

    if (out_h_buf2 > out_h_buf1) 
	(*IpsA_MemoryTable[IpsK_Dealloc])(out_h_buf1);
    else 
	(*IpsA_MemoryTable[IpsK_Dealloc])(out_h_buf2);
    return (IpsX_SUCCESS);
}


/******************************************************************************
**
**  FUNCTIONAL DESCRIPTION:
**
**      Scale a frame up/down using nearest neighbor approach.
**
**	  Given :
**		input x(u,v) u=0..width_in-1, v=0..height_in-1
**		scale_xfactor = (width_in - 1) / (width_out - 1)
**		scale_yfactor = (height_in - 1) / (height_out - 1)
**
**	  Calulate new output out(i,j) as follows:
**
**		For i = 0 to width_out - 1 , j = 0 to height_out - 1
**
**		out(i,j) = (out1 * cy) + (out2 * (1-cy))
**
**	  where:
**		out1 = x(px,py) * cx + x(px+1,py) * (1-cx)
**		out2 = x(px,py+1) * cx + x(px+1,py+1) * (1-cx)
**
**		px = int(i * scale_xfactor) 
**		cx = 1 - frac(i * scale_xfactor)
**		py = int(j * scale_yfactor)
**		cy = 1 - frac(i * scale_yfactor)
**
**  FORMAL PARAMETERS:
**
**	src_udp --> pointer to source      udp (initialized)
**	dst_udp --> pointer to destination udp (uninitialized)
**
**  IMPLICIT INPUTS:	none
**
**  IMPLICIT OUTPUTS:	none
**
**  FUNCTION VALUE:	status return
**
**  SIGNAL CODES:	none
**
**  SIDE EFFECTS:	none
**
**  RESTRICTIONS:                                                              
**
**	Operates on the ISL native format for BYTE_IMAGES only.
**	(UDP class == BU ; a pixel stride of one byte is assumed)
**                                                                            
*******************************************************************************/
long _IpsScaleNearestNeighborByte (src_udp, dst_udp)
struct UDP *src_udp;
struct UDP *dst_udp;
{
    unsigned char *dst_byte_ptr;	  /* used for dst image		      */
    unsigned      char *row_ptr;	  /* dynamc row pointer		      */
    unsigned      char *src_row_ptr;	  /* start of src data		      */
    unsigned long ix,iy;	  	  /* loop counters 		      */
    long	  sbyte_stride;		  /* src scan line stride in BYTES    */
    long	  px,py;		  /* row, column pointer	      */
    float	  zx,zy;		  /* computational scale factors      */
    double	  x_scale;		  /* x scale factor		      */
    double	  y_scale;		  /* y scale factor                   */

    if (dst_udp->UdpL_PxlPerScn == 1) 
	x_scale = 0.0;
    else 
	{
	x_scale  = (src_udp->UdpL_PxlPerScn - 1);
	x_scale /= (dst_udp->UdpL_PxlPerScn - 1);
	}
    if (dst_udp->UdpL_ScnCnt == 1)
	y_scale = 0.0;
    else 
	{
	y_scale  = (src_udp->UdpL_ScnCnt - 1);
	y_scale /= (dst_udp->UdpL_ScnCnt - 1);
	}

    sbyte_stride = (src_udp->UdpL_ScnStride >> 3);

    src_row_ptr  = (unsigned char *)src_udp->UdpA_Base + (src_udp->UdpL_Pos>>3)
	+ (src_udp->UdpL_ScnStride>>3) * src_udp->UdpL_Y1 + src_udp->UdpL_X1;
    dst_byte_ptr = (unsigned char *)dst_udp->UdpA_Base + (dst_udp->UdpL_Pos>>3);

    for (zy = 0,iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)/* for each output y  */
	{
	py = zy + 0.5;					/* integerize fact.   */

	/* get src row pointer */

	row_ptr = (unsigned char *)(src_row_ptr + (py * sbyte_stride));

	for (zx = 0,ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
	    {
	    px = zx + 0.5;			    /* integerize and round */
	    *dst_byte_ptr++ = *(row_ptr + px);
	    zx += x_scale;
	    }
	zy += y_scale;				  /* update scale factor      */
	}					  /* end of iy loop	      */

    return (IpsX_SUCCESS);
}
