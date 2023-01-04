
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
**  IMG__CONVERT_UTILS
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
*/
#ifdef NODAS_PROTO
long	_ImgCvtCompSpaceOrg();

long	_ImgCvtCsOrgGen();
long	_ImgCvtCsOrg1To3();
long	_ImgCvtCsOrg2To3();
long	_ImgCvtCsOrg3To1();
long	_ImgCvtCsOrg3To2();
long	_ImgCvtCsOrg3To3();
long	_ImgCvtCsOrg3To4();
long	_ImgCvtCsOrg4To3();
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
**	Symbol Definitions For Message Codes
*/
#include <img/ImgStatusCodes.h>

/*
**	External Entry Points
**					    ...  from module  ...
*/
#ifdef NODAS_PROTO
void	 _IpsCopyData();		    /* IPS__UDP_UTILS	*/
char	*_ImgCalloc();			    /* IMG__MEMORY_MGT	*/
long	 _ImgInterleaveBits();		    /* IMG__DATA_UTILS	*/
long	 _ImgInterleaveComponents();	    /* IMG__DATA_UTILS	*/
long	 _ImgSeparateBits();		    /* IMG__DATA_UTILS	*/
long	 _ImgSeparateComponents();	    /* IMG__DATA_UTILS	*/
#endif

/*
**  Local Storage:
**
**	none
*/


/******************************************************************************
**  _ImgCvtCompSpaceOrg
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
long _ImgCvtCompSpaceOrg( src_cs_attrs, ret_cs_attrs )
struct CSA  *src_cs_attrs;
struct CSA  *ret_cs_attrs;
{
long	ret_cs_org;
long	ret_status;
long	src_cs_org;
/*
**  Convert component space dispatch table:
**
**	Org types are abbreviated as follows:
**
**	    org1 = band interleaved by pixel
**	    org2 = band interleaved by plane (native format)
**	    org3 = bit interleaved by plane
**	    org4 = band interleaved by line
*/
static long (*convert_cs_table[4][4])() = {
     { _ImgCvtCsOrgGen,  _ImgCvtCsOrgGen,  _ImgCvtCsOrg1To3, _ImgCvtCsOrgGen  }
    ,{ _ImgCvtCsOrgGen,  _ImgCvtCsOrgGen,  _ImgCvtCsOrg2To3, _ImgCvtCsOrgGen  }
    ,{ _ImgCvtCsOrg3To1, _ImgCvtCsOrg3To2, _ImgCvtCsOrg3To3, _ImgCvtCsOrg3To4 }
    ,{ _ImgCvtCsOrgGen,  _ImgCvtCsOrgGen,  _ImgCvtCsOrg4To3, _ImgCvtCsOrgGen  }
    };

/*
** Dispatch to the appropriate conversion function.  NOTE that the
** cs org types have literal values between 1 and 4, which makes one
** less than the type the appropriate index into the dispatch table.
**
**  NOTE:   Although each function is passed 6 arguments, not all
**	    arguments are needed.  The convert routines that are
**	    dispatched to DO NOT do any error checking.
*/
src_cs_org = src_cs_attrs->CsaL_CompSpaceOrg - 1;
ret_cs_org = ret_cs_attrs->CsaL_CompSpaceOrg - 1;
ret_status = (*convert_cs_table[src_cs_org][ret_cs_org])(
						src_cs_attrs, ret_cs_attrs);

return ret_status;
} /* end of _ImgCvtCompSpaceOrg */


/******************************************************************************
**  _ImgCvtCsOrgGen
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
long _ImgCvtCsOrgGen( src_cs_attrs, ret_cs_attrs )
struct CSA	*src_cs_attrs;
struct CSA	*ret_cs_attrs;
{
long	comp_cnt	= src_cs_attrs->CsaL_NumberOfComp;
long	comp_idx;
long	ret_status	= ImgX_SUCCESS;
long	status;

struct UDP  *ret_udp_ptr    = ret_cs_attrs->CsaR_CompUdpList;
struct UDP  *src_udp_ptr    = src_cs_attrs->CsaR_CompUdpList;

for ( comp_idx = 0; comp_idx < comp_cnt; ++comp_idx )
    {
    _IpsCopyData( src_udp_ptr, ret_udp_ptr );
    ++src_udp_ptr;
    ++ret_udp_ptr;
    }

return ret_status;
} /* end of _ImgCvtCsOrgGen */


/******************************************************************************
**  _ImgCvtCsOrg1To3
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
long _ImgCvtCsOrg1To3( src_cs_attrs, ret_cs_attrs )
struct CSA	*src_cs_attrs;
struct CSA	*ret_cs_attrs;
{
long	 comp_cnt	= src_cs_attrs->CsaL_NumberOfComp;
long	 comp_idx;
long	*qbits_per_comp	= src_cs_attrs->CsaL_QuantBitsPerComp;
long	 ret_dp_signif	= ret_cs_attrs->CsaL_PlaneSignif;
long	 ret_status	= ImgX_SUCCESS;
long	 status;

struct UDP  *ret_plane_udps = ret_cs_attrs->CsaR_PlaneUdpList;
struct UDP  *src_comp_udp   = src_cs_attrs->CsaR_CompUdpList;

switch ( ret_dp_signif )
    {
    case ImgK_LsbitFirst:
	for ( comp_idx = 0; comp_idx < comp_cnt; ++comp_idx )
	    {
	    status = _ImgSeparateBits(
			src_comp_udp,
			ret_dp_signif,
			ret_plane_udps );
	    if ( (status&1) != 1 )
		{
		ret_status = status;
		break;
		}

	    ++src_comp_udp;
	    ret_plane_udps += qbits_per_comp[ comp_idx ];
	    }
	break;

    case ImgK_MsbitFirst:
	for ( comp_idx = comp_cnt-1 ; comp_idx >= 0 ; --comp_idx )
	    {
	    status = _ImgSeparateBits(
			&(src_comp_udp[comp_idx]),
			ret_dp_signif,
			ret_plane_udps );
	    if ( (status&1) != 1 )
		{
		ret_status = status;
		break;
		}

	    ret_plane_udps += qbits_per_comp[ comp_idx ];
	    }
    default:
	break;
    }

return ret_status;
} /* end of _ImgCvtCsOrg1To3 */


/******************************************************************************
**  _ImgCvtCsOrg2To3
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
long _ImgCvtCsOrg2To3( src_cs_attrs, ret_cs_attrs )
struct CSA	*src_cs_attrs;
struct CSA	*ret_cs_attrs;
{
long	 comp_cnt;
long	 comp_idx;
long	*quant_bits_per_comp	= src_cs_attrs->CsaL_QuantBitsPerComp;
long	 ret_status		= ImgX_SUCCESS;
long	 ret_udp_idx;
long	 src_udp_incr;
long	 status;

struct UDP  *ret_udp_ptr    = ret_cs_attrs->CsaR_PlaneUdpList;
struct UDP  *src_udp_ptr;

comp_cnt = src_cs_attrs->CsaL_NumberOfComp;
comp_idx = 0;

switch ( ret_cs_attrs->CsaL_PlaneSignif )
    {
    case ImgK_MsbitFirst:
	src_udp_ptr = src_cs_attrs->CsaR_PlaneUdpList + (comp_cnt-1);

	/*
	** Separate the bits a component at a time
	*/
	for ( comp_idx = comp_cnt - 1; comp_idx >= 0 ; --comp_idx )
	    {
	    status = _ImgSeparateBits(	src_udp_ptr,
					ret_cs_attrs->CsaL_PlaneSignif,
					ret_udp_ptr );
	    if ( (status & 1) != 1 )
		return status;
	    --src_udp_ptr;
	    ret_udp_ptr += quant_bits_per_comp[ comp_idx ];
	    }
	break;
    case ImgK_LsbitFirst:
	src_udp_ptr = src_cs_attrs->CsaR_PlaneUdpList;

	/*
	** Separate the bits a component at a time
	*/
	for ( comp_idx = 0; comp_idx < comp_cnt; ++comp_idx )
	    {
	   status =  _ImgSeparateBits(	src_udp_ptr,
					ret_cs_attrs->CsaL_PlaneSignif,
					ret_udp_ptr );
	    if ( (status & 1) != 1 )
		return status;
	    ++src_udp_ptr;
	    ret_udp_ptr += quant_bits_per_comp[ comp_idx ];
	    }
    default:
	break;
    }

return ret_status;
} /* end of _ImgCvtCsOrg2To3 */


/******************************************************************************
**  _ImgCvtCsOrg3To1
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
long _ImgCvtCsOrg3To1( src_cs_attrs, ret_cs_attrs )
struct CSA	*src_cs_attrs;
struct CSA	*ret_cs_attrs;
{
long	 comp_cnt		= src_cs_attrs->CsaL_NumberOfComp;
long	 comp_idx;
long	 ret_status		= ImgX_SUCCESS;
long	*ret_comp_offsets;
long	 src_dp_signif		= src_cs_attrs->CsaL_PlaneSignif;
long	*src_qbits_per_comp	= src_cs_attrs->CsaL_QuantBitsPerComp;
long	*src_planes_per_comp	= src_cs_attrs->CsaL_QuantBitsPerComp;
long	 status;
long	 total_qbits_per_pixel	= src_cs_attrs->CsaL_TotalQuantBitsPerPixel;

struct UDP  local_ret_udp;
struct UDP  *src_udp_ptr	= src_cs_attrs->CsaR_PlaneUdpList;

local_ret_udp = *(ret_cs_attrs->CsaR_PlaneUdpList);

/*
** Interleave the bit planes for each component directly
** into the destination buffer.  Note that a local copy of
** the destination udp is used so that the pos field can be
** used to skip to the correct component position.
*/
switch ( src_dp_signif )
    {
    case ImgK_LsbitFirst:
	for ( comp_idx = 0; comp_idx < comp_cnt; ++comp_idx )
	    {
	    local_ret_udp.UdpW_PixelLength = src_qbits_per_comp[comp_idx];
	    status = _ImgInterleaveBits(
			src_planes_per_comp[comp_idx],
			src_dp_signif,
			src_udp_ptr,
			&local_ret_udp
			);
	    if ( (status & 1) != 1 )
	        ret_status = status;

	    src_udp_ptr += src_planes_per_comp[comp_idx];
	    local_ret_udp.UdpL_Pos += src_qbits_per_comp[comp_idx];
	    }
	break;
    case ImgK_MsbitFirst:
	/*
	** Set up the ret udp pos field so that, with each
	** entrance into the loop, it is backed off by the size of
	** the component data for each component.
	*/
	local_ret_udp.UdpL_Pos += total_qbits_per_pixel;

	for ( comp_idx = comp_cnt-1; comp_idx >= 0; --comp_idx )
	    {
	    local_ret_udp.UdpW_PixelLength = src_qbits_per_comp[comp_idx];
	    local_ret_udp.UdpL_Pos -= src_qbits_per_comp[comp_idx];
	    status = _ImgInterleaveBits(
			src_planes_per_comp[comp_idx],
			src_dp_signif,
			src_udp_ptr,
			&local_ret_udp
			);
	    if ( (status & 1) != 1 )
	        ret_status = status;

	    src_udp_ptr += src_planes_per_comp[comp_idx];
	    }
    default:
	break;
    }

return ret_status;
} /* end of _ImgCvtCsOrg3To1 */


/******************************************************************************
**  _ImgCvtCsOrg3To2
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
long _ImgCvtCsOrg3To2( src_cs_attrs, ret_cs_attrs )
struct CSA	*src_cs_attrs;
struct CSA	*ret_cs_attrs;
{
long	 comp_cnt		= src_cs_attrs->CsaL_NumberOfComp;
long	 comp_idx;
long	 ret_status		= ImgX_SUCCESS;
long	*ret_comp_offsets;
long	 src_dp_signif		= src_cs_attrs->CsaL_PlaneSignif;
long	*src_qbits_per_comp	= src_cs_attrs->CsaL_QuantBitsPerComp;
long	*src_planes_per_comp	= src_cs_attrs->CsaL_QuantBitsPerComp;
long	 status;

struct UDP  *ret_udp_ptr;
struct UDP  *src_udp_ptr	= src_cs_attrs->CsaR_PlaneUdpList;

/*
** Interleave the bit planes for each component directly
** into the destination buffer for each component.  
*/
switch ( src_dp_signif )
    {
    case ImgK_LsbitFirst:
	ret_udp_ptr = ret_cs_attrs->CsaR_PlaneUdpList;
	for ( comp_idx = 0; comp_idx < comp_cnt; ++comp_idx )
	    {
	    status = _ImgInterleaveBits(
			src_planes_per_comp[comp_idx],
			src_dp_signif,
			src_udp_ptr,
			ret_udp_ptr
			);
	    if ( (status & 1) != 1 )
	        ret_status = status;

	    src_udp_ptr += src_planes_per_comp[comp_idx];
	    ++ret_udp_ptr;
	    }
	break;
    case ImgK_MsbitFirst:
	ret_udp_ptr = ret_cs_attrs->CsaR_PlaneUdpList + (comp_cnt - 1);

	for ( comp_idx = comp_cnt-1; comp_idx >= 0; --comp_idx )
	    {
	    status = _ImgInterleaveBits(
			src_planes_per_comp[comp_idx],
			src_dp_signif,
			src_udp_ptr,
			ret_udp_ptr
			);
	    if ( (status & 1) != 1 )
	        ret_status = status;

	    src_udp_ptr += src_planes_per_comp[comp_idx];
	    --ret_udp_ptr;
	    }
    default:
	break;
    }

return ret_status;
} /* end of _ImgCvtCsOrg3To2 */


/******************************************************************************
**  _ImgCvtCsOrg3To3
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
long _ImgCvtCsOrg3To3( src_cs_attrs, ret_cs_attrs )
struct CSA	*src_cs_attrs;
struct CSA	*ret_cs_attrs;
{
long	plane_cnt	= src_cs_attrs->CsaL_PlanesPerPixel;
long	plane_idx;
long	ret_status	= ImgX_SUCCESS;

struct UDP  *ret_udp_ptr;
struct UDP  *src_udp_ptr;

if ( src_cs_attrs->CsaL_PlaneSignif == ret_cs_attrs->CsaL_PlaneSignif )
    /*
    ** Copy data planes from src to dst in the same order.
    */
    {
    src_udp_ptr = src_cs_attrs->CsaR_PlaneUdpList;
    ret_udp_ptr = ret_cs_attrs->CsaR_PlaneUdpList;

    for ( plane_idx = 0; plane_idx < plane_cnt; ++plane_idx )
	{
	_IpsCopyData( src_udp_ptr, ret_udp_ptr );
	++src_udp_ptr;
	++ret_udp_ptr;
	}
    }
else
    /*
    ** Copy data planes from src to dst in reverse order.
    */
    {
    src_udp_ptr = src_cs_attrs->CsaR_PlaneUdpList;
    ret_udp_ptr = ret_cs_attrs->CsaR_PlaneUdpList + (plane_cnt - 1);

    for ( plane_idx = 0; plane_idx < plane_cnt; ++plane_idx )
	{
	_IpsCopyData( src_udp_ptr, ret_udp_ptr );
	++src_udp_ptr;
	--ret_udp_ptr;
	}
    }
return ret_status;
} /* end of _ImgCvtCsOrg3To3 */


/******************************************************************************
**  _ImgCvtCsOrg3To4
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
long _ImgCvtCsOrg3To4( src_cs_attrs, ret_cs_attrs )
struct CSA	*src_cs_attrs;
struct CSA	*ret_cs_attrs;
{
long	 comp_cnt		= src_cs_attrs->CsaL_NumberOfComp;
long	 comp_idx;
long	 pixels_per_line;
long	 prev_comp_stride;
long	 quant_bits_per_prev_comp;
long	 ret_status		= ImgX_SUCCESS;
long	*ret_comp_offsets;
long	*ret_comp_offset_lst;
long	 src_dp_signif		= src_cs_attrs->CsaL_PlaneSignif;
long	*src_qbits_per_comp	= src_cs_attrs->CsaL_QuantBitsPerComp;
long	*src_planes_per_comp	= src_cs_attrs->CsaL_QuantBitsPerComp;
long	 status;

struct UDP  local_ret_udp;
struct UDP  *src_udp_ptr	= src_cs_attrs->CsaR_PlaneUdpList;

local_ret_udp = *(ret_cs_attrs->CsaR_PlaneUdpList);

/*
** Allocate the space for the bit offset list which describes the offset in
** bits from the start of data to the start of each particular component 
** in the destination planes.
*/
ret_comp_offset_lst = (long*) _ImgCalloc( ret_cs_attrs->CsaL_NumberOfComp, 
					    LONGSIZE );

/*
** Now set up the offset list.  (Note that the offset for the first 
** component is zero, which is why the index starts at one.)
*/
pixels_per_line = src_cs_attrs->CsaR_PlaneUdpList[0].UdpL_PxlPerScn;
for ( comp_idx = 1; comp_idx < src_cs_attrs->CsaL_NumberOfComp; ++comp_idx )
    {
    /*
    ** Figure out the offset based on the amount of space (per line)
    ** needed for the previous component.
    */
    quant_bits_per_prev_comp = src_cs_attrs->CsaL_QuantBitsPerComp[comp_idx-1];
    prev_comp_stride = pixels_per_line * quant_bits_per_prev_comp;
    ret_comp_offset_lst[comp_idx] = ret_comp_offset_lst[comp_idx-1] +
					prev_comp_stride;
    }

/*
** Interleave the bit planes for each component directly
** into the destination buffer.  Note that a local copy of
** the destination udp is used so that the pos field can be
** used to skip to the correct component position.
*/
switch ( src_dp_signif )
    {
    case ImgK_LsbitFirst:
	for ( comp_idx = 0; comp_idx < comp_cnt; ++comp_idx )
	    {
	    local_ret_udp.UdpL_Pos = ret_comp_offset_lst[comp_idx];
	    local_ret_udp.UdpW_PixelLength = src_qbits_per_comp[comp_idx];
	    local_ret_udp.UdpL_PxlStride = src_qbits_per_comp[comp_idx];
	    status = _ImgInterleaveBits(
			src_planes_per_comp[comp_idx],
			src_dp_signif,
			src_udp_ptr,
			&local_ret_udp
			);
	    if ( (status & 1) != 1 )
	        ret_status = status;

	    src_udp_ptr += src_planes_per_comp[comp_idx];
	    }
	break;
    case ImgK_MsbitFirst:
	for ( comp_idx = comp_cnt-1; comp_idx >= 0; --comp_idx )
	    {
	    local_ret_udp.UdpL_Pos = ret_comp_offset_lst[comp_idx];
	    local_ret_udp.UdpW_PixelLength = src_qbits_per_comp[comp_idx];
	    local_ret_udp.UdpL_PxlStride = src_qbits_per_comp[comp_idx];

	    status = _ImgInterleaveBits(
			src_planes_per_comp[comp_idx],
			src_dp_signif,
			src_udp_ptr,
			&local_ret_udp
			);
	    if ( (status & 1) != 1 )
	        ret_status = status;

	    src_udp_ptr += src_planes_per_comp[comp_idx];
	    }
    default:
	break;
    }

return ret_status;
} /* end of _ImgCvtCsOrg3To4 */


/******************************************************************************
**  _ImgCvtCsOrg4To3
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
long _ImgCvtCsOrg4To3( src_cs_attrs, ret_cs_attrs )
struct CSA	*src_cs_attrs;
struct CSA	*ret_cs_attrs;
{
long	 comp_cnt	= src_cs_attrs->CsaL_NumberOfComp;
long	 comp_idx;
long	*qbits_per_comp	= src_cs_attrs->CsaL_QuantBitsPerComp;
long	 ret_dp_signif	= ret_cs_attrs->CsaL_PlaneSignif;
long	 ret_status	= ImgX_SUCCESS;
long	 status;

struct UDP  *ret_plane_udps = ret_cs_attrs->CsaR_PlaneUdpList;
struct UDP  *src_comp_udp   = src_cs_attrs->CsaR_CompUdpList;

switch ( ret_dp_signif )
    {
    case ImgK_LsbitFirst:
	for ( comp_idx = 0; comp_idx < comp_cnt; ++comp_idx )
	    {
	    status = _ImgSeparateBits(
			src_comp_udp,
			ret_dp_signif,
			ret_plane_udps );
	    if ( (status&1) != 1 )
		{
		ret_status = status;
		break;
		}

	    ++src_comp_udp;
	    ret_plane_udps += qbits_per_comp[ comp_idx ];
	    }
	break;

    case ImgK_MsbitFirst:
	for ( comp_idx = comp_cnt-1 ; comp_idx >= 0 ; --comp_idx )
	    {
	    status = _ImgSeparateBits(
			&(src_comp_udp[comp_idx]),
			ret_dp_signif,
			ret_plane_udps );
	    if ( (status&1) != 1 )
		{
		ret_status = status;
		break;
		}

	    ret_plane_udps += qbits_per_comp[ comp_idx ];
	    }
    default:
	break;
    }

return ret_status;
} /* end of _ImgCvtCsOrg4To3 */
