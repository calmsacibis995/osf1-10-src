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

/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_DISPLAY.C*/
/* *18   24-FEB-1993 17:47:10 BALLENGER "Fixes for large topic and Region memory leak."*/
/* *17    1-FEB-1993 10:01:37 RAMSHAW "Int QAR #10 - Graphics scroll"*/
/* *16    7-AUG-1992 10:05:36 KLUM "fix NO_RAGS"*/
/* *15   19-JUN-1992 20:19:20 BALLENGER "Cleanup for Alpha/OSF port."*/
/* *14    8-JUN-1992 19:05:17 BALLENGER "UCX$CONVERT"*/
/* *13    8-JUN-1992 12:45:39 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *12   30-APR-1992 22:20:35 GOSSELIN "updating with RAGS animation fixes"*/
/* *11   30-MAR-1992 16:03:20 BALLENGER "fix reference to font_entry->font"*/
/* *10   28-MAR-1992 17:26:04 BALLENGER "Add font support for converted postscript"*/
/* *9    19-MAR-1992 13:17:18 GOSSELIN "added new RAGS support"*/
/* *8     8-MAR-1992 19:16:06 BALLENGER "  Add topic data and text line support"*/
/* *7     3-MAR-1992 16:58:09 KARDON "UCXed"*/
/* *6    12-FEB-1992 11:59:31 PARMENTER "Using I18n lib for title and icon names"*/
/* *5     4-FEB-1992 14:29:31 FITZELL "alpha alignment changes"*/
/* *4    13-NOV-1991 14:50:15 GOSSELIN "alpha checkins"*/
/* *3     1-NOV-1991 13:04:46 BALLENGER "reintegrate  memex support"*/
/* *2    17-SEP-1991 18:13:20 BALLENGER "include function prototype headers"*/
/* *1    16-SEP-1991 12:39:07 PARMENTER "Display text, images, and RAGS graphics"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_DISPLAY.C*/
#ifndef VMS
 /*
#else
#module BKR_DISPLAY "V03-0004"
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
**	Display routines for text, images, and RAGS graphics.
**
**  AUTHORS:
**
**      James A. Ferguson
**
**  CREATION DATE:     26-Jan-1990
**
**  MODIFICATION HISTORY:
**
**      V03-0004    DLB0001	David L Ballenger	19-Feb-1991
**                  Add support for BMD_CHUNK_RAGS_NO_FILL for PIC 
**                  graphics support.
**
**	V03-0003    JAF0002	James A. Ferguson   	8-Oct-1990
**  	    	    Modify bkr_display_fext to NOT align RULE and TEXT
**  	    	    data packets on VMS for performance.
**
**	V03-0002    JMK0002	Joe Kraetsch		11-Jul-1990
**		    Add Display Postscript
**
**	V03-0001    JAF0001	James A. Ferguson   	26-Jan-1990
**  	    	    Extracted V2 routines and created new module.
**
**--
**/


/*
 * INCLUDE FILES
 */

#include <X11/Xlib.h>
#include "br_common_defs.h"  /* common BR #defines */
#include "br_meta_data.h"    /* typedefs and #defines for I/O of BR files */
#include "br_typedefs.h"     /* BR high-level typedefs and #defines */
#include "br_globals.h"      /* BR external variables declared here */
#include "bkr_display.h"     /* function prototypes for .c module */
#ifdef BKR_DPS_ON
#include "bkr_dps.h"         /* Bookreader interface to Display PostScript */
#endif
#include "bkr_error.h"       /* Error reporting routines */
#include "bkr_fetch.h"       /* Routines to fetch resource literals */
#include "bkr_font.h"        /* Routines to handle fonts */
#include "bkr_image.h"       /* Routines to handle images */
#include "bkr_rags.h"        /* Bookreader interface to rags routines */
#include "bkr_sgx.h"         /* Routines to handle simple graphics */


#define	    MAX_TEXT_ITEMS	    82

/*
 * FORWARD ROUTINES
 */
static void  	    nodisplay_data();


/*
 * FORWARD DEFINITIONS 
 */

static XTextItem    	    item_list[MAX_TEXT_ITEMS];	    

static char    	    	    *error_string;


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_display_data
**
** 	Dispatch routine which calls the appropriate display routines 
**  	given the data type.
**
**  FORMAL PARAMETERS:
**
**  	window_id    - window to draw in.
**  	chunk	     - pointer to the chunk containing the data.
**  	vwx 	     - x-coordinate of the parent Topic.
**  	vwy 	     - y-coordinate of the parent Topic.
**  	expose	     - pointer to the XExpose event.
**      window       - pointer to the BKR_WINDOW to display data in
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
bkr_display_data PARAM_NAMES((window_id,chunk,vwx,vwy,expose,window))
    Window	 window_id PARAM_SEP
    BMD_CHUNK    *chunk PARAM_SEP
    int	         vwx PARAM_SEP  /*  virtual window x, y relative to */
    int	         vwy PARAM_SEP  /*  origin of the "window"  */
    XExposeEvent *expose PARAM_SEP
    BKR_WINDOW   *window PARAM_END

{
    BR_UINT_8  	*data_end = &chunk->data_addr[chunk->data_len];
    int		xoff, yoff, xclip, yclip, wclip, hclip;
    BMD_IMAGE_PKT 	*image;

    /*  translate virtual origin to physical origin  */

    xoff = vwx + chunk->rect.left;
    yoff = vwy + chunk->rect.top;

    /*  compute the intersection of the chunk and expose rectangles  */

    xclip = MAX( xoff, expose->x );
    yclip = MAX( yoff, expose->y );
    wclip = MIN( xoff + chunk->rect.width, expose->x + expose->width ) - xclip;
    hclip = MIN( yoff + chunk->rect.height, expose->y + expose->height ) - yclip;

    if ( chunk->data_addr == NULL )
	return;
    if ( chunk->data_len == 0 )
	return;
    if ( ( wclip <= 0 ) || ( hclip <= 0 ) )
	return;

    /*  if we already failed once--don't try again  */

    if ( chunk->handle == NODISPLAY )
    {
	nodisplay_data( window_id, chunk, xoff, yoff, xclip, yclip, wclip, hclip );
	return;
    }

    switch ( chunk->data_type )
    {
    	case BMD_CHUNK_FTEXT:   	    /* DOCUMENT Formatted Text */
    	    chunk->handle = bkr_display_ftext( 
	    	window_id, 
	    	chunk->data_addr, chunk->data_len, 
	    	xoff, yoff,
	    	xclip, yclip, wclip, hclip,
    	    	window,
		chunk->handle );
	    break;

    	case BMD_CHUNK_RAGS:    	/* RAGS Graphics Editor format */
    	case BMD_CHUNK_RAGS_NO_FILL:
#ifdef NO_RAGS
            chunk->handle = NODISPLAY;
#else
	    if (!chunk->handle)
	      chunk->handle = bkr_rags_read(
				    window_id, 
		    	    	    chunk->data_addr, chunk->data_len,
		    	    	    xoff, yoff, 
		    	    	    xclip, yclip, wclip, hclip, 
    	    	    	    	    chunk->handle,
                                    chunk->data_type );

	    chunk->handle = bkr_rags_display(
		    	    	    window_id, 
		    	    	    chunk->data_addr, chunk->data_len,
		    	    	    xoff, yoff, 
		    	    	    xclip, yclip, wclip, hclip, 
    	    	    	    	    chunk->handle,
                                    chunk->data_type );
#endif 
	    break;

	case BMD_CHUNK_IMAGE:	/* Bitmap Image -- screen resolution  */
	    image = (BMD_IMAGE_PKT *) chunk->data_addr;
	    if ( ! chunk->handle )
	    {
	    	chunk->handle = bkr_image_init(
		    image->data, chunk->data_len,
		    image->pix_width, image->pix_height,
		    image->res_x, image->res_y,
		    chunk->data_type );
		if ( ! chunk->handle )
		    break;
	    }

	    bkr_image_display(
    	    	window_id, 
	    	image->data, chunk->data_len,
	    	xoff, yoff,
	    	xclip, yclip, wclip, hclip,
	    	chunk->rect.width, chunk->rect.height,
	    	chunk->handle );
	    break;

    	case BMD_CHUNK_IMAGE75: 	    /* NOTE: Version 1.0 books ONLY	*/
	    if ( ! chunk->handle )
	    {
	    	short int   pixel_width  = (short int) chunk->rect.width - 1,
			    pixel_height = (short int) chunk->rect.height - 1;

	    	/*
	    	 * Scale the width and height to 75dpi coordinates because
	    	 * for Version 1.0 books all graphics were created at 75dpi
	    	 * and stored in the database at 300dpi coordinates.
	    	 * We must do this because the width and height have already
	    	 * been scaled from 300dpi (in bkr_page.c) to the output 
	    	 * resolution which is different.
	    	 * NOTE: we do no rounding because this causes problems!
	    	 */

	    	if ( bkr_monitor_resolution != 75 )
	    	{
		    pixel_width = SCALE_GRAPHIC_VALUE75( pixel_width );
		    pixel_height = SCALE_GRAPHIC_VALUE75( pixel_height );
	    	}
	    	chunk->handle = bkr_image_init(
		    chunk->data_addr, chunk->data_len, 
		    pixel_width, pixel_height,	    /* in pixels */
		    75, 75, 	    	    	    /* created resolution */
		    chunk->data_type );

		if ( ! chunk->handle )
		    break;
	    }

	    bkr_image_display(
	    	window_id, 
	    	chunk->data_addr, chunk->data_len,
	    	xoff, yoff,
	    	xclip, yclip, wclip, hclip,
	    	chunk->rect.width, chunk->rect.height,	/* window width and height */
	    	chunk->handle );
	    break;

    	case BMD_CHUNK_POSTSCRIPT:    	/* Encapsulated PostScript format */
#ifdef BKR_DPS_ON
#ifdef GEDPS_ON
#ifdef NO_RAGS
            chunk->handle = NODISPLAY;
#else
	    if (!chunk->handle)
	      chunk->handle = bkr_rags_read(
				    window_id, 
		    	    	    chunk->data_addr, chunk->data_len,
		    	    	    xoff, yoff, 
		    	    	    xclip, yclip, wclip, hclip, 
    	    	    	    	    chunk->handle,
                                    chunk->data_type );

            chunk->handle = bkr_rags_display(
                                       window_id,
                                       chunk->data_addr, chunk->data_len,
                                       xoff, yoff,
                                       xclip, yclip, wclip, hclip,
                                       chunk->handle,
                                       chunk->data_type );
#endif
#else
	    chunk->handle = bkr_dps_display( window_id, chunk, 
    	    	    	    	xoff, yoff, xclip, yclip, wclip, hclip );
#endif
#else
            chunk->handle = NODISPLAY;
#endif
	    if ( chunk->handle == NODISPLAY ) 
	    {
		/*  put up a message and flag chunk as failed  */

	    	error_string = (char *) 
		    bkr_fetch_literal( "POSTSCRIPT_DISPLAY_ERROR", ASCIZ );
	    	if ( error_string != NULL )
	    	{
		    sprintf( errmsg, error_string );
		    bkr_error_modal( errmsg, window_id );
		    XtFree( error_string );
	    	}
		nodisplay_data( window_id, chunk, xoff, yoff, xclip, yclip, 
    	    	    	    	wclip, hclip );
	    }
	    else if ( chunk->handle != 0 )
	    {
		/*  close the context--we will recreate it each time for now  */
#ifdef BKR_DPS_ON
		bkr_dps_close( chunk->handle );
#endif 
		chunk->handle = 0;
	    }
	    break;

    	case BMD_CHUNK_SGX: 	    /* Simple X Graphics format */
    	    chunk->handle = bkr_sgx_display( window_id, chunk,
    	    	    	    	xoff, yoff, xclip, yclip, wclip, hclip );
    	    if ( chunk->handle == NODISPLAY )
    	    {
    	    	/*  put up a message and flag chunk as failed  */

	    	error_string = (char *) 
		    bkr_fetch_literal( "GRAPHIC_DISPLAY_ERROR", ASCIZ );
	    	if ( error_string != NULL )
	    	{
		    sprintf( errmsg, error_string );
		    bkr_error_modal( errmsg, window_id );
		    XtFree( error_string );
	    	}
		nodisplay_data( window_id, chunk, xoff, yoff, xclip, yclip, 
    	    	    	    	wclip, hclip );
    	    }
    	    break;

    	case BMD_CHUNK_ASCII:
    	case BMD_CHUNK_DDIF:
    	case BMD_CHUNK_PIXMAP:
    	    /* WARNING! Falling through to default case for now */
	default:
	    error_string = (char *) 
		bkr_fetch_literal( "UNKNOWN_DATA_FORMAT", ASCIZ );
	    if ( error_string != NULL )
	    {
		sprintf( errmsg, error_string, chunk->data_type );
		bkr_error_modal( errmsg, window_id );
		XtFree( error_string );
	    }
	    chunk->handle = NODISPLAY;
	    nodisplay_data( window_id, chunk, xoff, yoff, 
    	    	       xclip, yclip, wclip, hclip );
	    break;
    }    

};   /* end of bkr_display_data */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_display_ftext
**
** 	Display routine for handling a single FTEXT packet.
**  	An FTEXT packet can contain zero or more of the following
**  	types:
**
**  	    BMD_FTEXT_RULE
**  	    BMD_FTEXT_TEXT300
**  	    BMD_FTEXT_TEXT400
**
**  FORMAL PARAMETERS:
**
**  	window_id    - window_id to draw in.
**  	data_addr    - address of data to display.
**  	data_len     - length of data to display.
**  	xoff	     - x offset of origin in window.
**  	yoff	     - y offset of origin in window.
**  	xclip	     - x value of clip rectangle.
**  	yclip	     - y value of clip rectangle.
**  	wclip	     - width of clip rectangle.
**  	hclip	     - height of clip rectangle.
**  	    	    	    NOTE: clip rect. is wrt window origin, NOT wrt
**  	    	    	    	  (xoff, yoff).
**      window       - pointer to the BKR_WINDOW to display data in
**	handle	     - need a handle to pass back a nodisplay condition
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
bkr_display_ftext PARAM_NAMES((window_id,data_addr,data_len,xoff,yoff,
                               xclip,yclip,wclip,hclip,window,handle))
    Window       window_id PARAM_SEP
    BR_UINT_8	 *data_addr PARAM_SEP
    unsigned     data_len PARAM_SEP
    int 	 xoff PARAM_SEP
    int 	 yoff PARAM_SEP
    int 	 xclip PARAM_SEP
    int 	 yclip PARAM_SEP
    int 	 wclip PARAM_SEP
    int 	 hclip PARAM_SEP
    BKR_WINDOW   *window PARAM_SEP
    BR_HANDLE     handle PARAM_END

{
    int 		x, y, w, h;
    int 		y_bottom = yclip + hclip;
    BMD_FTEXT_PKT	*packet = (BMD_FTEXT_PKT *) data_addr;
    BMD_RULE_PKT	rule;
    BMD_RULE_PKT	*rule_ptr = (BMD_RULE_PKT *) &rule;
    BMD_TEXT_PKT	text;
    BMD_TEXT_PKT	*text_ptr = (BMD_TEXT_PKT *) &text;
    BR_UINT_8   	*data_end = &data_addr[data_len];
    BKR_FONT_DATA	*font_entry;
    BR_UINT_16          major_version = window->shell->book->version.major_num;

#define TEXT_PTR( pkt )	    ( (BMD_TEXT_PKT *) &( (BMD_FTEXT_PKT *) (pkt) )->value[0] )
#define WORD_PTR( txt )	    ( (BMD_WORD_PKT *) &(txt)->data[0] )
#define WORD_START( pkt )   WORD_PTR( TEXT_PTR( (pkt) ) )
#define WORD_END( pkt )	    ( (BMD_WORD_PKT *) &(pkt)->value[(pkt)->len - 2] )

    if ( data_addr == 0 )
	return(handle);

    while ( packet < (BMD_FTEXT_PKT *) data_end )
    {
	switch ( packet->tag )
	{
	    case BMD_FTEXT_RULE:
#if defined(VMS) && !defined(ALPHA)
    	    	rule_ptr = (BMD_RULE_PKT *) &packet->value[0];
#else
    	    	memcpy( &rule, &packet->value[0], sizeof( BMD_RULE_PKT ) );
#endif
	    	x = xoff + SCALE_VALUE( rule_ptr->x, major_version );
	    	y = yoff + SCALE_VALUE( rule_ptr->y, major_version );
	    	w = MAX( 1, SCALE_VALUE( rule_ptr->width, major_version ) );
	    	h = MAX( 1, SCALE_VALUE( rule_ptr->height, major_version ) );
/* Don't bother with the following test. If we got here this chunk is in the
 * 'event_region' so we shouldn't need to test whether this trule is visible.
 * The test is wrong anyway (a rule which starts above the clip area and
 * extends below it will not get drawn and it should).
 *
 *	    	if ( ( yclip <= y ) 	&& 
 *   	    	    ( y <= y_bottom )	|| 
 *   	    	    ( yclip <= y + h )	&& 
 *  	    	    ( y + h <= y_bottom ) )
 */
    	    	    XFillRectangle( bkr_display, window_id, bkr_text_gc, x, y, w, h );
	    	break;

	    case BMD_FTEXT_TEXT300:
#ifndef USE_TEXT_LINES
#if defined(VMS) && !defined(ALPHA)
    	    	text_ptr = (BMD_TEXT_PKT *) &packet->value[0];
#else
    	    	memcpy( &text, TEXT_PTR( packet ), sizeof( text ) );
#endif
	    	font_entry = window->shell->book->font_data[text_ptr->font_num];
	    	y = yoff + SCALE_VALUE300( text_ptr->y );

	    	/* Initialize font if necessary */

	    	if ( font_entry == NULL )
	    	{
                    font_entry = bkr_font_entry_init(window->shell->book,
                                                     text_ptr->font_num);
	    	}
                if ( font_entry == FONT_NOT_FOUND )
                {
                    break;
                }
	    	if ( ( y >= yclip - font_entry->font_struct->descent ) && 
		     ( y <= y_bottom + font_entry->font_struct->ascent) )
	    	{
		    unsigned	nitems = 0;
		    XTextItem	*item = item_list;
    	    	    BMD_WORD_PKT    	*word = WORD_START( packet );
    	    	    BMD_WORD_PKT 	*word_end = WORD_END( packet );

		    x = xoff + SCALE_VALUE300( text_ptr->x );

		    while ( word < word_end )
		    {
    	    	    	item->chars = (char *) &word->chars[0];
    	    	    	item->nchars = word->count;
#ifdef I18N_MULTIBYTE
			if (font_entry->font_2byte) 
			    item->nchars /= 2;
#endif
    	    	    	item->delta = SCALE_VALUE300( word->delta );
    	    	    	item->font = font_entry->font_struct->fid;
    	    	    	word = (BMD_WORD_PKT *) &word->chars[word->count];
		    	nitems++;
		    	item++;
		    }
#ifdef I18N_MULTIBYTE
		    if (font_entry->font_2byte)
			XDrawText16( bkr_display, window_id, bkr_text_gc, x, y, 
                                    (XTextItem16 *)item_list, nitems );
		    else
			XDrawText( bkr_display, window_id, bkr_text_gc, x, y, 
                                  item_list, nitems );
#else
		    XDrawText( bkr_display, window_id, bkr_text_gc, x, y, 
                              item_list, nitems );
#endif
	    	}    /* end if */
#endif
	    	break;	/* end case BMD_FTEXT_TEXT300: */

	    case BMD_FTEXT_TEXT400:
#ifndef USE_TEXT_LINES
#if defined(VMS) && !defined(ALPHA)
    	    	text_ptr = (BMD_TEXT_PKT *) &packet->value[0];
#else
    	    	memcpy( &text, TEXT_PTR( packet ), sizeof( text ) );
#endif
	    	font_entry = window->shell->book->font_data[text_ptr->font_num];
	    	y = yoff + SCALE_VALUE400( text_ptr->y );

	    	/* Initialize font if necessary */

	    	if ( font_entry == NULL )
	    	{
                    font_entry = bkr_font_entry_init(window->shell->book,
                                                     text_ptr->font_num);
	    	}
                if ( font_entry == FONT_NOT_FOUND )
                {
                    break;
                }
	    	if ( ( y >= yclip - font_entry->font_struct->descent ) && 
		    ( y <= y_bottom + font_entry->font_struct->ascent ) )
	    	{
		    unsigned	nitems = 0;
		    XTextItem	*item = item_list;
    	    	    BMD_WORD_PKT    	*word = WORD_START( packet );
    	    	    BMD_WORD_PKT 	*word_end = WORD_END( packet );

		    x = xoff + SCALE_VALUE400( text_ptr->x );

    	    	    while ( word < word_end )
		    {
    	    	    	item->chars = (char *) &word->chars[0];
    	    	    	item->nchars = word->count ;
    	    	    	item->delta = SCALE_VALUE400( word->delta );
#ifdef I18N_MULTIBYTE
			if (font_entry->font_2byte) 
			    item->nchars /= 2;
#endif
    	    	    	item->font = font_entry->font_struct->fid;
    	    	    	word = (BMD_WORD_PKT *) &word->chars[word->count];
		    	nitems++;
		    	item++;
		    }
#ifdef I18N_MULTIBYTE
		    if (font_entry->font_2byte)
			XDrawText16( bkr_display, window_id, bkr_text_gc, x, y, 
                                     (XTextItem16 *)item_list, nitems );
		    else
                        XDrawText( bkr_display, window_id, bkr_text_gc, x, y, 
                                  item_list, nitems );
#else
		    XDrawText( bkr_display, window_id, bkr_text_gc, x, y, 
                              item_list, nitems );
#endif
	    	}    /* end if */
#endif 
	    	break;	/* end case BMD_FTEXT_TEXT400: */

	    default:
	    	error_string = (char *) bkr_fetch_literal ("UNKNOWN_DATA_TAG", ASCIZ);
    	    	if ( error_string != NULL )
    	    	{
	    	    sprintf( errmsg, error_string, packet->tag, packet );
	    	    bkr_error_modal( errmsg, window_id );
	    	    XtFree( error_string );
    	    	}
		return(NODISPLAY);
	    }
    	    packet = (BMD_FTEXT_PKT *) &packet->value[packet->len - 2];
	    if ( packet->len == 0 )
	    	break;

    	}   /* end of "switch ( packet->tag )" */

    return(handle);

};  /* end of bkr_display_ftext */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	nodisplay_data
**
** 	Draws a box with diagonal lines the size of the bound box specified
**  	by the chunk to indicate that there is no data display data.
**
**  FORMAL PARAMETERS:
**
**  	window_id	- window_id to draw in.
**  	chunk	- pointer to the chunk containing the data.
**  	xoff	- x offset of origin in window.
**  	yoff	- y offset of origin in window.
**  	xclip	- x value of clip rectangle.
**  	yclip	- y value of clip rectangle.
**  	wclip	- width of clip rectangle.
**  	hclip	- height of clip rectangle.
**  	    	    NOTE: clip rect. is wrt window origin, NOT wrt
**  	    	    	  (xoff, yoff).
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
nodisplay_data ( window_id, chunk, xoff, yoff, xclip, yclip, wclip, hclip )
    Window  	    window_id;
    BMD_CHUNK	    *chunk;
    int             xoff;
    int             yoff;
    int             xclip;
    int             yclip;
    int             wclip;
    int             hclip;
{
    /*  ignore the clip rectangle and draw the whole rectangle + diagonals  */

    XDrawRectangle( 
	bkr_display, window_id, bkr_text_gc, 
	xoff, yoff, chunk->rect.width, chunk->rect.height );
    XDrawLine( 
	bkr_display, window_id, bkr_text_gc, 
	xoff, yoff, 
	xoff + chunk->rect.width, yoff + chunk->rect.height );
    XDrawLine( 
	bkr_display, window_id, bkr_text_gc, 
	xoff + chunk->rect.width, yoff, 
	xoff, yoff + chunk->rect.height );

};  /* end of nodisplay_data */




