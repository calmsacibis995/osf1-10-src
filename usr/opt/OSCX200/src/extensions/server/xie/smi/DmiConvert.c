/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */

/******************************************************************************
**
**  Copyright (c) Digital Equipment Corporation, 1990 All Rights Reserved.
**  Unpublished rights reserved under the copyright laws of the United States.
**  The software contained on this media is proprietary to and embodies the
**  confidential technology of Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and media is authorized only
**  pursuant to a valid written license from Digital Equipment Corporation.

**  RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the U.S.
**  Government is subject to restrictions as set forth in Subparagraph
**  (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
**
**  Derived from work bearing the following copyright and permissions:
**
**  Copyright 1989 by Digital Equipment Corporation, Maynard, Massachusetts,
**  and the Massachusetts Institute of Technology, Cambridge, Massachusetts.
**
**                     All Rights Reserved
**
**  Permission to use, copy, modify, and distribute this software and its 
**  documentation for any purpose and without fee is hereby granted, 
**  provided that the above copyright notice appear in all copies and that
**  both that copyright notice and this permission notice appear in 
**  supporting documentation, and that the names of Digital or MIT not be
**  used in advertising or publicity pertaining to distribution of the
**  software without specific, written prior permission.  
**/

/*****************************************************************************
**
**  FACILITY:
**
**      X Imaging Extension
**      DEC Machine Independant DDX
**
**  ABSTRACT:
**
**      This module contains all image data type and organization conversion 
**	routines.
**
**  ENVIRONMENT:
**
**	VAX/VMS V5.3
**	ULTRIX  V4.0
**
**  AUTHOR(S):
**
**      John Weber
**      Gary Grebus
**
**  CREATION DATE:
**
**      May 15, 1989
**      Aug 16, 1990 (cloned from SmiConvert.c)
**
**  MODIFICATION HISTORY:
**
*****************************************************************************/

/*
**  Definition to let include files know who's calling.
*/
#define _DmiConvert

/*
**  Include files
*/
#include <stdio.h>
#include <X.h>
#include <XieDdx.h>                     /* XIE device dependant definitions */
#include <XieUdpDef.h>

/*
**  Table of contents
*/
int	    DmiConvert();
int	    DmiConvertCpp();
int	    DmiCopy();
int	    DmiCopyCpp();
int	    DmiGetTyp();
int         DmiPosCl();

static int _CvtUnsOpr();

static int _CvtBsBs();
static int _CvtBsCl();
static int _CvtBsByt();
static int _CvtBsWrd();
static int _CvtBsFlt();

static int _CvtBytByt();
static int _CvtBytBs();
static int _CvtBytCl();
static int _CvtBytWrd();
static int _CvtBytFlt();

static int _CvtWrdWrd();
static int _CvtWrdBs();
static int _CvtWrdCl();
static int _CvtWrdByt();
static int _CvtWrdFlt();

static int _CvtFltFlt();
static int _CvtFltBs();
static int _CvtFltByt();
static int _CvtFltWrd();

static int _CvtClCl();
static int _CvtClBs();
static int _CvtClByt();
static int _CvtClWrd();
static int _CvtClFlt();

static int _CvtBsBsCpp();
static int _CvtBsBytCpp();
static int _CvtBsWrdCpp();
static int _CvtBsFltCpp();

static int _CvtBytBytCpp();
static int _CvtBytBsCpp();
static int _CvtBytWrdCpp();
static int _CvtBytFltCpp();

static int _CvtWrdWrdCpp();
static int _CvtWrdBsCpp();
static int _CvtWrdBytCpp();
static int _CvtWrdFltCpp();

static int _CvtFltFltCpp();
static int _CvtFltBsCpp();
static int _CvtFltBytCpp();
static int _CvtFltWrdCpp();

static int _ConHCBs();
static int _ConHCByt();
static int _ConHCWrd();

static int _ConCSBs();
static int _ConCSByt();
static int _ConCSWrd();

static int _ConHSBs();
static int _ConHSByt();
static int _ConHSWrd();

static void _BuildClBits();
static void _BuildClBytes();
static void _BuildClWords();

/*
**  MACRO definitions
*/
#define Min_(a,b) ((a) <= (b) ? (a) : (b))
#define Max_(a,b) ((a) > (b) ? (a) : (b))

/*
**  Equated Symbols
*/

/*
**  External References
*/


/*
**	Local Storage
*/

/*
**  This table describes functions to be called when converting from one
**  descriptor type to another.
*/
static int (*conversion_table[XieK_DATA_TYPE_COUNT][XieK_DATA_TYPE_COUNT])()={
 {_CvtBsBs,  _CvtBsBs,  _CvtUnsOpr,_CvtUnsOpr,_CvtBsCl,  _CvtBsByt, _CvtBsFlt, _CvtBsWrd},
 {_CvtBsBs,  _CvtBsBs,  _CvtUnsOpr,_CvtUnsOpr,_CvtBsCl,  _CvtBsByt, _CvtBsFlt, _CvtBsWrd},
 {_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr},
 {_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr},
 {_CvtClBs,  _CvtClBs,  _CvtUnsOpr,_CvtUnsOpr,_CvtClCl,  _CvtClByt, _CvtClFlt, _CvtClWrd},
 {_CvtBytBs, _CvtBytBs, _CvtUnsOpr,_CvtUnsOpr,_CvtBytCl, _CvtBytByt,_CvtBytFlt,_CvtBytWrd},
 {_CvtFltBs, _CvtFltBs, _CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtFltByt,_CvtFltFlt,_CvtFltWrd},
 {_CvtWrdBs, _CvtWrdBs, _CvtUnsOpr,_CvtUnsOpr,_CvtWrdCl, _CvtWrdByt,_CvtWrdFlt,_CvtWrdWrd}
};

/* Ditto for conversions under the control of a CPP */
static int (*conversion_table_cpp[XieK_DATA_TYPE_COUNT][XieK_DATA_TYPE_COUNT])()={
 {_CvtBsBsCpp, _CvtBsBsCpp, _CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtBsBytCpp, _CvtBsFltCpp, _CvtBsWrdCpp},
 {_CvtBsBsCpp, _CvtBsBsCpp, _CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtBsBytCpp, _CvtBsFltCpp, _CvtBsWrdCpp},
 {_CvtUnsOpr,  _CvtUnsOpr,  _CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,   _CvtUnsOpr,   _CvtUnsOpr},
 {_CvtUnsOpr,  _CvtUnsOpr,  _CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,   _CvtUnsOpr,   _CvtUnsOpr},
 {_CvtUnsOpr,  _CvtUnsOpr,  _CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,   _CvtUnsOpr,   _CvtUnsOpr},
 {_CvtBytBsCpp,_CvtBytBsCpp,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtBytBytCpp,_CvtBytFltCpp,_CvtBytWrdCpp},
 {_CvtFltBsCpp,_CvtFltBsCpp,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtFltBytCpp,_CvtFltFltCpp,_CvtFltWrdCpp},
 {_CvtWrdBsCpp,_CvtWrdBsCpp,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtWrdBytCpp,_CvtWrdFltCpp,_CvtWrdWrdCpp}
};

/*
**  This table describes the functions to be called when constraining data.
*/
static int (*constrain_table[5][XieK_DATA_TYPE_COUNT])()={
 {_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr},
 {_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr},
 {_ConHCBs,  _ConHCBs,  _CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_ConHCByt, _CvtUnsOpr,_ConHCWrd},
 {_ConCSBs,  _ConCSBs,  _CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_ConCSByt, _CvtUnsOpr,_ConCSWrd},
 {_ConHSBs,  _ConHSBs,  _CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_ConHSByt, _CvtUnsOpr,_ConHSWrd}
};

/*****************************************************************************
**  DmiConvert
**
**  FUNCTIONAL DESCRIPTION:
**
**      This is the externally callable entry point for type conversion. The
**	data specified by the source descriptor is reorganized to conform to
**	the specification of the destination descriptor.
**
**  FORMAL PARAMETERS:
**
**      src	- source data descriptor
**	dst	- destination data descriptor
**	mode    - transfer mode, one of the following:
**		    XieK_MoveMode,   move without constraining
**		    XieK_HardClip,   constrain data via hard clip model
**                  XieK_ClipScale,  constrain data via clip scale model
**                  XieK_HardScale,  constrain data via hard scale model
**
**  FUNCTION VALUE:
**
*****************************************************************************/
int	    DmiConvert(src, dst, mode)
UdpPtr  src;
UdpPtr  dst;
int     mode;
{
    int srctyp;
    int dsttyp;
    int status;
    /*
    **	Validate and get type of descriptors
    */
    srctyp = DdxGetTyp_(src);
    dsttyp = DdxGetTyp_(dst);
    
    if( srctyp >= XieK_DATA_TYPE_COUNT || dsttyp >= XieK_DATA_TYPE_COUNT )
	status  = _CvtUnsOpr(src, dst);
    
    else if( mode == XieK_MoveMode )
	/*
	**  Call specific conversion routine.
	*/
	status = (*conversion_table[srctyp][dsttyp])(src,dst);
    
    else if( srctyp != XieK_AF )
	status = _CvtUnsOpr(src, dst);
    else
	/*
	**  The input data must be type float.  Call conversion
	**  routine based on the constraint model requested.
	*/
	status = (*constrain_table[mode][dsttyp])(src,dst);
    
    return (status);
}				    /* end DmiConvert */

/*****************************************************************************
**  DmiConvertCpp
**
**  FUNCTIONAL DESCRIPTION:
**
**      This is the same as DmiConvert, except that a CPP is used to
**	select the input pixels to be copied.
**
**  FORMAL PARAMETERS:
**
**      src	- source data descriptor
**	dst	- destination data descriptor
**	mode    - transfer mode, one of the following:
**		    XieK_MoveMode,   move without constraining
**      cpp     - cpp data descriptor
**
**  FUNCTION VALUE:
**
*****************************************************************************/
int	    DmiConvertCpp(src, dst, cpp, mode)
UdpPtr  src;
UdpPtr  dst;
UdpPtr  cpp;
int     mode;
{
    int srctyp;
    int dsttyp;
    int status;
    /*
    **	Validate and get type of descriptors
    */
    srctyp = DdxGetTyp_(src);
    dsttyp = DdxGetTyp_(dst);

    if( srctyp >= XieK_DATA_TYPE_COUNT
     || dsttyp >= XieK_DATA_TYPE_COUNT || mode != XieK_MoveMode || cpp == NULL )
	status = _CvtUnsOpr(src, dst);
    else
	status = (*conversion_table_cpp[srctyp][dsttyp])(src,dst,cpp);

    return (status); 
}				    /* end DmiConvertCpp */

/*****************************************************************************
**  DmiCopy
**
**  FUNCTIONAL DESCRIPTION:
**
**      This is a jacket function over the CONVERT routine which provides a
**	straight-forward interface for copying ROIs.
**
**	The principle used is simple, but possibly hard to understand. This
**	routine relies on the fact that the CONVERT code matches source and 
**	destination image coordinates during the move (i.e. source pixel (0,0)
**	is moved to destination pixel (0,0). To accomplish the move of a source
**	ROI, the source UDP is modified so that only the ROI data is described
**	as image (all other data will be described as pad by careful use of
**      POS, and STRIDE fields). The coordinates of the source are shifted so
**      they match the desired destination coordinates.
**
**	Simple...right? (no way)
**
**  FORMAL PARAMETERS:
**
**	srcudp - UDP to copy from
**      dstudp - UDP to copy to
**	src_x  - Source X coordinate (X coordinate of upper left corner of
**	         source ROI).
**	src_y  - Source Y coordinate (Y coordinate of upper left corner of
**	         source ROI).
**	dst_x  - Destination X coordinate
**	dst_y  - Destination Y coordinate
**      width  - Width of ROI in pels
**	height - Height of ROI in pels
**  
**
**  FUNCTION VALUE:
**
*****************************************************************************/
int		DmiCopy( src, dst, srcx, srcy, dstx, dsty, width, height )
UdpPtr		src;
UdpPtr		dst;
long int	srcx;
long int	srcy;
long int	dstx;
long int	dsty;
long int	width;
long int	height;
{
    UdpRec  srcudp;
    long int clipped_width;
    long int clipped_height;
    /*
    **	Make a copy of the source UDP that we can modify.
    */
    srcudp = *src;
    /*
    **	Modify the source UDP so that is describes only the ROI and reflects
    **	where in the destination the data is to be copied.
    */
    srcudp.UdpL_X1 = dstx;
    srcudp.UdpL_Y1 = dsty;

    clipped_width  = Min_(srcx + width  -1, src->UdpL_X2) - srcx + 1;
    clipped_height = Min_(srcy + height -1, src->UdpL_Y2) - srcy + 1;

    srcudp.UdpL_X2 = dstx + clipped_width  - 1;
    srcudp.UdpL_Y2 = dsty + clipped_height - 1;

    srcudp.UdpL_PxlPerScn = clipped_width;
    srcudp.UdpL_ScnCnt    = clipped_height;

    srcudp.UdpL_Pos += srcy * srcudp.UdpL_ScnStride
		     + srcx * srcudp.UdpL_PxlStride;

    /*
    **	Now, call the CONVERT routine to do the data movement.
    */
    return (DdxConvert_(&srcudp, dst, XieK_MoveMode) );
}				    /* end DmiCopy */

/*****************************************************************************
**  DmiCopyCpp
**
**  FUNCTIONAL DESCRIPTION:
**
**     This is the same as DmiCopy, except the pixels to be copied
**     are selected by a CPP.
**
**  FORMAL PARAMETERS:
**
**	srcudp - UDP to copy from
**      dstudp - UDP to copy to
**      cpp    - UDP for the CPP
**	src_x  - Source X coordinate (X coordinate of upper left corner of
**	         source ROI).
**	src_y  - Source Y coordinate (Y coordinate of upper left corner of
**	         source ROI).
**	dst_x  - Destination X coordinate
**	dst_y  - Destination Y coordinate
**      width  - Width of ROI in pels
**	height - Height of ROI in pels
**  
**
**  FUNCTION VALUE:
**
*****************************************************************************/
int		DmiCopyCpp(src, dst, cpp, srcx, srcy, dstx, dsty, width, height)
UdpPtr		src;
UdpPtr		dst;
long int	srcx;
long int	srcy;
long int	dstx;
long int	dsty;
long int	width;
long int	height;
{
    UdpRec  srcudp;
    long int clipped_width;
    long int clipped_height;

    /*
    **	Make a copy of the source UDP that we can modify.
    */
    srcudp = *src;
    /*
    **	Modify the source UDP so that is describes only the ROI and reflects
    **	where in the destination the data is to be copied.
    */
    srcudp.UdpL_X1 = dstx;
    srcudp.UdpL_Y1 = dsty;

    clipped_width  = Min_(srcx + width  -1, src->UdpL_X2) - srcx + 1;
    clipped_height = Min_(srcy + height -1, src->UdpL_Y2) - srcy + 1;

    srcudp.UdpL_X2 = dstx + clipped_width  - 1;
    srcudp.UdpL_Y2 = dsty + clipped_height - 1;

    srcudp.UdpL_PxlPerScn = clipped_width;
    srcudp.UdpL_ScnCnt    = clipped_height;

    srcudp.UdpL_Pos += srcy * srcudp.UdpL_ScnStride
		     + srcx * srcudp.UdpL_PxlStride;

    /*
    **	Now, call the CONVERT routine to do the data movement.
    */
    return( DdxConvertCpp_( &srcudp, dst, cpp, XieK_MoveMode ) );
}				    /* end DmiCopyCpp */

/*****************************************************************************
**  DmiGetTyp
**
**  FUNCTIONAL DESCRIPTION:
**
**      Return constant which identifies a valid combination of descriptor
**	class and data type.
**
**  FORMAL PARAMETERS:
**
**      desc - data descriptor to validate and classify
**
**  FUNCTION VALUE:
**
**      Chf_InvDscTyp - data type of descriptor
**
**  SIGNAL CODES:
**
**      invalid descriptor type
**
*****************************************************************************/
int	DmiGetTyp(desc)
UdpPtr	desc;
{
    int type;

    switch (desc->UdpB_DType)
	{
    case UdpK_DTypeBU :
	switch (desc->UdpB_Class)
	    {
	case UdpK_ClassA:   type = XieK_ABU;		 break;
	case UdpK_ClassUBA:
	case UdpK_ClassUBS:
	case UdpK_ClassCL:  type = XieK_DATA_TYPE_COUNT; break;
	default:	    type = XieK_DATA_TYPE_COUNT; break;
	    } break;
    case UdpK_DTypeWU:
	switch (desc->UdpB_Class)
	    {
	case UdpK_ClassA:   type = XieK_AWU;		 break;
	case UdpK_ClassUBA:
	case UdpK_ClassUBS:
	case UdpK_ClassCL:  type = XieK_DATA_TYPE_COUNT; break;
	default:	    type = XieK_DATA_TYPE_COUNT; break;
	    } break;
    case UdpK_DTypeLU:
	switch (desc->UdpB_Class)
	    {
	case UdpK_ClassA:   type = XieK_UBAV; /* hack */ break;
	case UdpK_ClassUBA:
	case UdpK_ClassUBS:
	case UdpK_ClassCL:  type = XieK_DATA_TYPE_COUNT; break;
	default:	    type = XieK_DATA_TYPE_COUNT; break;
	    } break;
    case UdpK_DTypeF:
	switch (desc->UdpB_Class)
	    {
	case UdpK_ClassA:   type = XieK_AF;		 break;
	case UdpK_ClassUBA:
	case UdpK_ClassUBS:
	case UdpK_ClassCL:  type = XieK_DATA_TYPE_COUNT; break;
	default:	    type = XieK_DATA_TYPE_COUNT; break;
	    } break;
    case UdpK_DTypeVU:
	switch (desc->UdpB_Class)
	    {
	case UdpK_ClassUBA: type = XieK_UBAVU;		 break;
	case UdpK_ClassUBS: type = XieK_UBSVU;		 break;
	case UdpK_ClassA:
	case UdpK_ClassCL:  type = XieK_DATA_TYPE_COUNT; break;
	default:	    type = XieK_DATA_TYPE_COUNT; break;
	    } break;
    case UdpK_DTypeV:
	switch (desc->UdpB_Class)
	    {
	case UdpK_ClassUBA: type = XieK_UBAV;		 break;
	case UdpK_ClassUBS: type = XieK_UBSV;		 break;
	case UdpK_ClassA:
	case UdpK_ClassCL:  type = XieK_DATA_TYPE_COUNT; break;
	default:	    type = XieK_DATA_TYPE_COUNT; break;
	    } break;
    case UdpK_DTypeCL:
	switch (desc->UdpB_Class)
	    {
	case UdpK_ClassCL: type = XieK_CLCL;		 break;
	case UdpK_ClassA:  
	case UdpK_ClassUBA:
	case UdpK_ClassUBS: type = XieK_DATA_TYPE_COUNT; break;
	default:            type = XieK_DATA_TYPE_COUNT; break;
           } break;
    default :		    type = XieK_DATA_TYPE_COUNT; break;
	}
    return( type );
}				    /* end DmiGetTyp */

/*****************************************************************************
**  DmiPosCl
**
**  FUNCTIONAL DESCRIPTION:
**
**      Return the UdpL_Pos value (i.e. offset in bits) which corresponds
**	to the beginning of the specified scanline in a changelist Udp.
**
**      This code assumes the specified scanline actually exists in the
**      UDP.
**
**  FORMAL PARAMETERS:
**
**	base   - base address of change list
**	inipos - inipos value
**	scncnt - number of scanlines to move ahead
**
**  FUNCTION VALUE:
**
**      Bit offset to the scanline from base.
**
**  SIGNAL CODES:
**
**      none
**
*****************************************************************************/
int		 DmiPosCl(base, inipos, scncnt)
unsigned int	*base;
int		 inipos;
int		 scncnt;
{
    unsigned int *lptr;			/* Pointer to a changelist */
    long pos;				/* Return pos value */
    int  scanline, element_sz_bits;

    /* 
    **	Optimize the null case
    */
    if( scncnt == 0 )
	return( inipos );

    element_sz_bits = 8 * sizeof(unsigned int);
    lptr = base + (inipos / element_sz_bits);
    pos  = inipos;
    /* 
    **	Sum up the changelist lengths (in bits) 
    */
    for( scanline = 0; scanline < scncnt; scanline++ )
	{
	pos  += (*lptr + 2) * element_sz_bits;
	lptr += (*lptr + 2);
	}

    return( pos );
}					/* end DmiPosCl */

/*****************************************************************************
**  _CvtUnsOpr
**
**  FUNCTIONAL DESCRIPTION:
**
**      This is an action routine called when conversion from source descriptor
**	type to destination descriptor type is not supported. Simply signal an
**	error at this point.
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int _CvtUnsOpr(src, dst)
UdpPtr  src;
UdpPtr  dst;
{
    return( BadImplementation );
}

/*****************************************************************************
**  _CvtBsBs
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from bit stream to bit stream encoding
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int _CvtBsBs(src, dst)
UdpPtr  src;
UdpPtr  dst;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int src_mask, dst_mask, mask_24 = (1 << 24) - 1;
    int src_scan_offset, dst_scan_offset;
    int src_offset, dst_offset;
    int bits;
    int value;
    char  *data, copy_line = src->UdpW_PixelLength == 1
			  && dst->UdpW_PixelLength == 1
			  && src->UdpL_PxlStride   == 1
			  && dst->UdpL_PxlStride   == 1;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base =  DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL ) return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    src_mask = (1 << src->UdpW_PixelLength) - 1;
    dst_mask = (1 << dst->UdpW_PixelLength) - 1;

    src_scan_offset = src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;
/*
** Optimize the case of copying from one contiguous, aligned bit string to
** another. This is a common case when copying PCM data out of the X wire
** buffers.  Note copy_line being true insures PxlStride is one, so we
** can omit it from these calculations.
*/
    if( copy_line
     && src->UdpL_ScnStride == dst->UdpL_ScnStride
     && src->UdpL_ScnStride == X2 - X1 + 1
     && (src_scan_offset + X1 - src->UdpL_X1 & 7) == 0
     && (dst_scan_offset + X1 - dst->UdpL_X1 & 7) == 0 )
	{
	memcpy( dst->UdpA_Base + ((dst_scan_offset + (X1-src->UdpL_X1)) >> 3),
	        src->UdpA_Base + ((src_scan_offset + (X1-dst->UdpL_X1)) >> 3),
	        (src->UdpL_ScnStride * (Y2-Y1+1)) >> 3);

	if( src->UdpL_ScnStride & 7 )
	    {	/*
		**  Bit string length is not a multiple of byte length.
		**  Copy the trailing bits.
		*/
	    bits = (Y2-Y1+1) * src->UdpL_ScnStride & ~7;
	    src_offset = src_scan_offset + X1 - src->UdpL_X1 + bits;
	    dst_offset = dst_scan_offset + X1 - dst->UdpL_X1 + bits;
	    while( bits != (Y2-Y1+1) * src->UdpL_ScnStride )
		{
		value = GET_VALUE_(src->UdpA_Base, src_offset, src_mask);
		PUT_VALUE_(dst->UdpA_Base, dst_offset, value, dst_mask);
		src_offset++;
		dst_offset++;
		bits++;
		}
	    }
	}
    else
	/*
	** Handle the more general cases.
	*/
	for( scanline = Y1;  scanline <= Y2 ;  scanline++ )  
	    {
	    src_offset = src_scan_offset+(X1-src->UdpL_X1)*src->UdpL_PxlStride;
	    dst_offset = dst_scan_offset+(X1-dst->UdpL_X1)*dst->UdpL_PxlStride;
	    /*
	    **  Copy until the destination is at a byte boundary.
	    */
	    for( pixel = X1; pixel <= X2  &&  dst_offset & 7; pixel++ )
		{
		value = GET_VALUE_(src->UdpA_Base, src_offset, src_mask);
		PUT_VALUE_(dst->UdpA_Base, dst_offset, value, dst_mask);
		src_offset += src->UdpL_PxlStride;
		dst_offset += dst->UdpL_PxlStride;
		}
	    /*
	    **  If there is enough data, try for a higher performance copy.
	    */
	    if( copy_line  &&  X2 + 1 - pixel >= 24 )
		if( src_offset & 7 )
		    /*
		    **  Src and dst aren't aligned, so move in 3 byte chunks.
		    */
		    for( data = (char *)(dst->UdpA_Base+(dst_offset>>3));
			    pixel <= X2 + 1 - 24;
				src_offset += 24,
				dst_offset += 24,
				pixel      += 24 )
			{
			value =
			    GET_VALUE_(src->UdpA_Base, src_offset, mask_24);
#ifdef BIGENDIAN
			*data++ = *((unsigned char *)(&value)+3);
			*data++ = *((unsigned char *)(&value)+2);
			*data++ = *((unsigned char *)(&value)+1);
#else
			*data++ = *((unsigned char *)(&value));
			*data++ = *((unsigned char *)(&value)+1);
			*data++ = *((unsigned char *)(&value)+2);
#endif
			}
		else
		    {   /*
			**  Src and dst are aligned, so we can use
			**  the fast byte mover.
			*/
		    bits = X2 + 1 - pixel & ~7;
		    memcpy( dst->UdpA_Base + (dst_offset >> 3),
			    src->UdpA_Base + (src_offset >> 3), bits >> 3 );
		    src_offset += bits;
		    dst_offset += bits;
		    pixel      += bits;
		    }
	    /*
	    **  Copy the remainder.
	    */
	    while( pixel <= X2 )
		{
		value = GET_VALUE_(src->UdpA_Base, src_offset, src_mask);
		PUT_VALUE_(dst->UdpA_Base, dst_offset, value, dst_mask);
		src_offset += src->UdpL_PxlStride;
		dst_offset += dst->UdpL_PxlStride;
		pixel++;
		}
	    src_scan_offset += src->UdpL_ScnStride;
	    dst_scan_offset += dst->UdpL_ScnStride;
	    }
    return( Success );
}

/*****************************************************************************
**  _CvtBsCl
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from bit string to change list
**
**      The source and destination UDP's must have the same X1 and X2
**      coordinates. (i.e. only full scanlines are processed.)
**
**      Data is unconditionally appended at the specified scanline.
**      Scanlines must be appended in strict sequence (i.e. no gaps).
**
**      The destination buffer (if one exists) is assumed to be
**      large enough to hold ScnCnt * worst-case changelist size.
**
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int _CvtBsCl(src, dst)
UdpPtr  src;
UdpPtr  dst;
{
    int scanline;
    int i;
    int Y1, Y2;
    int src_scan_offset, dst_scan_byte_offset;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base =  DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL ) return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    src_scan_offset = src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_byte_offset = 
	DmiPosCl(dst->UdpA_Base, dst->UdpL_Pos, Y1 - dst->UdpL_Y1) >> 3;

    if( src->UdpL_PxlStride == 1 )
	for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	    {
	    DmiBuildChangeList(src->UdpA_Base, src_scan_offset,
			      src->UdpL_PxlPerScn,
			      dst->UdpA_Base + dst_scan_byte_offset,
			      src->UdpL_PxlPerScn + 1);

	    src_scan_offset += src->UdpL_ScnStride;
	    dst_scan_byte_offset +=
     	        (*(unsigned int *)(dst->UdpA_Base+dst_scan_byte_offset) + 2)
		    * sizeof(unsigned int);
	    }
    else
	/* Need to skip over pixel pad */
	for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	    {
	    _BuildClBits(src->UdpA_Base, src_scan_offset,
		 src->UdpL_PxlPerScn, src->UdpL_PxlStride,
		 dst->UdpA_Base + dst_scan_byte_offset,
		 src->UdpL_PxlPerScn + 1);

	    src_scan_offset += src->UdpL_ScnStride;
	    dst_scan_byte_offset +=
     	        (*(unsigned int *)(dst->UdpA_Base+dst_scan_byte_offset) + 1)
		    * sizeof(unsigned int);
	    }
    return (Success);
}

/*****************************************************************************
**  _CvtBsByt
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from bit string to byte array
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int _CvtBsByt(src, dst)
UdpPtr  src;
UdpPtr  dst;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int src_mask;
    int src_scan_offset, dst_scan_byte_offset;
    int src_offset, dst_byte_offset;
    int dst_byte_stride;
    int dst_byte_scn_stride;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base =  DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL ) return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    src_mask = (1 << src->UdpW_PixelLength) - 1;
    dst_byte_scn_stride = dst->UdpL_ScnStride >> 3;
    dst_byte_stride = dst->UdpL_PxlStride >> 3;

    src_scan_offset = src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_byte_offset =
	       (dst->UdpL_Pos >> 3) + (Y1 - dst->UdpL_Y1) * dst_byte_scn_stride;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	src_offset = src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	dst_byte_offset = dst_scan_byte_offset + (X1-dst->UdpL_X1);

	for( pixel = X1;  pixel <= X2;  pixel++ )
	    {
	    (dst->UdpA_Base)[dst_byte_offset] =
			GET_VALUE_(src->UdpA_Base,src_offset,src_mask);
	    src_offset += src->UdpL_PxlStride;
	    dst_byte_offset += dst_byte_stride;
	    }
	src_scan_offset += src->UdpL_ScnStride;
	dst_scan_byte_offset += dst_byte_scn_stride;
	}
    return( Success );
}

/*****************************************************************************
**  _CvtBsWrd
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from bit string to word array
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int  _CvtBsWrd( src, dst )
UdpPtr  src;
UdpPtr  dst;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int src_mask;
    int src_scan_offset, dst_scan_offset;
    int src_offset, dst_offset;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base = DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL )
	    return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    src_mask = (1 << src->UdpW_PixelLength) - 1;

    src_scan_offset =
	src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = 
	dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;

    for (scanline = Y1;  scanline <= Y2 ;  scanline++)
	{
	    src_offset = 
		src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	    dst_offset = 
		dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;

	    for (pixel = X1;  pixel <= X2;  pixel++)
		{
		    *((unsigned short *)(dst->UdpA_Base + dst_offset / 8)) =
			GET_VALUE_(src->UdpA_Base,src_offset,src_mask);
		    src_offset += src->UdpL_PxlStride;
		    dst_offset += dst->UdpL_PxlStride;
		}

	    src_scan_offset += src->UdpL_ScnStride;
	    dst_scan_offset += dst->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _CvtBsFlt
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from bit string to float
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int _CvtBsFlt(src, dst)
UdpPtr  src;
UdpPtr  dst;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int src_mask;
    int src_scan_offset, dst_scan_offset;
    int src_offset, dst_offset;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base =  DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL ) return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    src_mask = (1 << src->UdpW_PixelLength) - 1;

    src_scan_offset = src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	src_offset = src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	dst_offset = dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;

	for( pixel = X1;  pixel <= X2;  pixel++ )
	    {
	    *((float *)(dst->UdpA_Base + dst_offset / 8)) =
			(float)GET_VALUE_(src->UdpA_Base,src_offset,src_mask);
	    src_offset += src->UdpL_PxlStride;
	    dst_offset += dst->UdpL_PxlStride;
	    }
	src_scan_offset += src->UdpL_ScnStride;
	dst_scan_offset += dst->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _CvtBytByt
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from byte array to byte array
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int _CvtBytByt( src, dst )
UdpPtr  src;
UdpPtr  dst;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int src_scan_offset, dst_scan_offset;
    int src_offset, dst_offset;
    char copy_line = src->UdpW_PixelLength == dst->UdpW_PixelLength
		  && src->UdpW_PixelLength == src->UdpL_PxlStride
		  && dst->UdpW_PixelLength == dst->UdpL_PxlStride
		  && src->UdpL_PxlStride   == dst->UdpL_PxlStride;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base =  DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL ) return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    src_scan_offset = src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	src_offset = src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	dst_offset = dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;

	if( copy_line )
	    memcpy( dst->UdpA_Base + (dst_offset >> 3),
		    src->UdpA_Base + (src_offset >> 3), X2 - X1 + 1 );
	else
	    for( pixel = X1;  pixel <= X2;  pixel++ )
		{
		*((unsigned char *)(dst->UdpA_Base + (dst_offset >> 3))) =
		*((unsigned char *)(src->UdpA_Base + (src_offset >> 3)));

		src_offset += src->UdpL_PxlStride;
		dst_offset += dst->UdpL_PxlStride;
		}
	src_scan_offset += src->UdpL_ScnStride;
	dst_scan_offset += dst->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _CvtBytBs
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from byte array to bit string
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int _CvtBytBs(src, dst)
UdpPtr  src;
UdpPtr  dst;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int dst_mask;
    int src_scan_byte_offset, dst_scan_offset;
    int src_byte_offset, dst_offset;
    int src_byte_stride, src_scan_byte_stride;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base =  DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL ) return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    dst_mask = (1 << dst->UdpW_PixelLength) - 1;

    src_scan_byte_stride = src->UdpL_ScnStride >> 3;
    src_byte_stride = src->UdpL_PxlStride >> 3;

    src_scan_byte_offset =
	      (src->UdpL_Pos >> 3) + (Y1 - src->UdpL_Y1) * src_scan_byte_stride;
    dst_scan_offset = dst->UdpL_Pos+ (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	src_byte_offset =
		src_scan_byte_offset + (X1-src->UdpL_X1) * src_byte_stride;
	dst_offset = dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;

	for( pixel = X1;  pixel <= X2;  pixel++ )
	    {
	    PUT_VALUE_(dst->UdpA_Base,dst_offset,
		      (src->UdpA_Base)[src_byte_offset],dst_mask);
	    src_byte_offset += src_byte_stride;
	    dst_offset += dst->UdpL_PxlStride;
	    }
	src_scan_byte_offset += src_scan_byte_stride;
	dst_scan_offset += dst->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _CvtBytCl
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from byte array to change list
**
**      The source and destination UDP's must have the same X1 and X2
**      coordinates. (i.e. only full scanlines are processed.)
**
**      Data is unconditionally appended at the specified scanline.
**      Scanlines must be appended in strict sequence (i.e. no gaps).
**
**      The destination buffer (if one exists) is assumed to be
**      large enough to hold ScnCnt * worst-case changelist size.
**
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int _CvtBytCl(src, dst)
UdpPtr  src;
UdpPtr  dst;
{
    int scanline;
    int i;
    int Y1, Y2;
    int src_scan_byte_offset, dst_scan_byte_offset;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base =  DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL ) return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    src_scan_byte_offset =
	(src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride) >> 3;
    dst_scan_byte_offset = 
	DmiPosCl(dst->UdpA_Base, dst->UdpL_Pos, Y1 - dst->UdpL_Y1) >> 3;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	_BuildClBytes(src->UdpA_Base + src_scan_byte_offset,
		     src->UdpL_PxlPerScn, src->UdpL_PxlStride >> 3,
		     dst->UdpA_Base + dst_scan_byte_offset,
		     src->UdpL_PxlPerScn + 1);

	src_scan_byte_offset += (src->UdpL_ScnStride >> 3);
	dst_scan_byte_offset +=
	    (*(unsigned int *)(dst->UdpA_Base+dst_scan_byte_offset) + 2)
		* sizeof(unsigned int);
	}
    return (Success);
}

/*****************************************************************************
**  _CvtBytWrd
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from byte array to word array
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int  _CvtBytWrd( src, dst )
UdpPtr  src;
UdpPtr  dst;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int src_scan_offset, dst_scan_offset;
    int src_offset, dst_offset;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base = DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL )
	    return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    src_scan_offset = src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	src_offset = src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	dst_offset = dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;

	for( pixel = X1;  pixel <= X2;  pixel++ )
	    {
	    *((unsigned short *)(dst->UdpA_Base + (dst_offset >> 3))) =
	    *((unsigned char *)(src->UdpA_Base + (src_offset >> 3)));

	    src_offset += src->UdpL_PxlStride;
	    dst_offset += dst->UdpL_PxlStride;
	    }

	src_scan_offset += src->UdpL_ScnStride;
	dst_scan_offset += dst->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _CvtBytFlt
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from byte array to floating point
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int _CvtBytFlt(src, dst)
UdpPtr  src;
UdpPtr  dst;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int src_scan_byte_offset, dst_scan_float_offset;
    int src_byte_offset, dst_float_offset;
    int src_scan_byte_stride, src_byte_stride;
    int dst_scan_float_stride, dst_float_stride;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base =  DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL ) return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    src_scan_byte_stride = src->UdpL_ScnStride >> 3;
    src_byte_stride = src->UdpL_PxlStride >> 3;
    dst_scan_float_stride = dst->UdpL_ScnStride / (8 * sizeof(float));
    dst_float_stride = dst->UdpL_PxlStride / (8 * sizeof(float));

    src_scan_byte_offset =
	(src->UdpL_Pos >> 3) + (Y1 - src->UdpL_Y1) * src_scan_byte_stride;
    dst_scan_float_offset = 
	(dst->UdpL_Pos / (8 * sizeof(float))) +
	    (Y1 - dst->UdpL_Y1) * dst_scan_float_stride;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	src_byte_offset = 
		 src_scan_byte_offset + (X1-src->UdpL_X1) * src_byte_stride;
	dst_float_offset = 
		dst_scan_float_offset + (X1-dst->UdpL_X1) * dst_float_stride;

	for( pixel = X1;  pixel <= X2;  pixel++ )
	    {
	    ((float *)(dst->UdpA_Base))[dst_float_offset] =
		    (float)((unsigned char *)(src->UdpA_Base))[src_byte_offset];

	    src_byte_offset += src_byte_stride;
	    dst_float_offset += dst_float_stride;
	    }
	src_scan_byte_offset += src_scan_byte_stride;
	dst_scan_float_offset += dst_scan_float_stride;
	}
    return( Success );
}

/*****************************************************************************
**  _CvtWrdWrd
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from word array to word array
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int  _CvtWrdWrd( src, dst )
UdpPtr  src;
UdpPtr  dst;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int src_scan_offset, dst_scan_offset;
    int src_offset, dst_offset;
    char copy_line = src->UdpW_PixelLength == dst->UdpW_PixelLength
		  && src->UdpW_PixelLength == src->UdpL_PxlStride
		  && dst->UdpW_PixelLength == dst->UdpL_PxlStride
		  && src->UdpL_PxlStride   == dst->UdpL_PxlStride;

/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base = DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL )
	    return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    src_scan_offset = src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	src_offset = src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	dst_offset = dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;

	if( copy_line )
	    memcpy( dst->UdpA_Base + (dst_offset >> 3),
		    src->UdpA_Base + (src_offset >> 3), X2 - X1 + 1 << 1 );
	else
	    for( pixel = X1;  pixel <= X2;  pixel++ )
		{
		*((unsigned short *)(dst->UdpA_Base + (dst_offset >> 3))) =
		*((unsigned short *)(src->UdpA_Base + (src_offset >> 3)));

		src_offset += src->UdpL_PxlStride;
		dst_offset += dst->UdpL_PxlStride;
		}

	src_scan_offset += src->UdpL_ScnStride;
	dst_scan_offset += dst->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _CvtWrdBs
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from word array to bit string
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int  _CvtWrdBs( src, dst )
UdpPtr  src;
UdpPtr  dst;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int dst_mask;
    int src_scan_offset, dst_scan_offset;
    int src_offset, dst_offset;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base = DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL )
	    return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    dst_mask = (1 << dst->UdpW_PixelLength) - 1;

    src_scan_offset =
	src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = 
	dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;

    for (scanline = Y1;  scanline <= Y2 ;  scanline++)
	{
	    src_offset = 
		src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	    dst_offset = 
		dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;

	    for (pixel = X1;  pixel <= X2;  pixel++)
		{
		    PUT_VALUE_( dst->UdpA_Base,dst_offset,
			*((unsigned short *)(src->UdpA_Base + src_offset / 8)),
			dst_mask );
		    src_offset += src->UdpL_PxlStride;
		    dst_offset += dst->UdpL_PxlStride;
		}


	    src_scan_offset += src->UdpL_ScnStride;
	    dst_scan_offset += dst->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _CvtWrdCl
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from word array to change list
**
**      The source and destination UDP's must have the same X1 and X2
**      coordinates. (i.e. only full scanlines are processed.)
**
**      Data is unconditionally appended at the specified scanline.
**      Scanlines must be appended in strict sequence (i.e. no gaps).
**
**      The destination buffer (if one exists) is assumed to be
**      large enough to hold ScnCnt * worst-case changelist size.
**
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int _CvtWrdCl(src, dst)
UdpPtr  src;
UdpPtr  dst;
{
    int scanline;
    int i;
    int Y1, Y2;
    int src_scan_byte_offset, dst_scan_byte_offset;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base =  DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL ) return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    src_scan_byte_offset =
	(src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride) >> 3;
    dst_scan_byte_offset = 
	DmiPosCl(dst->UdpA_Base, dst->UdpL_Pos, Y1 - dst->UdpL_Y1) >> 3;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	_BuildClWords(src->UdpA_Base + src_scan_byte_offset,
		      src->UdpL_PxlPerScn, src->UdpL_PxlStride >> 4,
		      dst->UdpA_Base + dst_scan_byte_offset,
		      src->UdpL_PxlPerScn + 1);

	src_scan_byte_offset += (src->UdpL_ScnStride >> 3);
	dst_scan_byte_offset +=
	    (*(unsigned int *)(dst->UdpA_Base+dst_scan_byte_offset) + 2)
		* sizeof(unsigned int);
	}
    return (Success);
}

/*****************************************************************************
**  _CvtWrdByt
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from word array to byte array
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int  _CvtWrdByt( src, dst )
UdpPtr  src;
UdpPtr  dst;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int src_scan_offset, dst_scan_offset;
    int src_offset, dst_offset;

/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base = DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL )
	    return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    src_scan_offset = src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	src_offset = src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	dst_offset = dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;

	for( pixel = X1;  pixel <= X2;  pixel++ )
	    {
	    *((unsigned char *) (dst->UdpA_Base + (dst_offset >> 3))) =
	    *((unsigned short *)(src->UdpA_Base + (src_offset >> 3)));

	    src_offset += src->UdpL_PxlStride;
	    dst_offset += dst->UdpL_PxlStride;
	    }

	src_scan_offset += src->UdpL_ScnStride;
	dst_scan_offset += dst->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _CvtWrdFlt
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from word array to floating point
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int  _CvtWrdFlt( src, dst )
UdpPtr  src;
UdpPtr  dst;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int src_mask;
    int src_scan_offset, dst_scan_offset;
    int src_offset, dst_offset;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base = DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL )
	    return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    src_mask = (1 << src->UdpW_PixelLength) - 1;

    src_scan_offset =
	src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = 
	dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;

    for (scanline = Y1;  scanline <= Y2 ;  scanline++)
	{
	    src_offset = 
		src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	    dst_offset = 
		dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;

	    for (pixel = X1;  pixel <= X2;  pixel++)
		{
		    *((float *)(dst->UdpA_Base + dst_offset / 8)) =
			(float)*((unsigned short *)(src->UdpA_Base + src_offset / 8));

		    src_offset += src->UdpL_PxlStride;
		    dst_offset += dst->UdpL_PxlStride;
		}

	    src_scan_offset += src->UdpL_ScnStride;
	    dst_scan_offset += dst->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _CvtFltFlt
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from floating array to floating array
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int _CvtFltFlt( src, dst )
UdpPtr  src;
UdpPtr  dst;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int src_scan_offset, dst_scan_offset;
    int src_offset, dst_offset;
    char copy_line = src->UdpW_PixelLength == dst->UdpW_PixelLength
		  && src->UdpW_PixelLength == src->UdpL_PxlStride
		  && dst->UdpW_PixelLength == dst->UdpL_PxlStride
		  && src->UdpL_PxlStride   == dst->UdpL_PxlStride;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base =  DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL ) return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    src_scan_offset = src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	src_offset = src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	dst_offset = dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;

	if( copy_line )
	    memcpy( dst->UdpA_Base + (dst_offset >> 3),
		    src->UdpA_Base + (src_offset >> 3),
		     (X2 + 1 - X1) * sizeof(float) );
	else
	    for( pixel = X1;  pixel <= X2;  pixel++ )
		{
		*((float *)(dst->UdpA_Base + (dst_offset >> 3))) =
		*((float *)(src->UdpA_Base + (src_offset >> 3)));
		src_offset += src->UdpL_PxlStride;
		dst_offset += dst->UdpL_PxlStride;
		}
	src_scan_offset += src->UdpL_ScnStride;
	dst_scan_offset += dst->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _CvtFltBs
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from floating array to bit string
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int _CvtFltBs(src, dst)
UdpPtr  src;
UdpPtr  dst;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int dst_mask;
    int src_scan_offset, dst_scan_offset;
    int src_offset, dst_offset;
    int value;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base =  DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL ) return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    dst_mask = (1 << dst->UdpW_PixelLength) - 1;

    src_scan_offset = src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	src_offset = src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	dst_offset = dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;

	for( pixel = X1;  pixel <= X2;  pixel++ )
	    {
	    value = *((float *)(src->UdpA_Base + src_offset / 8)) + 0.5;
	    PUT_VALUE_(dst->UdpA_Base,dst_offset,value,dst_mask);

	    src_offset += src->UdpL_PxlStride;
	    dst_offset += dst->UdpL_PxlStride;
	    }
	src_scan_offset += src->UdpL_ScnStride;
	dst_scan_offset += dst->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _CvtFltByt
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from floating array to byte array
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int _CvtFltByt(src, dst)
UdpPtr  src;
UdpPtr  dst;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int src_scan_offset, dst_scan_offset;
    int src_offset, dst_offset;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base =  DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL ) return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    src_scan_offset = src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	src_offset = src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	dst_offset = dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;

	for( pixel = X1;  pixel <= X2;  pixel++ )
	    {
	    *((unsigned char *)(dst->UdpA_Base + dst_offset / 8)) =
		    *((float *)(src->UdpA_Base + src_offset / 8)) + 0.5;
	    src_offset += src->UdpL_PxlStride;
	    dst_offset += dst->UdpL_PxlStride;
	    }
	src_scan_offset += src->UdpL_ScnStride;
	dst_scan_offset += dst->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _CvtFltWrd
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from floating array to word array
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int  _CvtFltWrd( src, dst )
UdpPtr  src;
UdpPtr  dst;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int src_scan_offset, dst_scan_offset;
    int src_offset, dst_offset;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base = DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL )
	    return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    src_scan_offset =
	src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = 
	dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;

    for (scanline = Y1;  scanline <= Y2 ;  scanline++)
	{
	    src_offset = 
		src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	    dst_offset = 
		dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;

	    for (pixel = X1;  pixel <= X2;  pixel++)
		{
		    *((unsigned short *)(dst->UdpA_Base + dst_offset / 8)) =
			*((float *)(src->UdpA_Base + src_offset / 8)) + 0.5;
		    src_offset += src->UdpL_PxlStride;
		    dst_offset += dst->UdpL_PxlStride;
		}

	    src_scan_offset += src->UdpL_ScnStride;
	    dst_scan_offset += dst->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _CvtClCl
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from change list to change list
**
**      The source and destination UDP's must have the same X1 and X2
**      coordinates. (i.e. only full scanlines are processed.)
**
**      Data is unconditionally appended at the specified scanline.
**      Scanlines must be appended in strict sequence (i.e. no gaps).
**
**      The destination buffer (if one exists) is assumed to be
**      large enough to hold ScnCnt * worst-case changelist size.
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int _CvtClCl(src, dst)
UdpPtr  src;
UdpPtr  dst;
{
    int scanline;
    int i;
    int Y1, Y2;
    int src_scan_index, dst_scan_index;
    unsigned long length;

    unsigned int *srcptr;
    unsigned int *dstptr;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base =  DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL ) return( BadAlloc );
	}
    srcptr = (unsigned int *)src->UdpA_Base;
    dstptr = (unsigned int *)dst->UdpA_Base;

/*
**  Determine maximum common dimensions between source and destination.
*/
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    src_scan_index = 
	DmiPosCl(src->UdpA_Base, src->UdpL_Pos, Y1 - src->UdpL_Y1) / 
	    ( 8 * sizeof(unsigned int));
    dst_scan_index = 
	DmiPosCl(dst->UdpA_Base, dst->UdpL_Pos, Y1 - dst->UdpL_Y1) /
	    ( 8 * sizeof(unsigned int));

    for(scanline = Y1;  scanline <= Y2;  scanline++ )
	{
	length = srcptr[src_scan_index] + 2;
	memcpy( &(dstptr[dst_scan_index]),
		&(srcptr[src_scan_index]),
	       length * sizeof(unsigned int));

	src_scan_index += length;
	dst_scan_index += length;
	}
    return( Success );
}

/*****************************************************************************
**  _CvtClBs
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from change list to bit string
**
**      The source and destination UDP's must have the same X1 and X2
**      coordinates. (i.e. only full scanlines are processed.)
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int _CvtClBs(src, dst)
UdpPtr  src;
UdpPtr  dst;
{
    int scanline;
    int i;
    int  Y1, Y2;
    int src_scan_byte_offset, dst_scan_offset;

    long size;
    int index;
    int offset;
    unsigned int *cl_addr;
    int status;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base =  DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL ) return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    /* Position to proper place in source changelist */
    src_scan_byte_offset = 
	DmiPosCl(src->UdpA_Base, src->UdpL_Pos, Y1 - src->UdpL_Y1) >> 3; 

    dst_scan_offset = dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;

    /* Zero the destination region so we only have to write "1" bits */
    if( (status = DdxFillRegion_( dst, dst->UdpL_X1, Y1, dst->UdpL_PxlPerScn,
						Y2 - Y1 + 1, 0 ) ) != Success )
	return( status );

    if( dst->UdpL_PxlStride == 1 )
	for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	    {
	    DmiPutRuns( dst->UdpA_Base, dst_scan_offset,
		        src->UdpA_Base + src_scan_byte_offset );

	    src_scan_byte_offset +=
     	        (*(unsigned int *)(src->UdpA_Base+src_scan_byte_offset) + 2)
						    * sizeof(unsigned int);
	    dst_scan_offset += dst->UdpL_ScnStride;
	    }
    else
	/* Need to skip over pixel pad */
	for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	    {
	    cl_addr = (unsigned int *)(src->UdpA_Base+src_scan_byte_offset);
	    /*
	     ** While there are elements in the change list write runs
	     ** of bits into the destination image data buffer.
	     ** Since data buffer starts out as all 0's, we write only
	     ** runs of 1's.
	     */
	    for( index = 1; index < cl_addr[0]; index += 2 )
		for( offset = dst_scan_offset + cl_addr[index],
		     size   = cl_addr[index+1]- cl_addr[index];
		     size > 0;  offset += dst->UdpL_PxlStride, size-- )
		    PUT_VALUE_( dst->UdpA_Base, offset, 1, 1 );

	    src_scan_byte_offset +=
     	        (*(unsigned int *)(src->UdpA_Base+src_scan_byte_offset) + 1)
						    * sizeof(unsigned int);
	    dst_scan_offset += dst->UdpL_ScnStride;
	    }
    return( Success );
}

/*****************************************************************************
**  _CvtClByt
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from change list to byte format
**
**      The source and destination UDP's must have the same X1 and X2
**      coordinates. (i.e. only full scanlines are processed.)
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int _CvtClByt(src, dst)
UdpPtr  src;
UdpPtr  dst;
{
    int scanline;
    int i;
    int Y1, Y2;
    int dst_scan_byte_offset;

    long size;
    int index;
    int offset;
    int status;

    unsigned int *cl_addr = (unsigned int *)src->UdpA_Base;
    int byte_stride = dst->UdpL_PxlStride >> 3;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base =  DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL ) return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    /* Position to proper place in source changelist */
    cl_addr += (DmiPosCl(src->UdpA_Base, src->UdpL_Pos, Y1 - src->UdpL_Y1) / 
		    (8 * sizeof( int )));

    dst_scan_byte_offset =
	(dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride)/8;

    /* Zero the buffer so we only have to write "1" bits */
    if( (status = DdxFillRegion_( dst, dst->UdpL_X1, Y1, dst->UdpL_PxlPerScn,
						Y2 - Y1 + 1, 0 ) ) != Success )
	return( status );

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	for( index = 1; index < cl_addr[0]; index += 2 )
	    for( offset = dst_scan_byte_offset + cl_addr[index],
		 size   = cl_addr[index + 1] - cl_addr[index];
		 size > 0; offset += byte_stride, size-- )
		(dst->UdpA_Base)[offset] = 1;

	cl_addr += (cl_addr[0] + 2);
	dst_scan_byte_offset += dst->UdpL_ScnStride / 8;
	}
    return( Success);
}

/*****************************************************************************
**  _CvtClWrd
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from change list to word array
**
**      The source and destination UDP's must have the same X1 and X2
**      coordinates. (i.e. only full scanlines are processed.)
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int _CvtClWrd(src,dst)
UdpPtr  src;
UdpPtr  dst;
{
    int scanline;
    int i;
    int Y1, Y2;
    int dst_scan_word_offset;

    long size;
    int index;
    int offset;
    int status;

    unsigned int *cl_addr = (unsigned int *)src->UdpA_Base;
    int word_stride = dst->UdpL_PxlStride /(8 * sizeof(short));
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base =  DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL ) return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    /* Position to proper place in source changelist */
    cl_addr += (DmiPosCl(src->UdpA_Base, src->UdpL_Pos, Y1 - src->UdpL_Y1) / 
		    (8 * sizeof( int )));

    dst_scan_word_offset =
	(dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride)
	    / (8 * sizeof(short));

    /* Zero the destination region so we only have to write "1" bits */
    if( ( status = DdxFillRegion_( dst, dst->UdpL_X1, Y1, dst->UdpL_PxlPerScn,
						 Y2 - Y1 + 1, 0 ) ) != Success )
	return( status );

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	for( index = 1; index < cl_addr[0]; index += 2 )
	    for( offset = dst_scan_word_offset + cl_addr[index],
		 size   = cl_addr[index + 1] - cl_addr[index];
		 size > 0;  offset += word_stride, size-- )
		((unsigned short *)dst->UdpA_Base)[offset] = 1.0;

	cl_addr += (cl_addr[0] + 2);
	dst_scan_word_offset += dst->UdpL_ScnStride / (8 * sizeof(short));
	}
    return( Success );
}

/*****************************************************************************
**  _CvtClFlt
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from change list to floating array
**
**      The source and destination UDP's must have the same X1 and X2
**      coordinates. (i.e. only full scanlines are processed.)
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int _CvtClFlt(src,dst)
UdpPtr  src;
UdpPtr  dst;
{
    int scanline;
    int i;
    int Y1, Y2;
    int dst_scan_float_offset;

    long size;
    int index;
    int offset;
    int status;

    unsigned int *cl_addr = (unsigned int *)src->UdpA_Base;
    int float_stride = dst->UdpL_PxlStride /(8 * sizeof(float));
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base =  DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL ) return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    /* Position to proper place in source changelist */
    cl_addr += (DmiPosCl(src->UdpA_Base, src->UdpL_Pos, Y1 - src->UdpL_Y1) / 
		    (8 * sizeof( int )));

    dst_scan_float_offset =
	(dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride)
	    / (8 * sizeof(float));

    /* Zero the destination region so we only have to write "1" bits */
    if( ( status = DdxFillRegion_( dst, dst->UdpL_X1, Y1, dst->UdpL_PxlPerScn,
						 Y2 - Y1 + 1, 0 ) ) != Success )
	return( status );

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	for( index = 1; index < cl_addr[0]; index += 2 )
	    for( offset = dst_scan_float_offset + cl_addr[index],
		 size   = cl_addr[index + 1] - cl_addr[index];
		 size > 0;  offset += float_stride, size-- )
		((float *)dst->UdpA_Base)[offset] = 1.0;

	cl_addr += (cl_addr[0] + 2);
	dst_scan_float_offset += dst->UdpL_ScnStride / (8 * sizeof(float));
	}
    return( Success );
}

/*****************************************************************************
**  _CvtBsBsCpp
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from bit stream to bit stream encoding with CPP
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**      cpp - cpp data descriptor
**
*****************************************************************************/
static int _CvtBsBsCpp(src, dst, cpp)
UdpPtr  src;
UdpPtr  dst;
UdpPtr  cpp;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int src_mask, dst_mask;
    int src_scan_offset, dst_scan_offset, cpp_scan_offset;
    int src_offset, dst_offset, cpp_offset;
    int bits;
    int value;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base =  DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL ) return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    src_mask = (1 << src->UdpW_PixelLength) - 1;
    dst_mask = (1 << dst->UdpW_PixelLength) - 1;

    src_scan_offset = src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;
    cpp_scan_offset = cpp->UdpL_Pos + (Y1 - cpp->UdpL_Y1) * cpp->UdpL_ScnStride;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	src_offset = src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	dst_offset = dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;
	cpp_offset = cpp_scan_offset;
	/*
	**  Copy the bits.
	*/
	for( pixel = X1; pixel <= X2; pixel++ )
	    {
	    if( GET_VALUE_(cpp->UdpA_Base, cpp_offset, 1) )
		value = GET_VALUE_(src->UdpA_Base, src_offset, src_mask );
		PUT_VALUE_( dst->UdpA_Base, dst_offset, value, dst_mask );

	    src_offset += src->UdpL_PxlStride;
	    dst_offset += dst->UdpL_PxlStride;
	    cpp_offset += cpp->UdpL_PxlStride;
	    }
	src_scan_offset += src->UdpL_ScnStride;
	dst_scan_offset += dst->UdpL_ScnStride;
	cpp_scan_offset += cpp->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _CvtBsBytCpp
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from bit string to byte array using cpp
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**      cpp - cpp data descriptor
**
*****************************************************************************/
static int _CvtBsBytCpp(src, dst, cpp)
UdpPtr  src;
UdpPtr  dst;
UdpPtr  cpp;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int src_mask;
    int src_scan_offset, dst_scan_offset, cpp_scan_offset;
    int src_offset, dst_offset, cpp_offset;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base =  DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL ) return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    src_mask = (1 << src->UdpW_PixelLength) - 1;

    src_scan_offset = src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;
    cpp_scan_offset = cpp->UdpL_Pos + (Y1 - cpp->UdpL_Y1) * cpp->UdpL_ScnStride;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	src_offset = src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	dst_offset = dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;
	cpp_offset = cpp_scan_offset;

	for( pixel = X1;  pixel <= X2;  pixel++ )
	    {
	    if( GET_VALUE_(cpp->UdpA_Base, cpp_offset, 1) )
		*((unsigned char *)(dst->UdpA_Base + dst_offset / 8)) =
			 GET_VALUE_(src->UdpA_Base, src_offset, src_mask);

	    src_offset += src->UdpL_PxlStride;
	    dst_offset += dst->UdpL_PxlStride;
	    cpp_offset += cpp->UdpL_PxlStride;
	    }
	src_scan_offset += src->UdpL_ScnStride;
	dst_scan_offset += dst->UdpL_ScnStride;
	cpp_scan_offset += cpp->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _CvtBsWrdCpp
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from bit string to word array using cpp
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**      cpp - cpp data descriptor
**
*****************************************************************************/
static int  _CvtBsWrdCpp(src, dst, cpp)
UdpPtr  src;
UdpPtr  dst;
UdpPtr  cpp;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int src_mask;
    int src_scan_offset, dst_scan_offset, cpp_scan_offset;
    int src_offset, dst_offset, cpp_offset;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base = DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL )
	    return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    src_mask = (1 << src->UdpW_PixelLength) - 1;

    src_scan_offset =
	src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = 
	dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;
    cpp_scan_offset = cpp->UdpL_Pos + (Y1 - cpp->UdpL_Y1) * cpp->UdpL_ScnStride;

    for (scanline = Y1;  scanline <= Y2 ;  scanline++)
	{
	    src_offset = 
		src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	    dst_offset = 
		dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;
	    cpp_offset = cpp_scan_offset;

	    for (pixel = X1;  pixel <= X2;  pixel++)
		{
		    if (GET_VALUE_(cpp->UdpA_Base, cpp_offset, 1)) {
			*((unsigned short *)(dst->UdpA_Base + dst_offset / 8)) =
			        GET_VALUE_(src->UdpA_Base,src_offset,src_mask);
		    }
		    src_offset += src->UdpL_PxlStride;
		    dst_offset += dst->UdpL_PxlStride;
		    cpp_offset += cpp->UdpL_PxlStride;
		}

	    src_scan_offset += src->UdpL_ScnStride;
	    dst_scan_offset += dst->UdpL_ScnStride;
	    cpp_scan_offset += cpp->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _CvtBsFltCpp
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from bit string to float using a CPP
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**      cpp - cpp data descriptor
**
*****************************************************************************/
static int _CvtBsFltCpp(src, dst, cpp)
UdpPtr  src;
UdpPtr  dst;
UdpPtr  cpp;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int src_mask;
    int src_scan_offset, dst_scan_offset, cpp_scan_offset;
    int src_offset, dst_offset, cpp_offset;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base =  DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL ) return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    src_mask = (1 << src->UdpW_PixelLength) - 1;

    src_scan_offset = src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;
    cpp_scan_offset = cpp->UdpL_Pos + (Y1 - cpp->UdpL_Y1) * cpp->UdpL_ScnStride;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	src_offset = src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	dst_offset = dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;
	cpp_offset = cpp_scan_offset;

	for( pixel = X1;  pixel <= X2;  pixel++ )
	    {
	    if( GET_VALUE_(cpp->UdpA_Base, cpp_offset, 1) )
		*((float *)(dst->UdpA_Base + dst_offset / 8)) =
		  (float)GET_VALUE_(src->UdpA_Base, src_offset, src_mask);

	    src_offset += src->UdpL_PxlStride;
	    dst_offset += dst->UdpL_PxlStride;
	    cpp_offset += cpp->UdpL_PxlStride;
	    }
	src_scan_offset += src->UdpL_ScnStride;
	dst_scan_offset += dst->UdpL_ScnStride;
	cpp_scan_offset += cpp->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _CvtBytBytCpp
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from byte array to byte array
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int _CvtBytBytCpp( src, dst, cpp )
UdpPtr  src;
UdpPtr  dst;
UdpPtr  cpp;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int src_scan_offset, dst_scan_offset, cpp_scan_offset;
    int src_offset, dst_offset, cpp_offset;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base =  DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL ) return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    src_scan_offset = src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;
    cpp_scan_offset = cpp->UdpL_Pos + (Y1 - cpp->UdpL_Y1) * cpp->UdpL_ScnStride;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	src_offset = src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	dst_offset = dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;
	cpp_offset = cpp_scan_offset;

	for( pixel = X1;  pixel <= X2;  pixel++ )
	    {
	    if( GET_VALUE_(cpp->UdpA_Base, cpp_offset, 1) )
		*((unsigned char *)(dst->UdpA_Base + (dst_offset >> 3))) =
		*((unsigned char *)(src->UdpA_Base + (src_offset >> 3)));

	    src_offset += src->UdpL_PxlStride;
	    dst_offset += dst->UdpL_PxlStride;
	    cpp_offset += cpp->UdpL_PxlStride;
	    }
	src_scan_offset += src->UdpL_ScnStride;
	dst_scan_offset += dst->UdpL_ScnStride;
	cpp_scan_offset += cpp->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _CvtBytBsCpp
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from byte array to bit string using a CPP
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**      cpp - cpp data descriptor
**
*****************************************************************************/
static int _CvtBytBsCpp(src, dst, cpp)
UdpPtr  src;
UdpPtr  dst;
UdpPtr  cpp;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int dst_mask;
    int src_scan_offset, dst_scan_offset, cpp_scan_offset;
    int src_offset, dst_offset, cpp_offset;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base =  DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL ) return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    dst_mask = (1 << dst->UdpW_PixelLength) - 1;

    src_scan_offset = src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;
    cpp_scan_offset = cpp->UdpL_Pos + (Y1 - cpp->UdpL_Y1) * cpp->UdpL_ScnStride;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	src_offset = src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	dst_offset = dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;
	cpp_offset = cpp_scan_offset;

	for( pixel = X1;  pixel <= X2;  pixel++ )
	    {
	    if( GET_VALUE_(cpp->UdpA_Base, cpp_offset, 1) )
		PUT_VALUE_(dst->UdpA_Base, dst_offset,
			    *((unsigned char *)(src->UdpA_Base+src_offset/8)),
								    dst_mask);
	    src_offset += src->UdpL_PxlStride;
	    dst_offset += dst->UdpL_PxlStride;
	    cpp_offset += cpp->UdpL_PxlStride;
	    }
	src_scan_offset += src->UdpL_ScnStride;
	dst_scan_offset += dst->UdpL_ScnStride;
	cpp_scan_offset += cpp->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _CvtBytWrdCpp
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from byte array to word array using a cpp
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**	cpp - cpp descriptor
**
*****************************************************************************/
static int  _CvtBytWrdCpp( src, dst, cpp )
UdpPtr  src;
UdpPtr  dst;
UdpPtr  cpp;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int src_scan_offset, dst_scan_offset, cpp_scan_offset;
    int src_offset, dst_offset, cpp_offset;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base = DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL )
	    return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    src_scan_offset = src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;
    cpp_scan_offset = cpp->UdpL_Pos + (Y1 - cpp->UdpL_Y1) * cpp->UdpL_ScnStride;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	src_offset = src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	dst_offset = dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;
	cpp_offset = cpp_scan_offset;

	for( pixel = X1;  pixel <= X2;  pixel++ )
	{
	    if (GET_VALUE_(cpp->UdpA_Base, cpp_offset, 1)) {
		*((unsigned short *)(dst->UdpA_Base + (dst_offset >> 3))) =
		    *((unsigned char *)(src->UdpA_Base + (src_offset >> 3)));
	    }
	    src_offset += src->UdpL_PxlStride;
	    dst_offset += dst->UdpL_PxlStride;
	    cpp_offset += cpp->UdpL_PxlStride;
	}

	src_scan_offset += src->UdpL_ScnStride;
	dst_scan_offset += dst->UdpL_ScnStride;
	cpp_scan_offset += cpp->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _CvtBytFltCpp
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from byte array to floating point using a CPP
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int _CvtBytFltCpp(src, dst, cpp)
UdpPtr  src;
UdpPtr  dst;
UdpPtr  cpp;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int src_mask;
    int src_scan_offset, dst_scan_offset, cpp_scan_offset;
    int src_offset, dst_offset, cpp_offset;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base =  DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL ) return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    src_mask = (1 << src->UdpW_PixelLength) - 1;

    src_scan_offset = src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;
    cpp_scan_offset = cpp->UdpL_Pos + (Y1 - cpp->UdpL_Y1) * cpp->UdpL_ScnStride;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	src_offset = src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	dst_offset = dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;
	cpp_offset = cpp_scan_offset;

	for( pixel = X1;  pixel <= X2;  pixel++ )
	    {
	    if( GET_VALUE_(cpp->UdpA_Base, cpp_offset, 1) )
		*((float *)(dst->UdpA_Base + dst_offset / 8)) =
		  (float)*((unsigned char *)(src->UdpA_Base + src_offset / 8));

	    src_offset += src->UdpL_PxlStride;
	    dst_offset += dst->UdpL_PxlStride;
	    cpp_offset += cpp->UdpL_PxlStride;
	    }
	src_scan_offset += src->UdpL_ScnStride;
	dst_scan_offset += dst->UdpL_ScnStride;
	cpp_scan_offset += cpp->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _CvtWrdWrdCpp
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from word array to word array using a cpp
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**	cpp - cpp descriptor
**
*****************************************************************************/
static int  _CvtWrdWrdCpp( src, dst, cpp )
UdpPtr  src;
UdpPtr  dst;
UdpPtr  cpp;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int src_scan_offset, dst_scan_offset, cpp_scan_offset;
    int src_offset, dst_offset, cpp_offset;

/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base = DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL )
	    return( BadAlloc );
	}

/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    src_scan_offset = src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;
    cpp_scan_offset = cpp->UdpL_Pos + (Y1 - cpp->UdpL_Y1) * cpp->UdpL_ScnStride;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	src_offset = src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	dst_offset = dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;
	cpp_offset = cpp_scan_offset;

	for( pixel = X1;  pixel <= X2;  pixel++ )
	{
	    if (GET_VALUE_(cpp->UdpA_Base, cpp_offset, 1)) {
		*((unsigned short *)(dst->UdpA_Base + (dst_offset >> 3))) =
		    *((unsigned short *)(src->UdpA_Base + (src_offset >> 3)));
	    }
	    src_offset += src->UdpL_PxlStride;
	    dst_offset += dst->UdpL_PxlStride;
	    cpp_offset += cpp->UdpL_PxlStride;
	}

	src_scan_offset += src->UdpL_ScnStride;
	dst_scan_offset += dst->UdpL_ScnStride;
	cpp_scan_offset += cpp->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _CvtWrdBsCpp
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from word array to bit string using a CPP
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**      cpp - cpp data descriptor
**
*****************************************************************************/
static int  _CvtWrdBsCpp( src, dst, cpp )
UdpPtr  src;
UdpPtr  dst;
UdpPtr  cpp;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int dst_mask;
    int src_scan_offset, dst_scan_offset, cpp_scan_offset;
    int src_offset, dst_offset, cpp_offset;

/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base = DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL )
	    return( BadAlloc );
	}

/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    dst_mask = (1 << dst->UdpW_PixelLength) - 1;

    src_scan_offset =
	src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = 
	dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;
    cpp_scan_offset = cpp->UdpL_Pos + (Y1 - cpp->UdpL_Y1) * cpp->UdpL_ScnStride;

    for (scanline = Y1;  scanline <= Y2 ;  scanline++)
	{
	    src_offset = 
		src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	    dst_offset = 
		dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;
	    cpp_offset = cpp_scan_offset;

	    for (pixel = X1;  pixel <= X2;  pixel++)
		{
		    if (GET_VALUE_(cpp->UdpA_Base, cpp_offset, 1)) {
			PUT_VALUE_(dst->UdpA_Base,dst_offset,
				   *((unsigned short *)(src->UdpA_Base +
					       src_offset / 8)),dst_mask);
		    }
		    src_offset += src->UdpL_PxlStride;
		    dst_offset += dst->UdpL_PxlStride;
		    cpp_offset += cpp->UdpL_PxlStride;
		}

	    src_scan_offset += src->UdpL_ScnStride;
	    dst_scan_offset += dst->UdpL_ScnStride;
	    cpp_scan_offset += cpp->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _CvtWrdBytCpp
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from word array to byte array using a CPP
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**	cpp = CPP descriptor
**
*****************************************************************************/
static int  _CvtWrdBytCpp( src, dst, cpp )
UdpPtr  src;
UdpPtr  dst;
UdpPtr  cpp;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int src_scan_offset, dst_scan_offset, cpp_scan_offset;
    int src_offset, dst_offset, cpp_offset;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base = DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL )
	    return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    src_scan_offset = src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;
    cpp_scan_offset = cpp->UdpL_Pos + (Y1 - cpp->UdpL_Y1) * cpp->UdpL_ScnStride;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	src_offset = src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	dst_offset = dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;
	cpp_offset = cpp_scan_offset;

	for( pixel = X1;  pixel <= X2;  pixel++ )
	{
	    if (GET_VALUE_(cpp->UdpA_Base, cpp_offset, 1)) {
		*((unsigned char *)(dst->UdpA_Base + (dst_offset >> 3))) =
		    *((unsigned short *)(src->UdpA_Base + (src_offset >> 3)));
	    }
	    src_offset += src->UdpL_PxlStride;
	    dst_offset += dst->UdpL_PxlStride;
	    cpp_offset += cpp->UdpL_PxlStride;
	}

	src_scan_offset += src->UdpL_ScnStride;
	dst_scan_offset += dst->UdpL_ScnStride;
	cpp_scan_offset += cpp->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _CvtWrdFltCpp
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from word array to floating point using a CPP
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int  _CvtWrdFltCpp( src, dst, cpp )
UdpPtr  src;
UdpPtr  dst;
UdpPtr  cpp;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int src_mask;
    int src_scan_offset, dst_scan_offset, cpp_scan_offset;
    int src_offset, dst_offset, cpp_offset;

/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base = DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL )
	    return( BadAlloc );
	}

/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    src_mask = (1 << src->UdpW_PixelLength) - 1;

    src_scan_offset =
	src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = 
	dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;
    cpp_scan_offset = cpp->UdpL_Pos + (Y1 - cpp->UdpL_Y1) * cpp->UdpL_ScnStride;

    for (scanline = Y1;  scanline <= Y2 ;  scanline++)
	{
	    src_offset = 
		src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	    dst_offset = 
		dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;
	    cpp_offset = cpp_scan_offset;

	    for (pixel = X1;  pixel <= X2;  pixel++)
		{
		    if (GET_VALUE_(cpp->UdpA_Base, cpp_offset, 1)) {
			*((float *)(dst->UdpA_Base + dst_offset / 8)) =
			    (float)*((unsigned short *)(src->UdpA_Base +
						       src_offset / 8));
		    }

		    src_offset += src->UdpL_PxlStride;
		    dst_offset += dst->UdpL_PxlStride;
		    cpp_offset += cpp->UdpL_PxlStride;
		}

	    src_scan_offset += src->UdpL_ScnStride;
	    dst_scan_offset += dst->UdpL_ScnStride;
	    cpp_scan_offset += cpp->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _CvtFltFltCpp
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from floating array to floating array using a CPP
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**      cpp - cpp data descriptor
**
*****************************************************************************/
static int _CvtFltFltCpp( src, dst, cpp )
UdpPtr  src;
UdpPtr  dst;
UdpPtr  cpp;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int src_scan_offset, dst_scan_offset, cpp_scan_offset;
    int src_offset, dst_offset, cpp_offset;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base =  DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL ) return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    src_scan_offset = src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;
    cpp_scan_offset = cpp->UdpL_Pos + (Y1 - cpp->UdpL_Y1) * cpp->UdpL_ScnStride;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	src_offset = src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	dst_offset = dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;
	cpp_offset = cpp_scan_offset;

	for( pixel = X1;  pixel <= X2;  pixel++ )
	    {
	    if( GET_VALUE_(cpp->UdpA_Base, cpp_offset, 1) )
		*((float *)(dst->UdpA_Base + (dst_offset >> 3))) =
		 *((float*)(src->UdpA_Base + (src_offset >> 3)));

	    src_offset += src->UdpL_PxlStride;
	    dst_offset += dst->UdpL_PxlStride;
	    cpp_offset += cpp->UdpL_PxlStride;
	    }
	src_scan_offset += src->UdpL_ScnStride;
	dst_scan_offset += dst->UdpL_ScnStride;
	cpp_scan_offset += cpp->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _CvtFltBsCpp
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from floating array to bit string using a CPP
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**      cpp - cpp data descriptor
**
*****************************************************************************/
static int _CvtFltBsCpp(src, dst, cpp)
UdpPtr  src;
UdpPtr  dst;
UdpPtr  cpp;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int dst_mask;
    int src_scan_offset, dst_scan_offset, cpp_scan_offset;
    int src_offset, dst_offset, cpp_offset;
    int value;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base =  DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL ) return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    dst_mask = (1 << dst->UdpW_PixelLength) - 1;

    src_scan_offset = src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;
    cpp_scan_offset = cpp->UdpL_Pos + (Y1 - cpp->UdpL_Y1) * cpp->UdpL_ScnStride;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	src_offset = src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	dst_offset = dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;
	cpp_offset = cpp_scan_offset;

	for( pixel = X1;  pixel <= X2;  pixel++ )
	    {
	    if( GET_VALUE_(cpp->UdpA_Base, cpp_offset, 1) )
		{
		value = *((float *)(src->UdpA_Base + src_offset / 8)) + 0.5;
		PUT_VALUE_( dst->UdpA_Base, dst_offset, value, dst_mask );
		}
	    src_offset += src->UdpL_PxlStride;
	    dst_offset += dst->UdpL_PxlStride;
	    cpp_offset += cpp->UdpL_PxlStride;
	    }
	src_scan_offset += src->UdpL_ScnStride;
	dst_scan_offset += dst->UdpL_ScnStride;
	cpp_scan_offset += cpp->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _CvtFltBytCpp
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from floating array to byte array using a CPP
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**      cpp - cpp data descriptor
**
*****************************************************************************/
static int _CvtFltBytCpp(src, dst, cpp)
UdpPtr  src;
UdpPtr  dst;
UdpPtr  cpp;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int src_scan_offset, dst_scan_offset, cpp_scan_offset;
    int src_offset, dst_offset, cpp_offset;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base =  DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL ) return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    src_scan_offset = src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;
    cpp_scan_offset = cpp->UdpL_Pos + (Y1 - cpp->UdpL_Y1) * cpp->UdpL_ScnStride;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	src_offset = src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	dst_offset = dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;
	cpp_offset = cpp_scan_offset;

	for( pixel = X1;  pixel <= X2;  pixel++ )
	    {
	    if( GET_VALUE_(cpp->UdpA_Base, cpp_offset, 1) )
		*((unsigned char *)(dst->UdpA_Base + dst_offset / 8)) =
			*((float *)(src->UdpA_Base + src_offset / 8)) + 0.5;

	    src_offset += src->UdpL_PxlStride;
	    dst_offset += dst->UdpL_PxlStride;
	    cpp_offset += cpp->UdpL_PxlStride;
	    }
	src_scan_offset += src->UdpL_ScnStride;
	dst_scan_offset += dst->UdpL_ScnStride;
	cpp_scan_offset += cpp->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _CvtFltWrdCpp
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert from floating array to word array using a CPP
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**      cpp - cpp data descriptor
**
*****************************************************************************/
static int  _CvtFltWrdCpp(src, dst, cpp)
UdpPtr  src;
UdpPtr  dst;
UdpPtr  cpp;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int src_scan_offset, dst_scan_offset, cpp_scan_offset;
    int src_offset, dst_offset, cpp_offset;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base = DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL )
	    return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    src_scan_offset =
	src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = 
	dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;
    cpp_scan_offset = cpp->UdpL_Pos + (Y1 - cpp->UdpL_Y1) * cpp->UdpL_ScnStride;

    for (scanline = Y1;  scanline <= Y2 ;  scanline++)
	{
	    src_offset = 
		src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	    dst_offset = 
		dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;
	    cpp_offset = cpp_scan_offset;

	    for (pixel = X1;  pixel <= X2;  pixel++)
		{
		    if (GET_VALUE_(cpp->UdpA_Base, cpp_offset, 1)) {
			*((unsigned short *)(dst->UdpA_Base + dst_offset / 8)) =
			    *((float *)(src->UdpA_Base + src_offset /8)) + 0.5;
		    }
		    src_offset += src->UdpL_PxlStride;
		    dst_offset += dst->UdpL_PxlStride;
		    cpp_offset += cpp->UdpL_PxlStride;
		}

	    src_scan_offset += src->UdpL_ScnStride;
	    dst_scan_offset += dst->UdpL_ScnStride;
	    cpp_scan_offset += cpp->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _ConHCBs
**
**  FUNCTIONAL DESCRIPTION:
**
**      Constrain using the Hard Clip model from float to bit string:
**          values < 0       ==> 0.0
**          values > ceiling ==> ceiling
**          Round all values to nearest integer.
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int _ConHCBs(src, dst)
UdpPtr  src;
UdpPtr  dst;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int dst_mask;
    int src_scan_offset, dst_scan_offset;
    int src_offset, dst_offset;
    int value;
    float ceiling;
    float fvalue;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base =  DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL ) return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    dst_mask = (1 << dst->UdpW_PixelLength) - 1;
    ceiling  = (float)(dst->UdpL_Levels - 1);

    src_scan_offset = src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	src_offset = src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	dst_offset = dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;

	for( pixel = X1;  pixel <= X2;  pixel++ )
	    {
	    fvalue = *((float *)(src->UdpA_Base + src_offset / 8));

	    if( fvalue < 0.0 )	/* Clip */
		fvalue = 0.0;
	    else
		if( fvalue > ceiling )
		    fvalue = ceiling;

	    value = fvalue + 0.5;
	    PUT_VALUE_( dst->UdpA_Base, dst_offset, value, dst_mask );

	    src_offset += src->UdpL_PxlStride;
	    dst_offset += dst->UdpL_PxlStride;
	    }
	src_scan_offset += src->UdpL_ScnStride;
	dst_scan_offset += dst->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _ConHCByt
**
**  FUNCTIONAL DESCRIPTION:
**
**      Constrain using the Hard Clip model from float to unsigned byte:
**          values < 0       ==> 0.0
**          values > ceiling ==> ceiling
**          Round all values to nearest integer.
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int _ConHCByt(src, dst)
UdpPtr  src;
UdpPtr  dst;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int src_scan_offset, dst_scan_offset;
    int src_offset, dst_offset;
    float ceiling;
    float fvalue;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base =  DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL ) return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    ceiling = (float)(dst->UdpL_Levels - 1);

    src_scan_offset = src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	src_offset = src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	dst_offset = dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;

	for( pixel = X1;  pixel <= X2;  pixel++ )
	    {
	    fvalue = *((float *)(src->UdpA_Base + src_offset / 8));

	    if( fvalue < 0.0 )	/* Clip */
		fvalue = 0.0;
	    else
		if( fvalue > ceiling )
		    fvalue = ceiling;

	    *((unsigned char *)(dst->UdpA_Base + dst_offset / 8)) = fvalue+0.5;
	    src_offset += src->UdpL_PxlStride;
	    dst_offset += dst->UdpL_PxlStride;
	    }
	src_scan_offset += src->UdpL_ScnStride;
	dst_scan_offset += dst->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _ConHCWrd
**
**  FUNCTIONAL DESCRIPTION:
**
**      Constrain using the Hard Clip model from float to unsigned word:
**          values < 0       ==> 0.0
**          values > ceiling ==> ceiling
**          Round all values to nearest integer.
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int  _ConHCWrd( src, dst )
UdpPtr  src;
UdpPtr  dst;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int src_scan_offset, dst_scan_offset;
    int src_offset, dst_offset;
    float ceiling;
    float fvalue;

/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base = DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL )
	    return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    ceiling = (float)(dst->UdpL_Levels - 1);

    src_scan_offset =
	src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = 
	dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;

    for (scanline = Y1;  scanline <= Y2 ;  scanline++)
	{
	    src_offset = 
		src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	    dst_offset = 
		dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;

	    for (pixel = X1;  pixel <= X2;  pixel++)
		{
		fvalue = *((float *)(src->UdpA_Base + src_offset / 8));

		if (fvalue < 0.0)	/* Clip */
		    fvalue = 0.0;
		else
		    if (fvalue > ceiling)
			fvalue = ceiling;

		*((unsigned short *)(dst->UdpA_Base + dst_offset / 8)) =
		    fvalue + 0.5;
		src_offset += src->UdpL_PxlStride;
		dst_offset += dst->UdpL_PxlStride;
		}

	    src_scan_offset += src->UdpL_ScnStride;
	    dst_scan_offset += dst->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _ConCSBs
**
**  FUNCTIONAL DESCRIPTION:
**
**      Constrain using the Clip Scale model from float to bit string:
**          values < 0       ==> 0.0
**          values > ceiling ==> ceiling
**          Scale such that smallest value -> 0
**                          largest value -> ceiling
**                          
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int _ConCSBs(src, dst)
UdpPtr  src;
UdpPtr  dst;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int dst_mask;
    int src_scan_offset, dst_scan_offset;
    int src_offset, dst_offset;
    int value;
    float ceiling;
    float fvalue;
    float fmax, fmin, foffset, fscale;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base =  DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL ) return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    dst_mask = (1 << dst->UdpW_PixelLength) - 1;
    ceiling = (float)(dst->UdpL_Levels - 1);

    /* Make a pass through the pixels to determine the range of the data */
    fmin = ceiling;
    fmax = 0.0;

    src_scan_offset = src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	src_offset = src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;

	for( pixel = X1;  pixel <= X2;  pixel++ )
	    {
	    fvalue = *((float *)(src->UdpA_Base + src_offset / 8));

	    /* Clip the data */
	    if( fvalue < 0.0 )
		fvalue = 0.0;
	    else
		if( fvalue > ceiling )
		    fvalue = ceiling;

	    fmax = Max_(fmax,fvalue);
	    fmin = Min_(fmin,fvalue);

	    src_offset += src->UdpL_PxlStride;
	    }
	src_scan_offset += src->UdpL_ScnStride;
	}
    /* Calculate the scale value and offset */
    foffset = fmin;
    if( fmax - fmin != 0.0 )
	fscale = ceiling / ( fmax - fmin );
    else
	fscale = 1.0;

    /* Scale and convert the data */
    src_scan_offset = src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	src_offset = src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	dst_offset = dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;

	for( pixel = X1;  pixel <= X2;  pixel++ )
	    {
	    fvalue = *((float *)(src->UdpA_Base + src_offset / 8));

	    if( fvalue < 0.0 )	/* Clip */
		fvalue = 0.0;
	    else
		if( fvalue > ceiling )
		    fvalue = ceiling;

	    value = (fvalue - foffset) * fscale + 0.5; /* Scale */
	    PUT_VALUE_(dst->UdpA_Base,dst_offset,value,dst_mask);

	    src_offset += src->UdpL_PxlStride;
	    dst_offset += dst->UdpL_PxlStride;
	    }
	src_scan_offset += src->UdpL_ScnStride;
	dst_scan_offset += dst->UdpL_ScnStride;
	}
    return (Success);
}

/*****************************************************************************
**  _ConCSByt
**
**  FUNCTIONAL DESCRIPTION:
**
**      Constrain using the Clip Scale model from float to bit string:
**          values < 0       ==> 0.0
**          values > ceiling ==> ceiling
**          Scale such that smallest value -> 0
**                          largest value -> ceiling
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int _ConCSByt(src, dst)
UdpPtr  src;
UdpPtr  dst;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int src_scan_offset, dst_scan_offset;
    int src_offset, dst_offset;
    float ceiling;
    float fvalue;
    float fmax, fmin, foffset, fscale;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base =  DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL ) return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    ceiling = (float)(dst->UdpL_Levels - 1);

    src_scan_offset = src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;

    /* Make a pass through the pixels to determine the range of the data */
    fmin = ceiling;
    fmax = 0.0;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	src_offset = src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;

	for( pixel = X1;  pixel <= X2;  pixel++ )
	    {
	    fvalue = *((float *)(src->UdpA_Base + src_offset / 8));

	    if( fvalue < 0.0 )	/* Clip */
		fvalue = 0.0;
	    else
		if( fvalue > ceiling )
		    fvalue = ceiling;

	    fmax = Max_(fmax,fvalue);
	    fmin = Min_(fmin,fvalue);

	    src_offset += src->UdpL_PxlStride;
	    }
	src_scan_offset += src->UdpL_ScnStride;
	}

    /* Calculate the scale value and offset */
    foffset = fmin;
    if( fmax - fmin != 0.0 )
	fscale = ceiling / ( fmax - fmin );
    else
	fscale = 1.0;

    /* Scale and convert the data */
    src_scan_offset = src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	src_offset = src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	dst_offset = dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;

	for( pixel = X1;  pixel <= X2;  pixel++ )
	    {
	    fvalue = *((float *)(src->UdpA_Base + src_offset / 8));

	    if( fvalue < 0.0 )	/* Clip */
		fvalue = 0.0;
	    else
		if( fvalue > ceiling )
		    fvalue = ceiling;

	    *((unsigned char *)(dst->UdpA_Base + dst_offset / 8)) =
		(fvalue - foffset) * fscale + 0.5; /* Scale */

	    src_offset += src->UdpL_PxlStride;
	    dst_offset += dst->UdpL_PxlStride;
	    }
	src_scan_offset += src->UdpL_ScnStride;
	dst_scan_offset += dst->UdpL_ScnStride;
	}
    return (Success);
}

/*****************************************************************************
**  _ConCSWrd
**
**  FUNCTIONAL DESCRIPTION:
**
**      Constrain using the Clip Scale model from float to word array:
**          values < 0       ==> 0.0
**          values > ceiling ==> ceiling
**          Scale such that smallest value -> 0
**                          largest value -> ceiling
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int  _ConCSWrd( src, dst )
UdpPtr  src;
UdpPtr  dst;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int src_scan_offset, dst_scan_offset;
    int src_offset, dst_offset;
    float ceiling;
    float fvalue;
    float fmax, fmin, foffset, fscale;

/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base = DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL )
	    return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    ceiling = (float)(dst->UdpL_Levels - 1);

    src_scan_offset =
	src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;

/*
** Make a pass through the pixels to determine the range of the data 
*/
    fmin = ceiling;
    fmax = 0.0;

    for (scanline = Y1;  scanline <= Y2 ;  scanline++)
	{
	    src_offset = 
		src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;

	    for (pixel = X1;  pixel <= X2;  pixel++)
		{
		fvalue = *((float *)(src->UdpA_Base + src_offset / 8));

		if (fvalue < 0.0)	/* Clip */
		    fvalue = 0.0;
		else
		    if (fvalue > ceiling)
			fvalue = ceiling;

		fmax = Max_(fmax,fvalue);
		fmin = Min_(fmin,fvalue);

		src_offset += src->UdpL_PxlStride;
		}

	    src_scan_offset += src->UdpL_ScnStride;
	}

/*
** Calculate the scale value and offset 
*/
    foffset = fmin;
    if ((fmax-fmin) != 0.0)
	fscale = ceiling/(fmax-fmin);
    else
	fscale = 1.0;

/*
** Scale and convert the data 
*/
    src_scan_offset =
	src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = 
	dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;

    for (scanline = Y1;  scanline <= Y2 ;  scanline++)
	{
	    src_offset = 
		src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	    dst_offset = 
		dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;

	    for (pixel = X1;  pixel <= X2;  pixel++)
		{
		fvalue = *((float *)(src->UdpA_Base + src_offset / 8));

		if (fvalue < 0.0)	/* Clip */
		    fvalue = 0.0;
		else
		    if (fvalue > ceiling)
			fvalue = ceiling;

		*((unsigned short *)(dst->UdpA_Base + dst_offset / 8)) =
		    (fvalue - foffset) * fscale + 0.5; /* Scale */

		src_offset += src->UdpL_PxlStride;
		dst_offset += dst->UdpL_PxlStride;
		}

	    src_scan_offset += src->UdpL_ScnStride;
	    dst_scan_offset += dst->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _ConHSBs
**
**  FUNCTIONAL DESCRIPTION:
**
**      Constrain using the Hard Scale model from float to bit string:
**          Scale such that smallest value -> 0
**                          largest value -> ceiling
**                          
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int _ConHSBs(src, dst)
UdpPtr  src;
UdpPtr  dst;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int dst_mask;
    int src_scan_offset, dst_scan_offset;
    int src_offset, dst_offset;
    int value;
    float ceiling;
    float fvalue;
    float fmax, fmin, foffset, fscale;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base =  DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL ) return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    dst_mask = (1 << dst->UdpW_PixelLength) - 1;
    ceiling  = (float)(dst->UdpL_Levels - 1);

    /* Make a pass through the pixels to determine the range of the data */
    src_scan_offset = src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;

    fmin = *((float *)(src->UdpA_Base + (src_scan_offset +
			(X1-src->UdpL_X1) * src->UdpL_PxlStride)/ 8));
    fmax = fmin;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	src_offset = src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;

	for( pixel = X1;  pixel <= X2;  pixel++ )
	    {
	    fvalue = *((float *)(src->UdpA_Base + src_offset / 8));

	    fmax = Max_(fmax,fvalue);
	    fmin = Min_(fmin,fvalue);

	    src_offset += src->UdpL_PxlStride;
	    }
	src_scan_offset += src->UdpL_ScnStride;
	}
    /* Calculate the scale value and offset */
    foffset = fmin;
    if( fmax - fmin != 0.0 )
	fscale = ceiling / ( fmax - fmin );
    else
	fscale = 1.0;

    /* Scale and convert the data */
    src_scan_offset = src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	src_offset = src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	dst_offset = dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;

	for( pixel = X1;  pixel <= X2;  pixel++ )
	    {
	    fvalue = *((float *)(src->UdpA_Base + src_offset / 8));

	    value = (fvalue - foffset) * fscale + 0.5; /* Scale */
	    PUT_VALUE_(dst->UdpA_Base,dst_offset,value,dst_mask);

	    src_offset += src->UdpL_PxlStride;
	    dst_offset += dst->UdpL_PxlStride;
	    }
	src_scan_offset += src->UdpL_ScnStride;
	dst_scan_offset += dst->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _ConHSByt
**
**  FUNCTIONAL DESCRIPTION:
**
**      Constrain using the Hard Scale model from float to bit string:
**          Scale such that smallest value -> 0
**                          largest value -> ceiling
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int _ConHSByt(src, dst)
UdpPtr  src;
UdpPtr  dst;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int src_scan_offset, dst_scan_offset;
    int src_offset, dst_offset;
    float ceiling;
    float fvalue;
    float fmax, fmin, foffset, fscale;
/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base =  DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL ) return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    ceiling = (float)(dst->UdpL_Levels - 1);

    src_scan_offset = src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;

    /* Make a pass through the pixels to determine the range of the data */
    fmin = *((float *)(src->UdpA_Base + (src_scan_offset +
			(X1-src->UdpL_X1) * src->UdpL_PxlStride)/ 8));
    fmax = fmin;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	src_offset = src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;

	for( pixel = X1;  pixel <= X2;  pixel++ )
	    {
	    fvalue = *((float *)(src->UdpA_Base + src_offset / 8));

	    fmax = Max_(fmax,fvalue);
	    fmin = Min_(fmin,fvalue);

	    src_offset += src->UdpL_PxlStride;
	    }
	src_scan_offset += src->UdpL_ScnStride;
	}

    /* Calculate the scale value and offset */
    foffset = fmin;
    if( fmax - fmin != 0.0 )
	fscale = ceiling / ( fmax - fmin );
    else
	fscale = 1.0;

    /* Scale and convert the data */
    src_scan_offset = src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;

    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	src_offset = src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	dst_offset = dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;

	for( pixel = X1;  pixel <= X2;  pixel++ )
	    {
	    fvalue = *((float *)(src->UdpA_Base + src_offset / 8));

	    *((unsigned char *)(dst->UdpA_Base + dst_offset / 8)) =
		(fvalue - foffset) * fscale + 0.5;     /* Scale */

	    src_offset += src->UdpL_PxlStride;
	    dst_offset += dst->UdpL_PxlStride;
	    }
	src_scan_offset += src->UdpL_ScnStride;
	dst_scan_offset += dst->UdpL_ScnStride;
	}
    return ( Success );
}

/*****************************************************************************
**  _ConHSWrd
**
**  FUNCTIONAL DESCRIPTION:
**
**      Constrain using the Hard Scale model from float to word array:
**          Scale such that smallest value -> 0
**                          largest value -> ceiling
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int  _ConHSWrd( src, dst )
UdpPtr  src;
UdpPtr  dst;
{
    int pixel;
    int scanline;
    int X1, Y1, X2, Y2;
    int src_scan_offset, dst_scan_offset;
    int src_offset, dst_offset;
    float ceiling;
    float fvalue;
    float fmax, fmin, foffset, fscale;

/*
**  Allocate memory for destination buffer.
*/
    if( dst->UdpA_Base == NULL )
	{
	dst->UdpA_Base = DdxMallocBits_( dst->UdpL_ArSize );
	if( dst->UdpA_Base == NULL )
	    return( BadAlloc );
	}
/*
**  Determine maximum common dimensions between source and destination.
*/
    X1 = src->UdpL_X1 > dst->UdpL_X1 ? src->UdpL_X1 : dst->UdpL_X1;
    X2 = src->UdpL_X2 < dst->UdpL_X2 ? src->UdpL_X2 : dst->UdpL_X2;
    Y1 = src->UdpL_Y1 > dst->UdpL_Y1 ? src->UdpL_Y1 : dst->UdpL_Y1;
    Y2 = src->UdpL_Y2 < dst->UdpL_Y2 ? src->UdpL_Y2 : dst->UdpL_Y2;

    ceiling = (float)(dst->UdpL_Levels - 1);

    src_scan_offset =
	src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;

/*
** Make a pass through the pixels to determine the range of the data 
*/
    fmin = *((float *)(src->UdpA_Base + (src_scan_offset +
			(X1-src->UdpL_X1) * src->UdpL_PxlStride)/ 8));
    fmax = fmin;

    for (scanline = Y1;  scanline <= Y2 ;  scanline++)
	{
	    src_offset = 
		src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;

	    for (pixel = X1;  pixel <= X2;  pixel++)
		{
		fvalue = *((float *)(src->UdpA_Base + src_offset / 8));

		fmax = Max_(fmax,fvalue);
		fmin = Min_(fmin,fvalue);

		src_offset += src->UdpL_PxlStride;
		}

	    src_scan_offset += src->UdpL_ScnStride;
	}

/*
** Calculate the scale value and offset 
*/
    foffset = fmin;
    if ((fmax-fmin) != 0.0)
	fscale = ceiling/(fmax-fmin);
    else
	fscale = 1.0;

/*
** Scale and convert the data 
*/
    src_scan_offset =
	src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = 
	dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;

    for (scanline = Y1;  scanline <= Y2 ;  scanline++)
	{
	    src_offset = 
		src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	    dst_offset = 
		dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;

	    for (pixel = X1;  pixel <= X2;  pixel++)
		{
		fvalue = *((float *)(src->UdpA_Base + src_offset / 8));

		*((unsigned short *)(dst->UdpA_Base + dst_offset / 8)) =
		    (fvalue - foffset) * fscale + 0.5;     /* Scale */

		src_offset += src->UdpL_PxlStride;
		dst_offset += dst->UdpL_PxlStride;
		}

	    src_scan_offset += src->UdpL_ScnStride;
	    dst_scan_offset += dst->UdpL_ScnStride;
	}
    return( Success );
}

/************************************************************************
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine builds a change list from a buffer of image data,
**      where the buffer contains pixels of an arbitrary bit length.
**      Run length transitions are detected by alternately searching for zero
**	pixels or one pixels.  Bit offsets where the transitions take place
**	are stored in subsequent entries in the change list.
**
**  FORMAL PARAMETERS:
**
**		src_addr;   - scanline buffer address
**		src_pos;    - scanline buffer offset (in bits)
**		size;	    - scanline buffer length (in pixels)
**		stride;     - the source pixel stride
**		cl_addr;    - change list address (ptr to array of long)
**		cl_len;	    - chane list length (num of cells in array)
**
************************************************************************/
static void _BuildClBits( src_addr, src_pos, size, stride, cl_addr, cl_len )
long	src_addr;
long	src_pos;
long	size;
long	stride;
long	cl_addr;
long	cl_len;
{
    long 		pos;
    long 		end;
    int			index;
    unsigned int	*cl;


    cl  = (unsigned int *) cl_addr;
    end = src_pos + size * stride;
    pos = src_pos;

    /*
    **  While there's room in the changlist and data to search,  find
    **  transitions and load them into the change list.
    */
    for( index = 1; index < cl_len && size > 0; index++, size = end - pos )
	{
	if( index % 2 )
	    /*
	    ** look for a transition to a 1 bit
	    */
	    while( pos < end )
		{
		if( GET_VALUE_(src_addr, pos, 1 ) == 1 )
		    break;
		pos += stride;
		}
	else
	    /*
	    ** look for a transition to a 0 bit
	    */
	    while( pos < end )
		{
		if( GET_VALUE_(src_addr, pos, 1 ) == 0 )
		    break;
		pos += stride;
		}
	/*
	** save the transition point in the changelist
	*/
	cl[index] = (pos - src_pos)/stride;
	}
    /*
    ** store the number of valid entries
    */
    cl[0] = --index;
}					/* end _BuildClBits */

/************************************************************************
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine builds a change list from a buffer of image data,
**      where the buffer contains byte-length pixels.
**      Run length transitions are detected by alternately searching for zero
**	pixels or one pixels.  Pixel offsets where the transitions take place
**	are stored in subsequent entries in the change list.
**
**  FORMAL PARAMETERS:
**
**		src_addr;   - scanline buffer address
**		size;	    - scanline buffer length (in pixels)
**		stride;     - pixel stride (bytes)
**		cl_addr;    - change list address (ptr to array of long)
**		cl_len;	    - change list length (num of cells in array)
**
************************************************************************/
static void _BuildClBytes( src_addr, size, stride, cl_addr, cl_len )
unsigned char *src_addr;
long	size;
long	stride;
long	cl_addr;
long	cl_len;
{
    long 		pos;
    long 		end;
    int			index;
    unsigned int	*cl;

    cl  = (unsigned int *) cl_addr;
    end = size * stride;
    pos = 0;

    /*
    **  While there's room in the changlist and data to search,  find
    **  transitions and load them into the change list.
    */
    for( index = 1; index < cl_len && size > 0; index++, size = end - pos )
	{
	if( index % 2 )
	    /*
	    ** look for a transition to a 1 bit
	    */
	    while( pos < end )
		{
		if( src_addr[pos] == 1 )
		    break;
		pos += stride;
		}
	else
	    /*
	    ** look for a transition to a 0 bit
	    */
	    while( pos < end )
		{
		if( src_addr[pos] == 0 )
		    break;
		pos += stride;
		}
	/*
	** save the transition point in the changelist
	*/
	cl[index] = pos/stride;
	}
    /*
    ** store the number of valid entries
    */
    cl[0] = --index;
}					/* end _BuildCLBytes */

/************************************************************************
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine builds a change list from a buffer of image data,
**      where the buffer contains word-length pixels.
**      Run length transitions are detected by alternately searching for zero
**	pixels or one pixels.  Pixel offsets where the transitions take place
**	are stored in subsequent entries in the change list.
**
**  FORMAL PARAMETERS:
**
**		src_addr;   - scanline buffer address
**		size;	    - scanline buffer length (in pixels)
**		stride;     - pixel stride (words)
**		cl_addr;    - change list address (ptr to array of long)
**		cl_len;	    - change list length (num of cells in array)
**
************************************************************************/
static void _BuildClWords( src_addr, size, stride, cl_addr, cl_len )
unsigned short *src_addr;
long	size;
long	stride;
long	cl_addr;
long	cl_len;
{
    long 		pos;
    long 		end;
    int			index;
    unsigned int	*cl;

    cl  = (unsigned int *) cl_addr;
    end = size * stride;
    pos = 0;

    /*
    **  While there's room in the changlist and data to search,  find
    **  transitions and load them into the change list.
    */
    for( index = 1; index < cl_len && size > 0; index++, size = end - pos )
	{
	if( index % 2 )
	    /*
	    ** look for a transition to a 1 bit
	    */
	    while( pos < end )
		{
		if( src_addr[pos] == 1 )
		    break;
		pos += stride;
		}
	else
	    /*
	    ** look for a transition to a 0 bit
	    */
	    while( pos < end )
		{
		if( src_addr[pos] == 0 )
		    break;
		pos += stride;
		}
	/*
	** save the transition point in the changelist
	*/
	cl[index] = pos/stride;
	}
    /*
    ** store the number of valid entries
    */
    cl[0] = --index;
}					/* end _BuildCLWords */
/* end Module DmiConvert */
