
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
**  IMG__DATA_PLANE_UTILS
**
**  FACILITY:
**
**	Image Services Library
**
**  ABSTRACT:
**
**	This module contains a set of routines that are layered on top
**	of memory mgt calls and CDA calls that allocate and store data
**	plane buffers in ISL frames.  The need for special processing of 
**	data plane buffers arises from the need to store large image buffers 
**	that are used by ISL directly into the CDA frame structure, without
**	copying them into the frame structure in the manner that attributes
**	are copied for storage.
**
**	ISL takes advantage of the CDA routine CDA_ATTACH_ITEM_, which 
**	attaches a buffer of user memory directly into the frame.  The
**	quirk is that this buffer must include, as its first longword,
**	the length of the remainder of the buffer that is used to store
**	the image data:
**
**			+---------------+
**			| length	| : base buffer address
**			+---------------+
**			|		| : image data buffer address
**			+---------------+
**			|		|
**			.		.
**			.		.
**			.		.
**			|		|
**			+---------------+
**			|		| : end of image data
**			+---------------+
**
**	These routines manage the length field as well as the image data 
**	portion.
**
**  ENVIRONMENT:
**
**	VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
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
*/
#ifdef NODAS_PROTO
char	*_ImgAllocateDataPlane();
char	*_ImgAllocDataPlane();
long	 _ImgAttachDataPlane();
char	*_ImgDetachDataPlane();
void	 _ImgFreeDataPlane();
char	*_ImgReallocateDataPlane();
void	 _ImgStoreDataPlane();
void	 _ImgVerifyDataPlane();
#endif


/*
**  Include files:
**
**	none
*/
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
#define	LENGTH_FIELD_SIZE   sizeof(long)
#define IMG_K_SCRATCH_SPACE_EXTENDER sizeof(long)

/*
**  External References:
**
**  Most CDA references are in cdaptp.h
*/
#if defined(NEW_CDA_SYMBOLS)
unsigned long	cda_attach_item();
unsigned long	cda_detach_item();
#else
unsigned long	CDA_ATTACH_ITEM_();
unsigned long	CDA_DETACH_ITEM_();
#endif

#ifdef NODAS_PROTO
void	  ImgSetDataPlane();	/* froM IMG_CONTEXT_UTILS	    */

char	*_ImgAlloc();		/* from IMG__MEMORY_MGT		    */
void	 _ImgDealloc();		/* from IMG__MEMORY_MGT		    */
long	 _ImgErase();		/* from IMG__ATTRIBUTE_ACCESS_UTILS */
long	 _ImgFree_VM();		/* from IMG__MEMORY_MGT		    */
long	 _ImgGetVM();		/* from IMG__MEMORY_MGT		    */
long	 _ImgGet();		/* from IMG__ATTRIBUTE_ACCESS_UTILS */
void	 _ImgInitMemoryMgt();	/* from IMG__MEMORY_MGT		    */
char    *_ImgRealloc();		/* from IMG__MEMORY_MGT		    */
#endif



/*
**  Status codes
*/
#include    <img/ImgStatusCodes.h>

/*
**  Local Storage:
**
**	none
*/


/******************************************************************************
**  _ImgAllocateDataPlane
**
**  FUNCTIONAL DESCRIPTION:
**
**	Allocate a buffer to be used an image data plane, that can be
**	directly attached to the CDA frame structures with a call to
**	CDA_ATTACH_ITEM_.  This buffer must have its first longword reserved
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
char *_ImgAllocateDataPlane( size, fill_mask )
int	size;
int	fill_mask;
{
char		*base_bufadr;
char		*image_data;
long		 actual_size;
long		 flags;

flags = ImgM_InitMem;
image_data = _ImgAllocDataPlane( size, flags, fill_mask );

/*
** Return the address of the image-date portion of the buffer.
*/
return image_data;
} /* end of _ImgAllocateDataPlane


/******************************************************************************
**  _ImgAllocDataPlane
**
**  FUNCTIONAL DESCRIPTION:
**
**	Allocate a buffer to be used an image data plane, that can be
**	directly attached to the CDA frame structures with a call to
**	CDA_ATTACH_ITEM_.  This buffer must have its first longword reserved
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
char *_ImgAllocDataPlane( size, flags, fill_mask )
long		size;
long		flags;
int		fill_mask;
{
char		*base_bufadr;
char		*image_data;
long		 actual_size;

/*
** Make sure ISL memory mgt has been initialized
*/
_ImgInitMemoryMgt();

/*
** Add the extra up-front longword length to the requested size, and
** then allocate the memory.
**
** Add the scratch space extender value too, although don't
** count it as part of the stored data plane size.
*/
actual_size = size + LENGTH_FIELD_SIZE + IMG_K_SCRATCH_SPACE_EXTENDER;

base_bufadr = (char *)_ImgAlloc( actual_size, flags, fill_mask );
/* 
** _ImgGetVM( &actual_size, &base_bufadr, 0 );
*/

/*
** Store the requested size in the first longword of the actual buffer
** allocated, and calculate the start address of the image data.
*/
*((int*)base_bufadr) = size;
image_data = base_bufadr + LENGTH_FIELD_SIZE;

/*
** Fill in the memory with the fill mask if it is non-zero
**if ( fill_mask != 0 )
**    memset( image_data, fill_mask, size );
*/

/*
** Return the address of the image-date portion of the buffer.
*/
return image_data;
} /* end of _ImgAllocDataPlane


/******************************************************************************
**  _ImgAttachDataPlane
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
**	data_plane  Address of image data to attach.  (This is the address
**		    of the actual data, and not the base address of the
**		    buffer that will be attached.)
**		    Passed by value.
**
**	index	    Index value of the idu to use.
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
struct	 FCT	*_ImgAttachDataPlane( fid, data_plane, index )
struct	 FCT	*fid;
char		*data_plane;
long		 index;
{
char	*base_bufadr;
char	*original_base_address;
int	 aggregate_item;
long	 status;

_ImgGet( fid, Img_DataPlaneBase, &original_base_address, LONGSIZE, 0, index );
if ( data_plane != original_base_address )
    {
    /*
    ** Erase the old data plane if there is one...
    */
    _ImgErase( fid, Img_PlaneData, index );

    /*
    ** Save the current frame context (idu pointer) and set it
    ** to correspond to the index passed in ...
    */
/*    ImgSaveCtx( fid );
*/
    ImgSetDataPlane( fid, index );

    /*
    ** Get the actual base address of the data buffer, and use this
    ** address to attach the data plane to the (newly set) current
    ** IDU aggregate.
    */
    base_bufadr = data_plane - LENGTH_FIELD_SIZE;
#if defined(NEW_CDA_SYMBOLS)
    aggregate_item = DDIF_IDU_PLANE_DATA;
#else
    aggregate_item = DDIF$_IDU_PLANE_DATA;
#endif
    status = CDA_ATTACH_ITEM_(
	    &(fid->FctL_IduAggr),
	    &aggregate_item,
	    base_bufadr );
    if ( (status&1) != 1 )
	ChfStop( 1,  status );

    /*
    ** Now restore the original frame context and return
    */
/*    ImgRestoreCtx( fid );
*/
    }

return fid;
} /* end of _ImgAttachDataPlane */

/******************************************************************************
**  _ImgDetachDataPlane
**
**  FUNCTIONAL DESCRIPTION:
**
**	This function detaches an image data plane from an existing image
**	frame. Once detached the application may privately process the data.
**
**  FORMAL PARAMETERS:
**
**	fid	    Frame id of frame to attach data plane to.
**		    Passed by value.
**
**	index	    The plane index ranging from 0 to n-1
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
**	address of the data plane
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
char *_ImgDetachDataPlane( fid, index )
struct	 FCT	*fid;
unsigned long	 index;
{
unsigned char	*data_plane_base = 0;
unsigned char	*base_bufadr;
int		aggregate_item;
unsigned long	idu_aggr;
int		status;

/*
** Locate the data plane if there is one and detach it.
*/
_ImgGet(fid,Img_DataPlaneBase,&data_plane_base,sizeof(data_plane_base),0,index);

/*
** Save the current frame context (idu pointer) and set it
** to correspond to the index passed in ...
*/

/*ImgSaveCtx( fid );
*/
ImgSetDataPlane( fid, index );

#if defined(NEW_CDA_SYMBOLS)
aggregate_item = DDIF_IDU_PLANE_DATA;
#else
aggregate_item = DDIF$_IDU_PLANE_DATA;
#endif
status = CDA_DETACH_ITEM_(
	    &(fid->FctL_IduAggr),
	    &aggregate_item );

if ((status & 1) != 1)
    ChfStop(1, status);

/*
** Now restore the original frame context and return
*/
/*ImgRestoreCtx( fid );
*/

return ((char *)data_plane_base);

} /* end of _ImgDetachDataPlane */

/******************************************************************************
**  _ImgFreeDataPlane
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
void _ImgFreeDataPlane( image_data )
char	*image_data;
{
char	*base_bufadr;
long	 actual_size;

/*
** Make sure ISL memory mgt has been initialized
*/
_ImgInitMemoryMgt();

/*
** Backup from the image data to the base address by the size of the
** length field.
*/
base_bufadr = image_data - LENGTH_FIELD_SIZE;
actual_size = (*(long*)base_bufadr) + LENGTH_FIELD_SIZE;

_ImgDealloc( (unsigned char *)base_bufadr );
/*
** _ImgFree_VM( &actual_size, &base_bufadr, 0 );
*/

return;
} /* end of _ImgFreeDataPlane */


/******************************************************************************
**  _ImgReallocateDataPlane
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
char *_ImgReallocateDataPlane( image_data, new_size )
char	*image_data;
int	 new_size;
{
char	*oldbase_bufadr;
char	*newbase_bufadr;
char	*new_image_bufadr;

/*
** Make sure ISL memory mgt has been initialized
*/
_ImgInitMemoryMgt();

/*
** Subtract away the length field size to get the actual base buffer address.
*/
oldbase_bufadr = image_data - LENGTH_FIELD_SIZE;

/*
** Reallocate the buffer, returning the new base buffer address.
*/
newbase_bufadr = _ImgRealloc( oldbase_bufadr, new_size + LENGTH_FIELD_SIZE );

/*
** Store the new size in the length field longword at the beginning of
** the data buffer.
*/
*((int*)newbase_bufadr) = new_size;

/*
** Return the address of the start of the image data.
*/
new_image_bufadr = newbase_bufadr + LENGTH_FIELD_SIZE;

return new_image_bufadr;
} /* end of _ImgReallocateDataPlane */


/******************************************************************************
**  _ImgStoreDataPlane
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
void _ImgStoreDataPlane( fid, image_data )
struct	 FCT	*fid;
char		*image_data;
{
long	idu_index   = 0;

/*
** This function is obsolete, but just in case someone calls it,
** perform this function as a call to _ImgAttachDataPlane.
*/
_ImgAttachDataPlane( fid, image_data, idu_index );

return;
} /* end of _ImgStoreDataPlane */


/******************************************************************************
**  _ImgVerifyDataPlane()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Verify that the address passed in is actually the address of a
**	data plane buffer allocated by _ImgAllocateDataPlane.
**
**  FORMAL PARAMETERS:
**
**	image_data	Base address of the image data portion of the
**			buffer.  This address is the buffer address known
**			to the user.  Passed by reference.
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
void _ImgVerifyDataPlane( fid, data_plane, index )
struct FCT *fid;
char	*data_plane;
long	 index;
{
long	    all_but_one_line_stride;
long	    arsize;
long	    compression_type;
long	    data_plane_size;
long	    last_line_stride;

struct UDP  udp;

_ImgGet( fid, Img_CompressionType, &compression_type, sizeof(long), 0, index );
_ImgGet( fid, Img_Udp, &udp, sizeof(struct UDP), 0, index );

data_plane_size = *(((long*)data_plane)-1);
all_but_one_line_stride = udp.UdpL_ScnStride * (udp.UdpL_ScnCnt - 1);
last_line_stride = udp.UdpL_PxlStride * udp.UdpL_PxlPerScn;

arsize = (( all_but_one_line_stride + last_line_stride + udp.UdpL_Pos) + 7)/8;

if ( compression_type == ImgK_PcmCompression )
    {
    /*
    ** For uncompressed data, the data plane MUST be at least as
    ** large as the array size (in bytes).
    */
    if ( data_plane_size < arsize )
	ChfSignal( 5, ImgX_INVDPSIZE, 3, data_plane_size, arsize, index );
    }
else
    /*
    ** For compressed data of any sort, the data plane MUST
    ** be less than the calculated size of the data if it were
    ** uncompressed.
    */
    if ( data_plane_size >= arsize )
	ChfSignal( 5, ImgX_INVDPSIZE, 3, data_plane_size, arsize, index );

return;
} /* end of _ImgVerifyDataPlane */
