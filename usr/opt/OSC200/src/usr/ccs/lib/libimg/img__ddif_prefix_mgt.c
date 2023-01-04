
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
**  IMG__DDIF_PREFIX_MGT.C
**
**  FACILITY:
**
**	Image Services Library
**
**  ABSTRACT:
**
**	Functions to manage the import and export of the DDIF Descriptor
**	and the DDIF Header, which together are refered to as the DDIF
**	Prefix.  [Note that "DDIF Prefix" is a term used by the ISL
**	DDIF mgt code, and is not used by the official DDIF toolkit and
**	documentation.]
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
**	19-OCT-1987
**
**  MODIFICATION HISTORY:
**
************************************************************************/

/*
**  Table of contents:
**
**	Global routines
*/
#ifdef NODAS_PROTO
void	_ImgGetDDIFPrefix();
void	_ImgPutDDIFDsc();
void	_ImgPutDDIFDhd();
void	_ImgPutDDIFPrefix();
#endif


/*
**  Include files:
**
*/
#include    <string.h>
#include    <time.h>

#if defined(__VMS) || defined(VMS)
#include    <cda$def.h>
#include    <cda$ptp.h>
#include    <ddif$def.h>
#else
#if defined(NEW_CDA_SYMBOLS)
#include    <cdadef.h>
#include    <ddifdef.h>
#else
#include    <cda_def.h>
#include    <ddif_def.h>
#endif
#if defined(NEW_CDA_CALLS)
#include    <cdaptp.h>
#else
#include    <cda_ptp.h>
#endif
#endif

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
*/

/*
**  External References:
**
**	DDIF Toolkit Routines are defined in <cdaptp.h>
*/

/*
**	Module local routines
*/
#ifdef NODAS_PROTO
static void Store_ddif_descriptor_items();
static void Store_ddif_header_items();
#else
PROTO(static void Store_ddif_descriptor_items, (CDArootagghandle /*root_aggr*/, CDAagghandle /*aggr_handle*/));
PROTO(static void Store_ddif_header_items, (CDArootagghandle /*root_aggr*/, CDAagghandle /*aggr_handle*/));
#endif

/*
**  Error Codes
*/

/*
**  Local Storage:
**
**	none
*/


/******************************************************************************
**  _ImgGetDDIFPrefix
**
**  FUNCTIONAL DESCRIPTION:
**
**	Get the document descriptor and header (which together are called
**	the prefix).  Store the aggregate handles for each one in the
**	dcb, and attach each aggregate to the input root aggregate.  This
**	will allow applications to read the desc and header information
**	by retrieving the aggregates.
**
**  FORMAL PARAMETERS:
**
**	dcb		Document context block.
**			Passed by reference.
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
**	none (by ISL) -- CDA codes only.
**
**  SIDE EFFECTS:
**
**	none
**
**  CALLED BY:
**
**	_ImgCreateDDIFStream	    (module IMG$DDIF_IO_MGT)
**	_ImgOpenDDIFFIle	    (module IMG$DDIF_IO_MGT)
******************************************************************************/
void _ImgGetDDIFPrefix( dcb )
struct	DCB *dcb;
{
int	aggregate_type;
CDAconstant aggregate_item;
long	item_size;
int	status;

/*
** Get the document descriptor
*/
status = CDA_GET_AGGREGATE_(
	    &(dcb->DcbL_RootAggr),
	    &(dcb->DcbL_StrmCtx),
	    &(dcb->DcbL_DscAggr),
	    &aggregate_type );
LOWBIT_TEST_( status );

#if defined(NEW_CDA_SYMBOLS)
aggregate_item = DDIF_DDF_DESCRIPTOR;
#else
aggregate_item = DDIF$_DDF_DESCRIPTOR;
#endif
item_size = sizeof(dcb->DcbL_DscAggr);

status = CDA_STORE_ITEM_(			/* Attach the document	*/
	    &(dcb->DcbL_RootAggr),	/* descriptor that was	*/
	    (CDAagghandle *)&(dcb->DcbL_RootAggr),	/* read in to the doc-	*/
	    &aggregate_item,		/* ment root.		*/
	    &item_size,
	    &(dcb->DcbL_DscAggr),
	    0, 0 );
LOWBIT_TEST_( status );

/*
** Get the document header
*/
status = CDA_GET_AGGREGATE_(
	    &(dcb->DcbL_RootAggr),
	    &(dcb->DcbL_StrmCtx),
	    &(dcb->DcbL_DhdAggr),
	    &aggregate_type );
LOWBIT_TEST_( status );

#if defined(NEW_CDA_SYMBOLS)
aggregate_item = DDIF_DDF_HEADER;
#else
aggregate_item = DDIF$_DDF_HEADER;
#endif

item_size = sizeof(dcb->DcbL_DhdAggr);
status = CDA_STORE_ITEM_(			/* Attach the document	*/
	    &(dcb->DcbL_RootAggr),	/* header that was read	*/
	    (CDAagghandle *)&(dcb->DcbL_RootAggr),	/* in to the document	*/
	    &aggregate_item,		/* root.		*/
	    &item_size,
	    &(dcb->DcbL_DhdAggr),
	    0, 0 );
LOWBIT_TEST_( status );

return;
} /* end of _ImgGetDDIFPrefix */


/******************************************************************************
**  _ImgPutDDIFDsc
**
**  FUNCTIONAL DESCRIPTION:
**
**	Put the DDIF document descriptor out to the output stream.
**	This will create a descriptor aggregate, and fill it in with
**	ISL default values.  It will then be put, after which it
**	will be deleted.
**
**  FORMAL PARAMETERS:
**
**	dcb		Document context block.
**			Passed by reference.
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
void _ImgPutDDIFDsc( dcb )
struct	DCB *dcb;
{
CDAagghandle aggr_handle;
#if defined(NEW_CDA_SYMBOLS)
int	aggregate_type  = DDIF_DSC;
#else
int	aggregate_type  = DDIF$_DSC;
#endif
int	status;

if ( dcb->DcbL_DscAggr == 0 )
    {
    /*
    ** Create DDIF document descriptor aggregate.
    */
    status = CDA_CREATE_AGGREGATE_(
		&(dcb->DcbL_RootAggr),
		&aggregate_type,
		&aggr_handle );
    LOWBIT_TEST_( status );

    /*
    ** Populate the aggregate with the ISL default values
    */
    Store_ddif_descriptor_items( dcb->DcbL_RootAggr, aggr_handle );
    }
else
    /*
    ** Use the dsc aggr stored in the DCB (which was passed in by the
    ** application).
    */
    {
    aggr_handle = dcb->DcbL_DscAggr;
    }

/*
** Write the DDIF descriptor aggregate.
*/
status = CDA_PUT_AGGREGATE_(
		&(dcb->DcbL_RootAggr),
		&(dcb->DcbL_StrmCtx),
		&aggr_handle );
LOWBIT_TEST_( status );

if ( dcb->DcbL_DscAggr == 0 )
    {
    /*
    ** Get rid of the descriptor aggregate now that we are done with it.
    */
    status = CDA_DELETE_AGGREGATE_(
		&(dcb->DcbL_RootAggr),
		&aggr_handle );
    LOWBIT_TEST_( status );
    }

return;
} /* end of _ImgPutDDIFDsc */


/******************************************************************************
**  _ImgPutDDIFDhd
**
**  FUNCTIONAL DESCRIPTION:
**
**	Put the DDIF document header to the output stream.  This will
**	create a temporary document header aggregate and fill it with
**	ISL default values.  It will then put the aggregate, and afterwards
**	delete it.
**
**  FORMAL PARAMETERS:
**
**	dcb		Document context block.
**			Passed by reference.
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
void _ImgPutDDIFDhd( dcb )
struct	DCB *dcb;
{
CDAagghandle aggr_handle;
#if defined(NEW_CDA_SYMBOLS)
int	aggregate_type  = DDIF_DHD;
#else
int	aggregate_type  = DDIF$_DHD;
#endif
int	status;

if ( dcb->DcbL_DhdAggr == 0 )
    {
    /*
    ** Create DDIF document header aggregate.
    */
    status = CDA_CREATE_AGGREGATE_(
		&(dcb->DcbL_RootAggr),
		&aggregate_type,
		&aggr_handle );
    LOWBIT_TEST_( status );

    /*
    ** Populate the header aggregate with the ISL default values
    */
    Store_ddif_header_items( dcb->DcbL_RootAggr, aggr_handle );
    }
else
    /*
    ** Use the dhd aggr stored in the DCB (which was passed in by the
    ** application).
    */
    {
    aggr_handle = dcb->DcbL_DhdAggr;
    }

/*
** Write the DDIF header aggregate.
*/
status = CDA_PUT_AGGREGATE_(
		&(dcb->DcbL_RootAggr),
		&(dcb->DcbL_StrmCtx),
		&aggr_handle );
LOWBIT_TEST_( status );

if ( dcb->DcbL_DhdAggr == 0 )
    {
    /*
    ** Get rid of the header aggregate now that we are done with it.
    */
    status = CDA_DELETE_AGGREGATE_(
		&(dcb->DcbL_RootAggr),
		&aggr_handle );
    LOWBIT_TEST_( status );
    }

return;
} /* end of _ImgPutDDIFDhd */


/******************************************************************************
**  _ImgPutDDIFPrefix
**
**  FUNCTIONAL DESCRIPTION:
**
**	Control the output of the DDIF document descriptor and header.
**
**  FORMAL PARAMETERS:
**
**	dcb		Document context block.
**			Passed by reference.
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
**  CALLED BY:
**
**	_ImgCreateDDIFStream	    (module IMG$DDIF_IO_MGT)
**	_ImgOpenDDIFFIle	    (module IMG$DDIF_IO_MGT)
******************************************************************************/
void _ImgPutDDIFPrefix( dcb )
struct	DCB *dcb;
{
CDAagghandle root_segment_aggr;
CDAconstant scope_code;
int	aggregate_type;
int	status;

/*
** Open (or enter) the scope level for a new document.
** This scope level will be left (or closed) when the document closed.
*/
#if defined(NEW_CDA_SYMBOLS)
scope_code = DDIF_K_DOCUMENT_SCOPE;
#else
scope_code = DDIF$K_DOCUMENT_SCOPE;
#endif

status = CDA_ENTER_SCOPE_(
		    &(dcb->DcbL_RootAggr),
		    &(dcb->DcbL_StrmCtx),
		    &scope_code,
		    0 );
LOWBIT_TEST_( status );

/*
** Put the document prefix aggregates, namely the doc descriptor and
** the document header.  All applications to call their own put
** prefix routine.
*/
if ( dcb->DcbA_PrefixRtn != NULL )
    {
    (*(dcb->DcbA_PrefixRtn))(		/* Call to application	*/
	    &(dcb->DcbL_RootAggr),	/* routine to put both	*/
	    &(dcb->DcbL_StrmCtx),		/* DSC and DHD aggrs.	*/
	    dcb->DcbL_PrefixRtnParam		/* This MUST do the	*/
	    );					/* right thing.		*/
    }
else
    {
    /*
    ** Put the DDIF document descriptor
    **
    **	This routine MUST create and put an aggregate of type DDIF$_DSC
    */
    _ImgPutDDIFDsc( dcb );

    /*
    ** Put the DDIF document header
    **
    **	This routine MUST create and put an aggregate of type DDIF$_DHD
    */
    _ImgPutDDIFDhd( dcb );
    }

/*
** Now enter document content scope.  Calls to Img__EXPORT_DDIF_FRAME
** will enter and exit segment scope in order to put frames.  Content
** scope is closed when the stream or file is closed (by Img__PutDOCUMENT_END).
*/
#if defined(NEW_CDA_SYMBOLS)
scope_code = DDIF_K_CONTENT_SCOPE;
#else
scope_code = DDIF$K_CONTENT_SCOPE;
#endif

status = CDA_ENTER_SCOPE_(
		    &(dcb->DcbL_RootAggr),
		    &(dcb->DcbL_StrmCtx),
		    &scope_code,
		    0 );
LOWBIT_TEST_( status );

/*
** Finally, enter the root segment scope.  This puts a dummy segment aggregate
** out to the file.  This is required by the DDIF semantic analyzer so that
** multiple image frames (as a series of Begin/End Segments) can be written
** to the output stream.  Without this root segment scope, only one image
** frame can be written to the file.
**
** If a single image frame is written to the file, this segment is somewhat
** redundant, but it doesn't hurt anything.
**
** The steps in the process are:
**
**	Create a dummy segment aggregate.
**	Enter the root segment scope with it.
**	Delete the dummy segment aggregate.
*/
#if defined(NEW_CDA_SYMBOLS)
aggregate_type = DDIF_SEG;
#else
aggregate_type = DDIF$_SEG;
#endif
status = CDA_CREATE_AGGREGATE_(
		&(dcb->DcbL_RootAggr),
		&aggregate_type,
		&root_segment_aggr );
LOWBIT_TEST_( status );

#if defined(NEW_CDA_SYMBOLS)
scope_code = DDIF_K_SEGMENT_SCOPE;
#else
scope_code = DDIF$K_SEGMENT_SCOPE;
#endif

status = CDA_ENTER_SCOPE_(
		    &(dcb->DcbL_RootAggr),
		    &(dcb->DcbL_StrmCtx),
		    &scope_code,
		    &root_segment_aggr );
LOWBIT_TEST_( status );

status = CDA_DELETE_AGGREGATE_(
		    &(dcb->DcbL_RootAggr),
		    &root_segment_aggr );
LOWBIT_TEST_( status );

return;
} /* end of _ImgPutDDIFPrefix */


/******************************************************************************
**  Store_ddif_descriptor_items
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine stores ISL default values into the default document
**	descriptor aggregate that is used for output.
**
**  FORMAL PARAMETERS:
**
**	root_aggr	Root aggregate handle associated with the
**			doc. desc. aggr.
**			Passed by value.
**	aggr_handle	Document descriptor aggregate handle.
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
**  CALLED BY:
**
**	_ImgPutDDIFDsc
**
******************************************************************************/
static void Store_ddif_descriptor_items( root_aggr, aggr_handle )
CDArootagghandle root_aggr;
CDAagghandle aggr_handle;
{
char	*prodid = IMG_KT_PRODID;	/* defined in Img__DDIF */
char	*prodnam = IMG_KT_PRODNAM;	/* defined in Img__DDIF	*/
CDAconstant aggregate_item;
int	 aggregate_index;
int	 item_size;
int	 item_value;
long	 length;
int	 status;
CDAconstant string_type;

/*
** Store Major Version Number
*/
#if (0)
CDA told us not to write these (Peter Derr, June 10, 1992)

#if defined(NEW_CDA_SYMBOLS)
aggregate_item = DDIF_DSC_MAJOR_VERSION;
#else
aggregate_item = DDIF$_DSC_MAJOR_VERSION;
#endif

item_size = sizeof(item_value);
item_value = 1;
status = CDA_STORE_ITEM_(    &root_aggr,
			    &aggr_handle,
			    &aggregate_item,
			    &item_size,	    /* Item length = 4	*/
			    &item_value,    /* Item value = 1	*/
			    0, 0 );
LOWBIT_TEST_( status );

/*
** Store Minor Version Number
*/
#if defined(NEW_CDA_SYMBOLS)
aggregate_item = DDIF_DSC_MINOR_VERSION;
#else
aggregate_item = DDIF$_DSC_MINOR_VERSION;
#endif
item_size = sizeof(item_value);
item_value = 0;
status = CDA_STORE_ITEM_(	    &root_aggr,
			    &aggr_handle,
			    &aggregate_item,
			    &item_size,	    /* Item length = 4	*/
			    &item_value,    /* Item value = 0	*/
			    0, 0 );
LOWBIT_TEST_( status );
#endif

/*
** Store Product Identifier
*/
#if defined(NEW_CDA_SYMBOLS)
aggregate_item = DDIF_DSC_PRODUCT_IDENTIFIER;
#else
aggregate_item = DDIF$_DSC_PRODUCT_IDENTIFIER;
#endif
length = strlen( prodid );
status = CDA_STORE_ITEM_(	    &root_aggr,
			    &aggr_handle,
			    &aggregate_item,
			    &length,	    /* Item length	*/
			    prodid,	    /* Item value	*/
			    0, 0 );
LOWBIT_TEST_( status );

/*
** Store Product Name
*/
#if defined(NEW_CDA_SYMBOLS)
aggregate_item = DDIF_DSC_PRODUCT_NAME;
#else
aggregate_item = DDIF$_DSC_PRODUCT_NAME;
#endif

length = strlen( prodnam );
aggregate_index = 0;
#if defined(NEW_CDA_SYMBOLS)
string_type = CDA_K_ISO_LATIN1;
#else
string_type = CDA$K_ISO_LATIN1;
#endif
status = CDA_STORE_ITEM_(	    &root_aggr,
			    &aggr_handle,
			    &aggregate_item,
			    &length,	     /* Item length	*/
			    prodnam,	     /* Item value	*/
			    &aggregate_index,/* Array index = 0	*/
			    &string_type     /* type of string	*/
			    );
LOWBIT_TEST_( status );

return;
} /* end of Store_ddif_descriptor_items */


/******************************************************************************
**  Store_ddif_header_items
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine stores ISL default values into the default document
**	header aggregate that is used for output.
**
**  FORMAL PARAMETERS:
**
**	root_aggr	Root aggregate handle associated with the
**			doc. desc. aggr.
**			Passed by value.
**	aggr_handle	Document header aggregate handle.
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
**  CALLED BY:
**
**	_ImgPutDDIFDsc
**
******************************************************************************/
static void Store_ddif_header_items( root_aggr, aggr_handle )
CDArootagghandle root_aggr;
CDAagghandle aggr_handle;
{
char	*ascii_time;
char	*author		    = IMG_KT_AUTHOR;
char	*title		    = IMG_KT_TITLE;
char	*version	    = IMG_KT_VERSION;
int	 aggregate_index;
CDAconstant aggregate_item;
long	 length;
CDAconstant string_type;
int	 status;
time_t	 binary_time;

/*
** Store the default title string.
*/
length = strlen( title );
aggregate_index = 0;
#if defined(NEW_CDA_SYMBOLS)
aggregate_item = DDIF_DHD_TITLE;
string_type = CDA_K_ISO_LATIN1;
#else
aggregate_item = DDIF$_DHD_TITLE;
string_type = CDA$K_ISO_LATIN1;
#endif
status = CDA_STORE_ITEM_(	    &root_aggr,
			    &aggr_handle,
			    &aggregate_item,
			    &length,	      /* Item length	*/
			    title,	      /* Item value	*/
			    &aggregate_index, /* Array index = 0	*/
			    &string_type      /* type of string	*/
			    );
LOWBIT_TEST_( status );

/*
** Store the default author string.
*/
#if defined(NEW_CDA_SYMBOLS)
aggregate_item = DDIF_DHD_AUTHOR;
string_type = CDA_K_ISO_LATIN1;
#else
aggregate_item = DDIF$_DHD_AUTHOR;
string_type = CDA$K_ISO_LATIN1;
#endif
length = strlen( author );
aggregate_index = 0;
status = CDA_STORE_ITEM_(	    &root_aggr,
			    &aggr_handle,
			    &aggregate_item,
			    &length,	    /* Item length	*/
			    author,	    /* Item value	*/
			    &aggregate_index, /* Array index = 0	*/
			    &string_type      /* type of string	*/
			    );
LOWBIT_TEST_( status );

/*
** Store the default file version string.
*/
#if defined(NEW_CDA_SYMBOLS)
aggregate_item = DDIF_DHD_VERSION;
string_type = CDA_K_ISO_LATIN1;
#else
aggregate_item = DDIF$_DHD_VERSION;
string_type = CDA$K_ISO_LATIN1;
#endif
length = strlen( version );
aggregate_index = 0;
status = CDA_STORE_ITEM_(	    &root_aggr,
			    &aggr_handle,
			    &aggregate_item,
			    &length,	    /* Item length	*/
			    version,	    /* Item value	*/
			    &aggregate_index,   /* Array index = 0	*/
			    &string_type        /* type of string	*/
			    );
LOWBIT_TEST_( status );

/*
** Retrieve the current date and time and store it.  Note that first
** the binary time is retrieved, which is then converted to ASCII.
*/
time( &binary_time );
ascii_time = ctime( &binary_time );

#if defined(NEW_CDA_SYMBOLS)
aggregate_item = DDIF_DHD_DATE;
string_type = CDA_K_ISO_LATIN1;
#else
aggregate_item = DDIF$_DHD_DATE;
string_type = CDA$K_ISO_LATIN1;
#endif
length = strlen( ascii_time );
aggregate_index = 0;
status = CDA_STORE_ITEM_(	    &root_aggr,
			    &aggr_handle,
			    &aggregate_item,
			    &length,	    /* Item length	*/
			    ascii_time,	    /* Item value	*/
			    &aggregate_index,   /* Array index = 0	*/
			    &string_type        /* type of string	*/
			    );
LOWBIT_TEST_( status );

return;
} /* end of Store_ddif_header_items */
