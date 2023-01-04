
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
**  IMG__IO_MGT.C
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
**      VAX/VMS, VAX/Ultrix, RISC/Ultrix
**
**  AUTHOR(S):
**
**	Mark W. Sornson
**
**  CREATION DATE:
**
**	1-APR-1990
**
************************************************************************/

/*
**  Table of contents:
**
**	Global Routines
*/
#ifdef NODAS_PROTO
long	     _ImgCloseFile();
long	     _ImgImportAction();
long	     _ImgExportAction();
struct FCB  *_ImgOpenFile();
#endif

/*
**  Include files:
*/
#if defined(__VMS) || defined(VMS)
#include        <iodef.h>
#include        <rms.h>
#include        <ssdef.h>
#endif

#include        <stdio.h>
#include	<string.h>

#include        <img/ChfDef.h>
#include	<img/ImgDef.h>
#include	<ImgDefP.h>
#include	<ImgMacros.h>
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
**
**	Literal definitions
*/
#define DEFAULT_IO_BUFSIZE  (512 * 2)
#define MAX_IO_BUFSIZE	    (512 * 127)
#define DEFAULT_FNAME	    ".DDIF"

/*
**	Structure definitions
*/

/*
**	ISL Routines			<-----   from module   ----->
*/
char	*_ImgCalloc();			/* IMG__MEMORY_MGT	*/
void	 _ImgDealloc();			/* IMG__MEMORY_MGT	*/
void	 _ImgFree();			/* IMG__MEMORY_MGT	*/
char	*_ImgMalloc();			/* IMG__MEMORY_MGT	*/

/*
**	From VAX/VMS system services
*/
#if defined(__VMS) || defined(VMS)
long	sys$clref();	    /* clear event flag	    */
long	sys$close();
long	sys$connect();
long	sys$create();
long	sys$dassgn();	    /* deassign I/O channel */
long	sys$disconnect();
long	sys$get();
long	sys$open();
long	sys$parse();
long	sys$put();
long	sys$qio();
long	sys$qiow();
long	sys$read();
long	sys$setef();	    /* set event flag	    */
long	sys$waitfr();	    /* wait for event flag  */
long	sys$write();
#endif

/*
**  External symbol definitions (status codes) currently commented out
**
*/
#include    <img/ImgStatusCodes.h>

/*
**	Module local routines
*/
#ifdef NODAS_PROTO
#if defined(__VMS) || defined(VMS)
static void Add_DDIF_file_semantics();
#endif
static long Close_file();
static long Create_file();
static void Delete_fcb();
static long Open_file();
#if defined(__VMS) || defined(VMS)
static void Qio_read_complete();    /* asynchronous IO completion routines  */
static void Qio_write_complete();
static void Rms_read_fail();
static void Rms_read_success();
static void Rms_write_fail();
static void Rms_write_success();
#endif
#else
PROTO(static long Close_file, (struct FCB */*fcb*/));
PROTO(static long Create_file, (struct FCB */*fcb*/, unsigned long /*flags*/));
PROTO(static void Delete_fcb, (struct FCB */*fcb*/));
PROTO(static long Open_file, (struct FCB */*fcb*/, unsigned long /*flags*/));
#if defined(__VMS) || defined(VMS)
PROTO(static void Qio_read_complete, (struct FCB * /*fcb*/));
PROTO(static void Qio_write_complete, (struct FCB * /*fcb*/));
PROTO(static void Rms_read_fail, (struct RAB * /*rab*/));
PROTO(static void Rms_read_success, (struct RAB * /*rab*/));
PROTO(static void Rms_write_fail, (struct RAB * /*rab*/));
PROTO(static void Rms_write_success, (struct RAB * /*rab*/));
#endif
#endif

/*
**  Global Storage:
*/


/******************************************************************************
**  _ImgCloseFile
**
**  FUNCTIONAL DESCRIPTION:
**
**	Close a file that was opened by _ImgOpenFile.
**
**  FORMAL PARAMETERS:
**
**      fcb	File context block, passed by reference.
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
**      status	Return status, passed by value.
**		This should always be success (low bit set).
**
**  SIGNAL CODES:
**
**      none.
**
**  SIDE EFFECTS:
**
**      none
**
******************************************************************************/
long _ImgCloseFile( fcb )
struct FCB  *fcb;                                                           
{
long	     status;

/*
** Close the connection to the file
*/
status = Close_file( fcb );

/*
** Deallocate the RMS and FCB structure and buffer resources
*/
Delete_fcb( fcb );

return status;
} /* end of _ImgCloseFile */
                                                        

/******************************************************************************
**  _ImgExportAction
**
**  FUNCTIONAL DESCRIPTION:
**
**	Export (or write) data out to the I/O destination (file) that
**	was opened by _ImgOpenFile.
**
**  FORMAL PARAMETERS:
**
**	iobufadr    I/O buffer address, passed by value
**	iobufcnt    I/O buffer length in bytes, passed by value
**	fcb	    File Context Block, passed by reference
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
**	status	    Status passed by by lower level system services.
**		    This should always be success (low bit set).
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
long _ImgExportAction( iobufadr, iobufcnt, fcb )
unsigned char	*iobufadr;
unsigned long	 iobufcnt;
struct FCB	*fcb;
{
long	      blk_size;
#if defined(__VMS) || defined(VMS)
long	    (*qio)()		= sys$qiow;
void	    (*qio_ast)()	= 0;
long	      qio_ast_param	= 0;
long	    (*rms_write)()	= sys$write;
#endif
long	      status;
#if defined(__VMS) || defined(VMS)
void	    (*write_fail)()	= 0;
void	    (*write_success)()	= 0;
#endif
struct IOSB  *iosb		= fcb->FcbA_QioIosb;

/*
** If the I/O is asynchronous, wait for the previous write to have
** completed.
*/
#if defined(__VMS) || defined(VMS)
if ( fcb->FcbL_Flags.FcbV_AsynchIONotDone )
    {
    status = sys$waitfr( fcb->FcbL_EventFlag );
    if ( (status & 1) != 1 )
	ChfStop( 1, status );

    /*
    ** See if the previous write failed ....
    */
    if ( (fcb->FcbL_IoStatus & 1) != 1 )
	ChfStop( 1, fcb->FcbL_IoStatus );
    }
#endif

if ( fcb->FcbL_Flags.FcbV_Asynchronous )
    /*
    ** If the previous write was complete before this action
    ** routine was called, make sure that everything was OK ...
    */
    if ( (fcb->FcbL_IoStatus & 1) == 1 )
	/*
	** OK ... set up next asynch write
	*/
	fcb->FcbL_Flags.FcbV_AsynchIONotDone = 1;
    else
	/*
	** Not OK ... stop on error
	*/
	ChfStop( 1, fcb->FcbL_IoStatus );

switch ( fcb->FcbL_AccessType )
    {
#if defined(__VMS) || defined(VMS)
    case ImgK_RmsBio:
	if ( fcb->FcbL_Flags.FcbV_Asynchronous )
	    {
	    fcb->FcbR_RmsRab->rab$v_asy = 1;
	    write_fail = Rms_write_fail;
	    write_success = Rms_write_success;
	    status = sys$clref( fcb->FcbL_EventFlag );
	    }
	else
	    fcb->FcbR_RmsRab->rab$v_asy = 0;

	fcb->FcbR_RmsRab->rab$l_bkt = fcb->FcbL_VirtualBlkPos;
	fcb->FcbR_RmsRab->rab$l_rbf = (char *)iobufadr;
	fcb->FcbR_RmsRab->rab$w_rsz = (unsigned short) iobufcnt;
	status = sys$write( fcb->FcbR_RmsRab,
			    write_fail,
			    write_success );
	break;
    case ImgK_Qio:
	/*
	** Write the data out to the file channel.
	*/
	if ( fcb->FcbL_Flags.FcbV_Asynchronous )
	    {
	    qio		    = sys$qio;
	    qio_ast	    = Qio_write_complete;
	    qio_ast_param   = (long)fcb;
	    }

	status = (*qio)( fcb->FcbL_EventFlag 	    /* event flags num	    */
			,fcb->FcbL_FileChannel	    /* dev channel num	    */
			,IO$_WRITEVBLK		    /* function code	    */
			,iosb			    /* I/O status block	    */
			,qio_ast		    /* AST routine	    */
			,qio_ast_param		    /* AST parameter	    */
			,iobufadr		    /* data buf adr	    */
			,iobufcnt		    /* data buf len	    */
			,fcb->FcbL_VirtualBlkPos    /* virtual blk position */
			,0,0,0 );
	break;
#else
    case ImgK_UltrixIo:
	status = write(	 fcb->FcbL_CFilePtr
			,iobufadr
			,iobufcnt );

	if ( status == -1 )
	    {
	    status = ImgX_FILWRITER;
	    }
	else
	    {
	    status = ImgX_SUCCESS;
	    }
	break;    
#endif
    default:
	break;
    }

/*
** Update the virtual block position file pointer and total byte count.
*/
blk_size = (iobufcnt + 511)/512;
fcb->FcbL_VirtualBlkPos += blk_size;
fcb->FcbL_TotalByteCnt += iobufcnt;

return status;
} /* end of _ImgExportAction */


/******************************************************************************
**  _ImgImportAction
**
**  FUNCTIONAL DESCRIPTION:
**
**	Import (or read) data from an I/O source (file).  This function
**	will read synchronously or asynchronously.
**
**  FORMAL PARAMETERS:
**
**	iobufadr	I/O buffer address, passed by value
**	iobuflen	I/O buffer length in bytes, pased by value
**	iobufretlen	Length, in bytes, of data returned in I/O buffer,
**			passed by reference.
**	fcb		File context block, passed by reference
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
**	status	    Return status, passed by value.
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
long _ImgImportAction( iobufadr, iobuflen, iobufretlen, fcb )
unsigned char	*iobufadr;
unsigned long	 iobuflen;
unsigned long	*iobufretlen;
struct FCB	*fcb;
{
long	      blk_size;
long	      local_bytecnt	= 0;
long	      local_retlen	= 0;
#if defined(__VMS) || defined(VMS)
long	    (*qio)()		= sys$qiow;
void	    (*qio_ast)()	= 0;
long	    (*rms_read)()	= sys$read;
void	    (*read_fail)()	= 0;
void	    (*read_success)()	= 0;
#endif
long	      status;

unsigned long bytes_read	= 0;

struct IOSB  *iosb		= fcb->FcbA_QioIosb;
struct FCB   *qio_ast_param	= 0;

/*
** Before we do anything, set the returnlen field to zero, just
** in case the routine returns with an error before doing anything.
*/
if ( iobufretlen != 0 )
    *iobufretlen = 0;

/*
** If the previous asynchronous read is not done, wait for it
*/
#if defined(__VMS) || defined(VMS)
if ( fcb->FcbL_Flags.FcbV_AsynchIONotDone )
    {
    /*
    ** Wait for event flag to signal completion of asynchronous read.
    ** NOTE that AST routine will clear the FcbV_AsynchIONotDone flag.
    */
    status = sys$waitfr( fcb->FcbL_EventFlag );
    if ( (status & 1) != 1 )
	ChfStop( 1, status );
    }

/*
** Make sure the event flag is cleared (since the RMS routine doesn't
** clear it like the qio call does).
*/
status = sys$clref( fcb->FcbL_EventFlag );
if ( (status & 1) != 1 )
    ChfStop( 1, status );
#endif

/*
** If the I/O operation is asynchronous, the previous read has now
** completed.  Update the virtual block position file pointer to
** set up the next read.
*/
if ( fcb->FcbL_Flags.FcbV_Asynchronous )
    {
    blk_size = (fcb->FcbL_IoBytesTransfered + 511)/512;
    fcb->FcbL_VirtualBlkPos += blk_size;
    fcb->FcbL_Flags.FcbV_AsynchIONotDone = 1;

    /*
    ** Return the number of bytes read by the previous read.
    */
    if ( iobufretlen != 0 )
	*iobufretlen = fcb->FcbL_IoBytesTransfered;
    }

/*
** Stop at this point if END OF FILE was found by the previous read.
*/
#if defined(__VMS) || defined(VMS)
if ( fcb->FcbL_IoStatus == SS$_ENDOFFILE ||
     fcb->FcbL_IoStatus == RMS$_EOF )
    {
    if ( fcb->FcbL_IoBytesTransfered != 0 )
	return ImgX_SUCCESS;
    else
	return fcb->FcbL_IoStatus;
    }
#endif

/*
** Set up and call the appropriate read routine.
*/
fcb->FcbA_IoBufInUse = (char *) iobufadr;

switch ( fcb->FcbL_AccessType )
    {
#if defined(__VMS) || defined(VMS)
    case ImgK_RmsBio:
	if ( fcb->FcbL_Flags.FcbV_Asynchronous )
	    {
	    fcb->FcbR_RmsRab->rab$v_asy = 1;
	    read_fail = Rms_read_fail;
	    read_success = Rms_read_success;
	    }
	else
	    fcb->FcbR_RmsRab->rab$v_asy = 0;

	fcb->FcbR_RmsRab->rab$l_bkt = fcb->FcbL_VirtualBlkPos;
	fcb->FcbR_RmsRab->rab$l_ubf = (char *)iobufadr;
	fcb->FcbR_RmsRab->rab$w_usz = (unsigned short) iobuflen;

	/*
	** Use the fab$l_ctx field to pass a parameter into 
	** the completion routines.
	*/
	fcb->FcbR_RmsFab->fab$l_ctx = (unsigned long)fcb;

	status = sys$read(  fcb->FcbR_RmsRab, 
			    read_fail, 
			    read_success );

	if ( !(fcb->FcbL_Flags.FcbV_Asynchronous) )
	    bytes_read = (unsigned long) fcb->FcbR_RmsRab->rab$w_rsz;

	break;
    case ImgK_Qio:
	if ( fcb->FcbL_Flags.FcbV_Asynchronous )
	    {
	    qio = sys$qio;
	    qio_ast = Qio_read_complete;
	    qio_ast_param = fcb;
	    }

	status = (*qio)( fcb->FcbL_EventFlag	    /* event flags num	    */
			,fcb->FcbL_FileChannel	    /* dev channel num	    */
			,IO$_READVBLK		    /* function code	    */
			,iosb			    /* I/O status block	    */
			,qio_ast		    /* AST routine	    */
			,qio_ast_param		    /* AST parameter	    */
			,iobufadr		    /* data buf adr	    */
			,iobuflen		    /* data buf len	    */
			,fcb->FcbL_VirtualBlkPos    /* virtual blk position */
			,0,0,0 );

	if ( (status & 1) != 1 )
	    ChfStop( 1, status );

	if ( !(fcb->FcbL_Flags.FcbV_Asynchronous) )
	    bytes_read = (unsigned long) iosb->IosbL_ByteCnt;
	break;
#else
    case ImgK_UltrixIo:
	status = read(	 fcb->FcbL_CFilePtr
			,iobufadr
			,iobuflen );

	switch ( status )
	    {
	    case -1:
		bytes_read = 0;
		status = ImgX_FILREADER;
		break;
	    case  0:
		bytes_read = 0;
		status = ImgX_EOF;
		break;
	    default:
		bytes_read = status;
		status = ImgX_SUCCESS;
		break;
	    }
	break;
#endif
    default:
	break;
    }

/*
** If the I/O operation was synchronous, update the virtual block
** position file pointer now that everything has completed.
*/
if ( !(fcb->FcbL_Flags.FcbV_Asynchronous) )
    {
    blk_size = (bytes_read + 511)/512;
    fcb->FcbL_VirtualBlkPos += blk_size;

    if ( iobufretlen != 0 )
	*iobufretlen = bytes_read;
    }

return status;
} /* end of _ImgImportAction */


/******************************************************************************
**  _ImgOpenFile
**
**  FUNCTIONAL DESCRIPTION:
**
**	Open a file for reads or writes.  I/O to files opened or created
**	by this routine is done in block mode (512 byte multiples), and
**	may be synchronous or asynchronous.
**
**	In addition, for flexibility (and for testing purposes), the I/O
**	may be done either by QIOs or by RMS block IO.
**
**  FORMAL PARAMETERS:
**
**      mode		    Access mode, passed by value.
**			    Defined values are:
**
**				ImgK_ModeExport
**				ImgK_ModeImport
**
**	access_type	    Access type, passed by value.
**			    Defined values are:
**
**				ImgK_Qio	(Qio access)
**				ImgK_RmsBio	(RMS block I/O access)
**				ImgK_UltrixIo	(Ultrix synch I/O)
**
**	filename_len	    Filename length in bytes, passed by value.
**	filename_buf	    Filename buffer, passed by reference.
**	rfilename_len	    Return-filename buffer length in bytes, 
**			    passed by value
**	rfilename_buf	    Return-filename buffer, passed by reference.
**	rfilename_retlen    Longword to store the length in bytes of the
**			    returned-filename, passed by reference.
**	io_bufsize	    Size, in (512 byte) blocks, to use when allocating
**			    I/O buffers.  Passed by value.
**
**			    If omitted (i.e., passed as zero), a default
**			    value of 2 blocks is used.
**
**			    For QIO access, io_bufsize may be described
**			    as a longword value.
**
**			    For RMS BIO access, io_bufsize is limitted to
**			    127 blocks, since RMS can't handle anything
**			    larger.
**
**	flags		    Special processing flags, passed by value.
**			    Defined flags are:
**
**			    ImgM_Asynchronous	makes I/O asynchronous.
**						This routine fires off a
**						read-ahead of the mode is
**						import.  Subsequent import
**						operations return the
**						results of the previous read.
**						Export operations are also
**						asynchronous, meaning that
**						they don't wait for completion
**						before returning.
**
**						I/O is synchronous by default.
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
**      status	    Returned status, passed by value.
**		    Defined return codes are:
**
**  SIGNAL CODES:
**
**		    ImgX_FILOPENER  File couldnt' be opened or created
**				    for the reason given (as a second signal)
**		    ImgX_INVFILACC  Invalid file access type (not qio or bio).
**
**
**  SIDE EFFECTS:
**
**      none
**
******************************************************************************/
struct FCB *_ImgOpenFile(   io_mode, access_type, file_type,
			    filename_len, filename_buf, 
			    rfilename_len, rfilename_buf, rfilename_retlen,
			    io_bufsize, flags )
unsigned long	 io_mode;
unsigned long	 access_type;
unsigned long	 file_type;
unsigned long	 filename_len;
unsigned char	*filename_buf;
unsigned long	 rfilename_len;
unsigned char	*rfilename_buf;
unsigned long	*rfilename_retlen;
unsigned long	 io_bufsize;	    /* size in blocks	*/
unsigned long	 flags;
{
long	     local_io_bufsize	= io_bufsize * 512;
long	     status;

struct FCB  *fcb;

/*
** Calculate buffersize; insure that it is a 512 byte multiple.
** If the access type is not QIO (i.e., RMS), clip the buffer size
** to 512 * 127, since the RMS rab buffer size field is only a word long.
*/
if ( local_io_bufsize == 0 )
    local_io_bufsize = DEFAULT_IO_BUFSIZE;
else if ( access_type != ImgK_Qio && local_io_bufsize > MAX_IO_BUFSIZE )
    local_io_bufsize = MAX_IO_BUFSIZE;
else
    /*
    ** Make sure the buffer size is a block multiple
    */
    local_io_bufsize = ((local_io_bufsize + 511)/512) * 512;

/*
** Allocate and initialize the file control block to be returned
*/
fcb = (struct FCB *)_ImgCalloc( sizeof(struct FCB), 1 );
fcb->FcbL_AccessType	    = access_type;
fcb->FcbL_EventFlag	    = 1;
fcb->FcbL_FilenameLen	    = filename_len;
fcb->FcbA_FilenameBuf	    = (char *)filename_buf;
fcb->FcbL_FileType	    = file_type;
fcb->FcbL_IoMode	    = io_mode;
fcb->FcbL_IoBufSize	    = local_io_bufsize;
fcb->FcbL_IoActionParam	    = (long) fcb;
fcb->FcbL_RfilenameLen	    = rfilename_len;
fcb->FcbA_RfilenameBuf	    = (char *) rfilename_buf;
fcb->FcbA_RfilenameRetlen   = (long *) rfilename_retlen;
fcb->FcbL_VirtualBlkPos	    = 1;

/*
** Check for FCB flags.
*/
#if defined(__VMS) || defined(VMS)
if ( (flags & ImgM_Asynchronous) != 0 )
    fcb->FcbL_Flags.FcbV_Asynchronous = 1;
#endif

/*
**  I/O buffer allocation
*/
fcb->FcbA_IoBuf1Adr = (char *) _ImgCalloc( local_io_bufsize, 1 );
fcb->FcbL_Flags.FcbV_IoBuf2InUse = 0;

#if defined(__VMS) || defined(VMS)
if ( fcb->FcbL_Flags.FcbV_Asynchronous )
    fcb->FcbA_IoBuf2Adr = _ImgCalloc( local_io_bufsize, 1 );

if ( access_type == ImgK_Qio )
    fcb->FcbA_QioIosb = (struct IOSB *)_ImgCalloc( sizeof(struct IOSB), 1 );

if ( access_type == ImgK_RmsBio || access_type == ImgK_Qio )
    {
    fcb->FcbR_RmsFab    = (struct FAB *)    _ImgCalloc( sizeof(struct FAB), 1 );
    fcb->FcbR_RmsNam    = (struct NAM *)    _ImgCalloc( sizeof(struct NAM), 1 );
    fcb->FcbR_RmsRab    = (struct RAB *)    _ImgCalloc( sizeof(struct RAB), 1 );
    fcb->FcbR_RmsXaball = (struct XABALL *) _ImgCalloc(sizeof(struct XABALL),1);

    *(fcb->FcbR_RmsFab)	    = cc$rms_fab;
    *(fcb->FcbR_RmsNam)	    = cc$rms_nam;
    *(fcb->FcbR_RmsRab)	    = cc$rms_rab;
    *(fcb->FcbR_RmsXaball)  = cc$rms_xaball;
    }
#endif

/*
** Switch on mode (import or export)
*/
switch ( io_mode )
    {
    case ImgK_ModeImport:
	status = Open_file( fcb, flags );
	break;
    case ImgK_ModeExport:
	status = Create_file( fcb, flags );
	break;
    default:
	break;
    } /* end switch ( io_mode ) */

if ( (status & 1) != 1 )
    {
    Delete_fcb( fcb );
    ChfStop( 3, ImgX_FILOPENER, 1, status );
    }

return fcb;
} /* end of _ImgOpenFile */


#if defined(__VMS) || defined(VMS)
static void Add_DDIF_file_semantics( fcb, xaball )
struct FCB	*fcb;
struct XABALL	*xaball;
{
/*
** Static storage which sets up the DDIF semantics ...
*/
static unsigned char xabitmbuf[7] = {
	    0x2B, 0x0C, 0x87, 0x73, 0x01, 0x03, 0x01 };

static struct XABITMLST {
    short	     XilW_Length;
    short	     XilW_ItemCode;
    unsigned char   *XilA_ItemAddress;
    long	     XilL_ReturnLength;
    } xabitmlst[2]  = {	 
	 { sizeof(xabitmbuf), XAB$_STORED_SEMANTICS , &xabitmbuf[0], 0 }
	,{ 0,0,0,0 }
	};
/*
** Runtime storage to point to the DDIF semantics ...
*/
struct XABITM	*xabitm	= (struct XABITM *)_ImgCalloc( sizeof(struct XABITM), 1 );

/*
** Chain XABITM block to XABALL block.
*/
xaball->xab$l_nxt = (char *)xabitm;

/*
** Set up the XABITM block ...
**
**	This includes setting the file semantics of the file
**	to be DDIF.
*/
xabitm->xab$b_cod	= XAB$C_ITM;
xabitm->xab$b_bln	= XAB$C_ITMLEN;
xabitm->xab$l_itemlist	= (char *)xabitmlst;
xabitm->xab$b_mode	= XAB$K_SETMODE;

return;
} /* end of Add_DDIF_file_semantics */
#endif


static long Close_file( fcb )
struct FCB  *fcb;                                                           
{
long	     status;

switch ( fcb->FcbL_AccessType )
    {
#if defined(__VMS) || defined(VMS)
    case ImgK_RmsBio:
	do
	    status = sys$disconnect( fcb->FcbR_RmsRab );
	while ( status == RMS$_ACT || status == RMS$_RSA );

	status = sys$close( fcb->FcbR_RmsFab );
	break;

    case ImgK_Qio:
	status = sys$dassgn( fcb->FcbL_FileChannel );
	break;
#else
    case ImgK_UltrixIo:
	status = close( fcb->FcbL_CFilePtr );
	if ( status != 0 )
	    status = ImgX_FILCLOSER;
	else
	    status = ImgX_SUCCESS;
        break;
#endif
    default:
	break;
    }

return status;
} /* end of Close_file */


static long Create_file( fcb, flags )
struct FCB	*fcb;
unsigned long	 flags;
{
char	*def_fname	= DEFAULT_FNAME;
char	*tmp_fname;

long	 def_fname_len;
long	 protection	= 0;
long	 status;

#if defined(__VMS) || defined(VMS)
struct FAB	*file_fab	= fcb->FcbR_RmsFab;
struct NAM	*file_nam	= fcb->FcbR_RmsNam;
struct RAB	*file_rab	= fcb->FcbR_RmsRab;
struct XABALL	*file_xaball	= fcb->FcbR_RmsXaball;
#endif

/*
** Initialize RMS structures
**
**  File Access Block
*/
#if defined(__VMS) || defined(VMS)
file_fab->fab$l_ctx = (unsigned long) fcb;
file_fab->fab$l_dna = def_fname;
file_fab->fab$b_dns = strlen( def_fname );
file_fab->fab$b_fac = FAB$M_PUT | FAB$M_BIO;
file_fab->fab$l_fna = fcb->FcbA_FilenameBuf;
file_fab->fab$b_fns = fcb->FcbL_FilenameLen;
file_fab->fab$l_fop = FAB$M_CBT | FAB$M_SQO | FAB$M_TEF;
file_fab->fab$w_mrs = 512;
file_fab->fab$l_nam = file_nam;
file_fab->fab$b_org = FAB$C_SEQ;
file_fab->fab$b_rat = FAB$M_BLK;
file_fab->fab$b_rfm = FAB$C_FIX;
file_fab->fab$l_xab = (char *)file_xaball;

/*
** XAB setup
**
**  Create file with the following options:
**
**	- contiguous best try
**	- on cylinder boundary
*/
file_xaball->xab$b_aop = XAB$M_CBT | XAB$M_ONC;

if ( fcb->FcbL_FileType == ImgK_FtypeDDIF )
    Add_DDIF_file_semantics( fcb, file_xaball );

/*
**  Record Access Block
*/
file_rab->rab$l_fab = file_fab;
file_rab->rab$l_bkt = 0;

/*
**  Name block (used in parsing the filename)
*/
file_nam->nam$l_esa = fcb->FcbA_RfilenameBuf;
file_nam->nam$b_ess = ( fcb->FcbL_RfilenameLen > 255? 255: 
			fcb->FcbL_RfilenameLen );

/* 
** Parse the source filename get the resultant (extended) filename. 
*/
status = sys$parse( file_fab );
if ( (status & 1) != 1 )
    return status;

*(fcb->FcbA_RfilenameRetlen) = (unsigned long)(file_nam->nam$b_esl);
#endif

/*
** Ultrix specific result filename
*/
#ifndef VMS
#endif

/*
** Switch on access type to create the file
*/
switch ( fcb->FcbL_AccessType )
    {
    /*
    ** RMS block IO access for use with RMS I/O calls.
    */
#if defined(__VMS) || defined(VMS)
    case ImgK_RmsBio:

	status = sys$create( file_fab );
	if ( (status & 1) != 1 )
	    return status;

	do
	    {
	    status = sys$connect( file_rab );
	    }
	while ( status == RMS$_ACT );

	if ( (status & 1) != 1 )
	    {
	    sys$close( file_fab );
	    return status;
	    }
	break;

    /*
    ** RMS user file open access for use with QIO I/O calls.
    */
    case ImgK_Qio:
	/*
	** User File Open (UFO) option gives I/O control to ISL 
	** after RMS opens the file.
	*/
	file_fab->fab$l_fop |= FAB$M_UFO;

	status = sys$create( file_fab );
	if ( (status & 1) != 1 )
	    return status;

	fcb->FcbL_FileChannel = file_fab->fab$l_stv;
	break;
#else
    case ImgK_UltrixIo:
	tmp_fname = (char *)
	            _ImgCalloc( (fcb->FcbL_FilenameLen + 1), sizeof(char) );
	memcpy( tmp_fname, fcb->FcbA_FilenameBuf, fcb->FcbL_FilenameLen );
	/*
	** Under ultrix, simply create the file.
	*/
	status = creat( tmp_fname, protection );
	if ( status == -1 )
	    status = ImgX_FILECRERR;
	else
	    {
	    fcb->FcbL_CFilePtr = status;
	    status = ImgX_SUCCESS;
	    }
	break;
#endif
    default:
	status = ImgX_INVFILACC;
	break;
    }

/*
** Clear the asynch IO in progress flag and set the IO status to
** success so the first asynch write (export) will not get stuck.
*/
if ( fcb->FcbL_Flags.FcbV_Asynchronous )
    {
    fcb->FcbL_Flags.FcbV_AsynchIONotDone = 0;
    fcb->FcbL_IoStatus = 1;
    }

return status;
} /* end of Create_file */


static void Delete_fcb( fcb )
struct FCB  *fcb;
{
if ( fcb->FcbA_QioIosb != 0 )
    _ImgFree( fcb->FcbA_QioIosb );

#if defined(__VMS) || defined(VMS)
_ImgFree( fcb->FcbR_RmsFab );
_ImgFree( fcb->FcbR_RmsNam );
_ImgFree( fcb->FcbR_RmsRab );
_ImgFree( fcb->FcbR_RmsXaball );
if ( fcb->FcbR_RmsXabitm != 0 )
    _ImgFree( fcb->FcbR_RmsXabitm );
#endif

if ( fcb->FcbA_IoBuf1Adr != 0 )
    _ImgFree( fcb->FcbA_IoBuf1Adr );
if ( fcb->FcbA_IoBuf2Adr != 0 )
    _ImgFree( fcb->FcbA_IoBuf2Adr );

_ImgFree( fcb );

return;
} /* end of Delete_fcb */


static long Open_file( fcb, flags )
struct FCB	*fcb;
unsigned long	 flags;
{
char	*def_fname	= DEFAULT_FNAME;
char	*tmp_fname;

long	 def_fname_len;
unsigned long	 retlen;
long	 protection	= 0;
long	 status;

#if defined(__VMS) || defined(VMS)
struct FAB  *file_fab	= fcb->FcbR_RmsFab;
struct NAM  *file_nam	= fcb->FcbR_RmsNam;
struct RAB  *file_rab	= fcb->FcbR_RmsRab;
#endif

/*
** Initialize RMS structures
**
**  File Access Block
*/
#if defined(__VMS) || defined(VMS)
file_fab->fab$l_ctx = (unsigned long) fcb;
file_fab->fab$l_dna = def_fname;
file_fab->fab$b_dns = strlen( def_fname );
file_fab->fab$b_fac = FAB$M_GET | FAB$M_BIO;
file_fab->fab$l_fna = fcb->FcbA_FilenameBuf;
file_fab->fab$b_fns = fcb->FcbL_FilenameLen ;
file_fab->fab$w_mrs = 512;
file_fab->fab$l_nam = file_nam;
file_fab->fab$b_org = FAB$C_SEQ;
file_fab->fab$b_rat = FAB$M_BLK;
file_fab->fab$b_rfm = FAB$C_FIX;

/*
**  Record Access Block
*/
file_rab->rab$l_bkt = 0;
file_rab->rab$l_fab = file_fab;
file_rab->rab$b_rac = RAB$C_SEQ;

/*
**  Name block (used in parsing the filename)
*/
file_nam->nam$l_esa = fcb->FcbA_RfilenameBuf;
file_nam->nam$b_ess = ( fcb->FcbL_RfilenameLen > 255? 255: 
			fcb->FcbL_RfilenameLen );

/* 
** Parse the source filename get the resultant (extended) filename. 
*/
status = sys$parse( file_fab );
if ( (status & 1) != 1 )
    return status;

*(fcb->FcbA_RfilenameRetlen) = (unsigned long)(file_nam->nam$b_esl);
#endif

/*
** Ultrix specific result filename
*/
#ifdef ultrix
#endif

/*
** Switch on access type to open the file
*/
switch ( fcb->FcbL_AccessType )
    {
#if defined(__VMS) || defined(VMS)
    /*
    ** RMS block IO access for use with RMS I/O calls.
    */
    case ImgK_RmsBio:

	status = sys$open( file_fab );
	if ( (status & 1) != 1 )
	    return status;

	status = sys$connect( file_rab );
	if ( (status & 1) != 1 )
	    {
	    sys$close( file_fab );
	    return status;
	    }
	break;

    /*
    ** RMS user file open access for use with QIO I/O calls.
    */
    case ImgK_Qio:
	/*
	** User File Open (UFO) option gives I/O control to ISL 
	** after RMS opens the file.
	*/
	file_fab->fab$l_fop |= FAB$M_UFO;

	status = sys$open( file_fab );
	if ( (status & 1) != 1 )
	    return status;

	fcb->FcbL_FileChannel = file_fab->fab$l_stv;
	break;
#else
    case ImgK_UltrixIo:
	tmp_fname = (char *) 
	            _ImgCalloc( (fcb->FcbL_FilenameLen + 1), sizeof(char) );
	memcpy( tmp_fname, fcb->FcbA_FilenameBuf, fcb->FcbL_FilenameLen );

	status = (long) fopen( tmp_fname, "rb" );
	if ( status == (long) NULL )
	    status = ImgX_FILOPENER;
	else
	    {
	    fcb->FcbL_CFilePtr = fileno( (FILE *)status );
	    status = ImgX_SUCCESS;
	    }
	break;
#endif
    default:
	status = ImgX_INVFILACC;
	break;
    }

/*
** If asynchronous I/O is to be used, fire off the first read
** (to get the first scoop of data ready before CDA fires off
** a request for DDIF data).
*/
if ( fcb->FcbL_Flags.FcbV_Asynchronous )
    {
    fcb->FcbL_Flags.FcbV_AsynchIONotDone = 0;
    _ImgImportAction( (unsigned char *)fcb->FcbA_IoBuf1Adr, fcb->FcbL_IoBufSize, &retlen, fcb );
    }

return status;
} /* end of Open_file */


#if defined(__VMS) || defined(VMS)
static void Qio_read_complete( fcb )
struct FCB  *fcb;
{
long	     status;
struct IOSB *iosb	= fcb->FcbA_QioIosb;

/*
** Get I/O status block information.
*/
fcb->FcbL_IoBytesTransfered = iosb->IosbL_ByteCnt;

switch ( iosb->IosbW_Status )
    {
    case SS$_ENDOFFILE:
    case SS$_NORMAL:
	fcb->FcbL_IoStatus = (long) (iosb->IosbW_Status);
	break;

    default:
	/*
	** Everything else is bad ...
	*/
	fcb->FcbL_IoStatus = (long) (iosb->IosbW_Status);
	break;
    }

/*
** Clear the flag that says that the asynch read hasn't completed.
*/
fcb->FcbL_Flags.FcbV_AsynchIONotDone = 0;

return;
} /* end of Qio_read_complete */
#endif


#if defined(__VMS) || defined(VMS)
static void Qio_write_complete( fcb )
struct FCB  *fcb;
{
struct IOSB *iosb   = fcb->FcbA_QioIosb;

printf( "Qio_write_complete, status: %d, byte-count: %d\n",
	    iosb->IosbW_Status, iosb->IosbL_ByteCnt ); 

/*
** Save the status
*/
fcb->FcbL_IoStatus = (long) iosb->IosbW_Status;

/*
** Clear the flag that says that the asynch write hasn't completed.
*/
fcb->FcbL_Flags.FcbV_AsynchIONotDone = 0;

return;
} /* end of Qio_write_complete */
#endif


#if defined(__VMS) || defined(VMS)
static void Rms_read_fail( rab )
struct RAB  *rab;
{
long	     status;
struct FAB  *fab    = rab->rab$l_fab;
struct FCB  *fcb    = (struct FCB *)fab->fab$l_ctx;

/*
** Get RAB read complete information.
*/
fcb->FcbL_IoBytesTransfered = (long)(rab->rab$w_rsz);
fcb->FcbL_IoStatus = rab->rab$l_sts;
fcb->FcbL_IoStatus2 = rab->rab$l_stv;

/*
** Set the event flag in case the import routine is waiting 
** for the IO to complete.
*/
status = sys$setef( fcb->FcbL_EventFlag );

/*
** Clear the flag that says that the asynch read hasn't completed.
*/
fcb->FcbL_Flags.FcbV_AsynchIONotDone = 0;

return;
} /* end of Rms_read_fail */
#endif


#if defined(__VMS) || defined(VMS)
static void Rms_read_success( rab )
struct RAB  *rab;
{
long	     status;
struct FAB  *fab    = rab->rab$l_fab;
struct FCB  *fcb    = (struct FCB *)fab->fab$l_ctx;

/*
** Get RAB read complete information.
*/
fcb->FcbL_IoBytesTransfered = (long)(rab->rab$w_rsz);
fcb->FcbL_IoStatus = rab->rab$l_sts;
fcb->FcbL_IoStatus2 = rab->rab$l_stv;

/*
** Set the event flag in case the import routine is waiting 
** for the IO to complete.
*/
status = sys$setef( fcb->FcbL_EventFlag );

/*
** Clear the flag that says that the asynch read hasn't completed.
*/
fcb->FcbL_Flags.FcbV_AsynchIONotDone = 0;

return;
} /* end of Rms_read_success */
#endif


#if defined(__VMS) || defined(VMS)
static void Rms_write_fail( rab )
struct RAB  *rab;
{
long	     status;
long	     status2;
struct FAB  *fab    = rab->rab$l_fab;
struct FCB  *fcb    = (struct FCB *) fab->fab$l_ctx;

/*
** Return status in the fcb.  NOTE that the AsynchIONotDone flag
** is still set (true).
*/
status = fcb->FcbL_IoStatus = rab->rab$l_sts;
status2 = fcb->FcbL_IoStatus2 = rab->rab$l_stv;

/*
** Set the event flag in case the import routine is waiting 
** for the IO to complete.
*/
if ( fcb->FcbL_IoStatus != RMS$_RSA )
    {
    status = sys$setef( fcb->FcbL_EventFlag );
    }

return;
} /* end of Rms_write_fail */
#endif


#if defined(__VMS) || defined(VMS)
static void Rms_write_success( rab )
struct RAB  *rab;
{
long	     status;
long	     status2;
struct FAB  *fab    = rab->rab$l_fab;
struct FCB  *fcb    = (struct FCB *) fab->fab$l_ctx;

/*
** Return status in the fcb.  
*/
status = fcb->FcbL_IoStatus = rab->rab$l_sts;
status2 = fcb->FcbL_IoStatus2 = rab->rab$l_stv;

/*
** Set the event flag in case the import routine is waiting 
** for the IO to complete.
*/
status = sys$setef( fcb->FcbL_EventFlag );

/*
** Clear the flag that says that the asynch write hasn't completed.
*/
fcb->FcbL_Flags.FcbV_AsynchIONotDone = 0;

return;
} /* end of Rms_write_success */
#endif
