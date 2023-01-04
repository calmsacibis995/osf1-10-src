
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
**  IMG__ROI
**
**  FACILITY:
**
**      Image Services Library
**
**  ABSTRACT:
**
**      This module contains routines for Region of Interest (ROI) support
**
**	Image presentation services, such as rotate and scale, and export 
**	routines need the ability to select a portion of the image as the 
**	subject of an operation.  Thus, there is a need to develop a general 
**	purpose region of interest (ROI) interface.  This interface will 
**	allow the application to operate on any part of the image bounded 
**	by a closed polygon.
**
**	In ISL, a ROI is not an image attribute and does not claim to have 
**	any association with any particular frame.  It is a structure which 
**	describes a particular part of an image.  ROI's are passed by 
**	identifiers so they may be applied to several different frames or 
**	images.
**
**	In the current version of ISL, ROI's only support rectangles that 
**	are parallel to the frame sides and irregular regions constructed
**	by chain codes.  In future versions of ISL, ROI support will include 
**	all forms of closed polygons.
**	
**  ENVIRONMENT:
**
**      VAX/VMS, VAX/ULTRIX , RISC/ULTRIX
**
**  AUTHOR(S):
**
**	Michael D. O'Connor
**
**  CREATION DATE:
**
**	June 4, 1987
**
*****************************************************************************/

/*
**  Table of contents
**
**  VMS Entry Points 
*/
#if defined(__VMS) || defined(VMS)
long	 IMG$CREATE_ROI();	/* Create an ROI		      */
void	 IMG$DELETE_ROI();	/* Delete an ROI		      */
#endif

#ifdef NODAS_PROTO
long	 ImgCreateRoi();	/* DECWindow style entry point	      */
long	 _ImgExportRoi();	/* export bitmap from udp	      */
long	 _ImgValidateRoi();	/* ROI validation for version 3	      */
void	 ImgDeleteRoi();	/* DECWindow style entry point        */

struct UDP	*_ImgSetRoi();	/* redefine UDP image data bounds     */
struct ROI	*_ImgVerifyRoi();/* Verify that an ROI's bounds are OK */
struct ROI	*roi_create_ccode_brect();/* Create ChainCode bound rect*/

void		 roi_verify_ccode_content();/* Verify chain code contents */
void		 roi_verify_rect_content();    /* Verify rectangle contents  */
void		 roi_verify_struct_content();  /* Verify stucture of ROI     */
#endif


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
**  Literals
*/

/*
**  External References from ISL                 <- from module ->
*/
#ifdef NODAS_PROTO
struct BHD	*_ImgBlkAlloc();
char		*_ImgCalloc();			/* Img__MEMORY_MGT	*/
void		 _ImgCfree();			/* Img__MEMORY_MGT	*/
void		 _ImgFree();			/* Img__MEMORY_MGT	*/
char		*_ImgMalloc();			/* Img__MEMORY_MGT	*/

void		 _IpsMovv5();			/* IPS__EXTEND_INSTRUCT	*/

/*
**  External References from CHF                   <- usage ->
*/
void		 ChfSignal();			/* signal exception	*/
void             ChfStop();
#endif

/*
**  Local Storage
*/
#if defined(__VAXC) || defined(VAXC)
static readonly char change_x[8] = {1,1,0,-1,-1,-1,0,1};
static readonly char change_y[8] = {0,-1,-1,-1,0,1,1,1};
#else
char change_x[8] = {1,1,0,-1,-1,-1,0,1};
char change_y[8] = {0,-1,-1,-1,0,1,1,1};
#endif
/*
** no longer use descrip.h - this struct is a substitute and is used locally
*/

struct shape
    {
    long length;
    long dtype;
    long class;
    char *pointer;
    };

/*
** External references
**
**	Signal codes
*/
#include <img/ImgStatusCodes.h>

/*
**  Local
*/

#ifdef NODAS_PROTO
struct ROI	*roi_create_rect();/* Create a ROI of type rectangle     */
struct ROI	*roi_create_ccode();/* Create a ROI of type chain code    */
#else
struct ROI *roi_create_rect(struct shape */*shape_desc*/);
struct ROI *roi_create_ccode(struct shape */*shape_desc*/);
#endif


/****************************************************************************
**               BEGINNING OF VMS ENTRY POINTS                             **
****************************************************************************/
#if defined(__VMS) || defined(VMS)
long IMG$CREATE_ROI(type,bufptr,buflen)
long type;		     /* ROI type - RECTANGLE, CHAIN_CODE, etc.*/
char *bufptr;		     /* pointer to ROI descriptive data	      */
long buflen;		     /* number of bytes in buffer	      */
{

return (ImgCreateRoi(type,bufptr,buflen));
}
#endif

#if defined(__VMS) || defined(VMS)
void IMG$DELETE_ROI(roi_id)
struct ROI *roi_id;	    /* ROI identifier - pointer to ROI structure     */
{
    ImgDeleteRoi(roi_id);
    return;
}
#endif

/****************************************************************************
**                        END OF VMS ENTRY POINTS                          **
****************************************************************************/


/***************************************************************************
**
**  ImgCreateRoi
**
**  FUNCTIONAL DESCRIPTION:
**
**	Creates an ROI.
**
**  FORMAL PARAMETERS:
**
**	type	    - roi type, passed by value
**	bufptr	    - pointer to buffer containing the ROI descriptive data
**		      Buffer passed by reference.
**	buflen	    - length of buffer pointed to by buflen, passed by value
**
**  IMPLICIT INPUTS: none
**
**  IMPLICIT OUTPUTS: none
**
**  FUNCTION VALUE:
**      retroi - pointer to ROI
**
**  SIGNAL CODES:
**	ImgX_BUFWNOLEN	- buffer address passed with no length specified
**	ImgX_INVARGCNT	- invalid number of arguments
**	ImgX_INVARGCON	- Invalid argument contents
**	ImgX_INVROI	- region of interest falls outside current image bounds.
**
**  SIDE EFFECTS:
**	none
**
*****************************************************************************/
long ImgCreateRoi(type,bufptr,buflen)
long type;			     /* ROI type - RECTANGLE, CHAIN_CODE, etc.*/
char *bufptr;			     /* pointer to ROI descriptive data	      */
long buflen;			     /* number of bytes in buffer	      */
{
    struct ROI *roi_id = NULL;		     /* ROI id				      */
    struct shape *shape_desc = (struct shape *)
	    _ImgCalloc(sizeof(struct shape),1);
				     /* shape descriptor - points to ROI data */

    /*
    **	All arguments must be positive..
    */
    if ((type == 0) || (bufptr == 0) || (buflen == 0))
	ChfSignal( 1,  ImgX_INVARGCON);

    shape_desc->length  = buflen;
    shape_desc->dtype   = type;
    shape_desc->class   = 0;
    shape_desc->pointer = bufptr;    

    switch(type)
	{
	case ImgK_RoitypeRect:
	    roi_id = roi_create_rect(shape_desc);
	    break;
	case ImgK_RoitypeCCode:
	    roi_id = roi_create_ccode(shape_desc);
	    break;
	default:
	    ChfSignal( 1,  ImgX_INVARGCON);
	    break;
	} /* end switch */

    _ImgVerifyRoi(roi_id,IMG_K_VERIFY_CONTENT);

    /*
    ** Delete the locally allocate shape descriptor, which
    ** hasn't been saved.
    */
    _ImgFree( shape_desc );

    return( (long)roi_id );

}/* end ImgCreateRoi */

/************************************************************************
**  IMG$DELETE_ROI
**
**  FUNCTIONAL DESCRIPTION:
**
**	Deletes an roi.
**
**  FORMAL PARAMETERS:
**
**	roi     - Pointer to ROI structure
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
**      none
**
**  SIGNAL CODES:
**
**	ImgX_INVARGCNT	- Invalid argument count.
**
**  SIDE EFFECTS:
**
**	none
**
*****************************************************************************/
void ImgDeleteRoi(roi_id)
struct ROI *roi_id;         /* ROI identifier - pointer to ROI structure     */
{

    _ImgVerifyRoi(roi_id,IMG_K_VERIFY_CONTENT);

    /*
    **	Check to see if the shape buffer was internally allocated.
    **	If the shape buffer pointer is non-zero, free the memory.
    */
    if ((roi_id->RoiL_Flags.RoiV_ShapeBufIntAlloc) && 
	(roi_id->RoiA_Shape != 0)) 
	_ImgCfree( roi_id->RoiA_Shape );

    /*
    **	Check to see if the ROI structure was internally allocated.
    **  If so, free the memory.
    */
    if (roi_id->RoiR_Blkhd.BhdL_Flags.BhdV_InternalAlloc)
	_ImgCfree( roi_id );

} /* end ImgDeleteRoi */

/***********************************************************************
**  _ImgVerifyRoi
**
**  FUNCTIONAL DESCRIPTION:
**
**	Verify that an ROI is valid.  Bounds, etc.
**
**  FORMAL PARAMETERS:
**
**	roi_id	    - Pointer to ROI structure
**	level       - verify on CONTENT level or STRUCTURE level
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
**	Returns the roi identifier passed in.
**
**  SIGNAL CODES:
**
**	ImgX_INVROI	- Invalid ROI.
**
**  SIDE EFFECTS:
**
**      none
**
*****************************************************************************/
struct ROI *_ImgVerifyRoi(roi_id,level)
struct ROI *roi_id;	    /* ROI identifier - pointer to ROI structure     */
long level;		    /* level of checking - CONTENT or STRUCTURE_ONLY */
{
    long invroi = FALSE;	    /* flag, set true if ROI is invalid	*/

    roi_verify_struct_content(roi_id);

    if (level == IMG_K_VERIFY_CONTENT)
	switch (roi_id->RoiB_Type)
	    {
	    case ImgK_RoitypeRect: 
		roi_verify_rect_content(roi_id);
		break;
	    case ImgK_RoitypeCCode: 
		roi_verify_ccode_content(roi_id);
		break;
	    default:
		ChfSignal( 1,  ImgX_INVROI);
	    } /* end switch */

    return(roi_id);

} /* end _ImgVerifyRoi */

/*****************************************************************************
**  _ImgValidateRoi
**
**  FUNCTIONAL DESCRIPTION: Determine if ROI is valid relative to UDP
**  FORMAL PARAMETERS:	    roi	    - roi id
**			    udp	    - udp pointer
**  IMPLICIT INPUTS:	    none
**  IMPLICIT OUTPUTS:	    none
**  FUNCTION VALUE:	    TRUE  --> valid   roi
**			    FALSE --> invalid roi
**  SIGNAL CODES:	    none
**  SIDE EFFECTS:	    none
**
*****************************************************************************/
long _ImgValidateRoi(roi,udp)
struct ROI  *roi;   /* ROI block pointer	*/
struct UDP  *udp;   /* udp structure pointer	*/
{
long invalid;
/*
**  Test requested region of interest to make 
**  sure it lies within the image boundaries.
*/
long r_lft,r_rgt,r_top,r_bot;
long u_lft,u_rgt,u_top,u_bot;

r_lft = roi->RoiR_BoundingRect.BrectL_Ulx;
r_rgt = r_lft +	roi->RoiR_BoundingRect.BrectL_XPixels - 1;
r_top = roi->RoiR_BoundingRect.BrectL_Uly;
r_bot = r_top + roi->RoiR_BoundingRect.BrectL_YPixels - 1;

u_lft = udp->UdpL_X1;
u_rgt = udp->UdpL_X2;
u_top = udp->UdpL_Y1;
u_bot = udp->UdpL_Y2;

invalid = (
	    (r_lft > r_rgt) || (r_top > r_bot) ||
	    (r_lft < u_lft) || (r_lft > u_rgt) ||
	    (r_rgt < u_lft) || (r_rgt > u_rgt) ||
	    (r_top < u_top) || (r_top > u_bot) ||
	    (r_bot < u_top) || (r_bot > u_bot)
	  );

return(! invalid);
}

/***********************************************************************
**  _ImgExportRoi
**
**  FUNCTIONAL DESCRIPTION:
**
**	Extract (or export) the portion of a bitmap described by a 
**	Region of Interest into a destination buffer.
**
**      Copy from source image array to destination image array.  The 
**	area copied is defined by the lower and upper bounds in the 
**	supplied source UDP descriptor.  The supplied destination UDP 
**	descriptor describes the output desired.
**
**  FORMAL PARAMETERS:
**
**	udp	    - a pointer to a UDP
**	bufptr	    - pointer to a buffer
**	buflen	    - length of the buffer
**	action	    - action routine to be called if specified
**	userparam   - integer value used by action routine.
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
**      bytcnt - returns number of bytes written to ourput buffer
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
*****************************************************************************/ 
long _ImgExportRoi(udp,bufptr,buflen,action,userparam)
struct UDP  *udp;
char *bufptr;
long   buflen;
long  (*action)();
long   userparam;
{
long    bufpos=0;	/* offset from start of buf for dst img	*/
long	savpos=0;	/* save the current position - for ROIs */
long	curpxl=0;	/* current pixel in src buf. - for ROIs */
long	bytes_moved=0;	/* Num bytes transfered from frame	*/
long	srcpos=0;	/* offset from beg. of buf for cur img	*/
long	srclen=0;	/* length of source buffer		*/
char	*srcptr=0;	/* pointer to destination buffer	*/
long	movlen=0;	/* num bytes to be moved to work buffer	*/
long	movlen_bits=0;	/* num bits to be moved to work buffer	*/
long	movlen_bytes=0; /* num bytes to be moved to work buffer	*/
long	scanline_count=0; /* number of scanlines of interest	*/
long	scanline_stride=0;/* number of pixels from line to line */
long	pixels_per_line=0;/* number of pits in line of interest */
long	i=0;		  /* index for for loop			*/
long	internal_buf = FALSE;/*flag for whether buffer is internal */

/*
** If there is no action routine and no destination buffer pointer then
** signal an error.
*/
if ((action == 0) && (bufptr == 0))
    ChfSignal( 1, ImgX_INVARGCON);

/* 
** The udp field is a pointer to either an uncompressed UDP data plane
** or a compressed udp.
** In the class field of the UDP is DSC$K_CLASS_UBA (value is 34).
** In the class field of the UDP is DSC$K_CLASS_Z (value is 0).
** This is how it is determined which has been passed.  It is necessary 
** to find out which has been passed because ROIs are not allowed
** with compressed images.  If the udp is a UDP_DESC then define find
** scanline count (absolute), pixels per line, and scanline stride (pixels).
*/
 
if (udp->UdpB_Class == UdpK_ClassUBA)
    {
    scanline_count  = udp->UdpL_Y2 - udp->UdpL_Y1 + 1;
    pixels_per_line = udp->UdpL_X2 - udp->UdpL_X1 + 1;
    scanline_stride = udp->UdpL_ScnStride;
    srcpos = udp->UdpL_Pos;
    }
else 
    srcpos = 0;
/*
** Define the source image bit offset, source length, and where the source
** buffer is.  
*/

  
if (udp->UdpB_Class != UdpK_ClassUBA)
    srclen = (udp->UdpL_ArSize + 7) / 8;
else
    srclen = ((udp->UdpL_ScnStride * (udp->UdpL_Y2 -
                         udp->UdpL_Y1 + 1)) + 7) / 8;

srcptr = (char *)udp->UdpA_Base;

/*
**  If the output buffer pointer is zero then one must be allocated for it.
**  If the output buffer length is zero then I take my best guess.
**  In this case, I assume that the DP descriptor is good and does
**  contain the size of it's image, and use it to allocate a new buffer.  
**  If the output buffer pointer is non-zero but the output buffer length 
**  is zero, that's an error.  
**  If the output buffer length is non-zero and the output buffer pointer 
**  is zero, then a buffer is allocated with size of output buffer length.
*/
bufpos = 0;
if (bufptr == 0)
    {
    /* 
    ** If no buffer pointer was passed in, then check to see if a length
    ** was passed.  If a length exists, create a buffer equal to that 
    ** size, otherwise create a buffer the size of the source buffer.
    */
    if (buflen == 0)
        buflen =  srclen;
    else 
        buflen = buflen;
    bufptr  = _ImgMalloc( buflen );	/* NOTE: new mem mgt	*/
    internal_buf = TRUE;			/* signals all errors	*/
    }
else
    /* 
    ** If a buffer pointer was passed but the buffer length is 0, then
    ** signal an error.
    */
    if (buflen == 0)
        ChfStop( 1, ImgX_BUFWNOLEN);

/*
**	There are two classes of bitmaps that can be exported, compressed and
**  uncompressed.  In the uncompressed images, the image can exported using
**  ROIs or not.  In the exportation of these images, the user has the 
**  choice of using action routines or not.  Thus, there are four types
**  of exports that are supported :
**  whole bitmaps (compressed or uncompressed) without action routines
**  whole bitmaps (compressed or uncompressed) with action routines
**  partial bitmaps (uncompressed only) using ROIs without action routines
**  partial bitmaps (uncompressed only) using ROIs with action routines
**
**  To move partial bitmaps, the extended instruction _IpsMovv5 which 
**  moves data on a bit basis.  When the scanlines are stored contiguously,
**  as in the case of whole bitmaps, they are moved using memcpy().
**
**  If the pixels per line is not the same as the scanline stride then 
**  we are using ROIs.
*/
if (pixels_per_line != scanline_stride)
    if (action != 0)
    /*
    **	At this point we know that an uncompressed image is being 
    **  exported WITH action routines and WITH ROIs.  
    **	Bitmaps called with ROIs are exported ONE SCANLINE AT A TIME.
    **  If the destination buffer is smaller than a scanline, then
    **  the buffer will be filled and the action routine called
    **	repeatedly until the whole scanline is processed.  And then
    **	the process is repeated for all scanlines.
    **	
    **	Note: the interface to action routines only allows the 
    **	buffer length field to be specified in BYTES.  If the ROI
    **	defines a scanline that is not on a byte boundary, then the 
    **	top-level routine must tell the action routine that there
    **	will be useless bits at the end of the buffer.  When the 
    **	routine copies an image with a ROI and NO action routine
    **	the scanlines DO NOT necessarily land on byte boundaries.
    */
    while( scanline_count-- > 0)
	{
	curpxl = 0;
	savpos = srcpos;
	for (i = (pixels_per_line - 1) / (buflen * 8); i >= 0; i--)
	    {
	    movlen = ((buflen * 8) < (pixels_per_line - curpxl)) ?
		(buflen * 8) : (pixels_per_line - curpxl);
	    _IpsMovv5(	movlen,		/* num bits to move */
				srcpos,		/* source offset    */
	       (unsigned char *)srcptr,		/* source base	    */
				0,		/* dest offset	    */
	       (unsigned char *)bufptr);	/* dest base	    */
	    curpxl += movlen;
	    srcpos += movlen;
	    movlen = ((movlen + 7) / 8);
	    (*action)(bufptr,movlen,userparam);
	    bytes_moved += movlen;
	    } /* end for */
	srcpos = savpos + scanline_stride;
	}/* end while */
    else /* action == 0 */
	/* 
	**	In this case, there are ROIs but no action routine.  This 
	**	simply moves bits into the destination buffer a scanline
	**	at a time.
	*/
	{
	movlen_bits = (pixels_per_line * udp->UdpL_PxlStride);
	movlen_bytes = (movlen_bits + 7) / 8;
	while( scanline_count-- > 0)
	    {
	    _IpsMovv5(movlen_bits,			/* number of bits     */
			   srcpos, (unsigned char *)srcptr,		/* src offset/base    */
			   bufpos, (unsigned char *)bufptr);		/* dst offset/base    */
	    srcpos += scanline_stride;		/* skip to next line  */
	    bufpos += movlen_bits;
	    bytes_moved +=  movlen_bytes;
	    }/* end while */
	}
    else /* pixels_per_line == scanline_stride */
	 /*
	 **  At this point we know that we are dealing moving contigous memory.
	 */
	if (action != 0)
	    /*
	    **	If an action routine is specified, then fill the destination
	    **  buffer and call the action routine.
	    */
	    while (bytes_moved < srclen)
		{
		movlen = ((srclen - bytes_moved) < buflen) ?	
			    (srclen - bytes_moved) : buflen;
		memcpy( bufptr, srcptr, movlen );
		bytes_moved += movlen;
		srcptr += movlen;
		(*action)(bufptr,movlen,userparam);
		} /* end while */
        else
	    /*
	    **  Since there is no action routine, just copy the whole thing
	    */
	    {
	    bytes_moved = srclen;
	    memcpy( bufptr, srcptr, srclen );
	    } /* end else (action) */

/*
**	If the buffer was allocated for strictly internal use. 
**	(action routines used and no buffer pointer passed)
*/
if (internal_buf)
    _ImgFree( bufptr );

/*
**	Return the number of bytes moved.
*/
return(bytes_moved);

}/* end ImgExportRoi */

/************************************************************************
**  _ImgSetRoi
**
**  FUNCTIONAL DESCRIPTION:
**
**	Redefine the image data boundaries in the supplied UDP.
**
**  FORMAL PARAMETERS:
**
**	udp	- pointer to UDP to modify
**	ROI     - OPTIONAL - pointer to ROI structure
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
**      retudp - pointer to UDP passed in as parameter: udp
**
**  SIGNAL CODES:
**
**	ImgX_INVROI - region of interest falls outside current image bounds.
**
**  SIDE EFFECTS:
**
**	none
**
*****************************************************************************/
struct UDP *_ImgSetRoi( udp, roi )   /* set ROI definition in UDP    */
struct UDP  *udp;
struct ROI  *roi;
{

if ( roi != 0 )
    {
    /*
    **  Test requested region of interest to make sure it lies within
    **  the current source image boundaries.
    */
    if(   (udp->UdpL_X1 > roi->RoiR_BoundingRect.BrectL_Ulx)
	   || (udp->UdpL_Y1 > roi->RoiR_BoundingRect.BrectL_Uly)
	   || (udp->UdpL_X2 < roi->RoiR_BoundingRect.BrectL_Ulx + 
		    roi->RoiR_BoundingRect.BrectL_XPixels - 1)
	   || (udp->UdpL_Y2 < roi->RoiR_BoundingRect.BrectL_Uly + 
		    roi->RoiR_BoundingRect.BrectL_YPixels - 1))
	    /* 
	    ** ROI falls outside of current bounds 
	    */
	    ChfSignal( 1,  ImgX_INVROI ); 

    /*
    **  Compute the data offset (POS) to the new start of data
    */
    udp->UdpL_Pos += 
      ((roi->RoiR_BoundingRect.BrectL_Uly - udp->UdpL_Y1) * udp->UdpL_ScnStride) +
      ((roi->RoiR_BoundingRect.BrectL_Ulx - udp->UdpL_X1) * udp->UdpL_PxlStride);

    /*
    **  Define new image data boundries for region of interest.
    */
    udp->UdpL_X1 = 0;
    udp->UdpL_Y1 = 0;
    udp->UdpL_X2 = roi->RoiR_BoundingRect.BrectL_XPixels - 1;
    udp->UdpL_Y2 = roi->RoiR_BoundingRect.BrectL_YPixels - 1;
    udp->UdpL_PxlPerScn = roi->RoiR_BoundingRect.BrectL_XPixels;
    udp->UdpL_ScnCnt = roi->RoiR_BoundingRect.BrectL_YPixels;
    } /* end if */

/*
**  Return the supplied UDP address.
*/
return( udp );
}/* end _ImgSetRoi */


/************************************************************************
** Create a ROI of type RECTANGLE.
************************************************************************/
struct ROI *roi_create_rect(shape_desc)
struct shape *shape_desc; /* shape descriptor - points to rect data*/
{
    long *rect_fields = (long *) shape_desc->pointer;
    struct ROI *new_roi = (struct ROI *) 
                          _ImgBlkAlloc(sizeof(struct ROI) ,ImgK_BlktypRoi);

    /*
    **	The data pointed to by the shape descriptor is a buffer with 
    **	four longwords.  They correspond to the x-coordinate of the
    **	upper-left corner, the y-coordinate of the upper-left corner,
    **	the number of pixels from the upper-left to the upper-right, and
    **	the number of pixels from the upper-left to the lower-left.
    **	These values are put in the appropriate ROI fields.
    **
    **  There is no data in the shape descriptor buffer of a ROI 
    **	that is of type RECTANGLE.  All the information for this 
    **	kind of ROI can be kept in the BOUNDING_RECT structure.
    */
    new_roi->RoiR_BoundingRect.BrectL_Ulx = *(rect_fields++);
    new_roi->RoiR_BoundingRect.BrectL_Uly = *(rect_fields++);
    new_roi->RoiR_BoundingRect.BrectL_XPixels = *(rect_fields++);
    new_roi->RoiR_BoundingRect.BrectL_YPixels = *(rect_fields++);
    new_roi->RoiL_StartX = new_roi->RoiR_BoundingRect.BrectL_Ulx;
    new_roi->RoiL_StartY = new_roi->RoiR_BoundingRect.BrectL_Uly;
    new_roi->RoiB_Type   = ImgK_RoitypeRect;

    return(new_roi);

}/* end roi_create_rect */

/*********************************************************************
**  Create a ROI of type CHAIN_CODE.
*********************************************************************/
struct ROI *roi_create_ccode(shape_desc)
struct shape *shape_desc;/*shape descriptor,point to chaincode data*/
{
    struct ROI *new_roi = (struct ROI *)
                          _ImgBlkAlloc(sizeof(struct ROI) ,ImgK_BlktypRoi);
    long *shape_ptr = (long *) shape_desc->pointer;

    /*
    **	The data pointed to by the shape descriptor is a buffer with 
    **	two longwords at the beginning denoting the x and y-coordinate
    **	of the start of the chain code.  Following that is a contiguous
    **	list of bytes.  Each byte is a link in the chain.  Each link
    **	is a direction to the next point in the boundary.  The direction
    **	table is as follows:
    **				3 2 1
    **				 \|/
    **				4-*-0
    **				 /|\
    **				5 6 7
    ** The chain information is copied into an internal buffer and then 
    ** a procedure is called to calculate the bounding rectangle about the 
    ** chain code boundary.
    */
    new_roi->RoiB_Type   = ImgK_RoitypeCCode;
    new_roi->RoiW_Length = shape_desc->length - (2 * sizeof(long));
    new_roi->RoiL_StartX = *(shape_ptr++);
    new_roi->RoiL_StartY = *(shape_ptr++);
    new_roi->RoiA_Shape  = _ImgCalloc(new_roi->RoiW_Length,1);
    memcpy( new_roi->RoiA_Shape, shape_ptr, (long) new_roi->RoiW_Length );
    new_roi->RoiL_Flags.RoiV_ShapeBufIntAlloc = 1;

    roi_create_ccode_brect(new_roi);

    return(new_roi);

} /* end roi_create_ccode */

/*********************************************************************
**  Create the bounding rectangle about a ROI of type chain code.
**********************************************************************/
struct ROI *roi_create_ccode_brect(roi_id)
struct ROI *roi_id;
{
    char *chain_code = roi_id->RoiA_Shape;
    long  new_x  = roi_id->RoiL_StartX;	    /* current x position on boundary */
    long  new_y  = roi_id->RoiL_StartY;	    /* current y position on boundary */
    long  max_x  = roi_id->RoiL_StartX;	    /* maximum value in x direction   */
    long  max_y  = roi_id->RoiL_StartY;	    /* maximum value in y direction   */
    long  min_x  = roi_id->RoiL_StartX;	    /* minimum value in x direction   */
    long  min_y  = roi_id->RoiL_StartY;	    /* minimum value in y direction   */
    /* 
    ** change_x and change_y are tables that correspond to the change in 
    ** x and y coordinate values given a direction code from the chain.
    ** They are defined globally in the Local Storage section of this module.
    */
    long i = 0; /* index */

    /*
    **	Step through the whole chain finding the maximum and minimum 
    **	x and y boundaries.  The user must specify the length of the 
    **  shape buffer.  The termination condition is when the routine
    **  has gone through the whole buffer.
    */
    do
	{
	new_x = new_x + change_x[chain_code[i]];	
	new_y = new_y + change_y[chain_code[i]];
	i++;
	max_x = (new_x > max_x) ? new_x : max_x;
	max_y = (new_y > max_y) ? new_y : max_y;
	min_x = (new_x < min_x) ? new_x : min_x;
	min_y = (new_y < min_y) ? new_y : min_y;
	} while ((roi_id->RoiW_Length - i) > 0);

    /*
    **  Now that you've gone throught the whole buffer, make sure
    **  that you're ending x,y pair is the same as your starting pair.
    **  If not, signal INValid ROI.
    */
    if ((new_x != roi_id->RoiL_StartX) || (new_y != roi_id->RoiL_StartY))
	ChfSignal( 1, ImgX_INVROI);

    /*
    **  At this point, the chain has been verified and we can set the
    ** bounding rectangle limits.
    */
    roi_id->RoiR_BoundingRect.BrectL_Ulx = min_x;
    roi_id->RoiR_BoundingRect.BrectL_Uly = min_y;
    roi_id->RoiR_BoundingRect.BrectL_XPixels = max_x - min_x;
    roi_id->RoiR_BoundingRect.BrectL_YPixels = max_y - min_y;

    return(roi_id);

}/* end roi_create_ccode_brect */

/*********************************************************************
**  Verify the content of a chain code ROI.
**********************************************************************/
void roi_verify_ccode_content(roi_id)
struct ROI *roi_id;
{
    long invroi = FALSE;

    /*
    **  There is no verification of the content of a chain code ROI at this time
    */
    if (invroi)
	ChfSignal( 1, ImgX_INVROI);

}/* end roi_verify_ccode_content */

/*******************************************************************
**  Verify the content of a rectangular ROI.
********************************************************************/
void roi_verify_rect_content(roi_id)
struct ROI *roi_id;
{
    long invroi = FALSE;

    /*
    **	The starting x coordinate of the ROI and the upper-left x coordinate
    **	of the bounding rectangle must be equal.
    */
    if (roi_id->RoiR_BoundingRect.BrectL_Ulx != roi_id->RoiL_StartX)
	invroi = TRUE;

    /*
    **	The starting y coordinate of the ROI and the upper-left y coordinate
    **	of the bounding rectangle must be equal.
    */
    if (roi_id->RoiR_BoundingRect.BrectL_Uly != roi_id->RoiL_StartY)
	invroi = TRUE;

    /*
    **  The type of a rectangular ROI must be IMG$L_ROITYPE_RECT
    */
    if (roi_id->RoiB_Type   != ImgK_RoitypeRect)
	invroi = TRUE;

    if (invroi)
	ChfSignal( 1, ImgX_INVROI);
}/* end roi_verify_rect */

/****************************************************************
**  Verify the structure_content of a ROI
****************************************************************/
void roi_verify_struct_content(roi_id)
struct ROI *roi_id;
{
    long invroi = FALSE;

    /*
    **	Blockhead type of ROI and it's length must be set accordingly.
    */
    if ((roi_id->RoiR_Blkhd.BhdB_Type != ImgK_BlktypRoi) ||
	(roi_id->RoiR_Blkhd.BhdW_Length != sizeof(struct ROI)))
	ChfSignal( 3,  ImgX_INVBLKTYP, 1,ImgK_BlktypRoi);

    /*
    **	Stacks of ROI's are not supported.
    */
    if (roi_id->RoiR_Blkhd.BhdA_Flink != roi_id->RoiR_Blkhd.BhdA_Blink)
	invroi = TRUE;

    if (invroi)
	ChfSignal( 1, ImgX_INVROI);
}/* end roi_verify_structure_content */
