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
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_RAGS.C*/
/* *10   19-JUN-1992 20:19:42 BALLENGER "Cleanup for Alpha/OSF port."*/
/* *9     9-JUN-1992 09:58:01 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *8    14-MAY-1992 13:41:10 KLUM "add #ifndef NO_RAGS"*/
/* *7    30-APR-1992 22:20:44 GOSSELIN "updating with RAGS animation fixes"*/
/* *6    10-APR-1992 13:20:21 FITZELL "always let rags check to see that a ps file is a ps file and not some other supported type"*/
/* *5    28-MAR-1992 12:58:39 GOSSELIN "fixed data_type"*/
/* *4    19-MAR-1992 13:18:55 GOSSELIN "stream_lf'ed"*/
/* *3    19-MAR-1992 13:16:59 GOSSELIN "added new RAGS support"*/
/* *2     3-MAR-1992 17:02:39 KARDON "UCXed"*/
/* *1    16-SEP-1991 12:40:08 PARMENTER "calls Rags"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_RAGS.C*/
#ifndef VMS
 /*
#else
# module BKR_RAGS "V03-0003"
#endif
#ifndef VMS
  */
#endif

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
**	Display routines for RAGS data.
**
**  AUTHORS:
**
**      James A. Ferguson
**
**  CREATION DATE:     20-Jul-1990
**
**  MODIFICATION HISTORY:
**
**      V03-0002  DLB0001	David L Ballenger	19-Feb-1991
**                Add support for BMD_CHUNK_RAGS_NO_FILL for PIC graphics 
**                support.
**
**	V03-0002    DLB0002     David L Ballenger       06-Feb-1991
**                  Correct capitalization of geGks.h.
**
**	V03-0001    JAF0001 	James A. Ferguson   	20-Jul-1990
**  	    	    Removed routines from BKR_DISLAY module.
**
**--
**/

#ifndef NO_RAGS

/*
 * INCLUDE FILES
 */

#include    <string.h>
#include "br_common_defs.h"  /* common BR #defines */
#include "br_meta_data.h"    /* typedefs and #defines for I/O of BR files */
#include "br_typedefs.h"     /* BR high-level typedefs and #defines */
#include "br_globals.h"      /* BR external variables declared here */
#include "bkr_rags.h"        /* function prototypes for .c module */
#include "geGks.h"

#ifdef VMS
# define    GRAPHICS_DIR	    "DECW$BOOK:"
#else
# define    GRAPHICS_DIR    	    "/usr/lib/dxbook"
#endif	/* VMS */


/*
 * EXTERNAL ROUTINES
 */

extern unsigned		    geDispArt();



/*
 * FORWARD DEFINITIONS
 */

static unsigned char	    rags_init = FALSE;
static GC   	    	    white_gc = NULL;



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_rags_read
**
** 	Read routine for handling the CHUNK_RAGS data type.
**
**  FORMAL PARAMETERS:
**
**  	window	    - window to display in.
**  	data_addr   - address of data to display.
**  	data_len    - length of data to display.
**  	xoff	    - x offset of origin in window.
**  	yoff	    - y offset of origin in window.
**  	xclip	    - x value of clip rectangle.
**  	yclip	    - y value of clip rectangle.
**  	wclip	    - width of clip rectangle.
**  	hclip	    - height of clip rectangle.
**  	    	    	    NOTE: clip rect. is wrt window origin, 
**  	    	    	      	  NOT wrt (xoff, yoff)
**  	display_handle - Rags segment handle
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
**	Returns:    handle for graphic being displayed, or
**  	    	    NODISPLAY value if an error occurred.
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
BR_HANDLE
bkr_rags_read PARAM_NAMES((window,data_addr,data_len,xoff,yoff,
                           xclip,yclip,wclip,hclip,display_handle,data_type))
    Window		window PARAM_SEP /*  window to display in	    */
    BR_UINT_8		*data_addr PARAM_SEP /*  address of data to display */
    BR_UINT_32		data_len PARAM_SEP /*  length of data to display    */
    int 		xoff PARAM_SEP	/*  x offset of origin in window    */
    int 		yoff PARAM_SEP	/*  y offset of origin in window    */
    int 		xclip PARAM_SEP	/*  x value of clip rectangle	    */
    int 		yclip PARAM_SEP	/*  y value of clip rectangle	    */
    int 		wclip PARAM_SEP	/*  width of clip rectangle	    */
    int 		hclip PARAM_SEP	/*  height of clip rectangle	    */
					/*  NOTE: clip rect. is wrt window  */
					/*  origin, NOT wrt (xoff, yoff)    */
    BR_HANDLE	    	display_handle PARAM_SEP /*  Rags segment handle    */
    BR_UINT_32	    	data_type PARAM_END      /*  data type              */

{
    BR_HANDLE	handle;
    BR_INT_32   rags_data_type;

    if (( data_type == BMD_CHUNK_RAGS ) ||
	( data_type == BMD_CHUNK_RAGS_NO_FILL ))
        rags_data_type = GEIMGTYPE_RAGS;
    else if ( data_type == BMD_CHUNK_POSTSCRIPT )
        rags_data_type = GEIMGTYPE_NONE; /* really GEIMGTYPE_PS */
    else
        rags_data_type = GEIMGTYPE_NONE;

    if ( ! rags_init )
    	bkr_rags_init();

    if ( display_handle == NODISPLAY )
	return ( NODISPLAY );

    /* No display_handle, tell RAGS to read the data */

    if ( display_handle == 0 )
    {
    	/* 
    	 *  If length is less than minimum graphics object (154 bytes),
	 *  Assume it is old RAGS format (data is RAGS Filename)
	*/
    	if ( data_len <= 154 )
    	    handle = bkr_rags_file( data_addr );
    	else
    	    handle = bkr_rags_mem( data_addr, data_len, rags_data_type);
    }
    else    /* Valid handle passed; so use it! */
    	handle = display_handle;

    if ( ( handle == 0 ) || ( handle == NODISPLAY ) )
	return ( NODISPLAY );
    else
      return ( handle );

};   /* end of bkr_rags_read */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_rags_display
**
** 	Display routine for handling the CHUNK_RAGS data type.
**
**  FORMAL PARAMETERS:
**
**  	window	    - window to display in.
**  	data_addr   - address of data to display.
**  	data_len    - length of data to display.
**  	xoff	    - x offset of origin in window.
**  	yoff	    - y offset of origin in window.
**  	xclip	    - x value of clip rectangle.
**  	yclip	    - y value of clip rectangle.
**  	wclip	    - width of clip rectangle.
**  	hclip	    - height of clip rectangle.
**  	    	    	    NOTE: clip rect. is wrt window origin, 
**  	    	    	      	  NOT wrt (xoff, yoff)
**  	display_handle - Rags segment handle
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
**	Returns:    handle for graphic being displayed, or
**  	    	    NODISPLAY value if an error occurred.
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
BR_HANDLE
bkr_rags_display PARAM_NAMES((window,data_addr,data_len,xoff,yoff,
                           xclip,yclip,wclip,hclip,display_handle,data_type))
    Window		window PARAM_SEP /*  window to display in	    */
    BR_UINT_8		*data_addr PARAM_SEP /*  address of data to display */
    BR_UINT_32		data_len PARAM_SEP /*  length of data to display    */
    int 		xoff PARAM_SEP	/*  x offset of origin in window    */
    int 		yoff PARAM_SEP	/*  y offset of origin in window    */
    int 		xclip PARAM_SEP	/*  x value of clip rectangle	    */
    int 		yclip PARAM_SEP	/*  y value of clip rectangle	    */
    int 		wclip PARAM_SEP	/*  width of clip rectangle	    */
    int 		hclip PARAM_SEP	/*  height of clip rectangle	    */
					/*  NOTE: clip rect. is wrt window  */
					/*  origin, NOT wrt (xoff, yoff)    */
    BR_HANDLE	    	display_handle PARAM_SEP /*  Rags segment handle    */
    BR_UINT_32	    	data_type PARAM_END      /*  data type              */

{
    BR_HANDLE	handle;
    BR_INT_32   rags_data_type;

    if (( data_type == BMD_CHUNK_RAGS ) ||
	( data_type == BMD_CHUNK_RAGS_NO_FILL ))
        rags_data_type = GEIMGTYPE_RAGS;
    else if ( data_type == BMD_CHUNK_POSTSCRIPT )
        rags_data_type = GEIMGTYPE_NONE; /* really GEIMGTYPE_PS */
    else
        rags_data_type = GEIMGTYPE_NONE;

    if ( ! rags_init )
    	bkr_rags_init();

    if ( display_handle == NODISPLAY )
	return ( NODISPLAY );

    /* No display_handle, tell RAGS to read the data */

    if ( display_handle == 0 )		/* Should NEVER happen		    */
    {
    	/* 
    	 *  If length is less than minimum graphics object (154 bytes),
	 *  Assume it is old RAGS format (data is RAGS Filename)
	*/
    	if ( data_len <= 154 )
    	    handle = bkr_rags_file( data_addr );
    	else
    	    handle = bkr_rags_mem( data_addr, data_len, rags_data_type);
    }
    else    /* Valid handle passed; so use it! */
    	handle = display_handle;

    if ( ( handle == 0 ) || ( handle == NODISPLAY ) )
	return ( NODISPLAY );

    /*    we don't want to do this anymore	*/

/*    if (data_type != BMD_CHUNK_RAGS_NO_FILL) {

         * First burn the exposed area to make sure we have a 
         * white background  
         *
        XFillRectangle( bkr_display, window, white_gc, xclip, yclip, wclip, hclip );
    } */

    geDispArt(
	(long)GECDISP,	/*  Command	*/
	bkr_display,	/*  Display	*/
	window,		/*  Window	*/
	NULL,		/*  FileName	*/
	data_len,	/*  Buff length */
	&handle,	/*  &Segment	*/
	xoff,		/*  xoff	*/
	yoff,		/*  yoff	*/
	xclip,		/*  xclip	*/
	yclip,		/*  yclip	*/
	wclip,		/*  wclip	*/
	hclip,     	/*  hclip	*/
	rags_data_type,	/*  type	*/
	NULL );

    return ( handle );

};   /* end of bkr_rags_display */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_rags_close
**
** 	Closes a rags graphics given its handle.
**
**  FORMAL PARAMETERS:
**
**	handle - id of the displayed graphic.
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
bkr_rags_close PARAM_NAMES((handle))
    BR_HANDLE handle PARAM_END
{
    if ( ( ! rags_init ) || ( ! handle ) || ( handle == NODISPLAY ) )
	return;

    geDispArt(
	(long)GEKILL,		/*  Command	*/
	bkr_display,	/*  Display	*/
	NULL,		/*  Window	*/
	NULL,		/*  FileName	*/
	0,		/*  Buff length */
	&handle,	/*  &Segment	*/
	0,		/*  xoff	*/
	0,		/*  yoff	*/
	0,		/*  xclip	*/
	0,		/*  yclip	*/
	0,		/*  wclip	*/
	0,		/*  hclip	*/
	0,		/*  type	*/
	NULL );
};   /* end of bkr_rags_close */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_rags_wevent
**
** 	Passes event to rags.
**
**  FORMAL PARAMETERS:
**
**	handle - id of the displayed graphic.
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
bkr_rags_wevent PARAM_NAMES((window,display_handle,event))
    BKR_WINDOW	   *window PARAM_SEP
    BR_HANDLE	   display_handle PARAM_SEP	/*  Rags segment handle      */
    XButtonEvent   *event PARAM_END		/*  pointer to event	     */
{
    int		cknum;
    BR_HANDLE	handle;
    BMD_CHUNK 	*chunk;

    if (window && event)
      {if ( !(handle = display_handle) )
	 {/*
	   * See if can find a RAGS chunk
	   */
	  chunk = (BMD_CHUNK *) &window->u.topic.chunk_list[0];
	  for ( cknum = 0; cknum < window->u.topic.num_chunks;
	       cknum++, chunk++ )
	    {if (( chunk->chunk_type == BMD_DATA_CHUNK ||
		  chunk->chunk_type == BMD_DATA_SUBCHUNK ) &&
		 ( ( chunk->data_type == BMD_CHUNK_RAGS )
		  || ( chunk->data_type == BMD_CHUNK_RAGS_NO_FILL )))
	       handle = chunk->handle;
	    }
	 }

       if (handle)
	 geDispArt(
		   (long)GEWEVENT,		/*  Command	*/
		   bkr_display,		/*  Display	*/
		   event->window,       /*  Window	*/
		   NULL,		/*  FileName	*/
		   0,		/*  Buff length */
		   &handle,		/*  &Segment	*/
		   0,		/*  xoff	*/
		   0,		/*  yoff	*/
		   0,		/*  xclip	*/
		   0,		/*  yclip	*/
		   0,		/*  wclip	*/
		   0,		/*  hclip	*/
		   0,		/*  type	*/
		   event );
      }

};   /* end of bkr_rags_wevent */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_rags_file
**
** 	Display a RAGS figure given the filename.
**
**  FORMAL PARAMETERS:
**
**	data_addr - pointer to the filename.
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
**	Returns:    handle for graphic being displayed, or
**  	    	    NODISPLAY value if an error occurred.
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
BR_HANDLE
bkr_rags_file PARAM_NAMES((data_addr))
     BR_UINT_8 * data_addr PARAM_END
{
    char		filename[256];
    BR_HANDLE		handle = NULL;	/*  Rags segment handle	    */
    int 		error_code;

    strcpy( filename, GRAPHICS_DIR );
    strcpy( &filename[strlen( GRAPHICS_DIR )], data_addr );

    error_code = geDispArt(
	(long)GEREAD,	/*  Command	*/
	bkr_display,	/*  Display	*/
	NULL,		/*  Window	*/
	filename,	/*  FileName	*/
	0,		/*  Buff length */
	&handle,	/*  &Segment	*/
	0,		/*  xoff	*/
	0,		/*  yoff	*/
	0,		/*  xclip	*/
	0,		/*  yclip	*/
	0,		/*  wclip	*/
	0,		/*  hclip	*/
	GEIMGTYPE_RAGS,	/*  type	*/
	NULL);

    if ( error_code == -1 )
	return NODISPLAY;
    else
	return handle;

};   /* end of bkr_rags_file */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_rags_init
**
** 	Initializes the RAGS display interface.
**
**  FORMAL PARAMETERS:
**
**	none
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
bkr_rags_init( VOID_PARAM )
{
    unsigned int    value_mask;
    XGCValues       xgcvalues;
    Window  	    gc_window;

    gc_window = (Window) XDefaultRootWindow( bkr_display );

    if ( rags_init )
	return;

    geDispArt(
	(long)GEINIT,		/*  Command	*/
	bkr_display,	/*  Display	*/
	gc_window,	/*  Window	*/
	NULL,		/*  FileName	*/
	0,		/*  Buff length */
	NULL,		/*  &Segment	*/
	0,		/*  xoff	*/
	0,		/*  yoff	*/
	0,		/*  xclip	*/
	0,		/*  yclip	*/
	0,		/*  wclip	*/
	0,		/*  hclip	*/
	0,		/*  type	*/
	NULL );
    xgcvalues.background = XWhitePixel( bkr_display, XDefaultScreen( bkr_display ) );
    xgcvalues.foreground = xgcvalues.background;
    value_mask = GCForeground | GCBackground;
    white_gc = (GC) XCreateGC( bkr_display, XDefaultRootWindow( bkr_display ),
    	    	    	value_mask, &xgcvalues );

    rags_init = TRUE;

};   /* end of bkr_rags_init */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_rags_mem
**
** 	Display a RAGS figure given a pointer to the display data.
**
**  FORMAL PARAMETERS:
**
**	none
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
**	Returns:    handle for graphic being displayed, or
**  	    	    NODISPLAY value if an error occurred.
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
BR_HANDLE
bkr_rags_mem PARAM_NAMES((data_addr,data_len,rags_type))
    BR_UINT_8		*data_addr PARAM_SEP	/*  address of data buffer	    */
    BR_UINT_32		data_len PARAM_SEP	/*  length of data buffer	    */
    BR_UINT_32		rags_type PARAM_END	/* type of data			    */
{
    BR_HANDLE		handle = NULL;	/*  Rags segment handle	    */
    int		error_code;

    error_code = geDispArt(
	(long)GEREADM,	/*  Command	*/
	bkr_display,	/*  Display	*/
	NULL,		/*  Window	*/
	data_addr,	/*  Buffer	*/
	data_len,	/*  Buff length */
	&handle,	/*  &Segment	*/
	0,		/*  xoff	*/
	0,		/*  yoff	*/
	0,		/*  xclip	*/
	0,		/*  yclip	*/
	0,		/*  wclip	*/
	0,		/*  hclip	*/
	rags_type,	/*  data type   */
	NULL);
 
    if ( error_code == -1 )
	return NODISPLAY;
    else
	return handle;

};   /* end of bkr_rags_mem */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_rags_term
**
** 	Terminates use of the RAGS display interface.
**
**  FORMAL PARAMETERS:
**
**	none
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
bkr_rags_term( VOID_PARAM )
{
    if ( ! rags_init )
	return;

    geDispArt(
	(long)GETERM,		/*  Command	*/
	bkr_display,	/*  Display	*/
	NULL,		/*  Window	*/
	NULL,		/*  FileName	*/
	0,		/*  Buff length */
	NULL,		/*  &Segment	*/
	0,		/*  xoff	*/
	0,		/*  yoff	*/
	0,		/*  xclip	*/
	0,		/*  yclip	*/
	0,		/*  wclip	*/
	0,		/*  hclip	*/
	0,		/*  type	*/
	NULL );
    XFreeGC( bkr_display, white_gc );
    white_gc = NULL;

    rags_init = FALSE;

};   /* end of bkr_rags_term */


#endif /* NO_RAGS */
