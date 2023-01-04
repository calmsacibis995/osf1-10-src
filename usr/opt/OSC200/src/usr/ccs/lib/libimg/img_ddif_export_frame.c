
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
**  IMG$DDIF_EXPORT_FRAME.C
**
**  FACILITY:
**
**	Image Services Library
**
**  ABSTRACT:
**
**	This module contains the routines for data conversion between
**	ISL and DDIF that create a stream of DDIF data from an
**	in-memory ISL frame.
**
**	
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
**	2-APR-1987
**
************************************************************************/

/*
**  Include files:
**
**	From SYS$LIBRARY:VAXCDEF.TLB
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
#include    <cda$def.h>
#include    <cda$msg.h>
#include    <cda$ptp.h>
#include    <ddif$def.h>
#else
#if defined(NEW_CDA_SYMBOLS)
#include    <cdadef.h>
#include    <cdamsg.h>
#include    <ddifdef.h>
#else
#include    <cda_def.h>
#include    <cda_msg.h>
#include    <ddif_def.h>
#endif
#if defined(NEW_CDA_CALLS)
#include    <cdaptp.h>
#else
#include    <cda_ptp.h>
#endif
#endif

/*
**  Table of contents:
**
**	VMS Veneer Global Entry Points 
*/
#if defined(__VMS) || defined(VMS)
long	IMG$EXPORT_DDIF_FRAME();		/* "front-door"	*/
#endif

/*
**	Portable Global Entry Points
*/
#ifdef NODAS_PROTO
long	 ImgExportDDIFFrame();
long	_ImgExportDDIFFrame();		/* "back-door"	*/
#endif

/*
**	Module Local Routines
*/                       
#ifdef NODAS_PROTO
static void Attach_pvt_quant_levels();
static void Detach_pvt_quant_levels();
static void Enter_segment_scope();
static void End_segment_scope();
static void Put_img_content();
#else
PROTO(static void Attach_pvt_quant_levels, (struct FCT */*fct*/));
PROTO(static void Detach_pvt_quant_levels, (struct FCT */*fct*/));
PROTO(static void Enter_segment_scope, (struct FCT */*fid*/, CDArootagghandle /*root_aggregate*/, DDISstreamhandle /*stream_handle*/));
PROTO(static void End_segment_scope, (CDArootagghandle /*root_aggregate*/, DDISstreamhandle /*stream_handle*/));
PROTO(static void Put_img_content, (struct FCT */*fid*/, CDArootagghandle /*root_aggr*/, DDISstreamhandle /*stream_handle*/));
#endif

/*
**  MACRO definitions:
** 
**       (also see ImgMacros.h)
*/                                              

/*
**  Equated Symbols:
*/

/*                                
**  External References:
**  CDA references are done in <cdaptp.h>
*/
#ifdef NODAS_PROTO
long	     ImgCopy();
struct DCB  *ImgCreateDDIFStream();	/* from IMG$DDIF_IO_MGT	    */
void	     ImgDeleteDDIFStream();
void	     ImgDeleteFrame();
void	     ImgExportDDIFPageBreak();  /* from IMG$DDIF_EXPORT_PAGE_BREAK */
void	     ImgVerifyFrame();

void	     _ImgGet();		/* from Img__ACCESS_UTILS   */
long	     _ImgGetVerifyStatus();
void	     _ImgVerifyDcb();		/* from Img__DDIF_UTILS	    */

/*
**	From VAX RTL
*/
void	ChfSignal();			/* signal condition handler */
void	ChfStop();			/* signal condition handler to stop */
#endif

/*
** External data
**
**	Status code definitions commented out!
**
*/
#include    <img/ImgStatusCodes.h>

/*
**  String name(s)
**
**	Name of private attr aggr used to store quant levels per comp
*/
#if defined(__VAXC) || defined(VAXC)
globalref char isl_pvt_sga_attr_name[];	/* from IMG__FRAME_UTILS    */
#else
extern char isl_pvt_sga_attr_name[];	/* from IMG__FRAME_UTILS    */
#endif

/*
**  Local Storage:
**
**       none
*/


/******************************************************************************
**  IMG$EXPORT_DDIF_FRAME
**  ImgExportDDIFFrame
**
**  FUNCTIONAL DESCRIPTION:
**
**      Create an image-only DDIF data stream using an ISL image frame as the
**	image data source.
**
**  FORMAL PARAMETERS:
**
**      fid		frame id, by value, required
**			Frame id of the frame to be exported to DDIF.
**
**	SID	     	stream id, by value, optional
**			Stream id created by a user-call to 
**			IMG$CREATE_DDIF_STREAM.  A stream-id must be 
**			established externally if multiple frames are
**			to be exported to the same DDIF stream.
**			This parameter may be omitted if a complete
**			DDIF document stream containing ONE image frame
**			only is desired, thereby circumventing the
**			need for the caller to use calls to IMG$OPEN_DDIF-
**			_STREAM  and IMG$DELETE_DDIF_STREAM.
**
**	BUFADR		buffer address, by reference, optional
**			Address of buffer into which is put the exported DDIF
**			data.  If omitted, a buffer will be allocated
**			internally, using the value of the buflen parameter.
**			If the buffer is too small to hold the entire DDIF
**			stream, it will be emptied by the action routine
**			(which must be supplied in this case).  If this
**			parameter is omitted, an action routine MUST be
**			supplied.  If present, buflen must also be specified.
**
**			NOTE that this parameter MUST be omitted if it was
**			passed into IMG$CREATE_DDIF_STREAM.
**
**	BUFLEN		buffer length, by value, optional
**			Length in bytes of the buffer passed in as bufadr.
**			If bufadr is omitted, buflen will be used as the
**			size of the internal buffer to be allocated.  If
**			both bufadr and buflen are omitted, a default
**			length value (DcbK_DefIobLen) is used.
**			
**			NOTE that this parameter MUST be omitted if it was
**			passed into IMG$CREATE_DDIF_STREAM.
**
**	FLAGS		flags, by value, optional
**			(Values to be supplied.)
**
**	ACTION		action routine, by reference, optional
**			Action routine to be called to empty the buffer used
**			to hold the DDIF-stream data.  The action routine
**			will be called when the buffer is full (and more
**			data has yet to be written to the DDIF stream), and
**			when the stream is closed (thereby flushing the buffer
**			to the data sink managed by the action routine).  If
**			the bufadr parameter is omitted, an action routine
**			MUST be specified.
**
**	USER_PARAM	action routine parameter, by value, optional
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
**      fid		returns value of fid passed in.
**
**  SIGNAL CODES:
**
**      ImgX_INVARGCNT	    Invalid argument count.
**      ImgX_BUFWNOLEN	    Buffer with no length.  Bufadr was specified,
**			    but buflen was zero.
**	ImgX_PARAMCONF	    Parameter conflict.  Stream id was nonzero, and
**			    bufadr, buflen, action, or user_param was not zero.
**
**  SIDE EFFECTS:
**
**      none
**
******************************************************************************/
#if defined(__VMS) || defined(VMS)
long IMG$EXPORT_DDIF_FRAME( fid, roi_id, sid, bufadr, buflen, flags, 
			    action, user_param )
long	      fid;			/* frame id			    */
long	     *roi_id;			/* roi id                           */
struct	DCB  *sid;			/* stream id                        */
char	     *bufadr;			/* adr of I/O buffer                */
long	      buflen;			/* len of I/O buffer                */
long	      flags;			/* flags                	    */
long	    (*action)();		/* user I/O action routine          */
long	      user_param;		/* param for user I/O action        */
{

return (ImgExportDDIFFrame(fid,roi_id,sid,bufadr,buflen,flags,action,
	user_param));
} /* end of IMG$EXPORT_DDIF_FRAME */
#endif


/*******************************************************************************
** Portable entry point
*******************************************************************************/
struct FCT *ImgExportDDIFFrame( fid, roi_id, sid, bufadr, buflen, flags, 
			action, user_param )
struct FCT *fid;			/* frame id			    */
struct ROI *roi_id;			/* roi id                           */
struct	DCB  *sid;			/* stream id                        */
char	     *bufadr;			/* adr of I/O buffer                */
long	      buflen;			/* len of I/O buffer                */
long	      flags;			/* flags                	    */
long	    (*action)();		/* user I/O action routine          */
long	      user_param;		/* param for user I/O action        */
{
long	      local_flags;
struct FCT *savfid	    = NULL;
long	      status;
struct	DCB  *dcb	    = 0;	/* ddif context block		    */


if ( bufadr != NULL && buflen == 0 )
	ChfStop( 1,  ImgX_BUFWNOLEN );

if (roi_id != NULL)			    /* Save the frame-id, copy the  */
    {					    /* frame with the ROI and subst */
    savfid = fid;			    /* the temp frame-id for the    */
    fid = ImgCopy( fid, roi_id, 0 );	    /* real id.  The temp frame is  */
    }					    /* deleted before we return.    */

/*
** Setup document context block, allocating one if one is not present.
*/
if ( sid == NULL )			    /* Stand-alone, one-call mode?  */
    {					    /* YEP, therefore make a stream */
    /*
    ** Open a DDIF stream: note that the validity of the bufadr, buflen
    ** and action parameters will be checked by IMG$CREATE_DDIF_STREAM
    ** Indicate no stream id passed in, which means the DDIF stream will
    ** be closed when the export operation on this frame is finished.
    */    
    dcb = ImgCreateDDIFStream( DcbK_ModeWrite, bufadr, buflen,
    		flags, action, user_param);
    dcb->DcbL_Flags.DcbV_NoSid = TRUE;	
    }
else					    /* NOPE, therefore use sid	    */
    {
    /*
    ** Repeated call mode:  Note that bufadr and buflen have been
    ** established by IMG$CREATE_DDIF_STREAM, and so may not be passed
    ** in.  Note that the following arguments must be zero or omitted,
    ** having been established when the stream was opened:
    **
    **		    buflen, bufadr, action, user_param
    */
    if ( bufadr != NULL || 
         buflen != 0 || 
         action != NULL || 
         user_param != (long) NULL )
	ChfStop( 1,  ImgX_PARAMCONF );	    /* parameter conflict   */

    dcb = (struct DCB *) sid;		    /* Use input sid and verify it. */
    _ImgVerifyDcb( dcb, DcbK_ModeWrite,0 );
    }

/*
** Check flag fields of the flags input argument, setting the
** corresponding flags in the context block.
*/
if ( flags != (long) NULL )
    {
    if ((flags & ImgM_NoBufRealloc) != FALSE )
    	dcb->DcbL_Flags.DcbV_NoBufRealloc = TRUE;
    }

/*
** Verify that the frame-id is valid
*/
local_flags = ImgM_NonstandardVerify;
if ( VERIFY_ON_ )
    ImgVerifyFrame( fid, local_flags );

/*
** Save the current frame-id in the document context block just as a
** record of the most recent frame exported.
*/
dcb->DcbL_Fid = (struct FCT *) fid;

/*
** Put a new page break first?
*/
if ((flags & ImgM_PageBreak) != FALSE )
    ImgExportDDIFPageBreak(dcb, flags );

/*
** Export the DDIF frame.  
*/
_ImgExportDDIFFrame( fid, dcb->DcbL_RootAggr, dcb->DcbL_StrmCtx,
			flags );

/*
** Restore the original frame id and delete the temporrary one
*/
if (roi_id != NULL)
    {
    ImgDeleteFrame(fid);
    fid = savfid;
    }

/*
** Close the stream if it was temporary.
*/
if ( dcb->DcbL_Flags.DcbV_NoSid == TRUE )
    {
    ImgDeleteDDIFStream( dcb );
    }

return fid;
} /* end of ImgExportDDIFFrame */


/******************************************************************************
**  _ImgExportDDIFFrame
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine actually exports the ISL image attributes and data
**	to DDIF.
**
**	This routine is also a "back door" routine for sophisticated
**	applications that are doing their own DDIF IO (e.g. compound
**	document processing).  This routine will allow applications
**	-- that have already established a DDIF stream environment for
**	themselves using DDIF Toolkit routines -- to convert a single
**	ISL image frame into a DDIF image frame (Beginsegment, ImageContent,
**	Endsegment).
**
**	This routine creates a begin-segment for an image frame, writes
**	the segment contents (image data and attributes), and closes the
**	frame with an end-segment.  [This is done with CDA_ENTER_SCOPE_,
**	CDA_PUT_AGGREGATE_, and CDA$CLOSE_SCOPE calls.]
**
**  FORMAL PARAMETERS:
**
**	fid		Frame-id of the frame to export.
**			Unsigned longword.
**			Passed by VALUE.
**
**	root_aggregate	Root aggregate handle associated with the output
**			stream.
**			Unsigned longword.
**			Passed by VALUE (not by REFERENCE).
**
**	stream_handle	Stream handle associated with the output stream.
**			Unsigned longword.
**			Passed by VALUE (not by REFERENCE).
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
**	Frame id of the frame exported.  
**	Unsigned longword.
**	Passed by VALUE.
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
struct FCT *_ImgExportDDIFFrame( fid, root_aggregate, stream_handle, flags )
struct FCT *fid;
CDArootagghandle root_aggregate;
DDISstreamhandle stream_handle;
long flags;
{

Attach_pvt_quant_levels( fid );
Enter_segment_scope( fid, root_aggregate, stream_handle );
Put_img_content( fid, root_aggregate, stream_handle );
End_segment_scope( root_aggregate, stream_handle );
Detach_pvt_quant_levels( fid );

return fid;
} /* end of _ImgExportDDIFFrame */


/******************************************************************************
**  Attach_pvt_quant_levels()
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
static void Attach_pvt_quant_levels( fct )
struct FCT  *fct;
{
CDAconstant aggregate_item;
long aggregate_item_value;
int	aggregate_type;
long	comp_cnt;
long	comp_idx;
long	length;
CDAagghandle main_aggregate_handle;
CDAagghandle prev_sub_aggregate_handle;
CDAagghandle sub_aggregate_handle;
int	status;

/*
** Only go through this code if quant levels per comp had to be
** implemented privately by ISL
*/
if ( !(fct->FctL_Flags.FctV_PvtQLevels) )
    return;

/*
** Create the main private attr aggr, set the name string, and
** declare the data type.
*/
#if defined(NEW_CDA_SYMBOLS)
aggregate_type = DDIF_PVT;
#else
aggregate_type = DDIF$_PVT;
#endif
status = CDA_CREATE_AGGREGATE_(
		&(fct->FctL_RootAggr),
		&aggregate_type,
		&main_aggregate_handle
		);
LOWBIT_TEST_( status );

#if defined(NEW_CDA_SYMBOLS)
aggregate_item = DDIF_PVT_NAME;
#else
aggregate_item = DDIF$_PVT_NAME;
#endif
length = strlen( isl_pvt_sga_attr_name );
status = CDA_STORE_ITEM_(
		&(fct->FctL_RootAggr),
		&main_aggregate_handle,
		&aggregate_item,
		&length,
		isl_pvt_sga_attr_name,
		0, 0 );
LOWBIT_TEST_( status );

#if defined(NEW_CDA_SYMBOLS)
aggregate_item = DDIF_PVT_DATA_C;
aggregate_item_value = DDIF_K_VALUE_LIST;
#else
aggregate_item = DDIF$_PVT_DATA_C;
aggregate_item_value = DDIF$K_VALUE_LIST;
#endif
length = sizeof(aggregate_item_value);
status = CDA_STORE_ITEM_(
		&(fct->FctL_RootAggr),
		&main_aggregate_handle,
		&aggregate_item,
		&length,
		(CDAaddress)&aggregate_item_value,
		0, 0 );
LOWBIT_TEST_( status );

/*
** Store each quant_level_per_comp value in a private sub-aggregate,
** and attach each sub-aggregate to the main aggregate.
*/
_ImgGet( fct, Img_NumberOfComp, &comp_cnt, LONGSIZE, 0, 0 );
for ( comp_idx = 0; comp_idx < comp_cnt; ++comp_idx )
    {
    /*
    ** Create the sub aggregate
    */
#if defined(NEW_CDA_SYMBOLS)
    aggregate_type = DDIF_PVT;
#else
    aggregate_type = DDIF$_PVT;
#endif
    status = CDA_CREATE_AGGREGATE_(
		&(fct->FctL_RootAggr),
		&aggregate_type,
		&sub_aggregate_handle
		);
    LOWBIT_TEST_( status );

    /*
    ** Declare the data type ...
    */
#if defined(NEW_CDA_SYMBOLS)
    aggregate_item = DDIF_PVT_DATA_C;
    aggregate_item_value = DDIF_K_VALUE_INTEGER;
#else
    aggregate_item = DDIF$_PVT_DATA_C;
    aggregate_item_value = DDIF$K_VALUE_INTEGER;
#endif
    length = sizeof(aggregate_item_value);
    status = CDA_STORE_ITEM_(
		&(fct->FctL_RootAggr),
		&sub_aggregate_handle,
		&aggregate_item,
		&length,
		(CDAaddress)&aggregate_item_value,
		0, 0 );
    LOWBIT_TEST_( status );

    /*
    ** Store the data in the sub-aggregate ...
    **
    **	NOTE:	the pvt_q_level data is stored in a counted
    **		longword array attached directly to the FCT
    */
#if defined(NEW_CDA_SYMBOLS)
    aggregate_item = DDIF_PVT_DATA;
#else
    aggregate_item = DDIF$_PVT_DATA;
#endif
    aggregate_item_value = (fct->FctA_PvtQLevels)[comp_idx + 1];
    status = CDA_STORE_ITEM_(
		&(fct->FctL_RootAggr),
		&sub_aggregate_handle,
		&aggregate_item,
		&length,
		(CDAaddress)&aggregate_item_value,
		0, 0 );
    LOWBIT_TEST_( status );

    /*
    ** Attach the sub-aggregate to the main aggregate
    */
    if ( comp_idx == 0 )
	{
#if defined(NEW_CDA_SYMBOLS)
	aggregate_item = DDIF_PVT_DATA;
#else
	aggregate_item = DDIF$_PVT_DATA;
#endif
	status = CDA_STORE_ITEM_(
		    &(fct->FctL_RootAggr),
		    &main_aggregate_handle,
		    &aggregate_item,
		    &length,
		    &sub_aggregate_handle,
		    0,0
		    );
	LOWBIT_TEST_( status );
	}
    else
	{
	status = CDA_INSERT_AGGREGATE_(
		    &sub_aggregate_handle,
		    &prev_sub_aggregate_handle
		    );
	LOWBIT_TEST_( status );
	}

    prev_sub_aggregate_handle = sub_aggregate_handle;
    } /* end for */

/*
** Attach the main pvt aggr to the SGA aggr
*/
#if defined(NEW_CDA_SYMBOLS)
aggregate_item = DDIF_SGA_IMG_PRIVATE_DATA;
#else
aggregate_item = DDIF$_SGA_IMG_PRIVATE_DATA;
#endif
status = CDA_STORE_ITEM_(
	    &(fct->FctL_RootAggr),
	    &(fct->FctL_SgaAggr),
	    &aggregate_item,
	    &length,
	    &main_aggregate_handle,
	    0,0
	    );
LOWBIT_TEST_(status );

return;
} /* end of Attach_pvt_quant_levels */


/******************************************************************************
**  Detach_pvt_quant_levels()
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
static void Detach_pvt_quant_levels( fct )
struct FCT  *fct;
{
CDAconstant aggregate_item;
int	status;

if ( fct->FctL_Flags.FctV_PvtQLevels )
    {
#if defined(NEW_CDA_SYMBOLS)
    aggregate_item = DDIF_SGA_IMG_PRIVATE_DATA;
#else
    aggregate_item = DDIF$_SGA_IMG_PRIVATE_DATA;
#endif
    status = CDA_ERASE_ITEM_(
	    &(fct->FctL_RootAggr),
	    &(fct->FctL_SgaAggr),
	    &aggregate_item,
	    0 );
    LOWBIT_TEST_(status );
    }

return;
} /* end of Detach_pvt_quant_levels */


/******************************************************************************
**  Enter_segment_scope
**
**  FUNCTIONAL DESCRIPTION:
**
**	Open the DDIF segment scope.  Once this scope is entered, image
**	content segments can be written to the output stream.
**
**  FORMAL PARAMETERS:
**
**	fid		Frame-id.
**			Passed by value.
**
**	root_aggregate	Root aggregate handle.
**			Passed by value.
**
**	stream_handle	Stream handle.
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
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
static void Enter_segment_scope( fid, root_aggregate, stream_handle )
struct FCT *fid;
CDArootagghandle root_aggregate;
DDISstreamhandle stream_handle;
{
CDAagghandle seg_aggr;
long	scope_code;
long	status;

/*
** Get the segment aggregate handle from the frame.  This aggregate
** contains the user-label, frame parameters, image presentation
** attributes, and the image spectral organization attributes.
*/
_ImgGet( fid, Img_SegAggr, &seg_aggr, sizeof(seg_aggr), 0, 0 );

/*
** Enter/open the frame-segment scope and write the segment aggregate
** with the segment attributes to the output stream.  (Note again that 
** this puts out the attributes, but not the content.)
*/
#if defined(NEW_CDA_SYMBOLS)
scope_code = DDIF_K_SEGMENT_SCOPE;
#else
scope_code = DDIF$K_SEGMENT_SCOPE;
#endif
status = CDA_ENTER_SCOPE_( &root_aggregate, &stream_handle,
			  &scope_code, &seg_aggr );
LOWBIT_TEST_( status );

return;
} /* end of Enter_segment_scope */


/******************************************************************************
**  End_content_scope
**
**  FUNCTIONAL DESCRIPTION:
**
**	Close the DDIF frame-segment scope that was opened by 
**	Put_begin_segment().  This signifies that the image-frame
**	has been completely written to the output stream.
**
**  FORMAL PARAMETERS:
**
**	root_aggregate	    Root aggr handle.
**			    Passed by value.
**
**	stream_handle	    Stream handle.
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
static void End_segment_scope( root_aggregate, stream_handle )
CDArootagghandle root_aggregate;
DDISstreamhandle stream_handle;
{
#if defined(NEW_CDA_SYMBOLS)
CDAconstant scope_code = DDIF_K_SEGMENT_SCOPE;
#else
CDAconstant scope_code = DDIF$K_SEGMENT_SCOPE;
#endif
int	status;

status = CDA_LEAVE_SCOPE_(
	    &root_aggregate, 
	    &stream_handle, 
	    &scope_code );
LOWBIT_TEST_( status );

return;
} /* end of End_segment_scope */


/******************************************************************************
**  Put_img_content
**
**  FUNCTIONAL DESCRIPTION:
**
**	Put the image content of an image frame to the output stream.
**	Simply, this routine finds the first DDIF$_IMG content aggregate
**	in the frame, and outputs it and all DDIF$_IMG aggregates linked
**	to it.  This will output ALL the image data in the frame.
**
**  FORMAL PARAMETERS:
**
**	fid		Frame-id.
**			Passed by value.
**
**	root_aggregate	Root aggregate handle.
**			Passed by value.
**
**	stream_handle	Stream handle.
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
static void Put_img_content( fid, root_aggr, stream_handle )
struct FCT *fid;
CDArootagghandle root_aggr;
DDISstreamhandle stream_handle;
{
CDAconstant aggregate_item;
CDAagghandle img_aggr;
CDAaddress item;
long	 length;
CDAagghandle seg_aggr;
int	 status;

/*
** Get the segment aggregate from the frame.
*/
_ImgGet( fid, Img_SegAggr, &seg_aggr, sizeof(seg_aggr), 0, 0 );

/*
** Get the first image content DDIF$_IMG segment from the temporary
** DDIF$_SEG aggregate.
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
img_aggr = *(CDAagghandle *)item;

/*
** Put the current image content aggregate and get the next one.
** Stop when there are no more.
*/
do
    {
/*
** Put the aggregate
*/
    status = CDA_PUT_AGGREGATE_(
                &root_aggr,
                &stream_handle,
                &img_aggr );
    LOWBIT_TEST_( status );

/*
** Get the next one.
*/
    status = CDA_NEXT_AGGREGATE_(
                &img_aggr, 
                &img_aggr );
    }
#if defined(NEW_CDA_SYMBOLS)
while ( status != CDA_ENDOFSEQ );
#else
while ( status != CDA$_ENDOFSEQ );
#endif

return;
} /* end of Put_img_content */
