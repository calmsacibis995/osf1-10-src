
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

/*******************************************************************************
**  IMG_FRAME_UTILS
**
**  FACILITY:
**
**      Image Services Library
**
**  ABSTRACT:
**
**	The utility functions in this module are for use in creating,
**	copying, modifying, and deleting image frames.  As high level
**	functions, they are all object oriented, and pass frame identifiers
**	as input and output.
**
**  ENVIRONMENT:
**
**      VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
**	Mark W. Sornson
**
**  CREATION DATE:     
**
**	12-SEPT-1989
**
*******************************************************************************/

/*
**  Include files 
*/
#include <stdio.h>
#include <stdlib.h>		/* for getenv() */

#include <img/ChfDef.h>
#include <img/ImgDef.h>
#include <ImgDefP.h>
#include <ImgMacros.h>
#include <ImgVectorTable.h>
#ifndef NODAS_PROTO
#include <imgprot.h>	    /* Img prototypes */
#endif

#if defined(__VMS) || defined(VMS)
#include <cda$msg.h>
#include <ddif$def.h>
#else
#if defined(NEW_CDA_SYMBOLS)
#include <cdamsg.h>
#include <ddifdef.h>
#else
#include <cda_msg.h>
#include <ddif_def.h>
#endif
#if defined(NEW_CDA_CALLS)
#include <cdaptp.h>
#else
#include <cda_ptp.h>
#endif
#endif

/*
**  Table of contents
**
**	Global routines
**
**	VMS Entry Points:
*/
#if defined(__VMS) || defined(VMS)
struct FCT  *IMG$ALLOCATE_FRAME();
struct FCT  *IMG$CONVERT_FRAME();
struct FCT  *IMG$COPY();
struct FCT  *IMG$COPY_FRAME();
struct FCT  *IMG$CREATE_FRAME();
void	     IMG$DEALLOCATE_FRAME();
void	     IMG$DELETE_FRAME();
struct FCT  *IMG$STANDARDIZE_FRAME();
void	     IMG$DUMP_FRAME();
void	     IMG$VERIFY_FRAME();
#endif

/*
**	Portable Entry Points (for VMS and ULTRIX)
*/
#ifdef NODAS_PROTO
struct FCT  *ImgAllocateFrame();
struct FCT  *ImgConvertFrame();
struct FCT  *ImgCopy();
struct FCT  *ImgCopyFrame();
struct FCT  *ImgCreateFrame();
void	     ImgDeallocateFrame();
void	     ImgDeleteFrame();
void	     ImgDumpFrame();
void	     ImgSaveFrame();
struct FCT  *ImgStandardizeFrame();
void	     ImgVerifyFrame();
#endif

/*
**  MACRO definitions
**
**	none
*/

/*
**  Equated Symbols
*/
#define NUM_BYTES_DEFAULT 1056000      /* number of bytes in 8 1/2 by 11 page */
#define FALSE 0

struct ALIGN_ATTRS {
    long    data_offset;
    long    pixel_stride;
    long    plane_bits_per_pixel;
    long    scanline_stride;
    };

typedef struct _ITEM_CODE {
    unsigned long    item_code;
    char	    *item_code_str;
    } ITEM_CODE;

/*
**	Module local routines
*/
#ifdef NODAS_PROTO
static void		 Attach_data_planes();
static struct ITMLST	*Setup_cvt_alignment();
#else
PROTO(static void Attach_data_planes, (struct FCT */*fid*/));
PROTO(static struct ITMLST *Setup_cvt_alignment, (struct FCT */*fid*/, struct ITMLST */*src_itmlst*/, struct ALIGN_ATTRS */*align_attrs*/));

PROTO(static void ImgDumpCodingAttrs, (struct FCT */*fct*/, FILE */*fp*/));
PROTO(static void ImgDumpCompSpaceAttrs, (struct FCT */*fct*/, FILE */*fp*/));
PROTO(static void ImgDumpFrameParams, (struct FCT */*fct*/, FILE */*fp*/));
PROTO(static void ImgDumpMiscAttrs, (struct FCT */*fct*/, FILE */*fp*/));
PROTO(static void ImgDumpPresentAttrs, (struct FCT */*fct*/, FILE */*fp*/));
PROTO(static void ImgDumpStructure, (struct FCT */*fct*/, FILE */*fp*/));
PROTO(static void ImgGetAndPrintLongAttr, (FILE * /*fp*/, struct FCT */*fid*/,
	ITEM_CODE */*item_code*/, unsigned long /*index*/ ));

PROTO(static void _ImgFixVAXCameraImage, (struct FCT */*fid*/ ));
PROTO(static int _ImgVerifyVAXcameraImage, (struct FCT */*fid*/ ));
#endif

/*
**  External References
**       CDA Toolkit references are in <cdaptp.h>
*/

#ifdef NODAS_PROTO
struct FCT	     *ImgAdjustFrameDef();
char		     *ImgAllocateDataPlane();
struct FCT	     *ImgAttachDataPlane();
struct FCT	     *ImgCvtAlignment();
struct FCT	     *ImgCvtCompSpaceOrg();
struct FCT	     *ImgCreateFrameDef();
void		      ImgDeleteFrameDef();
unsigned long	      ImgExtractRoi();
struct FCT	     *ImgGetFrameSize();
void		      ImgResetCtx();
struct FCT	     *ImgSetFrameSize();
long		      ImgSetRectRoi();
long		      ImgUnsetRectRoi();
void		      ImgVerifyFrameDef();

char		    *_ImgCalloc();
long		     _ImgCopy();
struct FCT	    *_ImgCopyFrameDef();
struct ITMLST	    *_ImgCreateItmlst();
struct ITMLST	    *_ImgCvtPutlstToItmlst();
void		     _ImgDeleteItmlst();
void		     _ImgErrorHandler();
long		     _ImgExtractItmlstItem();
long		     _ImgExtractPutlstItem();
void		     _ImgFrameDealloc();
void		     _ImgFree();
long		     _ImgGet();
struct ITMLST	    *_ImgGetStandardizedAttrlst();
long		     _ImgGetVerifyStatus();
long		     _ImgNextContentElement();
void		     _ImgPut();
long		     _ImgTotalPutlstItems();
long		     _ImgVerifyAttributes();
long		     _ImgVerifyDataPlanes();
long		     _ImgVerifyStandardFormat();
void		     _ImgVerifyStructure();

void	     ChfSignal();			/* Signal condition codes   */
void         ChfStop();
#endif


/*
** External symbol definitions (status codes)
*/
#include <img/ImgStatusCodes.h>
    
/*
**  Local Storage
**
*/
static struct ITMLST empty_itmlst = { 0,0,0,0,0 };

/******************************************************************************
**  IMG$ALLOCATE_FRAME
**  ImgAllocateFrame
**
**  FUNCTIONAL DESCRIPTION:
**
**	Allocate an image frame.  Frame allocation allocates all 
**	internal data structure for storing attribute data, and
**	allocates and attaches data planes to uncompressed frames
**	(i.e., which use PCM encoding) if enough attribute information
**	is supplied in order to determine data plane size.
**
**	Frame allocate requires the attributes of the frame to be 
**	described using one of the following combinations of arguments:
**
**	    Allocate empty frame with default attributes:
**		ImgAllocateFrame( data_class, 0, 0, 0 )
**
**	    Allocate frame and attach data:
**		ImgAllocateFrame( data_class, itmlst, 0, 0 )
**		ImgAllocateFrame( 0, 0, frame_def, 0 )
**		ImgAllocateFrame( 0, itmlst, frame_def, 0 )
**
**	    Allocate empty frame with specified attributes:
**		flags = ImgM_NoDataPlaneAlloc
**		ImgAllocateFrame( data_class, itmlst, 0, flags )
**		ImgAllocateFrame( 0, 0, frame_def, flags )
**		ImgAllocateFrame( 0, itmlst, frame_def, flags )
**
**  FORMAL PARAMETERS:
**
**	data_class  longword (unsigned), read only, by value
**		    Image data class specifier:	
**
**			ImgK_ClassPrivate
**			ImgK_ClassBitonal
**			ImgK_ClassGreyscale
**			ImgK_ClassMultispect
**
**	itmlst	    ITMLST, read only, by reference
**		    Item list of attributes to be given to the frame.
**
**	frame_def   pointer to FCT (frame_def), read only, by value
**		    Frame definition specifier to be used to define
**		    the frame attributes.
**
**	flags	    longword (unsigned), read only, by value
**		    Flags specifier:
**
**			ImgM_NoDataPlaneAlloc	Inhibit data plane allocation
**			ImgM_AutoDealloc	Automatically deallocate the
**						input frame definition.
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
**	frame-id    longword (unsigned), write only, by value
**		    Frame identifier of allocated frame.
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
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$ALLOCATE_FRAME( data_class, itmlst, frame_def, flags )
unsigned long	     data_class;
struct ITMLST	    *itmlst;
struct FCT	    *frame_def;
unsigned long	     flags;
{
struct FCT  *frame_id;

frame_id = ImgAllocateFrame( data_class, itmlst, frame_def, flags );
return frame_id;
} /* end of IMG$ALLOCATE_FRAME */
#endif


/*******************************************************************************
** Portable entry point
*******************************************************************************/
struct FCT *ImgAllocateFrame( data_class, itmlst, frame_def, flags )
unsigned long	     data_class;
struct ITMLST	    *itmlst;
struct FCT	    *frame_def;
unsigned long	     flags;
{
long		     auto_deallocate	    = FALSE;
long		     local_flags	    = 0;
long		     working_frame_created  = FALSE;
struct FCT	    *frame_id;
struct FCT	    *working_frame_def	    = frame_def;
struct ITMLST	    *local_itmlst	    = itmlst;

if ( itmlst == 0 )
    local_itmlst = &empty_itmlst;

if ( working_frame_def == 0 )
    /*
    ** Create a frame definition based on the data class and itmlst.
    */
    {
    working_frame_def = ImgCreateFrameDef( data_class, local_itmlst );
    working_frame_created = TRUE;
    }
else
    /*
    ** Verify the input frame def.  Note that since this might actually
    ** be another image frame, just explicitly do the same checking
    ** that ImgVerifyFrameDef does (but without the FDF block type check).
    */
    {
    if ( VERIFY_ON_ )
	{
	_ImgVerifyStructure( working_frame_def );
	_ImgVerifyAttributes( working_frame_def, 0 );
	}

    /*
    ** Indicate whether the input frame def is to be discarded
    ** by this function.
    */
    if ( (flags&ImgM_AutoDeallocate) != 0 )
	auto_deallocate = TRUE;
    }

/*
** If an itemlist was passed in, adjust the working frame definition 
** using the item list, and use the resulting definition as the skeleton 
** of the frame to be returned.
*/
if ( (local_itmlst != 0) && (local_itmlst->ItmL_Code != 0) )
    {
    frame_id = ImgAdjustFrameDef( working_frame_def, local_itmlst, 0 );
    }
else
    /*
    ** Since no adjustments need to be made, convert the frame
    ** definition into a frame id with no processing.
    */
    {
    if ( auto_deallocate )
	{
	/*
	** Since the working frame def was passed in and is supposed
	** to be disgarded, simply recycle it immediately by using
	** it as the return frame id.  Set the flag to false so the
	** working_frame_def (which as become the frame id) does not
	** get deallocated.
	*/
	frame_id = working_frame_def;
	auto_deallocate = FALSE;
	}
    else
	frame_id = _ImgCopyFrameDef( working_frame_def, 0 );
    }

/*
** Officially mark the frame-id structure to be a FRAME (FCT) rather
** than a FRAME DEF (FDF).
*/
frame_id->FctR_Blkhd.BhdB_Type = ImgK_BlktypFct;

/*
** Allocate the data planes for the frame unless explicitly
** directed otherwise.
*/
if ( (flags&ImgM_NoDataPlaneAlloc) == 0 )
    Attach_data_planes( frame_id );

/*
** Delete the working frame-def if created in this function, 
** and return the frame_id after verification.
*/
if ( working_frame_created || auto_deallocate )
    ImgDeleteFrameDef( working_frame_def );

/*
** Verify that the frame is OK ...
**
**  NOTE: allow frame to be non standard if so flagged by the caller
*/
if ( (flags&ImgM_NoStandardize) != TRUE &&
     (flags&ImgM_NonstandardVerify) != TRUE )
    local_flags = ImgM_NonstandardVerify;

if ( itmlst == 0 && frame_def == 0 )
    local_flags |= ImgM_NoAttrVerify;

if ( (flags&ImgM_NoDataPlaneAlloc) != 0 )
    local_flags |= ImgM_NoDataPlaneVerify;

if ( VERIFY_ON_ )
    ImgVerifyFrame( frame_id, local_flags );

return frame_id;
} /* end of ImgAllocateFrame */


/******************************************************************************
**  IMG$CONVERT_FRAME
**  ImgConvertFrame
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
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$CONVERT_FRAME( srcfid, itmlst, flags )
struct FCT	*srcfid;
struct ITMLST	*itmlst;
long		 flags;
{
struct FCT  *retfid;

retfid = ImgConvertFrame( srcfid, itmlst, flags );
return retfid;
} /* end of IMG$CONVERT_FRAME */
#endif


/*******************************************************************************
** Portable entry point
*******************************************************************************/
struct FCT *ImgConvertFrame( srcfid, itmlst, flags )
struct FCT	*srcfid;
struct ITMLST	*itmlst;
long		 flags;
{
long	     alignment_converted    = FALSE;
long	     compression_type;
long	     cvt_itemcode;
long	     cvt_itemvalue;
long	     found_status;
long	     this_is_the_first_cvt  = TRUE;
long	     local_flags;
long	     ret_cs_org;
long	     ret_dp_signif;
long	     src_cs_org;
long	     src_dp_signif;
long	     src_itemvalue;

struct FCT	*retfdf;
struct FCT	*retfid;
struct FCT	*srcfdf;
struct FCT	*wrkfid		= srcfid;
struct ITMLST	*local_itmlst   = itmlst;

/*
** Verify that the structure and content of the frame is consistent.
*/
local_flags = ImgM_NonstandardVerify ;
if ( VERIFY_ON_ )
    ImgVerifyFrame( srcfid, local_flags );

/*
** CONFORMANCE checking:
**
**  - Prohibit conversions on compresed data.
*/
_ImgGet( srcfid, Img_CompressionType, &compression_type, LONGSIZE, 0, 0 );
if ( compression_type != ImgK_PcmCompression )
    ChfStop( 1, ImgX_FRMNOTUNC );

/*
** Set up a dummy item list if one is not passed in.
*/
if ( itmlst == 0 )
    local_itmlst = &empty_itmlst;

/*
** Convert the frame characteristics.
*/
for ( ; local_itmlst->ItmL_Code != 0 ; ++local_itmlst )
    {
    cvt_itemcode = local_itmlst->ItmL_Code;
    switch (local_itmlst->ItmL_Length)
      {
      case 1:
	cvt_itemvalue = *(local_itmlst->ItmA_Buffer);
	break;
      case 2:
	cvt_itemvalue = *((short *) local_itmlst->ItmA_Buffer);
	break;
      case 4:
	cvt_itemvalue = *((int *) local_itmlst->ItmA_Buffer);
	break;
      case 8:
	cvt_itemvalue = *((long *) local_itmlst->ItmA_Buffer);
	break;
      }
    switch ( cvt_itemcode )
	{
	case Img_BitOrder:
	    break;
	case Img_BrtPolarity:
	    break;
	case Img_ByteOrder:
	    break;
	case Img_CompressionType:
	    break;

	case Img_CompSpaceOrg:
	    {
	    _ImgGet( srcfid, Img_CompSpaceOrg, &src_cs_org, LONGSIZE, 0, 0 );
	    _ImgGet( srcfid, Img_PlaneSignif, &src_dp_signif, LONGSIZE, 0, 0 );

	    /*
	    ** Look for the data plane signif in the next item.  If 
	    ** not present, use the source frame value as the default.
	    */
	    if ( local_itmlst[1].ItmL_Code == Img_PlaneSignif )
		{
		ret_dp_signif = *((long*)(local_itmlst[1].ItmA_Buffer));
		++local_itmlst;
		}
	    else
		_ImgGet( srcfid, Img_PlaneSignif, &ret_dp_signif, LONGSIZE,0,0);

	    if ( src_cs_org != cvt_itemvalue ||
		 (src_cs_org == ImgK_BitIntrlvdByPlane &&
		  src_dp_signif != ret_dp_signif ) )
		{
		if ( this_is_the_first_cvt )
		    {
		    local_flags = 0;
		    this_is_the_first_cvt = FALSE;
		    }
		else
		    local_flags = ImgM_AutoDeallocate;

		wrkfid = ImgCvtCompSpaceOrg( wrkfid, cvt_itemvalue, 
						ret_dp_signif, local_flags );

		}
	    break;
	    } /* end case Img_CompSpaceOrg */

	case Img_DataOffset:
	case Img_PixelAlignment:
	case Img_ScanlineAlignment:
	    /*
	    ** If anyone of these is specified, convert the alignment
	    ** for all the items in the item list.
	    */
	    {
	    if ( !alignment_converted )
		{
		long		     dp_cnt;
		struct ALIGN_ATTRS  *align_attrs;
		struct ITMLST	    *cvt_itmlst;

		_ImgGet( wrkfid, Img_PlanesPerPixel, &dp_cnt, LONGSIZE, 0, 0);
		align_attrs = (struct ALIGN_ATTRS *) 
		  _ImgCalloc( dp_cnt, sizeof(struct ALIGN_ATTRS) );
		cvt_itmlst = Setup_cvt_alignment( wrkfid, local_itmlst,
				align_attrs );

		if ( this_is_the_first_cvt )
		    {
		    local_flags = 0;
		    this_is_the_first_cvt = FALSE;
		    }
		else
		    local_flags = ImgM_AutoDeallocate;
		wrkfid = ImgCvtAlignment( wrkfid, cvt_itmlst, local_flags );

		_ImgDeleteItmlst( cvt_itmlst );
		_ImgFree( align_attrs );
		alignment_converted = TRUE;
		}
	    break;
	    } /* end case Img_DataOffset, Img_Pixel/ScanlineAlignment */
	case Img_DataType:
	    break;
	case Img_GridType:
	    break;
	case Img_LineProgression:
	    break;
	case Img_PixelGroupOrder:
	    break;
	case Img_PixelPath:
	    break;
	case Img_PlaneSignif:
	    {
	    _ImgGet( srcfid, Img_CompSpaceOrg, &src_cs_org, LONGSIZE, 0, 0 );
	    _ImgGet( srcfid, Img_PlaneSignif, &src_dp_signif, LONGSIZE, 0, 0 );

	    if ( src_cs_org == ImgK_BitIntrlvdByPlane && 
		 src_dp_signif != cvt_itemvalue )
		{
		if ( this_is_the_first_cvt )
		    {
		    local_flags = 0;
		    this_is_the_first_cvt = FALSE;
		    }
		else
		    local_flags = ImgM_AutoDeallocate;

		wrkfid = ImgCvtCompSpaceOrg( wrkfid, src_cs_org, 
						cvt_itemvalue, local_flags );
		}
	    break;
	    } /* end case Img_PlaneSignif */

	case Img_SpectralMapping:
	    break;
	default:
	    break;
	} /* end switch */
    } /* end for */

/*
** If no item list was specified, make a copy of the 
** source frame.
*/
if ( itmlst == 0 || itmlst->ItmL_Code == 0 )
    {
    wrkfid = ImgCopyFrame( srcfid, 0 );
    }

/*
** Delete the source frame?
*/
if ( (flags&ImgM_AutoDeallocate) != 0 )
    {
    ImgDeallocateFrame( srcfid );
    srcfid = 0;
    }

/*
** If no conversions (and no copy) took place, the frame didn't
** need to be converted; therefore, return the source frame.
*/
if ( wrkfid == srcfid )
    retfid = srcfid;
else
    retfid = wrkfid;

if ( VERIFY_ON_ )
    ImgVerifyFrame( retfid, ImgM_NonstandardVerify );

return retfid;
} /* end of ImgConvertFrame */


/******************************************************************************
**  IMG$COPY
**  ImgCopy
**  IMG$COPY_FRAME
**  ImgCopyFrame
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
/*******************************************************************************
** V 2.0 entry points
**
** VMS specific entry point
*******************************************************************************/
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$COPY( srcfid, roi, itmlst )
struct FCT	    *srcfid;
unsigned long	     roi;
unsigned long	     itmlst;
{
struct FCT  *retfid;

retfid = ImgCopy( srcfid, roi, itmlst );
return retfid;
} /* end of IMG$COPY */
#endif

/*
** Portable entry point
*/
struct FCT *ImgCopy( src_fid, roi, itmlst )
struct FCT *src_fid;
struct ROI *roi;
struct PUT_ITMLST   *itmlst;
{
long	ret_data_offset	    = 0;
struct FCT *ret_fid;
long	ret_scanline_stride;
long    std_cs_org          = ImgK_BandIntrlvdByPlane;
long    saved_cs_org;
long    saved_data_class;
long    saved_dp_signif;
long    status;
struct FCT *tmp_fid_1;
struct FCT *tmp_fid_2;

struct ITMLST	*local_itmlst	= 0;

if ( itmlst != 0 )
    if ( itmlst->PutL_Code != 0 )
	{
	local_itmlst = _ImgCvtPutlstToItmlst( itmlst );
	}

if ( roi != 0 )
    {
    /*
    ** Verify the source frame ...
    */
    if ( !VERIFY_OFF_ )
	ImgVerifyFrame( src_fid, ImgM_NonstandardVerify );

    /*
    ** Convert the source frame to standard format if necessary
    */
    _ImgGet( src_fid, Img_ImageDataClass, &saved_data_class, LONGSIZE, 0, 0 );
    _ImgGet( src_fid, Img_CompSpaceOrg, &saved_cs_org, LONGSIZE, 0, 0 );
    _ImgGet( src_fid, Img_PlaneSignif, &saved_dp_signif, LONGSIZE, 0, 0 );
    if ( (saved_data_class == ImgK_ClassBitonal ||
	  saved_data_class == ImgK_ClassGreyscale ) &&
	  saved_cs_org == ImgK_BandIntrlvdByPixel )
	{
	_ImgPut( src_fid, Img_CompSpaceOrg, &std_cs_org, LONGSIZE, 0 );
	}

    status = _ImgVerifyStandardFormat( src_fid, ImgM_NoChf );
    if ( !(status & 1) )
        tmp_fid_1 = ImgStandardizeFrame( src_fid, 0 );
    else
        tmp_fid_1 = src_fid;

    /*
    ** Call V3.0 entry point ...
    */
    tmp_fid_2 = ImgExtractRoi( tmp_fid_1, roi, 0 );

    /*
    ** Convert Comp. Space Org of the resultant frame back to the original
    ** saved value if the original value was not band interleaved by plane.
    */
    if ( saved_cs_org != ImgK_BandIntrlvdByPlane )
	{
	ret_fid =
	  ImgCvtCompSpaceOrg( tmp_fid_2,saved_cs_org,saved_dp_signif,0);
	ImgDeallocateFrame( tmp_fid_2 );
	_ImgPut( src_fid, Img_CompSpaceOrg, &saved_cs_org, LONGSIZE, 0 );
	}
    else
	ret_fid = tmp_fid_2;

    if ( tmp_fid_1 != src_fid )
	ImgDeallocateFrame( tmp_fid_1 );

    /*
    ** If attributes have been supplied, align the frame using them.
    */
    if ( local_itmlst != 0 && local_itmlst->ItmL_Code != 0 )
	{
	tmp_fid_1 = ret_fid;
	ret_fid = ImgCvtAlignment( tmp_fid_1, local_itmlst, 0 );
	ImgDeallocateFrame( tmp_fid_1 );
	}
    }
else
    ret_fid = ImgCvtAlignment( src_fid, local_itmlst, 0 );

if ( local_itmlst != 0 )
    _ImgDeleteItmlst( local_itmlst );

if ( !VERIFY_OFF_ )
    ImgVerifyFrame( ret_fid, ImgM_NonstandardVerify );

return ret_fid;
} /* end of Img$Copy */


/*******************************************************************************
** V 3.0 entry points
**
** VMS specific entry point
*******************************************************************************/
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$COPY_FRAME( srcfid, flags )
struct FCT	*srcfid;
unsigned long	 flags;
{
struct FCT  *retfid;

retfid = ImgCopyFrame( srcfid, flags );
return retfid;
} /* end of IMG$COPY_FRAME */
#endif


/*******************************************************************************
** Portable entry point
*******************************************************************************/
struct FCT *ImgCopyFrame( srcfid, flags )
struct FCT	*srcfid;
unsigned long	 flags;
{
char	    *dst_dp;
float	     change_ratio_x;
float	     change_ratio_y;
long	     data_offset	= 0;
long	     dp_cnt;
long	     dp_idx;
long	     frame_size_x;
long	     frame_size_y;
long	     index;
long	     local_flags;
long	     status;
long	     spectral_type;		/* spectral type		    */

struct UDP   dstudp;
struct FCT  *frame_def;
struct UDP  *rect_roi_info;
struct FCT  *retfid;
struct UDP   srcudp;

local_flags = ImgM_NonstandardVerify;
if ( VERIFY_ON_ )
    ImgVerifyFrame( srcfid, local_flags );

/*
** Allocate a new frame ...
*/
local_flags = ImgM_NoDataPlaneAlloc;
retfid = ImgAllocateFrame( 0, 0, srcfid, local_flags );

/*
** If a rect roi has been set in the source, zero the data offset in
** the return fid.
*/
_ImgGet( srcfid, Img_RectRoiInfo, &rect_roi_info, LONGSIZE, 0, 0 );
if ( rect_roi_info != 0 )
    {
    _ImgGet( retfid, Img_PlanesPerPixel, &dp_cnt, LONGSIZE, 0, 0 );
    for ( index = 0; index < dp_cnt; ++index )
	_ImgPut( retfid, Img_DataOffset, &data_offset, LONGSIZE, index );
    }

/*
** Copy the src data into the ret fid.
*/
/*ImgSaveCtx( srcfid );
**ImgSaveCtx( retfid );
*/
ImgResetCtx( srcfid );
ImgResetCtx( retfid );

/*
** Tell IPS to retain the source dimensions.  (Fixes a bug where the
** IDU_PIXELS_PER_LINE (which is byte aligned) is not equal to the
** IDU_SCANLINE_STRIDE (which may be word aligned).
*/
local_flags = IpsM_RetainSrcDim;

do
    {
    _ImgGet( retfid, Img_PlanesPerPixel, &dp_cnt, LONGSIZE, 0, 0 );
    for ( dp_idx = 0; dp_idx < dp_cnt; ++dp_idx )
	{
	_ImgGet( srcfid,Img_ImageDataClass, &spectral_type,
		 sizeof(spectral_type),0,dp_idx );
	_ImgGet( srcfid, Img_Udp, &srcudp, sizeof( srcudp ), 0, dp_idx );

	dstudp.UdpA_Base = 0;

	if (spectral_type == ImgK_ClassBitonal)
		status = (*ImgA_VectorTable[ImgK_CopyBitonal])(
			    &srcudp, &dstudp, local_flags );
	else
		status = (*ImgA_VectorTable[ImgK_Copy])(
			    &srcudp, &dstudp, local_flags );

        if ((status & 1) != 1)
	    {
	    ImgDeallocateFrame( retfid );
	    _ImgErrorHandler(status);
	    return;
	    }

	_ImgPut( retfid, Img_Udp, &dstudp, sizeof( dstudp ), dp_idx );
	}
    _ImgNextContentElement( srcfid );
    status = _ImgNextContentElement( retfid );
    }
while ( status );

/*ImgRestoreCtx( srcfid );
**ImgRestoreCtx( retfid );
*/

return retfid;
} /* end of ImgCopyFrame */


/******************************************************************************
**  IMG$CREATE_FRAME - Create an image frame structure
**  ImgCreateFrame
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine creates a frame structure from data and attributes 
**	specified by item list parameters.
**
**  FORMAL PARAMETERS:
**
**      itmlst		PUT_ITMLST, read only, by reference.
**			Item list structure which specifies the
**			attributes of the frame.
**
**	data_class	longword (unsigned), read only, by value
**			Data class of image frame to create:
**
**			    ImgK_ClassPrivate
**			    ImgK_ClassBitonal
**			    ImgK_ClassGreyscale
**			    ImgK_ClassMultispect
**
**			This argument is optional.  If a zero value is passed
**			in, ImgK_ClassBitonal is used by default.
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      frame_id - id value of the created frame aka fct
**
**  SIGNAL CODES:
**
**      ImgX_INVARGCNT	Invalid argument count
**
**  SIDE EFFECTS:
**
**      none
******************************************************************************/
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$CREATE_FRAME( itmlst, data_class )
struct PUT_ITMLST   *itmlst[];
int		     data_class;
{
unsigned long   argcnt;

return ( ImgCreateFrame( itmlst, data_class ) );
} /* end of IMG$CREATE_FRAME */
#endif


/*******************************************************************************
** Portable entry point
*******************************************************************************/
struct FCT *ImgCreateFrame( itmlst, data_class )
struct PUT_ITMLST   *itmlst;
unsigned long	     data_class;
{
long		     add_332_bpc	= 0;
long		     attr_idx;
long		     bpc_0_found	= 0;	/* bits per comp found	*/
long		     default_bpc_red	= 3;	/* default bits per comp red */
long		     default_bpc_green	= 3;	/* def. bits per comp green */
long		     default_bpc_blue	= 2;	/* def. bits per comp blue */
long		     default_cs_org	= ImgK_BandIntrlvdByPixel;
long		     element_cnt_1;
long		     element_cnt_2;
long		     cs_org_found	= 0;
long		     flags		= ImgM_NoDataPlaneAlloc |
					  ImgM_NoDataPlaneVerify;
long		     idx;
long		     length;
long		     occurance		= 1;
long		     user_bpc_0		= 0;
long		     user_cs_org	= 0;
struct FCT	    *fct;
struct ITMLST	    *local_itmlst	= 0;
struct ITMLST	    *local_itmlst_tmp	= 0;
struct PUT_ITMLST   *local_put_itmlst	= itmlst;

static long		one		= 1;
static long		two		= 2;
static long		three		= 3;
static long		four		= 4;
static long		eight		= 8;
static long		v2_cs_org	= ImgK_BandIntrlvdByPixel;

static struct ITMLST	v2_bitonal_defaults[] = {
			     { Img_CompSpaceOrg, sizeof(long), (char *)&v2_cs_org, 0,0 }
			    ,{ 0, 0, 0, 0, 0 }
			    };
static struct ITMLST	v2_greyscale_defaults[] = {
			     { Img_CompSpaceOrg, sizeof(long), (char *)&v2_cs_org, 0,0 }
			    ,{ 0, 0, 0, 0, 0 }
			    };
static struct ITMLST	v2_multispect_defaults[] = {
			     { Img_CompSpaceOrg, sizeof(long), (char *)&v2_cs_org, 0,0 }
			    ,{ Img_QuantLevelsPerComp, sizeof(long),(char *)&eight,0,0 }
			    ,{ Img_QuantLevelsPerComp, sizeof(long),(char *)&eight,0,1 }
			    ,{ Img_QuantLevelsPerComp, sizeof(long),(char *)&four,0,2 }
			    ,{ Img_BitsPerComp, sizeof(long), (char *)&three, 0, 0 }
			    ,{ Img_BitsPerComp, sizeof(long), (char *)&three, 0, 1 }
			    ,{ Img_BitsPerComp, sizeof(long), (char *)&two, 0, 2 }
			    ,{ Img_PlaneBitsPerPixel, sizeof(long),(char *)&eight, 0,0 }
			    ,{ Img_PixelStride, sizeof(long), (char *)&eight, 0,0 }
			    ,{ 0, 0, 0, 0, 0 }
			    };

if ( data_class == 0)
    data_class = ImgK_ClassBitonal;

/*
** Copy the PUT_ITMLST into a local ITMLST structure for 
** compatibility with ImgAllocateFrame.
*/
if ( itmlst != 0 )
    {
    /*
    ** See if the user passed in the comp space org item ... if
    ** missing, add it to the end of the ITMLST 
    */
    cs_org_found = _ImgExtractPutlstItem( itmlst, Img_CompSpaceOrg, 
			&user_cs_org, &length, &attr_idx,
			occurance );

    /*
    ** See if the user passed in the bits per comp item for
    ** index 0.  If missing, and the data class is multi-spectral,
    ** set the bits per comp to be 3,3,2 for RGB, respectively.
    */
    bpc_0_found = _ImgExtractPutlstItem( itmlst, Img_BitsPerComp, 
			&user_bpc_0, &length, &attr_idx,
			occurance );

    /*
    ** Get the itemlist length (not including the terminator field)
    **
    **	NOTE: if the user omitted the CS org attr, add an extra
    **	      element to the list
    */
    element_cnt_1 = _ImgTotalPutlstItems( itmlst );
    element_cnt_2 = element_cnt_1;
    if ( !cs_org_found )
	++element_cnt_2;

    /*
    ** See if we have to add bits per comp for multispectral images
    */
    if ( !cs_org_found && !bpc_0_found && 
	 data_class == ImgK_ClassMultispect )
	{
	element_cnt_2 += 3;
	add_332_bpc = 1;
	}

    /*
    ** Allocate an ITMLST item list including the terminator field.
    */
    local_itmlst = _ImgCreateItmlst( element_cnt_2 + 1 );
    local_itmlst_tmp = local_itmlst;
    for ( idx = 0; idx < element_cnt_1; ++idx )
	{
	local_itmlst_tmp->ItmL_Code	= local_put_itmlst->PutL_Code;
	local_itmlst_tmp->ItmL_Length   = local_put_itmlst->PutL_Length;
	local_itmlst_tmp->ItmA_Buffer   = local_put_itmlst->PutA_Buffer;
	local_itmlst_tmp->ItmA_Retlen   = 0;
	local_itmlst_tmp->ItmL_Index    = local_put_itmlst->PutL_Index;
	++local_itmlst_tmp;
	++local_put_itmlst;
	}
    /*
    ** Add the default CS org to the list if it wasn't passed in.
    */
    if ( !cs_org_found )
	{
	local_itmlst_tmp->ItmL_Code	= Img_CompSpaceOrg;
	local_itmlst_tmp->ItmL_Length   = sizeof( long );
	local_itmlst_tmp->ItmA_Buffer   = (char *)&default_cs_org;
	local_itmlst_tmp->ItmA_Retlen   = 0;
	local_itmlst_tmp->ItmL_Index    = 0;
	++local_itmlst_tmp;
	}

    if ( add_332_bpc)
	{
	local_itmlst_tmp->ItmL_Code	= Img_BitsPerComp;
	local_itmlst_tmp->ItmL_Length   = sizeof( long );
	local_itmlst_tmp->ItmA_Buffer   = (char *) &default_bpc_red;
	local_itmlst_tmp->ItmA_Retlen   = 0;
	local_itmlst_tmp->ItmL_Index    = 0;
	++local_itmlst_tmp;

	local_itmlst_tmp->ItmL_Code	= Img_BitsPerComp;
	local_itmlst_tmp->ItmL_Length   = sizeof( long );
	local_itmlst_tmp->ItmA_Buffer   = (char *) &default_bpc_green;
	local_itmlst_tmp->ItmA_Retlen   = 0;
	local_itmlst_tmp->ItmL_Index    = 1;
	++local_itmlst_tmp;

	local_itmlst_tmp->ItmL_Code	= Img_BitsPerComp;
	local_itmlst_tmp->ItmL_Length   = sizeof( long );
	local_itmlst_tmp->ItmA_Buffer   = (char *) &default_bpc_blue;
	local_itmlst_tmp->ItmA_Retlen   = 0;
	local_itmlst_tmp->ItmL_Index    = 2;
	++local_itmlst_tmp;
	}
    }
else
    /*
    ** No itemlist was passed in.  Make sure V2 attributes are
    ** used for the appropriate data class.
    */
    {
    switch ( data_class )
	{
	case ImgK_ClassBitonal:
	    local_itmlst = v2_bitonal_defaults;
	    break;
	case ImgK_ClassGreyscale:
	    local_itmlst = v2_greyscale_defaults;
	    break;
	case ImgK_ClassMultispect:
	    local_itmlst = v2_multispect_defaults;
	    break;
	default:
	    break;
	}
    }

fct = ImgAllocateFrame( data_class, local_itmlst, 0, flags );

if ( local_itmlst != 0 )
    _ImgDeleteItmlst( local_itmlst );

return fct;
} /* end of ImgCreateFrame */


/******************************************************************************
**  IMG$DEALLOCATE_FRAME
**  ImgDeallocateFrame
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
#if defined(__VMS) || defined(VMS)
void IMG$DEALLOCATE_FRAME( fid )
struct FCT  *fid;
{

ImgDeallocateFrame( fid );
return;
} /* end of IMG$DEALLOCATE_FRAME */
#endif


/*******************************************************************************
** Portable entry point
*******************************************************************************/
void ImgDeallocateFrame( fid )
struct FCT  *fid;
{

if ( !VERIFY_OFF_ )
    _ImgVerifyStructure( fid );

_ImgFrameDealloc( fid );

return;
} /* end of ImgDeallocateFrame */


/************************************************************************
**  IMG$DELETE_FRAME
**
**  FUNCTIONAL DESCRIPTION:
**
**      Deletes a frame.
**
**  FORMAL PARAMETERS:
**
**      fid - A Frame ID
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
************************************************************************/
#if defined(__VMS) || defined(VMS)
void IMG$DELETE_FRAME( fid )
struct FCT  *fid;
{

ImgDeleteFrame( fid );
return;
} /* end of IMG$DELETE_FRAME */
#endif


/*******************************************************************************
** Portable entry point
*******************************************************************************/
void ImgDeleteFrame( fid )
struct FCT  *fid;
{

ImgDeallocateFrame( fid );
} /* end of ImgDeleteFrame */


/******************************************************************************
**  IMG$DUMP_FRAME
**  ImgDumpFrame
**
**  FUNCTIONAL DESCRIPTION:
**
**	Dump the attributes of a frame in readable text.
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
#if defined(__VMS) || defined(VMS)
void IMG$DUMP_FRAME( fid, outfile )
unsigned long	 fid;
char		*outfile;
{
ImgDumpFrame( fid, outfile );

return;
} /* end of IMG$DUMP_FRAME */
#endif


void ImgDumpFrame( fid, outfile )
struct FCT	*fid;
char		*outfile;
{
char	*fname	= 0;
FILE	*fp	= stdout;

if ( outfile != 0 )
    fname = outfile;
else
    fname = getenv( "IMG_DUMP_FRAME_FILE" );

if ( fname != 0 )
    {
    fp = fopen( fname, "w" );
    if ( fp == 0 )
	fp = stdout;
    }

ImgDumpPresentAttrs( fid, fp );
ImgDumpCompSpaceAttrs( fid, fp );
ImgDumpFrameParams( fid, fp );
ImgDumpCodingAttrs( fid, fp );
ImgDumpMiscAttrs( fid, fp );
ImgDumpStructure( fid, fp );

if ( fp != stdout )
    fclose( fp );

return;
} /* end of ImgDumpFrame */


/******************************************************************************
**  IMG$SAVE_FRAME
**  ImgSaveFrame
**
**  FUNCTIONAL DESCRIPTION:
**
**	Save the frame to a file.
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
#if defined(__VMS) || defined(VMS)
void IMG$SAVE_FRAME( fid, outfile )
unsigned long	 fid;
char		*outfile;
{
ImgSaveFrame( fid, outfile );

return;
} /* end of IMG$SAVE_FRAME */
#endif


void ImgSaveFrame ( 
     struct FCT	*fid
    ,char	*outfile
    )
{
char	    *fname	= 0;
long	     fnameLen;
struct DCB  *fileCtx;

if ( outfile != 0 )
    fname = outfile;
else
    fname = getenv( "IMG_SAVE_FRAME_FILE" );

fnameLen = strlen( fname );
fileCtx = ImgOpenFile(	 ImgK_ModeExport
			,ImgK_FtypeDDIF
			,fnameLen
			,fname
			,0
			,0 );

ImgExportFrame( fid, fileCtx, 0 );

ImgCloseFile( fileCtx, 0 );

return;
} /* end of ImgSaveFrame */


/******************************************************************************
**  IMG$STANDARDIZE_FRAME
**  ImgStandardizeFrame
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
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$STANDARDIZE_FRAME( srcfid, flags )
struct FCT  *srcfid;
long	     flags;
{
struct FCT  *retfid;

retfid = ImgStandardizeFrame( srcfid, flags );
return retfid;
} /* end of IMG$STANDARDIZE_FRAME */
#endif


/*******************************************************************************
** Portable entry point
*******************************************************************************/
struct FCT *ImgStandardizeFrame( srcfid, flags )
struct FCT  *srcfid;
long	     flags;
{
long		 local_flags;
long		 status;
struct FCT	*retfid;
struct ITMLST	*standardized_attrs;

/*
** Verify that the source frame is valid.  First, make sure the
** structure is OK and the attributes are consistent.  Once that's
** done, just verify whether the frame is already in native format.
** Signalling isn't necessary on this latter check, since this 
** function is supposed to convert any non-native aspect of the frame
** to native without complaint.
*/
local_flags = ImgM_NonstandardVerify ;
if ( VERIFY_ON_ )
    ImgVerifyFrame( srcfid, local_flags );

/*
** Make a special check for VAXcamera images, and fix them
** (by changing a few attributes) before doing the standard format check).
*/
status = _ImgVerifyVAXcameraImage( srcfid ); 
if ( status )
    _ImgFixVAXCameraImage( srcfid );

local_flags = ImgM_NoChf;
_ImgVerifyStandardFormat( srcfid, local_flags );

if ( (srcfid->FctL_Flags.FctV_NativeFormat) )
    /*
    ** The frame IS in native format, so copy it.
    */
    {
    if ( (flags&ImgM_InPlace) == 0 )
	{
	retfid = ImgCopyFrame( srcfid, 0 );
	if ( (flags&ImgM_AutoDeallocate) != 0 )
	    ImgDeallocateFrame( srcfid );
	}
    else
	retfid = srcfid;
    }
else
    {
    /*
    ** The frame is not in native format, so standardize it.
    */
    standardized_attrs = _ImgGetStandardizedAttrlst( srcfid );
    retfid = ImgConvertFrame( srcfid, standardized_attrs, flags );
    }

if ( VERIFY_ON_ )
    ImgVerifyFrame( retfid, 0 );

return retfid;
} /* end of ImgStandardizeFrame */


/******************************************************************************
**  IMG$VERIFY_FRAME
**  ImgVerifyFrame
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
#if defined(__VMS) || defined(VMS)
void IMG$VERIFY_FRAME( srcfid, flags )
struct FCT	*srcfid;
unsigned long	 flags;
{
struct FCT  *retfid;

ImgVerifyFrame( srcfid, flags );
return;
} /* end of IMG$VERIFY_FRAME */
#endif


/*******************************************************************************
** Portable entry point
*******************************************************************************/
void ImgVerifyFrame( srcfid, flags )
struct FCT	*srcfid;
unsigned long	 flags;
{
struct FCT  *retfid;

if ( srcfid->FctR_Blkhd.BhdB_Type != ImgK_BlktypFct )
    ChfStop( 1, ImgX_INVFRMID );

_ImgVerifyStructure( srcfid );

if ( (flags & ImgM_NoAttrVerify) == 0 )
    _ImgVerifyAttributes( srcfid, flags );

if ( (flags & ImgM_NoDataPlaneVerify) == 0 )
    _ImgVerifyDataPlanes( srcfid );

/*
** Unless flagged as non-standard, verify this as a native format frame.
*/
if ( (flags & ImgM_NonstandardVerify) == 0 )
    _ImgVerifyStandardFormat( srcfid, flags );

return;
} /* end of ImgVerifyFrame */


/******************************************************************************
**  Attach_data_planes()
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
static void Attach_data_planes( fid )
struct FCT *fid;
{
char	*data_plane;
long	 arsize;
long	 compression_type;
long	 ice_cnt;
long	 ice_idx	= 0;
long	 idu_cnt;
long	 idu_idx = 0;

struct UDP  udp;

_ImgGet( fid, Img_Icecnt, &ice_cnt, sizeof(long), 0, 0 );

for ( ; ice_idx < ice_cnt; ice_idx++ )
    {
    _ImgGet( fid, Img_IduCnt, &idu_cnt, sizeof(long), 0, 0 );
    for ( ; idu_idx < idu_cnt; idu_idx++ )
	{
	_ImgGet( fid, Img_CompressionType, &compression_type, sizeof(long),
	    0, idu_idx );
	if ( compression_type == ImgK_PcmCompression )
	    {
	    /*
	    ** From the image data unit UDP, calculate the array size
	    ** in bytes, allocate a data plane, and attach it to the frame.
	    */
	    _ImgGet( fid, Img_Udp, &udp, sizeof(struct UDP), 0, idu_idx );
	    arsize = (((udp.UdpL_ScnStride*udp.UdpL_ScnCnt)+udp.UdpL_Pos)+7)/8;
	    data_plane = ImgAllocateDataPlane( arsize, 0 );
	    ImgAttachDataPlane( fid, data_plane, idu_idx );
	    }
	} /* end of inner for loop */
    } /* end outer for loop */

return;
} /* end of Attach_data_planes */


/******************************************************************************
**  Setup_cvt_alignment()
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
static struct ITMLST *Setup_cvt_alignment( fid, src_itmlst, align_attrs )
struct FCT *fid;
struct ITMLST	    *src_itmlst;
struct ALIGN_ATTRS  *align_attrs;
{
long		     dp_cnt;
long		     element_count;
long		     found;
long		     index;
long		     occurance;
long		     pixels_per_line;
long		     tmp_index;
long		     tmp_item;
struct ALIGN_ATTRS  *local_align_attrs	= align_attrs;
struct ITMLST	    *local_ret_itmlst;
struct ITMLST	    *ret_itmlst;

/*
** Allocate the return itemlist to contain a specification for 
** Img_DataOffset, Img_PixelStride, Img_PlaneBitsPerPixel, & 
** Img_ScanlineStride for each data plane, plus a zero filled 
** terminator item.
*/
_ImgGet( fid, Img_PlanesPerPixel, &dp_cnt, LONGSIZE, 0, 0 );
element_count = (dp_cnt * 4) + 1;
ret_itmlst = _ImgCreateItmlst( element_count );

/*
** Initialize the alignment attribute values with the values in the 
** source frame, and then overwrite them with those that are specified 
** in the itemlist.
*/
for ( index = 0; index < dp_cnt; ++index )
    {
    _ImgGet( fid, Img_DataOffset, &(align_attrs[index].data_offset), 
		LONGSIZE, 0, index );
    _ImgGet( fid, Img_PixelStride, &(align_attrs[index].pixel_stride), 
		LONGSIZE, 0, index );
    _ImgGet( fid, Img_PlaneBitsPerPixel, 
		&(align_attrs[index].plane_bits_per_pixel), 
		LONGSIZE, 0, index );
    _ImgGet( fid, Img_ScanlineStride, &(align_attrs[index].scanline_stride), 
		LONGSIZE, 0, index );
    }

occurance = 1;
do
    {
    found = _ImgExtractItmlstItem(  src_itmlst,
				    Img_DataOffset,
				    &tmp_item,
				    0, 0, 
				    &tmp_index,
				    occurance );
    /*
    ** Store the data offset value from the source itemlist if
    ** the item was found and it's index is in range.  This means
    ** that out of range items are ignored.
    */
    if ( found && (tmp_index < dp_cnt) )
	align_attrs[tmp_index].data_offset = tmp_item;
    ++occurance;
    }
while ( found );

occurance = 1;
do
    {
    found = _ImgExtractItmlstItem(  src_itmlst,
				    Img_PixelAlignment,
				    &tmp_item,
				    0, 0, 
				    &tmp_index,
				    occurance );
    /*
    ** Align and store the pixel stride value from the source itemlist 
    ** if the item was found and it's index is in range.  This means
    ** that out of range items are ignored.
    */
    if ( found && (tmp_index < dp_cnt) )
	{
	align_attrs[tmp_index].plane_bits_per_pixel = 
	    ((align_attrs[tmp_index].plane_bits_per_pixel + tmp_item - 1)
		/ tmp_item) * tmp_item;

	align_attrs[tmp_index].pixel_stride = 
				align_attrs[tmp_index].plane_bits_per_pixel;

	}
    ++occurance;
    }
while ( found );

occurance = 1;
do
    {
    found = _ImgExtractItmlstItem(  src_itmlst,
				    Img_ScanlineAlignment,
				    &tmp_item,
				    0, 0, 
				    &tmp_index,
				    occurance );
    /*
    ** Calculate and align the scanline stride value based on the
    ** pixel count and pixel stride in conjunction with the scanline
    ** alignment value.  Of course, do this only if scanline alignment
    ** was specified by a data plane index that is within range.
    */
    if ( found && (tmp_index < dp_cnt) )
	{
	_ImgGet( fid, Img_PixelsPerLine, &pixels_per_line, 
		    LONGSIZE, 0, tmp_index );
	align_attrs[tmp_index].scanline_stride = 
	    ( ((align_attrs[tmp_index].pixel_stride * pixels_per_line) +
	       (tmp_item - 1)) / tmp_item ) * tmp_item;
	}
    ++occurance;
    }
while ( found );

/*
** Initialize the returned item list.
*/
local_ret_itmlst = ret_itmlst;
for ( index = 0; index < dp_cnt; ++index )
    {
    long    lstidx;

    lstidx = index * dp_cnt;
    local_ret_itmlst[lstidx].ItmL_Code	    = Img_DataOffset;
    local_ret_itmlst[lstidx].ItmL_Length    = LONGSIZE;
    local_ret_itmlst[lstidx].ItmA_Buffer    = (char *) 
      &(align_attrs[index].data_offset);
    local_ret_itmlst[lstidx].ItmA_Retlen    = 0;
    local_ret_itmlst[lstidx].ItmL_Index	    = index;

    local_ret_itmlst[lstidx+1].ItmL_Code    = Img_PlaneBitsPerPixel;
    local_ret_itmlst[lstidx+1].ItmL_Length  = LONGSIZE;
    local_ret_itmlst[lstidx+1].ItmA_Buffer  = (char *)
      &(align_attrs[index].plane_bits_per_pixel);
    local_ret_itmlst[lstidx+1].ItmA_Retlen  = 0;
    local_ret_itmlst[lstidx+1].ItmL_Index   = index;

    local_ret_itmlst[lstidx+2].ItmL_Code    = Img_PixelStride;
    local_ret_itmlst[lstidx+2].ItmL_Length  = LONGSIZE;
    local_ret_itmlst[lstidx+2].ItmA_Buffer  = (char *) 
      &(align_attrs[index].pixel_stride);
    local_ret_itmlst[lstidx+2].ItmA_Retlen  = 0;
    local_ret_itmlst[lstidx+2].ItmL_Index   = index;

    local_ret_itmlst[lstidx+3].ItmL_Code    = Img_ScanlineStride;
    local_ret_itmlst[lstidx+3].ItmL_Length  = LONGSIZE;
    local_ret_itmlst[lstidx+3].ItmA_Buffer  = (char *) 
      &(align_attrs[index].scanline_stride);
    local_ret_itmlst[lstidx+3].ItmA_Retlen  = 0;
    local_ret_itmlst[lstidx+3].ItmL_Index   = index;
    }

return ret_itmlst;
} /* end of Setup_cvt_alignment */


static void ImgDumpCodingAttrs( fid, fp )
struct FCT	*fid;
FILE		*fp;
{
unsigned long	dp_cnt;
unsigned long	dp_index;
unsigned long	index;

static ITEM_CODE   item_codes1[] = {
			 { Img_PixelsPerLine	    ,"Img_PixelsPerLine" }
			,{ Img_NumberOfLines	    ,"Img_NumberOfLines" }
			,{ Img_CompressionType	    ,"Img_CompressionType" }
			,{ Img_DataOffset	    ,"Img_DataOffset" }
			,{ Img_PixelStride	    ,"Img_PixelStride" }
			,{ Img_ScanlineStride	    ,"Img_ScanlineStride" }
			,{ Img_BitOrder		    ,"Img_BitOrder" }
			,{ Img_PlaneBitsPerPixel    ,"Img_PlaneBitsPerPixel" }
			,{ Img_ByteUnit		    ,"Img_ByteUnit" }
			,{ Img_ByteOrder	    ,"Img_ByteOrder" }
			,{ Img_DataType		    ,"Img_DataType" }
			,{ Img_DataPlaneBase	    ,"Img_DataPlaneBase" }
			,{ Img_DataPlaneSize	    ,"Img_DataPlaneSize" }
			,{ Img_DataPlaneBitSize	    ,"Img_DataPlaneBitSize" }
			,{ Img_PixelAlignment	    ,"Img_PixelAlignment" }
			,{ Img_ScanlineAlignment    ,"Img_ScanlineAlignment" }
			,{ Img_VirtualArsize	    ,"Img_VirtualArsize" }
			,{ Img_DType		    ,"Img_DType" }
			,{0,0} };

fprintf( fp, "\n" );
fprintf( fp, "*** Coding Attributes ***\n");

/*
** Get and print all the attributes by itemcode ...
*/
ImgGet( fid, Img_PlanesPerPixel, &dp_cnt, sizeof(dp_cnt), 0, 0 );
for ( dp_index = 0; dp_index < dp_cnt; ++dp_index )
    {
    for ( index = 0; item_codes1[ index ].item_code != 0; ++index )
	ImgGetAndPrintLongAttr( fp, fid, &item_codes1[ index ], dp_index );

    fprintf( fp, "\n" );
    }

return; 
} /* end of ImgDumpCodingAttrs */


static void ImgDumpCompSpaceAttrs( fid, fp )
struct FCT	*fid;
FILE		*fp;
{

unsigned long	    comp_cnt;
unsigned long	    comp_index;
unsigned long	    index;
static ITEM_CODE    item_codes2[] = {
		 { Img_CompSpaceOrg		,"Img_CompSpaceOrg" }
		,{ Img_PlanesPerPixel		,"Img_PlanesPerPixel" }
		,{ Img_PlaneSignif		,"Img_PlaneSignif" }
		,{ Img_NumberOfComp		,"Img_NumberOfComp" }
		,{ Img_BPCListCnt		,"Img_BPCListCnt" }
		,{ Img_TotalBitsPerPixel	,"Img_TotalBitsPerPixel" }
		,{ Img_TotalQuantBitsPerPixel	,"Img_TotalQuantBitsPerPixel" }
		,{ 0,0 } };

static ITEM_CODE    item_codes3[] = {
		 { Img_BitsPerComp		,"Img_BitsPerComp" }
		,{ Img_QuantLevelsPerComp	,"Img_QuantLevelsPerComp" }
		,{ Img_QuantBitsPerComp		,"Img_QuantBitsPerComp" }
		,{ 0,0 } };

fprintf( fp, "\n" );
fprintf( fp, "*** Component Space Attributes ***\n");

/*
** Get and print all the attributes by itemcode ...
*/
for ( index = 0; item_codes2[ index ].item_code != 0; ++index )
    ImgGetAndPrintLongAttr( fp, fid, &item_codes2[ index ], 0 );

ImgGet( fid, Img_NumberOfComp, &comp_cnt, sizeof(comp_cnt), 0, 0 );
fprintf( fp, "\n" );
for ( comp_index = 0; comp_index < comp_cnt; ++comp_index )
    {
    for ( index = 0; item_codes3[ index ].item_code != 0; ++index )
	ImgGetAndPrintLongAttr( fp, fid, &item_codes3[ index ], comp_index );

    fprintf( fp, "\n" );
    }

return; 
} /* end of ImgDumpCompSpaceAttrs */


static void ImgDumpFrameParams( fid, fp )
struct FCT	*fid;
FILE		*fp;
{
float		    x_size_float;
float		    y_size_float;
unsigned long	    index;
unsigned long	    x_size_long;
unsigned long	    y_size_long;

static ITEM_CODE    item_codes4[] = {
			 { Img_FrmBoxLLXC	    ,"Img_FrmBoxLLXC" }
			,{ Img_FrmBoxLLX	    ,"Img_FrmBoxLLX" }
			,{ Img_FrmBoxLLYC	    ,"Img_FrmBoxLLYC" }
			,{ Img_FrmBoxLLY	    ,"Img_FrmBoxLLY" }
			,{ Img_FrmBoxURXC	    ,"Img_FrmBoxURXC" }
			,{ Img_FrmBoxURX	    ,"Img_FrmBoxURX" }
			,{ Img_FrmBoxURYC	    ,"Img_FrmBoxURYC" }
			,{ Img_FrmBoxURY	    ,"Img_FrmBoxURY" }
			,{ Img_FrmPositionC	    ,"Img_FrmPositionC" }
			,{ Img_FrmFxdPositionXC	    ,"Img_FrmFxdPositionXC" }
			,{ Img_FrmFxdPositionX	    ,"Img_FrmFxdPositionX" }
			,{ Img_FrmFxdPositionYC	    ,"Img_FrmFxdPositionYC" }
			,{ Img_FrmFxdPositionY	    ,"Img_FrmFxdPositionY" }
			,{ 0, 0 } };

fprintf( fp, "\n" );
fprintf( fp, "*** Frame Parameters ***\n");

/*
** Get and print all the attributes by itemcode ...
*/
for ( index = 0; item_codes4[ index ].item_code != 0; ++index )
    ImgGetAndPrintLongAttr( fp, fid, &item_codes4[ index ], 0 );

/*
** Get and print frame size information according to unit type
*/
ImgGetFrameSize( fid, &x_size_long, &y_size_long, ImgK_Bmus );
fprintf( fp, "\n" );
fprintf( fp, "X size (BMUs): %d\n", x_size_long );
fprintf( fp, "Y size (BMUs): %d\n", y_size_long );

ImgGetFrameSize( fid, &x_size_float, &y_size_float, ImgK_Centimeters );
fprintf( fp, "\n" );
fprintf( fp, "X size (Centimeters): %f\n", x_size_float );
fprintf( fp, "Y size (Centimeters): %f\n", y_size_float );

ImgGetFrameSize( fid, &x_size_float, &y_size_float, ImgK_Inches );
fprintf( fp, "\n" );
fprintf( fp, "X size (Inches): %f\n", x_size_float );
fprintf( fp, "Y size (Inches): %f\n", y_size_float );

return; 
} /* end of ImgDumpFrameParams */


static void ImgDumpMiscAttrs( fid, fp )
struct FCT	*fid;
FILE		*fp;
{
char		*user_label_buf;
unsigned long	 index;
unsigned long	 user_label_buflen;
unsigned long	 user_label_cnt;
unsigned long	 user_label_len;
static ITEM_CODE    item_codes5[] = {
				 { Img_UserField	,"Img_UserField" }
				,{ Img_ImageDataClass	,"Img_ImageDataClass" }
				,{ Img_PageBreak	,"Img_PageBreak" }
				,{ Img_RectRoiInfo	,"Img_RectRoiInfo" }
				,{ Img_StandardFormat	,"Img_StandardFormat" }
				,{ Img_UserLabelCnt	,"Img_UserLabelCnt" }
				,{ 0, 0 } };

fprintf( fp, "\n" );
fprintf( fp, "*** Miscellaneous Frame Attributes ***\n");

/*
** Get and print all the attributes by itemcode ...
*/
for ( index = 0; item_codes5[ index ].item_code != 0; ++index )
    ImgGetAndPrintLongAttr( fp, fid, &item_codes5[ index ], 0 );

/*
** Print User Label info ...
*/
ImgGet( fid, Img_UserLabelCnt, &user_label_cnt, sizeof(user_label_cnt), 0, 0 );
if ( user_label_cnt > 0 )
    for ( index = 0; index < user_label_cnt; ++index )
        {
        ImgGet( fid, Img_UserLabelLen, &user_label_len, sizeof(user_label_len),
                0, 0 );
        user_label_buflen = user_label_len + 1;
        user_label_buf = _ImgCalloc( 1, user_label_buflen );
        ImgGet( fid, Img_UserLabel, user_label_buf, user_label_buflen, 0,
                    index );

        fprintf( fp, "Img_UserLabel[%2d]: %s\n", index, user_label_buf );

        _ImgFree( user_label_buf );
        }

return; 
} /* end of ImgDumpMiscAttrs */


static void ImgDumpPresentAttrs( fid, fp )
struct FCT	*fid;
FILE		*fp;
{
unsigned long	index;
static ITEM_CODE    item_codes6[] = {
				 { Img_PixelPath	,"Img_PixelPath" }
				,{ Img_LineProgression	,"Img_LineProgression" }
				,{ Img_PpPixelDist	,"Img_PpPixelDist" }
				,{ Img_LpPixelDist	,"Img_LpPixelDist" }
				,{ Img_BrtPolarity	,"Img_BrtPolarity" }
				,{ Img_GridType		,"Img_GridType" }
				,{ Img_SpectralMapping	,"Img_SpectralMapping" }
				,{ Img_PixelGroupSize	,"Img_PixelGroupSize" }
				,{ Img_PixelGroupOrder	,"Img_PixelGroupOrder" }
				,{ 0, 0 } };
fprintf( fp, "\n" );
fprintf( fp, "*** Presentation Attributes ***\n");

/*
** Get and print all the attributes by itemcode ...
*/
for ( index = 0; item_codes6[ index ].item_code != 0; ++index )
    ImgGetAndPrintLongAttr( fp, fid, &item_codes6[ index ], 0 );

return; 
} /* end of ImgDumpPresentAttrs */


static void ImgDumpStructure( fid, fp )
struct FCT	*fid;
FILE		*fp;
{

return; 
} /* end of ImgDumpStructure */


static void ImgGetAndPrintLongAttr( fp, fid, item_code, index )
FILE		*fp;
struct FCT	*fid;
ITEM_CODE	*item_code;
unsigned long	 index;
{
unsigned long	retlen;
unsigned long	value;

/*
** Get the item_code value from the frame and print it.
*/
ImgGet( fid, item_code->item_code, &value, sizeof(value), &retlen, index );
fprintf( fp, "%32s (index %2d): %10d\n", item_code->item_code_str, index, value );

return;
} /* end of ImgGetAndPrintLongAttr */


/******************************************************************************
**  _ImgFixVAXCameraImage
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
static void _ImgFixVAXCameraImage( fid )
struct FCT	*fid;
{
int	plane_bits_per_pixel	= 8;
int	quant_levels_per_comp	= 128;

_ImgPut( fid, Img_PlaneBitsPerPixel, &plane_bits_per_pixel, 
	    sizeof(plane_bits_per_pixel), 0 );
_ImgPut( fid, Img_QuantLevelsPerComp, &quant_levels_per_comp,
	    sizeof( quant_levels_per_comp), 0 );

return;
} /* end of _ImgFixVAXCameraImage */


/******************************************************************************
**  _ImgVerifyVAXcameraImage
**
**  FUNCTIONAL DESCRIPTION:
**
**	Check to see if this is a VAXcamera image.
**
**  FORMAL PARAMETERS:
**
**	fid	frame-id of frame to verify.
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
**	status	    1 if the image is a VAXcamera image
**		    0 if it isn't
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
static int _ImgVerifyVAXcameraImage( fid )
struct FCT	*fid;
{
int	data_class;
int	bits_per_comp;
int	comp_space_org;
int	compression_type;
int	plane_bits_per_pixel;
int	pixel_stride;
int	quant_levels_per_comp;
int	status;

_ImgGet( fid, Img_ImageDataClass,	&data_class,		
	    sizeof(data_class),		    0,0 );
_ImgGet( fid, Img_BitsPerComp,		&bits_per_comp,		
	    sizeof(bits_per_comp),	    0,0 );
_ImgGet( fid, Img_CompSpaceOrg,		&comp_space_org,	
	    sizeof(comp_space_org),	    0,0 );
_ImgGet( fid, Img_CompressionType,	&compression_type,
	    sizeof(compression_type),	    0,0 );
_ImgGet( fid, Img_PlaneBitsPerPixel,	&plane_bits_per_pixel,	
	    sizeof(plane_bits_per_pixel),   0,0 );
_ImgGet( fid, Img_PixelStride,		&pixel_stride,		
	    sizeof(pixel_stride),	    0,0 );
_ImgGet( fid, Img_QuantLevelsPerComp,	&quant_levels_per_comp, 
	    sizeof(quant_levels_per_comp),  0,0 );

if (	data_class == ImgK_ClassGreyscale	    &&
	bits_per_comp == 8			    &&
	comp_space_org != ImgK_BitIntrlvdByPlane    &&
	compression_type == ImgK_PcmCompression	    &&
	plane_bits_per_pixel == 7		    &&
	pixel_stride == 8			    &&
	quant_levels_per_comp == 256 )
    /*
    ** This IS a VAXcamera image, as produced by DAS V2.0 (or possibly
    ** later).
    */
    status = 1;
else
    /*
    ** It isn't the kind of image we're looking for.
    */
    status = 0;

return status;
} /* end of _ImgVerifyVAXcameraImage */
