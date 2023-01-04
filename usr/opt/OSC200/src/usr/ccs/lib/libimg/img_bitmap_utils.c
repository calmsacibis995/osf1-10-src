
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
**  IMG_BITMAP_UTILS
**
**  FACILITY:
**
**      Image Services Library
**
**  ABSTRACT:
**
**	This module contains the functions for importing and exporting
**	bitmaps between application buffers and ISL frames.
**
**  ENVIRONMENT:
**
**      VAX/VMS
**
**  AUTHORS:
**
**	Mark Sornson
**
**
**  CREATION DATE:     
**
**	22 NOV 1989
**
************************************************************************/

/*
**  Include files 
*/
#include    <string.h>
#include    <img/ChfDef.h>
#include    <img/ImgDef.h>
#include    <ImgMacros.h>
#ifndef NODAS_PROTO
#include <imgprot.h>	    /* Img prototypes */
#endif

/*
**  Table of contents
**
**	VMS specific global entry points
*/
#if defined(__VMS) || defined(VMS)
struct FCT  *IMG$EXPORT_BITMAP();
struct FCT  *IMG$IMPORT_BITMAP();
#endif

/*
**	Portable global entry points
*/
#ifdef NODAS_PROTO
struct FCT  *ImgExportBitmap();
struct FCT  *ImgImportBitmap();
#endif

/*
**	Module local routines
*/
#ifdef NODAS_PROTO
static long	     img_calculate_bufsiz();
static long	     img_calculate_extsiz();
#else
PROTO(static long img_calculate_bufsiz, (struct FCT */*fid*/));
PROTO(static long img_calculate_extsiz, (struct FCT */*fid*/, long /*bytes_used*/));
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

/*
**  External References
**					    <-  from module ->
*/
#ifdef NODAS_PROTO
void		ImgDeallocateFrame();
unsigned long	ImgExportDataPlane();	    /* IMG_DATA_PLANE_UTILS	    */
unsigned long	ImgExtractRoi();	    /* IMG_ROI_UTILS		    */
void		ImgVerifyFrame();	    /* IMG_FRAME_UTILS		    */

char	    *_ImgAllocateDataPlane();	    /* IMG__DATA_PLANE_UTILS	    */
long	     _ImgExportRoi();		    /* IMG__ROI			    */
void	     _ImgFree();		    /* IMG__MEMORY_MGT		    */
struct	FCT *_ImgGet();			    /* IMG__ATTRIBUTE_ACCESS_UTILS  */
long	     _ImgGetVerifyStatus();
char	    *_ImgMalloc();		    /* IMG__MEMORY_MGT		    */
struct	FCT *_ImgPut();			    /* IMG__ATTRIBUTE_ACCESS_UTILS  */
char	    *_ImgReallocateDataPlane();
void	     _ImgStoreDataPlane();	    /* IMG__DATA_PLANE_UTILS	    */
void	     _ImgSetRoi();		    /* IMG__ROI			    */
long	     _ImgVerifyAttributes();	    /* IMG__VERIFY_UTILS	    */
long	     _ImgVerifyStructure();	    /* IMG__VERIFY_UTILS	    */

void	     ChfSignal();		    /* CHF$OBJLIB.OLB		    */
void         ChfStop();			    /* CHF$OBJLIB.OLB		    */
#endif

/*
** External symbol definitions (status codes)
**
*/
#include <img/ImgStatusCodes.h>
    
/*
**  Local Storage
**
**	none
*/


/************************************************************************
**  IMG$EXPORT_BITMAP
**  ImgExportBitmap
**
**  FUNCTIONAL DESCRIPTION:
**
**      Read an image from a frame and copy it into a buffer.
**	An action routine may also be specified to perform any action that 
**	may be required to empty the image from the output buffer.
**
**  FORMAL PARAMETERS:
**
**	    Required: srcfid - Frame id
**	    Optional: bufptr - Pointer to buffer		 
**	    Optional: buflen - Length of buffer		 
**			NOTE: Buflen is Required if bufptr is specified.
**	    Optional: bytcnt - Number of bytes transfered (by ref)
**	    Optional: action - Address of an action routine		 
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
**      fct	    Pointer to FCT originally specified as SRCFID
**
**  SIGNAL CODES:
**
**	ImgX_INVARGCNT	Invalid argument count.
**	ImgX_INVARGCON	Invalid argument condition.  Compressed frames
**			cannot be given a ROI.
**	ImgX_NOIMGDATA	No image data in the frame.
**
**  SIDE EFFECTS:
**
**      none
**
************************************************************************/
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$EXPORT_BITMAP
		(srcfid,roi_id,bufptr,buflen,bytcnt,flags,action,userprm)
struct FCT  *srcfid;
struct ROI  *roi_id;
char	    *bufptr;
int	     buflen;
int	    *bytcnt;
int	     flags;
long	    (*action)();
long	     userprm;
{
return (ImgExportBitmap
	    (srcfid,roi_id,bufptr,buflen,bytcnt,flags,action,userprm));
} /* end of IMG$EXPORT_BITMAP */
#endif


/*******************************************************************************
** Portable entry point 
*******************************************************************************/
struct FCT  *ImgExportBitmap
		(srcfid,roi_id,bufptr,buflen,bytcnt,flags,action,userprm)
struct FCT  *srcfid;
struct ROI  *roi_id;
char	    *bufptr;
long	     buflen;
long	    *bytcnt;
long	     flags;
long	    (*action)();
long	     userprm;
{
long	dummy;
long	ctype;
long	status;
struct FCT *working_fid = srcfid;

/*
**  Point bytcnt to temporary storage if undefined
*/
if (bytcnt == NULL)
    bytcnt = &dummy;

if ( VERIFY_ON_ )
    ImgVerifyFrame( srcfid, ImgM_NonstandardVerify );

/*
** If the frame is uncompressed and a ROI is specified, extract
** the roi.
*/
_ImgGet( srcfid, Img_CompressionType, &ctype, sizeof(int), 0, 0 );
if ( ctype != ImgK_PcmCompression && roi_id != 0 )
    ChfStop( 1, ImgX_INVARGCON );

if ( ctype == ImgK_PcmCompression && roi_id != 0 )
    {
    /*
    ** Extract the ROI and make a new frame.
    **
    **	NOTE: the extracted frame will be in standard format,
    **	      and thus at least byte aligned.
    */
    working_fid = ImgExtractRoi( srcfid, roi_id, flags );
    }

*bytcnt = (long)ImgExportDataPlane(	
			working_fid,
		    	0,
			(unsigned char *)bufptr,
			buflen,
			flags,
			(unsigned long (*)())action,
			userprm );

if ( working_fid != srcfid )
    ImgDeallocateFrame( working_fid );

return srcfid;
} /* end of ImgExportBitmap */


/************************************************************************
**  IMG$IMPORT_BITMAP
**  ImgImportBitmap
**
**  FUNCTIONAL DESCRIPTION:
**
**      Read in an image from a buffer and put it into a buffer that belongs
**	to a frame.  An action routine may also be specified to perform any
**	action that may be required to get the image into the input buffer.
**
**  FORMAL PARAMETERS:
**
**	    Required: srcfid - Frame id
**	    Optional: bufptr - Pointer to buffer		 
**	    Optional: buflen - Length of buffer		 
**			NOTE: Buflen is Required if bufptr is specified.
**	    Optional: bytcnt - Number of bytes transfered (by ref)
**	    Optional: action - Address of an action routine		 
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
**      fct	    Pointer to FCT originally specified as SRCFID
**
**  SIGNAL CODES:
**
**	ImgX_INVARGCNT	Invalid argument count.
**	ImgX_INVARGCON	Invalid argument condition.  Bufadr with no length
**			or no bufptr and no action routine.
**
**  SIDE EFFECTS:
**
**	If the frame does not have a buffer specified to receive the bitmap,
**	then one will be allocated for it and the DP info adjusted accordingly.
**      (i.e. the BASE and the bufsiz/arsize, whichever is applicable).
**
************************************************************************/
#if defined(__VMS) || defined(VMS)
struct FCT  *IMG$IMPORT_BITMAP(srcfid,bufptr,buflen,bytcnt,flags,action,userprm)
struct FCT  *srcfid;
char	    *bufptr;
int	     buflen;
int	    *bytcnt;
int	     flags;
long	    (*action)();
long	     userprm;
{

return(ImgImportBitmap(srcfid,bufptr,buflen,bytcnt,flags,action,userprm));
} /* end of IMG$IMPORT_BITMAP */
#endif


/*******************************************************************************
** Portable entry point
*******************************************************************************/
struct FCT  *ImgImportBitmap(srcfid,bufptr,buflen,bytcnt,flags,action,userprm)
struct FCT  *srcfid;
char	    *bufptr;
int	     buflen;
int	    *bytcnt;
int	     flags;
long	    (*action)();
long	     userprm;
{

#define BUFPTR_PARAM 2
#define BUFLEN_PARAM 3
#define BYTCNT_PARAM 4
#define FLAGS_PARAM  5
#define ACTION_PARAM 6
#define USERPRM_PARAM 7

char	 *bitmap;		/* intermediate buf to store entire bitmap  */
char	  INTERNAL_BITMAP = FALSE; /* bool for internal alloc		    */
char	  INTERNAL_BUFFER = FALSE;
int	  bitmaplen;		/* length in bytes of the entire bitmap	    */
long	  bitmapptr;		/* offset into bitmap buf for next data write */
int	  buf_extent;
int	  bytes_used;
int	  idu_aggr;		/* current Image Data Unit aggregate	    */
int	  ret_len=0;		/* Num bytes in buffer after action	    */

unsigned  long status;		/* for the action routine call		    */


if ( bufptr != NULL && buflen == 0 )
    ChfStop( 1,  ImgX_INVARGCON );
if ( bufptr == NULL && action == NULL )
    ChfStop( 1,  ImgX_INVARGCON );

if ( VERIFY_ON_ )
    {
    _ImgVerifyStructure( srcfid );
    _ImgVerifyAttributes( srcfid, ImgM_NoDataPlaneVerify );
    }

/*
**  Create an internal io buffer if the user did not pass one in.
**  The buffer will be large enough to accept the entire image
**  (POS + (S2 * LINECOUNT)).
*/
if ( bufptr == NULL )
    {
    if ( buflen == 0 )
	buflen = img_calculate_bufsiz( srcfid );
    bufptr  = _ImgMalloc( buflen );
    INTERNAL_BUFFER = TRUE;
    }

/*
** Establish the buffer to be used to store the entire imported bitmap.
** If there is no action routine, then the buffer passed in is to be used
** as the entire bitmap.  If there IS an action routine, then it has to
** be assumed that the entire bitmap will not fit in the buffer that was
** passed in; therefore allocate an internal buffer.
*/
if ( action == NULL )
    {
    bitmap = bufptr;
    bitmaplen = buflen;
    bytes_used = buflen;
    }
else
    {
    /*
    ** Allocate an internal bitmap buffer.  
    */
    bitmaplen = img_calculate_bufsiz( srcfid );
    bitmap = (char *) _ImgAllocateDataPlane( bitmaplen, 0 );
    INTERNAL_BITMAP = TRUE;
    }

/*
** Import the bitmap using the user action routine, which will fill
** the io buffer, which in turn will be copied into the bitmap buffer.
** This process will terminate when the user action routine returns
** a VMS failure status (low bit clear).
**
**  NOTE: if there is no action routine, it is assumed that
**  the entire bitmap already exists in the user buffer, so
**  this process can be skipped.
*/
if ( action != NULL )
    {
    bitmapptr = (long) bitmap;
    bytes_used = 0;
    do
	{
	/*
	** Call the user action routine.
	*/
	status = (*action)( bufptr, buflen, &ret_len, userprm );
	if (VMS_STATUS_SUCCESS_(status))
	    {
	    if ((ret_len + bytes_used) > bitmaplen )
		{
		/*
		** Reallocate the bitmap buffer (which copies the
		** data from the original buffer into the larger one).
		** Not only extend it by the amount needed, but extend
		** by the buffer extend size so that it doesn't have
		** to be reallocated too often.
		*/
		buf_extent = img_calculate_extsiz( srcfid, bytes_used );
		bitmap = (char *) _ImgReallocateDataPlane( 
				bitmap, 
				(ret_len + bytes_used + buf_extent )
				);
		bitmaplen = (ret_len + bytes_used + buf_extent);
		bitmapptr = ((long) bitmap) + bytes_used;
		}
	    /*
	    ** Copy the image data from the io buffer to the routine
	    ** local bitmap buffer, and then update the position pointers.
	    */
	    if (((long) bitmap + bytes_used + ret_len) > (bitmapptr + ret_len ) )
		ChfStop( 1,  ImgX_BUFOVRFLW );
	    memcpy( (char *)bitmapptr, bufptr, ret_len );
	    bitmapptr += ret_len;
	    bytes_used += ret_len;
	    }
	}
    while (VMS_STATUS_SUCCESS_(status));
    }

/*
** Now that we're done, realloc the internal bitmap to make it as small
** as it can be (rounded up to the nearest page boundary).
*/
if ( INTERNAL_BITMAP == TRUE )
    _ImgReallocateDataPlane( bitmap, bytes_used );

/*
** Store the full bitmap in the frame.  If the data is in a user buffer,
** call _ImgPut, which will copy the user data into the frame.  If the
** data is in the internal bitmap buffer, attach it using the special
** attach item command.
*/
if ( INTERNAL_BITMAP == TRUE )
    {
    _ImgStoreDataPlane( srcfid, bitmap );
    }
else
    /*
    ** The real put item function will do the actual data copying in
    ** this case.
    */
    _ImgPut( srcfid, Img_PlaneData, bitmap, bytes_used, 0 );

/*
** Now that we're done, delete the internally allocated buffers.
*/
if ( INTERNAL_BUFFER == TRUE )
    _ImgFree( bufptr );

/*
** Return the byte count if a return address has been given.
*/
if ( bytcnt != NULL )
    *bytcnt = bytes_used;

if ( VERIFY_ON_ )
    ImgVerifyFrame( srcfid, ImgM_NonstandardVerify );

return(srcfid);
} /* end of ImgImportBitmap */


/******************************************************************************
**  img_calculate_bufsiz
**
**  FUNCTIONAL DESCRIPTION:
**
**	Calculate the size of a work buffer or image buffer to be used.
**	The size returned is one of the following:
**
**	    - POS + (Scanline-stride * Scanline-count)
**	    - NUM_BYTES_DEFAULT
**
**  FORMAL PARAMETERS:
**
**	fid	    Frame id, passed by value
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
**	bufsiz		Buffer size, by value
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
static long img_calculate_bufsiz( fid )
struct FCT *fid;
{
long	bufsiz;
long	linecnt;
long	pos;
long	scanline_stride;
long	total;

/*
** Get size info from the frame.
*/
_ImgGet( fid, Img_DataOffset, &pos, sizeof(int), 0, 0 );
_ImgGet( fid, Img_ScanlineStride, &scanline_stride, sizeof(int), 0, 0 );
_ImgGet( fid, Img_NumberOfLines, &linecnt, sizeof(int), 0, 0 );

/*
** Get the total in bits.
*/
total = pos + (scanline_stride * linecnt );

/*
** Use total in bytes if total in bits is nonzero, else use default.
*/
bufsiz = total? (total+7)/8: NUM_BYTES_DEFAULT;

return bufsiz;
} /* end of img_calculate_bufsiz */


/******************************************************************************
**  img_calculate_extsiz
**
**  FUNCTIONAL DESCRIPTION:
**
**	Calculate the buffer extend size to be used when reallocating an
**	internal image buffer for import.  This is one of the following:
**
**		- Scanline stride
**		- number of bytes already allocated
**
**  FORMAL PARAMETERS:
**
**	fid	    Frame id, passed by value
**	bytes_used  Number of bytes already used in the current buffer
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
**	extsiz		Buffer extend size, by value
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
static long img_calculate_extsiz( fid, bytes_used )
struct FCT *fid;
long	bytes_used;
{
long	extsiz;
long	scanline_stride;

/*
** Get scanline stride in bits from the frame.
*/
_ImgGet( fid, Img_ScanlineStride, &scanline_stride, sizeof(int), 0, 0 );

/*
** If scanline stride is non-zero, use it, else use the number of bytes
** already used.  (Note: scanline stride is rounded up in bytes.)
*/
extsiz = scanline_stride? (scanline_stride+7)/8: bytes_used;

return extsiz;
} /* end of img_calculate_extsiz */
