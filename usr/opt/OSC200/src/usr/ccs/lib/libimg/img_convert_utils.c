
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
**  IMG_CONVERT_UTILS
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
**	Mark W. Sornson 
**
**  CREATION DATE:
**
**	1989
**
**  MODIFICATION HISTORY:
**
************************************************************************/

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
**  Table of contents:
**
**	VMS-specific entry points
*/
#if defined(__VMS) || defined(VMS)
unsigned long	IMG$CVT_ALIGNMENT();
unsigned long	IMG$CVT_BYTE_BITS();
unsigned long	IMG$CVT_COMP_SPACE_ORG();
#endif

/*
**	Global portable entry points
*/
#ifdef NODAS_PROTO
unsigned long	ImgCvtAlignment();
unsigned long	ImgCvtByteBits();
unsigned long	ImgCvtCompSpaceOrg();
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
**	External Entry Points		    .......   From Module   .........
*/
#ifdef NODAS_PROTO
long	     ImgAdjustFrameDef();	    /* IMG_DEFINITION_UTILS	    */
long	     ImgAllocateFrame();	    /* IMG_DEFINITION_UTILS	    */
long	     ImgCopyFrame();		    /* IMG_FRAME_UTILS		    */
void	     ImgDeleteFrame();
void	     ImgDeleteFrameDef();	    /* IMG_DEFINITION_UTILS	    */
long	     ImgExtractFrameDef();	    /* IMG_DEFINITION_UTILS	    */
void	     ImgVerifyFrame();

long	     _ImgCvtCompSpaceOrg();	    /* IMG__CONVERT_UTILS	    */
void	     _ImgDeleteCsa();		    /* IMG__ATTRIBUTE_UTILS	    */
void	     _ImgErrorHandler();	    /* IMG_ERROR_HANDLER	    */
struct CSA  *_ImgExtractCsa();		    /* IMG__ATTRIBUTE_UTILS	    */
long	     _ImgGet();			    /* IMG__ATTRIBUTE_ACCESS_UTILS  */
long	     _ImgGetVerifyStatus();
long	     _ImgPut();
long	     _IpsCopyData();
#endif

/*
**  External References:
**
**	Status codes
*/
#include    <img/ImgStatusCodes.h>

/*
**  Local Storage:
**
**	none
*/


/******************************************************************************
**  IMG$CVT_ALIGNMENT
**  ImgCvtAlignment
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
******************************************************************************
** VMS-specific Entry Point
******************************************************************************/
#if defined(__VMS) || defined(VMS)
unsigned long IMG$CVT_ALIGNMENT( srcfid, itemlist, flags )
unsigned long	 srcfid;
struct ITMLST	*itemlist;
unsigned long	 flags;
{
unsigned long	retfid;

retfid = ImgCvtAlignment( srcfid, itemlist, flags );
return retfid;
} /* end of IMG$CVT_ALIGNMENT */
#endif


/*****************************************************************************
** Portable Entry Point
******************************************************************************/
struct FCT *ImgCvtAlignment( srcfid, itemlist, flags )
struct FCT *srcfid;
struct ITMLST	*itemlist;
unsigned long	 flags;
{
long		dp_cnt;
long		index;
long		status;

struct FCT *retfid;

struct UDP	dst_udp;
struct UDP	src_udp;

/*
** Verify the source frame ...
*/
if ( VERIFY_ON_ )
    ImgVerifyFrame( srcfid, ImgM_NonstandardVerify );

if ( itemlist == 0 || itemlist->ItmL_Code == 0 )
/*
**  Copy the source frame if no attributes were specified ...
*/
    {
    if ( (flags & ImgM_InPlace) )
	retfid = srcfid;
    else
	retfid = ImgCopyFrame( srcfid, 0 );
    }
else
/*
**  Create a new return frame with its data aligned.
*/
    {
    /*
    ** Allocate a new frame using the source as a template, and
    ** using the input itemlist to set up the attribute values
    ** of the destination frame.
    */
    retfid = ImgAllocateFrame( 0, itemlist, srcfid, 0 );
    _ImgGet( srcfid, Img_PlanesPerPixel, &dp_cnt, LONGSIZE, 0, 0 );

    /*
    ** Now align the data by copying the source udp(s) into
    ** the destination udp(s) which has (or have) been set
    ** up to reflect the specified alignment.
    */
    for ( index = 0; index < dp_cnt; ++index )
	{
	_ImgGet( srcfid, Img_Udp, &src_udp, sizeof(src_udp), 0, index );
	_ImgGet( retfid, Img_Udp, &dst_udp, sizeof(src_udp), 0, index );

	if ( src_udp.UdpW_PixelLength > dst_udp.UdpW_PixelLength )
	    src_udp.UdpW_PixelLength = dst_udp.UdpW_PixelLength;

	status = _IpsCopyData( &src_udp, &dst_udp );
	if ( (status & 1) != 1 )
	    _ImgErrorHandler( status );
	}

    }

if ( (flags & ImgM_AutoDeallocate) )
    ImgDeallocateFrame( srcfid );

if ( VERIFY_ON_ )
    ImgVerifyFrame( retfid, ImgM_NonstandardVerify );

return retfid;
} /* end of ImgCvtAlignment */


/******************************************************************************
**  IMG$CVT_BYTE_BITS
**  ImgCvtByteBits
**
**  FUNCTIONAL DESCRIPTION:
**
**	This function reverses the bits in each byte of every byte in
**	every data plane of an image.
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
******************************************************************************
** VMS-specific Entry Point
******************************************************************************/
#if defined(__VMS) || defined(VMS)
unsigned long IMG$CVT_BYTE_BITS( srcfid, bit_order, flags )
unsigned long	srcfid;
unsigned long	bit_order;
unsigned long	 flags;
{
unsigned long	retfid;

retfid = ImgCvtByteBits( srcfid, bit_order, flags );

return retfid;
} /* end of IMG$CVT_BYTE_BITS */
#endif


/*****************************************************************************
** Portable Entry Point
******************************************************************************/
struct FCT *ImgCvtByteBits( srcfid, bit_order, flags )
struct FCT *srcfid;
unsigned long	bit_order;
unsigned long	 flags;
{
struct FCT *retfid;

/*
** Verify the source frame ...
*/
if ( VERIFY_ON_ )
    ImgVerifyFrame( srcfid, ImgM_NonstandardVerify );


/*
** Verify the return frame ...
*/
if ( VERIFY_ON_ )
    ImgVerifyFrame( retfid, ImgM_NonstandardVerify );

return retfid;
} /* end of ImgCvtByteBits */


/******************************************************************************
**  ImgCvtCompSpaceOrg
**
**  FUNCTIONAL DESCRIPTION:
**
**	Convert the component space organization of the source frame into
**	the organization specified by the caller.  This routine will
**	convert uncompressed (PCM) images between all of the following
**	organizations:
**
**		Band Interleaved By Pixel
**		Band Interleaved By Plane
**		Bit Interleaved By Plane
**		Band Interleaved By Line
**
**  FORMAL PARAMETERS:
**
**	src_fid		Identifier of the source frame.  Passed by value.
**	ret_org		Comp. Space Org. to convert to.  Passed by value.
**	ret_dp_signif	Data plane significance of the return frame.
**			Valid if ret org is Bit Interleaved By Plane.
**			Passed by value.
**	flags		No flags are defined at present.
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
**	retfid	    Frame identifier of the converted output frame.
**		    Passed by value.
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************
** VMS-specific Entry Point
******************************************************************************/
#if defined(__VMS) || defined(VMS)
unsigned long IMG$CVT_COMP_SPACE_ORG( src_fid, ret_org, ret_dp_signif, flags )
unsigned long	src_fid;
unsigned long	ret_org;
unsigned long	ret_dp_signif;
unsigned long	flags;
{
unsigned long	retfid;

retfid = ImgCvtCompSpaceOrg( src_fid, ret_org, ret_dp_signif, flags );
return retfid;
} /* end of IMG$CVT_COMP_SPACE_ORG */
#endif

/*****************************************************************************
** Portable Entry Point
******************************************************************************/
struct FCT *ImgCvtCompSpaceOrg( src_fid, ret_org, ret_dp_signif, flags )
struct FCT *src_fid;
unsigned long	ret_org;
unsigned long	ret_dp_signif;
unsigned long	flags;
{
unsigned long	compression_type;
unsigned long	local_flags;
unsigned long	local_ret_dp_signif = ImgK_LsbitFirst;
struct FCT *ret_def;
struct FCT *ret_fid;
unsigned long	ret_plane_count;
unsigned long	ret_status;
unsigned long	src_data_class;
struct FCT *src_def;
unsigned long	src_dp_signif;
unsigned long	src_org;
unsigned long	src_plane_count;
unsigned long	udp_idx;

struct ITMLST	 itmlst[3];
struct CSA	*ret_cs_attrs;
struct CSA	*src_cs_attrs;

if ( VERIFY_ON_ )
    ImgVerifyFrame( src_fid, ImgM_NonstandardVerify );

_ImgGet( src_fid, Img_CompressionType, &compression_type, LONGSIZE, 0, 0 );
if ( compression_type != ImgK_PcmCompression )
    ChfStop( 1, ImgX_FRMNOTUNC );

_ImgGet( src_fid, Img_CompSpaceOrg, &src_org, LONGSIZE, 0, 0 );
_ImgGet( src_fid, Img_PlaneSignif, &src_dp_signif, LONGSIZE, 0, 0 );
_ImgGet( src_fid, Img_ImageDataClass, &src_data_class, LONGSIZE, 0, 0 );

if ( ret_dp_signif != 0 )
    local_ret_dp_signif = ret_dp_signif;

if ( ret_org == src_org && local_ret_dp_signif == src_dp_signif )
    /*
    ** Copy the frame if no difference is specified ... 
    */
    ret_fid = ImgCopyFrame( src_fid, 0 );
else if ( src_data_class == ImgK_ClassBitonal ||
	  (src_data_class == ImgK_ClassGreyscale && 
	    src_org != ImgK_BitIntrlvdByPlane && 
		ret_org != ImgK_BitIntrlvdByPlane ) )
    {
    /* ... or if the frame is BITONAL or GREYSCAlE [but not 
    ** bit-intrlved-by-plane] although we should set the return 
    ** cs org attr to the one specified.
    */
    ret_fid = ImgCopyFrame( src_fid, 0 );
    _ImgPut( ret_fid, Img_CompSpaceOrg, &ret_org, LONGSIZE, 0 );
    }
else
    /*
    ** Convert the component space org for real by reorganizing
    ** the data ...
    */
    {
    /*
    ** Extract the source frame definition and adjust it using the
    ** values input to this function.  Use the adjusted frame def to
    ** create the return frame.
    */
    src_def = ImgExtractFrameDef( src_fid, 0 );
    itmlst[0].ItmL_Code	    = Img_CompSpaceOrg;
    itmlst[0].ItmL_Length   = LONGSIZE;
    itmlst[0].ItmA_Buffer   = (char *)&ret_org;
    itmlst[0].ItmL_Index    = 0;

    itmlst[1].ItmL_Code	    = Img_PlaneSignif;
    itmlst[1].ItmL_Length   = LONGSIZE;
    itmlst[1].ItmA_Buffer   = (char *)&local_ret_dp_signif;
    itmlst[1].ItmL_Index    = 0;

    itmlst[2].ItmL_Code	    = 0;
    itmlst[2].ItmL_Length   = 0;
    itmlst[2].ItmA_Buffer   = 0;
    itmlst[2].ItmL_Index    = 0;

    ret_def = ImgAdjustFrameDef( src_def, itmlst, 0 );
    ImgDeleteFrameDef( src_def );
    local_flags = ImgM_AutoDeallocate | ImgM_NoStandardize;
    ret_fid = ImgAllocateFrame( 0, 0, ret_def, local_flags );

    src_cs_attrs = _ImgExtractCsa( src_fid );
    ret_cs_attrs = _ImgExtractCsa( ret_fid );

    ret_status = _ImgCvtCompSpaceOrg( src_cs_attrs, ret_cs_attrs );
    if ( (ret_status & 1 ) != 1 )
	ImgDeleteFrame( ret_fid );

    _ImgDeleteCsa( src_cs_attrs );
    _ImgDeleteCsa( ret_cs_attrs );
    }

if ( (flags & ImgM_AutoDeallocate) )
    ImgDeallocateFrame( src_fid );

if ( VERIFY_ON_ )
    ImgVerifyFrame( ret_fid, ImgM_NonstandardVerify );

return ret_fid;
} /* end of ImgCvtCompSpaceOrg */
