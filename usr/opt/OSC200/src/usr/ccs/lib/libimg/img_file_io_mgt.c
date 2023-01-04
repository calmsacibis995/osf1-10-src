
/*******************************************************************************
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
*******************************************************************************/

/************************************************************************
**  IMG$_FILE_IO_MGT.C
**
**  FACILITY:
**
**      Image Services Library
**
**  ABSTRACT:
**
**      This module contains routines for managing the use of generic
**	files and data streams.  Data streams are used to pass data
**	into and out of the ISL import and export services.
**
**  ENVIRONMENT:
**
**      VAX/VMS
**
**  AUTHOR(S):
**
**      Esther Sanchez Brown
**	Mark W. Sornson
**
**  CREATION DATE:
**
**	1-APR-1990
**
**  MODIFICATION HISTORY:
**
**
************************************************************************/

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
#include	<cdaptp.h>
#else
#include	<cda_ptp.h>
#endif
#endif

/*
**  Table of contents:
**
**	VMS Specific Global Routines
*/
#if defined(__VMS) || defined(VMS)
long	     IMG$CLOSE_FILE();
struct DCB  *IMG$OPEN_FILE();
#endif
/*
**	Portable Global Routines
*/
#ifdef NODAS_PROTO
long	     ImgCloseFile();
struct DCB  *ImgOpenFile();
#endif

/*
**	Module private routines
*/
#ifdef NODAS_PROTO
static long	     Close_DDIF_file();
static struct DCB   *Open_DDIF_file();
static struct DCB   *Open_DDIF_file_for_export();
static struct DCB   *Open_DDIF_file_for_import();
#else
PROTO(static long Close_DDIF_file, (struct DCB */*dcb*/, unsigned long /*flags*/));
PROTO(static struct DCB *Open_DDIF_file, 
	(long /*access_mode*/, 
	long /*filename_len*/, 
	char */*filename_str*/, 
	struct ITMLST */*itmlst*/, 
	long /*flags*/));
PROTO(static struct DCB *Open_DDIF_file_for_export, 
	(unsigned long /*access_type*/, 
	unsigned long /*filename_len*/, 
	unsigned char */*filename_str*/, 
	unsigned long /*rfilename_len*/, 
	unsigned char */*rfilename_buf*/, 
	unsigned long */*rfilename_retlen*/, 
	unsigned long /*doc_root_aggr*/, 
	unsigned long /*dsc_aggr*/, 
	unsigned long /*dhd_aggr*/, 
	unsigned long /*io_buf_size*/, 
	unsigned long /*flags*/));
PROTO(static struct DCB *Open_DDIF_file_for_import, 
	(unsigned long /*access_type*/, 
	unsigned long /*filename_len*/, 
	unsigned char */*filename_str*/, 
	unsigned long /*rfilename_len*/, 
	unsigned char */*rfilename_buf*/, 
	unsigned long */*rfilename_retlen*/, 
	void ** /*doc_root_aggr*/, 
	void ** /*dsc_aggr*/, 
	void ** /*dhd_aggr*/, 
	unsigned long /*io_buf_size*/, 
	unsigned long /*flags*/));
#endif


/*
**  MACRO definitions:
** 
**       (see ImgMacros.h)
*/                                              

/*
**  Equated Symbols:
**
*/
#define	DEFAULT_IOBUF_BLKCNT	2

/*
**	Structure definitions
*/

/*                                
**  External References:
*/

/*
**	ISL Routines			<-----   from module   ----->
*/
#ifdef NODAS_PROTO
void	     ImgCloseDDIFFile();
struct DCB  *ImgCreateDDIFStream();	/* IMG_DDIF_IO_MGT */
void	     ImgDeleteDDIFStream();	/* IMG_DDIF_IO_MGT */
long	     ImgOpenDDIFFile();
					/* OS dependant ...*/
char	    *_ImgCalloc();
long	     _ImgCloseFile();		/* IMG__VMS_IO_MGT */
long	     _ImgExportAction();	/* IMG__VMS_IO_MGT */
long	     _ImgExtractItmlstItem();	/* IMG__ITEMLIST_UTILS	*/
long	     _ImgExtractItmlstItemAdr();/* IMG__ITEMLIST_UTILS	*/
void	     _ImgFree();
void	     _ImgGetDDIFPrefix();
long	     _ImgImportAction();	/* IMG__VMS_IO_MGT */
struct FCB  *_ImgOpenFile();		/* IMG__VMS_IO_MGT */
void	     _ImgPutDDIFPrefix();
void	     _ImgPutDocumentEnd();	/* IMG_DDIF_IO_MGT  */

/*
**	From DAS condition handling facility
*/
void	ChfSignal();		    /* signal condition handler		*/
void	ChfStop();		    /* signal condition handler to stop	*/
#endif

/*
**  External symbol definitions (status codes) currently commented out
**
*/
#include	<img/ImgStatusCodes.h>

/*
**  Global Storage:
*/


/******************************************************************************
**  IMG$CLOSE_FILE
**  ImgCloseFile
**
**  FUNCTIONAL DESCRIPTION:
**
**	Close a file that was opended by IMG$OPEN_FILE.
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
**     byte_count  Returns the number of image and file related data bytes that 
**		   was written to the file or stream.
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
long IMG$CLOSE_FILE( ctx_id, flags )
struct DCB	*ctx_id;
unsigned long	 flags;
{

return(ImgCloseFile( ctx_id, flags ));
} /* end of IMG$CLOSE_FILE */
#endif


/*******************************************************************************
** Portable entry point
*******************************************************************************/
long ImgCloseFile( dcb, flags )
struct DCB	*dcb;
unsigned long	 flags;
{
long	status;
long    byte_count;

switch (dcb->DcbL_Ftype)
    {
    case ImgK_FtypeDDIF:
	byte_count = Close_DDIF_file( dcb, flags );
	break;
    default:
        ChfStop( 1,  ImgX_INVFTYPE );    /* signal invalid file type */
    } /* end switch */
                                   
/*
** Deallocate the document context block.
**
**  This is commented out for out because Close_DDIF_file already 
**  deallocates the DCB ... the code may have to be changed when we
**  add more file types ...
**
**_ImgDeallocateDcb(dcb);
*/

return byte_count ;
} /* end of ImgCloseFile */


/******************************************************************************
**  IMG$OPEN_FILE
**  ImgOpenFile
**
**  FUNCTIONAL DESCRIPTION:
**
**	Open a file for processing.  Processing direction may be
**	read or write.
**
**  FORMAL PARAMETERS:
**
**	access_mode	Access mode (IMG$K_MODE_READ/WRITE)
**			passed by value
**                      ImgK_ModeImport
**                      ImgK_ModeExport
**
**	file_type	Access mode (IMG$K_MODE_READ/WRITE)
**			passed by value
**                      ImgK_FtypeDDIF
**
**	filename_len	Length of File name to read or write.
**			Passed by value.
**
**	filename_str	File name to read or write.
**			Passed by reference.
**
**      itmlst          GET_ITMLST, read only, by reference
**                      Item list of attributes to be used dependent on file 
**			type.
**
**	FLAGS		Flags.  
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
**	dcb	    Context identifier -- actual a DCB address.
**
**  SIGNAL CODES:
**
**	ImgX_INVARGCNT  Invalid argument count
**	ImgX_INVMD	Invalid mode (not IMG$K_MODE_READ/WRITE)
**	ImgX_INVFTYPE	Invalid filetype
**	ImgX_INVITMCOD  Invalid item code
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
#if defined(__VMS) || defined(VMS)
struct DCB *IMG$OPEN_FILE( access_mode, file_type, filename_len, filename_str, 
			    itmlst, flags )
long		 access_mode;
long		 file_type;
long		 filename_len;
char		*filename_str;
struct ITMLST	*itmlst;
long		 flags;

{
return(ImgOpenFile( access_mode, file_type, filename_len, filename_str, 
			itmlst, flags ));
} /* end of IMG$OPEN_FILE */
#endif


/*******************************************************************************
** Portable entry point
*******************************************************************************/
struct DCB *ImgOpenFile( access_mode, file_type, filename_len, filename_str, 
			    itmlst, flags )
long		 access_mode;
long		 file_type;
long		 filename_len;
char		*filename_str;
struct ITMLST	*itmlst;
long		 flags;
{
long	     index;
long	     item_found;
long	     status;

struct  ITMLST	*rootaggr_itmlst    = 0;
struct  ITMLST	*dscaggr_itmlst	    = 0;
struct  ITMLST	*dhdaggr_itmlst	    = 0;
struct  ITMLST  *rfile_itmlst	    = 0;
struct	DCB	*dcb;
struct	FCB	*fcb;

/*
** Validate input parameter values
*/
switch ( access_mode )
    {
    case ImgK_ModeImport:
    case ImgK_ModeExport:
	break;
    default:
	ChfStop( 1,  ImgX_INVACCMOD );
    }

if ( filename_len == 0 || filename_str == 0 )
    ChfStop( 1, ImgX_INVFILBUF );

/*
** Open the file ...
*/
switch ( file_type )
    {
    case ImgK_FtypeDDIF:
	dcb = Open_DDIF_file( access_mode, filename_len, filename_str, 
				itmlst, flags );
	break;
    default:
	break;
    }

dcb->DcbL_Ftype = file_type;

return dcb;
} /* end of ImgOpenFile */


static long Close_DDIF_file( dcb, flags )
struct DCB	*dcb;
unsigned long	 flags;
{
long	access_type = ImgK_Cda;
long	byte_count  = 0;
long	status;

struct FCB  *fcb    = dcb->DcbA_Fcb;

if ( fcb != 0 )
    access_type = fcb->FcbL_AccessType;

switch ( access_type )
    {
    case ImgK_Cda:
	ImgCloseDDIFFile( dcb, flags );
	break;
#if defined(__VMS) || defined(VMS)
    case ImgK_Qio:
    case ImgK_RmsBlk:
#else
    case ImgK_UltrixIo:
#endif
	/*
	** Delete the stream ...
	**
	**  NOTE that the delete stream function actually
	**  updates the FCB with the final DDIS parser location,
	**  which, essentially, is final count of the number
	**  of stream bytes which were transferred.  The FCB
	**  isn't deleted by the delete stream function.
	*/
	ImgDeleteDDIFStream( dcb );
	byte_count = fcb->FcbL_DDISParserCnt;

	/*
	** Close the file ...
	*/
	status = _ImgCloseFile( fcb );
	if ( (status & 1) != 1 )
	    ChfSignal( 3, ImgX_FILCLOSER, 1, status );
	break;
    default:
	break;
    }

return byte_count;
} /* end of Close_DDIF_file */


static struct DCB *Open_DDIF_file( access_mode, filename_len, filename_str,
				    itmlst, flags )
long		 access_mode;
long		 filename_len;
char		*filename_str;
struct ITMLST	*itmlst;
long		 flags;
{
char	    *local_retfile_bufadr;
char	    *user_retfile_bufadr;

long	     access_type	    = ImgK_Cda;
long	     dhd_aggr		    = 0;
long	     doc_root_aggr	    = 0;
long	     dsc_aggr		    = 0;
long	     index		    = 0;
long	     item_found;
long	     io_buf_size	    = DEFAULT_IOBUF_BLKCNT;
long	     local_retfile_buflen   = 255;
long	     local_retfile_retlen;
long	     user_retfile_buflen;
long	    *user_retfile_retlen;

struct DCB  *dcb;

/*
** Allocate a local buffer to store the full, returned filespec.
** Allocate an extra byte at the end to make sure the filespec
** can be read as an ASCIZ string.
*/
local_retfile_bufadr = (char *) _ImgCalloc( 1, local_retfile_buflen + 1 );

/*
**  Get access type (QIO, RMS block or record I/O, or CDA)
*/
item_found = _ImgExtractItmlstItem(  itmlst
				    ,Img_AccessType
				    ,&access_type
				    ,0
				    ,0
				    ,&index		/* index	    */
				    ,1 );		/* 1st occurance    */

/*
** Get DDIF document root, descriptor, and header aggregate handles 
** if they were specified.
*/
item_found = _ImgExtractItmlstItem(  itmlst
				    ,Img_DocRootAggr
				    ,&doc_root_aggr
				    ,0
				    ,0
				    ,&index		/* index	    */
				    ,1 );		/* 1st occurance    */

item_found = _ImgExtractItmlstItem(  itmlst
				    ,Img_DscAggr
				    ,&dsc_aggr
				    ,0
				    ,0
				    ,&index		/* index	    */
				    ,1 );		/* 1st occurance    */

item_found = _ImgExtractItmlstItem(  itmlst
				    ,Img_DhdAggr
				    ,&dhd_aggr
				    ,0
				    ,0
				    ,&index		/* index	    */
				    ,1 );		/* 1st occurance    */

/*
** Get the I/O buffer size if one was specified.
**
**	NOTE: the buffer size is specified in blocks for 
**	      ease of use with the system level I/O calls.
*/
item_found = _ImgExtractItmlstItem(  itmlst
				    ,Img_IoBufSize
				    ,&io_buf_size
				    ,0
				    ,0
				    ,&index		/* index	    */
				    ,1 );		/* 1st occurance    */

/*
** Open (or create) the file
*/
switch ( access_mode )
    {
    case ImgK_ModeExport:
	dcb = Open_DDIF_file_for_export(
					 access_type
					,filename_len
					,(unsigned char *)filename_str
					,local_retfile_buflen
					,(unsigned char *)local_retfile_bufadr
					,(unsigned long *)&local_retfile_retlen
					,doc_root_aggr
					,dsc_aggr
					,dhd_aggr
					,io_buf_size
					,flags
					);
	break;
    case ImgK_ModeImport:
	dcb = Open_DDIF_file_for_import(
					 access_type
					,filename_len
					,(unsigned char *)filename_str
					,local_retfile_buflen
					,(unsigned char *)local_retfile_bufadr
					,(unsigned long *)&local_retfile_retlen
					,(void **)doc_root_aggr
					,(void **)dsc_aggr
					,(void **)dhd_aggr
					,io_buf_size
					,flags
					);
	break;
    default:
	break;
    }
               
/*
** Does the caller want the full (resultant) filename string returned?
** If so, retrieve the caller's retfilespec buffer info from the itemlist
** and copy the local copy of the full filespec.
*/
item_found = _ImgExtractItmlstItemAdr(  
				     itmlst
				    ,Img_FullFileSpec
				    ,(long *)&user_retfile_bufadr
				    ,&user_retfile_buflen
				    ,(unsigned long int **)&user_retfile_retlen
				    ,&index		/* index	    */
				    ,1 );		/* 1st occurance    */
if ( item_found )
    {
    if ( user_retfile_buflen > 0 && user_retfile_bufadr != 0 )
	{
	memcpy( user_retfile_bufadr, local_retfile_bufadr, 
		    local_retfile_retlen );
	if ( user_retfile_retlen != 0 )
	    *user_retfile_retlen = local_retfile_retlen;
	}
    }

/*
** Deallocate local copy of the returned filespec.
*/
_ImgFree( local_retfile_bufadr );

return dcb;
} /* end of Open_DDIF_file */


static struct DCB *Open_DDIF_file_for_export( access_type, filename_len,
					      filename_str, rfilename_len,
					      rfilename_buf, rfilename_retlen,
					      doc_root_aggr, dsc_aggr, 
					      dhd_aggr, io_buf_size, flags )
unsigned long	 access_type;
unsigned long	 filename_len;
unsigned char	*filename_str;
unsigned long	 rfilename_len;
unsigned char	*rfilename_buf;
unsigned long	*rfilename_retlen;
unsigned long	 doc_root_aggr;
unsigned long	 dsc_aggr;
unsigned long	 dhd_aggr;
unsigned long	 io_buf_size;		/* size in blocks   */
unsigned long	 flags;
{
struct DCB  *dcb;
struct FCB  *fcb;

/*
** Verify the access type
*/
switch ( access_type )
    {
    case ImgK_Cda:
#ifdef vms
    case ImgK_RmsBio:
#else
    case ImgK_UltrixIo:
#endif
	break;
    default:
	ChfStop( 1, ImgX_INVACCTYP );
    }

switch ( access_type )
    {
    case ImgK_Cda:
	flags |= ImgM_InhibitPrefixIO;
	dcb = ImgOpenDDIFFile(	 ImgK_ModeExport
				,filename_len
				,(char *)filename_str
				,rfilename_len
				,(char *)rfilename_buf
				,flags );

	/*
	** Get the length of the returned filespec by assuming
	** that the buffer is big enough, and was initially zeroed,
	** so that that ASCIZ string strlen function works ...
	*/
	if ( rfilename_retlen != 0 )
	    *rfilename_retlen = strlen( (char *)rfilename_buf );
	break;
#if defined(__VMS) || defined(VMS)
    case ImgK_RmsBio:
#else
    case ImgK_UltrixIo:
#endif
	/*
	**  Open the file, which includes allocating internal
	**  I/O action buffer.
	*/
	fcb = _ImgOpenFile(  ImgK_ModeExport
			    ,access_type
			    ,ImgK_FtypeDDIF
			    ,filename_len
			    ,filename_str
			    ,rfilename_len
			    ,rfilename_buf
			    ,rfilename_retlen
			    ,io_buf_size	/* size in blocks   */
			    ,flags );

	/*
	**  Create the DDIF stream context.  Associate the I/O buffer
	**  allocated by the previous routine with the stream.
	*/
	flags |= ImgM_InhibitPrefixIO;
	dcb = ImgCreateDDIFStream(   ImgK_ModeExport
				    ,fcb->FcbA_IoBuf1Adr
				    ,fcb->FcbL_IoBufSize
				    ,flags
				    ,_ImgExportAction
				    ,(long)fcb );

	/*
	** Put the file context block (fcb) in the document context
	** block (dcb) so that we can recover it later at file close
	** time (since we only pass the dcb back to the user).
	*/
	dcb->DcbA_Fcb = fcb;

	break;
    default:
	break;
    }

/*
** Put the prefix (descriptor and header) to the document ...
*/
if ( dsc_aggr != 0 )
    dcb->DcbL_DscAggr = *((CDAagghandle *)dsc_aggr);
if ( dhd_aggr != 0 )
    dcb->DcbL_DhdAggr = *((CDAagghandle *)dhd_aggr);

_ImgPutDDIFPrefix( dcb );

return dcb;
} /* end of Open_DDIF_file_for_export */


static struct DCB *Open_DDIF_file_for_import( access_type, filename_len,
					      filename_str, rfilename_len,
					      rfilename_buf, rfilename_retlen,
					      doc_root_aggr, dsc_aggr, 
					      dhd_aggr, io_buf_size, flags )
unsigned long	 access_type;
unsigned long	 filename_len;
unsigned char	*filename_str;
unsigned long	 rfilename_len;
unsigned char	*rfilename_buf;
unsigned long	*rfilename_retlen;
void		**doc_root_aggr;
void		**dsc_aggr;
void		**dhd_aggr;
unsigned long	 io_buf_size;		/* size in blocks   */
unsigned long	 flags;
{
struct DCB  *dcb;
struct FCB  *fcb;

/*
** Verify the access type
*/
switch ( access_type )
    {
    case ImgK_Cda:
#if defined(__VMS) || defined(VMS)
    case ImgK_Qio:
    case ImgK_RmsBio:
#else
    case ImgK_UltrixIo:
#endif
	break;
    default:
	ChfStop( 1, ImgX_INVACCTYP );
    }

switch ( access_type )
    {
    case ImgK_Cda:
	flags |= ImgM_InhibitPrefixIO;
	dcb = (struct DCB *) ImgOpenDDIFFile(	 ImgK_ModeImport
				,filename_len
				,(char *)filename_str
				,rfilename_len
				,(char *)rfilename_buf
				,flags );

	/*
	** Get the length of the returned filespec by assuming
	** that the buffer is big enough, and was initially zeroed,
	** so that that ASCIZ string strlen function works ...
	*/
	if ( rfilename_retlen != 0 )
	    *rfilename_retlen = strlen( (char *)rfilename_buf );
	break;
#if defined(__VMS) || defined(VMS)
    case ImgK_Qio:
    case ImgK_RmsBio:
#else
    case ImgK_UltrixIo:
#endif
	/*
	**  Open the file, which includes allocating internal
	**  I/O action buffer.
	**
	**  NOTE:   If the I/O is asynchronous, the open file function
	**	    will fire off a read before the CDA get aggregate
	**	    functions start requesting data from the stream.
	*/
	fcb = _ImgOpenFile(  ImgK_ModeImport
			    ,access_type
			    ,ImgK_FtypeDDIF
			    ,filename_len
			    ,filename_str
			    ,rfilename_len
			    ,rfilename_buf
			    ,rfilename_retlen
			    ,io_buf_size	/* size in blocks   */
			    ,flags );

	/*
	**  Create the DDIF stream context.  Associate the I/O buffer
	**  allocated by the previous routine with the stream.
	*/
	flags |= ImgM_InhibitPrefixIO;
	dcb = ImgCreateDDIFStream(   ImgK_ModeImport
				    ,fcb->FcbA_IoBuf1Adr
				    ,fcb->FcbL_IoBufSize
				    ,flags
				    ,_ImgImportAction
				    ,(long)fcb );
	dcb->DcbA_Fcb = fcb;

	break;
    default:
	break;
    }

/*
** Get the prefix (descriptor and header) from the document ...
*/
_ImgGetDDIFPrefix( dcb );

if ( doc_root_aggr != 0 )
    *doc_root_aggr  = dcb->DcbL_RootAggr;
if ( dsc_aggr != 0 )
    *dsc_aggr	    = dcb->DcbL_DscAggr;
if ( dhd_aggr != 0 )
    *dhd_aggr	    = dcb->DcbL_DhdAggr;

return dcb;
} /* end of Open_DDIF_file_for_import */
