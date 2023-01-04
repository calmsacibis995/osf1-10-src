
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
**  IMG_DDIF_IMPORT_FRAME.C
**
**  FACILITY:
**
**	Image Services Library
**
**  ABSTRACT:
**
**	This module contains the routines for data conversion between
**	DDIF and ISL that create an in-memory ISL frame from a stream
**	of DDIF data.
**
**  ENVIRONMENT:
**
**	VAX/VMS
**
**  AUTHOR(S):
**
**	Mark W. Sornson
**
**  CREATION DATE:
**
**	17-APR-1987
**
**  MODIFICATION HISTORY:
**
************************************************************************/

/*
**  Include files:
**                                                                        
*/
#include        <img/ChfDef.h>
#include	<img/ImgDef.h>
#include	<ImgDefP.h>
#include	<ImgMacros.h>
#ifndef NODAS_PROTO
#include <imgprot.h>	    /* Img prototypes */
#endif

#if defined(__VMS) || defined(VMS)
#include	<cda$def.h>
#include	<cda$msg.h>
#include        <cda$ptp.h>
#include	<ddif$def.h>
#else
#if defined(NEW_CDA_SYMBOLS)
#include	<cdadef.h>
#include	<cdamsg.h>
#include	<ddifdef.h>
#else
#include	<cda_def.h>
#include	<cda_msg.h>
#include	<ddif_def.h>
#endif
#if defined(NEW_CDA_CALLS)
#include        <cdaptp.h>
#else
#include        <cda_ptp.h>
#endif
#endif

/*
**  Table of contents:
**
**	VMS Veneer Global Entry Points
*/
#if defined(__VMS) || defined(VMS)
long	IMG$IMPORT_DDIF_FRAME();
#endif

/*
**	Portable Global Entry Points
*/
#ifdef NODAS_PROTO
long	 ImgImportDDIFFrame();
long	_ImgImportDDIFFrame();
long	_ImgVerifySegmentType();
#endif

/*
**	Module Local Routines
*/                       
#ifdef NODAS_PROTO
static void	Copy_inherited_sga_attrs();
static long	Get_img_content();
static struct FCT *Get_img_frame();
#else
PROTO(static void Copy_inherited_sga_attrs, (CDArootagghandle /*src_root_aggr*/, CDArootagghandle /*dst_root_aggr*/, CDAagghandle /*src_seg_aggr*/, CDAagghandle /*dst_seg_aggr*/));
PROTO(static long Get_img_content, (struct FCT */*fid*/, CDArootagghandle /*user_root_aggr*/, CDArootagghandle /*frame_root_aggr*/, CDAagghandle /*frame_seg_aggr*/, DDISstreamhandle /*stream_handle*/, long /*flags*/));
PROTO(static struct FCT *Get_img_frame, (struct DCB */*dcb*/));
#endif


/*
**  MACRO definitions:
** 
**       (also see ImgMacros.h)
*/                                              

/*
**  Equated Symbols:
*/
#define	ENDOFDOC    0

/*                                
**  External References:
**
**	DDIF Toolkit Utilities are in <cdaptp.h>
*/

/*
**	From ISL
*/
#ifdef NODAS_PROTO
struct	DCB  *ImgCreateDDIFStream();	/* from IMG$DDIF_IO_MGT		    */
long	      ImgDeleteDDIFStream();	/* from IMG$DDIF_IO_MGT		    */
long	      ImgResetCtx();		/* from IMG_CONTEXT_UTILS	    */
long	      ImgStandardizeFrame();	/* from IMG_FRAME_UTILS		    */
void	      ImgVerifyFrame();

long	     _ImgCreateRootAggregate();	/* from IMG$DDIF_IO_MGT		    */
long	     _ImgFrameAlloc();		/* from IMG_FRAME_UTILS		    */
void	     _ImgFrameDealloc();	/* from IMG_FRAME_UTILS		    */
void	     _ImgGet();			/* from IMG_ATTRIBUTE_ACCESS_UTILS  */
void	     _ImgPut();			/* from IMG_ATTRIBUTE_ACCESS_UTILS  */
void	     _ImgRestoreMemoryMgt();	/* from IMG_MEMORY_MGT		    */
void	     _ImgSetFrameDataType();	/* from IMG_FRAME_UTILS		    */
void	     _ImgSetMemoryMgt();	/* from IMG_MEMORY_MGT		    */
void	     _ImgVerifyDcb();		/* from IMG_BLOCK_UTILS		    */
void	     _ImgVerifyAttributes();	/* from IMG__VERIFY_UTILS	    */
void	     _ImgVerifyDataPlanes();	/* from IMG__VERIFY_UTILS	    */
long	     _ImgVerifyNativeFormat();	/* from IMG__VERIFY_UTILS	    */
void	     _ImgVerifyStructure();	/* from IMG__VERIFY_UTILS	    */

/*
**	From VAX RTL
*/
void	    ChfSignal();		    /* signal condition handler */
void	    ChfStop();			    /* signal to stop		*/
#endif
        
/*
** External data
**
**	Global literal (or symbol) value commented out
**
*/
#include    <img/ImgStatusCodes.h>

/*
** External reference to table of default CDA processing options
*/
#if defined(__VAXC) || defined(VAXC)
globalref struct ITEMLIST_ELEMENT ImgR_DefaultOptions[];
globalref struct ITEMLIST_ELEMENT ImgR_EvaluateContentOption[];
#else
extern struct ITEMLIST_ELEMENT ImgR_DefaultOptions[];
extern struct ITEMLIST_ELEMENT ImgR_EvaluateContentOption[];
#endif
    
/*
**  Local Storage:
**
**	none
*/


/******************************************************************************
**  IMG$IMPORT_DDIF_FRAME
**  ImgImportDdifFrame
**
**  FUNCTIONAL DESCRIPTION:
**
**      Create an ISL image frame from a DDIF stream.  Using a DDIF stream
**	that contains at least on image frame, this routine finds the
**	first image frame that was not imported to ISL, and imports it.
**
**	Repeated calls to this routine using the same DDIF stream will
**	import successive image frames from the stream until there are
**	no more frames.
**
**  FORMAL PARAMETERS:
**
**	sid		stream id, by value, optional
**			Stream id created by a user-call to 
**			IMG$CREATE_DDIF_STREAM.  A stream-id must be 
**			established externally if multiple frames are
**			to be imported from the same DDIF stream.
**			This parameter may be omitted if a complete
**			DDIF document stream containing ONE image frame
**			only is used, or if the caller only want the FIRST
**			image in the stream to be imported, thereby 
**			circumventing the need for the caller to use calls
**			to IMG$CREATE_DDIF_STREAM and IMG$DELETE_DDIF_STREAM.
**
**	bufadr		buffer address, by reference, optional
**			Address of buffer from which the imported DDIF data
**			is taken.  If omitted, a buffer will be allocated
**			internally, using the value of the buflen parameter.
**			If the buffer is too small to hold the entire DDIF
**			stream, it will be filled by the action routine
**			(which must be supplied in this case).  If this
**			parameter is omitted, an action routine MUST be
**			supplied.  If present, buflen must also be specified.
**
**			NOTE that this parameter MUST be omitted if it was
**			passed into IMG$CREATE_DDIF_STREAM.
**
**	buflen		buffer length, by value, optional
**			Length in bytes of the buffer passed in as bufadr.
**			If bufadr is omitted, buflen will be used as the
**			size of the internal buffer to be allocated.  If
**			both bufadr and buflen are omitted, a default
**			length value (DcbK_DefIobLen) is used.
**			
**			NOTE that this parameter MUST be omitted if it was
**			passed into IMG$CREATE_DDIF_STREAM.
**
**	flags		flags, by value, optional
**			(Values to be supplied.)
**
**	action		action routine, by reference, optional
**			Action routine to be called to fill the buffer used
**			to hold the DDIF-stream data.  The action routine
**			will be called when the buffer is empty (and more
**			data has yet to be read from the DDIF stream). If 
**			the bufadr parameter is omitted, an action routine
**			MUST be specified.
**
**	user_param	action routine parameter, by value, optional
**			This parameter is passed directly to the action
**			routine and is not interpreted by the DDIF processing
**			code in any way.
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
**      fid		Returns value of fid created by the import process.
**			A value of zero will be returned if no image data
**			was found in the DDIF stream.
**
**  SIGNAL CODES:
**
**      ImgX_INVARGCNT  Invalid argument count.
**	ImgX_BUFWNOLEN	Buffer address was given with no buffer length.
**	ImgX_PARAMCONF	Parameter conflict -- when a stream-id was given
**			at least on of the other args (aside from FLAGS)
**			was not zero or null.
**
**  SIDE EFFECTS:
**                                                     
**      none
**
******************************************************************************/
#if defined(__VMS) || defined(VMS)
long IMG$IMPORT_DDIF_FRAME( sid, bufadr, buflen, flags, action, user_param )
struct	DCB	 *sid;			/* stream id (doc ctx blk)	*/
char		 *bufadr;		/* IO buf address, optional	*/
long		  buflen;		/* IO buf len, optional		*/
long		  flags;		/* flags, optional		*/
long		(*action)();		/* user IO action rtn, optional	*/
long		  user_param; 		/* param to action, optional	*/
{

return (ImgImportDDIFFrame( sid, bufadr, buflen, flags, action, user_param ));
} /* end of IMG$IMPORT_DDIF_FRAME */
#endif


/*******************************************************************************
** Portable entry point
*******************************************************************************/
struct FCT *ImgImportDDIFFrame( sid, bufadr, buflen, flags, action, user_param )
struct	DCB	 *sid;			/* stream id (doc ctx blk)	*/
char		 *bufadr;		/* IO buf address, optional	*/
long		  buflen;		/* IO buf len, optional		*/
long		  flags;		/* flags, optional		*/
long		(*action)();		/* user IO action rtn, optional	*/
long		  user_param; 		/* param to action, optional	*/
{
long		  compression_type;
long		  comp_space_org;
long		  data_class;
struct FCT *fid			= 0;	/* frame id		*/
long		  img_data_found;		/* (processing flag)	*/
long		  status;
struct	DCB	 *dcb			= sid;	/* ddif doc context block */

if ( bufadr != NULL && buflen == 0 )
    ChfStop( 1,  ImgX_BUFWNOLEN ); /* buf adr with no length */

/*
** Set up document context block, creating one if one was not
** passed in.  NOTE that the document context block (dcb) adr and 
** the stream id (sid) value are synonyms.
*/
if ( sid == 0 )				    /* Was the stream id omitted? */
    {					    /* Yes. */
    /*
    ** Open a DDIF stream: note that the validity of the bufadr, buflen
    ** and action parameters will be checked by IMG$CREATE_DDIF_STREAM.
    ** Also, indicate that no stream id was passed in, which means that
    ** the DDIF stream will be automatically close when this routine finishes.
    */                 
    dcb = ImgCreateDDIFStream( DcbK_ModeRead, bufadr, buflen,
				flags, action, user_param);
    dcb->DcbL_Flags.DcbV_NoSid = TRUE;	
    }
else
    {					    /* No. */
    /*
    ** Since the stream id was passed in, we assume that this is
    ** repeated call mode, which means that IMG$IMPORT_DDIF_FRAME
    ** may be called more than once to extract successive image
    ** frames from the DDIF stream.
    **
    ** Note that the following arguments must be zero or omitted,
    ** having been established when the stream was opened:
    **
    **		    buflen, bufadr, action, user_param
    */
    if ( bufadr != NULL || 
         buflen != 0 || 
         action != NULL || 
         user_param != 0 )
        ChfStop( 1,  ImgX_PARAMCONF );         /* parameter conflict   */
    
    /*
    ** Verify the validity of the DCB passed in as the SID parameter.
    */
    _ImgVerifyDcb( dcb, DcbK_ModeRead, 0 );
    }             

/*
** Set internal process flags that correspond to the flags in
** the flags arguement.
*/
if ( flags != (long) NULL )
    {
    if ((flags & ImgM_SkipFrame) != FALSE )	    /* skip the next frame? */
    	dcb->DcbL_Flags.DcbV_SkipFrame = TRUE;
    }                     

/*
** Get the image frame data.  Note that if the end-of-stream was detected,
** no attempt will be made to create and return a frame.  Therefore, a
** null frame will be returned.
*/
if ( dcb->DcbL_Flags.DcbV_Eos != TRUE )
    {
    /*
    ** This drives the import operation.
    */
    fid = Get_img_frame( dcb );
    }

/*
** Close the stream if it was temporary.
*/
if ( sid == 0 )
    {
      ImgDeleteDDIFStream( dcb );
    }

if ( !VERIFY_OFF )
    ImgVerifyFrame( fid, ImgM_NonstandardVerify );

return fid;
} /* end of ImgImportDDIFFrame */
                  

/******************************************************************************
**  _ImgImportDDIFFrame
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine actually imports the ISL image data into the ISL/DDIF
**	in-memory environment.
**
**	This routine is also a "back door" routine for sophisticated
**	applications that are doing their own DDIF IO (e.g. compound
**	document processing).  This routine will allow applications
**	-- that have already established a DDIF stream environment for
**	themselves using DDIF Toolkit routines -- to read a single
**	ISL image frame into a DDIF memory format from an existing stream.
**
**	This routine assumes that a image frame BeginSegment has already
**	been found.
**
**  FORMAL PARAMETERS:
**
**	user_seg_aggr	Segment aggregate handle of an image frame that was
**			taken from an input DDIF stream.  A copy of this 
**			segment will be stored in the frame.
**			Longword unsigned.
**			Passed by VALUE (not by REFERENCE).
**
**	user_root_aggr	Root aggregate handle associated with the input
**			DDIF stream.
**			Longword unsigned.
**			Passed by VALUE (not by REFERENCE).
**
**	stream_handle	Stream handle associated with the input DDIF stream.
**			Longword unsigned.
**			Passed by VALUE (not by REFERENCE).
**
**	FLAGS		Processing flags.  None defined.  This argument
**			is optional.
**			Longword Unsigned.
**			Passed by VALUE.
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
**	Frame id	Frame id of the created ISL image frame.
**			Longword unsigned.
**			Passed by value.
**
**  SIGNAL CODES:
**
**	ImgX_NOTIMGSEG	Segment aggr passed in was not a DDIF$_SEG aggr.
**
**  SIDE EFFECTS:
**
**	none.
**
******************************************************************************/
struct FCT *_ImgImportDDIFFrame( user_seg_aggr, user_root_aggr, stream_handle, 
			    flags )
CDAagghandle user_seg_aggr;
CDArootagghandle user_root_aggr;
DDISstreamhandle stream_handle;
long	flags;
{
struct FCT *fid;
long	 img_content_found	= FALSE;
long	 img_frame_verify;
CDArootagghandle frame_root_aggr;
CDAagghandle frame_seg_aggr;
long	 memctx;
int	 status;
struct ITEMLIST_ELEMENT *options	= ImgR_DefaultOptions;

/*
** Process flags
*/
if ( (flags&ImgM_Noinheritance) != FALSE )
    options = ImgR_EvaluateContentOption;

/*
** Just to be sure, verify that the segment-aggregate passed in 
** actually is an image-frame segment.
*/
img_frame_verify = _ImgVerifySegmentType( 
			user_root_aggr,
#if defined(NEW_CDA_SYMBOLS)
			user_seg_aggr, DDIF_SEG );
#else
			user_seg_aggr, DDIF$_SEG );
#endif
if ( img_frame_verify == FALSE )
    ChfStop( 1,  ImgX_NOTIMGSEG );	    /* not an IMAGE segment */

/*
** Change the memory mgt routines used by the stream-root to the
** ISL memory mgt routines.
*/
_ImgSetMemoryMgt( user_root_aggr, (void **)&memctx );

/*
** Create a new frame.  First create the frame root, and then copy the
** user DDIF$_SEG aggregate.  Use the root and the new seg aggr to create
** the frame.  (NOTE that the calling routine should dispose of the
** segment aggr that was passed in to this routine, since it is no longer
** needed.)
**
** If attributes are inherited from the some higher segment, they are lost
** by the copy aggregate operation.  Therefore, unless inheritance is
** explicitly disabled, inherited attributes will have to be copied from
** the source SGA aggr into the new one.
*/
frame_root_aggr = _ImgCreateRootAggregate( options );
status = CDA_COPY_AGGREGATE_(
	    &frame_root_aggr,
	    &user_seg_aggr,
	    &frame_seg_aggr );
LOWBIT_TEST_( status );
Copy_inherited_sga_attrs( user_root_aggr, frame_root_aggr, user_seg_aggr, 
    frame_seg_aggr );
fid = _ImgFrameAlloc( frame_root_aggr, frame_seg_aggr );

/*
** Get the DDIF$_IMG content elements (if there any, which there should be).
*/
img_content_found = Get_img_content( fid, user_root_aggr, frame_root_aggr, 
					frame_seg_aggr, stream_handle, flags );

if ( img_content_found)		/* Image content found? */
    {				/* Yes.	*/
    /*
    ** Reset the frame context, setting the frame context block to
    ** point to the first content element (or cell), to the first data
    ** unit (or data plane) in the cell, and to the first lookup table
    ** (if there is one).
    */
    ImgResetCtx( fid );
    
    /*
    ** Set the spectral type attribute of the frame to indicate
    ** whether it is Bitonal, Greyscale, or Multispectral.
    */
    _ImgSetFrameDataType( fid );

    }
else
    {						/* No. */
    /*
    ** Deallocate the frame and set the return pointer to NULL.
    */
    _ImgFrameDealloc( fid );
    fid = (long) NULL;
    }

/*
** Restore memory mgt context used by user root.
*/
_ImgRestoreMemoryMgt( user_root_aggr, (void **)&memctx );

return fid;
} /* end of _ImgImportDDIFFrame */


/******************************************************************************
**  _ImgVerifySegmentType
**
**  FUNCTIONAL DESCRIPTION:
**
**	Verify that a begin segment aggregate (DDIF$_SEG) is a valid 
**	image-frame identifier.  This will distinguish between image
**	frame segments and segments that do not contain image data.
**
**  FORMAL PARAMETERS:
**
**	root_aggr	Root aggregate handle associated with input DDIF
**			stream.
**			Longword unsigned.
**			Passed by VALUE (not by REFERENCE)
**	seg_aggr	Aggregate handle of DDIF$_SEG to verify as an
**			image frame.
**			Longword unsigned.
**			Passed by VALUE (not by REFERENCE).
**	AGGR_TYPE	Aggregate type code of DDIF$_SEG aggregate
**			to be verified.  This parameter is optional.
**			NOTE that if not supplied, the aggregate must
**			be the right type.
**			Longword unsigned.
**			Passed by VALUE (not by REFERENCE).
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
**	img_seg_found	Returns a value of TRUE (1) or FALSE (0).
**			Longword unsigned.
**			Passed by value.
**
**  SIGNAL CODES:
**
**	ImgX_AGGNOTSEG	    Aggregate type was not DDIF$_SEG
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
long _ImgVerifySegmentType( root_aggr, seg_aggr, aggr_type )
CDArootagghandle root_aggr;
CDAagghandle seg_aggr;
long aggr_type;
{
char	*content_category_tag_string;
CDAconstant add_info;
long	 aggr_index;
long	 argcnt;
long	 img_seg_found = FALSE;
CDAaddress item;
long	 length;
CDAagghandle sga_handle;
CDAconstant aggregate_item;
long	 status;

/*
** Verify that the aggregate type is DDIF$_SEG, which means that
** the segment is a BeginSegment (containing image presentation attributes
** and image component space attributes).
*/
#if defined(NEW_CDA_SYMBOLS)
    if ( aggr_type != DDIF_SEG )
#else
    if ( aggr_type != DDIF$_SEG )
#endif
	ChfStop( 1,  ImgX_AGGNOTSEG );	/* aggr not a segment	*/

/*
** Locate the specific segment attributes.  NOTE that if there aren't
** any, this means that the segment is definitely NOT an image segment.
*/
#if defined(NEW_CDA_SYMBOLS)
aggregate_item = DDIF_SEG_SPECIFIC_ATTRIBUTES;
#else
aggregate_item = DDIF$_SEG_SPECIFIC_ATTRIBUTES;
#endif
status = CDA_LOCATE_ITEM_(
	    &root_aggr,
	    &seg_aggr,
	    &aggregate_item,
	    &item,
	    &length, 0, 0 );
switch ( status )
    {
#if defined(NEW_CDA_SYMBOLS)
    case CDA_NORMAL:
#else
    case CDA$_NORMAL:
#endif
    /* 
    ** Add default case too because it can also happen.
    ** Why? I don't know. - (Nice commenting)
    */
#if defined(NEW_CDA_SYMBOLS)
    case CDA_DEFAULT:
#else
    case CDA$_DEFAULT:
#endif
	sga_handle = *(CDAagghandle *)item;
	/*
	** Locate the content category tag and see if it contains the
	** image content tag string "$I".  (This is determined by looking
	** at the value returned as add_info.)
	*/
#if defined(NEW_CDA_SYMBOLS)
        aggregate_item = DDIF_SGA_CONTENT_CATEGORY;
#else
        aggregate_item = DDIF$_SGA_CONTENT_CATEGORY;
#endif
	status = CDA_LOCATE_ITEM_(
	        &root_aggr,
		&sga_handle,
	        &aggregate_item,
		&item,
	        &length,
		0,
	        &add_info );
	switch ( status )
	    {
#if defined(NEW_CDA_SYMBOLS)
	    case CDA_NORMAL:
	    case CDA_DEFAULT:
#else
	    case CDA$_NORMAL:
	    case CDA$_DEFAULT:
#endif
		content_category_tag_string = (char *)item;
#if defined(NEW_CDA_SYMBOLS)
/* pjw - gross hack to workaround CDA bug. add_info is 
 * supposed to be a constant between approx. 0 and 6. 
 * It's being returned here with the high order bit set!
 * Cast it to an int to nuke this bit and remember to file
 * a qar!
 */
		if ( (int) add_info == DDIF_K_I_CATEGORY )
#else
		if ( (int) add_info == DDIF$K_I_CATEGORY )
#endif
		    img_seg_found = TRUE;	/* we have an image segment */
		break;
#if defined(NEW_CDA_SYMBOLS)
	    case CDA_EMPTY:
#else
	    case CDA$_EMPTY:
#endif
		break;
	    default:
		ChfStop( 1,  status );
	    }

#if defined(NEW_CDA_SYMBOLS)
    case CDA_EMPTY:
#else
    case CDA$_EMPTY:
#endif
	break;
    default:
	ChfStop( 1,  status );
    }

return img_seg_found;
} /* end of _ImgVerifySegmentType */


/******************************************************************************
**  Copy_inherited_sga_attrs
**
**  FUNCTIONAL DESCRIPTION:
**
**	Copy the inherited attributes from a source SGA aggr into a 
**	destination SGA, both of which are attached to parent SEG aggrs.
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
static void Copy_inherited_sga_attrs( src_root_aggr, dst_root_aggr, 
		src_seg_aggr, dst_seg_aggr )
CDArootagghandle src_root_aggr;
CDArootagghandle dst_root_aggr;
CDAagghandle src_seg_aggr;
CDAagghandle dst_seg_aggr;
{
int	 array_index;
long	 array_size;
CDAconstant ddif_item_code;
CDAagghandle dst_sga_aggr;
CDAaddress item_addr;
CDAconstant item_code;
long	 item_index	    = 0;
long	 item_length;
CDAconstant	 item_value;
CDAagghandle src_sga_aggr;
int	 status;
long	 table_array_count;
long	 table_array_index;
int	 zero_index	    = 0;

#if defined(__VAXC) || defined(VAXC)
readonly
#endif
#if defined(NEW_CDA_SYMBOLS)
static int itemcode_table[]	= {  DDIF_SGA_IMG_PIXEL_PATH
					    ,DDIF_SGA_IMG_LINE_PROGRESSION
					    ,DDIF_SGA_IMG_PP_PIXEL_DIST
					    ,DDIF_SGA_IMG_LP_PIXEL_DIST
					    ,DDIF_SGA_IMG_BRT_POLARITY
					    ,DDIF_SGA_IMG_GRID_TYPE
					    ,DDIF_SGA_IMG_SPECTRAL_MAPPING
					    ,DDIF_SGA_IMG_LOOKUP_TABLES_C
					    ,DDIF_SGA_IMG_LOOKUP_TABLES
					    ,DDIF_SGA_IMG_COMP_WAVELENGTH_C
					    ,DDIF_SGA_IMG_COMP_WAVELENGTH
					    ,DDIF_SGA_IMG_COMP_SPACE_ORG
					    ,DDIF_SGA_IMG_PLANES_PER_PIXEL
					    ,DDIF_SGA_IMG_PLANE_SIGNIF
					    ,DDIF_SGA_IMG_NUMBER_OF_COMP
					    ,DDIF_SGA_IMG_BITS_PER_COMP
					    ,DDIF_SGA_IMG_PIXEL_GRP_SIZE
					    ,DDIF_SGA_IMG_PIXEL_GRP_ORDER
					    ,DDIF_SGA_IMG_COMP_QUANT_LEVELS
					    };
#else
static int itemcode_table[]	= {  DDIF$_SGA_IMG_PIXEL_PATH
					    ,DDIF$_SGA_IMG_LINE_PROGRESSION
					    ,DDIF$_SGA_IMG_PP_PIXEL_DIST
					    ,DDIF$_SGA_IMG_LP_PIXEL_DIST
					    ,DDIF$_SGA_IMG_BRT_POLARITY
					    ,DDIF$_SGA_IMG_GRID_TYPE
					    ,DDIF$_SGA_IMG_SPECTRAL_MAPPING
					    ,DDIF$_SGA_IMG_LOOKUP_TABLES_C
					    ,DDIF$_SGA_IMG_LOOKUP_TABLES
					    ,DDIF$_SGA_IMG_COMP_WAVELENGTH_C
					    ,DDIF$_SGA_IMG_COMP_WAVELENGTH
					    ,DDIF$_SGA_IMG_COMP_SPACE_ORG
					    ,DDIF$_SGA_IMG_PLANES_PER_PIXEL
					    ,DDIF$_SGA_IMG_PLANE_SIGNIF
					    ,DDIF$_SGA_IMG_NUMBER_OF_COMP
					    ,DDIF$_SGA_IMG_BITS_PER_COMP
					    ,DDIF$_SGA_IMG_PIXEL_GRP_SIZE
					    ,DDIF$_SGA_IMG_PIXEL_GRP_ORDER
					    ,DDIF$_SGA_IMG_COMP_QUANT_LEVELS
					    };
#endif
#if defined(__VAXC) || defined(VAXC)
readonly 
#endif
static long itemcode_table_size    = sizeof( itemcode_table );

/*
** Locate the src SGA aggr and the dst SGA aggr.
*/
#if defined(NEW_CDA_SYMBOLS)
item_code = DDIF_SEG_SPECIFIC_ATTRIBUTES;
#else
item_code = DDIF$_SEG_SPECIFIC_ATTRIBUTES;
#endif
status = CDA_LOCATE_ITEM_(
	    &src_root_aggr,
	    &src_seg_aggr,
	    &item_code,
	    &item_addr,
	    &item_length,
	    0, 0 );
LOWBIT_TEST_( status );
src_sga_aggr = *(CDAagghandle *)item_addr;

status = CDA_LOCATE_ITEM_(
	    &dst_root_aggr,
	    &dst_seg_aggr,
	    &item_code,
	    &item_addr,
	    &item_length,
	    0, 0 );
LOWBIT_TEST_( status );
dst_sga_aggr = *(CDAagghandle *)item_addr;

/*
** Loop through the src SGA aggr items, filling in items in the dst that are
** present but inherited in the src.  Since all the DDIF$_ item codes for image
** segment attributes are consequetively numbered, simply start at the first
** one and increment the item code until the last one has been checked.
*/
table_array_count = itemcode_table_size / sizeof( long );
for ( table_array_index = 0; 
	table_array_index < table_array_count; 
	    ++table_array_index )
    {
    ddif_item_code = itemcode_table[table_array_index];

    status = CDA_LOCATE_ITEM_(
		&src_root_aggr,
		&src_sga_aggr,
		&ddif_item_code,
		&item_addr,
		&item_length,
		&zero_index, 
		0 );
    switch( status )
	{
#if defined(NEW_CDA_SYMBOLS)
	case CDA_DEFAULT:
#else
	case CDA$_DEFAULT:
#endif
	    /*
	    ** The default case means the attribute was defaulted or
	    ** inherited.
	    */
#if defined(NEW_CDA_SYMBOLS)
	    if ( ddif_item_code != DDIF_SGA_IMG_BITS_PER_COMP &&
		 ddif_item_code != DDIF_SGA_IMG_COMP_QUANT_LEVELS )
#else
	    if ( ddif_item_code != DDIF$_SGA_IMG_BITS_PER_COMP &&
		 ddif_item_code != DDIF$_SGA_IMG_COMP_QUANT_LEVELS )
#endif
		{
		status = CDA_STORE_ITEM_(
			&dst_root_aggr,
			&dst_sga_aggr,
			&ddif_item_code,
			&item_length,
			item_addr,
			&zero_index, 
			0 );
		LOWBIT_TEST_(status);
		}
	    else
		{
		/*
		** The item is either the bits per comp attribute or the
		** quant levels per comp attribute, both of which are
		** array valued.
		*/
		status = CDA_GET_ARRAY_SIZE_(	&src_sga_aggr,
						&ddif_item_code,
						&array_size );
		LOWBIT_TEST_(status);

		for ( array_index = 0; array_index < array_size; ++array_index )
		    {
		    /*
		    ** Locate the item
		    */
		    status = CDA_LOCATE_ITEM_(
				&src_root_aggr,
				&src_sga_aggr,
				&ddif_item_code,
				&item_addr,
				&item_length,
				&array_index, 
				0 );

		    /*
		    ** Store the value
		    */
		    status = CDA_STORE_ITEM_(
				&dst_root_aggr,
				&dst_sga_aggr,
				&ddif_item_code,
				&item_length,
				item_addr,
				&array_index, 
				0 );
		    LOWBIT_TEST_(status);

		    } /* end for */
		}
	    break;
	default:
	    /*
	    ** Skip every other case
	    */
	    break;
	} /* end switch */
    } /* end for */

return;
} /* end of Copy_inherited_sga_attrs */


/******************************************************************************
**  Get_img_content
**
**  FUNCTIONAL DESCRIPTION:
**
**	Get the image content of a frame.  This function reads and stores
**	all the DDIF$_IMG and DDIF$_IDU aggregates of an image frame as
**	an ISL image frame.
**
**  FORMAL PARAMETERS:
**
**	fid		Frame-id to attach content to.
**			Passed by value.
**
**	user_root_aggr	Root aggregate owned by calling frame associated
**			with the input stream.
**			Passed by value.
**
**	frame_root_aggr	Root aggregate associated with the frame.
**			Passed by value.
**
**	frame_seg_aggr	DDIF$_SEG aggregate associated with the frame.
**			Passed by value.
**
**	stream_handle	Stream handle associated with the input stream.
**			Passed by value.
**
**	flags		Flags.  None defined.
**			Passed by value.
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
**	img_content_found	Boolean, True (1) or False (0)
**				passed by value
**
**  SIGNAL CODES:
**
**	ImgX_FRAMIMPER	    Frame import error, passes frame-id as FAO arg.
**			    Caused by a bad status returned from a CDA call.
**			    Fid passed to condition handler should be deleted
**			    by the application.
**
**  SIDE EFFECTS:
**
**	none
**
**  CALLED BY:
**
**	_ImgImportDDIFFrame
******************************************************************************/
static long Get_img_content( fid, user_root_aggr, frame_root_aggr, 
			    frame_seg_aggr, stream_handle, flags )
struct FCT *fid;
CDArootagghandle user_root_aggr;
CDArootagghandle frame_root_aggr;
CDAagghandle frame_seg_aggr;
DDISstreamhandle stream_handle;
long	flags;
{
CDAagghandle	aggregate_handle;
CDAconstant	aggregate_item;
int	aggregate_type;
long	get_next_aggregate	= TRUE;
long	ice_count		= 0;
long	img_content_found	= FALSE;
long	item_size;
CDAagghandle prev_aggr_handle;
int	status;

/*
** Get image data units.
*/
while ( get_next_aggregate )
    {
    status = CDA_GET_AGGREGATE_(
		&user_root_aggr,		/* NOTE that we GET the	*/
		&stream_handle,			/* aggr with the USER	*/
		&aggregate_handle,		/* root, but store it	*/
		&aggregate_type );		/* with the frame root.	*/
    if ( (status&1) != 1 )
	ChfStop( 4,  ImgX_FRAMIMPER, 1, fid, status );

    switch ( aggregate_type )
    	{
#if defined(NEW_CDA_SYMBOLS)
    	case DDIF_IMG:
#else
    	case DDIF$_IMG:
#endif
	    /*
	    ** Note that Getting the DDIF$_IMG aggregate gets ALL
	    ** the DDIF$_IDU aggregates attached to it.  (Yahoo!)
	    */
	    if (ice_count == 0)
		/*
		** "Store" the first DDIF$_IMG aggregate as an item
		** of the DDIF$_SEG aggregate.
		*/
		{
#if defined(NEW_CDA_SYMBOLS)
                aggregate_item = DDIF_SEG_CONTENT;
#else
                aggregate_item = DDIF$_SEG_CONTENT;
#endif
                item_size = sizeof(aggregate_handle);
		status = CDA_STORE_ITEM_(
			    &frame_root_aggr,
			    &frame_seg_aggr,
			    &aggregate_item,
			    &item_size,
			    &aggregate_handle,
			    0, 0 );
		if ( (status&1) != 1 )
		    ChfStop( 4,  ImgX_FRAMIMPER, 1, fid, status );
		prev_aggr_handle = aggregate_handle;
		}
	    else
		/*
		** Insert all subsequent DDIF$_IMG aggregates as elements
		** of a sequence of DDIF$_IMG aggregates, with each new
		** aggregate following the previous aggregate.
		*/
		{
		status = CDA_INSERT_AGGREGATE_(
			    &aggregate_handle,
			    &prev_aggr_handle );
		if ( (status&1) != 1 )
		    ChfStop( 4,  ImgX_FRAMIMPER, 1, fid, status );
		prev_aggr_handle = aggregate_handle;
		}
	    ++ice_count;		    /* count the ICEs		*/
            break;
#if defined(NEW_CDA_SYMBOLS)
    	case DDIF_EOS:
#else
    	case DDIF$_EOS:
#endif
            get_next_aggregate = FALSE;
            break;
    	default:			    /* skip non-img aggrs	*/
	    break;			    /* though there shouldn't	*/
					    /* be any.			*/
    	} /* end of switch */
    } /* end of while loop */

/*
** Store the image content element count.
*/
_ImgPut( fid, Img_Icecnt, &ice_count, sizeof(ice_count), 0 );

if ( ice_count > 0 )
    img_content_found = TRUE;

return img_content_found;
} /* end of Get_img_content */


/******************************************************************************
**  Get_img_frame
**
**  FUNCTIONAL DESCRIPTION:
**
**	Scan through the DDIF stream, getting aggregates until a complete
**	image frame has been found.  This function is the principle control
**	loop for this process.
**
**  FORMAL PARAMETERS:
**
**	dcb	    Document context block.
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
**	fid		    Frame-id of imported frame.
**			    Passed by value.
**
**  SIGNAL CODES:
**
**	ImgX_DDIFIMPER	    DDIF data import error.  Something went wrong
**			    with an attempt to get an aggregate from the file
**			    or stream.
**
**	ImgX_ILLIMGSEG	    Illegal image segment found in stream 
**			    Out of place.  Probably a semantic problem.
**	ImgX_INVARGSEQ	    Invalid document descriptor or document header
**			    found it the stream.  Out of place.  Probably
**			    a semantic problem.
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
static struct FCT *Get_img_frame( dcb )
struct 	DCB	*dcb;
{
CDAagghandle	 aggregate_handle;
int	 aggregate_type;
long	 get_next_aggr	    = TRUE;
long	 flags		    = ImgM_IslPrvtSeg;
long	 img_data_found	    = FALSE;
long	 img_frame_found    = FALSE;
CDAconstant	item_code;
CDAaddress	item_value_addr;
CDAconstant	item_value;
long	 length;
int	 status;
long	true		    = TRUE;

/*
** Loop, looking for an image beginsegment.
*/          
while ( get_next_aggr )
    {
    status = CDA_GET_AGGREGATE_( 
		    &(dcb->DcbL_RootAggr),
		    &(dcb->DcbL_StrmCtx),
		    &aggregate_handle,
		    &aggregate_type
		    );
    switch ( status )			    /* check status to	    */
	{				    /* see if we should	    */
                         		    /* continue processing, */
#if defined(NEW_CDA_SYMBOLS)
	case CDA_NORMAL:
#else
	case CDA$_NORMAL:
#endif
	    break;			    /* return with no	    */
					    /* data, or signal an   */
#if defined(NEW_CDA_SYMBOLS)
	case CDA_ENDOFDOC:
#else
	case CDA$_ENDOFDOC:
#endif
	    get_next_aggr = FALSE;	    /* error.		    */
            dcb->DcbL_Flags.DcbV_Eos = TRUE;
	    dcb->DcbL_Fid = 0;
	    aggregate_type = ENDOFDOC;
	    break;
	default:
	    ChfStop( 3,  ImgX_DDIFIMPER, 0, status );
	} /* end of switch */

    switch ( aggregate_type )
	{
	case ENDOFDOC:
	    /*
	    ** End of doc condition.  Do nothing.  The loop will exit
	    ** as of the next test, since get_next_aggr will have
	    ** been set to false.
	    */
	    break;
#if defined(NEW_CDA_SYMBOLS)
	case DDIF_HRD:
	case DDIF_SFT:
#else
	case DDIF$_HRD:
	case DDIF$_SFT:
#endif
	    /*
	    ** A directive was found.  Find out if it was a new page
	    ** directive.
	    */
#if defined(NEW_CDA_SYMBOLS)
	    item_code = aggregate_type == DDIF_HRD? DDIF_HRD_DIRECTIVE:
					    DDIF_SFT_DIRECTIVE;
#else
	    item_code = aggregate_type == DDIF$_HRD? DDIF$_HRD_DIRECTIVE:
					    DDIF$_SFT_DIRECTIVE;
#endif
	    length = sizeof( item_value );
	    status = CDA_LOCATE_ITEM_(
			&(dcb->DcbL_RootAggr),
			&aggregate_handle,
			&item_code,
			&item_value_addr,
			&length,
			0,0 );
	    LOWBIT_TEST_( status );
	    item_value = *(CDAconstant *)item_value_addr;

	    /*
	    ** Set a flag indicating a page break was found.
	    */
#if defined(NEW_CDA_SYMBOLS)
	    if ( item_value == DDIF_K_DIR_NEW_PAGE )
#else
	    if ( item_value == DDIF$K_DIR_NEW_PAGE )
#endif
		{
		dcb->DcbL_Flags.DcbV_PageBreak = TRUE;
#if defined(NEW_CDA_SYMBOLS)
		if (aggregate_type == DDIF_SFT )
#else
		if (aggregate_type == DDIF$_SFT )
#endif
		    dcb->DcbL_Flags.DcbV_SoftPageBreak = TRUE;
		}

	    /*
	    ** Either pass the aggregate along to a user action routine
	    ** or delete it.
	    */
	    if ( dcb->DcbA_AggrRtn != 0 )
		{
		/*
		** Call the application action routine to handle the
		** directive.
		*/
		status = (*(dcb->DcbA_AggrRtn))(
			    dcb->DcbL_RootAggr,
			    dcb->DcbL_StrmCtx,
			    aggregate_handle,
			    aggregate_type,
			    dcb->DcbL_AggrRtnParam );
		LOWBIT_TEST_( status );
		}
	    else
		{
		/*
		** Get rid of the aggregate.
		*/
		status = CDA_DELETE_AGGREGATE_(
			    &(dcb->DcbL_RootAggr),
			    &aggregate_handle );
		LOWBIT_TEST_( status );
		}

	    break;
#if defined(NEW_CDA_SYMBOLS)
	case DDIF_SEG:
#else
	case DDIF$_SEG:
#endif
            img_frame_found = _ImgVerifySegmentType( 
				dcb->DcbL_RootAggr,
				aggregate_handle,
				aggregate_type );
	    /*
	    ** If this is an image frame segment, then import the frame.
	    */
	    if ( img_frame_found )
		{
		/*
		** Should the image data be imported and a frame be
		** created, or should it be skipped?
		*/
		if ( dcb->DcbL_Flags.DcbV_SkipFrame == FALSE )
		    {
		    /*
		    ** Import the image frame data.  NOTE that this routine
		    ** actually makes a private copy of the DDIF$_SEG aggr.
		    */
		    dcb->DcbL_Flags.DcbV_SkipImgAggr = FALSE;
		    dcb->DcbL_Fid = (struct FCT *) _ImgImportDDIFFrame( 
					aggregate_handle,
					dcb->DcbL_RootAggr,
					dcb->DcbL_StrmCtx,
					flags );

		    /*
		    ** If there was a page break preceding this frame,
		    ** set frame Img_PageBreak attribute to TRUE
		    */
		    if ( dcb->DcbL_Fid != 0 && 
			 dcb->DcbL_Flags.DcbV_PageBreak )
			{
			long	type	= ImgK_HardPageBreak;
			if ( dcb->DcbL_Flags.DcbV_SoftPageBreak )
			    type = ImgK_SoftPageBreak;
			_ImgPut( dcb->DcbL_Fid, Img_PageBreak, &type,
			    sizeof( type ), 0 );
			dcb->DcbL_Flags.DcbV_PageBreak = FALSE;
			dcb->DcbL_Flags.DcbV_SoftPageBreak = FALSE;
			}

		    /*
		    ** All done.  Set loop condition to false to terminate.
		    */
		    if (dcb->DcbL_Fid != 0)
			get_next_aggr = FALSE;
		    }
		else
		    {
		    /*
		    ** Skip the image frame.  Note that the SKIP_IMGAGGR
		    ** flag is used on the next call to read an image frame,
		    ** since it is at that point that the image segment is
		    ** read.  The SKIP_FRAME flag is cleared.  Note that
		    ** skipping the frame returns a ZERO for the frame-id.
		    */
		    dcb->DcbL_Flags.DcbV_SkipImgAggr = TRUE;
		    dcb->DcbL_Flags.DcbV_SkipFrame = FALSE;
		    dcb->DcbL_Fid = 0;
		    get_next_aggr = FALSE;
		    }
		/*
		** Delete the original DDIF$_SEG aggregate since there
		** will be a copy of it in the frame if it was read,
		** and it won't be needed if the frame was skipped.
		*/
		status = CDA_DELETE_AGGREGATE_(
			    &(dcb->DcbL_RootAggr),
			    &aggregate_handle );
		LOWBIT_TEST_( status );
		}
	    /*
	    ** There's always the possibility that this is NOT an
	    ** image frame segment.
	    */
	    else
		{
		/*
		** This could be a segment aggregate that the application
		** knows about.  Therefore, call the aggregate action
		** routine if there is one.
		*/
		if ( dcb->DcbA_AggrRtn != 0 )
		    {
		    status = (*(dcb->DcbA_AggrRtn))(
				dcb->DcbL_RootAggr,
				dcb->DcbL_StrmCtx,
				aggregate_handle,
				aggregate_type,
				dcb->DcbL_AggrRtnParam );
		    LOWBIT_TEST_( status );
		    }
		else
		    {
		    /*
		    ** Since there's no action routine to handle it,
		    ** delete it.
		    */
		    status = CDA_DELETE_AGGREGATE_(
				&(dcb->DcbL_RootAggr),
				&aggregate_handle );
		    LOWBIT_TEST_( status );
		    }
		}
	    break;

	/*
	** Aggregates of type DDIF$_IMG, DDIF$_DSC, & DDIF$_DHD should
	** not ordinarily be found at this point.  (DDIF$_IMG aggregates are 
	** supposed to be found by calls to _ImgImportDDIFFrame.)  The
	** one exception to this rule is when a previous call to IMG$IMPORT_
	** DDIF_FRAME was passed the skip frame flag.  This means that the
	** previous read terminated at the DDIF$_SEG aggregate, but did not
	** read the DDIF$_IMG aggregate that followed.  Note that if the
	** skip flag was set, every DDIF$_IMG aggregate in the stream will be
	** skipped until the next image frame DDIF$_SEG aggr is found.
	*/
#if defined(NEW_CDA_SYMBOLS)
	case DDIF_IMG:
#else
	case DDIF$_IMG:
#endif
	    if ( !(dcb->DcbL_Flags.DcbV_SkipImgAggr) )
		ChfStop( 1,  ImgX_ILLIMGSEG );   /* illegal image segment	    */
	    else
		{
		/*
		** Delete the memory since we're skipping this image
		** aggregate.
		*/
		status = CDA_DELETE_AGGREGATE_(
			    &(dcb->DcbL_RootAggr),
			    &aggregate_handle );
		LOWBIT_TEST_( status );
		}
	    break;

#if defined(NEW_CDA_SYMBOLS)
	case DDIF_DSC:
	case DDIF_DHD:
#else
	case DDIF$_DSC:
	case DDIF$_DHD:
#endif
	    ChfStop( 1,  ImgX_INVAGRSEQ );   /* invalid aggregate sequence   */
	    break;
	/*
	** Aggregates that ISL doesn't handle.  If an content aggregate 
	** routine has been specified, call it.  Otherwise, simply skip
	** the aggregate.  Skipping the aggregate will cause the next
	** aggregate to be read.  Note that unprocessed aggregates get
	** deleted.
	*/
	default:
	    if ( dcb->DcbA_AggrRtn != 0 )
		{
		/*
		** Call the application action routine to handle
		** unknown aggregates.
		*/
		status = (*(dcb->DcbA_AggrRtn))(
			    dcb->DcbL_RootAggr,
			    dcb->DcbL_StrmCtx,
			    aggregate_handle,
			    aggregate_type,
			    dcb->DcbL_AggrRtnParam );
		LOWBIT_TEST_( status );
		}
	    else
		{
		/*
		** Get rid of the aggregate since it couldn't be dealt with.
		*/
		status = CDA_DELETE_AGGREGATE_(
			    &(dcb->DcbL_RootAggr),
			    &aggregate_handle );
		LOWBIT_TEST_( status );
		}

	} /* end of switch */
    } /* end of while loop */

return dcb->DcbL_Fid;
} /* end of Get_img_frame */
