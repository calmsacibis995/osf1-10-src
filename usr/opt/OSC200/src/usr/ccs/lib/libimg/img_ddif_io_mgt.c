
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
**  IMG$DDIF_IO_MGT.C
**
**  FACILITY:
**
**      Image Services Library
**
**  ABSTRACT:
**
**      This module contains routines for managing the use of DDIF
**	files and data streams.  Data streams are used to pass data
**	into and out of the DDIF-ISL import and export 	services.
**
**  ENVIRONMENT:
**
**      VAX/VMS
**
**  AUTHOR(S):
**
**      Mark W. Sornson 
**
**  CREATION DATE:
**
**	1-APR-1987
**
**  MODIFICATION HISTORY:
**
************************************************************************/

/*
**  Table of contents:
**
**	VMS Specific Global Routines
*/
#if defined(__VMS) || defined(VMS)
void		 IMG$CLOSE_DDIF_FILE();
struct	DCB	*IMG$CREATE_DDIF_STREAM();
void		 IMG$DELETE_DDIF_STREAM();
struct	DCB	*IMG$OPEN_DDIF_FILE();
#endif
/*
**	Portable Global Routines
*/
#ifdef NODAS_PROTO
void		 ImgCloseDDIFFile();
struct	DCB	*ImgCreateDDIFStream();
void		 ImgDeleteDDIFStream();
struct	DCB	*ImgOpenDDIFFile();
#endif

/*
**	ISL Private Global Routines
*/
#ifdef NODAS_PROTO
long		 _ImgActionFlush();
CDAstatus	 _ImgActionGet();
CDAstatus	 _ImgActionPut();

CDArootagghandle _ImgCreateRootAggregate();

void		 _ImgReallocateIOBuf();
void		 _ImgPutDocumentEnd();
#endif



/*
**  Include files:
*/
#include	<string.h>

#include        <img/ChfDef.h>
#include	<img/ImgDef.h>
#include	<ImgDefP.h>
#include	<ImgMacros.h>
#include	<ImgVectorTable.h>
#ifndef NODAS_PROTO
#include <imgprot.h>	    /* Img prototypes */
#endif

#if defined(__VMS) || defined(VMS)
#include	<cda$msg.h>
#include        <cda$ptp.h>
#include	<ddif$def.h>
#else
#if defined(NEW_CDA_SYMBOLS)
#include	<cdamsg.h>
#include	<ddifdef.h>
#else
#include	<cda_msg.h>
#include	<ddif_def.h>
#endif
#if defined(NEW_CDA_CALLS)
#include        <cdaptp.h>
#else
#include        <cda_ptp.h>
#endif
#endif

CDArootagghandle _ImgCreateRootAggregate(); /* this line was brought out of #ifdef NODAS_PROTO on 11/01/93 */

/*
**  MACRO definitions:
** 
**       (also see ImgMacros.h)
*/                                              

/*
**  Equated Symbols:
*/

/*
**	Structure definitions
*/
struct DSC {
    short    dsc_w_length;
    char     dsc_b_dtype;
    char     dsc_b_class;
    char    *dsc_a_pointer;
    };

/*                                
**  External References:
**
**	DDIF Toolkit Routines are in <cdaptp.h>
**	DDIS Toolkit Routines are in <cdaptp.h>
*/

/*
**	ISL Routines			<-----   from module   ----->
*/
#ifdef NODAS_PROTO
struct	DCB *_ImgAllocAndInitDcb();	/* IMG__BLOCK_UTILS	*/
char	    *_ImgCalloc();		/* IMG__MEMORY_MGT	*/
void	     _ImgCfree();		/* IMG__MEMORY_MGT	*/
void	     _ImgDeallocateDcb();	/* IMG__BLOCK_UILS	*/
long	     _ImgGetDDIFDsc();		/* IMG__DDIF_PREFIX_MGT	*/
long	     _ImgGetDDIFDhd();		/* IMG__DDIF_PREFIX_MGT	*/
void	     _ImgGetDDIFPrefix();	/* IMG__DDIF_PREFIX_MGT	*/
long	     _ImgExportAction();	/* IMG__IO_MGT		*/
long	     _ImgImportAction();	/* IMG__IO_MGT		*/
long	     _ImgPutDDIFDsc();		/* IMG__DDIF_PREFIX_MGT	*/
long	     _ImgPutDDIFDhd();		/* IMG__DDIF_PREFIX_MGT	*/
void	     _ImgPutDDIFPrefix();	/* IMG__DDIF_PREFIX_MGT	*/
void	     _ImgVerifyDcb();		/* IMG__BLOCK_UTILS	*/

/*
**	From VAX RTL
*/
void	ChfSignal();			/* signal condition handler */
void	ChfStop();			/* signal condition handler to stop */
#endif

/*
**  External symbol definitions (status codes) currently commented out
**
*/
#include	<img/ImgStatusCodes.h>

/*
** External pointers for CDA memory mgt
*/
#if defined(__VAXC) || defined(VAXC)
globalref CDAstatus	(*IMG_A_FREE_VM)();
globalref CDAstatus	(*IMG_A_GET_VM)();
globalref CDAuserparam	  IMG_L_MEMMGT_PARAM;
#else
extern CDAstatus	(*IMG_A_FREE_VM)();
extern CDAstatus	(*IMG_A_GET_VM)();
extern CDAuserparam	  IMG_L_MEMMGT_PARAM;
#endif

/*
**  Global Storage:
*/

/*
** Processing options for aggregate management
*/
#if defined(__VAXC) || defined(VAXC)
globaldef readonly
#endif
#if defined(NEW_CDA_SYMBOLS)
struct ITEMLIST_ELEMENT ImgR_DefaultOptions[] = {
				{0, DDIF_INHERIT_ATTRIBUTES,	    0	},
				{0, DDIF_EVALUATE_CONTENT,	    0	},
				{0,0,0}
				};
#else
struct ITEMLIST_ELEMENT ImgR_DefaultOptions[] = {
				{0, DDIF$_INHERIT_ATTRIBUTES,	    0	},
				{0, DDIF$_EVALUATE_CONTENT,	    0	},
				{0,0,0}
				};
#endif

#if defined(__VAXC) || defined(VAXC)
globaldef readonly
#endif
#if defined(NEW_CDA_SYMBOLS)
struct ITEMLIST_ELEMENT ImgR_EvaluateContentOption[] = {
				{0, DDIF_EVALUATE_CONTENT,	    0	},
				{0,0,0}
				};
#else
struct ITEMLIST_ELEMENT ImgR_EvaluateContentOption[] = {
				{0, DDIF$_EVALUATE_CONTENT,	    0	},
				{0,0,0}
				};
#endif

/******************************************************************************
**  IMG$CLOSE_DDIF_FILE
**  ImgCloseDDIFFile
**
**  FUNCTIONAL DESCRIPTION:
**
**	Close a file that was opended by IMG$OPEN_DDIF_FILE.
**
**  FORMAL PARAMETERS:
**
**	ctx-id, by value    Context identifier -- actually a DCB.
**			    Passed by value as an integer.
**
**	FLAGS, by value	    Processing flags.  OPTIONAL.
**
**			    ImgM_Abort	    Close file without flushing
**					    IO buffers and without closing
**					    DDIF stream.
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
**      ImgX_INVARGCNT - Invalid argument count
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
#if defined(__VMS) || defined(VMS)
void IMG$CLOSE_DDIF_FILE( ctx_id, flags )
struct	DCB *ctx_id;
long	flags;
{

ImgCloseDDIFFile( ctx_id, flags );

return;
} /* end of IMG$CLOSE_DDIF_FILE */
#endif


/*******************************************************************************
** Portable entry point
*******************************************************************************/
void ImgCloseDDIFFile( ctx_id, flags )
struct	DCB *ctx_id;
long	flags;
{
long	argcnt;
int	status;

/*
** Verify that the ctx-id is valid for an open file stream.
*/
_ImgVerifyDcb( ctx_id, ctx_id->DcbL_Flags.DcbV_AccessMode, DcbK_File );

/*
** Close the document protocol stream if the mode is export and the abort
** flag has not been set.
*/
if ( ctx_id->DcbL_Flags.DcbV_AccessMode == DcbK_ModeExport &&
     (flags & ImgM_Abort) == FALSE )
    _ImgPutDocumentEnd( ctx_id );

/*
** Delete the root aggregate used for non-image IO.
*/
if ( ctx_id->DcbL_Flags.DcbV_SaveRoot == FALSE )
    {
    status = CDA_DELETE_ROOT_AGGREGATE_( &(ctx_id->DcbL_RootAggr) );
    LOWBIT_TEST_( status );
    }

/*
** Close the file and file stream (all in one operation).
*/
status = CDA_CLOSE_FILE_(    &(ctx_id->DcbL_StrmCtx),
			    &(ctx_id->DcbL_FileCtx) );
LOWBIT_TEST_( status );

/*
** Deallocate the document context block.
*/
_ImgDeallocateDcb( ctx_id );

return;
} /* end of ImgCloseDDIFFile */


/******************************************************************************
**  IMG$CREATE_DDIF_STREAM
**  ImgCreateDDIFStream
**
**  FUNCTIONAL DESCRIPTION:
**
**      Open a DDIF stream for use in importing or exporting data between
**	ISL and DDIF format.
**
**  FORMAL PARAMETERS:
**
**      mode	stream mode, by value, required
**		Takes values of ImgK_ModeImport [alt. IMG$K_MODE_READ (=1)]
**		or ImgK_ModeExport [alt. IMG$K_MODE_WRITE (=2)].
**
**	BUFADR	buffer address, by reference, optional
**		Address of a buffer to be used by the import or export
**		routines.  If omitted, a buffer will be internally allocated.
**
**	BUFLEN	buffer length, by reference, optional
**		Size in bytes of a buffer to be used by the import or
**		export routines.  If omitted a default value (DcbK_DefIobExt)
**		will be used).  If bufadr is present, this parameter MUST be
**		supplied.
**
**	FLAGS	flags, by reference, optional
**		(Values TBD)
**
**	ACTION	action routine, by reference, optional
**		Action routine to fill or empty data buffer.  If bufadr is
**		omitted, this parameter is required.
**
**	USERPRM	action routine parameter, by reference, optional
**		Value passed directly to action routine, and is not processed
**		by the DDIF code.
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
**      sid	Returns stream-id in a longword.  Stream id is actually
**		the address of the DDIF context block (DCB).
**
**  SIGNAL CODES:
**
**	ImgX_INVARGCNT  Invalid argument count.
**      ImgX_BUFWNOLEN	Buffer address with no length specified.
**	ImgX_MODPRMSNG	Mode parameter missing.
**	ImgX_NOIOTARGT	No target (buffer or action routine) for IO operations.
**			Buffer address and action routine both omitted.
**
**  SIDE EFFECTS:
**
**      none
**
******************************************************************************/
#if defined(__VMS) || defined(VMS)
struct DCB *IMG$CREATE_DDIF_STREAM( mode, bufadr, buflen, flags, action, 
	    userprm )
long	  mode;			/* read or write		*/
char	 *bufadr;		/* IO buf address, optional	*/
long	  buflen;		/* IO buf len, optional		*/
long	  flags;		/* flags, optional		*/
long	(*action)();		/* User IO action rtn, optional	*/
long	  userprm;		/* User action parameter, opt.	*/
{

return (ImgCreateDDIFStream ( mode, bufadr, buflen, flags, action, userprm ));
} /* end of IMG$CREATE_DDIF_STREAM */
#endif


/*******************************************************************************
** Portable entry point
*******************************************************************************/
struct DCB *ImgCreateDDIFStream( mode, bufadr, buflen, flags, action, userprm )
long	  mode;			/* read or write		*/
char	 *bufadr;		/* IO buf address, optional	*/
long	  buflen;		/* IO buf len, optional		*/
long	  flags;		/* flags, optional		*/
long	(*action)();		/* User IO action rtn, optional	*/
long	  userprm;		/* User action parameter, opt.	*/
{
CDAsize			      size;
long			    (*put_prefix)() = 0;
int			      status;
struct	DCB		     *dcb;
struct	ITEMLIST_ELEMENT     *options	    = ImgR_DefaultOptions;

/*
**	Verify mode parameter
*/
switch ( mode )
    {
    case ImgK_ModeImport:
    case ImgK_ModeExport:
	break;
    default:
	ChfStop( 1,  ImgX_INVDDIFMD );	/* signal invalid ddif mode */
    } /* end of switch( mode ) */ 

/*
**	Verify bufadr, buflen, and action parameters.
**	Disallow bufadr with no buflen, and disallow
**	bufadr omission if action routine is missing.
*/
if ( bufadr != 0 && buflen == 0 )
    ChfStop( 1,  ImgX_BUFWNOLEN );	/* buffer with no length */
if ( bufadr == 0 && action == 0 )
    ChfStop( 1,  ImgX_NOIOTARGT );	/* no io target buf or rtn */

/*
** Allocate a DDIF context block and store the stream related
** information (whether passed in or defaulted) in the DCB.
*/
dcb = _ImgAllocAndInitDcb( mode, DcbK_Stream );

if ( buflen == 0 )				/* Store the DDIF data	*/
    {						/* buffer size.  If no	*/
    dcb->DcbL_IobLen = DcbK_DefIobExt;	/* length was supplied	*/
    dcb->DcbL_Flags.DcbV_DefSize = TRUE;	/* use a default size.	*/
    }
else
    {
    dcb->DcbL_IobLen = buflen;			/* (buflen passed in)	*/
    dcb->DcbL_Flags.DcbV_DefSize = FALSE;
    }

if (bufadr == 0 )
    {
    dcb->DcbA_IobAdr =				/* Store the IO buffer	*/
	_ImgCalloc( 1, dcb->DcbL_IobLen );	/* address.  If no buf	*/
    dcb->DcbL_Flags.DcbV_IntIoBuf = TRUE;	/* address was supplied	*/
    }						/* allocate a buffer	*/
else						/* from dynamic memory.	*/
    {
    dcb->DcbA_IobAdr = bufadr;			/* (bufadr passed in)	*/
    dcb->DcbL_Flags.DcbV_IntIoBuf = FALSE;
    }

dcb->DcbA_Action = action;			/* store action routine	*/
dcb->DcbL_IoParm = userprm;			/* store user parameter	*/
dcb->DcbA_PrefixRtn = put_prefix;		/* hook for future use	*/

if ( (flags & ImgM_SaveRoot) != FALSE )		/* save flags in dcb	*/
    dcb->DcbL_Flags.DcbV_SaveRoot = TRUE;

if ( (flags & ImgM_Noinheritance) != FALSE )
    options = ImgR_EvaluateContentOption;

/*
** Create a DDIF root aggregate for use with reads and writes
*/
dcb->DcbL_RootAggr = _ImgCreateRootAggregate( options );

/*
** Open or create the DDIF stream.  Additionally, process the
** DDIF descriptor and header aggregates.
*/
switch ( mode )
    {
    case ImgK_ModeImport:
	status = CDA_OPEN_STREAM_(  
		    IMG_A_GET_VM,
		    IMG_A_FREE_VM,
		    IMG_L_MEMMGT_PARAM,
		    _ImgActionGet,
		    dcb,		    /* _ImgActionGet param */
		    &(dcb->DcbL_StrmCtx) );
	LOWBIT_TEST_( status );

	/*
	** Process the DDIF descriptor and header.
	*/
	if ( !(flags & ImgM_InhibitPrefixIO) )
	    _ImgGetDDIFPrefix( dcb );
	break;
    case ImgK_ModeExport:
	size = dcb->DcbL_IobLen;
	status = CDA_CREATE_STREAM_(
		    IMG_A_GET_VM,
		    IMG_A_FREE_VM,
		    IMG_L_MEMMGT_PARAM,
		    _ImgActionPut,
		    dcb,		    /* _ImgActionPut param */
		    &size,
		    (CDAbufaddr)dcb->DcbA_IobAdr,
		    &(dcb->DcbL_StrmCtx) );
	LOWBIT_TEST_( status );

	/*
	** Write the DDIF descriptor and header.
	*/
	if ( !(flags & ImgM_InhibitPrefixIO) )
	    _ImgPutDDIFPrefix( dcb );
    }

return dcb;	/* returns ddif context block addr as stream id */
} /* end of ImgCreateDDIFStream */


/******************************************************************************
**  IMG$DELETE_DDIF_STREAM
**  ImgDeleteDDIFStream
**
**  FUNCTIONAL DESCRIPTION:
**
**      Close a DDIF stream established by a call to IMG$OPEN_DDIF_STREAM.
**	Deallocate the DDIF context block (dcb) and all local sub-structures
**	attached to it.
**
**  FORMAL PARAMETERS:
**                       
**      dcb/fid		Address of the ddif context block.  When treated
**			as a frame id (fid), it is considered to be passed
**			by value.  
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
**	none
**
**  SIGNAL CODES:
**
**      ImgX_INVARGCNT - Invalid argument count
**
**  SIDE EFFECTS:
**
**      none
**
******************************************************************************/
#if defined(__VMS) || defined(VMS)
void IMG$DELETE_DDIF_STREAM( dcb )
struct	DCB	*dcb;			/* stream id (doc. ctx. blk)	*/
{

ImgDeleteDDIFStream( dcb );
return;
} /* end of IMG$DELETE_DDIF_STREAM */
#endif


/*******************************************************************************
** Portable entry point
*******************************************************************************/
void ImgDeleteDDIFStream( dcb )
struct	DCB	*dcb;			/* stream id (doc. ctx. blk)	*/
{
long	     bytcnt;
long	     io_byte_count  = 0;
int	     status;

struct FCB  *fcb	    = dcb->DcbA_Fcb;

/*
** Verify that the DCB is valid
*/
_ImgVerifyDcb( dcb, dcb->DcbL_Flags.DcbV_AccessMode, 0 );

/*
** Terminate the document stream if in write mode.
*/
if (dcb->DcbL_Flags.DcbV_AccessMode == DcbK_ModeExport )
    _ImgPutDocumentEnd( dcb );

/*
** If the file control block is present, retrieve and store the
** final DDIS parse table location, which is the count of the 
** actual number of bytes of DDIS/DDIF data read or written.
*/
if ( fcb != 0 )
    {
    status = DDIS_GET_PARSE_LOCATION_(
		    &(dcb->DcbL_StrmCtx),
		    &io_byte_count );
    fcb->FcbL_DDISParserCnt = io_byte_count;
    }

/*
** Close the DDIF stream.
** 
** Deallocate the ISL DDIF context block (dbb).  This call will write out
** any leftover data in the IO buffer.
*/
status = CDA_CLOSE_STREAM_( &(dcb->DcbL_StrmCtx) );
LOWBIT_TEST_( status );

/*
** Delete the root aggregate used for non-image IO.
*/
if ( dcb->DcbL_Flags.DcbV_SaveRoot == FALSE )
    {
    status = CDA_DELETE_ROOT_AGGREGATE_( &(dcb->DcbL_RootAggr) );
    LOWBIT_TEST_( status );
    }

/*
** Deallocate ISL document context block.
*/
_ImgDeallocateDcb( dcb );

return;
} /* end of ImgDeleteDDIFStream */


/******************************************************************************
**  IMG$OPEN_DDIF_FILE
**  ImgOpenDDIFFile
**
**  FUNCTIONAL DESCRIPTION:
**
**	Open a DDIF file for processing.  Processing direction may be
**	read or write.
**
**  FORMAL PARAMETERS:
**
**	access_mode	Access mode (IMG$K_MODE_READ/WRITE)
**			passed by value
**
**	file		File name to read or write.
**			Passed by descriptor.
**
**	RFILE		Resultant full file name spec.
**			Passed by descriptor
**
**	FLAGS		Flags.  None defined.
**			Passed by value.
**
**	    NOTE: this next parameter is to be added sometime in
**	    the future.
**
**	[OPTIONS	DDIF processing options.  See DDIF spec.
**			Item list, passed by reference.]
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
**	dcb	    Context identifier -- actual a DCB address.
**
**  SIGNAL CODES:
**
**	ImgX_INVARGCNT  Invalid argument count
**	ImgX_INVDDIFMD	Invalid ddif mode (not IMG$K_MODE_READ/WRITE)
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
#if defined(__VMS) || defined(VMS)
struct DCB *IMG$OPEN_DDIF_FILE( access_mode, file, rfile, flags )
long	     access_mode;
struct DSC  *file;
struct DSC  *rfile;
long	     flags;

{
char	    *infilebuf;
char	    *retfilebuf	= 0;
long	     infilelen;
long	     retfilelen	= 0;
struct DCB  *dcb;

infilelen = file->dsc_w_length;
infilebuf = file->dsc_a_pointer;
if ( rfile != NULL )
    {
    retfilelen = rfile->dsc_w_length;
    retfilebuf = rfile->dsc_a_pointer;
    }

dcb = ImgOpenDDIFFile( access_mode, infilelen, infilebuf, 
			retfilelen, retfilebuf, flags );

return dcb;
} /* end of IMG$OPEN_DDIF_FILE */
#endif


/*******************************************************************************
** Portable entry point
*******************************************************************************/
struct DCB *ImgOpenDDIFFile( access_mode, infilelen, infilebuf, 
				retfilelen, retfilebuf, flags )
long	 access_mode;
long	 infilelen;
char	*infilebuf;
long	 retfilelen;
char	*retfilebuf;
int	 flags;
{
char			     *def_fname		= 0;
char			      rfile_buf[255];
CDAaggtype		      aggregate_type;
CDAsize			      def_fname_len	= 0;
long			      filespeclen;
long			    (*put_prefix)()	= 0;
long			      rfile_buflen	= 255;
long			      rfile_retlen;
long			      root_aggregate;
int			      status;
struct	DCB		     *dcb;
struct	ITEMLIST_ELEMENT     *options		= ImgR_DefaultOptions;

switch ( access_mode )
    {
    case ImgK_ModeImport:
    case ImgK_ModeExport:
	break;
    default:
	ChfStop( 1,  ImgX_INVDDIFMD );	/* signal invalid ddif mode */
    }

/*
** Allocate a DCB as the context identifier.
*/
dcb = _ImgAllocAndInitDcb( access_mode, DcbK_File );
dcb->DcbA_PrefixRtn = put_prefix;		/* hook for future use	*/

if ( flags & ImgM_SaveRoot != FALSE )		/* save flags in dcb	*/
    dcb->DcbL_Flags.DcbV_SaveRoot = TRUE;

if ( (flags & ImgM_Noinheritance) != FALSE )
    options = ImgR_EvaluateContentOption;

/*
** Open the file for read or write
*/
switch ( access_mode )
    {
    case ImgK_ModeImport:
#if defined(NEW_CDA_SYMBOLS)
        aggregate_type = DDIF_DDF;
#else
        aggregate_type = DDIF$_DDF;
#endif
	status = CDA_OPEN_FILE_(
		    &infilelen,
		    infilebuf,
		    &def_fname_len,		/* default file name length */
		    def_fname,			/* default file name ptr    */
		    IMG_A_GET_VM,		/* dyn mem alloc routine    */
		    IMG_A_FREE_VM,		/* dyn mem dealloc routine  */
		    IMG_L_MEMMGT_PARAM,		/* dyn mem de/alloc param   */
		    &aggregate_type,
		    (CDAitemlist *)options,
		    &rfile_buflen,		/* result file name buflen  */
		    rfile_buf,			/* result file name	    */
		    &rfile_retlen,		/* result file name length  */
		    &(dcb->DcbL_StrmCtx),
		    &(dcb->DcbL_FileCtx),
		    &(dcb->DcbL_RootAggr)
		    );
	LOWBIT_TEST_( status );

	/*
	** Process the DDIF descriptor and header.
	*/
	if ( !(flags & ImgM_InhibitPrefixIO) )
	    _ImgGetDDIFPrefix( dcb );
	break;
    case ImgK_ModeExport:
	/*
	** Create a root aggregate to pass into the CDA_CREATE_FILE_
	** routine.  This aggregate will is only a dummy, and will
	** be deleted when the file is closed.  Each export operation
	** will use root aggregates that are private to each ISL frame.
	*/
	dcb->DcbL_RootAggr = _ImgCreateRootAggregate( options );

	/*
	** Create the file.
	*/
	status = CDA_CREATE_FILE_(
		    &infilelen,
		    infilebuf,
		    &def_fname_len,		/* default file name length */
		    def_fname,			/* default file name ptr    */
		    IMG_A_GET_VM,		/* dyn mem alloc routine    */
		    IMG_A_FREE_VM,		/* dyn mem dealloc routine  */
		    IMG_L_MEMMGT_PARAM,		/* dyn mem de/alloc param   */
		    &(dcb->DcbL_RootAggr),
		    &rfile_buflen,		/* result file name buflen  */
		    rfile_buf,			/* result file name	    */
		    &rfile_retlen,		/* result file name length  */
		    &(dcb->DcbL_StrmCtx),
		    &(dcb->DcbL_FileCtx)
		    );
	LOWBIT_TEST_( status );

	/*
	** Write the DDIF descriptor and header.
	*/
	if ( !(flags & ImgM_InhibitPrefixIO) )
	    _ImgPutDDIFPrefix( dcb );
    } /* end switch */

/*
** Copy the returned file spec into the user-supplied return-filespec
** buffer
*/
if ( retfilebuf != 0 && retfilelen != 0 && rfile_retlen > 0 )
    memcpy( retfilebuf, rfile_buf, rfile_retlen );

return dcb;
} /* end of ImgOpenDDIFFile */


/******************************************************************************
**  _ImgActionFlush
**
**  FUNCTIONAL DESCRIPTION:
**
**	Stream flush call-back routine, called when a call to CDA$FLUSH_STREAM
**	is made.  This routine isn't used yet, but is included as a place
**	holder for future use.
**
**  FORMAL PARAMETERS:
**
**	dcb, by reference   Document context block.
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
long _ImgActionFlush( dcb )
struct	DCB *dcb;
{
int	status = 1;

return status;
} /* end of _ImgActionFlush */


/******************************************************************************
**  _ImgActionGet
**
**  FUNCTIONAL DESCRIPTION:
**
**	Action routine called by DDIF services fill the IO buffer used by 
**	the DDIF services.
**
**	If there is no user action routine associated with the DCB,
**	the data is retrieved directly from the IO buffer.  If there
**	is an application action routine, it is called.
**
**	NOTE that if there is no action routine, it is assumed that the
**	IO buffer contains the entire DDIF data stream.  Therefore, when
**	this is the case, this routine is only called at the beginning
**	of the IMPORT process.
**
**  FORMAL PARAMETERS:
**
**	dcb, by reference	    Document Context Block, required
**	num_bytes, by reference	    The number of bytes read.
**	buf_adr, by reference	    The buffer containing the input data.
**
**  IMPLICIT INPUTS:
**
**	Application action routine address
**	IO buffer address
**	IO buffer length
**	longword address for return of byte-count value.
**
**  IMPLICIT OUTPUTS:
**
**	DDIF data is returned in IO buffer if the action routine was called.
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	ImgX_ACTNOTSUC		The application action routine returned
**  				an error code.  The appl. code is passed
**				into the condition handler as a second signal
**				code.
**	ImgX_UNXPCTEOB		Unexpected end of buffer.  There was no
**				action routine supplied, and all the data
**				in the IO buffer was used up.
**
**  SIDE EFFECTS:
**
**	none
**
**  CALLED BY:
**
**	DDIS$GET_TAG()
**	DDIS$GET_VALUE()
**
******************************************************************************/
CDAstatus _ImgActionGet( usrparm, num_bytes, buf_adr )
CDAuserparam	usrparm;
CDAsize		*num_bytes;
CDAbufaddr	*buf_adr;
{                                                                         
struct	DCB	*dcb	= (struct DCB *)usrparm;
char	    *read_ahead_buf;
char	    *read_complete_buf;

long	     bytcnt;
CDAstatus    status = 1;

struct FCB  *fcb    = dcb->DcbA_Fcb;		/* file control block	*/

if ( dcb->DcbA_Action != 0 )			/* Is there an action rtn? */
    {
    /*
    ** If this is ISL's action routine (used privately to
    ** speed things up) AND the I/O is asynchronous ...
    */
    if ( dcb->DcbA_Action == _ImgImportAction &&
	 fcb->FcbL_Flags.FcbV_Asynchronous )

	/*
	** DOUBLE BUFFER for asynchronous I/O
	**
	**	NOTE: this assumes that a call to the action was
	**	      already fired off to read the data for the
	**	      current request by CDA.
	*/
	{

	if ( fcb->FcbA_IoBufInUse == fcb->FcbA_IoBuf1Adr )
	    {
	    read_ahead_buf	= fcb->FcbA_IoBuf2Adr;
	    read_complete_buf	= fcb->FcbA_IoBuf1Adr;
	    }
	else
	    {
	    read_ahead_buf	= fcb->FcbA_IoBuf1Adr;
	    read_complete_buf	= fcb->FcbA_IoBuf2Adr;
	    }

	/*
	** Fire off the next read-ahead ...
	*/
	status = (*(dcb->DcbA_Action))(
    		   	 read_ahead_buf
    			,dcb->DcbL_IobLen
			,&bytcnt		/* return cnt of prev. read */
     	    		,dcb->DcbL_IoParm );
	if ( (status&1) != TRUE )
	    {
	    ChfStop( 3,  ImgX_ACTNOTSUC, 1, status ); 
	    }

	/*
	** Return the number of bytes actually read (by the previous read) 
	** and return the address of the buffer they were read into.
	*/
	*num_bytes = bytcnt;
	*buf_adr = (CDAbufaddr)read_complete_buf;
	}
    else
	/*
	** SINGLE BUFFER for synchronous I/O for both ISL 
	** and application action routines.
	*/
	{
	status = (*(dcb->DcbA_Action))(
    		   	 dcb->DcbA_IobAdr
    			,dcb->DcbL_IobLen
			,&bytcnt
     	    		,dcb->DcbL_IoParm );

	/*
	** Stop if the action routine wasn't successful.
	*/
	if ( (status&1) != TRUE )
	    ChfStop( 3,  ImgX_ACTNOTSUC, 1, status ); 

	/*
	** Return the number of bytes actually read and return 
	** the address of the buffer they were read into.
	*/
	*num_bytes = bytcnt;
	*buf_adr = (CDAbufaddr)dcb->DcbA_IobAdr;
	}
    dcb->DcbL_ActionBytCnt += bytcnt;
    }
else	   					/* No. */
    {
    /*
    ** When no action routine is present, assume that the DDIF
    ** data is already in the supplied buffer.  The DDIS routines
    ** will call this routine the first time they are called.  On
    ** this first buffer read, simply set a flag indicating that
    ** the first read was done and allow this routine to return with 
    ** a successful status.  Signal an error on all subsequent calls
    ** for data when no action routine is supplied.
    **
    ** When an error is signalled, the buffer address and buffer length
    ** are passed to the condition handler which may handle the error
    ** (by refilling the data buffer) and signal to continue.
    */
    if (dcb->DcbL_Flags.DcbV_1stBufRead == 0)
    	dcb->DcbL_Flags.DcbV_1stBufRead = 1;
    else
    	ChfSignal( 4,  ImgX_UNXPCTEOB 		/* unexpected end of buffer */
    		,2
    		,dcb->DcbA_IobAdr
    	 	,dcb->DcbL_IobLen );
    
    /*
    ** If the condition handler fixed the problem and we are
    ** continuing.  Update the running byte count of all data
    ** read by the action routine.
    **
    ** Assume that the entire io buffer has been filled -- pass its length
    ** and address back to the DDIF code.
    */
    *num_bytes = dcb->DcbL_IobLen;
    *buf_adr = (CDAbufaddr)dcb->DcbA_IobAdr;

    dcb->DcbL_ActionBytCnt += dcb->DcbL_IobLen;
    }

return status;
} /* end of _ImgActionGet */


/******************************************************************************
**  _ImgActionPut
**
**  FUNCTIONAL DESCRIPTION:
**
**	Action routine called by DDIF put data routines to empty the
**	IO buffer used by the DDIF services.
**
**	If there is no user action routine associated with the DCB,
**	an error condition has resulted, since there is no way to
**	empty the buffer for reuse.
**
**	If there is no action routine, it must be assumed that the IO buffer
**	is sufficiently large to hold the entire DDIF stream.  If the IO
**	is larger than the entire DDIF stream, this routine will never be
**	called.  If the last, closing byte of the DDIF stream happens to
**	fill the buffer, this routine WILL be called, but the check on the
**	DDIF_EOC flag will prevent an error from being signalled.
**
**  FORMAL PARAMETERS:
**
**	dcb, by reference	    Document Context Block, required
**	buflen, by reference	    Number of bytes to write, required
**	bufadr, by reference	    Address of buffered data to write.
**
**	    NOTE: the buflen and bufadr argument values will always
**	    be kept in the DCB, so these parameters will never be
**	    used directly.
**
**	nextbuflen, by ref.	    Length of next buffer to use for output.
**	nextbufadr, by ref.	    Address of next buffer to use for output.
**
**	    NOTE: the nextbuflen and nextbufadr parameters will be used.
**
**
**  IMPLICIT INPUTS:
**
**	Application action routine address
**	IO buffer address
**	IO buffer length
**
**  IMPLICIT OUTPUTS:
**
**	none.
**
**  FUNCTION VALUE:                    
**
**	void (none)
**
**  SIGNAL CODES:
**
**	ImgX_ACTNOTSUC		The application action routine returned
**				an error code.  The appl. code is passed
**				into the condition handler as a second signal
**				code.
**	ImgX_UNXPCTEOB		Unexpected end of buffer.  There was no
**	     			action routine supplied, and the IO buffer
**				is full.
**
**  SIDE EFFECTS:
**
**	none
**
**  CALLED BY:
**
**	DDIS$GET_TAG()
**	DDIS$GET_VALUE()
**
******************************************************************************/
CDAstatus _ImgActionPut( usrparm, buflen, bufadr, nextbuflen, nextbufadr )
CDAuserparam	 usrparm;
CDAsize		*buflen;
CDAbufaddr	 bufadr;
CDAsize		*nextbuflen;
CDAbufaddr	*nextbufadr;

{
struct DCB  *dcb =	(struct DCB *)usrparm;
char	    *write_next_buf;
char	    *write_this_buf;
long	     parser_byte_count;
long	     position_blk[3];		    /* dummy variable	*/
long	     put_len;
CDAstatus     status		= 1;

struct FCB  *fcb		= dcb->DcbA_Fcb;

if ( dcb->DcbA_Action != 0 )		    /* Is there an action rtn? */
    {
    /*
    ** Check for end of document content condition.  Write out the
    ** entire IO buffer if we're not at the end of the document.
    */
    if ( dcb->DcbL_Flags.DcbV_DDIFEoc == FALSE )
	put_len = dcb->DcbL_IobLen;
    else
	/*
	** Calculate how much data is actually in the output buffer
	** so only that much gets written out by the action routine.
	*/
	{
	status = DDIS_GET_PARSE_LOCATION_(
		    &(dcb->DcbL_StrmCtx),
		    &parser_byte_count,
		     position_blk );
	LOWBIT_TEST_( status );

	put_len = parser_byte_count - dcb->DcbL_ActionBytCnt;
	}

    /*
    ** If this is ISL's action routine (used privately to
    ** speed things up) AND the I/O is asynchronous ...
    */
    if ( dcb->DcbA_Action == _ImgExportAction &&
	 fcb->FcbL_Flags.FcbV_Asynchronous )
	/*
	** DOUBLE BUFFER for asynchronous I/O
	*/
	{
	if ( bufadr == (CDAbufaddr)fcb->FcbA_IoBuf1Adr )
	    {
	    write_this_buf	= fcb->FcbA_IoBuf1Adr;
	    write_next_buf	= fcb->FcbA_IoBuf2Adr;
	    }
	else
	    {
	    write_this_buf	= fcb->FcbA_IoBuf2Adr;
	    write_next_buf	= fcb->FcbA_IoBuf1Adr;
	    }

	status = (*(dcb->DcbA_Action))(
    			 write_this_buf
    			,put_len
    			,dcb->DcbL_IoParm );
    
	if ( (status&1) != 1 )
		ChfStop( 3,  ImgX_ACTNOTSUC, 1, status );

	*nextbuflen = dcb->DcbL_IobLen;
	*nextbufadr = (CDAbufaddr)write_next_buf;
	}
    else
	/*
	** SINGLE BUFFER for synchronous I/O for both ISL 
	** and application action routines.
	*/
	{
	status = (*(dcb->DcbA_Action))(
    			 dcb->DcbA_IobAdr
    			,put_len
    			,dcb->DcbL_IoParm );
    
	if ( (status&1) != 1 )
	    ChfStop( 3,  ImgX_ACTNOTSUC, 1, status );/* action not successful */

	*nextbuflen = dcb->DcbL_IobLen;
	*nextbufadr = (CDAbufaddr)dcb->DcbA_IobAdr;
	}


    /*
    ** Increment the count of how many bytes have been written out by 
    ** the action routine.
    */
    dcb->DcbL_ActionBytCnt += put_len;
    }
else if (dcb->DcbL_Flags.DcbV_DDIFEoc != TRUE )
    {
    /*
    ** ERROR condition: if we aren't closing the file, and user action
    ** routine was supplied, this means that the parser has filled the
    ** buffer, but still has more data to write.  Since there's no way
    ** to empty the buffer, we can only signal an unexpected end-of-buffer
    ** condition.
    */
    ChfSignal( 4,  	 ImgX_UNXPCTEOB 	/* unexpected end of buffer */
    			,2
    			,dcb->DcbA_IobAdr
     			,dcb->DcbL_IobLen );
    }

/*
** Reallocate the buffer if it was internal (and the DDIF stream
** hasn't been closed with its final EOC).
**if (dcb->DcbL_Flags.DcbV_IntIoBuf == TRUE	 &&
**    dcb->DcbL_Flags.DcbV_DefSize == TRUE	 &&
**    dcb->DcbL_Flags.DcbV_NoBufRealloc == FALSE &&
**    dcb->DcbL_Flags.DcbV_DDIFEoc == FALSE )
**    _ImgReallocateIOBuf( dcb );
*/

return status;
} /* end of _ImgActionPut */


/******************************************************************************
**  _ImgCreateRootAggregate()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create a DDIF root aggregate.  The root will have ISL memory mgt
**	associated with it.
**
**  FORMAL PARAMETERS:
**
**	OPTIONS	    Item list of DDIF create aggregate options.
**		    This list is specified by the CDA toolkit documentation.
**		    Itemlist passed by reference.
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
CDArootagghandle _ImgCreateRootAggregate( options )
struct ITEMLIST_ELEMENT *options;
{
int   aggr_type;
CDArootagghandle root_aggregate;
int   status;

if (options == NULL)
    options = ImgR_DefaultOptions ;

#if defined(NEW_CDA_SYMBOLS)
aggr_type = DDIF_DDF;
#else
aggr_type = DDIF$_DDF;
#endif
status = CDA_CREATE_ROOT_AGGREGATE_(
		    IMG_A_GET_VM,
		    IMG_A_FREE_VM,
		    IMG_L_MEMMGT_PARAM,
		    (CDAitemlist *)options,
		    &aggr_type,
		    &root_aggregate
		    );
LOWBIT_TEST_( status );

return root_aggregate;
} /* end of _ImgCreateRootAggregate */


/******************************************************************************
**  Img__REALLOC_IOBUF
**
**  FUNCTIONAL DESCRIPTION:
**
**	Reallocate the IO buffer, extending it by the value in
**	dcb->DcbL_IobExtend.  This function is called if the IO buffer
**	has been internally allocated with the default size.  If the
**	buffer address is supplied by the application, or the buffer
**	size is specified by the application, this routine is not called.
**
**	If the attempt to allocate a larger buffer fails, the buffer size
**	value will be restored to the previous value, and an attempt will
**	be made to allocate a buffer of the previous size.  If this fails
**	as well, and error is signalled.
**
**	It is called by the internal DDIS action routines in this module.
**
**  FORMAL PARAMETERS:
**
**	dcb, by reference	Document Context Block, required.
**
**  IMPLICIT INPUTS:
**
**	 IO buffer address and size.
**
**  IMPLICIT OUTPUTS:
**
**	IO buffer address and size.
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	ImgX_IOBREALER		IO Buffer reallocation error.  Could not
**				reallocate the buffer with the next larger
**				size, and could not reallocate a buffer
**				with the original size.
**
**  SIDE EFFECTS:
**
**	none
**
**  CALLED BY:
**
**	Ddis_io_get_data()	in this module
**	Ddis_io_put_data()	in this module
**
******************************************************************************/
void _ImgReallocateIOBuf( dcb )
struct	DCB	*dcb;
{
int	status;
/*
** Deallocate the previous work buffer, increase the buffer size,
** and allocate a new one.  [realloc() isn't used because the buffer
** contents are not to be copied.]
*/
_ImgCfree( dcb->DcbA_IobAdr );
dcb->DcbL_IobLen += dcb->DcbL_IobExtend;	/* incr bufsiz by extend size */
dcb->DcbA_IobAdr = _ImgCalloc( 1, dcb->DcbL_IobLen );

/*
** If no memory was allocated, go back to the previous size
*/                                       
if (dcb->DcbA_IobAdr == 0)
    {
    dcb->DcbL_IobLen -= dcb->DcbL_IobExtend;	/* decrease the size */
    dcb->DcbA_IobAdr = _ImgCalloc( 1, dcb->DcbL_IobLen );
    
    /*
    ** If the buffer still can't be allocated, signal an error.
    */
    if (dcb->DcbA_IobAdr == 0)
    	ChfStop( 1,  ImgX_IOBREALER );
    }

return;
} /* end of _ImgReallocateIOBuf */


/******************************************************************************
**  _ImgPutDocumentEnd
**
**  FUNCTIONAL DESCRIPTION:
**
**	Put out the closing EOC tags to close the document-content
**	and ddifdocument-stream constructors.
**
**	This will call the output action routine if necessary.
**
**  FORMAL PARAMETERS:
**
**	dcb, by reference	Document Context Block, required
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	Data is written either to the IO buffer or passed into the
**	user action routine for output.
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
**	IMG$CLOSE_DDIF_STREAM	in this module
**
******************************************************************************/
void _ImgPutDocumentEnd( dcb )
struct	DCB	*dcb;

{                                             
CDAconstant     scope_code;
int	status;

/*
** Check for a previous end-of-content condition.  If found, do nothing
** but return.
*/
if (dcb->DcbL_Flags.DcbV_DDIFEoc == 0)
    {
    /*
    ** Logically close the DDIF document by leaving, in order,
    **
    **	    the root segment scope
    **	    the content scope 
    **	    and finally the document scope.
    */
#if defined(NEW_CDA_SYMBOLS)
    scope_code = DDIF_K_SEGMENT_SCOPE;
#else
    scope_code = DDIF$K_SEGMENT_SCOPE;
#endif
    status = CDA_LEAVE_SCOPE_(
		    &(dcb->DcbL_RootAggr),
		    &(dcb->DcbL_StrmCtx),
		    &scope_code );
    LOWBIT_TEST_( status );

#if defined(NEW_CDA_SYMBOLS)
    scope_code = DDIF_K_CONTENT_SCOPE;
#else
    scope_code = DDIF$K_CONTENT_SCOPE;
#endif
    status = CDA_LEAVE_SCOPE_(
		    &(dcb->DcbL_RootAggr),
		    &(dcb->DcbL_StrmCtx),
		    &scope_code );
    LOWBIT_TEST_( status );

#if defined(NEW_CDA_SYMBOLS)
    scope_code = DDIF_K_DOCUMENT_SCOPE;
#else
    scope_code = DDIF$K_DOCUMENT_SCOPE;
#endif
    status = CDA_LEAVE_SCOPE_(
		    &(dcb->DcbL_RootAggr),
		    &(dcb->DcbL_StrmCtx),
		    &scope_code );
    LOWBIT_TEST_( status );

    dcb->DcbL_Flags.DcbV_DDIFEoc = 1;	/* all done flag	*/
    }

return;
} /* end of _ImgPutDocumentEnd */
