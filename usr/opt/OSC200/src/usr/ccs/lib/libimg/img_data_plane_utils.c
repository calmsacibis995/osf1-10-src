
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
**  IMG_DATA_PLANE_UTILS
**
**  FACILITY:
**
**	Image Services Library
**
**  ABSTRACT:
**
**	This module contains the high level, public entry points into a
**	set of data plane management utility functions.  The lower level
**	functions that these are layered upon are found in the module
**	IMG__DATA_PLANE_UTILS.C.
**
**	The purpose of these functions are to allow applications greater
**	control over the image data plane that will be attached to a frame.
**
**  ENVIRONMENT:
**
**	VAX/VMS. VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
**	Mark W. Sornson
**
**  CREATION DATE:
**
**	3-OCT-1989
**
**  MODIFICATION HISTORY:
**
************************************************************************/

/*
**  Table of contents:
**
**	Global VMS veneer entry points
*/
#if defined(__VMS) || defined(VMS)
char		*IMG$ALLOCATE_DATA_PLANE();
char		*IMG$ALLOC_DATA_PLANE();
char		*IMG$DETACH_DATA_PLANE();
long		 IMG$ATTACH_DATA_PLANE();
unsigned long	 IMG$EXPORT_DATA_PLANE();
void		 IMG$FREE_DATA_PLANE();
unsigned long	 IMG$IMPORT_DATA_PLANE();
char		*IMG$REALLOCATE_DATA_PLANE();
unsigned long	 IMG$REMAP_DATA_PLANE();
long		 IMG$STORE_DATA_PLANE();
#endif

/*
**	Global portable entry points
*/
#ifdef NODAS_PROTO
char		*ImgAllocateDataPlane();
char		*ImgAllocDataPlane();
char		*ImgDetachDataPlane();
long		 ImgAttachDataPlane();
unsigned long	 ImgExportDataPlane();
void		 ImgFreeDataPlane();
unsigned long	 ImgImportDataPlane();
char		*ImgReallocateDataPlane();
unsigned long	 ImgRemapDataPlane();
long		 ImgStoreDataPlane();
#endif


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
#include    <cda$msg.h>
#include    <ddif$def.h>
#else
#if defined(NEW_CDA_SYMBOLS)
#include    <cdamsg.h>
#include    <ddifdef.h>
#else
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
**  MACRO definitions:
**
**	none
*/

/*
**  Equated Symbols:
*/
#define DEFAULT_BUFFER_SIZE	512

/*
**  External References:
**
*/
#ifdef NODAS_PROTO
void	    ChfSignal();			/* from Condition handling	*/
void	    ChfStop();			/* from Condition handling	*/

unsigned long ImgAllocateFrame();	/* from IMG_FRAME_UTILS		    */
unsigned long ImgCopyFrame();		/* from IMG_FRAME_UTILS		    */
void	      ImgDeallocateFrame();	/* from IMG_FRAME_UTILS		    */
unsigned long ImgSetRectRoi();		/* from IMG_ROI_UTILS		    */
unsigned long ImgUnsetRectRoi();	/* from IMG_ROI_UTILS		    */
void	      ImgVerifyFrame();		/* from IMG_FRAME_UTILS		    */
void	      ImgVerifyLutDef();	/* from IMG_LUT_UTILS		    */
void	      ImgVerifyRoi();		/* from IMG_ROI_UTILS		    */

char	    *_ImgAllocateDataPlane();	/* from IMG__DATA_PLANE_UTILS	    */
char	    *_ImgAllocDataPlane();	/* from IMG__DATA_PLANE_UTILS	    */
char	    *_ImgCalloc();
char	    *_ImgDetachDataPlane();	/* from IMG__DATA_PLANE_UTILS	    */
long	     _ImgAttachDataPlane();	/* from IMG__DATA_PLANE_UTILS	    */
void	     _ImgFreeDataPlane();	/* from IMG__DATA_PLANE_UTILS	    */
struct FCT  *_ImgGet();			/* from IMG__ATTRIBUTE_ACCESS_UTILS */
long	     _ImgGetVerifyStatus();
struct FCT  *_ImgPut();			/* from IMG__ATTRIBUTE_ACCESS_UTILS */
char	*    _ImgReallocateDataPlane();	/* from IMG__DATA_PLANE_UTILS	    */
void	     _ImgStoreDataPlane();	/* from IMG__DATA_PLANE_UTILS	    */
void	     _ImgVerifyDataPlane();	/* from IMG__DATA_PLANE_UTILS	    */

long	     _IpsRemapUdp();		/* from IPS__REMAP_SERVICES	    */
#endif
/*
** External status code symbols
*/
#include    <img/ImgStatusCodes.h>


/******************************************************************************
**  IMG$ALLOCATE_DATA_PLANE
**  ImgAllocateDataPlane
**
**  FUNCTIONAL DESCRIPTION:
**
**	Allocate a buffer to be used an image data plane, that can be
**	directly attached to the CDA frame structures with a call to
**	cda$attach_item.  This buffer must have its first longword reserved
**	for the remaining length of the buffer to be used for the image
**	data.
**
**  FORMAL PARAMETERS:
**
**	size		Size in bytes to be reserved for image data.
**			Passed by value.
**
**	FILL_MASK	An 8-bit mask value to be written into each
**			byte of the buffer.  The default value is 0.
**			This parameter is optional.
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
**	image_data	    Address of the start of the image data.
**			    Passed by value.
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
#if defined(__VMS) || defined(VMS)
char *IMG$ALLOCATE_DATA_PLANE( size, fill_mask )
int	size;
int	fill_mask;
{

return (ImgAllocateDataPlane( size, fill_mask ));
} /* end of IMG$ALLOCATE_DATA_PLANE */
#endif

/*******************************************************************************
** Portable entry point
*******************************************************************************/
char *ImgAllocateDataPlane( size, fill_mask )
int	size;
int	fill_mask;
{
char		*base_bufadr;
char		*image_data;
long		 actual_size;

image_data = _ImgAllocateDataPlane( size, fill_mask );

return image_data;
} /* end of ImgAllocateDataPlane */


/******************************************************************************
**  IMG$ALLOC_DATA_PLANE
**  ImgAllocDataPlane
**
**  FUNCTIONAL DESCRIPTION:
**
**	Allocate a buffer to be used an image data plane, that can be
**	directly attached to the CDA frame structures with a call to
**	cda$attach_item.  This buffer must have its first longword reserved
**	for the remaining length of the buffer to be used for the image
**	data.
**
**  FORMAL PARAMETERS:
**
**	size		Size in bytes to be reserved for image data.
**			Passed by value.
**
**	flags		flags:
**
**			    ImgM_InitMem    Initialize each byte of
**					    allocated memory with the
**					    fill mask value.
**
**	FILL_MASK	An 8-bit mask value to be written into each
**			byte of the buffer.  The default value is 0.
**			This parameter is optional.
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
**	image_data	    Address of the start of the image data.
**			    Passed by value.
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
#if defined(__VMS) || defined(VMS)
char *IMG$ALLOC_DATA_PLANE( size, flags, fill_mask )
long	size;
long	flags;
int	fill_mask;
{

return (ImgAllocDataPlane( size, flags, fill_mask ));
} /* end of IMG$ALLOC_DATA_PLANE */
#endif

/*******************************************************************************
** Portable entry point
*******************************************************************************/
char *ImgAllocDataPlane( size, flags, fill_mask )
long	size;
long	flags;
int	fill_mask;
{
char		*base_bufadr;
char		*image_data;
long		 actual_size;

image_data = _ImgAllocDataPlane( size, flags, fill_mask );

return image_data;
} /* end of ImgAllocDataPlane */


/******************************************************************************
**  IMG$DETACH_DATA_PLANE
**  ImgDetachDataPlane
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
**	pointer to data plane
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
char	*IMG$DETACH_DATA_PLANE( fid, index )
struct	    FCT	    *fid;
unsigned    long     index;
{
char *image_data = ImgDetachDataPlane( fid, index );
return image_data;
} /* end of IMG$DETACH_DATA_PLANE */
#endif

/*******************************************************************************
** Portable entry point
*******************************************************************************/
char *ImgDetachDataPlane( fid, index )
struct	    FCT	    *fid;
unsigned    long     index;
{
char *image_data;

/*
** First verify input frame
*/
if ( !VERIFY_OFF_ )
    ImgVerifyFrame( fid, ImgM_NoDataPlaneVerify|ImgM_NonstandardVerify );

image_data = _ImgDetachDataPlane( fid, index );
return image_data;
} /* end of ImgDetachDataPlane */


/******************************************************************************
**  IMG$ATTACH_DATA_PLANE
**  ImgAttachDataPlane
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
long IMG$ATTACH_DATA_PLANE( fid, data_plane, index )
long	 fid;
char	*data_plane;
long	 index;
{

return (ImgAttachDataPlane( fid, data_plane, index ));
} /* end of IMG$ATTACH_DATA_PLANE */
#endif

/*******************************************************************************
** Portable entry point
*******************************************************************************/
struct FCT *ImgAttachDataPlane( fid, data_plane, index )
struct FCT *fid;
char	*data_plane;
long	 index;
{

if ( !VERIFY_OFF_ )
    _ImgVerifyDataPlane( fid, data_plane, index );

_ImgAttachDataPlane( fid, data_plane, index );

return fid;
} /* end of ImgAttachDataPlane */


/******************************************************************************
**  IMG$EXPORT_DATA_PLANE
**  ImgExportDataPlane
**
**  FUNCTIONAL DESCRIPTION:
**
**	Export a data plane from an ISL frame into an application specified
**	buffer
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
**	byte_count - number of copied bytes of image data
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
unsigned long IMG$EXPORT_DATA_PLANE( fid, plane_idx, bufadr, buflen, flags,
				     action, usrparam)

unsigned long	fid;		/* source frame identifier		    */	
unsigned long	plane_idx;	/* index (zero based) of data plane	    */
unsigned char	*bufadr;	/* address of data buffer		    */
unsigned long	buflen;		/* length in bytes of data buffer	    */
unsigned long	flags;		/* processing flags			    */
unsigned long	(*action)();	/* application user routine		    */
long		usrparam;	/* action routine parameter(s)		    */
{

return (ImgExportDataPlane( fid, plane_idx, bufadr, buflen, flags, action,
			    usrparam));

} /* end of IMG$EXPORT_DATA_PLANE */
#endif

/*******************************************************************************
** Portable entry point
*******************************************************************************/
unsigned long ImgExportDataPlane( fid, plane_idx, bufadr, buflen, flags, 
				  action, usrparam)

struct FCT *fid;		/* source frame identifier		    */	
unsigned long	plane_idx;	/* index (zero based) of data plane	    */
unsigned char	*bufadr;	/* address of data buffer		    */
unsigned long	buflen;		/* length in bytes of data buffer	    */
unsigned long	flags;		/* processing flags			    */
unsigned long	(*action)();	/* application user routine		    */
long		usrparam;	/* action routine parameter(s)		    */
{
unsigned char	*data_plane_base; /* address of start of data plane	    */
unsigned long	data_plane_size;  /* size in bytes of data plane	    */
struct UDP	src_udp;	  /* source data plane for export operation */
/*
** Check parameters
*/
if (bufadr != 0 && buflen == 0)
    ChfStop(1, ImgX_BUFWNOLEN);		     /* buffer with no length given */

if (bufadr == 0 && action == 0)
    ChfStop(1, ImgX_PARAMCONF);			      /* parameter conflict */

/*
** Check frame and extract udp attributes
**
**	NOTE: disabled verification of standardized frame 
*/
if ( VERIFY_ON_ )
    ImgVerifyFrame(fid,ImgM_NonstandardVerify);

_ImgGet(fid, Img_Udp, &src_udp, sizeof(struct UDP), 0, plane_idx);
_ImgGet(fid, Img_DataPlaneSize, &data_plane_size, sizeof(long), 0, plane_idx);
_ImgGet(fid, Img_DataPlaneBase, &data_plane_base, sizeof(long), 0, plane_idx);

/*
** Determine whether action routine is to be called
*/
if (action != 0)
    {
    unsigned char *dp_ptr;
    unsigned long buffer_size	= (buflen == 0 ? DEFAULT_BUFFER_SIZE : buflen);
    unsigned long loop_count	= data_plane_size / buffer_size;
    unsigned long remainder	= data_plane_size % buffer_size;

    if (bufadr == 0)
	bufadr = (unsigned char *)_ImgCalloc( sizeof(char), buffer_size);

    dp_ptr = data_plane_base;
    
    if ( loop_count != 0)
	for ( ; loop_count != 0; loop_count--)
	    {
	    memcpy (bufadr, dp_ptr, buffer_size);
	    dp_ptr += buffer_size;
	    (*action)(bufadr, buffer_size, usrparam);
	    }/* end for loop */
    if ( remainder != 0)
	{
	memcpy (bufadr, dp_ptr, remainder);
	(* action)( bufadr, remainder, usrparam);
	}
    }/* end action routine specified */
else
    {
    /*
    ** no action routine specified
    */
    if (data_plane_size > buflen)
	ChfStop(1, ImgX_INVBUFLEN);
    else
        memcpy (bufadr, data_plane_base, data_plane_size);
    }/* end else no action routine */

/*
** return the number of bytes in the source udp
*/
return( data_plane_size);
} /* end of ImgExportDataPlane */


/******************************************************************************
**  IMG$FREE_DATA_PLANE
**  ImgFreeDataPlane
**
**  FUNCTIONAL DESCRIPTION:
**
**	Free an image data plane buffer from the actual base address.
**
**  FORMAL PARAMETERS:
**
**	image_data	Address of image data portion of a data plane
**			buffer.
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
#if defined(__VMS) || defined(VMS)
void IMG$FREE_DATA_PLANE( image_data )
char	*image_data;
{

ImgFreeDataPlane( image_data );
return;
} /* end of IMG$FREE_DATA_PLANE */
#endif

/*******************************************************************************
** Portable entry point
*******************************************************************************/
void ImgFreeDataPlane( image_data )
char	*image_data;
{

_ImgFreeDataPlane( image_data );
return;
} /* end of ImgFreeDataPlane */


/******************************************************************************
**  IMG$IMPORT_DATA_PLANE
**  ImgImportDataPlane
**
**  FUNCTIONAL DESCRIPTION:
**
**	Import a data plane from an an application specified buffer
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
**	byte_count - number of copied bytes of image data
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
unsigned long IMG$IMPORT_DATA_PLANE( fid, plane_idx, bufadr, buflen, flags,
				     action, usrparam)

unsigned long	fid;		/* source frame identifier		    */	
unsigned long	plane_idx;	/* index (zero based) of data plane	    */
unsigned char	*bufadr;	/* address of data buffer		    */
unsigned long	buflen;		/* length in bytes of data buffer	    */
unsigned long	flags;		/* processing flags			    */
unsigned long	(*action)();	/* application user routine		    */
long		usrparam;	/* action routine parameter(s)		    */
{

return (ImgImportDataPlane( fid, plane_idx, bufadr, buflen, flags, action,
			    usrparam));

} /* end of IMG$Import_DATA_PLANE */
#endif

/*******************************************************************************
** Portable entry point
*******************************************************************************/
unsigned long ImgImportDataPlane( fid, plane_idx, bufadr, buflen, flags, 
				  action, usrparam)

struct FCT *fid;		/* source frame identifier		    */	
unsigned long	plane_idx;	/* index (zero based) of data plane	    */
unsigned char	*bufadr;	/* address of data buffer		    */
unsigned long	buflen;		/* length in bytes of data buffer	    */
unsigned long	flags;		/* processing flags			    */
unsigned long	(*action)();	/* application user routine		    */
long		usrparam;	/* action routine parameter(s)		    */
{
unsigned long	bytecnt = 0;	  /* actual number of bytes recovered	    */
unsigned long	compression_type; /* compression type of frame		    */
unsigned char	*data_plane_base; /* address of start of data plane	    */
unsigned long	data_plane_size;  /* size in bytes of data plane	    */
unsigned long	virtual_arsize;	  /* size in bits of data plane		    */ 
struct UDP	src_udp;	  /* source data plane for export operation */
/*
** Check parameters
*/
if (bufadr != 0 && buflen == 0)
    ChfStop(1, ImgX_BUFWNOLEN);		     /* buffer with no length given */

if (bufadr == 0 && action == 0)
    ChfStop(1, ImgX_PARAMCONF);			      /* parameter conflict */

/*
** Check frame and extract udp attributes
**
**	NOTE: data plane verification has been disabled
*/
if ( VERIFY_ON_ )
    ImgVerifyFrame(fid,ImgM_NoDataPlaneVerify|ImgM_NonstandardVerify);

_ImgGet(fid, Img_Udp, &src_udp, sizeof(struct UDP), 0, plane_idx);
_ImgGet(fid, Img_CompressionType, &compression_type, sizeof(long),0,plane_idx);
_ImgGet(fid, Img_VirtualArsize, &virtual_arsize, sizeof(long),0,plane_idx);
_ImgGet(fid, Img_DataPlaneBase, &data_plane_base, sizeof(long),0,plane_idx);

/*
** Calculate data plane size independent of compression type of the data
*/
data_plane_size = (virtual_arsize + 7) / 8;			/* in bytes */

/*
** If frame has no data plane then allocate & attach one based upon virtual
** array size (i.e. scanline stride * scanlines)
*/
if (data_plane_base == 0)
    {
    if ( compression_type != ImgK_PcmCompression &&
	 buflen != 0 &&
	 action == 0 )
	/*
	** Use the value for buflen supplied by the caller as the size
	** of the data plane to allocate.  NOTE that the data is compressed
	** and we have to trust the caller.
	*/
	data_plane_size = buflen;

    data_plane_base = (unsigned char *)
			_ImgAllocDataPlane( data_plane_size, 0, 0);
    fid = _ImgAttachDataPlane( fid, (char *)data_plane_base, plane_idx);
    }/* end allocate & attach of data plane */

/*
** Determine whether action routine is to be called
*/
if (action != 0)
    {
    if (compression_type != ImgK_PcmCompression)
	ChfStop(1, ImgX_PARAMCONF);		      /* parameter conflict */
    else
	{
        unsigned char *dp_ptr;
        unsigned long buffer_size   = (buflen == 0 ? DEFAULT_BUFFER_SIZE : buflen);
        unsigned long loop_count    = data_plane_size / buffer_size;
        unsigned long remainder	    = data_plane_size % buffer_size;

        if (bufadr == 0)
	    bufadr = (unsigned char *)_ImgCalloc( sizeof(char), buffer_size);

        dp_ptr = data_plane_base;
    
        if ( loop_count != 0)
	    for ( ; loop_count != 0; loop_count--)
	        {
	        (*action)(bufadr, buffer_size, &bytecnt, usrparam);
	        memcpy (dp_ptr, bufadr, buffer_size);
	        dp_ptr += buffer_size;
	        }/* end for loop */
        if ( remainder != 0)
	    {
	    (* action)( bufadr, remainder, &bytecnt, usrparam);
	    memcpy (dp_ptr, bufadr, remainder);
	    }
	}/* end non-compressed data */
    }/* end action routine specified */
else
    {
    /*
    ** no action routine specified
    */
    if (data_plane_size > buflen)
	ChfStop(1, ImgX_INVBUFLEN);
    else
        memcpy (data_plane_base, bufadr, data_plane_size);

    bytecnt = data_plane_size;
    }/* end else no action routine */

/*
** return the number of bytes in the source udp
*/
return( bytecnt);
} /* end of ImgImportDataPlane */

/******************************************************************************
**  IMG$REALLOCATE_DATA_PLANE
**  ImgReallocateDataPlane
**
**  FUNCTIONAL DESCRIPTION:
**
**	Reallocate a data plane buffer, making it larger or smaller.
**	This function manages the reallocation of the buffer and its
**	length field.
**
**  FORMAL PARAMETERS:
**
**	image_data	Address of image data to supply reallocated buffer
**			for.
**			Passed by value.
**
**	new_size	Size in bytes to make the buffer.
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
#if defined(__VMS) || defined(VMS)
char *IMG$REALLOCATE_DATA_PLANE( image_data, new_size )
char	*image_data;
int	 new_size;
{

return (ImgReallocateDataPlane( image_data, new_size ));
} /* end of IMG$REALLOCATE_DATA_PLANE */
#endif
/*******************************************************************************
** Portable entry point
*******************************************************************************/
char *ImgReallocateDataPlane( image_data, new_size )
char	*image_data;
int	 new_size;
{
char	*base_bufadr;
char	*newbase_bufadr;


newbase_bufadr = _ImgReallocateDataPlane( image_data, new_size );

return newbase_bufadr;
} /* end of _ImgReallocateDataPlane */


/******************************************************************************
**  IMG$REMAP_DATA_PLANE
**  ImgRemapDataPlane
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
******************************************************************************
** VMS-specific Entry Point
******************************************************************************/
#if defined(__VMS) || defined(VMS)
unsigned long IMG$REMAP_DATA_PLANE( srcfid, dp_idx, lut, roi, flags )
unsigned long	srcfid;
unsigned long	dp_idx;
unsigned long	lut;
unsigned long	roi;
unsigned long	flags;
{

return (ImgRemapDataPlane( srcfid, dp_idx, lut, roi, flags ));
} /* end of IMG$REMAP_DATA_PLANE */
#endif

/*****************************************************************************
** Portable Entry Point
******************************************************************************/
struct FCT *ImgRemapDataPlane( srcfid, dp_idx, lut, roi, flags )
struct FCT *srcfid;
unsigned long	 dp_idx;
struct LTD	*lut;
struct ROI *roi;
unsigned long	 flags;
{
long	status;

unsigned long	data_class;
unsigned long	local_flags = 0;
struct FCT *retfid;

struct UDP	retudp;
struct UDP	srcudp;

/*
** Validate input args (incl. conformance checking)
*/
if ( VERIFY_ON_ )
    ImgVerifyFrame( srcfid, 0 );

if ( roi != 0 )
    ImgVerifyRoi( roi, srcfid, local_flags );

ImgVerifyLutDef( lut, local_flags, 0, 0 );

_ImgGet( srcfid, Img_ImageDataClass, &data_class, LONGSIZE, 0, 0 );
switch ( data_class )
    {
    case ImgK_ClassGreyscale:
    case ImgK_ClassMultispect:
	break;
    default:
	ChfStop( 1, ImgX_UNSDATACL );
	break;
    } 

if ( lut->LutL_TableType != ImgK_LtypeImplicitIndex )
    ChfStop( 1, ImgX_UNSLTABTY );

/*
** Set up the return frame:
**
**  If IN PLACE specified, use the source frame
**  else if a roi was specified, copy the source frame
**  else allocate a new frame (since the entire image is being remapped)
*/
if ( (flags & ImgM_InPlace) )
    retfid = srcfid;
else 
    retfid = ImgCopyFrame( srcfid, 0 );

/*
** For now, since only rectangular ROIs are supported, set the
** rect. roi. directly onto the src frame and the ret frame, so that 
** the UDP will be properly remapped.
*/
ImgSetRectRoi( srcfid, roi, 0 );

if (retfid != srcfid)
    ImgSetRectRoi( retfid, roi, 0 );

/*
** Get the src and ret udps, and remap the source udp using the
** lookup table.
*/
_ImgGet( srcfid, Img_Udp, &srcudp, sizeof( srcudp ), 0, dp_idx );
_ImgGet( retfid, Img_Udp, &retudp, sizeof( retudp ), 0, dp_idx );

local_flags = IpsM_RetainSrcDim;
status = _IpsRemapUdp(	&srcudp,
			&retudp,
			0,		/* no cpp for now    */
			(unsigned long *)lut->LutA_Table,
			lut->LutL_EntryCount,
			lut->LutL_EntryDataType,
			local_flags );

if ( !(status & 1) )
    {
    ImgDeallocateFrame( retfid );
    ChfStop( 3, ImgX_DPREMAPER, 1, status );
    }

/*
** Final cleanup and frame return checking ...
*/
ImgUnsetRectRoi( srcfid );

if( retfid != srcfid)
    ImgUnsetRectRoi( retfid );

if ( VERIFY_ON_ )
    ImgVerifyFrame( retfid, 0 );

return retfid;
} /* end of ImgRemapDataPlane */


/******************************************************************************
**  IMG$STORE_DATA_PLANE
**  ImgStoreDataPlane
**
**  FUNCTIONAL DESCRIPTION:
**
**	Store a data plane buffer in the frame, attaching it directly to
**	to the internal CDA structure (rather than copying it into the
**	frame).
**
**	This function erases any existing data in the frame before
**	attaching the new data (since this is required).
**
**  FORMAL PARAMETERS:
**
**	fid	    Frame id of frame to attach data plane to.
**		    Passed by value.
**
**	image_data  Address of image data to attach.  (This is the address
**		    of the actual data, and not the base address of the
**		    buffer that will be attached.)
**		    Passed by value.
**
**  IMPLICIT INPUTS:
**
**	IDU aggregate handle.	This value is stored in the FCT.
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
#if defined(__VMS) || defined(VMS)
long IMG$STORE_DATA_PLANE  ( fid, image_data )
long	 fid;
char	*image_data;
{

return (ImgStoreDataPlane ( fid, image_data ));
} /* end of IMG$STORE_DATA_PLANE */
#endif
/*******************************************************************************
** Portable entry point
*******************************************************************************/
struct FCT *ImgStoreDataPlane( fid, image_data )
struct FCT *fid;
char	*image_data;
{
long	idu_idx	= 0;
/*
** This function is now just a more limitted form of ImgAttachDataPlane.
** It always attaches the data plane to the first IDU in the given ICE.
*/
ImgAttachDataPlane( fid, image_data, idu_idx );

return fid;
} /* end of ImgStoreDataPlane */
