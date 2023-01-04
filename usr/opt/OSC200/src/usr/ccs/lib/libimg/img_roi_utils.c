
/************************************************************************
**
**  Copyright (c) Digital Equipment Corporation, 1990
**  All Rights Reserved.  Unpublished rights reserved
**  under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary
**  to and embodies the confidential technology of 
**  Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and
**  media is authorized only pursuant to a valid written
**  license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
**  disclosure by the U.S. Government is subject to
**  restrictions as set forth in Subparagraph (c)(1)(ii)
**  of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**
************************************************************************/

/************************************************************************
**  IMG_ROI_UTILS
**
**  FACILITY:
**
**	Image Services Library
**
**  ABSTRACT:
**
**	The routines in this module provide support for region of
**	interest creation and managment.
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
**	21-MAY-1990
**
**  MODIFICATION HISTORY:
**
**	V1-001	MWS001	Mark W. Sornson	    21-MAY-1990
**		Initial module creation.
**
************************************************************************/

/*
**  Include files:
*/
#include    <stdlib.h>
#include    <string.h>

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
**	VMS-bindings for global entry points
*/
#ifdef NODAS_PROTO
#if defined(__VMS) || defined(VMS)
struct ROI	*IMG$CREATE_ROI_DEF();
void		 IMG$DELETE_ROI_DEF();
unsigned long	 IMG$EXTRACT_ROI();
unsigned long	 IMG$SET_RECT_ROI();
unsigned long	 IMG$UNSET_RECT_ROI();
void		 IMG$VERIFY_ROI();
#endif
#endif

/*
**	Portable bindings for global entry points
*/
#ifdef NODAS_PROTO
struct ROI	*ImgCreateRoiDef();
void		 ImgDeleteRoiDef();
unsigned long	 ImgExtractRoi();
unsigned long	 ImgSetRectRoi();
unsigned long	 ImgUnsetRectRoi();
void		 ImgVerifyRoi();
#endif

/*
**	Module local routines
*/
#ifdef NODAS_PROTO
static struct ROI	*Allocate_roi();
static struct ROI	*Create_aligned_roi();
static struct ROI	*Create_rect_roi_def();
static struct FCT *Extract_rect_roi();
static void		 Verify_fid_roi();
static void		 Verify_rect_roi(); 
#else
PROTO(static struct ROI *Allocate_roi, (unsigned long /*roi_type*/, unsigned long /*flags*/));
PROTO(static struct ROI *Create_aligned_roi, (struct FCT */*src_fid*/, struct ROI */*src_roi*/));
PROTO(static struct ROI *Create_rect_roi_def, (struct ITMLST */*roi_info*/, unsigned long /*flags*/));
PROTO(static struct FCT *Extract_rect_roi, (struct FCT */*fid*/, struct ROI */*roi*/, unsigned long /*flags*/));
PROTO(static void Verify_fid_roi, (struct FCT */*fid*/, struct ROI */*roi*/));
PROTO(static void Verify_rect_roi, (struct ROI */*roi*/, unsigned long /*flags*/));
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
**					<--- from module --->
*/
#ifdef NODAS_PROTO
void		 ChfSignal();
void		 ChfStop();

#if defined(__VMS) || defined(VMS)
unsigned long	 ImgAllocateFrame();	/* IMG_FRAME_UTILS		*/
unsigned long	 ImgCreateRoi();	/* IMG_ROI_UTILS		*/
void		 ImgDeleteRoi();	/* IMG_ROI_UTILS		*/
unsigned long	 ImgGetFrameSize();	/* IMG_ATTRIBUTE_ACCESS_UTILS	*/
unsigned long	 ImgSetFrameSize();	/* IMG_ATTRIBUTE_ACCESS_UTILS	*/
void		 ImgVerifyFrame();	/* IMG_FRAME_UTILS		*/
#endif

char		*_ImgBlkAlloc();	/* IMG__BLOCK_UTILS		*/
void		 _ImgBlkDealloc();	/* IMG__BLOCK_UTILS		*/
char		*_ImgCalloc();		/* IMG__MEMORY_MGT		*/
struct ITMLST	*_ImgCreateItmlst();	/* IMG__ITEMLIST_UTILS		*/
void		 _ImgErase();		/* IMG__ATTRIBUTE_ACCESS_UTILS	*/
void		 _ImgFree();		/* IMG__MEMORY_MGT		*/
void		 _ImgGet();		/* IMG__ATTRIBUTE_ACCESS_UTILS	*/
unsigned long	 _ImgGetVerifyStatus();	/* IMG__VERIFY_UTILS		*/
void		 _ImgPut();		/* IMG__ATTRIBUTE_ACCESS_UTILS	*/
struct UDP	*_ImgSetRoi();		/* IMG__ROI			*/

void		 _IpsCopyData();
#endif

/*
**  Local Storage:
**
**	none
*/


/******************************************************************************
**  IMG$CREATE_ROI_DEF
**  ImgCreateRoiDef
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create a region of interest definition object.
**
**  FORMAL PARAMETERS:
**
**	roi_info    Region of interest definition information.
**		    ITMLST item list.  Passed by reference.
**
**	flags	    Processing flags.
**		    Unsigned longword.  Passed by value.
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
**	roi_id	    Region of interest identifier.
**		    Returned to the caller as an unsigned longword.
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
struct ROI *IMG$CREATE_ROI_DEF( roi_info, flags )
struct ITMLST	*roi_info;
unsigned long	 flags;
{

return (struct ROI *) (ImgCreateRoiDef( roi_info, flags ));
} /* end of IMG$CREATE_ROI_DEF */
#endif


/*****************************************************************************
** Portable Entry Point
******************************************************************************/
struct ROI *ImgCreateRoiDef( roi_info, flags )
struct ITMLST	*roi_info;
unsigned long	 flags;
{
struct ITMLST	*local_roi_info	= roi_info;
struct ROI	*roi_id;

if ( roi_info == 0 || roi_info->ItmL_Code == 0 )
    /*
    ** Invalid zero passed as ROI itmlst address or
    ** as the first itemcode in the ROI info list.
    */
    ChfStop( 1, ImgX_INVZERITM );

while ( local_roi_info->ItmL_Code != 0 )
    {
    switch( local_roi_info->ItmL_Code )
	{
	case Img_RoiRectangle:
	    roi_id = Create_rect_roi_def( local_roi_info, flags );
	    break;
	default:
	    /*
	    ** Signal that the itemcode is invalid.
	    */
	    ChfStop( 3, ImgX_INVROIITM, 1, local_roi_info->ItmL_Code );
	}
    ++local_roi_info;
    }

ImgVerifyRoi( roi_id, 0, flags );

return roi_id;
} /* end of ImgCreateRoiDef */


/******************************************************************************
**  IMG$DELETE_ROI_DEF
**  ImgDeleteRoiDef
**
**  FUNCTIONAL DESCRIPTION:
**
**	Delete a Region of Interest object (and the dynamic memory 
**	associated with it).
**
**  FORMAL PARAMETERS:
**
**	roi	    Region of interest identifier.
**		    Unsigned longword.  Passed by value.
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
void IMG$DELETE_ROI_DEF( roi )
struct ROI  *roi;
{

ImgDeleteRoi( roi );
return;
} /* end of IMG$DELETE_ROI_DEF */
#endif


/*****************************************************************************
** Portable Entry Point
******************************************************************************/
void ImgDeleteRoiDef( roi )
struct ROI  *roi;
{

/*
** Verify that this is a valid ROI object
*/
ImgVerifyRoi( roi, 0, 0 );

/*
** Deallocate the shape descriptor if there is one
*/
if ( roi->RoiA_Shape != 0 )
    _ImgFree( roi->RoiA_Shape );

/*
** Deallocate the ROI object
*/
_ImgBlkDealloc( (struct BHD *)roi );

return;
} /* end of ImgDeleteRoiDef */


/******************************************************************************
**  IMG$EXTRACT_ROI
**  ImgExtractRoi
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
unsigned long IMG$EXTRACT_ROI( fid, roi, flags )
struct FCT *fid;
struct ROI	*roi;
unsigned long	 flags;
{

return (ImgExtractRoi( fid, roi, flags ));
} /* end of IMG$EXTRACT_ROI */
#endif


/*****************************************************************************
** Portable Entry Point
******************************************************************************/
struct FCT *ImgExtractRoi( fid, roi, flags )
struct FCT *fid;
struct ROI	*roi;
unsigned long	 flags;
{
struct FCT *extracted_fid	= 0;

if ( VERIFY_ON_ )
    ImgVerifyFrame( fid, 0 );

ImgVerifyRoi( roi, fid, flags );

switch ( roi->RoiB_Type)
    {
    case ImgK_RoitypeRect:
	extracted_fid = Extract_rect_roi( fid, roi, flags );
	break;
    default:
	break;
    }

if ( VERIFY_ON_ )
    ImgVerifyFrame( extracted_fid, 0 );

return extracted_fid;
} /* end of ImgExtractRoi */


/******************************************************************************
**  IMG$SET_RECT_ROI
**  ImgSetRectRoi
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
unsigned long IMG$SET_RECT_ROI( srcfid, roi, flags )
struct FCT *srcfid;
struct ROI	*roi;
unsigned long	 flags;
{

return ( ImgSetRectRoi( srcfid, roi, flags ));
} /* end of IMG$SET_RECT_ROI */
#endif


/*****************************************************************************
** Portable Entry Point
******************************************************************************/
struct FCT *ImgSetRectRoi( srcfid, roi, flags )
struct FCT *srcfid;
struct ROI	*roi;
unsigned long	 flags;
{
long		 index;
long		 plane_cnt;
struct FCT *retfid		    = srcfid;
struct UDP	*saved_udps;
struct ROI	*tmp_roi;
struct UDP	 tmp_udp;
struct UDP	*udp_ptr;

/*
** Verify the source frame.
*/
if ( VERIFY_ON_ )
    ImgVerifyFrame( srcfid, ImgM_NonstandardVerify );

if ( roi != 0 )
    {
    /* 
    ** Unset any previously set rect. roi, and then 
    ** verify the new roi against the restored (unset) frame.
    */
    retfid = ImgUnsetRectRoi( srcfid );
    ImgVerifyRoi( roi, retfid, flags );
    
    /*
    ** Set the frame attributes to make it look as though the frame
    ** only contains the rectangular region of interest.
    */
    _ImgGet( retfid, Img_PlanesPerPixel, &plane_cnt, LONGSIZE, 0, 0 );
    saved_udps = (struct UDP *)_ImgCalloc( plane_cnt, sizeof(struct UDP) );
    udp_ptr = saved_udps;
    for ( index = 0; index < plane_cnt; ++index )
	{
	_ImgGet( retfid, Img_Udp, udp_ptr, sizeof(struct UDP), 0, index );
	tmp_udp = *udp_ptr;
	tmp_roi = Create_aligned_roi( srcfid, roi );
	_ImgSetRoi( &tmp_udp, tmp_roi );
	if ( tmp_roi != roi )
	    ImgDeleteRoiDef( tmp_roi );
	_ImgPut( retfid, Img_Udp, &tmp_udp, sizeof(struct UDP), index );
	++udp_ptr;
	}
    _ImgPut( retfid, Img_RectRoiInfo, &saved_udps, LONGSIZE, 0 );

    /*
    ** Now that the rect roi has been set, verify that the frame is
    ** still valid.
    */
    if ( VERIFY_ON_ )
	ImgVerifyFrame( retfid, 0 );
    }

return retfid;
} /* end of ImgSetRectRoi */


/******************************************************************************
**  IMG$UNSET_RECT_ROI
**  ImgUnsetRectRoi
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
unsigned long IMG$UNSET_RECT_ROI( srcfid )
struct FCT *srcfid;
{

return (ImgUnsetRectRoi( srcfid ));
} /* end of IMG$UNSET_RECT_ROI */
#endif


/*****************************************************************************
** Portable Entry Point
******************************************************************************/
struct FCT *ImgUnsetRectRoi( srcfid )
struct FCT *srcfid;
{
long	     index;
long	     plane_cnt;
struct UDP  *saved_udps;

if ( !VERIFY_OFF_ )
    ImgVerifyFrame( srcfid, ImgM_NonstandardVerify );

_ImgGet( srcfid, Img_RectRoiInfo, &saved_udps, LONGSIZE, 0, 0 );

/*
** If a rect roi was set, unset it.
*/
if ( saved_udps != 0 )
    {
    _ImgGet( srcfid, Img_PlanesPerPixel, &plane_cnt, LONGSIZE, 0, 0 );
    for ( index = 0; index < plane_cnt; ++index )
	{
	_ImgPut( srcfid, Img_Udp, saved_udps, sizeof(struct UDP), index );
	++saved_udps;
	}
    _ImgErase( srcfid, Img_RectRoiInfo, 0 );
    }

if ( !VERIFY_OFF_ )
    ImgVerifyFrame( srcfid, ImgM_NonstandardVerify );

return srcfid;
} /* end of ImgUnsetRectRoi */


/******************************************************************************
**  IMG$VERIFY_ROI
**  ImgVerifyRoi
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
void IMG$VERIFY_ROI( roi, fid, flags )
struct ROI	*roi;
struct FCT *fid;
unsigned long	 flags;
{

ImgVerifyRoi( roi, fid, flags );

return;
} /* end of IMG$VERIFY_ROI */
#endif

/*****************************************************************************
** Portable Entry Point
******************************************************************************/
void ImgVerifyRoi( roi, fid, flags )
struct ROI	*roi;
struct FCT *fid;
unsigned long	 flags;
{
long	comp_type;
long	frame_uncompressed  = TRUE;
long	index;
long	planes_per_pixel;

/*
** Verify that the ROI object is really a roi object
**
**	Was an identifier passed in?
*/
if ( roi == 0 )
    ChfStop( 1, ImgX_ZEROROIID );

/*
** Is the block type correct?
*/
if ((roi->RoiR_Blkhd.BhdB_Type != ImgK_BlktypRoi) ||
    (roi->RoiR_Blkhd.BhdW_Length != sizeof(struct ROI)))
    ChfStop( 1, ImgX_INVROIOBJ );

/*
** Is the source frame uncompressed?
*/
if ( fid != 0 )
    {
    _ImgGet( fid, Img_PlanesPerPixel, &planes_per_pixel, LONGSIZE, 0, 0 );
    for ( index = 0; index < planes_per_pixel; ++index )
	{
	_ImgGet( fid, Img_CompressionType, &comp_type, LONGSIZE, 0, index );
	if ( comp_type != ImgK_PcmCompression )
	    {
	    frame_uncompressed = FALSE;
	    ChfSignal( 3, ImgX_DPNOTUNC, 1, index );
	    }
	}
    if ( !frame_uncompressed )
	ChfStop( 1, ImgX_FRMNOTUNC );
    }

/*
**	Is the ROI internally consistent (and otherwise valid)?
*/
switch ( roi->RoiB_Type )
    {
    case ImgK_RoitypeRect:
	Verify_rect_roi( roi, flags );
	break;
    case ImgK_RoitypeCCode:
	ChfStop( 1, ImgX_UNSROITYP );
	break;
    default:
	ChfStop( 1, ImgX_INVROITYP );
	break;
    }

/*
** If a frame-id was passed in, verify that the ROI is actually
** a valid sub-region of the frame.
*/
if ( fid != 0 )
    {
    /*
    ** Verify that the frame is good.  
    **
    **	NOTE: it used to be true that the frame had to be in standard
    **	      format, but this has been changed to accomodate ImgSetRectRoi
    **	      which may produce a standardized frame after the roi is applied.
    */
    if ( !VERIFY_OFF_ )
	ImgVerifyFrame( fid, ImgM_NonstandardVerify );
    Verify_fid_roi( fid, roi );
    }

return;
} /* end of ImgVerifyRoi */


/******************************************************************************
**  Allocate_roi
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
static struct ROI *Allocate_roi( roi_type, flags )
unsigned long	roi_type;
unsigned long	flags;
{
struct ROI  *roi;

roi = (struct ROI *)_ImgBlkAlloc( sizeof(struct ROI), ImgK_BlktypRoi );
roi->RoiB_Type = (char)roi_type;

return roi;
} /* end of Allocate_roi */


/******************************************************************************
**  Create_aligned_roi
**
**  FUNCTIONAL DESCRIPTION:
**
**	This function takes a frame and a roi as input, and if the frame
**	is bitonal, it creates a new roi that is guaranteed to have its
**	scanlines begin on byte boundaries.  This means that the left edge
**	of a given roi might be moved left to guarantee byte alignment.
**
**	Continuous tone images aren't affected, since c.t. pixels are
**	always byte aligned, which means c.t. scanlines are always byte
**	aligned.  Therefore, the source roi is simply returned.
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
static struct ROI *Create_aligned_roi( src_fid, src_roi )
struct FCT *src_fid;
struct ROI	*src_roi;
{
long		 data_class;
long		 new_ulx;
long		 new_xpixels;
long		 x_pixel_diff;

struct BRECT	*brect		= (struct BRECT *) 
                                  &(src_roi->RoiR_BoundingRect);
struct ROI	*dst_roi	= src_roi;
struct ITMLST	 roi_info[2];
struct ROI_RECT	 roi_rect;

_ImgGet( src_fid, Img_ImageDataClass, &data_class, LONGSIZE, 0, 0 );

if ( data_class == ImgK_ClassBitonal )
    {
    /*
    ** If the left-most pixel coordinate isn't zero, and it isn't
    ** a multiple of 8 ...
    */
    if ( brect->BrectL_Ulx != 0 && brect->BrectL_Ulx % 8 != 0 )
	/*
	** Align the upper left pixel boundary, and adjust
	** the X-pixel count.
	*/
	{
	new_ulx = ( ((brect->BrectL_Ulx + 7)/8) - 1) * 8;
	x_pixel_diff = brect->BrectL_Ulx - new_ulx;
	new_xpixels = brect->BrectL_XPixels + x_pixel_diff;

	/*
	** Set up and create a new roi def for the aligned roi.
	*/
	roi_rect.RoiL_RectUlx	    = new_ulx;
	roi_rect.RoiL_RectUly	    = brect->BrectL_Uly;
	roi_rect.RoiL_RectPxls	    = new_xpixels;
	roi_rect.RoiL_RectScnlns    = brect->BrectL_YPixels;

	roi_info[0].ItmL_Code	= Img_RoiRectangle;
	roi_info[0].ItmL_Length	= sizeof( roi_rect );
	roi_info[0].ItmA_Buffer	= (char *) &roi_rect;
	roi_info[0].ItmA_Retlen	= 0;
	roi_info[0].ItmL_Index	= 0;
	roi_info[1].ItmL_Code	= 0;	/* terminator	*/

	dst_roi = ImgCreateRoiDef( roi_info, 0 );
	}
    }

return dst_roi;
} /* end of Create_aligned_roi */


/******************************************************************************
**  Create_rect_roi_def
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
static struct ROI  *Create_rect_roi_def( roi_info, flags )
struct ITMLST	*roi_info;
unsigned long	 flags;
{
struct ROI	*roi_id;
struct ROI_RECT	*roi_rect;
/*
** Allocate the generic ROI object block
*/
roi_id = Allocate_roi( ImgK_RoitypeRect, flags );

/*
** Make sure ROI info has been provided, and is the right size;
*/
if ( roi_info->ItmA_Buffer == 0 )
    /*
    ** No ROI_RECT structure was specified.
    */
    ChfStop( 1, ImgX_NORECTDSC );

if ( roi_info->ItmL_Length != sizeof(struct ROI_RECT) )
    /*
    ** The supplied data is NOT  a ROI_RECT structure
    ** (because it's the wrong size).
    */
    ChfStop( 4, ImgX_ROIRECTSZ, 
		2, roi_info->ItmL_Length, sizeof(struct ROI_RECT) );
/*
** Store the ROI_RECT information in the ROI object
*/
roi_rect = (struct ROI_RECT *) (roi_info->ItmA_Buffer);

/*
** Use the rectangular ROI description to define the bounding rectangle
** of the region (since they are the same).
*/
roi_id->RoiR_BoundingRect.BrectL_Ulx	    = roi_rect->RoiL_RectUlx;
roi_id->RoiR_BoundingRect.BrectL_Uly	    = roi_rect->RoiL_RectUly;
roi_id->RoiR_BoundingRect.BrectL_XPixels    = roi_rect->RoiL_RectPxls;
roi_id->RoiR_BoundingRect.BrectL_YPixels    = roi_rect->RoiL_RectScnlns;

/*
** Use the upper left X,Y values to define the origin (or start) of
** the ROI shape.
*/
roi_id->RoiL_StartX = roi_rect->RoiL_RectUlx;
roi_id->RoiL_StartY = roi_rect->RoiL_RectUly;

return roi_id;
} /* end of Create_rect_roi_def */


/******************************************************************************
**  Extract_rect_roi
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
static struct FCT *Extract_rect_roi( fid, roi, flags )
struct FCT *fid;
struct ROI	*roi;
unsigned long	 flags;
{
float		 change_ratio_x	    = 1.0;
float		 change_ratio_y	    = 1.0;
int frame_size_x;
int frame_size_y;
long		 index;
long		 itmlst_element_cnt;
long		 new_scanline_stride;
long		 pixel_stride;
long		 plane_count;
long		 roi_pixels_per_line;
long		 roi_number_of_lines;

struct FCT *extracted_fid;
unsigned long	 fdf;
struct BRECT	 brect;
struct ITMLST	*itmlst;
struct ITMLST	*itmlst_tmp;
struct UDP	 dstudp;
struct UDP	 srcudp;

brect = roi->RoiR_BoundingRect;
/*
** Create the empty desination frame (for the extracted image) ...
**
**  o First, set up an itemlist that describes the output frame's
**    pixel count and scanline count.
*/
roi_pixels_per_line = brect.BrectL_XPixels;
roi_number_of_lines = brect.BrectL_YPixels;
_ImgGet( fid, Img_PlanesPerPixel, &plane_count, LONGSIZE, 0, 0 );

    /*
    **	Allocate the ITMLST:
    **
    **	    There are two elements per plane, for
    **	    pixel count and scanline count, plus
    **	    one additional element as the zero-filled
    **	    terminator.
    */
itmlst_element_cnt = (3 * plane_count) + 1;
itmlst = _ImgCreateItmlst( itmlst_element_cnt );

    /*
    **	Fill in the item list
    */
itmlst_tmp = itmlst;
for ( index = 0; index < plane_count; ++index )
    {
    itmlst_tmp->ItmL_Code   = Img_PixelsPerLine;
    itmlst_tmp->ItmL_Length = LONGSIZE;
    itmlst_tmp->ItmA_Buffer = (char *) &roi_pixels_per_line;
    itmlst_tmp->ItmA_Retlen = 0;
    itmlst_tmp->ItmL_Index  = index;
    ++itmlst_tmp;

    _ImgGet( fid, Img_PixelStride, &pixel_stride, LONGSIZE, 0, index );
    new_scanline_stride = (((roi_pixels_per_line * pixel_stride) + 7)/8)*8;
    itmlst_tmp->ItmL_Code   = Img_ScanlineStride;
    itmlst_tmp->ItmL_Length = LONGSIZE;
    itmlst_tmp->ItmA_Buffer = (char *) &new_scanline_stride;
    itmlst_tmp->ItmA_Retlen = 0;
    itmlst_tmp->ItmL_Index  = index;
    ++itmlst_tmp;

    itmlst_tmp->ItmL_Code   = Img_NumberOfLines;
    itmlst_tmp->ItmL_Length = LONGSIZE;
    itmlst_tmp->ItmA_Buffer = (char *) &roi_number_of_lines;
    itmlst_tmp->ItmA_Retlen = 0;
    itmlst_tmp->ItmL_Index  = index;
    ++itmlst_tmp;
    }

/*
**  o Second, allocate the new frame using the source frame as a
**    template, and adjusting it's definition with the itemlist.
*/
extracted_fid = ImgAllocateFrame( 0, itmlst, fid, 0 );

/*
** o Third, extract the ROI a plane at a time the source and copy
**   it into the destination frame.
*/
for ( index = 0; index < plane_count; ++index )
    {
    /*
    ** Get the UDP from each plane in the source, and fake-out the
    ** udp to point just to the rectangular region of interest.
    */
    _ImgGet( fid, Img_Udp, &srcudp, sizeof(dstudp), 0, index );
    _ImgSetRoi( &srcudp, roi );
    /*
    ** Get the UDP from each plane in the destination, and copy
    ** the source into it.
    */
    _ImgGet( extracted_fid, Img_Udp, &dstudp, sizeof(dstudp), 0, index );
    _IpsCopyData( &srcudp, &dstudp );
    }

/*
** Reset frame bounding box params to reflect the new dimensions
** of the extracted ROI frame.
*/
if ( dstudp.UdpL_PxlPerScn < srcudp.UdpL_PxlPerScn )
    change_ratio_x = dstudp.UdpL_PxlPerScn / srcudp.UdpL_PxlPerScn;

if ( dstudp.UdpL_ScnCnt < srcudp.UdpL_ScnCnt )
    change_ratio_y = dstudp.UdpL_ScnCnt / srcudp.UdpL_ScnCnt;
/*
** get the frame size (Bmus are int's, but cast as a (float *) to satisfy
** ImgGetFrameSize())
*/
ImgGetFrameSize( fid, (float *)&frame_size_x, (float *)&frame_size_y, ImgK_Bmus );
frame_size_x = (float)frame_size_x * change_ratio_x;
frame_size_y = (float)frame_size_y * change_ratio_y;
ImgSetFrameSize( extracted_fid, (float *)&frame_size_x, (float *)&frame_size_y, ImgK_Bmus );

return extracted_fid;
} /* end of Extract_rect_roi */


/******************************************************************************
**  Verify_fid_roi
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
static void Verify_fid_roi( fid, roi )
struct FCT *fid;
struct ROI	*roi;
{
long		 max_x_pixels;
long		 max_y_pixels;
struct BRECT	 brect;
struct UDP	 udp;

brect = roi->RoiR_BoundingRect;
memset( &udp, 0, sizeof( udp ) );

/*
** Get the udp for the first data plane.
**
**	This assumes that all data planes are the
**	same size; otherwise ImgVerifyFrame (called
*	previously) wouldn't have worked.
*/
_ImgGet( fid, Img_Udp, &udp, sizeof(udp), 0, 0 );

/*
** Calculate the virtual size of the region of interest as
** though it started at the image origin, and extended to
** the farthest corner (as given in the definition).
**
**	NOTE that the Ulx and Uly values are zero-based
**	indices; so that adding the pixel counts to them
**	result in absolute pixel counts.
*/
max_x_pixels = brect.BrectL_Ulx + brect.BrectL_XPixels;
max_y_pixels = brect.BrectL_Uly + brect.BrectL_YPixels;

/*
** Compare the source image dimensions with the dimensions of the
** virtual maximum region of interest (as figured out above)
*/
if ( udp.UdpL_PxlPerScn < max_x_pixels )
    /*
    ** Invalid Horizontal Roi Dimension
    */
    ChfStop( 1, ImgX_INVHROIDM );

if ( udp.UdpL_ScnCnt < max_y_pixels )
    /*
    ** Invalid Vertical Roi Dimension
    */
    ChfStop( 1, ImgX_INVVROIDM );

return;
} /* end of Verify_fid_roi */


/******************************************************************************
**  Verify_rect_roi
**
**  FUNCTIONAL DESCRIPTION:
**
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
static void Verify_rect_roi( roi, flags )
struct ROI	*roi;
unsigned long	 flags;
{
long		 invroi	= FALSE;
struct BRECT	 brect;

brect = roi->RoiR_BoundingRect;

/*
**  The starting x coordinate of the ROI and the upper-left x coordinate
**  of the bounding rectangle must be equal.
*/
if ( roi->RoiR_BoundingRect.BrectL_Ulx != roi->RoiL_StartX )
    {
    invroi = TRUE;
    ChfSignal( 4, ImgX_BRXNESTRX, 2, roi->RoiR_BoundingRect.BrectL_Ulx,
		roi->RoiL_StartX );
    }

/*
**  The starting y coordinate of the ROI and the upper-left y coordinate
**  of the bounding rectangle must be equal.
*/
if ( roi->RoiR_BoundingRect.BrectL_Uly != roi->RoiL_StartY )
    {
    invroi = TRUE;
    ChfSignal( 4, ImgX_BRYNESTRY, 2, roi->RoiR_BoundingRect.BrectL_Ulx,
		roi->RoiL_StartX );
    }

/*
** Verify that the bounding rectangle values make sense.
*/
if ( brect.BrectL_XPixels == 0 )
    {
    invroi = TRUE;
    ChfSignal( 1, ImgX_INVBRXSZ );
    }

if ( brect.BrectL_YPixels == 0 )
    {
    invroi = TRUE;
    ChfSignal( 1, ImgX_INVBRYSZ );
    }

if ( invroi )
    ChfStop( 1, ImgX_INVROI );

return;
} /* end of Verify_rect_roi */
