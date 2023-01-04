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
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_TOPIC_INIT.C*/
/* *7    18-JUN-1992 16:15:50 BALLENGER "Cleanup for Alpha/OSF port."*/
/* *6     9-JUN-1992 09:59:46 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *5    14-MAR-1992 14:20:11 BALLENGER "Have bkr_topic_data_get open the page"*/
/* *4     8-MAR-1992 19:16:19 BALLENGER "  Add topic data and text line support"*/
/* *3     3-MAR-1992 17:05:27 KARDON "UCXed"*/
/* *2    17-SEP-1991 21:23:12 BALLENGER "change use of major and minor in version number to avoid conflict with sys/types.h*/
/*definition on ULTRIX"*/
/* *1    16-SEP-1991 12:40:55 PARMENTER "Initialize Topic Windows"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_TOPIC_INIT.C*/
#ifndef VMS
 /*
#else
#module BKR_TOPIC_INIT "V03-0001"
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
**	Initialization routines for opening a topic.
**
**  AUTHORS:
**
**      James A. Ferguson
**
**  CREATION DATE:     4-Feb-1990
**
**  MODIFICATION HISTORY:
**
**	V03-0001    JAF0001	James A. Ferguson   	4-Feb-1990
**  	    	    Extracted V2 routines and created new module.
**
**--
**/


/*
 * INCLUDE FILES
 */
#include "br_common_defs.h"  /* common BR #defines */
#include "br_meta_data.h"    /* typedefs and #defines for I/O of BR files */
#include "br_typedefs.h"     /* BR high-level typedefs and #defines */
#include "br_globals.h"      /* BR external variables declared here */
#include "br_malloc.h"       /* BKR_MALLOC, etc defined here */
#include "bkr_topic_init.h"  /* function prototypes for .c module */
#include "bkr_topic_callbacks.h"  /* function prototypes for .c module */
#ifdef USE_TEXT_LINES
#include "bkr_topic_data.h"
#endif 

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_topic_init_sensitivity
**
** 	Sets the sensitivity of the Next and Previous topic buttons.
**
**  FORMAL PARAMETERS:
**
**	topic_shell - pointer to the Topic shell.
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
bkr_topic_init_sensitivity PARAM_NAMES((window))
    BKR_WINDOW *window PARAM_END
{
    Boolean 	    	sensitive;
    BMD_BOOK_ID         book_id = window->shell->book->book_id;
    BMD_OBJECT_ID       page_id = window->u.topic.page_id;

    if ( window->type != BKR_STANDARD_TOPIC )
    	return;

    sensitive = bri_page_previous( book_id, page_id ) != 0;

    if ( window->widgets[W_PREV_TOPIC_BUTTON] != NULL ) {
    	XtSetSensitive( window->widgets[W_PREV_TOPIC_BUTTON], sensitive );
    }
    if ( window->widgets[W_PREV_TOPIC_ENTRY] != NULL ) {
    	XtSetSensitive( window->widgets[W_PREV_TOPIC_ENTRY], sensitive );
    }

    sensitive = bri_page_next( book_id, page_id ) != 0;
    if ( window->widgets[W_NEXT_TOPIC_BUTTON] != NULL ) {
    	XtSetSensitive( window->widgets[W_NEXT_TOPIC_BUTTON], sensitive );
    }
    if (window->widgets[W_NEXT_TOPIC_ENTRY] != NULL ) {
    	XtSetSensitive(window->widgets[W_NEXT_TOPIC_ENTRY], sensitive );
    }

};  /* end of bkr_topic_init_sensitivity */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_screen_init_sensitivity
**
** 	Sets the sensitivity of the Next and Previous screenbuttons.
**
**  FORMAL PARAMETERS:
**
**	topic_shell - pointer to the Topic shell.
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
bkr_screen_init_sensitivity PARAM_NAMES((window))
    BKR_WINDOW *window PARAM_END
{
    Boolean		next_page, prev_page, next_screen, prev_screen;
    BMD_BOOK_ID         book_id = window->shell->book->book_id;
    BMD_OBJECT_ID       page_id = window->u.topic.page_id;
    int	    	        last_screen_top;

    if ( window->type != BKR_STANDARD_TOPIC )
    	return;

    next_page =  bri_page_next( book_id, page_id );
    prev_page =  bri_page_previous( book_id, page_id );
    next_screen = ! bkr_find_last_screen_top( window, &last_screen_top ); 
    prev_screen = ( window->u.topic.y != 0 );

    if ( ! next_page && ! next_screen )
         XtSetSensitive( window->widgets[W_NEXT_SCREEN_BUTTON], FALSE );
    else XtSetSensitive( window->widgets[W_NEXT_SCREEN_BUTTON], TRUE );
  
    if ( ! prev_page &&  ! prev_screen )
         XtSetSensitive( window->widgets[W_PREV_SCREEN_BUTTON], FALSE );
    else XtSetSensitive( window->widgets[W_PREV_SCREEN_BUTTON], TRUE );

};  /* end of bkr_screen_init_sensitivity */



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_initialize_topic_chunks
**
** 	Initializes a topic's width, height and title, and its
**  	chunks list.
**
**  FORMAL PARAMETERS:
**
**  	book_id	    	    - book id of book which contains the topic.
**  	book_version	    - structure level of the book.
**  	page_id	    	    - page which contains the topic.
**  	topic_width_rtn	    - width of the topic being initialized.
**  	topic_height_rtn    - height of the topic being initialized.
**  	topic_title_rtn	    - title of the topic being initialized.
**  	chunk_list_rtn	    - chunk list of the topic being initialized.
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
**	Returns:  The number of chunks within the topic or zero.
**
**  SIDE EFFECTS:
**
**	virtual memory is allocated.
**
**--
**/
#ifndef USE_TEXT_LINES
void
bkr_initialize_topic_chunks PARAM_NAMES((window))
    BKR_WINDOW *window PARAM_END
{
    int	    	topic_width;
    int	    	topic_height;
    char    	*topic_title = NULL;
    unsigned	num_chunks = 0;
    BMD_CHUNK   *chunk;
    BMD_CHUNK	*chunk_list = NULL;
    int	    	cknum;
    BMD_BOOK_ID book_id = window->shell->book->book_id;
    unsigned short int major_version = window->shell->book->version.major_num;
    BMD_OBJECT_ID page_id = window->u.topic.page_id;

    /* Get the topic display information */

    num_chunks = bri_page_chunk_count(book_id, page_id );
    chunk_list = (BMD_CHUNK *) BKR_CALLOC( num_chunks, sizeof( BMD_CHUNK ) );
    memset( chunk_list, 0, num_chunks * sizeof( BMD_CHUNK ) );
    bri_page_data(book_id, page_id, chunk_list );

    /*  
     *  Find the title for the Topic.  First we search the chunks
     *  for the first title we find and if no title is found,
     *  we default to the books title.
     */

    for ( cknum = 0; cknum < num_chunks; cknum++ )
    {
	topic_title = bri_page_chunk_title( book_id, chunk_list[cknum].id );
	if ( ( topic_title != NULL ) && ( topic_title[0] != NULLCHAR ) )
	    break;
    }
    if ( ( ( topic_title != NULL ) && ( topic_title[0] == NULLCHAR ) ) 
    	    || ( topic_title == NULL ) )
	topic_title = bri_book_title( book_id );

    /* Get dimensions of main chunks and topic */

    topic_width = 0;
    topic_height = 0;
    for ( cknum = 0; cknum < num_chunks; cknum++ )
    {
	chunk = (BMD_CHUNK *) &chunk_list[cknum];

	/* NOTE: We need to convert x, y, width, height to pixels.
	 *       The conversion depends on chunk data type.
    	 *  	 We add 1 to the scaled width and height for round off.
    	 */

	chunk->width = SCALE_VALUE( chunk->width, major_version ) + 1;
	chunk->height = SCALE_VALUE( chunk->height, major_version ) + 1;

	if ( chunk->parent == 0 )   	/*  main level chunk?  */
	{
	    chunk->y = topic_height;	/*  chunk->x = 0  */
	    topic_width = MAX( topic_width, chunk->width );
	    topic_height += chunk->height;
	}
	else	    	    	    /*  sub-chunk, x-ref, or extension  */
	{
	    BMD_CHUNK	*pchunk;
	    int		pcknum;

	    /*  find the parent chunk  */
	    for ( pcknum = 0; pcknum < cknum; pcknum++ )
	    {
		pchunk = (BMD_CHUNK *) &chunk_list[pcknum];
		if ( pchunk->id == chunk->parent )
		{
		    chunk->x = SCALE_VALUE( chunk->x, major_version ) + pchunk->x;
		    chunk->y = SCALE_VALUE( chunk->y, major_version ) + pchunk->y;
		    if ( chunk->x > 0 )	    /* Decrement if > 0  */
			chunk->x--;
		    if ( chunk->y > 0 )	    /* Decrement if > 0  */
			chunk->y--;

		    /*  if it's a polygon, set up the point list  */
		    if ( ( chunk->chunk_type == BMD_REFERENCE_POLY ) ||
			 ( chunk->chunk_type == BMD_EXTENSION_POLY ) )
		    {
			int	point;
			int	origin_x, origin_y;
			BMD_POINT	*new_point_vec;

			origin_x = chunk->x - pchunk->x;
			origin_y = chunk->y - pchunk->y;

			chunk->xpoint_vec = (XPoint *)
    	    	    	    BKR_CALLOC( chunk->num_points + 1, sizeof(XPoint) );
			new_point_vec = (BMD_POINT *)
			    BKR_CALLOC( chunk->num_points + 1, sizeof(BMD_POINT) );
			for ( point = 0; point < chunk->num_points; point++ )
			{
			    int	    scaled_x, scaled_y;

			    scaled_x = SCALE_VALUE(chunk->point_vec[point].x,  
                                                   major_version );
			    scaled_y = SCALE_VALUE(chunk->point_vec[point].y, 
                                                   major_version);
			    new_point_vec[point].x =  pchunk->x + scaled_x;
			    new_point_vec[point].y =  pchunk->y + scaled_y;

			    if ( chunk->chunk_type == BMD_REFERENCE_POLY )
			    {
			      /*  translate the polygon region to (0,0)  */
			      chunk->xpoint_vec[point].x = scaled_x - origin_x;
			      chunk->xpoint_vec[point].y = scaled_y - origin_y;
			    }
			}
			/*
			 *  we allocated an extra point to ensure closure of 
			 *  the polygon.  we repeat the first point at the end 
			 *  of the list to ensure this closure.
			 */

			new_point_vec[chunk->num_points].x = new_point_vec[0].x;
			new_point_vec[chunk->num_points].y = new_point_vec[0].y;
			chunk->point_vec = new_point_vec;

			if ( chunk->chunk_type == BMD_REFERENCE_POLY )
			{
			    chunk->xpoint_vec[chunk->num_points].x = 
				chunk->xpoint_vec[0].x;
			    chunk->xpoint_vec[chunk->num_points].y = 
				chunk->xpoint_vec[0].y;
			    chunk->num_points++;
			    chunk->region = XPolygonRegion(chunk->xpoint_vec,
                                                           chunk->num_points,
                                                           WindingRule );
			}
			else
			    chunk->num_points++;
		    }	    	/* end of "if BMD_REFERENCE_POLY or BMD_EXTENSION_POLY" */
		    break;
		}   	/* end of "if ( pchunk->id == chunk->parent )" */
    	    }	    /* end of "for: pcknum < cknum "    */
	}   	/* end of "if ( chunk->parent == 0 )"  */
    }	    /* end of "for: cknum < topic_num_chunks"  */

    topic_width = MAX( 1, topic_width );  /* Avoid Xlib error for empties */
    topic_height += 20;	    	    /* Pad the bottom */

    /* Fill in the values in the window structure
     */
    window->u.topic.num_chunks 	    = num_chunks;
    window->u.topic.width 	    = topic_width;
    window->u.topic.height 	    = topic_height;
    window->u.topic.title 	    = topic_title;
    window->u.topic.chunk_list 	    = chunk_list;
};  /* end of bkr_initialize_topic_chunks */
#endif 



