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

/***********************************************************
Copyright 1989 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/*****************************************************************************
**
**  FACILITY:
**
**      X Imaging Extension
**      Sample Machine Independant DDX
**
**  ABSTRACT:
**
**      This module contains all image data type and organization conversion 
**	routines.
**
**  ENVIRONMENT:
**
**	VAX/VMS V5.3
**	ULTRIX  V3.1
**
**  AUTHOR(S):
**
**      John Weber
**	Rich Hennessy - Add 16 bit support(10/91)
**
**  CREATION DATE:
**
**      May 15, 1989
**
**  MODIFICATION HISTORY:
**
*****************************************************************************/

/*
**  Definition to let include files know who's calling.
*/
#define _SmiConvert

/*
**  Include files
*/
#include <stdio.h>
#include <X.h>
#include <XieDdx.h>                     /* XIE device dependant definitions */
#include <XieUdpDef.h>

#ifdef CLIENT
/*
** When this module is built for use on the client side, redirect the
** references which would go through the DDX dispatch table.
*/
#include <XieLib.h>

#undef  DdxConvert_
#define DdxConvert_	SmiConvert
#undef  DdxCopy_
#define DdxCopy_	SmiCopy
#undef  DdxConvertCpp_
#define DdxConvertCpp_	SmiConvertCpp
#undef  DdxGetTyp_
#define DdxGetTyp_	SmiGetTyp
#undef  DdxMallocBits_
#define DdxMallocBits_	XieMallocBits
#endif


/*
**  Table of contents
*/
int	    SmiConvert();
int	    SmiCopy();
int 	    SmiConvertCpp();
int	    SmiCopyCpp();
int	    SmiGetTyp();

static int  _CvtUnsOpr();

static int  _CvtBsBs();
static int  _CvtBsByt();
static int  _CvtBsFlt();
static int  _CvtBsWrd();

static int  _CvtBytByt();
static int  _CvtBytBs();
static int  _CvtBytFlt();
static int  _CvtBytWrd();

static int  _CvtFltFlt();
static int  _CvtFltBs();
static int  _CvtFltByt();
static int  _CvtFltWrd();

static int  _CvtWrdWrd();
static int  _CvtWrdBs();
static int  _CvtWrdByt();
static int  _CvtWrdFlt();

static int  _CvtBsBsCpp();
static int  _CvtBsBytCpp();
static int  _CvtBsFltCpp();
static int  _CvtBsWrdCpp();

static int  _CvtBytBytCpp();
static int  _CvtBytBsCpp();
static int  _CvtBytFltCpp();
static int  _CvtBytWrdCpp();

static int  _CvtFltFltCpp();
static int  _CvtFltBsCpp();
static int  _CvtFltBytCpp();
static int  _CvtFltWrdCpp();

static int  _CvtWrdWrdCpp();
static int  _CvtWrdBsCpp();
static int  _CvtWrdBytCpp();
static int  _CvtWrdFltCpp();


static int  _ConHCBs();
static int  _ConHCByt();
static int  _ConHCWrd();

static int  _ConCSBs();
static int  _ConCSByt();
static int  _ConCSWrd();

static int  _ConHSBs();
static int  _ConHSByt();
static int  _ConHSWrd();


/*
**  MACRO definitions
*/
#define Min_(a,b) ((a) <= (b) ? (a) : (b))
#define Max_(a,b) ((a)  > (b) ? (a) : (b))

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
    **  The table's indecis are from XieDDX.h, UdpRec data types constants.
    */
static int (*conversion_table[XieK_DATA_TYPE_COUNT][XieK_DATA_TYPE_COUNT])()={
 {_CvtBsBs,  _CvtBsBs,  _CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtBsByt, _CvtBsFlt, _CvtBsWrd},
 {_CvtBsBs,  _CvtBsBs,  _CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtBsByt, _CvtBsFlt, _CvtBsWrd},
 {_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr},
 {_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr},
 {_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr},
 {_CvtBytBs, _CvtBytBs, _CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtBytByt,_CvtBytFlt,_CvtBytWrd},
 {_CvtFltBs, _CvtFltBs, _CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtFltByt,_CvtFltFlt,_CvtFltWrd},
 {_CvtWrdBs, _CvtWrdBs, _CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtWrdByt,_CvtWrdFlt,_CvtWrdWrd}
};

    /*
    **  This table is for conversions under the control of a CPP 
    */
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
static int  (*constrain_table[5][XieK_DATA_TYPE_COUNT])()={
 {_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr},
 {_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr},
 {_ConHCBs,  _ConHCBs,  _CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_ConHCByt, _CvtUnsOpr,_ConHCWrd},
 {_ConCSBs,  _ConCSBs,  _CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_ConCSByt, _CvtUnsOpr,_ConCSWrd},
 {_ConHSBs,  _ConHSBs,  _CvtUnsOpr,_CvtUnsOpr,_CvtUnsOpr,_ConHSByt, _CvtUnsOpr,_ConHSWrd}
};

/*****************************************************************************
**  SmiConvert
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
int 	    SmiConvert(src, dst, mode)
UdpPtr  src;
UdpPtr  dst;
int     mode;
{
    int srctyp, dsttyp, status;
    /*
    **	Validate and get type of descriptors
    */
    srctyp = DdxGetTyp_(src);
    dsttyp = DdxGetTyp_(dst);

    if( srctyp >= XieK_DATA_TYPE_COUNT || dsttyp >= XieK_DATA_TYPE_COUNT )
	status  = _CvtUnsOpr(src,dst);

    else if( mode == XieK_MoveMode )
	/*
	**  Call specific conversion routine.
	*/
	status = (*conversion_table[srctyp][dsttyp])(src,dst);

    else if( srctyp != XieK_AF )
	status  = _CvtUnsOpr(src,dst);
	/*
	**  The input data must be type float.  Call conversion
	**  routine based on the constraint model requested.
	*/
    else
	status  = (*constrain_table[mode][dsttyp])(src,dst);

    return( status );
}				    /* end SmiConvert */

/*****************************************************************************
**  SmiConvertCpp
**
**  FUNCTIONAL DESCRIPTION:
**
**      This is the same as SmiConvert, except that a CPP is used to
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
int 	    SmiConvertCpp(src, dst, cpp, mode)
UdpPtr  src;
UdpPtr  dst;
UdpPtr  cpp;
int     mode;
{
    int srctyp, dsttyp, status;
    /*
    **	Validate and get type of descriptors
    */
    srctyp = DdxGetTyp_(src);
    dsttyp = DdxGetTyp_(dst);

    if( srctyp >= XieK_DATA_TYPE_COUNT || dsttyp >= XieK_DATA_TYPE_COUNT
			|| cpp == NULL || mode   != XieK_MoveMode )
	status = _CvtUnsOpr(src,dst);

    else
	status = (*conversion_table_cpp[srctyp][dsttyp])(src,dst,cpp);

    return( status );
}				    /* end SmiConvertCpp */

/*****************************************************************************
**  SmiCopy
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
**  FUNCTION VALUE:
**
*****************************************************************************/
int 		SmiCopy( src, dst, srcx, srcy, dstx, dsty, width, height )
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

    clipped_width = Min_(srcx + width -1, src->UdpL_X2) - srcx + 1;
    clipped_height = Min_(srcy + height -1, src->UdpL_Y2) - srcy + 1;

    srcudp.UdpL_X2 = dstx + clipped_width  - 1;
    srcudp.UdpL_Y2 = dsty + clipped_height - 1;

    srcudp.UdpL_PxlPerScn = clipped_width;
    srcudp.UdpL_ScnCnt = clipped_height;

    srcudp.UdpL_Pos += srcy * srcudp.UdpL_ScnStride
		     + srcx * srcudp.UdpL_PxlStride;
    /*
    **	Now, call the CONVERT routine to do the data movement.
    */
    return( DdxConvert_( &srcudp, dst, XieK_MoveMode ) );
}				    /* end SmiCopy */

/*****************************************************************************
**  SmiCopyCpp
**
**  FUNCTIONAL DESCRIPTION:
**
**     This is the same as SmiCopy, except the pixels to be copied
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
**  FUNCTION VALUE:
**
*****************************************************************************/
int 	SmiCopyCpp( src, dst, cpp, srcx, srcy, dstx, dsty, width, height )
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

    clipped_width = Min_(srcx + width -1, src->UdpL_X2) - srcx + 1;
    clipped_height = Min_(srcy + height -1, src->UdpL_Y2) - srcy + 1;

    srcudp.UdpL_X2 = dstx + clipped_width  - 1;
    srcudp.UdpL_Y2 = dsty + clipped_height - 1;

    srcudp.UdpL_PxlPerScn = clipped_width;
    srcudp.UdpL_ScnCnt = clipped_height;

    srcudp.UdpL_Pos += srcy * srcudp.UdpL_ScnStride
		     + srcx * srcudp.UdpL_PxlStride;
    /*
    **	Now, call the CONVERT routine to do the data movement.
    */
    return( DdxConvertCpp_( &srcudp, dst, cpp, XieK_MoveMode ) );
}				    /* end SmiCopyCpp */

/*****************************************************************************
**  SmiGetTyp
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
**      data type of descriptor
**
*****************************************************************************/
int	SmiGetTyp(desc)
UdpPtr	desc;
{
    int type;

    switch (desc->UdpB_DType)
	{
    case UdpK_DTypeBU :
	switch (desc->UdpB_Class)
	    {
	case UdpK_ClassA:   type = XieK_ABU;		    break;
	case UdpK_ClassUBA:
	case UdpK_ClassUBS:
	default:	    type = XieK_DATA_TYPE_COUNT;    break;
	    } break;
    case UdpK_DTypeWU:
	switch (desc->UdpB_Class)
	    {
	case UdpK_ClassA:    type = XieK_AWU;		    break;
	case UdpK_ClassUBA:
	case UdpK_ClassUBS:
	default:	     type = XieK_DATA_TYPE_COUNT;   break;
	    } break;
    case UdpK_DTypeLU:
	switch (desc->UdpB_Class)
	    {
	case UdpK_ClassA:
	case UdpK_ClassUBA:
	case UdpK_ClassUBS:
	default:	    type = XieK_DATA_TYPE_COUNT;    break;
	    } break;
    case UdpK_DTypeF:
	switch (desc->UdpB_Class)
	    {
	case UdpK_ClassA:   type = XieK_AF;		    break;
	case UdpK_ClassUBA:
	case UdpK_ClassUBS:
	default:	    type = XieK_DATA_TYPE_COUNT;    break;
	    } break;
    case UdpK_DTypeVU:
	switch (desc->UdpB_Class)
	    {
	case UdpK_ClassUBA: type = XieK_UBAVU;		    break;
	case UdpK_ClassUBS: type = XieK_UBSVU;		    break;
	case UdpK_ClassA:
	default:	    type = XieK_DATA_TYPE_COUNT;    break;
	    } break;
    case UdpK_DTypeV:
	switch (desc->UdpB_Class)
	    {
	case UdpK_ClassUBA: type = XieK_UBAV;		    break;
	case UdpK_ClassUBS: type = XieK_UBSV;		    break;
	case UdpK_ClassA:
	default:	    type = XieK_DATA_TYPE_COUNT;    break;
	    } break;
    default :		    type = XieK_DATA_TYPE_COUNT;    break;
	}
    return( type );
}				    /* end SmiGetTyp */

/*****************************************************************************
**  SmiPosCl
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
**      base   - data to position in
**	inipos - initial offset into data
**      scncnt - number of scanlines to position to
**
**  FUNCTION VALUE:
**
*****************************************************************************/
int		 SmiPosCl(base, inipos, scncnt)
unsigned int	*base;
int		 inipos;
int		 scncnt;
{
    /* Not implemented */
    return( BadImplementation );
}					/* end SmiPosCl */

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
    dst_mask = (1 << dst->UdpW_PixelLength) - 1;

    src_scan_offset = src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;
    dst_scan_offset = dst->UdpL_Pos + (Y1 - dst->UdpL_Y1) * dst->UdpL_ScnStride;
    for( scanline = Y1;  scanline <= Y2 ;  scanline++ )
	{
	src_offset = src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;
	dst_offset = dst_scan_offset + (X1-dst->UdpL_X1) * dst->UdpL_PxlStride;

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
		    value = GET_VALUE_(src->UdpA_Base, src_offset, mask_24);
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
		/*
		**  Src and dst are aligned, so we can use the fast byte mover.
		*/
		{
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
static int  _CvtBsByt(src, dst)
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
		    *((unsigned char *)(dst->UdpA_Base + dst_offset / 8)) =
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
static int  _CvtBsFlt(src, dst)
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
static int  _CvtBytByt( src, dst )
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
static int  _CvtBytBs(src, dst)
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
		    PUT_VALUE_(dst->UdpA_Base,dst_offset,
			*((unsigned char *)(src->UdpA_Base + src_offset / 8)),dst_mask);
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
static int  _CvtBytFlt(src, dst)
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
			(float)*((unsigned char *)(src->UdpA_Base + src_offset / 8));

		    src_offset += src->UdpL_PxlStride;
		    dst_offset += dst->UdpL_PxlStride;
		}

	    src_scan_offset += src->UdpL_ScnStride;
	    dst_scan_offset += dst->UdpL_ScnStride;
	}
    return( Success );
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
static int  _CvtFltFlt( src, dst )
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
static int  _CvtFltBs(src, dst)
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
static int  _CvtFltByt(src, dst)
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

/*   N O W   A L L   T H E   "W O R D"   F U N C T I O N		    */

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
	    *((unsigned char *)(dst->UdpA_Base + (dst_offset >> 3))) =
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

/*  N O W   A L L   T H E   "C O N S T R A I N T"   F U N C T I O N S	    */

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
static int  _ConHCBs(src, dst)
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

		    value = fvalue + 0.5;
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
static int  _ConHCByt(src, dst)
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

		*((unsigned char *)(dst->UdpA_Base + dst_offset / 8)) =
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
static int  _ConCSBs(src, dst)
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
    ceiling = (float)(dst->UdpL_Levels - 1);

    /* Make a pass through the pixels to determine the range of the data */
    fmin = ceiling;
    fmax = 0.0;

    src_scan_offset =
	src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;

    for (scanline = Y1;  scanline <= Y2 ;  scanline++)
	{
	    src_offset = 
		src_scan_offset + (X1-src->UdpL_X1) * src->UdpL_PxlStride;

	    for (pixel = X1;  pixel <= X2;  pixel++)
		{
		    fvalue = *((float *)(src->UdpA_Base + src_offset / 8));

		    /* Clip the data */
		    if (fvalue < 0.0)
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

    /* Calculate the scale value and offset */
    foffset = fmin;
    if ((fmax-fmin) != 0.0)
	fscale = ceiling/(fmax-fmin);
    else
	fscale = 1.0;

    /* Scale and convert the data */
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
**  _ConCSByt
**
**  FUNCTIONAL DESCRIPTION:
**
**      Constrain using the Clip Scale model from float to byte array:
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
static int  _ConCSByt(src, dst)
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

		*((unsigned char *)(dst->UdpA_Base + dst_offset / 8)) =
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
static int  _ConHSBs(src, dst)
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
    ceiling = (float)(dst->UdpL_Levels - 1);

    /* Make a pass through the pixels to determine the range of the data */
    src_scan_offset =
	src->UdpL_Pos + (Y1 - src->UdpL_Y1) * src->UdpL_ScnStride;

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

    /* Calculate the scale value and offset */
    foffset = fmin;
    if ((fmax-fmin) != 0.0)
	fscale = ceiling/(fmax-fmin);
    else
	fscale = 1.0;
	

    /* Scale and convert the data */
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
**      Constrain using the Hard Scale model from float to byte array:
**          Scale such that smallest value -> 0
**                          largest value -> ceiling
**
**  FORMAL PARAMETERS:
**
**      src - source data descriptor
**	dst - destination data descriptor
**
*****************************************************************************/
static int  _ConHSByt(src, dst)
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

    /* Make a pass through the pixels to determine the range of the data */
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

    /* Calculate the scale value and offset */
    foffset = fmin;
    if ((fmax-fmin) != 0.0)
	fscale = ceiling/(fmax-fmin);
    else
	fscale = 1.0;

    /* Scale and convert the data */
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

		*((unsigned char *)(dst->UdpA_Base + dst_offset / 8)) =
		    (fvalue - foffset) * fscale + 0.5;     /* Scale */

		src_offset += src->UdpL_PxlStride;
		dst_offset += dst->UdpL_PxlStride;
		}

	    src_scan_offset += src->UdpL_ScnStride;
	    dst_scan_offset += dst->UdpL_ScnStride;
	}
    return( Success );
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

/* N O W   "C P P   V E R S I O N S"   O F   T H E   B A S E   C O N V E R S I O N S	  */

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
static int  _CvtBsBsCpp(src, dst, cpp)
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
	    if (GET_VALUE_(cpp->UdpA_Base, cpp_offset, 1)) {
		value = GET_VALUE_(src->UdpA_Base, src_offset, src_mask);
		PUT_VALUE_(dst->UdpA_Base, dst_offset, value, dst_mask);
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
static int  _CvtBsBytCpp(src, dst, cpp)
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
			*((unsigned char *)(dst->UdpA_Base + dst_offset / 8)) =
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
static int  _CvtBsFltCpp(src, dst, cpp)
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
			(float)GET_VALUE_(src->UdpA_Base,src_offset,src_mask);
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
static int  _CvtBytBytCpp( src, dst, cpp )
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
static int  _CvtBytBsCpp(src, dst, cpp)
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
				   *((unsigned char *)(src->UdpA_Base +
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
static int  _CvtBytFltCpp(src, dst, cpp)
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
			    (float)*((unsigned char *)(src->UdpA_Base +
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
static int  _CvtFltFltCpp( src, dst, cpp )
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
		*((float *)(dst->UdpA_Base + (dst_offset >> 3))) =
		    *((float *)(src->UdpA_Base + (src_offset >> 3)));
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
static int  _CvtFltBsCpp(src, dst, cpp)
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
			value = *((float *)(src->UdpA_Base +
					    src_offset / 8)) + 0.5;
		    PUT_VALUE_(dst->UdpA_Base,dst_offset,value,dst_mask);
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
static int  _CvtFltBytCpp(src, dst, cpp)
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
			*((unsigned char *)(dst->UdpA_Base + dst_offset / 8)) =
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
/* end Module SmiConvert */
