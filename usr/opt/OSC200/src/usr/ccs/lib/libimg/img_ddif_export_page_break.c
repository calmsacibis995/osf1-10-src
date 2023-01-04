
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
**  IMG_DDIF_EXPORT_PAGE_BREAK
**
**  FACILITY:
**
**	Image Services Library
**
**  ABSTRACT:
**
**	This module contains a function that will export a page break
**	directive to a DDIF document stream.
**
**  ENVIRONMENT:
**
**	VAX/VMS
**
**  AUTHOR(S):
**
**	Mark Sornson
**
**  CREATION DATE:
**
**	9-FEB-1989
**
**  MODIFICATION HISTORY:
**
************************************************************************/

/*
**  Table of contents:
**
*/
#ifdef NODAS_PROTO
void	ImgExportDDIFPageBreak();
#endif
#if defined(__VMS) || defined(VMS)
void	IMG$EXPORT_DDIF_PAGE_BREAK();
#endif


/*
**  Include files:
**
*/
#include <img/ChfDef.h>
#include <img/ImgDef.h>
#include <ImgDefP.h>
#include <ImgMacros.h>
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
**	Symbol Definitions For Message Codes commented out
**
*/
#include    <img/ImgStatusCodes.h>

/*
**	External Routines 
**      CDA's are defined in <cdaptp.h>
*/
#ifdef NODAS_PROTO
void	_ImgVerifyDcb();		/* from module Img__DDIF_BLOCKS */
#endif

/*
**  Local Storage:
**
**	none
*/


/******************************************************************************
**  IMG$EXPORT_DDIF_PAGE_BREAK
**  ImgExportDDIFPageBreak
**
**  FUNCTIONAL DESCRIPTION:
**
**	Export a hard or soft page directive to a DDIF document stream.
**	The default page directive is hard page break.  Soft page breaks
**	can be specified by a flag.
**
**	NOTE: Currently, page directives are written directly to the
**	output stream at the same level as are the image frames.
**	In the future, page directives may be written out wrapped in 
**	their own $T segments.  The code to do this is present, but 
**	commented out for now.
**	
**
**  FORMAL PARAMETERS:
**
**	ctx	Context identifier to a file or stream.  Passed by value.
**
**	flags	Flags.  Passed by value.
**
**		    ImgM_SoftPageBreak   Output a soft directive.
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
**	CDA codes returned as status.
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
#if defined(__VMS) || defined(VMS)
void IMG$EXPORT_DDIF_PAGE_BREAK( ctx, flags )
struct DCB  *ctx;
long	     flags;
{

ImgExportDDIFPageBreak( ctx, flags );
return;
} /* end of IMG$EXPORT_DDIF_PAGE_BREAK */
#endif


/*******************************************************************************
** Portable entry point
*******************************************************************************/
void ImgExportDDIFPageBreak( ctx, flags )
struct DCB  *ctx;
long	     flags;
{
int	status;
CDAagghandle seg_aggr;
CDAagghandle directive_aggr;
CDArootagghandle root_aggr	= ctx->DcbL_RootAggr;
DDISstreamhandle stream_handle	= ctx->DcbL_StrmCtx;
#if defined(NEW_CDA_SYMBOLS)
int	aggr_type	= DDIF_HRD;
CDAconstant item_code	= DDIF_HRD_DIRECTIVE;
long	item_value	= DDIF_K_DIR_NEW_PAGE;
int	scope_code	= DDIF_K_SEGMENT_SCOPE;
#else
int	aggr_type	= DDIF$_HRD;
int	item_code	= DDIF$_HRD_DIRECTIVE;
long	item_value	= DDIF$K_DIR_NEW_PAGE;
int	scope_code	= DDIF$K_SEGMENT_SCOPE;
#endif
long	item_length	= sizeof( item_value );
/*
** Verify that the context is for export.
*/
_ImgVerifyDcb( ctx, DcbK_ModeWrite, 0 );

/*
** Set up the directive to be either hard or soft.
*/
if ( (flags & ImgM_SoftPageBreak) != 0 )
    {
#if defined(NEW_CDA_SYMBOLS)
    aggr_type   = DDIF_SFT;
    item_code   = DDIF_SFT_DIRECTIVE;
#else
    aggr_type   = DDIF$_SFT;
    item_code   = DDIF$_SFT_DIRECTIVE;
#endif
    }

/*
** Create the directive aggregate
*/
status = CDA_CREATE_AGGREGATE_(
	    &root_aggr,
	    &aggr_type,
	    &directive_aggr );
LOWBIT_TEST_( status );

/*
** Store the new page break item in the aggregate.
*/
status = CDA_STORE_ITEM_(
	    &root_aggr,
	    &directive_aggr,
	    &item_code,
	    &item_length,
	    &item_value,
	    0,
	    0 );
LOWBIT_TEST_( status );

/*
** Create a segment aggregate and attach the directive
** aggregate to it.
**aggr_type = DDIF$_SEG;
**status = CDA_CREATE_AGGREGATE_(
**	    &root_aggr,
**	    &aggr_type,
**	    &seg_aggr );
**LOWBIT_TEST_( status );
**
**item_code = DDIF$_SEG_CONTENT;
**status = CDA_STORE_ITEM_(
**	    &root_aggr,
**	    &seg_aggr,
**	    &item_code,
**	    &item_length,
**	    &directive_aggr,
**	    0, 0 );
**LOWBIT_TEST_( status );
*/

/*
** Write the directive aggregate out to the stream by writing
** the segment aggregate.

**status = CDA_ENTER_SCOPE_(
**	    &root_aggr,
**	    &stream_handle,
**	    &scope_code,
**	    &seg_aggr );
**LOWBIT_TEST_( status );
**
**status = CDA_LEAVE_SCOPE_(
**	    &root_aggr,
**	    &stream_handle,
**	    &scope_code );
**LOWBIT_TEST_( status );
*/

/*
** Just write out the directive aggregate for now.
*/
status = CDA_PUT_AGGREGATE_(
	    &root_aggr,	
	    &stream_handle,	
	    &directive_aggr );
LOWBIT_TEST_( status );

/*
** Delete the aggregates that were created above.
** The directive aggregate could be replaced with the seg aggregate.
*/
status = CDA_DELETE_AGGREGATE_(
	    &root_aggr,
	    &directive_aggr );
LOWBIT_TEST_( status );

return;
} /* end of ImgExportDDIFPageBreak */
