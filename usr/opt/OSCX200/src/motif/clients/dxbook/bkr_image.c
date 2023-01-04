/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_IMAGE.C*/
/* *7    26-MAR-1993 15:40:28 BALLENGER "Fix compilation problems for VAX ULTRIX."*/
/* *6     1-FEB-1993 10:01:50 RAMSHAW "Int QAR #10 - Graphics scroll"*/
/* *5    19-JUN-1992 20:19:33 BALLENGER "Cleanup for Alpha/OSF port."*/
/* *4     9-JUN-1992 09:56:49 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *3     3-MAR-1992 17:00:00 KARDON "UCXed"*/
/* *2    17-SEP-1991 20:15:26 BALLENGER "include function prototype headers"*/
/* *1    16-SEP-1991 12:39:36 PARMENTER "Image scaling for interface with ISL"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_IMAGE.C*/
#ifndef VMS
 /*
#else
#module BKR_IMAGE "V03-0005"
#endif
#ifndef VMS
  */
#endif
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/dxbook/bkr_image.c,v 1.1.4.2 1993/08/24 16:04:40 Ellen_Johansen Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */

/*
***************************************************************
**  Copyright (c) Digital Equipment Corporation, 1988, 1990  **
**  All Rights Reserved.  Unpublished rights reserved	     **
**  under the copyright laws of the United States.  	     **
**  	    	    	    	    	    	    	     **
**  The software contained on this media is proprietary	     **
**  to and embodies the confidential technology of  	     **
**  Digital Equipment Corporation.  Possession, use,	     **
**  duplication or dissemination of the software and	     **
**  media is authorized only pursuant to a valid written     **
**  license from Digital Equipment Corporation.	    	     **
**  	    	    	    	    	    	    	     **
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 	     **
**  disclosure by the U.S. Government is subject to 	     **
**  restrictions as set forth in Subparagraph (c)(1)(ii)     **
**  of DFARS 252.227-7013, or in FAR 52.227-19, as  	     **
**  applicable.	    	    	    	    	    	     **
***************************************************************
*/


/*
**++
**  FACILITY:
**
**      Bookreader User Interface (bkr)
**
**  ABSTRACT:
**
**	Image scaling routines for interfacing with the Image Services
**  	Library (ISL).
**
**  AUTHORS:
**
**      James A. Ferguson
**
**  CREATION DATE:     29-Jan-1990
**
**  MODIFICATION HISTORY:
**
**      V03-0005    DLB0003     David L Ballenger      14-Jun-1991
**                  Conditionalize scaling and use of Image Services routines.
**
**      V03-0004    DLB0002     David L Ballenger      13-May-1991
**                  Finish removing all "IMG$" symbols for portability.
**
**      V03-0003    DLB0001     David L Ballenger       6-Feb-1991
**                  Change defintion of cvt_x_image_to_fid() and
**                  cvt_fid_to_ximage to match forward declarations.
**
**                  Pass correct number of arguments to bkr_error_modal().
**
**	V03-0002    JAF0002	James A. Ferguson   	9-Oct-1990
**  	    	    Modify all routines to use "Img" C bindings 
**  	    	    instead of "img$" VAX bindings and remove #ifdef VMS
**  	    	    so all routines will now compile and run on Ultrix.
**
**	V03-0001    JAF0001	James A. Ferguson   	29-Jan-1990
**  	    	    Extracted V2 routines and created new module.
**
**--
**/


/*
 * INCLUDE FILES
 */

#ifdef BKR_SCALING
# include   	<ImgDef.h>
#endif 


#include	<X11/Xlib.h>
#include    	<X11/Xutil.h>
#include "br_common_defs.h"  /* common BR #defines */
#include "br_meta_data.h"    /* typedefs and #defines for I/O of BR files */
#include "br_typedefs.h"     /* BR high-level typedefs and #defines */
#include "br_globals.h"      /* BR external variables declared here */
#include "br_malloc.h"       /* BKR_MALLOC, etc */
#include "bkr_image.h"       /* function prototypes for .c module */
#include "bkr_error.h"       /* error reporting routines */
#include "bkr_fetch.h"       /* resource fetching routines */


/*
 * FORWARD ROUTINES
 */

#ifdef BKR_SCALING
static void  	    	    cvt_ximage_to_fid();
static void  	    	    cvt_fid_to_ximage();
#endif


/*
 * FORWARD DEFINITIONS
 */

static char	    	    *error_string;


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_image_close
**
** 	Frees any data and destroy any pixmaps associated with an image.
**
**  FORMAL PARAMETERS:
**
**	chunk - pointer to the chunk containing the image to be freed.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
void
bkr_image_close PARAM_NAMES((chunk))
    BMD_CHUNK	*chunk PARAM_END

{
    XImage	    *ximage_ptr;
    BMD_IMAGE_PKT   *image_ptr;

    if ( (chunk->handle == NULL) || (chunk->handle == NODISPLAY))
	return;

    ximage_ptr = (XImage *) chunk->handle;
    switch ( chunk->data_type )
    {
	case BMD_CHUNK_IMAGE75:	    /* NOTE: Version 1.0 books ONLY */
	    /*
	     * Graphic was scaled.  We must free the image data
	     * because we malloc'ed it when we did the scaling.
	     */

	    if ( bkr_monitor_resolution != 75 )
		BKR_FREE( ximage_ptr->data );		/* Free image data */

	    ximage_ptr->data = 0;		/* Zero pointer    */
	    XDestroyImage( ximage_ptr );    	/* Free the XImage */
	    break;

	case BMD_CHUNK_IMAGE:	    /* Bitmap Image -- screen resolution  */
	    image_ptr = (BMD_IMAGE_PKT *) chunk->data_addr;
	    /*
	     * Graphic was scaled.  We must free the image data
	     * because we malloc'ed it when we did the scaling.
	     */

	    if ( ( bkr_monitor_resolution != image_ptr->res_x)	|| 
		 (bkr_monitor_resolution != image_ptr->res_y ) )
    	    	BKR_FREE( ximage_ptr->data );	    /* Free image data */

	    ximage_ptr->data = 0;		/* Zero pointer    */
	    XDestroyImage( ximage_ptr );    	/* Free the XImage */
	    break;

	default:
	    error_string = (char *) bkr_fetch_literal( "UNKNOWN_IMAGE_DATA", ASCIZ );
    	    if ( error_string != NULL )
    	    {
	    	sprintf( errmsg, error_string, chunk->data_type );
	    	bkr_error_modal( errmsg, NULL );
	    	XtFree( error_string );
    	    }
	    break;
    }

};   /* end of bkr_image_close */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_image_display
**
** 	Displays an XImage in a window.
**
**  FORMAL PARAMETERS:
**
**  	window	    -  id of window to display in
**  	data_addr   -  address of data to display
**  	data_len    -  length of data to display
**  	xoff	    -  x offset of origin in window
**  	yoff	    -  y offset of origin in window
**  	xclip	    -  x value of clip rectangle
**  	yclip	    -  y value of clip rectangle
**  	wclip	    -  width of clip rectangle
**  	hclip	    -  height of clip rectangle
**  	im_width    -  width of X image
**  	im_height   -  height of X image
**  	ximage	    - X Image to display
**
**  	NOTE:  clip rectangle is wrt window origin, NOT wrt (xoff, yoff)
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
void
bkr_image_display PARAM_NAMES((window,data_addr,data_len,xoff,yoff,
                               xclip,yclip,wclip,hclip,
                               im_width,im_height,handle))
    Window		window PARAM_SEP
    BR_UINT_8		*data_addr PARAM_SEP
    BR_UINT_32		data_len PARAM_SEP
    int 		xoff PARAM_SEP
    int 		yoff PARAM_SEP
    int 		xclip PARAM_SEP
    int 		yclip PARAM_SEP
    int 		wclip PARAM_SEP
    int 		hclip PARAM_SEP
    int			im_width PARAM_SEP
    int			im_height PARAM_SEP
    BR_HANDLE		handle PARAM_END

{
    XRectangle  rect;

    rect.x = xclip;
    rect.y = yclip;
    rect.width  = wclip;
    rect.height = hclip;

/* Set up clipping rect to limit the amount of drawing to do
 */
    XSetClipRectangles(
	bkr_display,			/* display */
	bkr_text_gc,	    	    	/* GC */
	0, 0,
	&rect, 1, Unsorted);

    XPutImage(
	bkr_display,			/* display */
	window,				/* drawable */
	bkr_text_gc,	    	    	/* GC */
	(XImage *)handle,	        /* image */
	0, 0,				/* src_x, src_y */
	xoff, yoff,			/* dst_x, dst_y */
	im_width, im_height );		/* width, height */

/* Cancel the clipping rect
 */
    XSetClipMask(
	bkr_display,			/* display */
	bkr_text_gc,	    	    	/* GC */
	None);
};   /* end of bkr_image_display */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_image_init
**
** 	Creates an XImage for displaying in a window.
**
**  FORMAL PARAMETERS:
**
**  	data_addr   -  address of data to display
**  	data_len    -  length of data to display
**  	width	    -  image width in pixels
**  	height	    -  image height in pixels
**  	x_res	    -  horizontal resolution created at
**  	y_res	    -  vertical resolution created at
**  	data_type   -  data type identifier
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
BR_HANDLE
bkr_image_init PARAM_NAMES((data_addr,data_len,width,height,
                            x_res,y_res,data_type))
    BR_UINT_8		*data_addr PARAM_SEP
    BR_UINT_32		data_len PARAM_SEP
    short int		width PARAM_SEP
    short int		height PARAM_SEP
    short int		x_res PARAM_SEP
    short int		y_res PARAM_SEP
    BR_UINT_32	        data_type PARAM_END
{
    XImage	*ximage;
    XImage  	*ximage_rtn;
    float	xscale;	    	    	/* x-direction */
    float   	yscale;			/* y-direction */
    int		scale_bitmap = FALSE;

    ximage = (XImage *) XCreateImage(
	bkr_display,
	XDefaultVisual( bkr_display, 
    	    	XDefaultScreen( bkr_display ) ),
	1,					    /* depth 	      */
	XYBitmap,				    /* format 	      */
	0,					    /* offset 	      */
	(char *)data_addr,			    /* image data ptr */
	width,					    /* image width    */
	height,					    /* image height   */
	8,					    /* xpad 	      */
	0 );					    /* bytes_per_line */

    /*
     *  Set the bit and byte order.
     *  NOTE:  We set the bit and byte order because if any machines
     *  bit and byte order are different, Xlib will rearrange the bit 
     *  and/or bytes for us automatically.
     */

    ximage->byte_order = NATIVE_BYTE_ORDER;
    ximage->bitmap_bit_order = NATIVE_BIT_ORDER;

    switch ( data_type )
    {
	case BMD_CHUNK_IMAGE75:	    /* Bitmap Image --  75 dpi resolution */
#ifdef BKR_SCALING
	    if ( bkr_monitor_resolution != 75 )  /* Version 1.0 books ONLY	  */
	    {
		xscale = (float) bkr_monitor_resolution / 75.0;
		yscale = xscale;
		scale_bitmap = TRUE;
	    }
#endif 
	    break;

	case BMD_CHUNK_IMAGE:	    /* Bitmap Image */
#ifdef BKR_SCALING
	    if ( ( bkr_monitor_resolution != x_res )    || 
		 ( bkr_monitor_resolution != y_res ) )
	    {
		xscale = (float) bkr_monitor_resolution / (float) x_res;
		yscale = (float) bkr_monitor_resolution / (float) y_res;
		scale_bitmap = TRUE;
	    }
#endif 
	    break;

	default:
	    error_string = (char *) bkr_fetch_literal( "UNKNOWN_IMAGE_DATA", ASCIZ );
    	    if ( error_string != NULL )
    	    {
	    	sprintf( errmsg, error_string, data_type );
	    	bkr_error_modal( errmsg, NULL );
	    	XtFree( error_string );
    	    }
	    break;
    }
#ifdef BKR_SCALING
    /*
     * If we scale the bitmap we must free the XImage we passed in but
     * not the data.  The data is freed later in a BRI_ routine.
     */

    if ( scale_bitmap )
    {
	ximage_rtn = (XImage *) bkr_image_scale( ximage, &xscale, &yscale );
	ximage->data = 0;		/* NULL the pointer first */
	XDestroyImage( ximage );    	/* Free the XImage 	  */
	return( (BR_HANDLE)ximage_rtn );
    }
#endif 
    return( (BR_HANDLE)ximage );

};   /* end of bkr_image_init */


#ifdef BKR_SCALING
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_image_scale
**
** 	Scales an XImage given the new X and Y resolution.
**
**  FORMAL PARAMETERS:
**
**	ximage - pointer to the XImage to be scaled.
**  	xscale - pointer to the new X resolution.
**  	yscale - pointer to the new Y resolution.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	Returns:    Pointer to the scaled XImage.  
**
**  SIDE EFFECTS:
**
**  	The calling routine is responsible for free the XImage structure 
**  	and the data pointer within the XImage structure.
**
**--
**/
XImage *
bkr_image_scale PARAM_NAMES((ximage,xscale,yscale))
    XImage	*ximage PARAM_SEP
    float	*xscale PARAM_SEP
    float	*yscale PARAM_END

{
    unsigned long   fid;
    unsigned long   scaled_fid;
    XImage	    *ximage_rtn;

    cvt_ximage_to_fid( ximage, &fid );

    scaled_fid = ImgScale(
    	fid,	    	    	    	    	    	/* frame id    	    */
	xscale, 					/* x-direction 	    */
	yscale, 					/* y-direction 	    */
	NULL,	    	    	    	    	    	/* region	    */
	ImgM_SaveVertical | ImgM_SaveHorizontal,	/* save detail flag */
    	0 );	    	    	    	    	    	/* alignment-UNUSED */

    cvt_fid_to_ximage( &ximage_rtn, scaled_fid );

    ImgDeleteFrame( fid );
    ImgDeleteFrame( scaled_fid );

    return( ximage_rtn );

};   /* end of bkr_image_scale */



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	cvt_ximage_to_fid
**
** 	Converts an XImage to an ISL FID (frame id).
**
**  FORMAL PARAMETERS:
**
**	ximage - pointer to the XImage to be converted.
**	fid    - address to return the new FID.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
static void
cvt_ximage_to_fid( ximage, fid )
    XImage	*ximage;
    int		*fid;		/* frame id */
{
    int 	stride;			/* Difference in bit addresses  */
					/*   between the first bit in   */
					/*   adjacent scanlines		*/

    int		itmlst[13],
		new_itmlst[13],
		new_fid,
		dummy,
		size;

    stride     = ximage->bytes_per_line << 3;

    itmlst[0]  = Img_ScanlineStride;
    itmlst[1]  = 4;
    itmlst[2]  = (int) &stride;
    itmlst[3]  = 0;

    itmlst[4]  = Img_NumberOfLines;
    itmlst[5]  = 4;
    itmlst[6]  = (int) &ximage->height;
    itmlst[7]  = 0;

    itmlst[8]  = Img_PixelsPerLine;
    itmlst[9]  = 4;
    itmlst[10] = (int) &ximage->width;
    itmlst[11] = 0;

    itmlst[12] = 0;			/* NULL terminated item list */

    /* Create an empty frame */

    *fid = ImgCreateFrame(
	itmlst, 			/* input item list to use       */
	ImgK_StypeBitonal );		/* empty frame for bional image */

    /* Get the image data from our buffer in memory and put it in a frame */

    size = ximage->bytes_per_line * ximage->height;
    ImgImportBitmap(
	*fid, 			/* frame id			*/
	ximage->data,		/* bitmap data			*/
	size,			/* Length of bitmap in bytes 	*/
	0,			/* # of imported bytes		*/
	0,			/* flags - UNUNSED		*/
	0,			/* action routine		*/
	0 );			/* user parameter 		*/

    /* Byte align the image */

    if ( ( ximage->bytes_per_line << 3 ) != ximage->width )
    {
	ximage->bytes_per_line = (ximage->width + 7) >> 3;
	ximage->width          = ximage->bytes_per_line << 3;
	stride 		       = ximage->bytes_per_line << 3;
	new_itmlst[0] = Img_ScanlineStride;
	new_itmlst[1] = 4;
	new_itmlst[2] = (int) &stride;
	new_itmlst[3] = 0;

	new_itmlst[4] = Img_PixelsPerLine;
	new_itmlst[5] = 4;
	new_itmlst[6] = (int) &ximage->width;
	new_itmlst[7] = 0;

	new_itmlst[8] = 0;	/* NULL terminate item list */

	/* Copy the old frame to a new frame using the specified item list */

	new_fid = ImgCopy(
	    *fid, 		/* frame id */
	    0, 			/* region   */
	    new_itmlst );

	/* Delete the old frame */

	ImgDeleteFrame( *fid );
	*fid        = new_fid;

    }	/* end if byte_align_image */

};   /* end of cvt_ximage_to_fid */



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	cvt_fid_to_ximage
**
** 	Converts an ISL FID (frame id) to an XImage.
**
**  FORMAL PARAMETERS:
**
**	ximage 	    - address to return the XImage.
**	input_fid   - pointer to the FID to be converted to an XImage.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**  	The calling routine is responsible for free the XImage structure 
**  	and the data pointer within the XImage structure.
**
**--
**/
static void
cvt_fid_to_ximage( timage, input_fid )
	XImage       **timage;
	int	     input_fid;
{
    int 	stride,
		itmlst[21],
		new_itmlst[21],
		fid,
		new_fid,
		compress,
		dummy,
		size;
    XImage	*ximage;

    /* Create the XImage we will return */

    ximage = (XImage *) XCreateImage(
	bkr_display,
	XDefaultVisual( bkr_display, XDefaultScreen( bkr_display ) ),
	1,					/* depth  	  */
	XYBitmap,				/* format 	  */
	0,					/* offset 	  */
	0,					/* data addr  	  */
	0,					/* width  	  */
	0,					/* height 	  */
	8,					/* bitmap pad     */
	0 );					/* bytes per line */
    ximage->width = ximage->bytes_per_line << 3;

    /*
     * Set the bit and byte order.
     * NOTE:  We set the bit and byte order because if any machines 
     * bit and byte order are different, Xlib will rearrange the bit 
     * and/or bytes for us automatically.
     */

    ximage->byte_order = NATIVE_BYTE_ORDER;
    ximage->bitmap_bit_order = NATIVE_BIT_ORDER;

    itmlst[0] = Img_ScanlineStride;
    itmlst[1] = 4;
    itmlst[2] = (int) &stride;
    itmlst[3] = (int) &dummy;
    itmlst[4] = 0;

    itmlst[5] = Img_NumberOfLines;
    itmlst[6] = 4;
    itmlst[7] = (int) &ximage->height;
    itmlst[8] = (int) &dummy;
    itmlst[9] = 0;

    itmlst[10] = Img_PixelsPerLine;
    itmlst[11] = 4;
    itmlst[12] = (int) &ximage->width;
    itmlst[13] = (int) &dummy;
    itmlst[14] = 0;

    itmlst[15] = Img_CompressionType;
    itmlst[16] = 4;
    itmlst[17] = (int) &compress;
    itmlst[18] = (int) &dummy;
    itmlst[19] = 0;

    itmlst[20] = 0;

    fid = ImgGetFrameAttributes( input_fid, itmlst );

    /* Decompress the frame if needed */

    if ( compress != ImgK_PcmCompression ) 
    {
	new_fid = ImgDecompress( fid );
	fid = ImgGetFrameAttributes( new_fid, itmlst );
    }

    /* Byte align the image */

    ximage->bytes_per_line = stride >> 3;
    if ( ( ximage->bytes_per_line << 3 ) != ximage->width )
    {
	ximage->bytes_per_line 	= ( ximage->width + 7 ) >> 3;
	ximage->width 	      	= ximage->bytes_per_line << 3;
	stride 		      	= ximage->bytes_per_line << 3;

    	new_itmlst[0]         	= Img_ScanlineStride;
	new_itmlst[1]         	= 4;
	new_itmlst[2]         	= (int) &stride;
	new_itmlst[3]         	= 0;

	new_itmlst[4]         	= Img_PixelsPerLine;
	new_itmlst[5]         	= 4;
	new_itmlst[6]         	= (int) &ximage->width;
	new_itmlst[7]         	= 0;
	new_itmlst[8]         	= 0;

	new_fid               	= ImgCopy( fid, 0, new_itmlst );
	fid                   	= new_fid;
    }
    else
	new_fid        = 0;

    /* Export the frame into our XImage */

    size = ximage->bytes_per_line * ximage->height;
    ximage->data = (char *) BKR_MALLOC( size );
    ImgExportBitmap(
	fid,			/* frame id       */
	0,			/* region         */
	ximage->data,		/* bitmap data    */
	size,			/* size in bytes  */
	0,			/* byte cnt	  */
	0,			/* flags	  */
	0,			/* action routine */
	0 );			/* user parameter */

    *timage = ximage;
    if ( new_fid )
	ImgDeleteFrame( new_fid );

};   /* end of cvt_fid_to_ximage */



#endif 
