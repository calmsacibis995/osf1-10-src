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
**  _IpsFill
**
**  FACILITY:
**
**      Image Processing Services
**
**  ABSTRACT:
**
**      This module contains the routine that dispatches to image-type
**      specific functions for region filling with border counting as outside.
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
**      July 24, 1990
**
*****************************************************************************/

/*
**  Include files
*/
#include <IpsDef.h>
#include <IpsMemoryTable.h>                 /* IPS Memory Mgt Functions      */
#include <IpsMacros.h>                         /* Common macro definitions   */
#include <IpsStatusCodes.h>                    /* Status Codes               */
#ifndef NODAS_PROTO
#include <ipsprot.h>			    /* Ips prototypes */
#endif

/* subroutines */
#ifdef NODAS_PROTO
long filter_triplet ();
#endif

/*
**  External References:
*/
#ifdef NODAS_PROTO
long _IpsBuildDstUdp();                                   /* from udp_utils */
long _IpsLogicalBitonal();                   /* from ips__logical_bitonal.c */
long _IpsMinRectUdp();                             /* from ips__udp_utils.c */
#endif

#define GET_BIT_(base,offset) \
   (READ32_(base + (offset >> 3)) >> (offset & 0x7) & 1)
 
#define PUT_ZERO_BIT_(base,offset) \
     PUT_BIT_VALUE_(base,offset,GET_BIT_(base,offset))

#define IpsM_ExcludeBorder 2

/************************************************************* 
**
**  ABSTRACT:
**  The algorithm will work on three scanlines at a time.
**  Top, bottom left and right edges are handled as special.
**
**  On the first pass, the routine defines everything in the fill plane as
**  inside (1) and selectively declares what is outside (0) based on the 
**  corresponding values in the border plane.  The 1st pass check on the 
**  top, bottom, left and right edges assure us in defining a start on 
**  what is "outside". Subsequent triplet and frame processing will further
**  define the outside until no changes are noted.
**
**************************************************************/

_IpsFill (src_udp, dst_udp, flags)
struct UDP *src_udp;                    /* Working src UDP descriptor     */
struct UDP *dst_udp;                    /* Working dst UDP descriptor     */
long       flags;			/* Indicates if borders included */
{
unsigned char *border_plane;		/* pointer to the border plane data */
unsigned char *fill_plane;		/* pointer to fill plane data       */
long i,j;				/* loop indices */
long bit;
long status = 0;
long ydir = TRUE;			/* y axis direction */
long continue_flag = TRUE;
long frame_change_flag = FALSE;
unsigned long	d_left_offset, d_right_offset, d_bottom_offset;
unsigned long	s_left_offset, s_right_offset, s_bottom_offset;
long s_scanline_length;
long d_scanline_length;
long number_of_lines;
long src_udp_x1;
long src_udp_x2;
long src_udp_y1;
long src_udp_y2;
long src_udp_scncnt;
long src_udp_pxlperscn;
long src_udp_pos;
long dst_udp_x1;
long dst_udp_x2;
long dst_udp_y1;
long dst_udp_y2;
long dst_udp_scncnt;
long dst_udp_pxlperscn;
long dst_udp_pos;
long dst_udp_scnstr;
long bits;

/* First build the destination and allocate memory if needed */
status = _IpsBuildDstUdp (src_udp, dst_udp, 0, flags, 0);
if (status != IpsX_SUCCESS)
    return (status);

/* save src dimensions */
src_udp_x1 = src_udp->UdpL_X1;
src_udp_x2 = src_udp->UdpL_X2;
src_udp_y1 = src_udp->UdpL_Y1;
src_udp_y2 = src_udp->UdpL_Y2;
src_udp_scncnt = src_udp->UdpL_ScnCnt;
src_udp_pxlperscn = src_udp->UdpL_PxlPerScn;
src_udp_pos = src_udp->UdpL_Pos;

/* save dst dimensions, could be different from src if Retaining */
dst_udp_x1 = dst_udp->UdpL_X1;
dst_udp_x2 = dst_udp->UdpL_X2;
dst_udp_y1 = dst_udp->UdpL_Y1;
dst_udp_y2 = dst_udp->UdpL_Y2;
dst_udp_scnstr =  dst_udp->UdpL_ScnStride;
dst_udp_scncnt = dst_udp->UdpL_ScnCnt;
dst_udp_pxlperscn = dst_udp->UdpL_PxlPerScn;
dst_udp_pos = dst_udp->UdpL_Pos;

/* adjust for min rectangle */
status = _IpsMinRectUdp (src_udp);
if (status != IpsX_SUCCESS)
   return (status);

/* Readjust dst dimensions to reflect the new min rectangle for processing */ 

dst_udp->UdpL_PxlPerScn = src_udp->UdpL_PxlPerScn;
dst_udp->UdpL_ScnCnt = src_udp->UdpL_ScnCnt;
dst_udp->UdpL_X1 = src_udp->UdpL_X1;
dst_udp->UdpL_Y1 = src_udp->UdpL_Y1;
dst_udp->UdpL_X2 = src_udp->UdpL_X2;
dst_udp->UdpL_Y2 = src_udp->UdpL_Y2;
 
/* Fill the area of interest with 1's */
status = _IpsLogicalBitonal (dst_udp, dst_udp, dst_udp, 0,IpsK_SetSrc1);
if (status != IpsX_SUCCESS)
    return (status);

border_plane = src_udp->UdpA_Base + ((src_udp->UdpL_Pos + 
				    (src_udp->UdpL_ScnStride * 
				    src_udp->UdpL_Y1)) >> 3);
fill_plane = dst_udp->UdpA_Base + ((dst_udp->UdpL_Pos +
				(dst_udp->UdpL_ScnStride * 
				    dst_udp->UdpL_Y1)) >>3);

number_of_lines = dst_udp->UdpL_ScnCnt;

/* first, handle edges */
/* TOP line */
for (i = dst_udp->UdpL_X1; i <= dst_udp->UdpL_X2; i++)
    {
    bit = (long)GET_BIT_(border_plane, i);
    if (bit == TRUE) /* border point? */
        {
	if (flags == IpsM_ExcludeBorder) /* borders excluded? */
	    PUT_ZERO_BIT_( fill_plane, i);
	}
    else
        {
        PUT_ZERO_BIT_( fill_plane, i);
    	bit = (long)GET_BIT_(border_plane, (src_udp->UdpL_ScnStride + i));
        if (bit == FALSE)
	    PUT_ZERO_BIT_( fill_plane,(dst_udp->UdpL_ScnStride + i));
	}
    }/* end loop to check top edge for border points */

/* do leftmost and rightmost edges */
d_left_offset = dst_udp->UdpL_X1;
d_right_offset = dst_udp->UdpL_X2;

s_left_offset = src_udp->UdpL_X1;
s_right_offset = src_udp->UdpL_X2;

for (j=0; j < number_of_lines; j++) 
    {
    /* leftmost bit */
    bit = (long)GET_BIT_(border_plane, s_left_offset);  
    if (bit == TRUE) /* border point? */
        {
        if (flags == IpsM_ExcludeBorder) /* borders excluded? */
            PUT_ZERO_BIT_( fill_plane, d_left_offset);
        }
    else
        {
        PUT_ZERO_BIT_( fill_plane, d_left_offset);
    	bit = (long)GET_BIT_(border_plane, (s_left_offset + 1));
        if (bit == FALSE)
	    PUT_ZERO_BIT_( fill_plane, (d_left_offset + 1));
	}
    /* rightmost bit */
    bit = (long)GET_BIT_(border_plane, s_right_offset);  
    if (bit == TRUE) /* border point? */
        {
        if (flags == IpsM_ExcludeBorder) /* borders excluded? */
            PUT_ZERO_BIT_( fill_plane, d_right_offset);
        }
    else
        {
        PUT_ZERO_BIT_( fill_plane, d_right_offset);
    	bit = (long)GET_BIT_(border_plane, (s_right_offset-1));
        if (bit == FALSE)
	    PUT_ZERO_BIT_( fill_plane,(d_right_offset-1));
	}

    d_left_offset += dst_udp->UdpL_ScnStride;
    d_right_offset += dst_udp->UdpL_ScnStride;
    s_left_offset += src_udp->UdpL_ScnStride;
    s_right_offset += src_udp->UdpL_ScnStride;
    }/* end left and right edge for loop */

/* Bottom line: */
/* point to the second to last scanline */

d_bottom_offset = (number_of_lines - 1) * dst_udp->UdpL_ScnStride;
s_bottom_offset = (number_of_lines - 1) * src_udp->UdpL_ScnStride;

for (i = dst_udp->UdpL_X1; i <= dst_udp->UdpL_X2; i++)
    {
    /* get bit from last scanline */
    bit = (long)GET_BIT_(border_plane, s_bottom_offset+i);
    if (bit == TRUE) /* border point? */
        {
	if (flags == IpsM_ExcludeBorder) /* borders excluded? */
	    PUT_ZERO_BIT_( fill_plane, d_bottom_offset+i);
	}
    else
        {
        PUT_ZERO_BIT_( fill_plane, (d_bottom_offset+i));
	/* get bit from second to last scanline (one up from bottom) */
    	bit = (long)GET_BIT_(border_plane, 
		    (s_bottom_offset - src_udp->UdpL_ScnStride + i));
        if (bit == FALSE)
	    PUT_ZERO_BIT_( fill_plane, i);
	}
    }/* end bottom edge for loop */

border_plane = src_udp->UdpA_Base + ((src_udp->UdpL_Pos + 
				    (src_udp->UdpL_ScnStride * 
				    src_udp->UdpL_Y1)) >> 3);

fill_plane = dst_udp->UdpA_Base + ((dst_udp->UdpL_Pos +
				(dst_udp->UdpL_ScnStride * 
				    dst_udp->UdpL_Y1)) >> 3);

d_scanline_length = (dst_udp->UdpL_ScnStride >> 3);
s_scanline_length = (src_udp->UdpL_ScnStride >> 3);

while  (continue_flag == TRUE)
    {
    for (j=1; j < (number_of_lines-2); j++) 
        {
        status = filter_triplet(fill_plane,
				dst_udp->UdpL_X1,
				dst_udp->UdpL_X2,
                                src_udp->UdpL_ScnStride,
                                dst_udp->UdpL_ScnStride,
                                 border_plane,&frame_change_flag,
                                flags);
 
        if (ydir < 0)
	    {
	    border_plane -= s_scanline_length;
	    fill_plane -= d_scanline_length;
	    }
        else
	    {
            border_plane += s_scanline_length;
	    fill_plane += d_scanline_length;
	    }
        } /* end for */
    if (frame_change_flag == TRUE)
	{
	ydir *= -1;
	frame_change_flag = FALSE;
	}
    else 
        continue_flag = FALSE;        
    } /* end while */
src_udp->UdpL_X1 = src_udp_x1;
src_udp->UdpL_X2 = src_udp_x2;
src_udp->UdpL_Y1 = src_udp_y1;
src_udp->UdpL_Y2 = src_udp_y2;
src_udp->UdpL_ScnCnt = src_udp_scncnt;
src_udp->UdpL_PxlPerScn = src_udp_pxlperscn;
src_udp->UdpL_Pos = src_udp_pos;

dst_udp->UdpL_X1 = dst_udp_x1;
dst_udp->UdpL_X2 = dst_udp_x2;
dst_udp->UdpL_Y1 = dst_udp_y1;
dst_udp->UdpL_Y2 = dst_udp_y2;
dst_udp->UdpL_ScnCnt = dst_udp_scncnt;
dst_udp->UdpL_PxlPerScn = dst_udp_pxlperscn;
dst_udp->UdpL_Pos = dst_udp_pos;
dst_udp->UdpL_ScnStride = dst_udp_scnstr;
return (IpsX_SUCCESS);
}

/******************************************************************
** filter_triplet
**
**  Having gone through the edges we have now started to define the
**  outside (the zeroes) in the fill plane.  We must now propagate
**  the outside. 
**
**  This routine works with each pixel within the edge of an image.
**  It goes from left to right and if any changes were noted, 
**  right to left.
**
**  The neighbors of each pixel are checked to see if they are border
**  points in the source border plane. Neighbors which are not border
**  points are zeroed if the current pixel of interest is not a border
**  point. If any are border points, they are left alone because they
**  are considered inside. 
** 
**          For each pixel in fill plane
**                Are you a border point?
**                        Yes - 0 fill plane (iff borders are excluded)
**                        No  - Are you outside (i.e. fill plane pixel = 0) ?
**                                Yes - zero neighbors if and only if they are
**                                      not border points (note changes)
**                                No  - Do nothing
**
**  The flag IpsM_ExcludeBorder allows for border points to be zeroed.
**
**			 |  top   |
**		    ______________________
**		    left |        | right
**                  ______________________
**                       |bottom  |
** 
***********************************************************************/    
long filter_triplet(fill_plane, d_x1, d_x2,
		s_scanline_stride, d_scanline_stride, border_plane, 
		    frame_change_flag, flags)
unsigned char *fill_plane;
long d_x1;
long d_x2;
unsigned long s_scanline_stride;
unsigned long d_scanline_stride;
unsigned char *border_plane;
long *frame_change_flag;
long flags;
{
long i, j;
long local_change_flag = FALSE;
long border_bit;
long top_bit, bottom_bit, right_bit, left_bit;
long d_bottom_bit_offset, d_right_bit_offset, d_left_bit_offset, d_center_bit_offset;
long s_bottom_bit_offset, s_right_bit_offset, s_left_bit_offset, s_center_bit_offset;

s_center_bit_offset = s_scanline_stride;
s_bottom_bit_offset = 2 * s_scanline_stride;
s_right_bit_offset = s_scanline_stride + 1;
s_left_bit_offset = s_scanline_stride - 1;

d_center_bit_offset = d_scanline_stride;
d_bottom_bit_offset = 2 * d_scanline_stride;
d_right_bit_offset = d_scanline_stride + 1;
d_left_bit_offset = d_scanline_stride - 1;

/* first loop, xdir is positive */
/* left to right processing */
for (i= (d_x1+1); i <=  (d_x2 - 1); i++)
    {
    border_bit = (long)GET_BIT_( border_plane, (s_scanline_stride + i));	
    if (border_bit == 1) 
	{
	/* border inclusion? */
	if (flags == IpsM_ExcludeBorder) /* borders excluded? */
	    PUT_ZERO_BIT_( fill_plane, (d_center_bit_offset + i));
        }
    else
	{
	/* For neighbors at top, bottom,
	** that are not border points clear pixel
	** and for neighbors at left and right
	** that are not border points, clear pixel and set
	** local change flags if any neighbors changed 
	*/
	if (((long)GET_BIT_( fill_plane, (d_center_bit_offset+i))) == FALSE) 
	    {
	    /* top */
	    top_bit = (long)GET_BIT_( fill_plane, i);	
	    border_bit = (long)GET_BIT_( border_plane, i);	
	    if ((border_bit != 1) && (top_bit == 1))
	 	{
		local_change_flag = 1;
                PUT_ZERO_BIT_( fill_plane, i);
		}

	    /* bottom */	            
	    bottom_bit = (long)GET_BIT_( fill_plane, (i+d_bottom_bit_offset));	
	    border_bit = (long)GET_BIT_( border_plane, (i+s_bottom_bit_offset));	
	    if ((border_bit != 1) && (bottom_bit == 1))
	        {
	        local_change_flag = 1;
                PUT_ZERO_BIT_( fill_plane, (i + d_bottom_bit_offset));
	        }

	    /* right */    
	    right_bit = (long)GET_BIT_( fill_plane, (d_right_bit_offset + i));	
	    border_bit = (long)GET_BIT_( border_plane, (s_right_bit_offset + i));	
  	    if ((border_bit != 1) && (right_bit == 1))
	        {
	        local_change_flag = 1;
                PUT_ZERO_BIT_( fill_plane, (d_right_bit_offset + i));
		}
	            
	    /* left */
	    left_bit = (long)GET_BIT_( fill_plane, (d_left_bit_offset+i));	
	    border_bit = (long)GET_BIT_( border_plane, (s_left_bit_offset+i));	
	    if ((border_bit != 1) && (left_bit == 1))
		{
		local_change_flag = 1;
                PUT_ZERO_BIT_( fill_plane, (d_left_bit_offset+i));
		}
	    } /* end of not a border point in fill plane, check neighbors */
	}/* end of else (not a borderpoint) */
    }/* end for */
if (local_change_flag == TRUE)
    {
    *frame_change_flag = TRUE;
    /* right to left processing */
    for (i=(d_x2 - 1); i >= (d_x1 + 1); i--)
        {
        border_bit = (long)GET_BIT_( border_plane, s_center_bit_offset+i);	
        if (border_bit == 1)
	    {
	    /* border inclusion? */
	    if (flags == IpsM_ExcludeBorder) /* borders excluded? */
	        PUT_ZERO_BIT_( fill_plane, (d_center_bit_offset + i));
	    }
        else
	    {
	    if (((long)GET_BIT_( fill_plane, d_center_bit_offset + i)) == FALSE)
	        {
	        /* top */
		top_bit = (long)GET_BIT_( fill_plane, i);
	        border_bit = (long)GET_BIT_( border_plane, i);
		if ((border_bit != 1) && (top_bit == 1))
                    PUT_ZERO_BIT_( fill_plane, i);

		/* bottom */
		bottom_bit = (long)GET_BIT_( fill_plane, 
					    (i+d_bottom_bit_offset));
	        border_bit = (long)GET_BIT_( border_plane, 
					    (i+s_bottom_bit_offset));

		if ((border_bit != 1) && (bottom_bit == 1))
                    PUT_ZERO_BIT_( fill_plane, (i+d_bottom_bit_offset));

	        /* right */
		right_bit = (long)GET_BIT_( fill_plane, 
					    (d_right_bit_offset+i));
	        border_bit = (long)GET_BIT_( border_plane, 
					     (s_right_bit_offset+i));
		if ((border_bit != 1) && (right_bit == 1))
                    PUT_ZERO_BIT_( fill_plane, (d_right_bit_offset+i));

		/* left */
		left_bit = (long)GET_BIT_( fill_plane, 
					     (d_left_bit_offset+i));
	        border_bit = (long)GET_BIT_( border_plane, 
					     (s_left_bit_offset+i));
	        if ((border_bit != 1) && (left_bit == 1))
                    PUT_ZERO_BIT_( fill_plane, (d_left_bit_offset+i));
	        }/* end if fill_plane bit is 0 */
  	    }/* end else */
        }/* end for loop for right to left processing */
    }/* end if local_change_frame test */
return (IpsX_SUCCESS);
}
