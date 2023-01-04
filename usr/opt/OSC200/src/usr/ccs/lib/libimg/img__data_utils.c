
/***************************************************************************** 
**
**  Copyright (c) Digital Equipment Corporation, 1990 All Rights Reserved.
**  Unpublished rights reserved under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary to and embodies the 
**  confidential technology of Digital Equipment Corporation. Possession, use,
**  duplication or dissemination of the software and media is authorized only 
**  pursuant to a valid written license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the 
**  U.S. Government is subject to restrictions as set forth in 
**  Subparagraph (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**
*****************************************************************************/

/************************************************************************
**  IMG__DATA_UTILS.C
**
**  FACILITY:
**
**	Image Services Library
**
**  ABSTRACT:
**
**	[@tbs@]
**
**  ENVIRONMENT:
**
**	VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
**	Mark Sornson
**
**  CREATION DATE:
**
**	1989
**
**  MODIFICATION HISTORY:
**
************************************************************************/

/*
**  Table of contents:
**
**	Global entry points
*/
#ifdef NODAS_PROTO
long	_ImgInterleaveBits();
long	_ImgInterleaveComponents();
long	_ImgSeparateBits();
long	_ImgSeparateComponents();
#endif



/*
**  Include files:
*/
#include    <img/ChfDef.h>
#include    <img/ImgDef.h>
#include    <ImgDefP.h>
#include    <ImgMacros.h>
#ifndef NODAS_PROTO
#include <imgprot.h>	    /* Img prototypes */
#endif

/*
**  MACRO definitions:
**
**	none
*/

/*
**  Equated Symbols:
**
**	none
*/

/*
**  External References:
**
**	status codes
*/
#include <img/ImgStatusCodes.h>

/*
**	External Entry Points		<---   from module   --->
*/
#ifdef NODAS_PROTO
long		    _ImgConvertLevelsToBits();	/* IMG__ATTRIBUTE_UTILS	*/
unsigned    long    _IpsCopyData();		/* IPS__UDP_UTILS	*/
#endif
/*
**  Local Storage:
**
**	none
*/


/******************************************************************************
**  _ImgInterleaveBits
**
**  FUNCTIONAL DESCRIPTION:
**
**	[@tbs@]
**
**  FORMAL PARAMETERS:
**
**	[@description_or_none@]
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
**	void (none)
**
**  SIGNAL CODES:
**
**	[@description_or_none@]
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
long _ImgInterleaveBits( src_plane_cnt, src_plane_signif, src_udp_lst, dst_udp )
long	     src_plane_cnt;
long	     src_plane_signif;
struct UDP  *src_udp_lst;
struct UDP  *dst_udp;
{
long	     dst_pos_incr;
long	     src_plane_idx;
long	     status	    = ImgX_SUCCESS;
struct UDP   local_dst_udp;

/*
** Make a local copy of the destination udp so that the fill
** algorithm can modify it without changing the actual UDP 
** data structure that was passed in.
*/
local_dst_udp = *dst_udp;

switch ( src_plane_signif )
    {
    case ImgK_MsbitFirst:
	dst_pos_incr = -1;
	local_dst_udp.UdpL_Pos += src_plane_cnt - 1;
	break;

    case ImgK_LsbitFirst:
    default:
	dst_pos_incr = 1;
	break;
    } /* end switch */

/*
** Loop through source data planes, and copy each bit plane 
** into the destination buffer.  NOTE that the local copy of the
** dst udp has its pixel length set to one to keep from overwriting
** bits of higher significance when only 1 bit at a time is being
** written into the dst buffer.
*/
src_plane_idx = 0;
local_dst_udp.UdpW_PixelLength = 1;
for ( ; src_plane_idx < src_plane_cnt; ++src_plane_idx )
    {
    _IpsCopyData( &(src_udp_lst[src_plane_idx]), &local_dst_udp );
    local_dst_udp.UdpL_Pos += dst_pos_incr;
    }

return status;
} /* end of _ImgInterleaveBits */


/******************************************************************************
**  _ImgInterleaveComponents
**
**  FUNCTIONAL DESCRIPTION:
**
**	[@tbs@]
**
**  FORMAL PARAMETERS:
**
**	[@description_or_none@]
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
**	void (none)
**
**  SIGNAL CODES:
**
**	[@description_or_none@]
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
long _ImgInterleaveComponents( src_plane_cnt, src_udp_lst, dst_cs_org,
				dst_comp_offset, dst_udp )
long	     src_plane_cnt;
struct UDP  *src_udp_lst;
long	     dst_cs_org;
long	    *dst_comp_offset;
struct UDP  *dst_udp;
{
long	     bits_per_comp;
long	     src_plane_idx;
long	     status	    = ImgX_SUCCESS;

struct UDP   local_dst_udp;
struct UDP   local_src_udp;

/*
** Make a local copy of the destination udp so that the fill
** algorithm can modify it without changing the actual UDP
** data structure that was passed in.
*/
local_dst_udp = *dst_udp;

/*
** Loop through source data planes, and copy each component plane 
** into the destination buffer.
*/
switch ( dst_cs_org )
    {
    case ImgK_BandIntrlvdByLine:
	src_plane_idx = 0;
	for ( ; src_plane_idx < src_plane_cnt; )
	    {
	    /*
	    ** Make a local copy of the source UDP so that the pixel
	    ** length field can be modified (to reflect the fact that
	    ** only the quant bits per component are being copied without
	    ** any additional pixel padding bits)/
	    */
	    local_src_udp = src_udp_lst[src_plane_idx];
	    bits_per_comp = _ImgConvertLevelsToBits(local_src_udp.UdpL_Levels );
	    local_src_udp.UdpW_PixelLength = bits_per_comp;

	    /*
	    ** Set up Pos field to jump over previous components.
	    */
	    local_dst_udp.UdpL_Pos = dst_comp_offset[ src_plane_idx ];

	    /*
	    ** Set the dst pixel length and stride to be equal to the
	    ** src pixel length, so that the dst data for each band will
	    ** be fully packed.
	    */
	    local_dst_udp.UdpW_PixelLength = bits_per_comp;
	    local_dst_udp.UdpL_PxlStride = local_dst_udp.UdpW_PixelLength;

	    /*
	    ** Copy the data (by component)
	    */
	    _IpsCopyData( &local_src_udp, &local_dst_udp );
	    ++src_plane_idx;
	    }
	break;

    case ImgK_BandIntrlvdByPlane:
    default:
	src_plane_idx = 0;
	for ( ; src_plane_idx < src_plane_cnt; )
	    {
	    /*
	    ** Make a local copy of the source UDP so that the pixel
	    ** length field can be modified (to reflect the fact that
	    ** only the quant bits per component are being copied without
	    ** any additional pixel padding bits)/
	    */
	    local_src_udp = src_udp_lst[src_plane_idx];
	    bits_per_comp = _ImgConvertLevelsToBits( local_src_udp.UdpL_Levels);
	    local_src_udp.UdpW_PixelLength = bits_per_comp;

	    /*
	    ** Set up Pos field to jump over previous components and
	    ** copy the data by component.
	    */
	    local_dst_udp.UdpL_Pos = dst_comp_offset[ src_plane_idx ];
	    local_dst_udp.UdpW_PixelLength = bits_per_comp;
	    _IpsCopyData( &local_src_udp, &local_dst_udp );
	    ++src_plane_idx;
	    }
	break;
    }

return status;
} /* end of _ImgInterleaveComponents */


/******************************************************************************
**  _ImgSeparateBits
**
**  FUNCTIONAL DESCRIPTION:
**
**	[@tbs@]
**
**  FORMAL PARAMETERS:
**
**	[@description_or_none@]
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
**	void (none)
**
**  SIGNAL CODES:
**
**	[@description_or_none@]
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
long _ImgSeparateBits( src_udp, dst_plane_signif, dst_udp_lst )
struct UDP  *src_udp;
long	     dst_plane_signif;
struct UDP  *dst_udp_lst;
{
long	    dst_plane_cnt;
long	    dst_plane_idx	= 0;
long	    src_pos_incr;
long	    status		= ImgX_SUCCESS;
struct UDP  local_src_udp;

/*
** Figure out the number of destination planes that will result based
** on the number of quant levels in the source udp.
*/
dst_plane_cnt = _ImgConvertLevelsToBits( src_udp->UdpL_Levels );

/*
** Make a local copy of the source UDP.  Set the length field to one so
** that the data copies will move just one significant bit from each
** data element at a time.
*/
local_src_udp = *src_udp;
local_src_udp.UdpW_PixelLength = 1;

/*
** Set up variables that influence the order in which significant bits
** are copied.
*/
switch ( dst_plane_signif )
    {
    case ImgK_MsbitFirst:
	src_pos_incr = -1;
	local_src_udp.UdpL_Pos += dst_plane_cnt - 1;
	break;
    case ImgK_LsbitFirst:
	src_pos_incr = 1;
    default:
	break;
    } /* end switch */

/*
** Separate the bits.  NOTE that the src pos field, and how it's incremented
** (or decremented), controls the order of significance.
**
** Destination udps are always filled from the lowest index to the highest
** in the list.  If the plane signif is reversed, the data is taken from
** the source in reverse order.
*/
for ( ; dst_plane_idx < dst_plane_cnt; ++dst_plane_idx )
    {
    _IpsCopyData( &local_src_udp, &(dst_udp_lst[dst_plane_idx]) );
    local_src_udp.UdpL_Pos += src_pos_incr;
    } /* end for */

return status;
} /* end of _ImgSeparateBits */


/******************************************************************************
**  _ImgSeparateComponents
**
**  FUNCTIONAL DESCRIPTION:
**
**	[@tbs@]
**
**  FORMAL PARAMETERS:
**
**	[@description_or_none@]
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
**	void (none)
**
**  SIGNAL CODES:
**
**	[@description_or_none@]
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
long _ImgSeparateComponents( src_comp_cnt, src_comp_offset_lst, src_bpc_lst, 
				src_udp, dst_udp_lst)
long	     src_comp_cnt;
long	    *src_comp_offset_lst;
long	    *src_bpc_lst;		/* src bits per component list */
struct UDP  *src_udp;
struct UDP  *dst_udp_lst;
{
long	    dst_plane_idx	= 0;
long	    original_src_pos;
long	    src_pos_incr;
long	    status		= ImgX_SUCCESS;
struct UDP  local_src_udp;

local_src_udp = *src_udp;
original_src_pos = local_src_udp.UdpL_Pos;

for ( ; dst_plane_idx < src_comp_cnt ; ++dst_plane_idx )
    {
    local_src_udp.UdpW_PixelLength = src_bpc_lst[dst_plane_idx];
    local_src_udp.UdpL_Pos = 
			original_src_pos + src_comp_offset_lst[dst_plane_idx];
    _IpsCopyData( &local_src_udp, &(dst_udp_lst[dst_plane_idx]) );
    } /* end for */

return status;
} /* end of _ImgSeparateComponents */
