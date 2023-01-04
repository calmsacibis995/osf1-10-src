
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
** IMG__FRAME_UTILS
**
** FACILITY:
**
**  DECimage Application Services, Image Services Library (ISL)
**									
** ABSTRACT:
**
**  This module contains low-level utility functions which create and
**  manage image frame data structures.
**                                                                       
** ENVIRONMENT:								
**
**  VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**									
** AUTHOR:
**								
**  Mark W. Sornson 
**									
** CREATION DATE: 
**
**  4-OCT-1989						
**									
************************************************************************/

/*
** TABLE OF CONTENTS:
**
**	Global routines
*/
#ifdef NODAS_PROTO
struct	FCT	*_ImgCloneFrame();
long		 _ImgCreateFrameFromSegAggr();	/* back door function	*/
struct	FCT 	*_ImgFrameAlloc();
struct	FCT	*_ImgFrameAppendIce();
struct	FCT	*_ImgFrameAppendIdu();
void		 _ImgFrameDealloc();
void		 _ImgSetFrameDataType();
#endif


/*
** INCLUDE FILES:
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

#if defined(__VMS) || defined(VMS)
#include <ddif$def.h>
#include <cda$msg.h>
#include <cda$ptp.h>
#else
#if defined(NEW_CDA_SYMBOLS)
#include <ddifdef.h>
#include <cdamsg.h>
#else
#include <ddif_def.h>
#include <cda_msg.h>
#endif
#if defined(NEW_CDA_CALLS)
#include <cdaptp.h>
#else
#include <cda_ptp.h>
#endif
#endif

/*
** MACROS:
**
**  none
*/

/*
** EQUATED SYMBOLS: 
*/
#define	PRESENT		1
#define NOT_PRESENT	2
#define UNAVAILABLE	3

/*
** OWN STORAGE: 
*/
#if defined(__VAXC) || defined(VAXC)
globaldef 
#endif
char isl_pvt_sga_attr_name[]	= "IMG$_PVT_QUANT_LEVELS_PER_COMP";

/*
** EXTERNAL REFERENCES: 
**
**	From ISL
*/
#ifdef NODAS_PROTO
struct FCT  *ImgAllocateFrame();	/* from IMG_FRAME_UTILS		    */
struct FCT  *ImgExtractFrameDef();	/* from IMG_DEFINITION_UTILS	    */
void	     ImgResetCtx();		/* from IMG_CONTEXT_UTILS	    */

struct FCT  *_ImgAllocFct();		/* from IMG__BLOCK_UTILS	    */
char	    *_ImgCalloc();		/* from IMG__MEMORY_MGT_UTILS	    */
void	     _ImgDeallocFct();		/* from IMG__BLOCK_UTILS	    */
CDArootagghandle _ImgCreateRootAggregate();	/* from IMG_DDIF_IO_MGT		    */
int	     _ImgGet();			/* from IMG__ATTRIBUTE_ACCESS_UTILS */
int	     _ImgPut();			/* from IMG__ATTRIBUTE_ACCESS_UTILS */
#endif


/*
**	Module local routines
*/
#ifdef NODAS_PROTO
static void	 Copy_pvt_q_levels();
static void	 Init_pvt_q_levels();
static long	 Pvt_q_levels_check();
static void	 Verify_img_aggr();
#else
PROTO(static void Copy_pvt_q_levels, (struct FCT */*fct*/));
PROTO(static void Init_pvt_q_levels, (struct FCT */*fct*/));
PROTO(static long Pvt_q_levels_check, (struct FCT */*fct*/));
PROTO(static void Verify_img_aggr, (struct FCT */*fct*/, CDAagghandle /*img_aggr*/));
#endif


/*
**	From DDIF Toolkit are defined in <cdaptp.h>
*/

/*
**  External data
**
**	Status codes
*/
#if defined(__VAXC) || defined(VAXC)
globalvalue
     ImgX_AGNOTINFR
    ,ImgX_INVCMPMAP
    ,ImgX_NOIMGAGRP
    ;
#else
#include <img/ImgStatusCodes.h>
#endif

#if defined(__VAXC) || defined(VAXC)
globalref long int _ImgAL_DDIFAccessTable[];
#else
extern long int _ImgAL_DDIFAccessTable[];
#endif


/******************************************************************************
**  _ImgCloneFrame
**
**  FUNCTIONAL DESCRIPTION:
**
**	Clone a context block and frame structure from an original frame.
**	This returns a frame whose structure is the same as the original,
**	but which has no actual image data in it.  (The image data has to
**	be attached by the application.)
**
**  FORMAL PARAMETERS:
**
**      src_fid		    Old frame-id.  Frame-id if frame to be cloned.
**			    Passed by value.
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**	new_fid		    Frame-id of cloned frame.
**			    Passed by value.
**
**  SIGNAL CODES:
**
**	ImgX_NOIMGAGRP	    No image aggregate pointer was found in
**			    the (old) source frame.
**
**  SIDE EFFECTS:
**
******************************************************************************/
struct FCT *_ImgCloneFrame( src_fid )
struct	FCT *src_fid;
{
long	     flags	= ImgM_NoDataPlaneAlloc | ImgM_AutoDeallocate;
struct FCT  *frame_def;
struct FCT  *new_fid;

/*
** Extract a frame definition from the source frame and use it
** to produce the cloned frame (i.e., a copy of the source frame 
** but without data planes).
*/
frame_def = ImgExtractFrameDef( src_fid, 0 );
new_fid = ImgAllocateFrame( 0, 0, frame_def, flags );

return (struct FCT *) new_fid;
} /* end of _ImgCloneFrame */


/******************************************************************************
**  _ImgCreateFrameFromSegAggr()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create a frame from a fully populated DDIF$_SEG aggregate.
**	The contents of this frame will be owned by the application,
**	and not by the ISL frame.
**
**	The frame-id returned will have its context set to the first
**	DDIF$_SEG aggregate and the first DDIF$_IDU aggr in the SEG aggr.
**
**	NOTE:  This is a BACK DOOR function and is NOT used elsewhere by ISL.
**
**  FORMAL PARAMETERS:
**
**	root_aggr	Root aggregate handle to associate with frame.
**			Passed by value.
**
**	segment_aggr	Segment aggregate handle to associate with frame.
**			This segment must be of type $I, and must have
**			a DDIF$_SGA aggr associated with it, and must have
**			DDIF$_IMG content segments associated with it.
**			Passed by value.
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
**	none.
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
long _ImgCreateFrameFromSegAggr( root_aggr, seg_aggr )
CDArootagghandle root_aggr;
CDAagghandle seg_aggr;
{
long	     cs_org;
long	     data_type;
long	     planes_per_pixel;
long	     spectral_mapping;
struct	FCT *fid;

/*
** Allocate the frame using the given root and segment aggr handles.
*/
fid = _ImgFrameAlloc( root_aggr, seg_aggr );

/*
** Mark the frame content as application owned.  This means that when
** this frame is deleted, only the FCT block is deallocated.  The
** frame contents remains untouched.
*/
fid->FctL_Flags.FctV_ApplContent = TRUE;

/*
** If the the frame is a single plane monochrome image (bitonal or 
** greyscale), and the component space org is NOT band-interleaved-by-plane,
** set the cs-org attribute to BE band-interleaved-by-plane.
**
** This will keep the viewer for coughing on images that previously
** displayed without complaint.
*/
_ImgGet( fid, Img_CompSpaceOrg,	    &cs_org,		LONGSIZE, 0, 0 );
_ImgGet( fid, Img_PlanesPerPixel,   &planes_per_pixel,	LONGSIZE, 0, 0 );
_ImgGet( fid, Img_SpectralMapping,  &spectral_mapping,	LONGSIZE, 0, 0 );

if ( cs_org == ImgK_BandIntrlvdByPixel &&
     planes_per_pixel == 1 &&
     spectral_mapping == ImgK_MonochromeMap )
    {
    cs_org = ImgK_BandIntrlvdByPlane;
    _ImgPut( fid, Img_CompSpaceOrg, &cs_org, LONGSIZE, 0 );
    }

return (long) fid;
} /* end of _ImgCreateFrameFromSegAggr */


/************************************************************************
** _ImgFrameAlloc
**									
** FUNCTIONAL DESCRIPTION:						
**									
**	Allocate the required data structures needed for a frame	
**	and link them together.  If a block address is passed in,	
**	do not allocate one.  Omitted trailing parameters are		
**	automatically allocated, as are any parameters that are		
**	passed in with an address value of zero.			
**									
**									
** FORMAL PARAMETERS:
**
**	ROOT_AGGR	Root aggregate handle to be associated
**			with the allocated frame.  This argument
**			is optional.  If omitted, a root aggregate
**			will be created.
**			Passed by value.
**
**	SEG_AGGR	DDIF$_SEG (segment) aggregate handle to be
**			associated with the allocated frame.  This
**			argument is optional.  If omitted, a DDIF$_SEG
**			aggregate (and DDIF$_SGA aggregate) will be
**			created.
**			Passed by value.
**
** IMPLICIT INPUTS:							
**									
**	none.
**									
** IMPLICIT OUTPUTS:							
**									
**	none.
**
** FUNCTION VALUE:
**
**	fct		Frame context block address to be used by the
**			caller as a frame-id.
**			Passed by value.
**
** SIGNAL CODES:
**
**	none.
**
** SIDE EFFECTS:
**									
**	unknown								
*************************************************************************/
struct FCT *_ImgFrameAlloc( root_aggr, seg_aggr )
CDArootagghandle root_aggr;
CDAagghandle seg_aggr;
{
long	 aggregate_handle;
CDAconstant aggregate_item;
long aggregate_item_value;
CDAaggtype aggregate_type;
CDAconstant category_type;
CDAconstant ddif_code;
long	 icecnt			    = 0;
CDAaddress item;
long	 item_size;
CDAagghandle img_aggr;
long	 length;
long	 pvt_quant_levels_status;
CDAagghandle ret_sga_aggr;
CDAagghandle sga_aggr		    = NULL;
int	 status;

struct ITEMCODE	 item_code;
struct FCT	*fct;

/*
** If root and segment aggrs are passed in, get the SGA (segment attrs)
** aggr handle from the seg aggr.
*/
if ((root_aggr != NULL) && (seg_aggr != NULL))
    {
#if defined(NEW_CDA_SYMBOLS) 
    aggregate_item = DDIF_SEG_SPECIFIC_ATTRIBUTES;
#else
    aggregate_item = DDIF$_SEG_SPECIFIC_ATTRIBUTES;
#endif

    status = CDA_LOCATE_ITEM_(		    /* Retrieve and store DDIF$_SGA */
			&root_aggr,	    /* aggregate.		    */
			&seg_aggr,
			&aggregate_item,
			&item,
			&length,
			0, 0 );
    LOWBIT_TEST_( status );
    sga_aggr = *(CDAagghandle *)item;
    }

/*
** Allocate an empty FCT block.
*/
fct = _ImgAllocFct();

/*
** Create the root, segment, and presenation attributes aggregates
** if they don't already exist.
*/
if ( root_aggr == NULL )
    root_aggr = _ImgCreateRootAggregate(0);

if ( seg_aggr == NULL )
    {
#if defined(NEW_CDA_SYMBOLS)
    aggregate_type = DDIF_SEG;
#else
    aggregate_type = DDIF$_SEG;
#endif

/*
** Create the segment aggregate
*/
    status = CDA_CREATE_AGGREGATE_(
		&root_aggr,		
		&aggregate_type,
		&seg_aggr );
    LOWBIT_TEST_( status );
    }

if ( sga_aggr == NULL )
    {
#if defined(NEW_CDA_SYMBOLS)
    aggregate_type = DDIF_SGA;
#else
    aggregate_type = DDIF$_SGA;
#endif

/*
** Create the segment attributes aggregate
*/
    status = CDA_CREATE_AGGREGATE_(
		&root_aggr,
		&aggregate_type,
		&sga_aggr );
    LOWBIT_TEST_( status );

#if defined(NEW_CDA_SYMBOLS)
    aggregate_item = DDIF_SEG_SPECIFIC_ATTRIBUTES;
#else
    aggregate_item = DDIF$_SEG_SPECIFIC_ATTRIBUTES;
#endif
    item_size = sizeof(sga_aggr);

/*
** Store the presentation attribure aggregate in the segment aggregate
*/
    status = CDA_STORE_ITEM_(
		&root_aggr,
		&seg_aggr,
		&aggregate_item,
		&item_size,
		&sga_aggr,
		0, 0 );
    LOWBIT_TEST_( status);
    }

/*
** Store the image content category tag in the DDIF$_SGA aggregate.
*/
item_size = 2;
#if defined(NEW_CDA_SYMBOLS)
aggregate_item = DDIF_SGA_CONTENT_CATEGORY;
category_type = DDIF_K_I_CATEGORY;
#else
aggregate_item = DDIF$_SGA_CONTENT_CATEGORY;
category_type = DDIF$K_I_CATEGORY;
#endif

status = CDA_STORE_ITEM_(
	    &root_aggr,
	    &sga_aggr,
	    &aggregate_item,
	    &item_size,
	    "$I",
	    0,
	    &category_type );
LOWBIT_TEST_( status );

/*
** Store the aggregates and reset the current img content element and
** img data unit context.  If there are no content elements and/or
** no data units, the current values will retain their initial value.
*/
_ImgPut( fct, Img_RootAggr, &root_aggr, sizeof(root_aggr), 0 );
_ImgPut( fct, Img_SegAggr, &seg_aggr, sizeof(seg_aggr), 0 );
_ImgPut( fct, Img_SgaAggr, &sga_aggr, sizeof(sga_aggr), 0 );

/*
** Find out how many image content elements there are, since there
** might be some attached to the DDIF$_SEG aggregate that was passed
** in (if one was passed in, that is).
*/
#if defined(NEW_CDA_SYMBOLS)
aggregate_item = DDIF_SEG_CONTENT;
#else
aggregate_item = DDIF$_SEG_CONTENT;
#endif
status = CDA_LOCATE_ITEM_(
                &root_aggr,
                &seg_aggr,
                &aggregate_item,
                &item,
                &length,
                0, 0 );
switch ( status )
    {
#if defined(NEW_CDA_SYMBOLS)
    case CDA_NORMAL:
#else
    case CDA$_NORMAL:
#endif
	icecnt = 1;
	img_aggr = *(CDAagghandle *)item;
	/*
	** See if there are any more seg content
	** aggregates following the first one.
	*/
	do
	    {
	    status = CDA_NEXT_AGGREGATE_(
			&img_aggr,
			&img_aggr );

#if defined(NEW_CDA_SYMBOLS)
	    if ( status == CDA_NORMAL )
#else
	    if ( status == CDA$_NORMAL )
#endif
		++icecnt;
	    } /* end do-while */
#if defined(NEW_CDA_SYMBOLS)
	while ( status != CDA_ENDOFSEQ );
#else
	while ( status != CDA$_ENDOFSEQ );
#endif

#if defined(NEW_CDA_SYMBOLS)
    case CDA_EMPTY:
#else
    case CDA$_EMPTY:
#endif
	break;
    default:
	ChfSignal( 1,  status );
    }
_ImgPut( fct, Img_Icecnt, &icecnt, sizeof(icecnt), 0 );

/*
** Reset the frame context to the first image content element and the
** first image data unit of the first image content element.  (If neither
** one can be set, nothing bad happens.)
*/
ImgResetCtx( fct );

/*
** Mark frame content as being ISL owned.
*/
fct->FctL_Flags.FctV_ApplContent = FALSE;

status = Pvt_q_levels_check( fct );
if ( status )
    {
    Init_pvt_q_levels( fct );
    Copy_pvt_q_levels( fct );
    }

/*
**  This looks like a bug ... why was this here?
**else
**    Init_pvt_q_levels( fct );
*/

return fct;
} /* end of _ImgFrameAlloc function */

/******************************************************************************
** _ImgFrameAppendIce
**									
** FUNCTIONAL DESCRIPTION:						
**									
**	Append an image content element (ICE) to the sequence of image content
**	elements in the frame.  If the sequence is empty, the ICE will be
**	inserted as the first element of the sequence.  This function
**	will allocate a DDIF image content aggregate or take one as a 
**	parameter.  The image aggregate will represent the ICE.
**									
** FORMAL PARAMETERS:							
**									
**	fct, by reference:	address of Frame Context Block pointer
**	IMG_AGGR, by value:	address of DDIF Image Aggregate handle,
**				optional
**									
** IMPLICIT INPUTS:							
**									
**	none.
**									
** IMPLICIT OUTPUTS:							
**									
**	none.
**									
** FUNCTION VALUE:							
**
**	fct	    Frame context block.  Passed as frame it by value.
**
** SIGNAL CODES:							
**
**	none.
**
** SIDE EFFECTS:
**									
**	none								
******************************************************************************/
struct FCT *_ImgFrameAppendIce( fct, img_aggr )
struct FCT *fct;	/* pointer to FCT, required */
CDAagghandle img_aggr;	/* image aggregate handle   */
{
CDAconstant aggregate_item;
CDAaggtype aggregate_type;
CDAagghandle cur_img_aggr;
long  ice_cnt;
CDAagghandle idu_aggr	= NULL;
CDAaddress item;
long  item_size;
long  length;
CDAagghandle next_img_aggr;
CDArootagghandle root_aggr;
CDAagghandle seg_aggr;
int  status;

/*
** Get the frame root aggregate and create and/or locate the
** aggregates that will be stored in the frame as a result of
** this function.  Also get the frame segment aggregate and the
** content element count.
*/
_ImgGet( fct, Img_RootAggr, &root_aggr, sizeof(root_aggr), 0, 0 );
_ImgGet( fct, Img_SegAggr, &seg_aggr, sizeof(seg_aggr), 0, 0 );
_ImgGet( fct, Img_Icecnt, &ice_cnt, sizeof(ice_cnt), 0, 0 );
++ice_cnt;					/* incr ice cnt by 1	*/

if ( img_aggr == NULL )
    {
#if defined(NEW_CDA_SYMBOLS)
    aggregate_type = DDIF_IMG;
#else
    aggregate_type = DDIF$_IMG;
#endif

/*
** Create an image aggregate if there wasn't one passed in.
*/
    status = CDA_CREATE_AGGREGATE_(
		&root_aggr,
		&aggregate_type,
		&img_aggr );
    LOWBIT_TEST_( status );
    }
else
    {
#if defined(NEW_CDA_SYMBOLS)
    aggregate_item = DDIF_IMG_CONTENT;
#else
    aggregate_item = DDIF$_IMG_CONTENT;
#endif

/*
** Get the IDU aggregate from the image aggregate that was passed in.
*/
    status = CDA_LOCATE_ITEM_(
		&root_aggr,
		&img_aggr,
		&aggregate_item,
		&item,
		&length,
		0, 0 );
    LOWBIT_TEST_( status );
    idu_aggr = *(CDAagghandle *)item;
    }

/*
** Find the current end of the DDIF$_IMG sequence, and append the
** new DDIF$_IMG aggr (ICE) to the sequence.
*/
#if defined(NEW_CDA_SYMBOLS)
aggregate_item = DDIF_SEG_CONTENT;
#else
aggregate_item = DDIF$_SEG_CONTENT;
#endif

/*
** Get the first image aggragate attached to the frame.
*/
status = CDA_LOCATE_ITEM_(
	    &root_aggr,
	    &seg_aggr,
	    &aggregate_item,
	    &item,
	    &length,
	    0, 0 );
switch (status )
    {
    /*
    ** There are no other img aggrs in the frame, which makes this
    ** the first one.  Therefore, store it in the frame as the first
    ** segment content aggregate.
    */
#if defined(NEW_CDA_SYMBOLS)
    case CDA_EMPTY:
        aggregate_item = DDIF_SEG_CONTENT;
#else
    case CDA$_EMPTY:
        aggregate_item = DDIF$_SEG_CONTENT;
#endif
        item_size = sizeof(img_aggr);

	status = CDA_STORE_ITEM_(
		    &root_aggr,
		    &seg_aggr,
		    &aggregate_item,
		    &item_size,
		    &img_aggr,
		    0, 0 );
	LOWBIT_TEST_( status );
	/*
	** Verify that the item was stored.
	*/
#if defined(NEW_CDA_SYMBOLS)
        aggregate_item = DDIF_SEG_CONTENT;
#else
        aggregate_item = DDIF$_SEG_CONTENT;
#endif

	status = CDA_LOCATE_ITEM_(
		    &root_aggr,
		    &seg_aggr,
		    &aggregate_item,
		    &item,
		    &length,
		    0, 0 );
	LOWBIT_TEST_( status );
	break;
    /*
    ** There is at least one img aggr attached to the frame.  Loop
    ** finding the next until there are no more.  Attach the new aggr
    ** after the last one in the list.
    */
#if defined(NEW_CDA_SYMBOLS)
    case CDA_NORMAL:
#else
    case CDA$_NORMAL:
#endif
	cur_img_aggr = *(CDAagghandle *)item;
	next_img_aggr = cur_img_aggr;
	do
	    {
	    cur_img_aggr = next_img_aggr;
	    status = CDA_NEXT_AGGREGATE_(
			&cur_img_aggr,
			&next_img_aggr );
	    } 	    /* end do while */
#if defined(NEW_CDA_SYMBOLS)
	while ( status != CDA_ENDOFSEQ );
#else
	while ( status != CDA$_ENDOFSEQ );
#endif
	status = CDA_INSERT_AGGREGATE_(
		    &img_aggr,
		    &cur_img_aggr );
	LOWBIT_TEST_( status );
	break;
    /*
    ** Error, therefore stop.
    */
    default:
	ChfStop( 1,  status );
    }

/*
** Store the aggregates to establish the current context, and 
** store the ice count.
*/
_ImgPut( fct, Img_ImgAggr, &img_aggr, sizeof(img_aggr), 0 );
_ImgPut( fct, Img_IduAggr, &idu_aggr, sizeof(idu_aggr), 0 );
_ImgPut( fct, Img_Icecnt, &ice_cnt, sizeof(ice_cnt), 0 );

return( fct );
} /* end of _ImgFrameAppendIce function */

/******************************************************************************
** _ImgFrameAppendIdu
**									
** FUNCTIONAL DESCRIPTION:						
**									
**	Append an image data unit block (IDU) to the end		
**	of the IDU list pointed to by the current ICE. 
**									
** FORMAL PARAMETERS:							
**									
**	fct		Frame context block used as the frame identifier.
**			The frame-id is passed by value.
**
**	IMG_AGGR	Image content aggregate handle.
**			Passed by value.
**
**	IDU_AGGR	Image data unit aggregate handle.
**			Passed by value.
**									
** IMPLICIT INPUTS:							
**									
**	none
**									
** IMPLICIT OUTPUTS:							
**									
**	none
**									
** ROUTINE VALUE:							
**
**	fct	    Frame context block of the frame being operated on.
**		    Passed as frame-id by value.
**
** SIGNAL CODES:
**
**	none
**
** SIDE EFFECTS:
**									
**	none.
******************************************************************************/
struct FCT *_ImgFrameAppendIdu( fct, img_aggr, idu_aggr )
struct	FCT *fct;			/* pointer to FCT, required */
CDAagghandle img_aggr;
CDAagghandle idu_aggr;
{
CDAconstant aggregate_item;
CDAaggtype aggregate_type;
CDAagghandle cur_idu_aggr;
CDAaddress item;
long  item_size;
long  length;
CDAagghandle next_idu_aggr;
CDArootagghandle root_aggr;
CDAagghandle seg_aggr;
int  status;

/*
** Get the root and segment aggregates for the frame.
*/
_ImgGet( fct, Img_RootAggr, &root_aggr, sizeof(root_aggr), 0, 0 );
_ImgGet( fct, Img_SegAggr, &seg_aggr, sizeof(seg_aggr), 0, 0 );

if ( idu_aggr == NULL )
    {
#if defined(NEW_CDA_SYMBOLS)
    aggregate_type = DDIF_IDU;
#else
    aggregate_type = DDIF$_IDU;
#endif

/*
** Create and IDU aggregate if we weren't given one.
*/
    status = CDA_CREATE_AGGREGATE_(
		&root_aggr,
		&aggregate_type,
		&idu_aggr );
    LOWBIT_TEST_( status );
    }

/*
** Get the current img aggr from the frame if one wasn't passed in.
*/
if ( img_aggr == NULL )
    _ImgGet( fct, Img_ImgAggr, &img_aggr, sizeof(img_aggr), 0, 0 );

Verify_img_aggr( fct, img_aggr );

/*
** Attach the idu aggr to the ice img aggr.  First, get the first
** IDU in the img content seg.
*/
#if defined(NEW_CDA_SYMBOLS)
aggregate_item = DDIF_IMG_CONTENT;
#else
aggregate_item = DDIF$_IMG_CONTENT;
#endif
status = CDA_LOCATE_ITEM_(
	    &root_aggr,
	    &img_aggr,
	    &aggregate_item,
	    &item,
	    &length, 0,0 );
switch ( status )
    {
    /*
    ** There are no other idu aggrs in the img aggr, which makes this
    ** the first one.  Therefore, store it in the img aggr as the first
    ** aggregate.
    */
#if defined(NEW_CDA_SYMBOLS)
    case CDA_EMPTY:
        aggregate_item = DDIF_IMG_CONTENT;
#else
    case CDA$_EMPTY:
        aggregate_item = DDIF$_IMG_CONTENT;
#endif
        item_size = sizeof(idu_aggr);
	status = CDA_STORE_ITEM_(
		    &root_aggr,
		    &img_aggr,
		    &aggregate_item,
		    &item_size,
		    &idu_aggr,
		    0, 0 );
	LOWBIT_TEST_( status );
	break;
    /*
    ** There is at least one idu aggr attached to the img aggr.  Loop
    ** finding the next until there are no more.  Attach the new idu aggr
    ** after the last one in the list.
    */
#if defined(NEW_CDA_SYMBOLS)
    case CDA_NORMAL:
#else
    case CDA$_NORMAL:
#endif
	cur_idu_aggr = *(CDAagghandle *)item;
	next_idu_aggr = cur_idu_aggr;
	do
	    {
	    cur_idu_aggr = next_idu_aggr;
	    status = CDA_NEXT_AGGREGATE_(
			&cur_idu_aggr,
			&next_idu_aggr );
	    }	    /* end do while */
#if defined(NEW_CDA_SYMBOLS)
	while ( status != CDA_ENDOFSEQ );
#else
	while ( status != CDA$_ENDOFSEQ );
#endif
	status = CDA_INSERT_AGGREGATE_(
		    &idu_aggr,
		    &cur_idu_aggr );
	LOWBIT_TEST_( status );
	break;
    /*
    ** Error, therefore stop.
    */
    default:
	ChfStop( 1,  status );
    }

/*
** Store the aggregates to establish the current context.
*/
_ImgPut( fct, Img_ImgAggr, &img_aggr, sizeof(img_aggr), 0 );
_ImgPut( fct, Img_IduAggr, &idu_aggr, sizeof(idu_aggr), 0 );

return( fct );
} /* end of _ImgFrameAppendIdu function */

/******************************************************************************
** _ImgFrameDealloc
**									
** FUNCTIONAL DESCRIPTION:						
**									
**	Deassemble the frame data structures and deallocate those	
**	blocks with no other references to them.  (Only deallocate	
**	blocks dynamically allocated by the Img__BLOCK_UTILS.)		
**									
**									
** FORMAL PARAMETERS:							
**									
**	fct	    Frame context block.  Passed in as frame-id by value.
**									
** IMPLICIT INPUTS:							
**
**	none
**
** IMPLICIT OUTPUTS:
**
**	none
**									
** FUNCTION VALUE:
**
**	void
**
** SIGNAL CODES:
**
**	none.
**
** SIDE EFFECTS:
**									
**	none.								
******************************************************************************/
void _ImgFrameDealloc( fct )
struct FCT *fct;				/* pointer to FCT, required */
{
CDArootagghandle root_aggr;
CDAagghandle seg_aggr;
int status;

if ( fct->FctL_Flags.FctV_ApplContent == FALSE )
    {
    /*
    ** Retrieve the root aggregate handle and the segment aggregate handle.
    ** By deleting the segment aggregate, all the attributes are deleted, and
    ** all the image content aggregates attached to the segment aggregate as
    ** segment content are deleted.
    */
    _ImgGet( fct, Img_RootAggr, &root_aggr, sizeof(root_aggr), 0, 0 );
    _ImgGet( fct, Img_SegAggr, &seg_aggr, sizeof(seg_aggr), 0, 0 );

    /*
    ** Delete the segment aggregate.
    */
    status = CDA_DELETE_AGGREGATE_( &root_aggr, &seg_aggr );
    LOWBIT_TEST_( status );

    /*
    ** Delete the root aggregate
    */
    status = CDA_DELETE_ROOT_AGGREGATE_( &root_aggr );
    LOWBIT_TEST_( status );
    }

/*
** Deallocate the FCT block.
*/
_ImgDeallocFct( fct );

return;
} /* end of _ImgFrameDealloc function */

/******************************************************************************
**  _ImgSetFrameDataType
**
**  FUNCTIONAL DESCRIPTION:
**
**	Set the data type of the frame to be private, bitonal, greyscale,
**	or multispectral.
**
**  FORMAL PARAMETERS:
**
**	fid	    Frame id of frame whose frame data type to set.
**		    Passed by value.
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
**	ImgX_INVCMPMAP	Invalid component mapping scheme in frame.
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
void _ImgSetFrameDataType( fid )
    struct FCT *fid; 
    {
    long bits_per_comp;
    long bits_per_pixel;
    long index		= 0;
    long planes_per_pixel;
    long spect_type	= 0;
    long spectral_mapping;

    _ImgGet( fid, Img_BitsPerPixel, &bits_per_pixel, sizeof(bits_per_pixel), 0, 0 );
    _ImgGet( fid, Img_SpectralMapping, &spectral_mapping, sizeof(spectral_mapping), 0, 0 );
    _ImgGet( fid, Img_PlanesPerPixel, &planes_per_pixel, sizeof(planes_per_pixel), 0, 0 );
    _ImgGet( fid, Img_BitsPerComp, &bits_per_comp, sizeof(bits_per_comp), 0, index );
                   
    switch (spectral_mapping)
        {
#if defined(NEW_CDA_SYMBOLS)
        case DDIF_K_PRIVATE_MAP:
#else
        case DDIF$K_PRIVATE_MAP:
#endif
            spect_type = ImgK_ClassPrivate;
            break;
#if defined(NEW_CDA_SYMBOLS)
        case DDIF_K_MONOCHROME_MAP:
#else
        case DDIF$K_MONOCHROME_MAP:
#endif
            if ( planes_per_pixel == 1 && bits_per_comp == 1 &&
                bits_per_pixel == 1 )
                spect_type = ImgK_ClassBitonal;
            else
                spect_type = ImgK_ClassGreyscale;
            break;
#if defined(NEW_CDA_SYMBOLS)
        case DDIF_K_GENERAL_MAP:
        case DDIF_K_LUT_MAP:
        case DDIF_K_RGB_MAP:
        case DDIF_K_CMY_MAP:
        case DDIF_K_YUV_MAP:
        case DDIF_K_HSV_MAP:
        case DDIF_K_HLS_MAP:
        case DDIF_K_YIQ_MAP:
#else
        case DDIF$K_GENERAL_MAP:
        case DDIF$K_LUT_MAP:
        case DDIF$K_RGB_MAP:
        case DDIF$K_CMY_MAP:
        case DDIF$K_YUV_MAP:
        case DDIF$K_HSV_MAP:
        case DDIF$K_HLS_MAP:
        case DDIF$K_YIQ_MAP:
#endif
            spect_type = ImgK_ClassMultispect;
            break;
    
        default:
            /*
            ** Signal invalid component mapping scheme value.
            ** pass the parameter value for display by the
            ** message handler, and the fid for use by the condition
            ** handler (if there is one).
            */
            ChfStop( 3,  ImgX_INVCMPMAP, 2, spectral_mapping, fid );
        } /* end of switch */

    _ImgPut( fid, Img_ImageDataClass, &spect_type, sizeof(spect_type), 0 );
               
    return;
    } /* end of _ImgSetFrameDataType */              


/******************************************************************************
**  Copy_pvt_q_levels()
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
static void Copy_pvt_q_levels( fct )
struct FCT  *fct;
{
long	 comp_idx;
CDAagghandle cur_pvt_sub_aggr;
CDAconstant ddif_code;
CDAaddress item_addr;
long	 item_length;
long	 item_value;
CDAagghandle main_pvt_aggr;
CDAagghandle prev_pvt_sub_aggr;
long	 quant_levels_per_comp;
int	 status;
struct ITEMCODE	item_code;

/*
** Get main pvt SGA aggregate and first sub aggregate
*/
#if defined(NEW_CDA_SYMBOLS)
ddif_code = DDIF_SGA_IMG_PRIVATE_DATA;
#else
ddif_code = DDIF$_SGA_IMG_PRIVATE_DATA;
#endif
status = CDA_LOCATE_ITEM_(
	    &(fct->FctL_RootAggr),
	    &(fct->FctL_SgaAggr),
	    &ddif_code,
	    &item_addr,
	    &item_length,
	    0,0
	    );
LOWBIT_TEST_(status );
main_pvt_aggr = *(CDAagghandle *)item_addr;

#if defined(NEW_CDA_SYMBOLS)
ddif_code = DDIF_PVT_DATA;
#else
ddif_code = DDIF$_PVT_DATA;
#endif
status = CDA_LOCATE_ITEM_(
	    &(fct->FctL_RootAggr),
	    &main_pvt_aggr,
	    &ddif_code,
	    &item_addr,
	    &item_length,
	    0,0
	    );
LOWBIT_TEST_(status );
cur_pvt_sub_aggr = *(CDAagghandle *)item_addr;

/*
** Get quant levels per comp values from private aggrs
*/
comp_idx = 0;
do
    {
#if defined(NEW_CDA_SYMBOLS)
    ddif_code = DDIF_PVT_DATA;
#else
    ddif_code = DDIF$_PVT_DATA;
#endif
    status = CDA_LOCATE_ITEM_(
		&(fct->FctL_RootAggr),
		&cur_pvt_sub_aggr,
		&ddif_code,
		&item_addr,
		&item_length,
		0, 0
		);
    LOWBIT_TEST_( status );
    quant_levels_per_comp = *(long *)item_addr;

    _ImgPut( fct, Img_QuantLevelsPerComp, &quant_levels_per_comp, LONGSIZE,
		comp_idx );

    prev_pvt_sub_aggr = cur_pvt_sub_aggr;
    status = CDA_NEXT_AGGREGATE_( 
		&prev_pvt_sub_aggr,
		&cur_pvt_sub_aggr );

    ++comp_idx;
    }
#if defined(NEW_CDA_SYMBOLS)
while ( status != CDA_ENDOFSEQ );
#else
while ( status != CDA$_ENDOFSEQ );
#endif

/*
** Delete private sga attributes.  Deleting the top level
** aggr will automatically delete all the sub-aggregates.
*/
#if defined(NEW_CDA_SYMBOLS)
ddif_code = DDIF_SGA_IMG_PRIVATE_DATA;
#else
ddif_code = DDIF$_SGA_IMG_PRIVATE_DATA;
#endif
status = CDA_ERASE_ITEM_(
	    &(fct->FctL_RootAggr),
	    &(fct->FctL_SgaAggr),
	    &ddif_code,
	    0
	    );
LOWBIT_TEST_( status );

return;
} /* end of Copy_pvt_q_levels */


/******************************************************************************
**  Init_pvt_q_levels()
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
static void Init_pvt_q_levels( fct )
struct FCT  *fct;
{
long	 bits_per_comp;
CDAconstant ddif_code;
long	 idx;
CDAaddress item_addr;
int	 item_idx;
long	 item_length;
long	 number_of_comp;
long	*q_levels;
int	 status;

struct ITEMCODE	item_code;

*((long *)&item_code) = Img_QuantLevelsPerComp;
ddif_code = _ImgAL_DDIFAccessTable[item_code.ItmW_Offset];

status = CDA_LOCATE_ITEM_(
	    &(fct->FctL_RootAggr),
	    &(fct->FctL_SgaAggr),
	    &ddif_code,
	    &item_addr,
	    &item_length,
	    &item_idx,
	    0
	    );
switch ( status )
    {
#if defined(NEW_CDA_SYMBOLS)
    case CDA_NORMAL:
    case CDA_EMPTY:
#else
    case CDA$_NORMAL:
    case CDA$_EMPTY:
#endif
	/*
	** Quant levels are supported by CDA ... Therefore, there is no
	** need for ISL to store them privately.
	*/
	fct->FctL_Flags.FctV_PvtQLevels = FALSE;
	break;
#if defined(NEW_CDA_SYMBOLS)
    case CDA_INVITMCOD:
#else
    case CDA$_INVITMCOD:
#endif
    default:
	/*
	** Quant levels are not yet supported by CDA ... Therefore, allocate
	** dynamic storage to store them privately in.
	**
	**  NOTE:   Q levels are implemented as a counted list of
	**	    longwords.  Allocate one longword for each
	**	    for each component, plus a longword for the count.
	*/
	_ImgGet( fct, Img_NumberOfComp, &number_of_comp, LONGSIZE, 0, 0 );
	q_levels = (long *) _ImgCalloc( (number_of_comp + 1), LONGSIZE );
	*q_levels = number_of_comp;
	fct->FctA_PvtQLevels = q_levels;
	fct->FctL_Flags.FctV_PvtQLevels = TRUE;

	/*
	** Supply defaults based on the number of bits per pixel
	*/
	for ( idx = 0; idx < number_of_comp; ++idx )
	    {
	    _ImgGet( fct, Img_BitsPerComp, &bits_per_comp, LONGSIZE, 0, idx );
	    q_levels[ idx + 1 ] = 1 << bits_per_comp;
	    }
	break;
    }

return;
} /* end of Init_pvt_q_levels */


/******************************************************************************
**  Pvt_q_levels_check()
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
static long Pvt_q_levels_check( fct )
struct FCT  *fct;
{
CDAconstant ddif_code;
long	 found_status	    = 0;
CDAaddress item;
long	 item_size;
CDAagghandle pvt_sga_aggr;
int	 status;

/*
** Test to see if private sga attrs are present, and if they are,
** if they're ISL private attrs (identified by the static string
** isl_pvt_sga_attr_name defined in this module's header).
*/
#if defined(NEW_CDA_SYMBOLS)
ddif_code = DDIF_SGA_IMG_PRIVATE_DATA;
#else
ddif_code = DDIF$_SGA_IMG_PRIVATE_DATA;
#endif
status = CDA_LOCATE_ITEM_(
	    &(fct->FctL_RootAggr),
	    &(fct->FctL_SgaAggr),
	    &ddif_code,
	    &item,
	    &item_size,
	    0,0 );
switch ( status )
    {
#if defined(NEW_CDA_SYMBOLS)
    case CDA_NORMAL:
	ddif_code = DDIF_PVT_NAME;
#else
    case CDA$_NORMAL:
	ddif_code = DDIF$_PVT_NAME;
#endif
	pvt_sga_aggr = *(CDAagghandle *)item;
	status = CDA_LOCATE_ITEM_(
	    &(fct->FctL_RootAggr),
	    &pvt_sga_aggr,
	    &ddif_code,
	    &item,
	    &item_size,
	    0,0 );
	switch ( status )
	    {
#if defined(NEW_CDA_SYMBOLS)
	    case CDA_NORMAL:
#else
	    case CDA$_NORMAL:
#endif
		if ( item_size == strlen( isl_pvt_sga_attr_name ) )
		    {
		    status = memcmp( item, isl_pvt_sga_attr_name, item_size );
		    if ( status == 0 )
			{
			found_status = 1;
			}
		    }
	    default:
		break;
	    }
	break;
    default:
	break;
    }

return found_status;
} /* end Pvt_q_levels_check */


/******************************************************************************
**  Verify_img_aggr
**
**  FUNCTIONAL DESCRIPTION:
**
**	Verify that a particular DDIF$_IMG aggregate handle is
**	actually a constituant of a particular frame.
**
**  FORMAL PARAMETERS:
**
**	fct	    Frame context block.
**
**	img_aggr    Image content aggregate handle.
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
**	ImgX_AGNOTINFR	    Aggregate not in frame.  The img aggregate passed
**			    in (to be verified) was not in the frame that
**			    was passed in.
**
**	ImgX_NOIMGAGRP	    No image aggregate pointer.  Either the img_aggr
**			    parameter was 0, or DDIF$_SEG_CONTENT could
**			    not be found in the fct that was passed in.
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
static void Verify_img_aggr( fct, img_aggr )
struct	FCT *fct;
CDAagghandle img_aggr;
{
CDAconstant aggregate_item;
CDAagghandle cur_img_aggr    = NULL;
CDAaddress item;
long  length;
int  match	    = FALSE;
CDArootagghandle root_aggr;
CDAagghandle seg_aggr;
int  status;

if ( img_aggr == NULL )
    ChfStop( 1,  ImgX_NOIMGAGRP );

_ImgGet( fct, Img_RootAggr, &root_aggr, sizeof(root_aggr), 0, 0 );
_ImgGet( fct, Img_SegAggr, &seg_aggr, sizeof(seg_aggr), 0, 0 );

#if defined(NEW_CDA_SYMBOLS)
aggregate_item = DDIF_SEG_CONTENT;
#else
aggregate_item = DDIF$_SEG_CONTENT;
#endif
status = CDA_LOCATE_ITEM_(
	    &root_aggr,
	    &seg_aggr,
	    &aggregate_item,
	    &item,
	    &length,
	    0, 0 );
LOWBIT_TEST_( status );
cur_img_aggr = *(CDAagghandle *)item;

if ( cur_img_aggr == NULL )
    ChfStop( 1,  ImgX_NOIMGAGRP );

do
    {						/* loop, looking for a	*/
    if ( cur_img_aggr == img_aggr )		/* match between the	*/
	{					/* img aggr passed in	*/
	match = TRUE;				/* and the aggrs in the	*/
	break;					/* frame.		*/
	}
    status = CDA_NEXT_AGGREGATE_(
		&cur_img_aggr,
		&cur_img_aggr );
    }		/* end do while */
#if defined(NEW_CDA_SYMBOLS)
while ( status == CDA_NORMAL );
#else
while ( status == CDA$_NORMAL );
#endif

/*
** Stop if there was no match.
*/
if ( match == FALSE )
    ChfStop( 1,  ImgX_AGNOTINFR );			/* Aggr not in frame.	*/

return;
} /* end of Verify_img_aggr */
