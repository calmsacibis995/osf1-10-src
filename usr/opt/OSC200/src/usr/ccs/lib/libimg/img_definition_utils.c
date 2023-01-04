
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

/******************************************************************************
**  IMG_DEFINITION_UTILS
**
**  FACILITY:
**
**      Image Services Library
**
**  ABSTRACT:
**
**	This modules contains utility functions which create and manage
**	image frame definition objects.  A frame definition object has
**	all of the attribute characteristics of a frame object, but has no
**	actual image data associated with it.  A frame definition is 
**	therefore more like a 'super attribute' of a frame.
**
**  ENVIRONMENT:
**
**      VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHORS:
**
**	Mark Sornson
**
**  CREATION DATE:     
**
**	29-SEP-1989
**
**  MODIFICATION HISTORY:
**
************************************************************************/

/*
**  Include files 
*/
#include    <string.h>

#include    <img/ChfDef.h>
#include    <img/ImgDef.h>
#include    <ImgDefP.h>
#include    <ImgMacros.h>
#ifndef NODAS_PROTO
#include <imgprot.h>	    /* Img prototypes */
#endif

#if defined(__VMS) || defined(VMS)
#include <cda$msg.h>
#include <cda$ptp.h>
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
**	Global Routines (VMS Veneer)
*/
#if defined(__VMS) || defined(VMS)
struct FCT  *IMG$ADJUST_FRAME_DEF();
struct FCT  *IMG$CREATE_FRAME_DEF();
void	     IMG$DELETE_FRAME_DEF();
struct FCT  *IMG$EXTRACT_FRAME_DEF();
void	     IMG$VERIFY_FRAME_DEF();
#endif

/*
**	Global Routines (Portable Entry Points)
*/
#ifdef NODAS_PROTO
struct FCT  *ImgAdjustFrameDef();
struct FCT  *ImgCreateFrameDef();
void	     ImgDeleteFrameDef();
struct FCT  *ImgExtractFrameDef();
void	     ImgVerifyFrameDef();

/*
**	Global, low level utility functions
*/
struct FCT  *_ImgCopyFrameDef();
CDAagghandle	     _ImgCloneIduAggr();
CDAagghandle	     _ImgCloneImageAggr();
CDAagghandle	     _ImgCloneSegmentAggr();
#endif

/*
**	Module local routines
*/
#ifdef NODAS_PROTO
static struct FCT   *Create_frame_definition();
static void	     Set_coding_attrs();
static void	     Set_comp_space_attrs();
static void	     Set_frame_parameters();
static void	     Set_misc_attrs();
static void	     Set_presentation_attrs();
#else
PROTO(static struct FCT *Create_frame_definition, (struct FAT */*fat*/));
PROTO(static void Set_coding_attrs, (struct FCT */*fdf*/, struct IDU */*idu*/, long /*idu_idx*/));
PROTO(static void Set_comp_space_attrs, (struct FCT */*fdf*/, struct FAT */*fat*/));
PROTO(static void Set_frame_parameters, (struct FCT */*fdf*/, struct FAT */*fat*/));
PROTO(static void Set_misc_attrs, (struct FCT */*fdf*/, struct FAT */*fat*/));
PROTO(static void Set_presentation_attrs, (struct FCT */*fdf*/, struct FAT */*fat*/));
#endif


/*
**  MACRO definitions
**
**	none
*/

/*
**  Equated Symbols
*/
#define LONGWORD_SIZE sizeof(long)
/*
**  External References
*/
#ifdef NODAS_PROTO
void	    ChfSignal();
void	    ChfStop();

void	    ImgVerifyFrame();

struct FAT  *_ImgAdjustFat();
struct FAT  *_ImgCreateFat();
CDArootagghandle	     _ImgCreateRootAggregate();
void	     _ImgDeleteFat();
struct FAT  *_ImgExtractFat();
struct FCT  *_ImgFrameAlloc();
struct FCT  *_ImgFrameAppendIce();
struct FCT  *_ImgFrameAppendIdu();
void	     _ImgFrameDealloc();
void	     _ImgFree();
long	     _ImgGet();
long	     _ImgGetVerifyStatus();
char	    *_ImgMalloc();
struct FCT  *_ImgPut();
void	     _ImgVerifyAttributes();
void	     _ImgVerifyNativeFormat();
void	     _ImgVerifyStructure();
#endif

/*
** External symbol definitions (status codes) currently commented out
*/
#include <img/ImgStatusCodes.h>         /* ISL Status Codes             */
    
/*
**  Local Storage
**
**	none
*/


/************************************************************************
**  IMG$ADJUST_FRAME_DEF
**  ImgAdjustFrameDef - Set attributes in frame structure
**
**  FUNCTIONAL DESCRIPTION:
**
**      Set attributes in an existing frame structure from a user specified
**	item list.
**
**  FORMAL PARAMETERS:
**
**      fid		Frame def, a pointer to a FCT block
**	itmlst		Attributes for new frame definition
**	flags		Flags:
**
**			    ImgM_VerifyOn
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
**      retfid		returns the input frame id value
**
**  SIGNAL CODES:
**
**	ImgX_INVARGCNT	Invalid number of arguments
**
**  SIDE EFFECTS:
**
**	none
**
************************************************************************/
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$ADJUST_FRAME_DEF( fid, itmlst, flags )
struct FCT	*fid;
struct ITMLST	*itmlst[];
unsigned long	 flags;
{

return (ImgAdjustFrameDef( fid, itmlst, flags ));
} /* end of IMG$ADJUST_FRAME_DEF */
#endif


/*******************************************************************************
** Portable entry point
*******************************************************************************/
struct FCT *ImgAdjustFrameDef( fdf, itmlst, flags )
struct FCT	*fdf;
struct ITMLST	*itmlst;
unsigned long	 flags;
{
struct FAT  *adjusted_fat;
struct FAT  *fat;
struct FCT  *retfdf;

if ( itmlst != 0 )
    {
    /*
    ** Extract FAT from existing frame or defintion, and create an
    ** adjusted FAT from it.
    */
    fat = _ImgExtractFat( fdf );
    adjusted_fat = _ImgAdjustFat( fat, itmlst, 0 );

    /*
    ** Create a new frame definition from the adjusted FAT
    */
    retfdf = Create_frame_definition( adjusted_fat );

    /*
    ** Delete the FATs and return the new frame adjusted frame definition.
    */
    _ImgDeleteFat( fat );
    _ImgDeleteFat( adjusted_fat );
    }
else
    {
    /*
    ** No adjustment necessary, therefore simply make a copy of 
    ** the existing frame definition.
    */
    retfdf = _ImgCopyFrameDef( fdf, 0 );
    }

if ( VERIFY_ON_ )
    ImgVerifyFrameDef( retfdf, 
			(ImgM_NonstandardVerify | ImgM_NoDataPlaneVerify) );

return retfdf;
} /* end of ImgAdjustFrameDef */


/******************************************************************************
**  IMG$CREATE_FRAME_DEF
**  ImgCreateFrameDef - Create an image frame structure
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine creates a frame structure from data and attributes 
**	specified by item list parameters.
**
**  FORMAL PARAMETERS:
**
**      itmlst - address of an item list structure which specifies how the
**		 frame is to be built.
**
**	SPECTRAL_TYPE
**		 Symbolic type code representing type of frame to create.
**		 Passed by value.  OPTIONAL.
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
**      frame_defintion
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
struct FCT *IMG$CREATE_FRAME_DEF( itmlst, data_class )
struct ITMLST	*itmlst;
long		 data_class;
{

return (ImgCreateFrameDef( itmlst, data_class ));
} /* end of IMG$CREATE_FRAME_DEF*/
#endif



/*******************************************************************************
** Portable entry point
*******************************************************************************/
struct FCT *ImgCreateFrameDef( image_data_class, itmlst )
long		 image_data_class;
struct ITMLST	*itmlst;
{
long	     argcnt;
struct FCT  *fdf;
struct FAT  *fat;

/*
** Verify that the data class is valid.
*/
if ( image_data_class == 0 )
    ChfStop( 1, ImgX_INVDATCLA );

/*
** Create a Frame Itemcode descriptor (FAT) for the frame.
*/
fat = _ImgCreateFat( image_data_class, itmlst );

/*
** Create a frame definition block from the FAT.
*/
fdf = Create_frame_definition( fat );

/*
** Now that the frame definition block has been created, the
** FAT can be deleted.
*/
_ImgDeleteFat( fat );

return fdf;
} /* end of ImgCreateFrameDef*/


/************************************************************************
**  IMG$DELETE_FRAME_DEF
**  ImgDeleteFrameDef
**
**  FUNCTIONAL DESCRIPTION:
**
**      Deletes a list of FDEFS
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
void IMG$DELETE_FRAME_DEF( fid )
struct FCT  *fid;

{

ImgDeleteFrameDef( fid );

return;
} /* end of IMG$DELETE_FRAME_DEF*/
#endif


/*******************************************************************************
** Portable entry point
*******************************************************************************/
void ImgDeleteFrameDef( fdf )
struct FCT  *fdf;
{

if ( !VERIFY_OFF_ )
    _ImgVerifyStructure( fdf );

_ImgFrameDealloc( fdf );

return;
} /* end of ImgDeleteFrameDef*/


/******************************************************************************
**  IMG$EXTRACT_FRAME_DEF
**  ImgExtractFrameDef
**
**  FUNCTIONAL DESCRIPTION:
**
**	Extract a frame definition from a frame..
**
**  FORMAL PARAMETERS:
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
**	fid		= Frame-id of frame passed in.
**
**  SIGNAL CODES:
**
**	ImgX_INVARGCNT	    Invalid argument count
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$EXTRACT_FRAME_DEF( fid, flags )
struct FCT	*fid;
unsigned long	 flags;
{

return ( ImgExtractFrameDef( fid, flags ) );
} /* end of IMG$EXTRACT_FRAME_DEF*/
#endif


/*******************************************************************************
** Portable entry point
*******************************************************************************/
struct FCT *ImgExtractFrameDef( fid, flags )
struct FCT	*fid;
unsigned long	 flags;
{
long	     local_flags;
struct FCT  *fdf;
struct FAT  *fat;

/*
** Verify that the structure of the frame is OK.
*/
local_flags = ImgM_NonstandardVerify;
if ( VERIFY_ON_ )
    ImgVerifyFrame( fid, local_flags );

fat = _ImgExtractFat( fid );

fdf = Create_frame_definition( fat );

_ImgDeleteFat( fat );

return fdf;
} /* end of ImgExtractFrameDef*/


/******************************************************************************
**  IMG$VERIFY_FRAME_DEF
**  ImgVerifyFrameDef
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
void IMG$VERIFY_FRAME_DEF( frame_def, flags )
struct FCT	*frame_def;
unsigned long	 flags;
{

ImgVerifyFrameDef( frame_def, flags );
return;
} /* end of IMG$VERIFY_FRAME_DEF */
#endif


/*******************************************************************************
** Portable entry point
*******************************************************************************/
void ImgVerifyFrameDef( frame_def, flags )
struct FCT  *frame_def;
{

if ( frame_def->FctR_Blkhd.BhdB_Type != ImgK_BlktypFdf )
    ChfStop( 1, ImgX_INVFRMDEF );

_ImgVerifyStructure( frame_def );
_ImgVerifyAttributes( frame_def, flags );

return;
} /* end of ImgVerifyFrameDef */


/******************************************************************************
**  _ImgCopyFrameDef
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
struct FCT *_ImgCopyFrameDef( srcfdf, flags )
struct FCT	*srcfdf;
unsigned long	 flags;
{
long	data_class;
long	ice_cnt;
CDAagghandle	ret_idu_aggr;
CDAagghandle	ret_img_aggr;
CDArootagghandle	ret_root_aggr;
CDAagghandle	ret_seg_aggr;
CDAagghandle	src_idu_aggr;
CDAagghandle	src_img_aggr;
CDArootagghandle	src_root_aggr;
CDAagghandle	src_seg_aggr;
long	status;
long	zero		= 0;

struct FCT  *retfdf;

/*
** Make sure the source definition is 'structurally sound'.
*/
if ( VERIFY_ON_ )
    _ImgVerifyStructure( srcfdf );

/*
** Get root and image segment info from input frame def and use them
** to create a new root aggr for the new frame def.  
*/
_ImgGet( srcfdf, Img_RootAggr, &src_root_aggr, sizeof(src_root_aggr), 0, 0 );
_ImgGet( srcfdf, Img_SegAggr, &src_seg_aggr, sizeof(src_seg_aggr), 0, 0 );
ret_root_aggr	= _ImgCreateRootAggregate(0);
ret_seg_aggr	= _ImgCloneSegmentAggr( srcfdf, ret_root_aggr );
retfdf		= _ImgFrameAlloc( ret_root_aggr, ret_seg_aggr );

/*
** Duplicate the image data class.
*/
_ImgGet(srcfdf, Img_ImageDataClass, &data_class, sizeof(data_class), 0, 0);
_ImgPut(retfdf, Img_ImageDataClass, &data_class, sizeof(data_class), 0);

/*
** Duplicate the current ICE and its IDUs.
*/
_ImgGet( srcfdf, Img_ImgAggr, &src_img_aggr, sizeof(src_img_aggr), 0, 0 );
_ImgPut( retfdf, Img_Icecnt, &zero, sizeof(zero), 0 );
ret_img_aggr = _ImgCloneImageAggr( ret_root_aggr, src_root_aggr, src_img_aggr );
_ImgFrameAppendIce( retfdf, ret_img_aggr );

return retfdf;
} /* end of _ImgCopyFrameDef */


/******************************************************************************
**  _ImgCloneIduAggr
**
**  FUNCTIONAL DESCRIPTION:
**
**	Clone an existing IDU aggregate, copying all the attributes in
**	the original, but without copying the actual data.  This function
**	will create the new aggregate as well as fill it in.
**
**  FORMAL PARAMETERS:
**
**	ret_root_aggr	    Root aggregate handle to be used with the
**			    cloned IDU aggr.  Passed by value.
**	src_root_aggr	    Root aggregate handle associated with the
**			    IDU aggr being cloned.  Passed by value.
**	src_idu_aggr	    Handle of IDU aggr to be cloned.
**			    Passed by value.
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
**	ret_idu_aggr	    Handle of the IDU aggr that was created.
**			    Passed by value.
**
**  SIGNAL CODES:
**
**	CDA codes.  Otherwise none.
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
CDAagghandle _ImgCloneIduAggr( ret_root_aggr, src_root_aggr, src_idu_aggr )
CDArootagghandle ret_root_aggr;
CDArootagghandle src_root_aggr;
CDAagghandle src_idu_aggr;
{
char	*data_plane = 0;
CDAconstant      aggregate_item;
int	 data_detached	= FALSE;
long	 item;
long	*itemptr;
long	 length;
CDAagghandle	 ret_idu_aggr;
int	 status;

/*
** Locate the data plane if there is one and detach it.
*/
#if defined(NEW_CDA_SYMBOLS)
aggregate_item = DDIF_IDU_PLANE_DATA;
#else
aggregate_item = DDIF$_IDU_PLANE_DATA;
#endif
status = CDA_LOCATE_ITEM_(
	    &src_root_aggr,
	    &src_idu_aggr,
	    &aggregate_item,
	    (CDAaddress *)&data_plane,
	    &length,
	    0, 0 );
switch ( status )
    {
#if defined(NEW_CDA_SYMBOLS)
    case CDA_NORMAL:
#else
    case CDA$_NORMAL:
#endif
#if defined(NEW_CDA_SYMBOLS)
        aggregate_item = DDIF_IDU_PLANE_DATA;
#else
        aggregate_item = DDIF$_IDU_PLANE_DATA;
#endif
	status = CDA_DETACH_ITEM_(
		    &src_idu_aggr,
		    &aggregate_item );
	LOWBIT_TEST_( status );
	/*
	** Back the data plane address off by 4 bytes since
	** locate will return the address of the start of the
	** data field, not the address of the length field.
	*/
	data_plane -= LONGWORD_SIZE;
	data_detached = TRUE;
#if defined(NEW_CDA_SYMBOLS)
    case CDA_EMPTY:	
#else
    case CDA$_EMPTY:	
#endif
	break;
    default:
	ChfSignal( 1,  status );
    }

/*
** Copy the idu (without the data plane).  This will clone the idu aggr.
*/
status = CDA_COPY_AGGREGATE_(
	    &ret_root_aggr,
	    &src_idu_aggr,
	    &ret_idu_aggr
	    );
LOWBIT_TEST_( status );

/*
** Reattach the data plane to the original idu.
*/
if ( data_detached )
    {
#if defined(NEW_CDA_SYMBOLS)
    aggregate_item = DDIF_IDU_PLANE_DATA;
#else
    aggregate_item = DDIF$_IDU_PLANE_DATA;
#endif
    status = CDA_ATTACH_ITEM_(
		&src_idu_aggr,
		&aggregate_item,
		data_plane );
    LOWBIT_TEST_( status );
    }

return ret_idu_aggr;
} /* end of _ImgCloneIduAggr */


/******************************************************************************
**  _ImgCloneImageAggr
**
**  FUNCTIONAL DESCRIPTION:
**
**	Clone an existing DDIF$_IMG aggregate, copying all the DDIF$_IDU
**	aggregates except for the actual data in them.
**
**  FORMAL PARAMETERS:
**
**	ret_root_aggr	    Root aggregate handle to be used with the
**			    cloned IDU aggr.  Passed by value.
**	src_root_aggr	    Root aggregate handle associated with the
**			    IDU aggr being cloned.  Passed by value.
**	src_img_aggr	    Handle of IMG aggr to be cloned.
**			    Passed by value.
**
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
**	ret_img_aggr	    Handle of the IMG aggr that was created.
**			    Passed by value.
**
**  SIGNAL CODES:
**
**	CDA codes.  Otherwise none.
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
CDAagghandle _ImgCloneImageAggr( ret_root_aggr, src_root_aggr, src_img_aggr )
CDArootagghandle	ret_root_aggr;
CDArootagghandle	src_root_aggr;
CDAagghandle	src_img_aggr;
{
CDAconstant	 aggregate_item;
int	 aggregate_type;
CDAagghandle	 cur_idu_aggr	    = 0;
long	 idu_cnt	    = 0;
long	*item;
long	 item_size;
long	 length;
CDAagghandle	 ret_idu_aggr;
CDAagghandle	 ret_img_aggr	    = 0;
CDAagghandle	 next_idu_aggr	    = 0;
CDAagghandle	 prev_idu_aggr;
int	 status;

/*
** Create a new DDIF_IMG aggregate
*/
#if defined(NEW_CDA_SYMBOLS)
aggregate_type = DDIF_IMG;
#else
aggregate_type = DDIF$_IMG;
#endif
status = CDA_CREATE_AGGREGATE_(
	    &ret_root_aggr,
	    &aggregate_type,
	    &ret_img_aggr );
LOWBIT_TEST_( status );

/*
** Get the first IDU.  (Note that it is assumed that a first
** IDU will be found.  Otherwise, LIB$STOP will be called in
** the LOWBIT_TEST_ macro.)
*/
#if defined(NEW_CDA_SYMBOLS)
aggregate_item = DDIF_IMG_CONTENT;
#else
aggregate_item = DDIF$_IMG_CONTENT;
#endif
status = CDA_LOCATE_ITEM_(
	    &src_root_aggr,
	    &src_img_aggr,
	    &aggregate_item,
	    (CDAaddress *)&item,
	    &length,
	    0, 0 );
LOWBIT_TEST_( status );
next_idu_aggr = *(CDAagghandle *)item;
/*
** Loop through the old image aggregate, finding IDUs, creating
** duplicates, and copying the old into the new (except for the
** actual image data).
*/
do
    {
    /*
    ** Clone the current one.
    */
    cur_idu_aggr = next_idu_aggr;
    ret_idu_aggr = _ImgCloneIduAggr( 
			ret_root_aggr, 
			src_root_aggr, 
			cur_idu_aggr );
    /*
    ** Add the new IDU to the frame
    */
    if ( idu_cnt == 0 )
	{
#if defined(NEW_CDA_SYMBOLS)
        aggregate_item = DDIF_IMG_CONTENT;
#else
        aggregate_item = DDIF$_IMG_CONTENT;
#endif
        item_size = sizeof(ret_idu_aggr);
	status = CDA_STORE_ITEM_(
		    &ret_root_aggr,
		    &ret_img_aggr,
		    &aggregate_item,
		    &item_size,
		    &ret_idu_aggr,
		    0, 0 );
	LOWBIT_TEST_( status );
	prev_idu_aggr = ret_idu_aggr;
	}
    else
	{
	status = CDA_INSERT_AGGREGATE_(
		    &ret_idu_aggr,
		    &prev_idu_aggr );
	LOWBIT_TEST_( status );
	prev_idu_aggr = ret_idu_aggr;
	}

    /*
    ** Get the next one.
    */
    status = CDA_NEXT_AGGREGATE_(
		&cur_idu_aggr,
		&next_idu_aggr );
    }
#if defined(NEW_CDA_SYMBOLS)
while ( status == CDA_NORMAL );	    /* end do while */
#else
while ( status == CDA$_NORMAL );	    /* end do while */
#endif

#if defined(NEW_CDA_SYMBOLS)
if ( status != CDA_ENDOFSEQ )
#else
if ( status != CDA$_ENDOFSEQ )
#endif
    ChfStop( 1,  status );

return ret_img_aggr;
} /* end of _ImgCloneImageAggr */


/******************************************************************************
**  _ImgCloneSegmentAggr
**
**  FUNCTIONAL DESCRIPTION:
**
**	Copy a DDIF$_SEG aggregate for an image frame.  This will ensure that
**	the DDIF$_SEG aggregate that will be associated with an ISL frame
**	has been allocated by ISL memory mgt routines.
**
**  FORMAL PARAMETERS:
**
**	src_fid			Frame id of the source frame whose segment
**				aggregate is to be copied.
**
**	dst_root_aggr,		Handle of root aggregate that will own the 
**				copied segment aggregate.
**				By value.
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
**	dst_seg_aggr,		Handle of the segment aggregate copy.
**				By value.
**
**  SIGNAL CODES:
**
**	CDA codes.  Otherwise none.
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
CDAagghandle _ImgCloneSegmentAggr( src_fid, dst_root_aggr )
struct FCT	*src_fid;
CDArootagghandle	dst_root_aggr;
{
CDAconstant	 aggregate_item;
CDAagghandle	 dst_seg_aggr;
long	 ice_cnt;
CDAagghandle	 img_aggr;
long	 index;
long	*item;
long	 item_size;
long	 length;
CDAagghandle	*saved_img_aggrs;
CDArootagghandle	 src_root_aggr;
CDAagghandle	 src_seg_aggr;
int	 status;

/*
** Detach all image aggregates from the segment aggregate before
** copying it.  Test to see if there are any by checking the frame ICE count.
*/
_ImgGet( src_fid, Img_Icecnt, &ice_cnt, sizeof(ice_cnt), 0, 0 );
if ( ice_cnt != 0 )
    {
    /*
    ** Allocate some memory to store the DDIF$_IMG aggregate handles.
    */
    saved_img_aggrs = (CDAagghandle *) 
                            _ImgMalloc( ice_cnt * sizeof (CDAagghandle));

    /*
    ** Get the first image aggregate from the segment aggr.
    */
    _ImgGet( src_fid, Img_RootAggr, &src_root_aggr, sizeof(src_root_aggr), 0, 0 );
    _ImgGet( src_fid, Img_SegAggr, &src_seg_aggr, sizeof(src_seg_aggr), 0, 0 );
#if defined(NEW_CDA_SYMBOLS)
    aggregate_item = DDIF_SEG_CONTENT;
#else
    aggregate_item = DDIF$_SEG_CONTENT;
#endif
    status = CDA_LOCATE_ITEM_(
		&src_root_aggr,
		&src_seg_aggr,
		&aggregate_item,
		(CDAaddress *)&item,
		&length,
		0,0 );
    LOWBIT_TEST_( status );
    img_aggr = *(CDAagghandle *)item;

    /*
    ** Loop, detaching image aggregates from the segment aggregate,
    ** storing the handles in the saved_img_aggrs array.
    */
    index = 0;
    do
	{
	/*
	** Remove an image aggr from the segment aggr and store it.
	*/
	status = CDA_REMOVE_AGGREGATE_( &img_aggr );
	LOWBIT_TEST_( status );
	saved_img_aggrs[ index ] = img_aggr;
	++index;

	/*
	** See if there's another image aggr in the segment.
	*/
#if defined(NEW_CDA_SYMBOLS)
        aggregate_item = DDIF_SEG_CONTENT;
#else
        aggregate_item = DDIF$_SEG_CONTENT;
#endif
	status = CDA_LOCATE_ITEM_(
		    &src_root_aggr,
		    &src_seg_aggr,
		    &aggregate_item,
		    (CDAaddress *)&item,
		    &length,
		    0,0 );
	switch( status )
	    {
#if defined(NEW_CDA_SYMBOLS)
	    case CDA_NORMAL:	img_aggr = *(CDAagghandle *)item;
	    case CDA_EMPTY:	break;
#else
	    case CDA$_NORMAL:	img_aggr = *(CDAagghandle *)item;
	    case CDA$_EMPTY:	break;
#endif
	    default:		ChfStop( 1,  status );
	    }
	}
#if defined(NEW_CDA_SYMBOLS)
    while ( status != CDA_EMPTY );	    /* end do while */
#else
    while ( status != CDA$_EMPTY );	    /* end do while */
#endif
    }

/*
** Copy the segment aggregate.  (No image aggregates should be duplicated.)
*/
status = CDA_COPY_AGGREGATE_(
	    &dst_root_aggr,
	    &src_seg_aggr,
	    &dst_seg_aggr
	    );
LOWBIT_TEST_( status );

/*
** Reattach the image aggregates to the segment aggregate.
*/
if ( ice_cnt != 0 )
    {
    index = 0;
    /*
    ** Reattach the first saved image aggregate to the segment aggr.
    */
#if defined(NEW_CDA_SYMBOLS)
    aggregate_item = DDIF_SEG_CONTENT;
#else
    aggregate_item = DDIF$_SEG_CONTENT;
#endif
    item_size = sizeof(saved_img_aggrs[ index ]);
    status = CDA_STORE_ITEM_(
		&src_root_aggr,
		&src_seg_aggr,
		&aggregate_item,
		&item_size,
		&saved_img_aggrs[ index ],
		0,0
		);
    LOWBIT_TEST_( status );
    img_aggr = saved_img_aggrs[ index ];
    ++index;

    /*
    ** Loop, reattaching each additional saved image aggregate to 
    ** the segment aggr.
    */
    while ( index != ice_cnt )
	{
	status = CDA_INSERT_AGGREGATE_(
		    &saved_img_aggrs[ index ],
		    &img_aggr );
	LOWBIT_TEST_( status );
	img_aggr = saved_img_aggrs[ index ];
	++index;
	}

    /*
    ** Free the memory used to save the image aggregates.
    */
    _ImgFree( saved_img_aggrs );
    }

return dst_seg_aggr;
} /* end of _ImgCloneSegmentAggr */


/******************************************************************************
**  Create_frame_definition()
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
static struct FCT *Create_frame_definition( fat )
struct FAT  *fat;
{
long	     ice_cnt;
long	     ice_idx;
long	     ici_cnt;
long	     idu_cnt;
long	     idu_idx;

struct FCT  *fdf;
struct ICE  *ice;
struct ICI  *ici;
struct IDU  *idu;

/*
** Allocate the FCT block for the frame definition and set the block type.
*/
fdf = _ImgFrameAlloc( (long)0, (long)0 );

Set_misc_attrs( fdf, fat );
Set_frame_parameters( fdf, fat );
Set_presentation_attrs( fdf, fat );
Set_comp_space_attrs( fdf, fat );

ice_cnt = fat->FatL_IceCnt;
ice = fat->FatR_Ice;

for ( ice_idx = 0; ice_idx < ice_cnt; ++ice_idx )
    {
    _ImgFrameAppendIce( fdf, 0 );
    idu_cnt = ice->IceL_IduCnt;
    idu = ice->IceR_Idu;
    for ( idu_idx = 0; idu_idx < idu_cnt; ++idu_idx )
	{
	_ImgFrameAppendIdu( fdf, 0, 0 );
	Set_coding_attrs( fdf, idu, idu_idx );
	++idu;
	}
    ++ice;
    }

fdf->FctR_Blkhd.BhdB_Type = ImgK_BlktypFdf;

return fdf;
} /* end of Create_frame_definition */


/******************************************************************************
**  Set_coding_attrs()
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
static void Set_coding_attrs( fdf, idu, idu_idx )
struct FCT  *fdf;
struct IDU  *idu;
long	     idu_idx;
{

_ImgPut( fdf, Img_BitOrder,		&(idu->IduL_BitOrder),
	    LONGSIZE, idu_idx );
_ImgPut( fdf, Img_ByteOrder,		&(idu->IduL_ByteOrder),
	    LONGSIZE, idu_idx );
_ImgPut( fdf, Img_ByteUnit,		&(idu->IduL_ByteUnit),
	    LONGSIZE, idu_idx );
/* skip compression params */
_ImgPut( fdf, Img_CompressionType,	&(idu->IduL_CompressionType),
	    LONGSIZE, idu_idx );
_ImgPut( fdf, Img_DataOffset,		&(idu->IduL_DataOffset),
	    LONGSIZE, idu_idx );
_ImgPut( fdf, Img_DataType,		&(idu->IduL_DataType),
	    LONGSIZE, idu_idx );
_ImgPut( fdf, Img_NumberOfLines,	&(idu->IduL_NumberOfLines),
	    LONGSIZE, idu_idx );
_ImgPut( fdf, Img_PixelStride,		&(idu->IduL_PixelStride),
	    LONGSIZE, idu_idx );
_ImgPut( fdf, Img_PixelsPerLine,	&(idu->IduL_PixelsPerLine),
	    LONGSIZE, idu_idx );
_ImgPut( fdf, Img_PlaneBitsPerPixel,	&(idu->IduL_PlaneBitsPerPixel),
	    LONGSIZE, idu_idx );
_ImgPut( fdf, Img_ScanlineStride,	&(idu->IduL_ScanlineStride),
	    LONGSIZE, idu_idx );

return;
} /* end of Set_coding_attrs */


/******************************************************************************
**  Set_comp_space_attrs()
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
static void Set_comp_space_attrs( fdf, fat )
struct FCT  *fdf;
struct FAT  *fat;
{
long	     index;
struct ICI  *ici    = fat->FatR_Ici;

_ImgPut( fdf, Img_CompSpaceOrg,	    &(fat->FatL_CompSpaceOrg),	  LONGSIZE, 0 );
_ImgPut( fdf, Img_NumberOfComp,	    &(fat->FatL_NumberOfComp),	  LONGSIZE, 0 );
_ImgPut( fdf, Img_PixelGroupOrder,  &(fat->FatL_PixelGroupOrder), LONGSIZE, 0 );
_ImgPut( fdf, Img_PixelGroupSize,   &(fat->FatL_PixelGroupSize),  LONGSIZE, 0 );
_ImgPut( fdf, Img_PlaneSignif,	    &(fat->FatL_PlaneSignif),	  LONGSIZE, 0 );
_ImgPut( fdf, Img_PlanesPerPixel,   &(fat->FatL_PlanesPerPixel),  LONGSIZE, 0 );

for ( index = 0; index < fat->FatL_NumberOfComp; ++index )
    {
    _ImgPut( fdf, Img_BitsPerComp,
		&(ici[index].IciL_BitsPerComp),
		LONGSIZE, index );
    _ImgPut( fdf, Img_QuantLevelsPerComp,   
		&(ici[index].IciL_QuantLevelsPerComp),
		LONGSIZE, index );
    }

return;
} /* end of Set_comp_space_attrs */


/******************************************************************************
**  Set_frame_parameters()
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
static void Set_frame_parameters( fdf, fat )
struct FCT  *fdf;
struct FAT  *fat;
{

_ImgPut( fdf, Img_FrmBoxLLXC,	&(fat->FatL_FrmBoxLLXC),	LONGSIZE, 0 );
_ImgPut( fdf, Img_FrmBoxLLX,	&(fat->FatL_FrmBoxLLX),		LONGSIZE, 0 );
_ImgPut( fdf, Img_FrmBoxLLYC,	&(fat->FatL_FrmBoxLLYC),	LONGSIZE, 0 );
_ImgPut( fdf, Img_FrmBoxLLY,	&(fat->FatL_FrmBoxLLY),		LONGSIZE, 0 );
_ImgPut( fdf, Img_FrmBoxURXC,	&(fat->FatL_FrmBoxURXC),	LONGSIZE, 0 );
_ImgPut( fdf, Img_FrmBoxURX,	&(fat->FatL_FrmBoxURX),		LONGSIZE, 0 );
_ImgPut( fdf, Img_FrmBoxURYC,	&(fat->FatL_FrmBoxURYC),	LONGSIZE, 0 );
_ImgPut( fdf, Img_FrmBoxURY,	&(fat->FatL_FrmBoxURY),		LONGSIZE, 0 );
_ImgPut( fdf, Img_FrmPositionC, &(fat->FatL_FrmPositionC),	LONGSIZE, 0 );
_ImgPut( fdf, Img_FrmFxdPositionXC,
				&(fat->FatL_FrmfxdPositionXC),	LONGSIZE, 0);
_ImgPut( fdf, Img_FrmFxdPositionX,
				&(fat->FatL_FrmfxdPositionX),	LONGSIZE, 0 );
_ImgPut( fdf, Img_FrmFxdPositionYC,
				&(fat->FatL_FrmfxdPositionYC),	LONGSIZE, 0);
_ImgPut( fdf, Img_FrmFxdPositionY,
				&(fat->FatL_FrmfxdPositionY),	LONGSIZE, 0 );

return;
} /* end of Set_frame_parameters */


/******************************************************************************
**  Set_misc_attrs()
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
static void Set_misc_attrs( fdf, fat )
struct FCT  *fdf;
struct FAT  *fat;
{
char	 *user_label_str;
char	**user_label_strptrs;
long	  index;

user_label_strptrs = fat->FatA_UserLabelStrptrs;

if ( fat->FatL_UserLabelCnt != 0 )
    {
    for ( index = 0; index < fat->FatL_UserLabelCnt; ++index )
	{
	user_label_str = user_label_strptrs[ index ];
	_ImgPut( fdf, Img_UserLabel, user_label_str, strlen( user_label_str ),
		    index );
	}
    }

return;
} /* end of Set_misc_attrs */


/******************************************************************************
**  Set_presentation_attrs()
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
static void Set_presentation_attrs( fdf, fat )
struct FCT  *fdf;
struct FAT  *fat;
{

_ImgPut( fdf, Img_BrtPolarity,	    &(fat->FatL_BrtPolarity),	LONGSIZE, 0 );
/* Skip component wavelength info for now */
_ImgPut( fdf, Img_GridType,	    &(fat->FatL_GridType),	LONGSIZE, 0 );
_ImgPut( fdf, Img_LineProgression,  &(fat->FatL_LineProgression),LONGSIZE,0 );
/* skip lookup tables for now */
_ImgPut( fdf, Img_LPPixelDist,	    &(fat->FatL_LpPixelDist),	LONGSIZE, 0 );
_ImgPut( fdf, Img_PixelPath,	    &(fat->FatL_PixelPath),	LONGSIZE, 0 );
_ImgPut( fdf, Img_PPPixelDist,	    &(fat->FatL_PpPixelDist),	LONGSIZE, 0 );
/* skip private data for now */
_ImgPut( fdf, Img_SpectralMapping,  &(fat->FatL_SpectralMapping),LONGSIZE,0 );

return;
} /* end of Set_presentation_attrs */
